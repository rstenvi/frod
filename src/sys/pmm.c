/**
* \pmm.c
* Physical memory manager.
* A bitmap is used to keep track of pages. Each block has a size of PMM_BLK_SZ
* bytes, since it is meant to support paging, that size should be 4096 (4 KB).
* Since all the allocation and deallocation works with blocks and the
* initialization doesn't care about the block size, any block size should work.
* \todo Clean up code and variables.
* \todo Make a clean API
* \todo Document all parts
*/
/**
* \addtogroup pmm
* @{
*/

#include "sys/kernel.h"
#include "sys/multiboot1.h"

#include "lib/stdio.h"

#define PMM_BLK_SZ 4096

#define bitmap_set(a,b) a[(b)/8]|=(1<<((b)%8))

#define bitmap_unset(a,b) a[(b)/8] &= ~(1 << ((b) % 8));

#define bitmap_is_set(a,b) (a[(b)/8] & (1 << ((b) % 8)))

#define align_downwards(a,b) a-=(a%(b))
#define align_upwards(a,b) if(a%b!=0) {a=((a/b)*b)+b;}




/** Address to the bitmap. */
uint8_t* pmm_bitmap = NULL;

/**
* The number of blocks in the system and also the number of bits used after
* pmm_bitmap to mark available and unavailable space.
*/
uint32_t pmm_bitmap_sz = 0;




//--------------- Internal function definitions ---------------------------

/**
* Get highest physical address.
* \param[in] mmap The memory map we get from Grub
* \param[in] len The length of mmap which we also get from Grub.
* \return The highest physical address in the system.
*/
uint32_t pmm_get_max_space(multiboot_mmap* mmap, uint32_t len);







//------------------- Public API function implementations ------------------

uint32_t init_pmm(multiboot_mmap* mmap, uint32_t len)	{
	multiboot_mmap* copy = mmap;
	uint32_t max = pmm_get_max_space(copy, len);
	copy = mmap;
	
	// Number of bits needed to mark all blocks
	pmm_bitmap_sz = (max / PMM_BLK_SZ);
	
	uint32_t pmm_bytes = pmm_bitmap_sz/8;
	if(pmm_bitmap_sz%8 != 0)	{
		pmm_bytes++;
	}
	
	// Calculate how many blocks we need to allocate to store bitmap
	uint32_t blocks_bitmap = pmm_bytes / PMM_BLK_SZ;
	if( (pmm_bitmap_sz % (PMM_BLK_SZ*8)) != 0)	{
		blocks_bitmap++;
	}

	
	// Number of bytes used in bitmap
	kprintf(K_DEBUG, "Need %i bits, %i blocks, %i bytes to store bitmap @0x%X\n", pmm_bitmap_sz,
		blocks_bitmap, pmm_bytes, pmm_bitmap);
	
	// Find contigous area in memory to store our bitmap
	while((uint32_t)copy < (uint32_t)mmap + len)	{
		if(copy->type == 1 && (copy->length_l / PMM_BLK_SZ) >= blocks_bitmap)	{
			// We need to make sure the base is aligned
			uint32_t base = copy->base_addr_l;
			align_upwards(base,PMM_BLK_SZ);
			
			// Must also align the end address
			uint32_t end = copy->base_addr_l + copy->length_l;
			align_downwards(base,PMM_BLK_SZ);
			
			if( (end - base) / PMM_BLK_SZ >= blocks_bitmap)	{
				pmm_bitmap = (uint8_t*)base;
				uint32_t i = 0;
				for(i = 0; i < pmm_bytes; i++)	{
					*(pmm_bitmap+i) = 0x00;
				}
				// Must mark the memory region with bitmap as taken
				for(i = 0; i < blocks_bitmap; i++)	{
					bitmap_set(pmm_bitmap, (i+(base/PMM_BLK_SZ)));
				}
				break;
			}
		}
		copy = (multiboot_mmap*)((uint32_t)copy + (copy->size + 4));
	}
	copy = mmap;

	// Mark all reserved memory as taken
	uint32_t expected = 0;
	while((uint32_t)copy < (uint32_t)mmap + len && (copy->base_addr_l < max-1))	{
		// First check if there is a gap in the specification, if there is, that
		// memory is taken.
		if(copy->base_addr_l > expected/* && copy->base_addr_l < max-1*/)	{
			align_downwards(expected,PMM_BLK_SZ);

			while(expected < copy->base_addr_l)	{
				bitmap_set(pmm_bitmap, expected/PMM_BLK_SZ);
				expected += PMM_BLK_SZ;
			}
		}

		// If this memory is taken and below our max
		if(copy->type != 1 && (copy->base_addr_l + copy->length_l) < max-1)	{
			// Mark this memory as taken
			uint32_t base = copy->base_addr_l;
			align_downwards(base,PMM_BLK_SZ);
			while(base < (copy->base_addr_l + copy->length_l))	{
				bitmap_set(pmm_bitmap, base/PMM_BLK_SZ);
				base += PMM_BLK_SZ;
			}
		}
		expected = copy->base_addr_l + copy->length_l;
		copy = (multiboot_mmap*)((uint32_t)copy + (copy->size + 4));
	}
	// 0 is error-value, so is always marked as used, might actually be used for
	// bitmap
	bitmap_set(pmm_bitmap, 0);
	return max;
}

