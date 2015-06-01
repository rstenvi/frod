
/**
* \file ps2.c
* Generic PS/2 driver, used for the keyboard and maybe more in the future.
*
* About PS/2
* - It is a point-to-point protocol with only two parties.
* - The device (keyboard, mouse etc) is considered the master and is responsible
* for driving the clock.
* - The host computer is the slave.
* - The master decides the direction of data transfer and the slave must ask to
* transfer data
* - Each packet sent from master to slave has the following 11 bits:
*  - 1 start bit (always 0)
*  - 8 data bits (LSB)
*  - 1 parity bit (odd)
*  - 1 stop bit (always 1)
* - Data from slave to master has the following 12 bits:
*  - 1 Start bit (always 0)
*  - 8 data bits (LSB)
*  - 1 parity bit (odd)
*  - 1 stop bit (always 1)
*  - 1 Acknowledgement bit
*
* About the driver
* - It uses an interrupt-driven design, so caller must usually specify a
* call-back function for when the command is completed.
* - The driver uses a queue to store commands that should be sent and finishes
* each command (including response) before doing the next.
* - If the queue is full, caller will have to retry later.
* - To simplify the design on the caller side, the caller can chain multiple
* commands before executing them. The caller must also specify a failure and
* success function. With this, the state machine of the caller can be simplier,
* since it only need to be waware of the state of each collection of commands.
*
* Different data that can be sent:
* - Send command with no response
* - Send command with response
* - Both types of command can also be two bytes
* - Send data with no response
*/

#include "sys/kernel.h"
#include "hal/hal.h"
#include "drv/ps2.h"
#include "lib/string.h"
#include "lib/stdio.h"

#define PS2_PORT_READ_DATA    0x60

	#define PS2_RES_ACK 0xFA

#define PS2_PORT_WRITE_DATA   0x60

	#define PS2_CMD_IDENTIFY 0xF2

#define PS2_PORT_READ_STATUS  0x64
	// Output to us
	#define PS2_STATUS_OUTPUT_BUFF_FULL (1 << 0)

	// Input to PS/2 controller
	#define PS2_STATUS_INPUT_BUFF_FULL (1 << 1)
	
	// Send data to controller instead of device
	#define PS2_STATUS_FOR_CTRL (1 << 3)

#define PS2_PORT_WRITE_STATUS 0x64
	#define PS2_CMD_SEL_2ND 0xD4

	#define PS2_CMD_DISABLE_1ST 0xAD
	#define PS2_CMD_DISABLE_2ND 0xA7

	#define PS2_CMD_ENABLE_1ST 0xAE
	#define PS2_CMD_ENABLE_2ND 0xA8

	#define PS2_CMD_READ_CCB 0x20
	#define PS2_CMD_WRITE_CCB 0x60
		#define PS2_CCB_MASK_INTR_FIRST   (1 << 0)
		#define PS2_CCB_MASK_INTR_SECOND  (1 << 1)
		#define PS2_CCB_MASK_PASSED  (1 << 2)
		#define PS2_CCB_MASK_ZERO    (1 << 3)
		#define PS2_CCB_MASK_CLOCK1  (1 << 4)
		#define PS2_CCB_MASK_CLOCK2  (1 << 5)
		#define PS2_CCB_MASK_TRANS1  (1 << 6)
	#define PS2_CMD_SELF_TEST 0xAA
		#define PS2_SELF_TEST_SUCCESS 0x55
	#define PS2_CMD_TEST_1ST 0xAB
	#define PS2_CMD_TEST_2ND 0xA9
		#define PS2_CMD_TEST_SUCCESS     0x00
		#define PS2_CMD_TEST_CLOCK_STUCK 0x01
		#define PS2_CMD_TEST_DATA_STUCK  0x04

#define PS2_TO_DEFAULT 10000

// The status of each channel
static int16_t channel_status = 0x0000;

#define PS2_MAX_REGISTERED 2

ps2_devices registered[2];


extern cpu_info cpus[MAX_CPUS];

#define PS2_STATE_STD       0
#define PS2_STATE_IDENTIFY1 1
#define PS2_STATE_IDENTIFY2 2
volatile uint8_t ps2_state = 0;




