/**
* \ingroup paging
* \file vmm.c
* Implementation file for the virtual memory manager (paging).
* \todo Clean up the interface and remove unecessary code.
* \todo The structs are not really used, clean up!
*/

#include "sys/kernel.h"
#include "sys/vmm.h"
#include "sys/pmm.h"

#include "lib/string.h"
#include "lib/stdio.h"

#include "hal/hal.h"
#include "hal/isr.h"


//-------------- Definitions -----------------------------------
#define X86_PAGES_PER_TABLE 1024
#define X86_PAGES_PER_TABLE 1024

#define X86_PAGE_SIZE 4096

#define X86_PAGE_PRESENT    0x01
#define X86_PAGE_WRITABLE   0x02
#define X86_PAGE_USER       0x04
#define X86_PAGE_RESERVED   0x08|0x10|0x80|0x100
#define X86_PAGE_ACCESSED   0x20
#define X86_PAGE_DIRTY      0x40
#define X86_PAGE_FRAME      0xFFFFF000

#define X86_PAGEDIR_PRESENT         0x01
#define X86_PAGEDIR_WRITABLE        0x02
#define X86_PAGEDIR_USER            0x04
#define X86_PAGEDIR_WRITE_THROUGH   0x08
#define X86_PAGEDIR_CACHE           0x10
#define X86_PAGEDIR_ACCESSED        0x20
#define X86_PAGEDIR_RESERVED        0x40
#define X86_PAGEDIR_4MB             0x80
#define X86_PAGEDIR_CPU_GLOB        0x100
//#define X86_PAGEDIR_LV4_GLOB      0x200
#define X86_PAGEDIR_FRAME           0xFFFFF000

#define VMM_PAGE_CLONED 0x800


/**
* Page directory offset after paging has been enabled.
* 0xFFC00000 = (2^10 * 2^10 * 4) * 1023
* Virtual address of a page table is:
* - PAGEDIR_OFFSET + (ptable_index * 4096)
*/
#define PAGEDIR_OFFSET 0xFFC00000
//#define PAGEDIR_OFFSET 0xFFFFF000

#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

#define VMM_TMP_ADDRESS 0x00FF0000

vmm_dirtable* kernel_page_dir = NULL;
vmm_dirtable* current_page_dir = NULL;
vmm_dirtable* kernel_page_dir_virt = (vmm_dirtable*)0xFFFFF000;

uint8_t* pmm_references;
extern uint32_t pmm_bitmap_sz;


uint32_t handle_page_fault(Registers* regs);

void vmm_addr_to_index(uint32_t addr, uint32_t* dir_index, uint32_t* page_index);



//--------------------- Internal functions ---------------------------


void vmm_addr_to_index(uint32_t addr, uint32_t* dir_index, uint32_t* page_index)	{
	addr &= ~(0xFFF);
	*dir_index = addr / 0x400000;
	*page_index = (addr % 0x400000) / 0x1000;
}


