/**
* \ingroup pmm
* \file pmm.h
* Header file for the physical memory manager.
* About the physical memory manager:
* - The implementation uses a bitmap to store free or taken blocks.
* - Each bit represents 4KB of memory, this is chosen to coincide with the size
* of pages in virtual memory, which is eventually used.
* - Initialize the system:
*   - The system is first initialized with memory map from Grub, here the
*   manager finds out how much space it need to store the bitmap.
*   - The bitmap is stored on the first available space that is large enough.
*   - Memory map does not say where our code is located, so this is marked
*   seperately.
* - Virtual memory manager can allocate and free blocks as desired, both single
* and several consecutive blocks.
* \todo Store last block allocated / last freed block to make it faster to find
* available blocks.
* \remark There is no way to move physical memory around, so closing small holes
* to create larger gaps has not been implemented. Since paging is used, it
* should not be necessary, the kernel should instead pre-allocate some continous
* memory for when that is necessary (DMA).
*/

/**
* \addtogroup pmm
* @{
*/

#ifndef __PMM_H
#define __PMM_H

#include "kernel.h"
#include "multiboot.h"



/**
* Initialize the physical memory manager. This should of course be called early
* in the process and prefereably before paging is enabled. It assumes it is
* working with physical memory and the bitmap can be placed anywhere, but is
* placed at the earliest place possible, i.e. where it will find enough
* consecutive blocks to store the bitmap.
* \param[in] mmap Mmap we get from Grub.
* \param[in] len Length of the mmap structure, also from Grub
* \returns Returns the highest available address / number of bytes we manage
* with our bitmap.
* \remark After this paging can be enabled, but the area containing the bitmap
* must be identity mapped.
* \todo
* - Clean up and divide into internal functions
*/
uint32_t init_pmm(multiboot_mmap* mmap, uint32_t len);

/**
* Find, allocate and return the first avaiable block of memory. This can be
* called whenever you need 1 block of memory.
* \remark Only the virtual memory manager should call this as this is physical
* memory.
* \return Returns the address to the block.
* \todo
* - Will have very low speed after a while, should at least check whole byte
* first
*/
void* pmm_alloc_first();


void* pmm_alloc_first_n_blocks(uint32_t n);

/**
* Mark region of memory as taken.
* \param[in] start Start address of memory region. Will be aligned downwards.
* \param[in] end End address of memory region. Will be aligned upwards.
*/
void pmm_mark_mem_taken(uint32_t start, uint32_t end);

/**
* Free a block of memory. Free a block that has been previously allocated with
* pmm_alloc_first.
* \param[in] block Address of the block, should be aligned on 4 KB boundary.
* \remark The address is aligned downwards and if the block is already free,
* nothing happens.
*/
void pmm_free(void* block);


/**
* Check if a block is taken.
* \param[in] block The block number.
* \return Returns true if it is taken, false if it is free.
*/
bool pmm_is_taken(uint32_t block);


#endif	// File

/** @} */	// pmm