#define PS2_SEND_MASK_DATA     (1 << 0)
#define PS2_SEND_MASK_RESPONSE (1 << 1)
#define PS2_SEND_MASK_SECOND   (1 << 2)
#define PS2_SEND_MASK_DOUBLE   (1 << 3)
int16_t ps2_send_internal(uint16_t send, int ms, uint16_t flags);


/**
* Wait until we can read from the buffer.
*/
static inline bool wait_data_ready(int32_t ms)	{
	for(;
		ms > 0 && !(inb(PS2_PORT_READ_STATUS) & PS2_STATUS_OUTPUT_BUFF_FULL);
		ms--)
	{
		// wait
	}
	if(ms < 0)	return false;
	return true;
}

/**
* Wait until we can write to the buffer.
*/
static inline bool wait_buffer_ready(int32_t ms)	{
	for(;
		ms > 0 && (inb(PS2_PORT_READ_STATUS) & PS2_STATUS_INPUT_BUFF_FULL);
		ms--)
	{
		// TODO: wait(1 ms)
	}
	if(ms < 0)	return false;
	return true;
}

int16_t ps2_send_data_internal(uint8_t data, int to_ms, bool first, bool resp);
int16_t ps2_send_cmd_internal(uint8_t cmd, int ms, int16_t next, bool response);

#define PS2_SEND_MASK_DATA     (1 << 0)
#define PS2_SEND_MASK_RESPONSE (1 << 1)
#define PS2_SEND_MASK_SECOND   (1 << 2)
#define PS2_SEND_MASK_DOUBLE   (1 << 3)
int8_t ps2_send_cmd(uint8_t cmd, int to_ms)	{
	return ps2_send_internal(cmd, to_ms, 0);
}

int16_t ps2_send_cmd_resp(uint8_t cmd, int to_ms)	{
	uint16_t resp = ps2_send_internal(cmd, to_ms, PS2_SEND_MASK_RESPONSE);
	return resp;
}

int8_t ps2_send_cmd_double(uint8_t cmd, int to_ms, uint8_t cmd2)	{
	uint16_t cm = cmd2;
	cm <<= 8;
	cm |= cmd;
	return ps2_send_internal(cm, to_ms, PS2_SEND_MASK_DOUBLE);
}

int16_t ps2_send_cmd_resp_double(uint8_t cmd, int to_ms, uint8_t cmd2)	{
	uint16_t cm = cmd2;
	cm <<= 8;
	cm |= cmd;
	uint16_t resp = ps2_send_internal(cm, to_ms,
		PS2_SEND_MASK_RESPONSE | PS2_SEND_MASK_DOUBLE
	);
	return resp;
}

int16_t ps2_send_data(uint8_t data, int to_ms)	{
	return ps2_send_internal(data, to_ms, PS2_SEND_MASK_DATA);
}
int16_t ps2_send_data_second(uint8_t data, int to_ms)	{
	return ps2_send_internal(data, to_ms,
		PS2_SEND_MASK_DATA | PS2_SEND_MASK_SECOND
	);
}
int16_t ps2_send_data_response(uint8_t data, int to_ms)	{
	return ps2_send_internal(data, to_ms,
		PS2_SEND_MASK_DATA | PS2_SEND_MASK_RESPONSE
	);
}
int16_t ps2_send_data_response_second(uint8_t data, int to_ms)	{
	return ps2_send_internal(data, to_ms,
		PS2_SEND_MASK_DATA | PS2_SEND_MASK_RESPONSE | PS2_SEND_MASK_SECOND
	);
}


uint32_t ps2_handle_interrupt_1st(Registers* regs)	{
	(void)regs;
	uint8_t data = inb(PS2_PORT_READ_DATA);
	if(registered[0].active)
		registered[0].callback(data);
	return 0;
}
uint32_t ps2_handle_interrupt_2nd(Registers* regs)	{
	(void)regs;
	uint8_t data = inb(PS2_PORT_READ_DATA);
	if(registered[1].active)
		registered[1].callback(data);
	return 0;
}