/**
* \todo
* - The copy-on-write has not been tested at all, is most likely horribly wrong.
*/
uint32_t handle_page_fault(Registers* regs)	{
	uint32_t fault_addr;
	asm("mov %%cr2, %0" : "=r" (fault_addr));

	int pres = regs->err_code & 0x01;
	int rw = regs->err_code & 0x02;
	int us = regs->err_code & 0x04;
	int res = regs->err_code & 0x08;
	int id = regs->err_code & 0x10;

	// 0x0010FFEF
	kprintf(K_LOW_INFO, "Page fault (");
	if(regs->err_code & 0x01)
		kprintf(K_LOW_INFO, "page-level ");
	if(regs->err_code & 0x02)
		kprintf(K_LOW_INFO, "write ");
	if(regs->err_code & 0x04)
		kprintf(K_LOW_INFO, "user-mode ");
	if(regs->err_code & 0x08)
		kprintf(K_LOW_INFO, "reserved ");
	if(regs->err_code & 0x10)
		kprintf(K_LOW_INFO, "int-fetch ");
	kprintf(K_LOW_INFO, ") @0x%08X\n", fault_addr);
	print_regs(regs, K_LOW_INFO);

	/*
	* Need to test this.
	// If we tries to write to a read-only file
	if(rw)	{
		uint32_t dind, pind;
		vmm_addr_to_index(fault_addr, &dind, &pind);
		uint32_t fault_phys = 0;
		
		if(pres && kernel_page_dir_virt->tables[dind] & VMM_PAGE_CLONED)	{
			kprintf(K_LOW_INFO, "CLONED\n");

			// Make directory index writeable
			kernel_page_dir_virt->tables[dind] &= ~(VMM_PAGE_CLONED);
			kernel_page_dir_virt->tables[dind] |= X86_PAGEDIR_WRITABLE;

			
			// Should only copy over if this is not the last reference to it,
			// should not decrement here, because we must also change the page
			// table below.
			// TODO: I think it always will be more than 1
			if(pmm_references[fault_phys/4096] > 1)	{
				// Create and index new page table
				vmm_ptable* ptable = (vmm_ptable*)pmm_alloc_first();
				vmm_ptable* ptf = kernel_page_dir_virt->tables[dind];
	
				fault_phys = ptf->pages[pind] & X86_PAGE_FRAME;
				
				// Map and copy over table and then unmap
				vmm_map_page((uint32_t)ptable, (uint32_t)VMM_TMP_ADDRESS);
				memcpy((uint8_t*)VMM_TMP_ADDRESS, ptf, sizeof(vmm_ptable));
				vmm_unmap_page((uint32_t)VMM_TMP_ADDRESS);
			
				// Add this page table to the directory
				kernel_page_dir_virt->tables[dind] |= (uint32_t)ptable;
			}
		}
		
		vmm_ptable* ptf = kernel_page_dir_virt->tables[dind];

		// If page has been cloned, this should work both alone and in conjunction
		// with page faulting on the page table
		if(ptf->pages[pind] & VMM_PAGE_CLONED)	{
			ptf->pages[pind] &= ~(VMM_PAGE_CLONED);
			ptf->pages[pind] |= X86_PAGE_WRITABLE;

			// If we calculated physical fault-address above, we also changed the
			// page table, so we should not calculate it again, as it will give us
			// the wrong address.
			if(fault_phys == 0)
				fault_phys = ptf->pages[pind] & X86_PAGE_FRAME;

			if(pmm_references[fault_phys/4096] > 1)	{

				pmm_references[fault_phys/4096]--;

				// Copy over block
				uint8_t* block = (uint8_t*)pmm_alloc_first();
				uint8_t* vblock = VMM_TMP_ADDRESS;
				vmm_map_page((uint32_t)block, (uint32_t)VMM_TMP_ADDRESS);
				memcpy(vblock, fault_addr & X86_PAGE_FRAME, 4096);
				ptf->pages[pind] |= (uint32_t)block;
			}

			// We have handled it and should not page fault
			return (uint32_t)regs;
		}
	}
	*/

	PANIC("PAGE FAULT");
	return (uint32_t)regs;
}



//----------------------- Public API functions -------------------------------

int vmm_map_page(uint32_t phys_addr, uint32_t virt_addr)	{
	uint32_t dir_index, page_index;
	vmm_addr_to_index(virt_addr, &dir_index, &page_index);

	if(kernel_page_dir_virt->tables[dir_index] & 1)	{
		vmm_ptable* ptable = (vmm_ptable*)(0xFFC00000 + (dir_index * 4096));
		if(!(ptable->pages[page_index] & 1))	{
			ptable->pages[page_index] = phys_addr | 3;
		}
		else	{
			kprintf(K_BOCHS_OUT, "ptable = %x\n", ptable->pages[page_index]);
			return 1;
		}
	}
	else	{
		uint32_t* n_ptable = (uint32_t*)pmm_alloc_first();
		uint32_t* ptable_virt = (uint32_t*)(0xFFC00000 + (dir_index * 4096));
		
		kernel_page_dir_virt->tables[dir_index] = (uint32_t)n_ptable | 3;
		// Must remember to zero out the entire entry
		memset(ptable_virt, 0x00, 4096);
		ptable_virt[page_index] = phys_addr | 3;
	}
	return 0;

}


void vmm_unmap_page(uint32_t virt_addr)	{
	uint32_t dir_index, page_index;
	vmm_addr_to_index(virt_addr, &dir_index, &page_index);
	
	if(kernel_page_dir_virt->tables[dir_index] & 1)	{
		uint32_t* ptable_virt = (uint32_t*)(0xFFC00000 + (dir_index * 4096));
		if(ptable_virt[page_index] & 1)	{
			// Unmap page
			ptable_virt[page_index] = 0;
		}
		// Check if we should unmap the whole table
		int i;
		for(i = 0; i < 1024; i++)	{
			if(ptable_virt[i] & 1)	break;
		}
		if(i >= 1024)	{
			pmm_free((void*)(kernel_page_dir_virt->tables[dir_index] & 0xFFFFF000));
			kernel_page_dir_virt->tables[dir_index] = 0;
		}

	}
	flush_tlb_entry(virt_addr);
}


