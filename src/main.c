#include "wwp.h"

static int		error(char *whut)
{
	ft_printfd(2, "%s", whut);
	return (1);
}

void			newnm(char *ptr)
{
	unsigned int	magic_number;
	unsigned int	elf_magic_number;

	magic_number = *(int *)ptr;
	elf_magic_number = ELFMAG3 << 24 | ELFMAG2 << 16 | ELFMAG1 << 8 | ELFMAG0;
	if (magic_number ==  elf_magic_number)
		printf("C'EST UN ELF\n");
	else
		error("not recognized as a valid object file\n");
}

int				file_checker(char *file)
{
	int			fd;
	char		*ptr;
	struct stat	buf;

	if ((fd = open(file, O_RDONLY)) < 0)
		return (error("open failed\n"));
	if (fstat(fd, &buf) < 0)
		return (error("fstat failed\n"));
	if (S_ISDIR(buf.st_mode))
	{
		ft_printfd(2, "%s is a directory\n", file);
		return (1);
	}
	if ((ptr = mmap(0, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0))
		== MAP_FAILED)
		return (error("mmap failed\n"));
	newnm(ptr);
	if (munmap(ptr, buf.st_size) < 0)
		return (error("munmap failed\n"));
	if (close(fd) < 0)
		return (1);
	return (0);
}

int				main(int argc, char const *argv[])
{
	int	i;

	if (argc < 2)
		return (error("stp met un fichier merci\n"));
	i = 0;
	while (++i < argc)
		file_checker((char *)argv[i]);
	return (0);
}
