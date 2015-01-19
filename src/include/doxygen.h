/**
* \defgroup Architecture Architecture
* Function, structures and definition specific for each architecture. Currently,
* only x86 is created, but everything architecture specific should be seperated
* out into this module.
*
* \defgroup boot Boot up process
* Boot up is handled by Grub and information is received with multiboot 1.
* The only information we don't receive from Grub and we need is where the
* kernel starts and end, this is defined in linker-*.ld and pushed on the stack
* in boot.s.
*
* boot.s also defines the stack address in the .bss segment, which also is
* pushed on the stack to kmain. The stack has four values when kmain is called
* (order pushed):
* - stack address
* - End of kernel (in memory)
* - Start of kernel (in memory)
* - Pointer to multiboot_info structure.
*
* This is not a higher half kernel, so no GDT or paging is used.
* \ingroup Architecture

*
* \defgroup GDT Global Descriptor Table
* How we define various segments and how we tell the CPU to use those segments.
* From the kernels point of view, gdt_install should just be called once and
* that is it, everything is set up.
* \ingroup Architecture
*
* \defgroup IDT Interrupt Descriptor Table
* Interrupt Descriptor Table
* \ingroup Architecture
*
* \defgroup ISR Interrupt Service Routine
* Interrupt Service Routine
* \ingroup Architecture
*
* \defgroup HAL HAL API
* API to interact with the hardware. This is an attempt to create one unifying
* interface to interact with hardware, even if more architectures where
* supported, these functions should be supported.
* \ingroup Architecture


* \defgroup kernel Kernel
* Everything that deals with the kernel. This excludes drivers that interact
* with peripheral devices, architecture specific stuff and libraries.
*
* \defgroup multiboot Multiboot
* Information about the multiboot information we get from Grub. We use multiboot
* specification 1 (value 0xBAADB002).
* \ingroup kernel
*
* \defgroup memory Memory
* How memory is handled in the system. This is divided up in three parts,
* physical memory, virtual memory (paging) and heap. The heap is the final
* abstraction used by other parts of the system.
* \ingroup kernel
*
* \defgroup pmm Physical memory manager
* How the kernel allocate and deallocate physical memory. init_pmm creates a
* bitmap somewhere in memory, describing the allocation status of all available
* memory. Each bit represents a 4 KB page, which was chosen because it was made
* to support a paging architecture. init_pmm divides up the memory according the
* information we get from Grub. Grub does not contain information about where
* the kernel starts and end, so pmm_mark_mem_taken must be used here.
* \ingroup memory
*
* \defgroup paging Paging
* Paging is used as the virtual memory manager (vmm). The first 4 MB block is
* identity mapped. The last entry (1023) in each page directory points to
* itself, so that we have a way to access the page directory and page tables
* after paging has been enabled.
* \todo
* - Ensure that all necessary pages are identity mapped, all those marked as
* taken by the pmm.
* - Clean up interface, code and variables
* - Decide on a virtual memory structure
*  - Where is heap located?
*  - Where is the stack?
*  - Which part of memory is mapped in at every process?
* \ingroup memory
*
* \defgroup heap Heap
* How the kernel heap works (NOT heap in user mode)
* \ingroup memory
*
* \defgroup processes Processes
* How processes are organized and structured
* \ingroup kernel
*
* \defgroup syscalls System calls
* What system calls exist and how they are implemented
* \ingroup kernel
*

* \defgroup c_library C Library
* The C runtime environment for our kernel.
*
* \defgroup c_string string.h
* Attempts to imitate the string.h / cstring library.
* \ingroup c_library
*
* \defgroup c_ctype ctype.h
* Attempts to imitate the ctype.h / cctype library
* \ingroup c_library
*
* \defgroup c_stdio stdio.h
* Attempts to imitate the stdio.h / cstdio library
* \ingroup c_library
*
* \defgroup c_stdlib stdlib.h
* Attempts to imitate the stdlib.h / cstdlib library
* \ingroup c_library


* \defgroup sys_drivers Kernel Drivers
* Different drivers in the system. This is all the functionality that interacts
* with peripheral devices.
*
* \defgroup VGA_drv VGA
* Simple driver for writing text to screen.
* \ingroup sys_drivers
*
* \defgroup KBD_drv Keyboard Driver
* Keyboard driver to handle input from the user.
* \ingroup sys_drivers
*
*
* \defgroup MP Multi-processor
* The OS can take adavantage of multiple CPUs to execute tasks simoultaneously.
* Below are the steps taken to initialize it:
* 1. APIC is initialized. If APIC is not present, we don't boot (not necessary,
* but makes it simpler).
* 2. APIC tables are parsed to find information about each CPU
* 3. Local APIC is initialized.
* 4. ???
*
* Below is what is unique to each processor:
* - Local APIC - Addresses are system-wide, but each processor is contacting
* it's own APIC.
* - Register cr0 and cr3 - This is a control register, so it can be used to create
* different type of processors.
* - Register eflags
* - TLB - When you flush a TLB, that is that CPUs TLB. Need a system for
* flushing all TLBs.
* - APIC Timer - The timer is unique, there are various ways to do scheduling.
* - GDT
* - TSS
*
* Important considerations:
* - Disable interrupts only works for the processor it is executing on.
* - When starting APs, paging and interrupts are OFF.
*
*
* \defgroup apic APIC
* APIC consist of local APIC (LAPIC) and I/O APIC. In our system, APIC is a
* requirement, regardless of whether it is a multiprocessor or uniprocessor
* system.
*
*
* \defgroup locking Locking
* The locking mechanism ensures that resources are accesed in a safe manner.
* The method used is a simple spinlock with busy waiting. The reason for this is
* that it's simple and resources should not be hold for long periods of time, so
* it should be about as efficient as doing task switching instead.
*
* Interrupts are disabled before acquiring a lock, so no deadlock can occurr by
* having the same processor waiting to use each other resources. If doing nested
* locking, some problems might occurr.
*
* Have P1 get a lock on R1 and have P2 get a lock on R2. If P1 now wants R2 AND
* P2 wants R1, we have a deadlock. This is solved by being very strict in the
* way we can ask for resources. All the resources the kernel uses are ordered
* and the processor can only ask for a resource that has a lower number than a
* lock it currently has.
*
* If it tries to ask for a higher or equal number resource, the system should
* PANIC. Unsure how feasible this is to achieve.
*
* lock_resource determines the order of the resources.
* \ingroup MP
*/
