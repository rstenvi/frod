/**
* \addtogroup GDT
* @{
* \file gdt.h
* Structures and definitions used internally by the GDT code.
*/

#ifndef __GDT_H
#define __GDT_H

#if defined(x86)
	typedef uint32_t gdt_base_addr;
#elif defined(x86_64)
	typedef uint64_t gdt_base_addr;
#endif


#define NUM_GDT_ENTRIES 7


/**
* One entry in the GDT that can be read by the CPU.
* \todo Document remaining fields, see 3.4.5 in Intel manual.
*/
typedef struct	{
	/**
	* First 16 bits of the limit, the remaining 4 bits are:
	*/
	uint16_t limit_low;
	
	/**
	* First 16, out of 32, bits in the base field.
	*/
	uint16_t base_low;

	/**
	* The next 8 (16->23) bits in the base field.
	*/
	uint8_t base_middle;

	/**
	* What kind of access is allowed.
	*/
	uint8_t access;
	uint8_t granularity;

	/**
	* The last 8 (24->31) bits in the base field.
	*/
	uint8_t base_high;
} __attribute__((packed)) gdt_entry;


/** A structure that can be read by "lgdt" */
typedef struct	{
	/** Number of entries (gdt_entry) */
	uint16_t limit;
	
	/** Physical address to first gdt_entry. */
	gdt_base_addr base;
} __attribute__((packed)) gdt_pointer;

#endif	// File

/** @} */	// GDT
