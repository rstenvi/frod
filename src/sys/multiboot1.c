/**
* \file multiboot.c
* Parse and print out information about multiboot structure.
* \todo Change all to use kprintf
*/

#include "sys/kernel.h"
#include "sys/multiboot1.h"
#include "lib/string.h"

#define NUM_FLAGS_BITS 12

//-------------- Internal function definitions ---------------------------
void print_mb_memory(multiboot_info* m);
void print_mb_boot_device(multiboot_info* m);
void print_mb_cmdline(multiboot_info* m);
void print_mb_modules(multiboot_info* m);
void print_mb_sym_aout(multiboot_info* m);
void print_mb_sym_elf(multiboot_info* m);
void print_mb_mmap(multiboot_info* m);
void print_mb_drives(multiboot_info* m);
void print_mb_config(multiboot_info* m);
void print_mb_boot_name(multiboot_info* m);
void print_mb_apm(multiboot_info* m);
void print_mb_vbe(multiboot_info* m);



const uint32_t flags_int[NUM_FLAGS_BITS] = {
	MULTIBOOT_INFO_MEM,
	MULTIBOOT_INFO_BOOT_DEVICE,
	MULTIBOOT_INFO_CMDLINE,
	MULTIBOOT_INFO_MODS,
	MULTIBOOT_INFO_SYMS_AOUT,
	MULTIBOOT_INFO_SYMS_ELF,
	MULTIBOOT_INFO_MMAP,
	MULTIBOOT_INFO_DRIVES,
	MULTIBOOT_INFO_CONFIG,
	MULTIBOOT_INFO_BOOT_NAME,
	MULTIBOOT_INFO_APM,
	MULTIBOOT_INFO_VBE
};

const char flags_str[NUM_FLAGS_BITS][10] = {
	"MEM",
	"BOOT_DEV",
	"CMDLINE",
	"MODS",
	"AOUT",
	"ELF",
	"MMAP",
	"DRIVES",
	"CONFIG",
	"BOOT_NAME",
	"APM",
	"VBE"
};

typedef void (*print_info)(multiboot_info*);

const print_info funcs[NUM_FLAGS_BITS] = {
	print_mb_memory,
	print_mb_boot_device,
	print_mb_cmdline,
	print_mb_modules,
	print_mb_sym_aout,
	print_mb_sym_elf,
	print_mb_mmap,
	print_mb_drives,
	print_mb_config,
	print_mb_boot_name,
	print_mb_apm,
	print_mb_vbe
};

const char mem_type_str[6][20] = {
	"unknown",
	"available",
	"reserved",
	"acpi",
	"reserve preserve",
	"defective"
};



void print_mb_info(multiboot_info *mboot_ptr)	{
	printf("Flags set: ");
	int i;
	for(i = 0; i < NUM_FLAGS_BITS; i++)	{
		if(mboot_ptr->flags & flags_int[i])	{
			printf("%s ", flags_str[i]);
		}
	}
	printf("\n");
	for(i = 0; i < NUM_FLAGS_BITS; i++)	{
		if(mboot_ptr->flags & flags_int[i])	{
			funcs[i](mboot_ptr);
		}
	}
}


void print_mb_memory(multiboot_info* m)	{
	uint32_t low = m->mem_lower;
	uint32_t upper = m->mem_upper;
	char types[][3] = {"KB", "MB", "GB"};
	uint8_t l = 0, h = 0;
	while(low > 1024 && l < 2)	{	l++; low /= 1024;	}
	while(upper > 1024 && h < 2)	{	h++; upper /= 1024;	}
	printf("Memory:\n\tStart: %i %s\n\tUpper: %i %s\n", low,
		types[l], upper, types[h]);

}

void print_mb_boot_device(multiboot_info* m)	{
	printf("Boot dev:\n\tPart3: %i, Part2: %i, Part1: %i, Drive: %i\n",
		m->boot_device & 0xff,
		(m->boot_device & 0xff00)>>8,
		(m->boot_device & 0xff0000)>>16,
		(m->boot_device & 0xff000000)>>24);
}

