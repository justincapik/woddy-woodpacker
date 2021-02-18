#include "wwp.h"

// Loads parasite code into memory and defines parasite_code and parasite_size variables
void	ParasiteLoader(char *parasite_path)
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
	if (lstat(parasite_path, &buf) != 0)
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

// generator for a 16bit long printable ascii char*
char	*key_generator()
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
        // key[i] = 'Z';
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
    printf(BOLDRED"<o> "RESET YELLOW"key_value : "BOLDBLUE"["RESET YELLOW"%s"BOLDBLUE"]\n"RESET, retkey);
    return (retkey);
}

int		encryptor(char *ptr, off_t size)
{
	char	encryptedName[] = "encrypted";

	printf(BOLDRED"<o> "RESET YELLOW"encryption address : "BOLDBLUE"["RESET YELLOW"%x "BOLDBLUE"<-> "RESET YELLOW"%x"BOLDBLUE"]\n"RESET, textoff, textend);
	// printf("taille text ->%d || %d\n", sizeof(textoff), sizeof(textend));
	int j = -1;
	for (off_t i = textoff; i < textend; i++)
		ptr[i] ^= truekey[++j % 16];
	j = -1;
	for (off_t i = textoff; i < textend; i++)
		ptr[i] ^= truekey[12 - (++j % 8)]; // key from [12] to [5]

	return (0);
}
