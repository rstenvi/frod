/**
* \file process.c
*/

#include "sys/process.h"
#include "sys/heap.h"
#include "sys/pmm.h"
#include "sys/dllist.h"

#include "hal/hal.h"

#include "lib/string.h"
#include "lib/stdio.h"




pcb* proc_current = NULL;

pcb* proc_list = 0;

uint32_t last_pid = 0;


extern void trap_ret();
extern void return_fork();
pcb* alloc_proc(uint32_t, uint32_t, uint32_t);
int real_fork(uint32_t ret_stack);

/**
* This address and X MB upwards is virtual memory dedicated to the process
* system. VMM dirtables are stored here, nothing else.
* \todo Decide how much memory should be reserved. We can for example allow 2^16
* processes, then we would need to reserve 256 MB of memory.
*/

void process_dummy()	{
	kprintf(K_BOCHS_OUT, "Process dummy\n");
	for(;;);
}


int process_init()	{

	pcb* p = alloc_proc((uint32_t)process_dummy, 0x00, 0x00);
	p->state = PROC_RUNNING;
	//p->state = PROC_READY;

	// All of this is owerwritten when changing tasks
	// Set the registers to appropriate values, EIP is already set
	p->regs->ds = 0x10;
	p->regs->es = 0x10;
	p->regs->fs = 0x10;
	p->regs->gs = 0x10;
	p->regs->eflags = 0x202;
	p->regs->cs = 0x08;

	// Set as the current page directoty
	p->dirtable = (uint32_t*)get_page_dir_addr();

	// Should point to itself
	p->next = p;

	proc_current = p;

	change_tss(p);

	return p->pid;
}

/**
* Allocate a process.
*/
pcb* alloc_proc(uint32_t exit, uint32_t ret2, uint32_t ret_stack)	{
	// Step 1: Allocate space and set default variables
	pcb* p = (pcb*)heap_malloc(sizeof(pcb));
	p->pid = ++last_pid;

	/** \todo Handle this scenario. */
	if(last_pid >= PROC_MAX_PID)	PANIC("Max PID used");
	p->state = PROC_READY;
	

	// Step 2: Create a kernel stack
	// TODO: To support any other kernel stack size other than 4KB, we have to
	// allocate more physical memory
	uint32_t virt_addr = (uint32_t)(PROC_VMM_START + (last_pid * KSTACKSZ));
	uint32_t phys_addr = (uint32_t)pmm_alloc_first();
	if(phys_addr == 0)	{
		PANIC("Unable to allocate physical frame");
	}
	if(vmm_map_page(phys_addr, virt_addr, X86_PAGE_WRITABLE))	{
		PANIC("Unable to map address");
	}
	p->kstack = (uint8_t*)virt_addr;


	// Step 3: Set up the return stack, so it returns to the correct place
	uint32_t sp = (uint32_t)(p->kstack);
	sp += KSTACKSZ;

	sp -= 4;
	*(uint32_t*)sp = ret2;
	sp -= 4;
	*(uint32_t*)sp = ret_stack;

	// All registers, except EIP is 0
	sp -= sizeof(*p->regs);
	p->regs = (Registers*)sp;
	memset(p->regs, 0x00, sizeof(*p->regs));
	p->regs->eip = exit;
	return p;
}


/**
* \todo Save registers.
* - This is normally called via syscalls, then registers are saved, when called
* from the kernel we should simulate this, can use the same code from our
* interrupt stub to fill out the Registers struct.
*/
int fork()	{
	return real_fork((uint32_t)__builtin_return_address(0));
}

int real_fork(uint32_t ret_stack)	{
	// Must disable interrupts for this
	clear_int();
	uint32_t ebp;//, ebx, edi, esi;
	get_ebp(ebp);


	// Allocate the struct
	pcb* new_proc = alloc_proc((uint32_t)return_fork,
		(uint32_t)__builtin_return_address(0), ret_stack);
	
	memcpy(new_proc->regs, proc_current->regs, sizeof(*new_proc->regs));

	new_proc->regs->eip = (uint32_t)return_fork;
	new_proc->regs->ebp = ebp;


	// Create an address space for this process
	uint32_t virt_addr = (uint32_t)(0x10000000 + (last_pid * 4096));
	uint32_t phys_addr = vmm_create_address_space(virt_addr);

	new_proc->dirtable = (uint32_t*)phys_addr;
	
	uint32_t pid = new_proc->pid;

	new_proc->next = proc_current->next;
	proc_current->next = new_proc;


	enable_int();
	return pid;
}



uint32_t switch_task(Registers* regs)	{
	//if(proc_current == NULL || proc_current == proc_current->next)
	if(proc_list == NULL)
		return 0;
	kprintf(K_BOCHS_OUT, "DS = %x | CS = %x\n", regs->ds, regs->cs);

	pcb* old = proc_current;

	memcpy(old->regs, regs, sizeof(*old->regs));
	old->state = PROC_READY;


	//proc_current = proc_current->next;
//	proc_current = proc_current;
	proc_current = proc_list;
	proc_list = old;

	change_tss(proc_current);
	
	vmm_switch_pdir(proc_current->dirtable);
	
	proc_current->state = PROC_RUNNING;
	
	return (uint32_t)((proc_current->kstack + KSTACKSZ) - sizeof(*proc_current->regs) - 8);
}


void usermode()	{
	while(true);
}

void process_start_usermode()	{
	pcb* p = alloc_proc((uint32_t)usermode, 0x00, 0x00);
	p->state = PROC_READY;

	// Set the registers to appropriate values, EIP is already set
	p->regs->ds = 0x20 | 0x03;
	p->regs->es = 0x20 | 0x03;
	p->regs->fs = 0x20 | 0x03;
	p->regs->gs = 0x20 | 0x03;
	p->regs->eflags = 0x00000200;
	p->regs->cs = 0x18 | 0x03;

	// Set as the current page directoty
	uint32_t virt_addr = (uint32_t)(0x10000000 + (last_pid * 4096));
	uint32_t phys_addr = vmm_create_address_space(virt_addr);
	p->dirtable = (uint32_t*)phys_addr;

	// Should point to itself
	p->next = p;

	proc_list = p;
//	proc_current = p;

//	change_tss(p);

}

