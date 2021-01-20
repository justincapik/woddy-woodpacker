#include "wwp.h"

int			write_woody(char *ptr, off_t size)
{
	(void)size;

	Elf64_Ehdr	*ehdr = (Elf64_Ehdr*)ptr;
	Elf64_Shdr	*shdr = (Elf64_Shdr *)(ptr + ehdr->e_shoff);
	// Elf64_Phdr	*phdr = (Elf64_Phdr*)(ptr + ehdr->e_phoff);
	int shnum = ehdr->e_shnum;

	printf("entry point => %llx\n", ehdr->e_entry);

	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *const sh_strtab_p = ptr + sh_strtab->sh_offset;
	
	printf("shnum -> [%d]\n", shnum);
	printf("phnum -> [%d]\n", ehdr->e_phnum);


    printf("\n[00]:  <  virtual addr  > <  size  > \"name\"\n");

	for (int i = 0; i < shnum; ++i)
	{
    	// printf("[%2d]: <%16x> <%16x> \"%s\"\n", i, u16s(phdr[i].p_vaddr), u16s(phdr[i].p_paddr), sh_strtab_p + shdr[i].sh_name);
    	printf("[%2d]:  <%16llx> <%8llx> \"%s\"\n", i, shdr[i].sh_addr, shdr[i].sh_size, sh_strtab_p + shdr[i].sh_name);
	}
	printf("vrai <%16llx> notre <%16llx> \n", (unsigned long long int)(ptr + ehdr->e_phoff), (unsigned long long int)(ptr + shdr[25].sh_addr + shdr[25].sh_size));

	int fd;
	if ((fd = open("woody", O_WRONLY | O_APPEND | O_CREAT, 0644)) == -1)
		return (1);
	printf("===>%c<===\n", ptr[0x2009]);
	ptr[0x2008] = 'e';
	write(fd, ptr, size);
	// waaa c'est a 2008
	return 0;
}
