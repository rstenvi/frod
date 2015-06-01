/**
* \file uart.c
* Driver for the 8250 UART
* http://wiki.osdev.org/Serial_Ports
* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming
*/

#include "sys/kernel.h"
#include "hal/hal.h"
#include "drv/uart.h"

#define UART_COM1 0x3F8
#define UART_COM2 0x2F8
#define UART_COM3 0x3E8
#define UART_COM4 0x2E8
	
	// Transmitter Holding Buffer
	#define UART_OFFSET_THR 0	// DLAB 0 (write-only)
	
	// Receiver Buffer
	#define UART_OFFSET_RBR 0	// DLAB 0 (read-only)
	
	// Divisor Latch Low byte
	#define UART_OFFSET_DLL 0	// DLAB 1
	
	// Interrupt Enable Register
	#define UART_OFFSET_IER 1	// DLAB 0

		// Low power mode
		#define IER_MASK_LPM   (1 << 5)

		// Sleep mode
		#define IER_MASK_SM    (1 << 4)

		// Modem status interrupt
		#define IER_MASK_MSI   (1 << 3)

		// Receiver line status interrupt
		#define IER_MASK_RLSI  (1 << 2)

		// Transmitter holding register empty interrupt
		#define IER_MASK_THREI (1 << 1)

		// Received data available interrupt
		#define IER_MASK_RDAI  (1 << 0)
	
	// Divisor Latch High byte
	#define UART_OFFSET_DLH 1	// DLAB 1
	
	// Interrupt Identification Register
	#define UART_OFFSET_IIR 2	// read-only

		#define IIR_MASK_FIFO   (3 << 6)
			#define IIR_FIFO_NO          (0 << 6)
			#define IIR_FIFO_RESERVED    (1 << 6)
			#define IIR_FIFO_ENABLED_ERR (2 << 6)
			#define IIR_FIFO_ENABLED     (3 << 6)
		#define IIR_MASK_64FIFO (1 << 5)
		#define IIR_MASK_EVENT  (7 << 1)
			#define IIR_EVENT_MSI   (0 << 1)
			#define IIR_EVENT_THREI (1 << 1)
			#define IIR_EVENT_RDAI  (2 << 1)
			#define IIR_EVENT_RLSI  (3 << 1)
			#define IIR_EVENT_RES1  (4 << 1)
			#define IIR_EVENT_RES2  (5 << 1)
			#define IIR_EVENT_TIME  (6 << 1)
			#define IIR_EVENT_RES3  (7 << 1)
		#define IIR_MASK_PEND   (1 << 0)
	
	// FIFO Control Register
	#define UART_OFFSET_FCR 2	// write-only
		
		// Value on hte levels depend on the byte
		#define FCR_MASK_TRG_LEV1  (0 << 6)
		#define FCR_MASK_TRG_LEV2  (1 << 6)
		#define FCR_MASK_TRG_LEV3  (2 << 6)
		#define FCR_MASK_TRG_LEV4  (3 << 6)

		#define FCR_MASK_64BYTE    (1 << 5)
		#define FCR_MASK_DMA_MOE   (1 << 3)
		#define FCR_MASK_CLR_TRANS (1 << 2)
		#define FCR_MASK_CLR_REC   (1 << 1)
		#define FCR_MASK_ENABLE    (1 << 0)
	
	// Line Control Register
	#define UART_OFFSET_LCR 3

		#define LCR_MASK_DLAB         (1 << 7)
		#define LCR_MASK_ENABLE_BREAK (1 << 6)

		#define LCR_MASK_PARITY_NO    (0 << 3)
		#define LCR_MASK_PARITY_ODD   (1 << 3)
		#define LCR_MASK_PARITY_EVEN  (3 << 3)
		#define LCR_MASK_PARITY_MARK  (5 << 3)
		#define LCR_MASK_PARITY_SPACE (7 << 3)

		#define LCR_MASK_STOP2        (1 << 2)

		#define LCR_MASK_WORD_LEN5    (0 << 0)
		#define LCR_MASK_WORD_LEN6    (1 << 0)
		#define LCR_MASK_WORD_LEN7    (2 << 0)
		#define LCR_MASK_WORD_LEN8    (3 << 0)
	
	// Modem Control Register
	#define UART_OFFSET_MCR 4

		#define MCR_MASK_AUTOFLOW (1 << 5)
		#define MCR_MASK_LOOPBACK (1 << 4)
		#define MCR_MASK_AUX2     (1 << 3)
		#define MCR_MASK_AUX1     (1 << 2)
		#define MCR_MASK_REQ_SEND (1 << 1)
		#define MCR_MASK_TERM_RDY (1 << 0)
	
	// Line Status Register
	#define UART_OFFSET_LSR 5	// read-only

		#define LSR_MASK_ERR_FIFO    (1 << 7)
		#define LSR_MASK_EMPTY_EDH   (1 << 6)
		#define LSR_MASK_EMPTY_THR   (1 << 5)
		#define LSR_MASK_BRK_INTR    (1 << 4)
		#define LSR_MASK_ERR_FRAME   (1 << 3)
		#define LSR_MASK_ERR_PARITY  (1 << 2)
		#define LSR_MASK_ERR_OVERRUN (1 << 1)
		#define LSR_MASK_DATA_RDY    (1 << 0)

		#define LSR_MASK_NO_UART 0xFF
	
	// Modem Status Register
	#define UART_OFFSET_MSR 6	// read-only

		#define MSR_BIT_CARR_DETECT 7
		#define MSR_BIT_RING_IND    6
		#define MSR_BIT_DATA_RDY    5
		#define MSR_BIT_CLR_SEND    4
		#define MSR_BIT_DDC_DETECT  3
		#define MSR_BIT_TRAIL_RING  2
		#define MSR_BIT_DD_SET_RDY  1
		#define MSR_BIT_DC_SEND     0

	// Scratch Register
	#define UART_OFFSET_SR  7

