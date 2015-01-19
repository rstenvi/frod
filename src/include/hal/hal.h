/**
* \ingroup GDT IDT HAL ISR
* \file hal.h
* Abstraction layer for the OS kernel and some function definitions used by
* architecture-specific code.
* \todo Should mark each function as only available on x86, but that is the only
* architecture supported, so not necessary yet.
*/

#ifndef __HAL_H
#define __HAL_H


#include "../sys/kernel.h"
#include "../sys/process.h"

#include "isr.h"
#include "gdt.h"
#include "tss.h"


#if ARCH==x86
	#define PTR_SZ 4
#else
	#define PTR_SZ 8
#endif

/**
* Information about each CPU on the system. Variables are located in apic.c.
*/
typedef struct	cpu	{
	/** ID of the processor, it is usually also the index to the array, I think. */
	uint8_t id;

	/** If the CPU has started. */
	volatile bool started;

	/** Number of times we have diabled interrupts. */
	int32_t num_cli;

	/**
	* If interrupts was enabled before we disabled them.
	*/
	bool int_enabled;

	bool boot_cpu;

	gdt_entry gdt_entries[NUM_GDT_ENTRIES];
	gdt_pointer gdt_ptr;

	tss_entry tss;


	struct cpu* cpu;
//	struct proc* proc;
} cpu_info;


/**
* Idea taken directly from xv6, cpu referes to the current CPU.
* \todo Must initialize the variable to &cpus[cpunum()]
*/
extern struct cpu *cpu asm("%gs:0");
//extern struct proc *proc asm("%gs:4");

// Defined in lapic.c
int lapic_cpuid();

bool lapic_install();


/**
* Initialize multiple processors according to the MP specification.
* \remark Regardless of what is returned, it should not affect other operations.
* \returns Returns true if multiple processors where started, returns false if
* we are only using 1 processor.
* \todo Check that everything works, regardless of how many processors are
* present and whether or not MP specification is supported.
*/
bool init_mp();

/**
* \ingroup GDT HAL
* Enable GDT for the first time.
*/
void gdt_install();

/**
* \ingroup IDT HAL
* Enable IDT
*/
void idt_install();

/**
* \ingroup ISR HAL
* Install the various interrupt routines.
*/
void isr_install();


/**
* \ingroup ISR HAL
* Register which function should be called when a specific interrupt happen.
* \param[in] n 
* \param[in] handler Pointer to the function that should be called.
*/
void register_interrupt_handler(uint8_t n, isr_t handler);

/*
inline uint32_t alock(volatile uint32_t* addr, uint32_t n_val)	{
	uint32_t ret;
	asm volatile("lock xchgl %0, %1" :
		"+m" (*addr), "=a" (ret) :
		"1" (n_val) :
		"cc");
	return ret;
}
*/

uint32_t read_eip();


/**
* \ingroup ISR
* Hardware interrupts.
*/
enum interrupt_num	{
	division_zero = 0,
	debugger = 1,
	nmi = 2,
	breakpoint = 3,
	overflow = 4,
	bounds = 5,
	invalid_opcode = 6,
	coprocessor_na = 7,
	double_fault = 8,
	coprocessor_overrun = 9,
	invld_tss = 10,
	segment_not_pres = 11,
	stack_fault = 12,
	gen_protect_fault = 13,
	page_fault = 14,
	reserved = 15,
	math_fault = 16,
	align_check = 17,
	mach_check = 18,
	simd_float_except = 19
};





/**
* \addtogroup HAL
* @{
*/

#define bochs_break() asm("xchg %bx, %bx")


/** Enable interrupts */
#define enable_int() asm("sti")

/** Disable interrupts */
#define clear_int() asm("cli")

/** Halt the processor. \remark Not a full halt. */
#define halt() asm("hlt")

/** Halt the processor completely. */
#define full_halt() asm("cli; hlt")

#define get_base_pointer(a) asm volatile("mov %%ebp, %0" : "=r"(a))


#define get_stack_pointer(a) asm volatile("mov %%esp, %0" : "=r"(a))


#define get_edi(a) asm volatile("mov %%edi, %0" : "=r"(a))
#define get_esi(a) asm volatile("mov %%esi, %0" : "=r"(a))
#define get_ebx(a) asm volatile("mov %%ebx, %0" : "=r"(a))
#define get_ebp(a) asm volatile("mov %%ebp, %0" : "=r"(a))

#define get_esp(a) asm volatile("mov %%esp, %0" : "=r"(a))

#define get_eax(a) asm volatile("mov %%eax, %0" : "=r"(a))


#define set_esp(a) asm volatile("mov %0, %%esp" : : "r" (a))
#define set_ebp(a) asm volatile("mov %0, %%ebp" : : "r" (a))

#define set_gs(a)  asm volatile("movw %0, %%gs" : : "r" (a))


#define push_all() asm("pusha")

#define pop_all() asm("popa")



/**
* Send a byte to a given port.
* Function defined in ports.s
* \param[in] port 
* \param[in] value 
*/
void outb(uint16_t port, uint8_t value);

/**
* Send a word (16 bits) to a given port.
* Function defined in ports.s
* \param[in] port 
* \param[in] value 
*/
void outw(uint16_t port, uint16_t value);

/**
* Receive a byte from a given port.
* Function defined in ports.s
* \param[in] port 
* \return 
*/
uint8_t inb(uint16_t port);

/**
* Receive a word (16 bits) from a given port.
* Function defined in ports.s
* \param[in] port 
* \return 
*/
uint16_t inw(uint16_t port);


/**
* Enable paging. This function should only be called the first time paging is
* enabled, should not be used for switching page directories.
* Function is defined in memory_asm.s
* \param[in] pagedir Address to the page directory.
*/
void paging_enable(uint32_t* pagedir);

/**
* Enter usermode, defined in common.s
* \param[in] data_sel User mode data selector (index)
* \param[in] code_sel User mode code selector (index)
*/
void enter_usermode(uint32_t data_sel, uint32_t code_sel);


/**
* Flush a TLB entry, required when changing a PDE or a PTE that was previously
* mapped to something else. In other words, you are changing an entry that was
* previously marked as present (bit 0). Implemented in memory_asm.s.
* \param[in] entry The virtual address that should be invalidated.
*/
void flush_tlb_entry(uint32_t addr);


/**
* Load a new page directory. Implemented in memory_asm.s.
* \param[in] addr Physical address of the new page directory.
* \remark The TLB is flushed automatically, so this call is enough.
*/
void load_page_dir_addr(uint32_t addr);


/**
* Get the address of the page directory that is currently in use.
* \return The physical address that is in use.
*/
uint32_t get_page_dir_addr();


/**
* Check if paging is enabled or not. Implemented in memory_asm.s.
* \return Returns 1 if paging is enabled, 0 if it's not, ZF-flag is also set
* appropriately.
*/
uint32_t paging_enabled();


/**
* Get the vendor ID (cpuid).
* \param[out] str Output string is loaded here, must be room for 12 bytes plus
* the additional zero-byte that is placed at the end.
* \return Returns the same as str.
*/
char* get_vendor_id(char* str);

uint32_t get_eip();


void change_tss(pcb* p);


/** @} */	// HAL

#endif
