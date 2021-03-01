#include "wwp.h"

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

--- Create key and encrypt .text segment
6.  injectkey to (ptr + parasite_offset)

---	Inject Parasite to Host @ ptr
7.	Inject parasite code to (ptr + parasite_offset + keysize)


8.	Write infection to woody

*/

int			write_woody(char *ptr, off_t size, char *filename)
{
	fprintf(stdout, BOLDBLUE"-x-x-x-x- "RED"\\_<O>_<O>_/ "BLUE"-x-x-x-x-\n"RED"-> "CYAN"%s\n\n"RESET, filename); 

	// ###################################################################################################################
													// INIT
	// ###################################################################################################################
	encr_bundle_size = 16; // size of parasite's needs
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *) ptr;
	HOST_IS_EXECUTABLE = 0;	// Host is LSB Executable and not Shared Object

	// Identify the binary & SKIP Relocatable, files and 32-bit class of binaries
    if (ehdr->e_type == ET_REL || ehdr->e_type == ET_CORE)
    	return (0);
	else if (ehdr->e_type == ET_EXEC)
		HOST_IS_EXECUTABLE = 1;
	else if (ehdr->e_type == ET_DYN )
		HOST_IS_EXECUTABLE = 0;
    if (ehdr->e_ident[EI_CLASS] == ELFCLASS32)
    	return (0);

	// Load Parasite into memory (from disk), uses extern 'parasite_path_for_exec' defined in main.c implicitly
	ParasiteLoader("./obj/ASM/parasite.bin");

	// ###################################################################################################################
											// PADDING + HDR ADJUSTMENTS
	// ###################################################################################################################

	// Get Home size (in bytes) of parasite residence in host
	// and check if host's home size can accomodate a parasite this big in size
	Elf64_Off	padding_size = PaddingSizeFinder(ptr);
	u_int16_t	PADDING_BOOSTED = 0;
	if (padding_size < parasite_full_size)
	{
		fprintf(stderr, BOLDRED"<o>"RESET YELLOW" Host "CYAN"%s"YELLOW" parasite too big, boosting padding\n"RESET, filename);
		padding_size = PaddingBooster(ptr, padding_size, parasite_full_size);
		PADDING_BOOSTED = 1;
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

	// ###################################################################################################################
										// PARASITE PATCHING + HOST INFESTATION
	// ###################################################################################################################

	// Patch Parasite with entrypoint and .text start
	AddrPatcher(parasite_code, 0xAAAAAAAAAAAAAAAA, textend - original_entry_point);
	if (HOST_IS_EXECUTABLE)
		AddrPatcher(parasite_code, 0x1111111111111111, textend - load_textoff);
	else
		AddrPatcher(parasite_code, 0x1111111111111111, textend - textoff);
	textend = parasite_offset;

	// call to the key generator then the enncryptor
	truekey = key_generator();
	// if (HOST_IS_EXECUTABLE == 0)
	encryptor(ptr, size);

	// passage d'informations pour le decryptage
	ft_memmove((ptr + parasite_offset), truekey, 16);
	free(truekey);

	// Inject parasite in Host memory
	ft_memmove((ptr + parasite_offset + encr_bundle_size), parasite_code, parasite_size);

	// ###################################################################################################################
													// WRITE
	// ###################################################################################################################

	//write memory in a new file
	int fd;
	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return (1);
	if (PADDING_BOOSTED)
	{
		write(fd, ptr, (textend + parasite_full_size));
		for (off_t i = 0; i < (padding_size - parasite_full_size); ++i)
			write(fd, "\0", 1);
		write(fd, ptr + textafter, (size - textafter));
	}
	else
		write(fd, ptr, size);
	close(fd);

	fprintf(stdout, BOLDCYAN"<o>"RESET YELLOW" success \\o/  :  "CYAN"%s\n"RESET, filename);

	return 0;
}
