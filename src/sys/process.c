/**
* \file process.c
*/

#include "sys/process.h"
#include "sys/heap.h"
#include "sys/pmm.h"

#include "hal/hal.h"

#include "lib/string.h"
#include "lib/stdio.h"




pcb* proc_current = NULL;

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
#define PROC_VMM_START 0x10000000

void process_dummy()	{
	kprintf(K_LOW_INFO, "Process dummy\n");
	for(;;);
}

int process_init(/*Registers regs*/)	{
	pcb* p = alloc_proc((uint32_t)process_dummy, 0x00, 0x00);
	p->state = PROC_RUNNING;

	// All others are 0
	p->regs->ds = 0x10;
	p->regs->es = 0x10;
	p->regs->fs = 0x10;
	p->regs->gs = 0x10;
	p->regs->eflags = 0x202;
	p->regs->cs = 0x08;

	// Set as the current page directoty
	p->dirtable = (vmm_dirtable*)get_page_dir_addr();


	proc_current = p;
	p->next = p;

	change_tss(p);

	return p->pid;
}

pcb* alloc_proc(uint32_t exit, uint32_t ret2, uint32_t ret_stack)	{
	pcb* p = (pcb*)heap_malloc(sizeof(pcb));
	p->pid = ++last_pid;
	p->state = PROC_READY;
	
	uint32_t virt_addr = (uint32_t)(0x15000000 + (last_pid * 4096));
	uint32_t phys_addr = (uint32_t)pmm_alloc_first();
	if(phys_addr == 0)	{
		PANIC("Unable to allocate physical frame");
	}
	if(vmm_map_page(phys_addr, virt_addr))	{
		PANIC("Unable to map address");
	}

	p->kstack = (uint32_t*)virt_addr;


	uint32_t sp = (uint32_t)(p->kstack);
	sp += KSTACKSZ;

	sp -= 4;
	*(uint32_t*)sp = ret2;
	sp -= 4;
	*(uint32_t*)sp = ret_stack;

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
		__builtin_return_address(0), ret_stack);
	
	memcpy(new_proc->regs, proc_current->regs, sizeof(*new_proc->regs));

	new_proc->regs->eip = (uint32_t)return_fork;
	new_proc->regs->ebp = ebp;


	// Create an address space for this process
	uint32_t virt_addr = (uint32_t)(0x10000000 + (last_pid * 4096));
	uint32_t phys_addr = (uint32_t)pmm_alloc_first();
	if(phys_addr == 0)	{
		PANIC("Unable to allocate physical frame");
	}
	if(vmm_map_page(phys_addr, virt_addr))	{
		PANIC("Unable to map address");
	}

	create_address_space(virt_addr, phys_addr);

	new_proc->dirtable = phys_addr;
	
	vmm_map_address_space(virt_addr, (uint32_t*)0xFFFFF000);

	uint32_t pid = new_proc->pid;

	new_proc->next = proc_current->next;
	proc_current->next = new_proc;


	enable_int();
	return pid;
}



volatile uint32_t switch_task(Registers* regs)	{
	if(proc_current == NULL || proc_current == proc_current->next)
		return (uint32_t)regs;

	pcb* old = proc_current;

	memcpy(old->regs, regs, sizeof(*old->regs));
	old->state = PROC_READY;


	proc_current = proc_current->next;

	change_tss(proc_current);
	
	vmm_switch_pdir(proc_current->dirtable);
	
	proc_current->state = PROC_RUNNING;
	
	return (uint32_t)((proc_current->kstack + KSTACKSZ) - sizeof(*proc_current->regs) - 8);
}


