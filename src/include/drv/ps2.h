#ifndef __PS2_H
#define __PS2_H


#define PS2_ERR_INPUT_BUFFER_FULL -1
#define PS2_ERR_OUTPUT_BUFFER_EMPTY -2

#define PS2_ERR_SELF_TEST_FAILURE -3 

#define PS2_ERR_NO_USABLE_CHANNEL -4

#define PS2_ERR_INVALID_PARAM -5

#define PS2_ERR_CHANNEL_BROKEN -6
#define PS2_ERR_CHANNEL_ACTIVE -7
#define PS2_ERR_RESET          -8

#define PS2_SUCCESS 0

#define PS2_DEV_STATUS_SELF_TEST (1 << 0)
#define PS2_DEV_STATUS_READY 0



#define PS2_MAX_NAME_SZ 32

typedef void (*ps2_input)(uint8_t data);

typedef struct	{
	ps2_input callback;
	char name[PS2_MAX_NAME_SZ];
	bool active;
} ps2_devices;


/**
* Initialize the PS/2 controller.
* \remark This assumes that a PS/2 controller is present, FADT should be parsed
* to ensure that it is present.
* \return Returns a positive value if 1 or 2 devices work, 0 if both work. A
* negative value means that the controller doesn't work.
*/
int16_t ps2_init();


int8_t ps2_register(ps2_input handler, char* name, int n);

#endif
