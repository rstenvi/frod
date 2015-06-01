



#include "sys/kernel.h"
#include "sys/vmm.h"
#include "sys/pmm.h"

#include "hal/hal.h"


/**
* Address of the kernel directory (physical).
*/
uint32_t* kernel_dir = NULL;

/**
* Address of the directory to the current address space (physica).
*/
uint32_t* current_dir = NULL;

/**
* Virtual address to the directory which is currently in use.
*/
uint32_t* dir_virtual = (uint32_t*)0xFFFFF000;


uint32_t vmm_handle_page_fault(Registers* regs);


uint32_t vmm_return_kernel_dir()	{
	return (uint32_t)kernel_dir;
}




static inline uint32_t* vmm_get_physical_page()	{
	uint32_t* ret = (uint32_t*)pmm_alloc_first();
	memset(ret, 0x00, KB4);
	return ret;
}

#define ADDR2INDEX(addr,diri,pagei)\
	addr &= ~(0xFFF);\
	diri = addr / MB4;\
	pagei = (addr % MB4) / KB4




void vmm_init()	{

	// Allocate space for directory and 1 page (1st 4MB)
	kernel_dir = vmm_get_physical_page();
	uint32_t* ptable = vmm_get_physical_page();


	// Identity map 1st 4MB
	int i;
	for(i = 0; i < 1024; i++)	{
		ptable[i] = (i*KB4);
		ptable[i] |= 
			X86_PAGE_PRESENT | X86_PAGE_WRITABLE | X86_PAGE_USER;
	}

	// Map in the LAPIC address space
	ptable[LAPIC_PHYS_VIRT_ADDR/KB4] = 0xFEE00000 |
		X86_PAGE_PRESENT | X86_PAGE_WRITABLE | X86_PAGE_CACHE_DIS |
		X86_PAGE_USER;
	lapic_new_address(LAPIC_PHYS_VIRT_ADDR);

	// Mark first 4 physical MB as taken
	pmm_mark_mem_taken(0, MB4);

	// TODO: Can unmark LAPIC addr, as that is not identity mapped


	// Map in first 4MB
	kernel_dir[0] = (uint32_t)ptable |
		X86_PAGEDIR_PRESENT | X86_PAGEDIR_WRITABLE | X86_PAGE_USER;

	// Map intex on itself
	// TODO: Could also have this as second 4MB block, makes more sense when I'm
	// in the lower half
	kernel_dir[1023] = (uint32_t)kernel_dir |
		X86_PAGEDIR_PRESENT | X86_PAGEDIR_WRITABLE | X86_PAGE_USER;

	register_interrupt_handler(14, vmm_handle_page_fault);

	current_dir = kernel_dir;

	paging_enable(current_dir);
}




int vmm_map_page(uint32_t phys_addr, uint32_t virt_addr, uint32_t acl)	{

	// Get indexes and frame of virtual address
	uint32_t diri, pagei;
	ADDR2INDEX(virt_addr, diri, pagei);


	// If directory entry is present
	if(dir_virtual[diri] & X86_PAGEDIR_PRESENT)	{
		uint32_t* ptable = (uint32_t*)(0xFFC00000 + (diri*KB4));
		if( (ptable[pagei] & X86_PAGE_PRESENT))
			return VMM_ERR_PAGE_IN_USE;
		else
			ptable[pagei] = phys_addr | X86_PAGE_PRESENT | acl;
	}
	else	{
		// Directory entry is NOT present

		// Get page and map it in
		uint32_t* new_ptable = pmm_alloc_first();
		dir_virtual[diri] = (uint32_t)new_ptable | X86_PAGEDIR_PRESENT | acl;

		// Get virtual address and zero it
		uint32_t* ptable = (uint32_t*)(0xFFC00000 + (diri*KB4));
		memset(ptable, 0x00, KB4);

		// Map the page in
		ptable[pagei] = phys_addr | X86_PAGE_PRESENT | acl;
	}
	return VMM_SUCCESS;
}


int vmm_unmap_page(uint32_t vaddr)	{
	// Get indexes and frame of virtual address
	uint32_t diri, pagei;
	ADDR2INDEX(vaddr, diri, pagei);

	if(dir_virtual[diri] & X86_PAGEDIR_PRESENT)	{
		uint32_t* ptable = (uint32_t*)(0xFFC00000 + (diri*KB4));
		if(ptable[pagei] & X86_PAGE_PRESENT)	{
			ptable[pagei] = 0;
		}

		// Check if entire directory entry is empty
		int i;
		for(i = 0; i < 1024; i++)	{
			if(ptable[i] & X86_PAGE_PRESENT)	break;
		}
		if(i >= 1024)	{
			pmm_free( (void*)(dir_virtual[diri] & 0xFFFFF000));
			dir_virtual[diri] = 0;
		}
	}
	else	{
		return VMM_ERR_NO_PAGEDIR_ENTRY;
	}
	flush_tlb_entry(vaddr);

	return VMM_SUCCESS;
}



uint32_t* vmm_create_address_space(uint32_t* virt)	{
//	uint32_t* addr_space = vmm_get_physical_page();
	uint32_t* addr_space = (uint32_t*)pmm_alloc_first;
	vmm_map_page((uint32_t)addr_space, (uint32_t)virt, X86_PAGE_WRITABLE);
	memset(virt, 0x00, KB4);


	// TODO: Only copy relevant pages, saves some time
	int i;
	for(i = 0; i < 1024; i++)	{
		virt[i] = dir_virtual[i];
	}

	// Map in itself
	virt[1023] = (uint32_t)addr_space |
		X86_PAGEDIR_PRESENT | X86_PAGEDIR_WRITABLE;
	return addr_space;
}



void vmm_switch_pdir(uint32_t* pdir)	{
	current_dir = pdir;
	load_page_dir_addr( (uint32_t)current_dir);
}

void vmm_switch_kernel()	{
	current_dir = kernel_dir;
	load_page_dir_addr( (uint32_t)current_dir);
}




uint32_t vmm_handle_page_fault(Registers* regs)	{
	print_regs(regs, K_BOCHS_OUT);
	PANIC("Page fault");

	// Get the address that caused the exception
	uint32_t addr;
	read_cr2(addr);

	// Check if address belongs to the process address space
	if((regs->err_code & X86_PF_PROTECT) == 0)	{
		// Fault was caused by a non-present page
		PANIC("Page NOT present");
	}
	else	{
		uint32_t diri, pagei;
		ADDR2INDEX(addr, diri, pagei);
		uint32_t* dir = 0x00;
		if ((regs->err_code & X86_PF_WRITE))	{
			// If it was a write to cloned directory entry
			if(dir[diri] & X86_PAGE_CLONED)	{

			}
			uint32_t* ptable = dir[diri];
			if(ptable[pagei] & X86_PAGE_CLONED)	{

			}
		}
		if((regs->err_code & X86_PF_USERMODE))	{
			// Kill the process
		}
		else	{
			PANIC("Page protection fault in kernel");
		}
	}

	return 0;
}
