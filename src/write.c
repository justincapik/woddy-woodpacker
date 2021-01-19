#include "wwp.h"

int			write_woody(char *ptr, off_t size)
{
	(void)size;

	Elf64_Ehdr	*ehdr = (Elf64_Ehdr*)ptr;
	Elf64_Shdr	*shdr = (Elf64_Shdr *)(ptr + ehdr->e_shoff);
	int shnum = ehdr->e_shnum;

	printf("entry point => %llx\n", ehdr->e_entry);
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *const sh_strtab_p = ptr + sh_strtab->sh_offset;
	printf("shnum -> [%d]\n", shnum);
	Elf64_Phdr	*phdr = (Elf64_Phdr*)(ptr + ehdr->e_phoff);

    printf("\n [00]: <  virtual addr  > <  physical addr > \"name\"\n");

	for (int i = 0; i < shnum; ++i)
	{
    	printf("[%2d]: <%16llx> <%16llx> \"%s\"\n", i, phdr[i].p_vaddr, phdr[i].p_paddr, sh_strtab_p + shdr[i].sh_name);
	}

	return 0;
}
