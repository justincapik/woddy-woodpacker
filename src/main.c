#include "wwp.h"

static int			error(char *whut)
{
	ft_printfd(2, "%s", whut);
	return (1);
}

static t_file_data	*processing(char *ptr, off_t size)
{
	unsigned int	magic_number;
	unsigned int	elf_magic_number;
	t_file_data		*file_info;

	magic_number = *(int *)ptr;
	elf_magic_number = ELFMAG3 << 24 | ELFMAG2 << 16 | ELFMAG1 << 8 | ELFMAG0;
	if (magic_number ==  elf_magic_number)
	{
		file_info = (t_file_data *)malloc(sizeof(t_file_data));
		file_info->ptr = ptr;
		file_info->header = (Elf64_Ehdr *)ptr;
		printf("entry point => %x\n", u16s(file_info->header->e_entry));
		file_info->sections = (void *)(ptr + u16s(file_info->header->e_shoff));
		if (write_woody(file_info, size))
			return (NULL);
		// change file_info
		// write to other file
		
		return (file_info);
	}
	else
	{
		error("not recognized as a valid object file_info\n");
		return (NULL);
	}
}

static int			file_checker(char *file)
{
	int			fd;
	char		*ptr;
	struct stat	buf;
	t_file_data	*file_info;

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
	if ((file_info = processing(ptr, buf.st_size)) == NULL)
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
