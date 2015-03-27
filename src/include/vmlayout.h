/**
* \file vmlayout.h
* Determines the addresses and sizes of the various parts of kernel memory.
* This is all placed in the same place to ensure that they don't overlap.
*/

#ifndef __VMLAYOUT_H
#define __VMLAYOUT_H

#include "sizes.h"
#include "config.h"

#define BIOS_DATA_ADDR  KB1
#define MAIN_BIOS_START (KB1*0x380)
#define MAIN_BIOS_END   (MB1-1)

// Kernel is at 1MB

// The stack, 2 4KB blocks for each CPU core
#define KERNEL_STACK_TOP MB4
#define KERNEL_STACK_SZ  (KB4*2)

#define LAPIC_PHYS_VIRT_ADDR (KERNEL_STACK_TOP - (KERNEL_STACK_SZ*MAX_CPUS) - KB4)
//#define MAX_KERNEL_MEM (KERNEL_STACK_TOP - (KERNEL_STACK_SZ*MAX_CPUS))
#define MAX_KERNEL_MEM (LAPIC_PHYS_VIRT_ADDR-KB4)


#define PROC_VMM_START MB256
#define PROC_VMM_SIZE  MB256
#define PROC_VMM_END   (PROC_VMM_START + PROC_VMM_SIZE)

// The kernel heap
#define HEAP_START PROC_VMM_END
#define HEAP_SIZE MB256
#define HEAP_END (HEAP_START + HEAP_SIZE)


// Must be changed when adding new sections to always represent end of kernel memory
#define KERNEL_MAX_VM HEAP_END

// First GB is reserved for kernel, then user space
#define USERMODE_START GB1


// Sanity check that we are not using too much VM
#if USERMODE_START < KERNEL_MAX_VM
	#define VM_VALID false
#else
	#define VM_VALID true
#endif


#endif