int16_t ps2_init()	{
	int i;
	for(i = 0; i < PS2_MAX_REGISTERED; i++)	{
		memset(&registered[i], 0x00, sizeof(ps2_devices));
		registered[i].active = false;
	}


	// Step 1: Disable devices, the second will be ignored if there is only 1
	// channel
	ps2_send_cmd(PS2_CMD_DISABLE_1ST, 0);
	ps2_send_cmd(PS2_CMD_DISABLE_2ND, 0);

	// Step 2: Flush the output buffer
	inb(PS2_PORT_READ_DATA);

	// Step 3: Set controller configuration byte, disable interrupts,
	// translation and chech if single or dual channel
	uint8_t ccb;
	int16_t resp = ps2_send_cmd_resp(PS2_CMD_READ_CCB, 0);
	if(resp < 0)	return resp;
	ccb = (resp & 0xFF);
	ccb &= ~(PS2_CCB_MASK_INTR_FIRST);
	ccb &= ~(PS2_CCB_MASK_INTR_SECOND);
	ccb &= ~(PS2_CCB_MASK_TRANS1);

	ps2_send_cmd_double(PS2_CMD_WRITE_CCB, 0, ccb);


	// Step 4: Perform self-test
	resp = ps2_send_cmd_resp(PS2_CMD_SELF_TEST, 0);
	if(resp < 0)	return resp;
	if((uint8_t)(resp & 0x00FF) != PS2_SELF_TEST_SUCCESS)	{
		channel_status = PS2_DEV_STATUS_SELF_TEST | (PS2_DEV_STATUS_SELF_TEST << 8);
		return PS2_ERR_SELF_TEST_FAILURE;
	}


	// Step 5: If there might be two channels, check
	ps2_send_cmd(PS2_CMD_ENABLE_2ND, 0);
	resp = ps2_send_cmd_resp(PS2_CMD_READ_CCB, 0);
	if(resp < 0)	{
		channel_status = 0x0202;
		return resp;
	}
	if( (resp & 0x00FF) & PS2_CCB_MASK_CLOCK2)	{
		channel_status = 0x0400;
	}
	else	{
		ps2_send_cmd(PS2_CMD_DISABLE_2ND, 0);
	}

	// Step 6: Test the interface
	resp = ps2_send_cmd_resp(PS2_CMD_TEST_1ST, 0);
	if(resp < 0)	{
		channel_status |= 0x08;
	}

	if( (channel_status & 0xFF00) == 0)	{
		resp = ps2_send_cmd_resp(PS2_CMD_TEST_2ND, 0);
		if(resp < 0)	{
			channel_status |= 0x0800;
		}
	}

	// If both channels are unusable, we return an error
	if( (channel_status & 0xFF00) && (channel_status & 0x00FF))	{
		return PS2_ERR_NO_USABLE_CHANNEL;
	}


	// Step 7: Enable devices and interrupts if they work
	resp = ps2_send_cmd_resp(PS2_CMD_READ_CCB, 0);
	if(resp < 0)	return resp;
	ccb = (resp & 0xFF);
	if((channel_status & 0xFF) == 0)	{
		ps2_send_cmd(PS2_CMD_ENABLE_1ST, 0);
		ccb |= PS2_CCB_MASK_INTR_FIRST;

	}
	if((channel_status & 0xFF00) == 0)	{
		ps2_send_cmd(PS2_CMD_ENABLE_2ND, 0);
		ccb |= PS2_CCB_MASK_INTR_SECOND;
	}
	// Send the byte back, which enables interrupts on one or both
	ps2_send_cmd_double(PS2_CMD_WRITE_CCB, 0, ccb);

	// TODO: Register for the interrupts


	// Step 8: Reset both devices
	if((channel_status & 0xFF) == 0)	{
		resp = ps2_send_data_response(0xFF, 0);
		if( (uint8_t)(resp & 0x00FF) != 0xFA)	{
			kprintf(K_HIGH_INFO, "\t1: 0x%x\n", resp);
			channel_status |= 4;
		}
	}
	if((channel_status & 0xFF00) == 0)	{
		resp = ps2_send_data_response_second(0xFF, 0);
		if( (uint8_t)(resp & 0x00FF) != 0xFA)	{
			kprintf(K_HIGH_INFO, "\t2: 0x%x\n", resp);
			channel_status |= (4 << 8);
		}
	}


	register_interrupt_handler(IRQ_KEYBOARD, ps2_handle_interrupt_1st);
	register_interrupt_handler(IRQ_PS2, ps2_handle_interrupt_2nd);

	pic_enable_irq(IRQ_KBD);
	pic_enable_irq(IRQ_PS2-IRQ0);
	return channel_status;
}


