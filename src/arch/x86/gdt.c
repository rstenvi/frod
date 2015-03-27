/**
* \addtogroup GDT
* @{
* \file gdt.c
* Used to implement GDT for x86.
* \todo Document all the entries in gdt_entries.
*/
#include "sys/kernel.h"
#include "sys/process.h"
#include "hal/gdt.h"
#include "hal/tss.h"
#include "hal/hal.h"

#include "lib/string.h"

//------------ External functions and defines ------------------
// in flush.s
extern void gdt_flush(uint32_t);
extern void tss_flush(uint32_t);

/** Exact number of GDT entries. */
//#define NUM_GDT_ENTRIES 6

extern cpu_info cpus[];


//---------------- Internal function definitions ------------------

/**
Takes in intuitive values of the GDT entries and manipulate them to the format
the CPU expects to see them in.
\param[in] num Index to gdt_entries
\param[in] base Base address that should be added for each memory reference
\param[in] limit  The limit of the entry, only the first 20 bits are used.
\param[in] access Used directly, see documentation for gdt_entry
\param[in] gran Only the 4 most significant bits are used, the remaining four is
taken from limit.
*/
static void gdt_set_gate(cpu_info* c, int num, unsigned long base, unsigned long limit,
	uint8_t access, uint8_t gran);


/**
* Write the TSS entry.
* \param[in] c The CPU it should be installed on (currently executing CPU).
* \param[in] num The entry in the GDT-table.
* \param[in] ss0 Kernel stack segment
* \param[in] esp0 Kernel stack pointer
*/
void write_tss(cpu_info* c, uint32_t num, uint16_t ss0, uint32_t esp0);






//-------------- Public API function implementations -------------------

void gdt_install()	{
	// Get the executing CPU
	cpu_info* c = &cpus[lapic_cpuid()];


	c->gdt_ptr.limit = (sizeof(gdt_entry) * NUM_GDT_ENTRIES) - 1;

	c->gdt_ptr.base = (uint32_t)&c->gdt_entries;

	// Mandatory 0-entry
	gdt_set_gate(c, 0, 0, 0, 0, 0);

	// Kernel space code, offset 0x08
	gdt_set_gate(c, 1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	// Kernel space data, offset 0x10
	gdt_set_gate(c, 2, 0, 0xFFFFFFFF, 0x92, 0xCF);

	// User space code, offset 0x18
	gdt_set_gate(c, 3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

	// User space data, offset 0x20
	gdt_set_gate(c, 4, 0, 0xFFFFFFFF, 0xF2, 0xCF); 

	// TSS entry, offset 0x28
	write_tss(c, 5, 0x10, 0);

	// Map in the pointer to the current CPU. Concept taken from xv6.
	gdt_set_gate(c, 6, &c->cpu, &c->cpu+(PTR_SZ*1), 0x92, 0xCF);

	gdt_flush((uint32_t)&c->gdt_ptr);

	// Needed so that we can reference "cpu" to get the current cpu.
	set_gs(6 * 8);

	cpu = c;

	tss_flush(0x28);
}


void change_tss(pcb* p)	{
	cpu->tss.esp0 = (uint32_t)p->kstack + KSTACKSZ;
}

void set_kernel_stack(uint32_t stack)	{
	cpu->tss.esp0 = stack;
}



//----------------- Internal function implementation ------------------------

static void gdt_set_gate(cpu_info* c, int num, unsigned long base, unsigned long limit,
	uint8_t access, uint8_t gran)	{
	
	c->gdt_entries[num].base_low = (base & 0xFFFF);
	c->gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	c->gdt_entries[num].base_high = (base >> 24) & 0xFF;
 
	c->gdt_entries[num].limit_low = (limit & 0xFFFF);
	c->gdt_entries[num].granularity = ((limit >> 16) & 0x0F);
 
	c->gdt_entries[num].granularity |= (gran & 0xF0);
	c->gdt_entries[num].access = access;
}


void write_tss(cpu_info* c, uint32_t num, uint16_t ss0, uint32_t esp0)	{
	uint32_t base = (uint32_t)&c->tss;
	uint32_t limit = base + sizeof(c->tss);
	gdt_set_gate(c, num, base, limit, 0xe9, 0x00);

	memset(&c->tss, 0x00, sizeof(c->tss));

	c->tss.eflags = 0x0002;
	c->tss.ss0 = ss0;
	c->tss.esp0 = esp0;

	c->tss.cs = 0x08 | 0x03;
	c->tss.ss = c->tss.ds = c->tss.es = c->tss.fs = 0x10 | 0x03;
	c->tss.gs = (6*8) | 0x03;
}


/** @} */   // GDT
