

/**
* \addtogroup ISR
* @{
* \todo All of this should be placed in hal.h
*/

#ifndef __ISR_H
#define __ISR_H


/**
* Values we get when an interrupt happen. This values must bee in this order, it
* is the order it's pushed by "pusha".
*/
typedef struct  {
	// Pushed by us
	uint32_t gs, fs, es, ds;

	// Pushed with pusha (order is important)
	uint32_t edi, esi, ebp, tampered_esp, ebx, edx, ecx, eax;

	// err_code is sometimes pushed by the system otherwise it is 0,
	// int_no is pushed by us
	uint32_t int_no, err_code;
	//uint32_t eip, cs, eflags, esp, ss;

	// We get these values when the interrupt happen
	uint32_t eip, cs, eflags;
} __attribute__((packed)) Registers;

typedef uint32_t (*isr_t)(Registers*);



#define IRQ_PIT			32
#define IRQ_KEYBOARD		33
#define IRQ_UNUSED		34
#define IRQ_COM2			35
#define IRQ_COM1			36
#define IRQ_LPT2			37
#define IRQ_FLOPPY		38
#define IRQ_LPT1			39
#define IRQ_CMOS			40
#define IRQ_FREE1			41
#define IRQ_FREE2			42
#define IRQ_FREE3			43
#define IRQ_PS2			44
#define IRQ_FPU			45
#define IRQ_ATA_PRIM		46
#define IRQ_ATA_SEC		47


// This interrupt will happen regularly and can be safely ignored
#define IRQ_SPURIOUS 63
#define IRQ_TIMER    64
#define IRQ_ERROR    65




// TODO: Remove this list for the one above
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47


#endif	// File


/** @} */	// ISR
