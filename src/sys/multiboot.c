/**
* \file multiboot.c
* Parse and print out information about multiboot structure.
*/

#include "sys/kernel.h"
#include "sys/multiboot.h"
#include "lib/stdio.h"

void print_mb_info(multiboot_info *mboot_ptr)	{
	kprintf(K_BOCHS_OUT, "Available flags:\n\t");
	if(mboot_ptr->flags & MULTIBOOT_INFO_MEM)	{
		kprintf(K_BOCHS_OUT, "MEM ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_BOOT_DEVICE)	{
		kprintf(K_BOCHS_OUT, "BOOT_DEV ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_CMDLINE)	{
		kprintf(K_BOCHS_OUT, "CMDLINE ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_MODS)	{
		kprintf(K_BOCHS_OUT, "MODS ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_SYMS_AOUT)	{
		kprintf(K_BOCHS_OUT, "AOUT ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_SYMS_ELF)	{
		kprintf(K_BOCHS_OUT, "ELF ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_MMAP)	{
		kprintf(K_BOCHS_OUT, "MMAP ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_DRIVES)	{
		kprintf(K_BOCHS_OUT, "DRIVES ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_CONFIG)	{
		kprintf(K_BOCHS_OUT, "CONFIG ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_BOOT_NAME)	{
		kprintf(K_BOCHS_OUT, "BOOT_NAME ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_APM)	{
		kprintf(K_BOCHS_OUT, "APM ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_VBE)	{
		kprintf(K_BOCHS_OUT, "VBE ");
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_FB)	{
		kprintf(K_BOCHS_OUT, "FB ");
	}
	kprintf(K_BOCHS_OUT, "\n");

	// Does not print exact information
	if(mboot_ptr->flags & MULTIBOOT_INFO_MEM)	{
		uint32_t low = mboot_ptr->mem_lower;
		uint32_t upper = mboot_ptr->mem_upper;
		char types[][3] = {"KB", "MB", "GB"};
		uint8_t l = 0, h = 0;
		while(low > 1024 && l < 2)	{	l++; low /= 1024;	}
		while(upper > 1024 && h < 2)	{	h++; upper /= 1024;	}

		kprintf(K_BOCHS_OUT, "Memory:\n\tStart: %i %s\n\tUpper: %i %s\n", low,
			types[l], upper, types[h]);
	}
	
	if(mboot_ptr->flags & MULTIBOOT_INFO_BOOT_DEVICE)	{
		kprintf(K_BOCHS_OUT, "Boot dev:\n\tPart3: %i, Part2: %i, Part1: %i, Drive: %i\n",
			mboot_ptr->boot_device & 0xff,
			(mboot_ptr->boot_device & 0xff00)>>8,
			(mboot_ptr->boot_device & 0xff0000)>>16,
			(mboot_ptr->boot_device & 0xff000000)>>24);
	}
	
	if(mboot_ptr->flags & MULTIBOOT_INFO_CMDLINE)	{
		kprintf(K_BOCHS_OUT, "CMD line(0x%X): %s\n", mboot_ptr->cmdline, mboot_ptr->cmdline);
	}
	
	if(mboot_ptr->flags & MULTIBOOT_INFO_MODS)	{
		kprintf(K_BOCHS_OUT, "Modules loaded: %i\n", mboot_ptr->mods_count);
		if(mboot_ptr->mods_count > 0)	{
			kprintf(K_BOCHS_OUT, "\tLoaded at: %i\n", mboot_ptr->mods_addr);
		}
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_SYMS_AOUT)	{
		
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_SYMS_ELF)	{
		kprintf(K_BOCHS_OUT, "ELF kernel section header\n\tNum: %i, Size: %i, Addr: 0x%X, "
			"Shndx: %i\n", mboot_ptr->u.elf_sec.num, mboot_ptr->u.elf_sec.size,
			mboot_ptr->u.elf_sec.addr, mboot_ptr->u.elf_sec.shndx);
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_MMAP)	{
		kprintf(K_BOCHS_OUT, "Mmap @0x%X, length = %i\n", mboot_ptr->mmap_addr,
			mboot_ptr->mmap_length);
		
		multiboot_mmap* mmap =  (multiboot_mmap*)mboot_ptr->mmap_addr;
		
		while((uint32_t)mmap < mboot_ptr->mmap_addr + mboot_ptr->mmap_length)	{
			kprintf(K_BOCHS_OUT, "\tMap @0x%X + 0x%X, Len: %X + %i, Sz: %i, Type: 0x%X\n",
				mmap->base_addr_l, mmap->base_addr_h, mmap->length_l,
				mmap->length_h, mmap->size, mmap->type);
			mmap = (multiboot_mmap*)((uint32_t)mmap + (mmap->size + 4));
		}
		
		
		// TODO: Print the rest of the information
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_DRIVES)	{
		kprintf(K_BOCHS_OUT, "Drive @0x%X, length = %i\n", mboot_ptr->drives_addr,
			mboot_ptr->drives_length);
		// TODO: More information to print here

	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_CONFIG)	{
		kprintf(K_BOCHS_OUT, "Config table @0x%X\n", mboot_ptr->config_table);
		// TODO: More information about the table
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_BOOT_NAME)	{
		kprintf(K_BOCHS_OUT, "Boot loader name: %s\n", mboot_ptr->boot_loader_name);
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_APM)	{
		kprintf(K_BOCHS_OUT, "APM table @0x%X\n", mboot_ptr->apm_table);
		// TODO: Print more info about the table
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_VBE)	{
		kprintf(K_BOCHS_OUT, "VBE control @0x%X, VBE mode @0x%X\n\tVideo mode: 0x%X\n",
			mboot_ptr->vbe_control_info, mboot_ptr->vbe_mode_info,
			mboot_ptr->vbe_mode);
	}
	if(mboot_ptr->flags & MULTIBOOT_INFO_FB)	{
	
	}
}


bool check_necessary_flags(uint32_t flags)	{
	bool ret = true;

	// Need to have information about memory
	if((flags & MULTIBOOT_INFO_MEM) == 0)	{
		kprintf(K_FATAL, "Multiboot: No information about memory\n");
		ret = false;
	}

	/*
	// Will probably need this is the future
	if(flags & MULTIBOOT_INFO_MODS == 0)	{
		kprintf(K_FATAL, "Multiboot: No information about modules loaded\n");
		ret = false;
	}
	*/

	// Need information about memory map
	if((flags & MULTIBOOT_INFO_MMAP) == 0)	{
		kprintf(K_FATAL, "Multiboot: No information about memory map\n");
		ret = false;
	}

	return ret;
}