bool pmm_is_taken(uint32_t block)	{
	return bitmap_is_set(pmm_bitmap,block);	
}


void pmm_mark_mem_taken(uint32_t start, uint32_t end)	{
	align_downwards(start, PMM_BLK_SZ);
	align_upwards(end, PMM_BLK_SZ);

	uint32_t i = start / PMM_BLK_SZ, j = end / PMM_BLK_SZ;
	for(; i < j; i++)	{
		bitmap_set(pmm_bitmap, i);
	}
}



// Is very slow when many of the first are taken, there are several ways around
// that
void* pmm_alloc_first()	{
	uint32_t i = 1;
	for(i = 1; i< pmm_bitmap_sz; i++)	{
		if(!bitmap_is_set(pmm_bitmap,i))	{
			bitmap_set(pmm_bitmap, i);
			return (void*)(i*PMM_BLK_SZ);
		}
	}
	return NULL;
}

void pmm_free(void* block)	{
	uint32_t num = (uint32_t)block;
	num /= PMM_BLK_SZ;
	bitmap_unset(pmm_bitmap, num);
}




void* pmm_alloc_first_n_blocks(uint32_t n)	{
	uint32_t found = 0, i;
	for(i = 1; i < pmm_bitmap_sz; i++)	{
		if(!bitmap_is_set(pmm_bitmap,i))	{
			found++;
			if(found == n)	{
				for(; found > 0; found--)	{
					bitmap_set(pmm_bitmap, i-(found-1));
				}
				return (void*)((i-(n-1))*PMM_BLK_SZ);
			}
		}
		else	found = 0;
	}
	return NULL;
}





//------------------- Internal function implementation ------------------------

uint32_t pmm_get_max_space(multiboot_mmap* mmap, uint32_t len)	{
	uint32_t max = 0;
	uint32_t mmap_addr = (uint32_t)mmap;
	while((uint32_t)mmap < mmap_addr + len)	{
		if(mmap->type == 1)	{
			max = ((uint32_t)mmap->base_addr_l + (uint32_t)mmap->length_l);
		}
		mmap = (multiboot_mmap*)((uint32_t)mmap + (mmap->size + 4));
	}
	return max;
}



//------------- Test-code -------------------------



#ifdef TEST_KERNEL
int pmm_test_defines()	{
	uint8_t arr[2];
	memset(arr, 0x00, 2);
	
	bitmap_set(arr, 1);
	if(!bitmap_is_set(arr,1))	return 1;

	bitmap_unset(arr, 1);
	if(arr[0] != 0)	return 2;

	bitmap_set(arr, 2);
	bitmap_set(arr, 3);
	if(!bitmap_is_set(arr,2) || !bitmap_is_set(arr,3))	return 3;


	bitmap_set(arr, 8);
	if(!bitmap_is_set(arr,8))	return 4;

	int32_t num = 0;

	align_downwards(num, 4096);
	if(num != 0)	return 5;
	
	num = 100;
	align_downwards(num, 4096);
	if(num != 0)	return 6;

	num = 5000;
	align_downwards(num, 4096);
	if(num != 4096)	return 7;


	// Should be no change
	align_upwards(num, 4096);
	if(num != 4096)	return 8;

	num--;	// 4095
	
	align_upwards(num, 4096);
	if(num != 4096)	return 9;
	
	num += 1;	// 4097

	align_upwards(num, 4096);
	if(num != 8192)	return 10;

	return 0;
}

bool pmm_test_emulate()	{

	return true;
}


bool pmm_run_all_tests()	{
	int res;
	res = pmm_test_defines();
	if(res > 0)	{
		kprintf(K_LOW_INFO, "pmm_test_defines() failed: %i\n", res);
		return false;
	}
	return true;
}

#endif

/** @} */	// pmm
