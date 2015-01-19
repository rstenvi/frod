/**
* \ingroup multiboot
* \file multiboot.h
* Definitions for parsing the information we get from the bootloader. This file
* deals with multiboot 1, specification gathered from:
* http://git.savannah.gnu.org/cgit/grub.git/tree/doc/multiboot.texi?h=multiboot
* and http://git.savannah.gnu.org/cgit/grub.git/tree/doc/multiboot.h?h=multiboot
* The state multiboot leaves us in:
* - EAX - Contains the value MULTIBOOT_BOOTLOADER_MAGIC
* - EBX - Contains the physical address to the multiboot info structure (32-bit)
* - CS - 32-bit read/execute code segment, offset of 0 and limit of 0xFFFFFFFF
* - DS, ES, FS, GS, SS - Same as CS, but a read/write data segment.
* - A20 - Enabled
* - CR0 - Bit 31 is cleared(paging disabled), all other is undefined.
* - EFLAGS - Bit 17 (VM) is cleared. Bit 0 (PE) is set. All other bits are
*   undefined.
* - All other registers are undefined, some important to take note of are:
*   - ESP - We must initialize the stack before jumping to kmain.
*   - GDT - Is set, but the value is not defined.
*   - IDT - Must set up our own.
*/

#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H

/**
* \addtogroup multiboot
* @{
*/

//---------------- Defines ----------------------------------------

/** Magic value for multiboot_header. */
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002

/** Value we receive in eax. */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/** All modules are loaded on 4KB boundaries. */
#define MULTIBOOT_PAGE_ALIGN 1<<0

/** Information about memory is available. */
#define MULTIBOOT_MEMORY_INFO 1<<1

/** Information about the video mode is available. */
#define MULTIBOOT_VIDEO_MODE 1<<2

/** Needed to properly load files in a.out format, we use ELF, so not needed. */
#define MULTIBOOT_AOUT_KLUDGE 1<<16


#define MULTIBOOT_INFO_MEM 1<<0
#define MULTIBOOT_INFO_BOOT_DEVICE 1<<1
#define MULTIBOOT_INFO_CMDLINE 1<<2
#define MULTIBOOT_INFO_MODS 1<<3
#define MULTIBOOT_INFO_SYMS_AOUT (1<<4)
#define MULTIBOOT_INFO_SYMS_ELF (1<<5)
#define MULTIBOOT_INFO_MMAP 1<<6
#define MULTIBOOT_INFO_DRIVES 1<<7
#define MULTIBOOT_INFO_CONFIG 1<<8
#define MULTIBOOT_INFO_BOOT_NAME 1<<9
#define MULTIBOOT_INFO_APM 1<<10
#define MULTIBOOT_INFO_VBE 1<<11
#define MULTIBOOT_INFO_FB 1<<12


#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB 1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2


#define MULTIBOOT_MMAP_TYPE_AVAILABLE 		1
#define MULTIBOOT_MMAP_TYPE_RESERVED 		2
// Usable ACPI information
#define MULTIBOOT_MMAP_TYPE_USABLE_ACPI 	3
// Reserved memory that must be preserved on hibernation
#define MULTIBOOT_MMAP_TYPE_RESERVED_PRES	4
// Defective RAM modules
#define MULTIBOOT_MMAP_TYPE_DEFECTIVE		5





//-------------------- Structures ---------------------------------

/**
* Mandatory fields in the header.
*/
typedef struct	{
	/** Must be the value MULTIBOOT_HEADER_MAGIC */
	uint32_t magic,
	
	/**
	* Interesting bit values.
	* - MULTIBOOT_PAGE_ALIGN
	* - MULTIBOOT_MEMORY_INFO
	* - MULTIBOOT_VIDEO_MODE
	* - MULTIBOOT_AOUT_KLUDGE
	*/
	flags,

	/** When this is added to magic and flags, the result should be zero. */
	checksum;
} __attribute__((packed)) multiboot_header;

/**
* Available if bit MULTIBOOT_AOUT_KLUDGE in multiboot_header.flags is set.
*/
typedef struct	{
	uint32_t header_addr,
	load_start_addr,
	load_end_addr,
	bss_end_addr,
	entry_addr;
} __attribute__((packed)) multiboot_aout;

