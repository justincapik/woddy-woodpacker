#ifndef WWP_H
# define WWP_H

#include "libmaster.h"

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

int				write_woody(char *ptr, off_t size);
u_int64_t		u64s(u_int64_t n);
u_int32_t		u32s(u_int32_t n);
u_int16_t		u16s(u_int16_t n);

#endif
