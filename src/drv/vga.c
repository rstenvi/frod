/**
* \addtogroup VGA_drv
* @{
* \file vga.c
* Implementaion details for the VGA driver.
*/

#include "drv/vga.h"
#include "hal/hal.h"	// outb

/** Max number of characters that can be displaye horizontally. */
#define SCREEN_LENGTH 80

/** Max number of vertical lines. */
#define SCREEN_HEIGHT 25

#define SCREEN_ADDR 0xb8000

#define write_blank(a,b) vga_write_mem(a,b,(uint16_t)' ')

static inline void vga_write_mem(uint8_t x, uint8_t y, uint16_t val);

static void vga_scroll();
static void vga_update_cursor();

vga_screen screen;


void vga_init(vga_color fg, vga_color bg)	{
	init_spinlock(&screen.lock, LOCK_CONSOLE);
	spinlock_acquire(&screen.lock);
	screen.x = screen.y = 0;
	screen.tab_sz = 4;
	screen.color = (bg << 4) + fg;
	screen.mem = (uint16_t*)SCREEN_ADDR;
	vga_clear();
	spinlock_release(&screen.lock);
}


void vga_putc(char ch)	{
	uint16_t write = (uint16_t)ch + ((uint16_t)screen.color << 8);

	// Backspace
	if(ch == 0x09 && screen.x > 0)	{
		write_blank(screen.x, screen.y);
		screen.x--;
	}
	// tab
	else if(ch == 0x09)	{
		screen.x += (screen.tab_sz-((screen.x+screen.tab_sz)%screen.tab_sz));
	}

	else if(ch == '\r')	{
		screen.x = 0;
	}
	else if(ch == '\n')	{
		screen.x = 0;
		screen.y++;
		if(screen.y >= (SCREEN_HEIGHT))	{
			vga_scroll();
		}
	}
	else if(ch >= ' ' && ch <= '~')	{
		vga_write_mem(screen.x, screen.y, write);
		screen.x++;
	}
	else	{
		// Don't know how to handle this yet
		return;
	}

	if(screen.x >= SCREEN_LENGTH)	{
		screen.x = 0;
		screen.y++;
		if(screen.y >= (SCREEN_HEIGHT))	{
			vga_scroll();
		}
	}
	vga_update_cursor();
}

void vga_write(char* str)	{
	int i = 0;
	while(str[i] != 0x00)	{
		vga_putc(str[i]);
		i++;
	}
}

void vga_clear()	{
	uint16_t fill = ((uint16_t)screen.color << 8) + (uint16_t)' ';
	int i;
	for(i = 0; i < SCREEN_LENGTH * SCREEN_HEIGHT; i++)	{
		screen.mem[i] = fill;
	}
}



static inline void vga_write_mem(uint8_t x, uint8_t y, uint16_t val)	{
	screen.mem[(y*SCREEN_LENGTH) + x] = val;
}


static void vga_scroll()	{
	if(screen.y >= SCREEN_HEIGHT)	{
		int i;
		for(i = 0; i < (SCREEN_HEIGHT-1)*SCREEN_LENGTH; i++)	{
			screen.mem[i] = screen.mem[i+SCREEN_LENGTH];
		}
		for(i = (SCREEN_HEIGHT-1)*SCREEN_LENGTH; i <
			SCREEN_HEIGHT*SCREEN_LENGTH; i++)	{
			screen.mem[i] = (uint16_t)' ' + ((uint16_t)screen.color << 8);
		}
		screen.y -= 1;
	}
}

static void vga_update_cursor()	{
	uint16_t loc = screen.y * SCREEN_LENGTH + screen.x;
	outb(0x3D4, 14);
	outb(0x3D5, loc >> 8);
	outb(0x3D4, 15);
	outb(0x3D5, loc);
}


/** @}*/	// VGA_drv