/**
* Specifies the preferred graphics mode.
* Available if bit MULTIBOOT_VIDEO_MODE in multiboot_header.flags is set.
*/
typedef struct	{
	uint32_t mode_type,
	width,
	height,
	depth;
} __attribute__((packed)) multiboot_video;

/**
*/
typedef struct	{
	uint32_t tab_sz,
	str_sz,
	addr,
	reserved;
} __attribute__((packed)) multiboot_aout_table;

/**
* Information about the ELF section header table.
*/
typedef struct	{
	/** Number of entries. */
	uint32_t num,
	size,
	addr,
	shndx;
} __attribute__((packed)) multiboot_elf_table;


/**
* Information about one module.
*/
typedef struct	{
	/** Start address. */
	uint32_t start;
	/** End address. */
	uint32_t end;

	/** Null-terminated string with the name. */
	char* name;

	/**
	* Reserved bytes.
	* \remark This should not be touched.
	*/
	uint32_t reserved;
} __attribute__((packed)) multiboot_module;


typedef struct	{
	uint8_t red, green, blue;
} __attribute__((packed)) multiboot_color_palette;

typedef struct	{
	multiboot_color_palette* palette_addr;
	uint8_t num_colors;
} __attribute__((packed)) multiboot_color;


/**
* Memory map for the one piece of memory in the system.
*/
typedef struct	{
	/** The size of this structure - the four bytes for the this field. */
	uint32_t size,

	/** The lower 4 bytes of the address. */
	base_addr_l,

	/** The upper four bytes of the address. */
	base_addr_h,

	/** Lower four bytes of the length of this map. */
	length_l,

	/** Upper 4 bytes of the length of this map. */
	length_h,

	/**
	* Type of memory map, is defined in MULTIBOOT_MMAP_TYPE_*. 1 is available to
	* use.
	*/
	type;
} __attribute__((packed)) multiboot_mmap;


/**
* The multiboot info structure we are passed by GRUB.
*/
typedef struct	{
	/**
	* Determines what other information below is defined, all the different bits
	* used are in MULTIBOOT_INFO_*
	*/
	uint32_t flags,

	/** The amount of lower memory in KB, it starts at address 0. */
	mem_lower,
	/** The amount of upper memory in KB, it starts at address 1MB. */
	mem_upper,

	/**
	* Which disk device the OS image was loaded from. Has the following format:
	* - | Part3 | Part2 | Part1 | drive |
	*  - drive is the low level disk interface, like 0x00 for floppy or 0x80 for
	*    HD
	*  - Part1 is the top-level partition number.
	*  - Part2 and part3 are sub-partions
	*/
	boot_device,

	/** Physical address to null-terminated command line arguments. */
	cmdline,

	/** Number of modules loaded. Can be zero, even though bit 3 in flags is set*/
	mods_count;

	/** Physical address of first module. */
	multiboot_module* mods_addr;

	union	{
		multiboot_aout_table aout_sym;
		multiboot_elf_table elf_sec;
	} u;


	/** The length of the memory map. */
	uint32_t mmap_length,
	
	/** Physical address of the memory map.*/
	mmap_addr,

	drives_length,

	/** Physical address to drive structures.*/
	drives_addr,

	/** Address to a rom configuration table. */
	config_table,

	/** Physical address to the name of the boot loader booting the kernel. */
	boot_loader_name,

	/** Physical address of an APM table. */
	apm_table,

	/** Address to VBE control info */
	vbe_control_info,
	/**
	* Address to VBE mode info.
	*/
	vbe_mode_info,

	/** Video mode. */
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

	/**
	* Possible values:
	* - MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED
	* - MULTIBOOT_FRAMEBUFFER_TYPE_RGB
	* - MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT
	*/
	framebuffer_type;


	multiboot_color color;

} __attribute__((packed)) multiboot_info;




//-------------- Function definitions -------------------------

#if DEBUG < 2
	void print_mb_info(multiboot_info *mboot_ptr);
#endif


/**
* Check if the necessary flags are set. Prints out a message for all necessary
* flags that are not set.
* \returns Returns true if all the necessary flags are set, false if one or more
* is not set.
*/
bool check_necessary_flags(uint32_t flags);


/** @} */	// multiboot

#endif
