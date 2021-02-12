#include "wwp.h"

static int			error(char *whut)
{
	ft_printfd(2, "%s", whut);
	return (1);
}

static int	*processing(char *ptr, off_t size, char* filename)
{
	unsigned int	magic_number;
	unsigned int	elf_magic_number;

	magic_number = *(int *)ptr;
	elf_magic_number = ELFMAG3 << 24 | ELFMAG2 << 16 | ELFMAG1 << 8 | ELFMAG0;
	if (magic_number ==  elf_magic_number)
	{
		if (write_woody(ptr, size, filename))
			return (0);
		return (0);
	}
	else
	{
		error("not recognized as a valid object\n");
		return (0);
	}
}

static int			file_checker(char *filemame)
{
	int			fd;
	char		*ptr;
	struct stat	buf;

	if ((fd = open(filemame, O_RDONLY)) < 0)
		return (error("open failed\n"));
	if (fstat(fd, &buf) < 0)
		return (error("fstat failed\n"));
	if (S_ISDIR(buf.st_mode))
	{
		ft_printfd(2, "%s is a directory\n", filemame);
		return (1);
	}
	if ((ptr = mmap(0, buf.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0))
		== MAP_FAILED)
		return (error("mmap failed\n"));
	if (processing(ptr, buf.st_size, filemame))
		return (1);
	if (munmap(ptr, buf.st_size) < 0)
		return (error("munmap failed\n"));
	if (close(fd) < 0)
		return (error("failes to close fd\n"));
	return (0);
}

int					main(int argc, char const *argv[])
{
	int	i;

	if (argc < 2)
		return (error("stp met un fichier merci\n"));
	i = 0;
	while (++i < argc)
		file_checker((char *)argv[i]);
	return (0);
}
