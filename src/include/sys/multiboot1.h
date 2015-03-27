/**
* \file multiboot.h
*/

#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H


/** Value we receive in eax. */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

#define MULTIBOOT_INFO_MEM         (1<<0)
#define MULTIBOOT_INFO_BOOT_DEVICE (1<<1)
#define MULTIBOOT_INFO_CMDLINE     (1<<2)
#define MULTIBOOT_INFO_MODS        (1<<3)
#define MULTIBOOT_INFO_SYMS_AOUT   (1<<4)
#define MULTIBOOT_INFO_SYMS_ELF    (1<<5)
#define MULTIBOOT_INFO_MMAP        (1<<6)
#define MULTIBOOT_INFO_DRIVES      (1<<7)
#define MULTIBOOT_INFO_CONFIG      (1<<8)
#define MULTIBOOT_INFO_BOOT_NAME   (1<<9)
#define MULTIBOOT_INFO_APM         (1<<10)
#define MULTIBOOT_INFO_VBE         (1<<11)


#define MULTIBOOT_MMAP_TYPE_AVAILABLE 		1
#define MULTIBOOT_MMAP_TYPE_RESERVED 		2
// Usable ACPI information
#define MULTIBOOT_MMAP_TYPE_USABLE_ACPI 	3
// Reserved memory that must be preserved on hibernation
#define MULTIBOOT_MMAP_TYPE_RESERVED_PRES	4
// Defective RAM modules
#define MULTIBOOT_MMAP_TYPE_DEFECTIVE		5


typedef struct	{
	uint32_t tab_sz,
	str_sz,
	addr,
	reserved;
} __attribute__((packed)) multiboot_aout_table;

typedef struct	{
	uint32_t num,
	size,
	addr,
	shndx;
} __attribute__((packed)) multiboot_elf_table;


typedef struct	{
	uint32_t start;
	uint32_t end;
	char* name;
	uint32_t reserved;
} __attribute__((packed)) multiboot_module;


typedef struct	{
	uint8_t red, green, blue;
} __attribute__((packed)) multiboot_color_palette;

typedef struct	{
	multiboot_color_palette* palette_addr;
	uint8_t num_colors;
} __attribute__((packed)) multiboot_color;

typedef struct	{
	uint32_t size,
	base_addr_l,
	base_addr_h,
	length_l,
	length_h,
	type;
} __attribute__((packed)) multiboot_mmap;


typedef struct	{
	uint32_t flags,
	mem_lower,
	mem_upper,
	boot_device,
	cmdline,
	mods_count;
	multiboot_module* mods_addr;
	union	{
		multiboot_aout_table aout_sym;
		multiboot_elf_table elf_sec;
	} u;
	uint32_t mmap_length,
	mmap_addr,
	drives_length,
	drives_addr,
	config_table,
	boot_loader_name,
	apm_table,
	vbe_control_info,
	vbe_mode_info,
	vbe_mode,
	vbe_interface_seg,
	vbe_interface_off,
	vbe_interface_len;
	uint32_t framebuffer_addr_low,
	framebuffer_addr_high,
	framebuffer_pitch,
	framebuffer_width,
	framebuffer_height;
	uint8_t framebuffer_bpp,
	framebuffer_type;
	multiboot_color color;
} __attribute__((packed)) multiboot_info;


void print_mb_info(multiboot_info *mboot_ptr);

/**
* Move a named module to a given location.
* \remark This assumes that the module structure is present
* \return Returns true if module was found and we moved it.
*/
bool move_module(multiboot_info* m, char* name, uint8_t* to);


void check_necessary_flags(uint32_t flags);

#endif
