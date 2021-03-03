#include "wwp.h"

// Finds the placeholder (for address where our parasite code will jump after executing its body) and
// writes the host's entry point (original entry point address) to it.
void		AddrPatcher(u_int8_t *parasite, long placeholder, long address)
{
	u_int8_t *ptr = parasite;
	int i;
	for (i = 0 ; i < parasite_size ; ++i)
	{
		long potential_placeholder = *((long *)(ptr + i));

		if (!(placeholder ^ potential_placeholder))
		{
			*((long *)(ptr + i)) = address;
			return;
		}
	}
}

// Patch SHT (i.e. find the last section of .text segment and increase its size by parasite_size)
void		SHT_Patcher(void *ptr)
{
	Elf64_Ehdr	*ehdr 			= (Elf64_Ehdr *) ptr;	
	Elf64_Off	sht_offset 		= ehdr->e_shoff;
	u_int16_t	shnum			= ehdr->e_shnum;
    Elf64_Off	current_section_end_offset;

    // Point shdr (Pointer to iterate over SHT) to the last entry of SHT
    Elf64_Shdr *shdr = (Elf64_Shdr *) (ptr + sht_offset);

    int i;
    for (i=0 ; i < shnum ; ++i)
    {
		current_section_end_offset = shdr[i].sh_offset + shdr[i].sh_size;
        if (text_segment_end_offset == current_section_end_offset)
        {
            // This is the last section of .text Segment
			// Increase the sizeof this section by a parasite_full_size to accomodate parasite
            shdr[i].sh_size = shdr[i].sh_size + parasite_full_size;
			return ;
		}
    }
}

// add more padding to accomodate virus
Elf64_Off	PaddingBooster(void *ptr, Elf64_Off padding_size, u_int64_t parasite_full_size)
{
	Elf64_Off	padding_size_mem = padding_size;

	// while (padding_size < parasite_full_size)
	padding_size += getpagesize();
	padding_size -= padding_size_mem;

	Elf64_Ehdr	*ehdr		= (Elf64_Ehdr *) ptr;
	int			i;

	u_int16_t	phnum		= ehdr->e_phnum;
	Elf64_Off	pht_offset	= ehdr->e_phoff;
	Elf64_Phdr	*phdr = (Elf64_Phdr *)(ptr + pht_offset);
	int			flag = 0;

	for (i = 0 ; i < phnum ; ++i)
	{
		printf("phdr[%d]", i);
		// Find all segment after .text Segment and add padding
		if (phdr[i].p_offset > OffsetPadder || flag)
		{
			printf(" -> boosted");
			flag = 1;
			phdr[i].p_offset += padding_size;
		}
		printf("\n");
	}

	u_int16_t	shnum			= ehdr->e_shnum;
	Elf64_Off	sht_offset		= ehdr->e_shoff;
    Elf64_Shdr	*shdr = (Elf64_Shdr *) (ptr + sht_offset);

	for (i = 0 ; i < shnum ; ++i)
	{
		printf("shdr[%d]", i);
		// Find all shdr after .text segment
		if (shdr[i].sh_offset > OffsetPadder)
		{
			printf(" -> boosted");
			shdr[i].sh_offset += padding_size;
		}
		printf("\n");
	}
	ehdr->e_shoff += padding_size;
	padding_size += padding_size_mem;
	return (padding_size);
}

u_int64_t  textoff_getter(void *ptr)
{
	Elf64_Ehdr	*ehdr		= (Elf64_Ehdr *) ptr;


	u_int16_t	shnum			= ehdr->e_shnum;
	Elf64_Off	sht_offset		= ehdr->e_shoff;
    Elf64_Shdr	*shdr = (Elf64_Shdr *) (ptr + sht_offset);
    char *ajustor = ptr + (shdr + ehdr->e_shstrndx)->sh_offset;

	for (int i = 0 ; i < shnum ; ++i)
	{
        if (!ft_strcmp((char*)(ajustor + shdr[i].sh_name), ".text"))
    		return ((size_t)shdr[i].sh_offset);
	}
    return (0);
}

u_int64_t  load_textoff_getter(void *ptr)
{
	Elf64_Ehdr	*ehdr		= (Elf64_Ehdr *) ptr;


	u_int16_t	shnum			= ehdr->e_shnum;
	Elf64_Off	sht_offset		= ehdr->e_shoff;
    Elf64_Shdr	*shdr = (Elf64_Shdr *) (ptr + sht_offset);
    char *ajustor = ptr + (shdr + ehdr->e_shstrndx)->sh_offset;

	for (int i = 0 ; i < shnum ; ++i)
	{
        if (!ft_strcmp((char*)(ajustor + shdr[i].sh_name), ".text"))
    		return ((size_t)shdr[i].sh_addr);
	}
    return (0);
}

// Returns gap size (accomodation for parasite code in padding between .text segment and next segment 
// after .text segment) 
// Also Patch phdr
Elf64_Off	PaddingSizeFinder(void *ptr)
{   
	Elf64_Ehdr	*ehdr		= (Elf64_Ehdr *) ptr;
	u_int16_t	phnum 		= ehdr->e_phnum;
	Elf64_Off	pht_offset 	= ehdr->e_phoff;
	Elf64_Phdr *phdr = (Elf64_Phdr *)(ptr + pht_offset);

	OffsetPadder = 0;
	// Parse PHT entries
	u_int16_t TEXT_SEGMENT_FOUND = 0;
	int i;
	for (i = 0 ; i < phnum ; ++i)
	{
		// printf("phdr[%d]", i);
		// Find the .text Segment (containing .text section)
		if (TEXT_SEGMENT_FOUND  == 0 &&
			phdr[i].p_type  == PT_LOAD &&
			phdr[i].p_flags == (PF_R | PF_X))
		{
			// printf("text seg");
			TEXT_SEGMENT_FOUND = 1;
			// get the offset of the .text segment for encryption
			textoff = phdr[i].p_offset;
			load_textoff = phdr[i].p_vaddr;
			if (!textoff)
			{
				printf(BOLDRED"<o> "RESET YELLOW"textoff is down, getting another one\n"RESET);
				textoff = textoff_getter(ptr);
				load_textoff = load_textoff_getter(ptr);
			}
			if (!textoff)
				printf(BOLDRED"<o> "RESET YELLOW"cannot get textoff right\n"RESET);

			// Calculate the offset where the .text segment ends to bellow calculate padding_size 
			text_segment_end_offset	= phdr[i].p_offset + phdr[i].p_filesz;
			parasite_offset			= text_segment_end_offset;
			parasite_load_address	= phdr[i].p_vaddr  + phdr[i].p_filesz;

			// Increase its p_filesz and p_memsz by parasite_full_size (to accomodate parasite)
			phdr[i].p_filesz = phdr[i].p_filesz + parasite_full_size;
			phdr[i].p_memsz  = phdr[i].p_memsz  + parasite_full_size;

			// Make text segment writable
			phdr[i].p_flags = PF_R | PF_W | PF_X;

			OffsetPadder = phdr[i].p_offset + phdr[i].p_filesz;
		}
		// Find next segment after .text Segment and calculate padding size
		else if (TEXT_SEGMENT_FOUND)
		{
			// printf("aftertext seg\n");
			textafter = phdr[i].p_offset;
			if (!textoff)
				printf(BOLDRED"<o> "RESET YELLOW"textafter is down, doesnt sound good\n"RESET);
			// Return padding_size
			return (phdr[i].p_offset - parasite_offset);
		}
		// printf("\n");
	}

	return 0;
}
