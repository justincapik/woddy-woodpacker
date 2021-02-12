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


---	Inject Parasite to Host @ ptr
6.	Inject parasite code to (ptr + parasite_offset)


7.	Write infection to disk x_x

*/

int 			encr_bundle_size = 16;			// size of key + addrs
Elf64_Off		textoff;						// offset of .text segment
Elf64_Off		textend;						// offset of the end of .text segment
Elf64_Off		textlol;
Elf64_Addr		parasite_load_address;			// parasite entry point (if parasite is LSB EXEC)
Elf64_Off		parasite_offset;				// Parasite entry point (if parasite is .so)
u_int64_t		parasite_size;					
// u_int64_t		keys_size = 32;
u_int64_t		parasite_full_size;					
int8_t			*parasite_code;					// Parasite residence (in memory before meeting its host )
Elf64_Off		text_segment_end_offset;		// Location to inject parasite
char *truekey;

// generator for a 16bit long printable ascii char*
char    *key_generator()
{
    int             fdrandom;
    int             i;
    char            key[16];
    char            *retkey;

    // opening urandom to avoid gettig the same output at each exec
    // getting a 16 char long key
    fdrandom = open("/dev/urandom", O_RDONLY);
    if (!fdrandom)
    {
    	perror(RED"key_generator, open"RESET);
		exit(0x61);
    }
    read(fdrandom, &key, 16);
    close(fdrandom);

    // changing the random number for a printable ascii char
    i = -1;
    while (++i < 16)
    {
        if (key[i] < 0)
        	key[i] *= -1;
        key[i] = ((key[i] % (125 - 32)) + 33);
        // key[i] = 'A';
    }
    key[15] = 0;

    // transfering the key to a malloc version
    if (!(retkey = (char*)malloc(sizeof(char) * 16)))
    {
    	perror(RED"key_generator, malloc"RESET);
		exit(0x61);
    }
    i = -1;
    while(++i < 16)
    	retkey[i] = key[i];
    printf(YELLOW"key_value : ["RED"%s"YELLOW"]\n"RESET, retkey);
    return (retkey);
}

// Finds the placeholder (for address where our parasite code will jump after executing its body) and
// writes the host's entry point (original entry point address) to it.
void AddrPatcher(u_int8_t *parasite, long placeholder, long address)
{
	u_int8_t *ptr = parasite;
	// printf("AddrPatcher\n");
	int i;
	for (i = 0 ; i < parasite_size ; ++i)
	{
		long potential_placeholder = *((long *)(ptr + i));

		if ( !(placeholder ^ potential_placeholder) ) 
		{
			// printf("found placeholder [%d] -> [%x]\n", i, address);
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
			// Increase the sizeof this section by a parasite_full_size to accomodate parasite
            shdr[i].sh_size = shdr[i].sh_size + parasite_full_size;
			return ;
		}
    }
}

// Returns gap size (accomodation for parasite code in padding between .text segment and next segment 
// after .text segment) 
Elf64_Off PaddingSizeFinder(void *ptr)
{   
	Elf64_Ehdr	*ehdr		= (Elf64_Ehdr *) ptr;
	u_int16_t	phnum 		= ehdr->e_phnum;
	Elf64_Off	pht_offset 	= ehdr->e_phoff;
	Elf64_Phdr *phdr = (Elf64_Phdr *)(ptr + pht_offset);

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

			// get the offset of the .text segment for encryption
			textoff = phdr[i].p_offset;

			// Calculate the offset where the .text segment ends to bellow calculate padding_size 
			text_segment_end_offset	= phdr[i].p_offset + phdr[i].p_filesz;
			parasite_offset			= text_segment_end_offset;
			parasite_load_address	= phdr[i].p_vaddr  + phdr[i].p_filesz;


			// Increase its p_filesz and p_memsz by parasite_full_size (to accomodate parasite)
			phdr[i].p_filesz = phdr[i].p_filesz + parasite_full_size;
			phdr[i].p_memsz  = phdr[i].p_memsz  + parasite_full_size;

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
	parasite_full_size = parasite_size + encr_bundle_size;
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

int			encryptor(char *ptr, off_t size)
{
	char	encryptedName[] = "encrypted";

	printf(YELLOW"encryption addresss ["RED"%x "YELLOW"<-> "RED"%x"YELLOW"]\n"RESET, textoff, textend);
	// printf("taille text ->%d || %d\n", sizeof(textoff), sizeof(textend));
	int j = -1;
	for (off_t i = textoff; i < textend; i++)
		ptr[i] ^= truekey[++j % 16];

	return (0);
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
	if (padding_size < parasite_full_size)
	{
		fprintf(stderr, RED"[+]"RESET" Host "YELLOW"%s"RESET" cannot accomodate parasite, parasite is angry "RED"x_x \n"RESET, filename); 
		return 0;
	}
	
	// Save original_entry_point of host and patch host entry point with parasite_offset
	Elf64_Addr original_entry_point = ehdr->e_entry; 
	if (HOST_IS_EXECUTABLE)
		ehdr->e_entry 	= parasite_load_address + encr_bundle_size;
	else
		ehdr->e_entry 	= parasite_offset + encr_bundle_size;

	textend = ehdr->e_entry - encr_bundle_size;
	// Patch SHT
	SHT_Patcher(ptr);

	// Patch Parasite with entrypoint and .text start
	AddrPatcher(parasite_code, 0xAAAAAAAAAAAAAAAA, textend - original_entry_point);
	AddrPatcher(parasite_code, 0x1111111111111111, textend - textoff);

	// call to the key generator then the enncryptor
	truekey = key_generator();
	encryptor(ptr, size);

	// passage d'informations pour le decryptage
	ft_memmove((ptr + parasite_offset), truekey, 16);
	free(truekey);

	// Inject parasite in Host memory
	ft_memmove((ptr + parasite_offset + encr_bundle_size), parasite_code, parasite_size);

	//write memory in a new file
	int fd;
	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return (1);
	write(fd, ptr, size);
	close(fd);


	// DEBUG
	fprintf(stdout, BLUE"<+>"RED" success \\o/"RESET"  :  "GREEN"%s\n"RESET, filename);

	return 0;
}
