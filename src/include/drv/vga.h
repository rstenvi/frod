/**
* \addtogroup VGA_drv
* @{
* \file vga.h
* Header file and public API for the VGA driver.
*/

#ifndef __VGA_H
#define __VGA_H

#include "../sys/kernel.h"
#include "../sys/lock.h"

/**
* Different colors for foreground and background color.
*/
typedef enum	{
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Pink = 5,
	Brown = 6,
	LightGray = 7,
	DarkGray = 8,
	LightBlue = 9,
	LightGreen = 10, 
	LightCyan = 11, 
	LightRed = 12, 
	LightPink = 13, 
	Yellow = 14, 
	White = 15
} vga_color;


/**
* Structure defining the entire screen for the VGA driver.
*/
typedef struct {
	/** Byte for combined foreground and background color. */
	uint8_t color;
	uint8_t x,	/**< Current horizontal index. */
			y;	/**< Current vertical index. */
	uint8_t tab_sz;	/**< Number of spaces in a tab. */
	uint16_t* mem;	/**< Address to the VGA memory. */

	/** Lock to control who has access to the CPU. */
	spinlock lock;
} vga_screen;




/**
* Initializes the screen. This includes setting all the variables in the structure
* vga_screen and painting the entire screen with the color of bg.
* \param[in] fg Foreground color for future text.
* \param[in] bg Color of screen after this and background color with future
* writes.
*/
void vga_init(vga_color fg, vga_color bg);

/**
* Place one character on the screen. This also handle \\n, \\r etc.
* \param[in] ch The character that should be printed.
*/
void vga_putc(char ch);


/**
* Print a string to the screen, wrapper to vga_putc.
* \param[in] str Null-terminated string.
*/
void vga_write(char* str);


/**
* Clear the entire screen with the bacground color that has been set.
*/
void vga_clear();


/** @} */	// VGA_drv
#endif
