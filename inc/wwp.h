#ifndef WWP_H
# define WWP_H

# include "libmaster.h"
# include "color.h"

#include <elf.h>

# include <fcntl.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdio.h>

# include <sys/mman.h>
# include <sys/stat.h>

typedef struct	s_file_data
{
	char		*ptr;
	Elf64_Ehdr	*header;
	Elf64_Shdr	*sections;
}				t_file_data;

int 			encr_bundle_size;				// size of decryption bundle
Elf64_Off		textoff;						// offset of .text segment
Elf64_Off		load_textoff;						// load offset of .text segment
Elf64_Off		textend;						// offset of the end of .text segment
Elf64_Off		textafter;						// offset of the segment just after .text segment
Elf64_Addr		parasite_load_address;			// parasite entry point (if parasite is LSB EXEC)
Elf64_Off		parasite_offset;				// Parasite entry point (if parasite is .so)
u_int64_t		parasite_size;					// Size of parasite
u_int64_t		parasite_full_size;				// Size of parasite including key size
int8_t			*parasite_code;					// Parasite residence (in memory before meeting its host)
Elf64_Off		text_segment_end_offset;		// Location to inject parasite
char			*truekey;						// malloc version of the encryption key
u_int64_t		OffsetPadder;					// offset of the .text segment
int				HOST_IS_EXECUTABLE;				// exec || so
// ----------------MAIN-------------------
int				write_woody(char *ptr, off_t size, char *filename);

// ----------------PATCHER----------------
void			AddrPatcher(u_int8_t *parasite, long placeholder, long address);
void			SHT_Patcher(void *ptr);
Elf64_Off		PaddingBooster(void *ptr, Elf64_Off padding_size, u_int64_t parasite_full_size);
Elf64_Off		PaddingSizeFinder(void *ptr);

// ----------------INFECTOR---------------
char			*key_generator();
void			ParasiteLoader(char *parasite_path);
int				encryptor(char *ptr, off_t size);

#endif