#define UART_COM1_IRQ IRQ4
#define UART_COM2_IRQ IRQ3
#define UART_COM3_IRQ IRQ4
#define UART_COM4_IRQ IRQ3



static bool uart_present;

static void uart_set_baud_rate(uint16_t baud_rate);

uint32_t uart_interrupt_com1(Registers* regs);



int8_t uart_init()	{
	outb(UART_COM1+UART_OFFSET_IER, 0x00);	// Disable Interrupts

	// Set the baud rate

	// Baud rate defined in config.h
	uart_set_baud_rate(UART_BAUD_RATE);


	// Set the FIFO control register
	outb(UART_COM1+UART_OFFSET_FCR,
		FCR_MASK_ENABLE |
		FCR_MASK_CLR_REC |
		FCR_MASK_CLR_TRANS |
		FCR_MASK_TRG_LEV4
	);
	outb(UART_COM1+UART_OFFSET_MCR,
		MCR_MASK_TERM_RDY |
		MCR_MASK_REQ_SEND |
		MCR_MASK_AUX2
	);

	// Receive interrupts when data is available to receive
	outb(UART_COM1+UART_OFFSET_IER,
		IER_MASK_RDAI
	);

	// Check if there actually is a serial port
	if(inb(UART_COM1+UART_OFFSET_LSR) == LSR_MASK_NO_UART)	{
		uart_present = false;
		return UART_ERR_NOT_PRESENT;
	}

	// Set 8-bit words
	uint8_t lcr = inb(UART_COM1+UART_OFFSET_LCR);
	lcr |= LCR_MASK_WORD_LEN8;
	outb(UART_COM1+UART_OFFSET_LCR, lcr);

	// We have a serial port and should enable it
	uart_present = true;

	// Acknowledge any lingering interrupts
	inb(UART_COM1+UART_OFFSET_IIR);
	inb(UART_COM1+UART_OFFSET_RBR);

	// Enable interrupt in IOAPIC

	register_interrupt_handler(UART_COM1_IRQ, uart_interrupt_com1);
	pic_enable_irq(UART_COM1_IRQ-IRQ0);

	return UART_SUCCESS;
}

int16_t uart_getc()	{
	if(uart_present == false)	return UART_ERR_NOT_PRESENT;

	if(inb(UART_COM1+UART_OFFSET_LSR) & LSR_MASK_DATA_RDY)
		return (int16_t)inb(UART_COM1+UART_OFFSET_RBR);
	
	return UART_ERR_GETC_NOT_READY;
}

int8_t uart_putc(uint8_t c)	{
	if(uart_present == false)	return UART_ERR_NOT_PRESENT;
	int i;
	for(i = 0;
		i < 1024*16 && !(inb(UART_COM1+UART_OFFSET_LSR) & LSR_MASK_EMPTY_THR);
		i++
	)	{
		// TODO: Insert delay
	}
	
	// Check if we are ready to transmit or we timed out
	if(inb(UART_COM1+UART_OFFSET_LSR) & LSR_MASK_EMPTY_THR)	{
		outb(UART_COM1+UART_OFFSET_THR, c);
		return UART_SUCCESS;
	}
	return UART_ERR_PUTC_NOT_READY;
}


static void uart_set_baud_rate(uint16_t baud_rate)	{
	uint16_t dlv = 115200/baud_rate;

	// Set DLAB to 1 - Set baud rate - Set DLAB to 0
	outb(UART_COM1+UART_OFFSET_LCR, inb(UART_COM1+UART_OFFSET_LCR) | LCR_MASK_DLAB);
	outb(UART_COM1+UART_OFFSET_DLL, dlv & 0xFF);
	outb(UART_COM1+UART_OFFSET_DLH, ((dlv & 0xFF00) >> 8));
	outb(UART_COM1+UART_OFFSET_LCR, inb(UART_COM1+UART_OFFSET_LCR) & ~(LCR_MASK_DLAB));
}

uint32_t uart_interrupt_com1(Registers* regs)	{
	(void)regs;
	kprintf(K_BOCHS_OUT, "UART INTERR\n");

	return 0;
}

