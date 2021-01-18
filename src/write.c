#include "wwp.h"

int			write_woody(t_file_data *file_info, off_t size)
{
	int			fd;
	Elf64_Shdr	*section;

	if ((fd = open("./woody", O_CREAT | O_WRONLY)) == -1)
		return (1);
	if (file_info->header->e_shstrndx != SHN_UNDEF)
		printf("well the section's there, it's %d\n",
				file_info->header->e_shstrndx);
	else
		printf("well.\n");

	section = (Elf64_Shdr*)file_info->header->e_shoff;
	printf("so there's either %d or %d sections\n",
			u16s(file_info->header->e_shnum),
			file_info->header->e_shnum);
	
	for(int i = 0xffff; i < file_info->header->e_shnum; ++i)
	{
		printf("---------------%d--------------\n", i);
		printf("section true addr	-> %p\n", (section + i));
		printf("section told addr	-> 0x%llX\n", (section + i)->sh_addr);
		printf("section offset		-> 0x%llx\n", (section + i)->sh_offset);
		printf("section size		-> 0x%llx\n", (section + i)->sh_size);
		printf("section link		-> 0x%X\n", (section + i)->sh_link);
		printf("section entry size	-> 0x%llX\n", (section + i)->sh_entsize);

		//section = (void*)(file_info->ptr + file_info->header->e_shentsize);
	}



	if (file_info == NULL || size == 0)
		return (1);
	return (0);
}
