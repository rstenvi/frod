/**
* \file tss.h
* HW task switching is not used, but struct is maintained to keep it working
* properly.
*/

#ifndef __TSS_H
#define __TSS_H



typedef struct	{
	uint32_t prev_tss,
				esp0,
				ss0,
				esp1,
				ss1,
				esp2,
				ss2,
				cr3,
				eip,
				eflags,
				eax,
				ecx,
				edx,
				ebx,
				esp,
				ebp,
				esi,
				edi,
				es,
				cs,
				ss,
				ds,
				fs,
				gs,
				ldt;
	uint16_t trap,
				iomap_base;
} __attribute__((packed)) tss_entry;


#endif
