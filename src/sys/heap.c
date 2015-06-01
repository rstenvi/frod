/**
* \ingroup heap
* \file heap.c
* Implementation of the kernel heap, description of algorithm in heap.h.
*/

/**
* \addtogroup heap
* @{
*/

#include "sys/kernel.h"
#include "sys/heap.h"

#include "sys/vmm.h"
#include "sys/pmm.h"

#include "lib/stdio.h"


Heap kheap;


/**
* Allocate a new virtual block for use by the heap.
* \param[in,out] prev The block the new element should be placed after. If the
* value is non-NULL, the new element will be placed after this element, if it is
* NULL, the new elements prev and next pointer will point to itself.
* \return The new element is returned. This is alwys valid.
* \remark PANIC is raised if there are no more free pages.
*/
LLMalloc* heap_get_new_block(LLMalloc* prev);





//---------------- Public API implementation ------------------------

void heap_init()	{
	init_spinlock(&kheap.lock, LOCK_HEAP);
	spinlock_acquire(&kheap.lock);

	kheap.blocks_allocked = 0;
	kheap.kern_heap = heap_get_new_block(NULL);
	
	
	spinlock_release(&kheap.lock);
}

void* heap_malloc(uint32_t sz)	{
	// We always align on 4 B boundaries
	sz += (sz%4);

	// Check in the beginning, that it can be possible
	if(sz > (4096 - sizeof(LLMalloc)) )	{
		PANIC("We don't have memory pages that big\n");
	}

	LLMalloc* it = kheap.kern_heap;
	LLMalloc* end = kheap.kern_heap;

	void* ret = NULL;

	do	{
		if(it->used == 0 && it->size >= sz)	{
			break;
		}
		it = it->next;
	} while(it != end);

	// Have to check again
	if(it->used == 0 && it->size >= sz)	{
		it->used = 1;

		// Check if we need to split it up
		if( (it->size - sz) > sizeof(LLMalloc) )	{
			LLMalloc* n = (LLMalloc*)((uint32_t)it + sz + + sizeof(LLMalloc));
			n->used = 0;
			// New size = old size - struct size - taken size
			n->size = it->size - sizeof(LLMalloc) - sz;

			// Insert the new element
			n->next = it->next;
			n->prev = it;
			it->next = n;

			// New size of allocated block
			it->size = sz;
		}
		// Pointer to the allocated memory
		ret = (void*)((uint32_t)it + sizeof(LLMalloc));
	}
	else	{
		// Put it at second place in the list, so that it will be found quickly
		// when we call the function again
		(void)heap_get_new_block(it);
		return heap_malloc(sz);
	}
	return ret;
}

/**
* \todo Must verify that the contigous memory check makes sense.
*/
void heap_free(void* addr)	{
	LLMalloc* free = (LLMalloc*)( (uint32_t)addr - sizeof(LLMalloc));
	
	// Check magic numbers
	if( free->used != 0x01 || free->magic1 != 0xabcd || free->magic2 != 0xef)	{
		PANIC("Heap magic value does not fit.\n");
	}

	// If previous is different and consist of free space, we should connect
	// previous and this.
	if(free->prev != free && free->prev->used == 0 &&
		((uint32_t)free->prev + free->prev->size) == (uint32_t)free )	{
		free = free->prev;
		free->size += free->next->size + sizeof(LLMalloc);
		// Special case if there are only two elements
		if(free->prev == free->next)	{
			free->next = free->prev = free;
		}
		// All other cases
		else	{
			free->next = free->next->next;
		}
	}

	// Do the same thing as above, but for the next node, both can happen.
	// Because this happens every time, there should only be 1 free block in a
	// row. Therefore we only need to do this once on each free
	if(free->next != free && free->next->used == 0)	{
		free->size += free->next->size + sizeof(LLMalloc);
		if(free->next == free->prev)	{
			free->next = free->prev = free;
		}
		else	{
			free->next = free->next->next;
		}
	}

	addr = NULL;
}




//----------------- Internal function implementations -----------------

LLMalloc* heap_get_new_block(LLMalloc* prev)	{
	uint32_t i = 0, heap_start = HEAP_START+(kheap.blocks_allocked*4096), phys;
	// We allocate 4 MB each time
	for(i = 0; i < HEAP_BLOCKS; i++)	{
		phys = (uint32_t)pmm_alloc_first();

		if(vmm_map_page(phys, (HEAP_START+(kheap.blocks_allocked*4096)),
			X86_PAGE_WRITABLE))	{
			printf("i = %i, phys = %p\n", i, phys);
			PANIC("Unable to map page");
		}
		kheap.blocks_allocked++;
	}

	LLMalloc* ret = (LLMalloc*)heap_start;
	if(prev != NULL)	{
		ret->next = prev->next;
		ret->prev = prev;
		prev->next = ret;
	}
	else	{
		ret->next = ret;
		ret->prev = ret;
	}

	// Block is 
	ret->size = (4096*HEAP_BLOCKS) - sizeof(LLMalloc);
	ret->used = 0;

	ret->magic1 = 0xabcd;
	ret->magic2 = 0xef;
	return ret;
}




//----------- Testing code --------------------

#ifdef TEST_KERNEL

#define check_header_addr(got,exp) \
if((uint32_t)got != (uint32_t)exp)	{\
	kprintf(K_BOCHS_OUT,\
		"Alloc returned wrong address, expected 0x%x, got 0x%p\n",\
		exp, got);\
	return false;\
}

#define check_read_write(addr,sz) \
if(!(write_read(addr,sz)))	{\
	kprintf(K_BOCHS_OUT,\
		"Read/write test failed on addr: %p with size: %i\n",\
		addr, sz);\
	return false;\
}



bool write_read(uint32_t* addr, uint32_t sz)	{
	uint32_t i;
	for(i = 0; i < sz; i++)	{
		addr[i] = i;
	}
	for(i = 0; i < sz; i++)	{
		if(addr[i] != i)	return false;
	}
	return true;
}


bool heap_run_all_tests()	{
	uint32_t* alloc = NULL, *alloc2 = NULL;
	uint32_t curr_heap_addr;	// The expected heap address
	
	#define TEST_SZ1 100
	#define TEST_SZ2 5000



	// Allocate and check the first block
	curr_heap_addr = HEAP_START + sizeof(LLMalloc);
	alloc = (uint32_t*)heap_malloc(sizeof(uint32_t)*TEST_SZ1);
	
	check_header_addr(alloc, curr_heap_addr)
	check_read_write(alloc,TEST_SZ1)


	// Allocate and check the second block
	curr_heap_addr += sizeof(LLMalloc) + (sizeof(uint32_t)*TEST_SZ1);
	// Allocate more than one 4 KB block
	alloc2 = (uint32_t*)heap_malloc(sizeof(uint32_t)*TEST_SZ2);
	
	check_header_addr(alloc2, curr_heap_addr)
	check_read_write(alloc2, TEST_SZ2)
	

	// Free the first block and allocate it again, should be exactly enough space
	// to get the same block back.
	heap_free(alloc);
	
	// We should get the first block back
	curr_heap_addr = HEAP_START + sizeof(LLMalloc);
	alloc = (uint32_t*)heap_malloc(sizeof(uint32_t)*TEST_SZ1);
	
	check_header_addr(alloc, curr_heap_addr)
	check_read_write(alloc, TEST_SZ1)


	// Free both objects
	heap_free(alloc);
	heap_free(alloc2);
	return true;
}

#endif	// End for test code



/** @} */	// heap
