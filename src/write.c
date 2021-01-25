#include "wwp.h"
void lafonction(void);
extern u_int32_t lasize;

int			write_woody(char *ptr, off_t size)
{
	(void)size;

	Elf64_Ehdr	*ehdr = (Elf64_Ehdr*)ptr;
	Elf64_Shdr	*shdr = (Elf64_Shdr *)(ptr + ehdr->e_shoff);
	Elf64_Phdr	*phdr = (Elf64_Phdr*)(ptr + ehdr->e_phoff);
	int shnum = ehdr->e_shnum;
	int phnum = ehdr->e_phnum;
	// int the_place = 0;

	printf("entry point => %llx\n", ehdr->e_entry);

	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *const sh_strtab_p = ptr + sh_strtab->sh_offset;
	
	printf("shnum -> [%d]\n", shnum);
	printf("phnum -> [%d]\n", phnum);


    printf("\n[00]:  <  virtual addr  > <  size  > \"name\"\n");


    // #########################################################################################################################################
    // #########################################################################################################################################
    // #########################################################################################################################################

	for (int i = 0; i < shnum; ++i)
	{
    	// printf("[%2d]: <%16x> <%16x> \"%s\"\n", i, u16s(phdr[i].p_vaddr), u16s(phdr[i].p_paddr), sh_strtab_p + shdr[i].sh_name);
    	printf("[%2d]:  <%016llx> <%08llx> \"%s\"\n", i, shdr[i].sh_addr, shdr[i].sh_size, sh_strtab_p + shdr[i].sh_name);
		// if (!ft_strcmp((char*)(sh_strtab_p + shdr[i].sh_name), ".text"))
		// {
		// 	printf("found .text -> [%2d] addr <%016llx>\n", i, shdr[i].sh_addr);
		// }
	}

    // #########################################################################################################################################
    // #########################################################################################################################################
    // #########################################################################################################################################
 	
 	Elf64_Phdr* segaddr;
 	int end_text_addr;
    int ptr_begin_text_origin;
	for (int i = 0; i < phnum; ++i)
	{
		printf("[%2d] [%d]\n", i , phdr[i].p_flags);
		if (phdr[i].p_type == PT_LOAD && phdr[i].p_flags & 0x011)
		{
			printf("text found -> %d\n", i);
			ptr_begin_text_origin = i;
			segaddr = &phdr[i];
			// end_text_addr = phdr[i].p_vaddr + phdr[i].p_memsz;
			end_text_addr = phdr[i].p_offset + phdr[i].p_filesz;
			printf("off %llx size %llx\n", phdr[i].p_offset, phdr[i].p_filesz);
		}
	}

	printf("addr de debut -> <%p>\n", segaddr);
	printf("addr de fin   -> <%p>\n", ptr + end_text_addr);
	printf("addr de fin   -> <%x>\n", end_text_addr);


    // #########################################################################################################################################
    // #########################################################################################################################################
    // #########################################################################################################################################
	

	// int			nfd;
	// struct stat	buf2;
	// char		*asmptr;


	// if ((nfd = open("obj/ASM/payload.o", O_RDONLY)) < 0)
	// 	return (1);
	// if (fstat(nfd, &buf2) < 0)
	// 	return (1);
	// if (S_ISDIR(buf2.st_mode))
	// {
	// 	ft_printfd(2, "%s is a directory\n", "obj/ASM/payload.o");
	// 	return (1);
	// }
	// if ((asmptr = mmap(0, buf2.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, nfd, 0))
	// 	== MAP_FAILED)
	// 	return (1);

    // #########################################################################################################################################
    // #########################################################################################################################################
 //    // #########################################################################################################################################
	
	// Elf64_Ehdr	*ehdr2 = (Elf64_Ehdr*)asmptr;
	// Elf64_Shdr	*shdr2 = (Elf64_Shdr *)(asmptr + ehdr2->e_shoff);

	// Elf64_Shdr *sh_strtab2 = &shdr2[ehdr2->e_shstrndx];
	// const char *const sh_strtab_p2 = asmptr + sh_strtab2->sh_offset;

	// printf("\nle mechant\n");
	// int ptr_begin_text;
	// for (int i = 0; i < shnum; ++i)
	// {
 //    	// printf("[%2d]: <%16x> <%16x> \"%s\"\n", i, u16s(phdr2[i].p_vaddr), u16s(phdr2[i].p_paddr), sh_strtab_p2 + shdr2[i].sh_name);
 //    	printf("[%2d]:  <%016llx> <%08llx> \"%s\"\n", i, shdr2[i].sh_addr, shdr2[i].sh_size, sh_strtab_p2 + shdr2[i].sh_name);
	// 	if (!ft_strcmp((char*)(sh_strtab_p2 + shdr2[i].sh_name), ".text"))
	// 	{
	// 		ptr_begin_text = i;
	// 		printf("found .text -> [%2d] addr <%016llx>\n", i, shdr2[i].sh_addr);
	// 	}
	// }
	// printf("evidemment\n");
	// (void)ptr_begin_text;
	// printf("voila\n");

    // #########################################################################################################################################
    // #########################################################################################################################################
    // #########################################################################################################################################

	int fd;
	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		return (1);

    // #########################################################################################################################################
    // #########################################################################################################################################
    // #########################################################################################################################################

/*
	printf("addr -> {%016llx}\n", shdr2[ptr_begin_text].sh_addr);
	printf("addr -> {%p}\n", asmptr + shdr2[ptr_begin_text].sh_addr);
	printf("addr -> {%p}\n", &shdr2[ptr_begin_text]);
	
	u_int64_t old_one = ((phdr[ptr_begin_text_origin].p_vaddr + phdr[ptr_begin_text_origin].p_memsz - ehdr->e_entry) * (-1));
	ft_memcpy(asmptr + 0x1019, &old_one, 8);

	// ft_memcpy(asmptr + 0x1019, &ehdr->e_entry, 8);
  	// my_elfi_mem_subst (ptr + end_text_addr, shdr2[ptr_begin_text].sh_size, 0x11111111, (long)ehdr->e_entry);

	printf("old entry point => %llx\n", ehdr->e_entry);
	printf("his old entry point => %llx\n", (phdr[ptr_begin_text_origin].p_vaddr + phdr[ptr_begin_text_origin].p_memsz - ehdr->e_entry) * (-1));
	ehdr->e_entry = (Elf64_Addr)(end_text_addr);
	printf("new entry point => %llx\n", ehdr->e_entry);

	phdr[ptr_begin_text_origin].p_filesz += shdr2[ptr_begin_text].sh_size;
	phdr[ptr_begin_text_origin].p_memsz += shdr2[ptr_begin_text].sh_size;

	// shdr[15].sh_size += shdr2[ptr_begin_text].sh_size;

	ft_memmove(ptr + 0x11db, "lol", 3);
	ft_memmove(ptr + end_text_addr, asmptr + shdr2[ptr_begin_text].sh_offset, shdr2[ptr_begin_text].sh_size);
	ft_memcpy(ptr + 0x2008, "Joie ", 5);

*/
    // #########################################################################################################################################
    // #########################################################################################################################################
    // #########################################################################################################################################

	printf("%d\n", ptr_begin_text_origin);

	// ft_memmove(asmptr + 0x25d, &ehdr->e_entry, 8);

	printf("%p <=> %llx\n", &ehdr->e_entry, ehdr->e_entry);
	// ehdr->e_entry = (Elf64_Addr)(end_text_addr);
	// phdr[ptr_begin_text_origin].p_filesz += lasize;
	// phdr[ptr_begin_text_origin].p_memsz += lasize;
	// shdr[15].sh_size += lasize;


	// write(fd, ptr, size);
	write(fd, ptr, end_text_addr);
	write(fd, &lafonction, lasize);
	// write(fd, "lol", 3);
	write(fd, ptr + end_text_addr + lasize, (size - end_text_addr - lasize));



    // #########################################################################################################################################
    // #########################################################################################################################################
    // #########################################################################################################################################

	// if (munmap(asmptr, buf2.st_size) < 0)
	// 	return (1);
	// close(nfd);
	close(fd);
	return 0;
}