int8_t ps2_register(ps2_input handler, char* name, int n)	{
	if(n < 1 || n > 2)	return PS2_ERR_INVALID_PARAM;
	if(n == 1 && (channel_status & 0xFF) != 0)		return PS2_ERR_CHANNEL_BROKEN;
	if(n == 2 && (channel_status & 0xFF00) != 0)	return PS2_ERR_CHANNEL_BROKEN;

	n--;
	if(registered[n].active == true)	return PS2_ERR_CHANNEL_ACTIVE;

	registered[n].active = true;
	strncpy(registered[n].name, name, PS2_MAX_NAME_SZ);

	registered[n].callback = handler;
	return PS2_SUCCESS;
}




/**
* Internal function for sending a (1 or 2-byte) command and optionally waiting
* for response.
*/
int16_t ps2_send_cmd_internal(uint8_t cmd, int ms, int16_t next, bool response)	{
	if(ms == 0)	ms = PS2_TO_DEFAULT;
	uint16_t ret = PS2_SUCCESS;
	outb(PS2_PORT_WRITE_STATUS, cmd);
	if(next >= 0)	{
		if(wait_buffer_ready(ms) == false)	return PS2_ERR_INPUT_BUFFER_FULL;
		outb(PS2_PORT_WRITE_DATA, (uint8_t)(next & 0xFF));
	}
	if(response)	{
		if(wait_data_ready(ms) == false)	return PS2_ERR_OUTPUT_BUFFER_EMPTY;
		ret = (uint16_t)inb(PS2_PORT_READ_DATA);
	}
	return ret;
}


int16_t ps2_send_data_internal(uint8_t data, int ms, bool first, bool resp)	{
	if(ms == 0)	ms = PS2_TO_DEFAULT;
	int16_t ret;

	if(first == false)	{
		outb(PS2_PORT_WRITE_STATUS, PS2_CMD_SEL_2ND);
	}
	if(wait_buffer_ready(ms) == false)	return PS2_ERR_INPUT_BUFFER_FULL;
	outb(PS2_PORT_WRITE_DATA, data);

	if(resp)	{
		if(wait_data_ready(ms) == false)	return PS2_ERR_OUTPUT_BUFFER_EMPTY;
		ret = (uint16_t)inb(PS2_PORT_READ_DATA);
		return ret;
	}
	return PS2_SUCCESS;
}


int16_t ps2_send_internal(uint16_t send, int ms, uint16_t flags)	{
	if(ms == 0)	ms = PS2_TO_DEFAULT;
	int16_t ret = PS2_SUCCESS;
	if(flags & PS2_SEND_MASK_DATA)	{
		if(flags & PS2_SEND_MASK_SECOND)	{
			if( (channel_status & 0xFF00) )	return PS2_ERR_NO_USABLE_CHANNEL;
			outb(PS2_PORT_WRITE_STATUS, PS2_CMD_SEL_2ND);
		}
		else if( (channel_status & 0xFF) )	return PS2_ERR_NO_USABLE_CHANNEL;
		if(wait_buffer_ready(ms) == false)	return PS2_ERR_INPUT_BUFFER_FULL;
		outb(PS2_PORT_WRITE_DATA, (uint8_t)send);
	}
	else	{
		if( (channel_status & 0xFF) && (channel_status & 0xFF00))
			return PS2_ERR_NO_USABLE_CHANNEL;

		outb(PS2_PORT_WRITE_STATUS, (uint8_t)(send & 0xFF));
		if(flags & PS2_SEND_MASK_DOUBLE)	{
			if(wait_buffer_ready(ms) == false)	return PS2_ERR_INPUT_BUFFER_FULL;
			outb(PS2_PORT_WRITE_DATA, (uint8_t)((send & 0xFF00)>>8));
		}
	}
	if(flags & PS2_SEND_MASK_RESPONSE)	{
		if(wait_data_ready(ms) == false)	return PS2_ERR_OUTPUT_BUFFER_EMPTY;
		ret = (uint16_t)inb(PS2_PORT_READ_DATA);
	}
	return ret;
}