void print_mb_cmdline(multiboot_info* m)	{
	printf("CMD line(0x%p): %s\n", m->cmdline, m->cmdline);
}

void print_mb_modules(multiboot_info* m)	{
	printf("Modules loaded: %i\n", m->mods_count);
	uint32_t i;
	multiboot_module* mm = m->mods_addr;
	for(i = 0; i < m->mods_count; i++)	{
		kprintf(K_BOCHS_OUT, "\tModule %i loaded at: %p\n", i, mm);
		kprintf(K_BOCHS_OUT, "\t\t%s: %x -> %x\n",
			mm->name,
			mm->start,
			mm->end
		);
		uint32_t t = (uint32_t)mm;
		t += sizeof(multiboot_module);
		mm = (multiboot_module*)t;
	}
}

void print_mb_sym_aout(multiboot_info* m)	{
	(void)m;
}


void print_mb_sym_elf(multiboot_info* m)	{
	printf("ELF kernel section header\n\tNum: %i, Size: %i, Addr: 0x%X, "
		"Shndx: %i\n", m->u.elf_sec.num, m->u.elf_sec.size,
		m->u.elf_sec.addr, m->u.elf_sec.shndx);
}

void print_mb_mmap(multiboot_info* m)	{
	printf("Mmap @0x%X, length = %i\n", m->mmap_addr, m->mmap_length);
		
	multiboot_mmap* mmap =  (multiboot_mmap*)m->mmap_addr;
		
	while((uint32_t)mmap < m->mmap_addr + m->mmap_length)	{
		printf("\tMap @0x%X+0x%X, Len: %X+%X, Type: 0x%X ",
			mmap->base_addr_h,
			mmap->base_addr_l,
			mmap->length_h,
			mmap->length_l,
			mmap->type
		);

		if(mmap->type < 6)
			printf("(%s)\n", mem_type_str[mmap->type]);
		else
			printf("(unknown)\n");

		mmap = (multiboot_mmap*)((uint32_t)mmap + (mmap->size + 4));
	}

}

void print_mb_drives(multiboot_info* m)	{
	printf("Drive @0x%X, length = %i\n",
		m->drives_addr,
		m->drives_length
	);
}

void print_mb_config(multiboot_info* m)	{
	printf("Config table @0x%X\n", m->config_table);
}

void print_mb_boot_name(multiboot_info* m)	{
	printf("Boot loader name: %s\n", m->boot_loader_name);

}

void print_mb_apm(multiboot_info* m)	{
	printf("APM table @0x%X\n", m->apm_table);
}

void print_mb_vbe(multiboot_info* m)	{
	printf("VBE control @0x%X, VBE mode @0x%X\n\tVideo mode: 0x%X\n",
		m->vbe_control_info, m->vbe_mode_info,
		m->vbe_mode);
}


bool move_module(multiboot_info* m, char* name, uint8_t* to)	{
	bool ret = false;
	uint32_t i;
	multiboot_module* mm = m->mods_addr;
	for(i = 0; i < m->mods_count; i++)	{
		if(strcmp(mm->name, name) == 0)	{
			memcpy(
				to,
				(uint8_t*)mm->start,
				(mm->end - mm->start)
			);

			// Mark this memory as taken is the memory manager
			pmm_mark_mem_taken(mm->start,mm->end);

			ret = true;
			break;
		}
		uint32_t t = (uint32_t)mm;
		t += sizeof(multiboot_module);
		mm = (multiboot_module*)t;
	}
	return ret;
}


void check_necessary_flags(uint32_t flags)	{

	// Need to have information about memory
	if((flags & MULTIBOOT_INFO_MEM) == 0)	{
		PANIC("Multiboot: No information about memory\n");
	}

	
	if((flags & MULTIBOOT_INFO_MODS) == 0)	{
		PANIC("Multiboot: No information about modules loaded\n");
	}
	

	// Need information about memory map
	if((flags & MULTIBOOT_INFO_MMAP) == 0)	{
		kprintf(K_LOW_INFO, "Multiboot: No information about memory map\n");
	}
}



