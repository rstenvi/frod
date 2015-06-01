/**
* \ingroup paging
* \file vmm.h
* Header file for the virtual memory manager (paging).
*/

#ifndef __VMM_H
#define __VMM_H


#define VMM_ERR_PAGE_IN_USE      -1
#define VMM_ERR_NO_PAGEDIR_ENTRY -2


#define VMM_SUCCESS 0




#define X86_PAGE_PRESENT    0x01
#define X86_PAGE_WRITABLE   0x02
#define X86_PAGE_USER       0x04
#define X86_PAGE_RESERVED   0x08|0x80|0x100
#define X86_PAGE_CACHE_DIS  0x10
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

// Our own bit to check if this page was cloned
#define X86_PAGE_CLONED 0x800

#define X86_PF_PROTECT  (1 << 0)
#define X86_PF_WRITE    (1 << 1)
#define X86_PF_USERMODE (1 << 2)
#define X86_PF_RSVD     (1 << 3)
#define X86_PF_ID       (1 << 4)


//typedef struct	{
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
//	uint32_t pages[1024];
//} __attribute__((packed)) vmm_ptable;



//typedef struct	{
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
//	uint32_t tables[1024];
//} __attribute__((packed)) vmm_dirtable;


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
void vmm_init();


/**
* Map a physical address with a virtual address.
* \param[in] phys_addr The physical address that should point to an available
* block.
* \param[in] virt_addr A virtual address.
* \return Returns 0 if we are successful or non-zero if we are not.
* \todo Define error codes.
*/
int vmm_map_page(uint32_t phys_addr, uint32_t virt_addr, uint32_t acl);


/**
* Unmap a virtual address from its physical page.
* \param[in] virt_addr The virtual address that should be unmapped.
*/
int vmm_unmap_page(uint32_t vaddr);



/**
* Create an empty address space that has the kernel mapped in.
* \param[in,out] virt_addr A virtual memory address that is mapped to a physical
* address, must be 4 KB in size.
* \return Returns the physical address space
*/
uint32_t* create_address_space(uint32_t* virt_addr);


/**
* Create a mapping between two address spaces (clone of address space). The
* purpose of this function is to support fork()-like functionality. This
* function only copies the addresses in the page directory and marks each PDE
* that was writeable as readable and cloned (VMM_PAGE_CLONED). The actual
* copying is handled by handle_page_fault.
* \param[in,out] pdir_to The new page directory.
* \param[in] pdir_from The old page directiry.
*/
//void vmm_map_address_space(uint32_t* pdir_to, uint32_t* pdir_from);


void vmm_switch_pdir(uint32_t* pdir);



#endif
