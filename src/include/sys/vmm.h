/**
* \ingroup paging
* \file vmm.h
* Header file for the virtual memory manager (paging).
*/

#ifndef __VMM_H
#define __VMM_H


typedef struct	{
	/**
	* Format of each integer (PTE) is (num. bits):
	* - Present (1)
	* - Read/write (1)
	* - User mode (1)
	*  - 0 - CPL 3 has access
	*  - 1 - CPL < 3 has access
	* - Reserved (2)
	* - Accessed (1)
	* - Dirty (1)
	* - Reserved (2)
	* - Available (3)
	* - Physical address (20)
	* One entry holds information about a 4 KB memory block.
	*/
	uint32_t pages[1024];
} __attribute__((packed)) vmm_ptable;



typedef struct	{
	/**
	* Format of the integer (PDE) is (num. bits):
	* - Present (1)
	* - Read/write (1)
	* - User mode (1)
	* - Reserved (2)
	* - Accessed (1)
	* - Dirty (1)
	* - Reserved (2)
	* - Available (3)
	* - Physical address (20)
	* One integer holds information about where to find one vmm_ptable.
	*/
	uint32_t tables[1024];
} __attribute__((packed)) vmm_dirtable;


/**
* Create the page tables and enable paging.
* Method:
* - Allocate 1 physical block for kernel page directory (identity mapped)
* - Allocate 1 physical block for first page table (identity mapped)
* - Idenity map the first 4 MB of data
* - The past page directory entry (1023) points back to the page directory
*  - This means that the CPU will interpret the page directory as a page table
*  in the last 4 MB of virtual memory. That way we can have access to our page
*  directory when paging is enabled.
*  - See
*  http://www.rohitab.com/discuss/topic/31139-tutorial-paging-memory-mapping-with-a-recursive-page-directory/
*/
void vmm_initialize();


/**
* Map a physical address with a virtual address.
* \param[in] phys_addr The physical address that should point to an available
* block.
* \param[in] virt_addr A virtual address.
* \return Returns 0 if we are successful or non-zero if we are not.
* \todo Define error codes.
*/
int vmm_map_page(uint32_t phys_addr, uint32_t virt_addr);


/**
* Unmap a virtual address from its physical page.
* \param[in] virt_addr The virtual address that should be unmapped.
*/
void vmm_unmap_page(uint32_t virt_addr);



/**
* Create an empty address space that has the kernel mapped in.
* \param[in,out] virt_addr A virtual memory address that is mapped to a physical
* address, must be 4 KB in size.
* \param[in] phys_addr Physical address to virt_addr. This is needed to map the 
* \return Returns the same as virt_addr.
*/
uint32_t* create_address_space(uint32_t* virt_addr, uint32_t* phys_addr);


/**
* Create a mapping between two address spaces (clone of address space). The
* purpose of this function is to support fork()-like functionality. This
* function only copies the addresses in the page directory and marks each PDE
* that was writeable as readable and cloned (VMM_PAGE_CLONED). The actual
* copying is handled by handle_page_fault.
* \param[in,out] pdir_to The new page directory.
* \param[in] pdir_from The old page directiry.
*/
void vmm_map_address_space(uint32_t* pdir_to, uint32_t* pdir_from);


void vmm_switch_pdir(vmm_dirtable* pdir);



#endif
