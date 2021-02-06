#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <elf.h>
#include <stdlib.h>

extern int errno ;

int		error(char *message)
{
	printf(message);
	return (1);
}

int		main(int argc, char **argv)
{
	int		src_fd;
	int		dst_fd;
	char		*ptr;
	char		key[] = "HAH sucker.";
	char		encryptedName[] = "encrypted";
	struct stat	buf;

	printf("argv 1 => %s\n", argv[1]);
	if (argc <= 1 || (src_fd = open(argv[1], O_RDONLY) < 0))
		return (error("hey couldn't open src file\n"));
	if (fstat(src_fd, &buf) < 0)
		return (error("fstat failed\n"));
	printf("buf size = %d\n", buf.st_size);
	if (S_ISDIR(buf.st_mode))
		return (error("it's a folder yo\n"));
	if ((ptr = mmap(0, buf.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, src_fd, 0))
		== MAP_FAILED)
	{
		fprintf(stderr, "Error doing mmap: %s\n", strerror(errno));
		return (error("mmap failed\n"));
	}
	if ((dst_fd = open(encryptedName, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)	
		return (error("ey couldn't make the encryption file\n"));
	printf("key is %s\n", key);

	return (0);
}