uint32_t* create_address_space(uint32_t* virt_addr, uint32_t* phys_addr)	{
	
	// Make sure everything is 0
	memset(virt_addr, 0x00, 4096);

	// The first 4 MB must be mapped in
	uint32_t i;
	for(i = 0; i < 256; i++)
		virt_addr[i] = kernel_page_dir_virt->tables[i];
	
	virt_addr[1023] = (uint32_t)phys_addr | X86_PAGEDIR_PRESENT | X86_PAGEDIR_WRITABLE;

	return (uint32_t*)virt_addr;
}


/**
* \todo
* - This should fully enable copy-on-write
* - Need to add another reference
*/
void vmm_map_address_space(uint32_t* pdir_to, uint32_t* pdir_from)	{
	uint32_t i, j;
	for(i = 0; i < 1023; i++)	{
		pdir_to[i] = kernel_page_dir_virt->tables[i];
		if(kernel_page_dir_virt->tables[i] & X86_PAGEDIR_WRITABLE)	{
			vmm_ptable* pt = (vmm_ptable*)(0xFFC00000 + (i * 4096));
			for(j = 0; j < 1024; j++)	{
				if(pt->pages[j] & X86_PAGE_WRITABLE)	{
					pt->pages[j] &= ~(X86_PAGE_WRITABLE);
					pt->pages[j] |= VMM_PAGE_CLONED;

					// Get physical address and increment the reference count
					uint32_t phys_frame = pt->pages[j] &= X86_PAGE_FRAME;
					pmm_references[phys_frame/4096]++;
				}
			}
			pdir_to[i] &= ~(X86_PAGEDIR_WRITABLE);
			kernel_page_dir_virt->tables[i] &= ~(X86_PAGEDIR_WRITABLE);
			
			pdir_to[i] |= VMM_PAGE_CLONED;
			kernel_page_dir_virt->tables[i] |= VMM_PAGE_CLONED;

			

			// TODO: I think we can handle the rest after the page fault happens,
			// we then have:
			// - Do a full copy of the entire 4 MB frame, or
			// - Mark all the individual pages that contain data as cloned
			//   - Except page that caused page fault
		}

	}
}

void vmm_switch_pdir(vmm_dirtable* pdir)	{
	current_page_dir = pdir;
	load_page_dir_addr( (uint32_t)pdir );
}

void vmm_initialize()	{
	// Need 64 blocks per 1GB of physical memory
	uint32_t blocks = pmm_bitmap_sz / 4096;
	if(pmm_bitmap_sz % 4096 != 0)	blocks++;
	// pmm_references

	pmm_references = (uint8_t*)pmm_alloc_first_n_blocks(blocks);
	if(pmm_references == NULL)
		PANIC("No more physical blocks\n");

	memset(pmm_references, 0x00, pmm_bitmap_sz);
	kprintf(K_BOCHS_OUT, "Blocks: %i | Bytes: %i\n", blocks, pmm_bitmap_sz);


	// Allocate 4 KB for the page directory
	kernel_page_dir = (vmm_dirtable*)pmm_alloc_first();
	memset(kernel_page_dir, 0x00, sizeof(vmm_dirtable));

	// Allocate 4 KB for the first page table
	vmm_ptable* ptable = (vmm_ptable*)pmm_alloc_first();
	memset(ptable, 0x00, sizeof(vmm_ptable));
	
	uint32_t i = 0;
	for(i = 0; i < 1024; i++)	{
		ptable->pages[i] = (uint32_t)(i*4096);
		if(pmm_is_taken(i))	{
			pmm_references[i] += 1;
			ptable->pages[i] |= X86_PAGE_PRESENT | X86_PAGE_WRITABLE;
		}
		else
			ptable->pages[i] |= X86_PAGE_WRITABLE;
	}


	// Map in the first 4 MB
	kernel_page_dir->tables[0] = (uint32_t)ptable | X86_PAGEDIR_PRESENT | X86_PAGEDIR_WRITABLE;
	
	// Map the last index onto itself
	kernel_page_dir->tables[1023] = (uint32_t)kernel_page_dir | X86_PAGEDIR_PRESENT | X86_PAGEDIR_WRITABLE;

	// Register page fault handler
	register_interrupt_handler(14, handle_page_fault);
	
	current_page_dir = kernel_page_dir;

	// Enable paging
	paging_enable(kernel_page_dir->tables);
}




#ifdef TEST_KERNEL

bool vmm_run_all_tests()	{

	return true;
}

#endif	// TEST_KERNEL


