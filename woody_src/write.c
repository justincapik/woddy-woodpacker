#include "wwp.h"

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
    int ENC = 1;
    if (ENC)
		ParasiteLoader("./obj/ASM/parasite.bin");
	else
		ParasiteLoader("./obj/ASM/no_enc_parasite.bin");

	// ###################################################################################################################
											// PADDING + HDR ADJUSTMENTS
	// ###################################################################################################################

	// Get Home size (in bytes) of parasite residence in host
	// and check if host's home size can accomodate a parasite this big in size
	Elf64_Off	padding_size = PaddingSizeFinder(ptr);
	u_int16_t	PADDING_BOOSTED = 0;
	if (padding_size < parasite_full_size)
	{
		fprintf(stderr, BOLDRED"<o>"RESET YELLOW" Host "CYAN"%s"YELLOW" parasite too big, not supported YET :(\n"RESET, filename);
		return (0);
		fprintf(stderr, BOLDRED"<o>"RESET YELLOW" Host "CYAN"%s"YELLOW" parasite too big, boosting padding\n"RESET, filename);
		padding_size = PaddingBooster(ptr, padding_size, parasite_full_size);
		PADDING_BOOSTED = 1;
	}
	
	// Save original_entry_point of host and patch host entry point with parasite_offset
	Elf64_Addr original_entry_point = ehdr->e_entry; 
	
	ehdr->e_entry 	= parasite_load_address + encr_bundle_size;
	textend = ehdr->e_entry - encr_bundle_size;
	// Patch SHT
	SHT_Patcher(ptr);

	// ###################################################################################################################
										// PARASITE PATCHING + HOST INFESTATION
	// ###################################################################################################################

	// Patch Parasite with entrypoint and .text start
	AddrPatcher(parasite_code, 0xAAAAAAAAAAAAAAAA, textend - original_entry_point);
	AddrPatcher(parasite_code, 0x1111111111111111, textend - load_textoff);
	textend = parasite_offset;

	// call to the key generator then the enncryptor
	truekey = key_generator();
	if (ENC)
		encryptor(ptr, size);

	// ###################################################################################################################
													// WRITE
	// ###################################################################################################################

	//write memory in a new file
	int fd;
	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return (1);

	// ###################################################################################################################
	//  MEM STRAT
	// // passage d'informations pour le decryptage
	// ft_memmove((ptr + parasite_offset), truekey, 16);
	// free(truekey);

	// // Inject parasite in Host memory
	// ft_memmove((ptr + parasite_offset + encr_bundle_size), parasite_code, parasite_size);
	// write(fd, ptr, size);
	// ###################################################################################################################

	write(fd, ptr, (textend));
	write(fd, truekey, encr_bundle_size);
	write(fd, parasite_code, parasite_size);
	for (off_t i = 0; i < (padding_size - parasite_full_size); ++i)
		write(fd, "\0", 1);
	write(fd, ptr + textafter, (size - textafter));
	close(fd);

	fprintf(stdout, BOLDCYAN"<o>"RESET YELLOW" success \\o/  :  "CYAN"%s\n"RESET, filename);

	return 0;
}
