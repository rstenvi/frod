/**
* \file config.h
* Defines the configuration of the system and how the kernel behaves.
* \remark All variables are definitions, so one change here requires
* re-compilation.
*/

#ifndef __CONFIG_H
#define __CONFIG_H


//-------------------------- CONSOLE ---------------------------------

/** Default background colors, possible values are defined in vga_color. */
#define DEFAULT_BG Black

/** Default foreground colors, possible values are defined in vga_color. */
#define DEFAULT_FG White






//-------------------------- CPU ---------------------------------------

/**
* The max number of CPUs than can be used on the system. Is just used to mark
* the size of the array, but it will limit the number of processors in use if it
* is lower than the actual number of processors.
*/
#define MAX_CPUS 8


/**
* What should be set as the number of times each processor is interrupted each
* second.
*/
#define DEFAULT_INTR_SEC 10





//-------------------------- Virtual memory -------------------------------

/**
* Should be able to define this here and use pmm and vmm sizes accordingly. Must
* also set the appropriate bit for the CPU. Is only possible if PAE is NOT used.
* See address translation on section 4.3 in I3A. If PAE is enabled, this can
* mean 2MB pages, which is allowed.
*/
#define PAGES_4MB 0

/**
* Enable PAE extension. See section 4.4 in I3A. This is needed to use
* execute-disable, see 5.13 in I3A.
*/
#define PAE_ENABLE 0





//------------------------------ Heap ---------------------------------------

/**
* Number of 4KB blocks to allocate each time we need more physical space.
*/
#define HEAP_BLOCKS 1024

#endif
