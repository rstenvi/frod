/**
* \file stdio.h
*/

#ifndef __STDIO_H
#define __STDIO_H

#include <stdarg.h>


/**
* Printf message levels. Which ones are printed is controlled by the compiler, a
* debug level of 0 will print everything, a level of 2 is meant to be normal
* printing level.
*/
enum KM_Level	{
	/** Print the output to bochs console. */
	K_BOCHS_OUT = 0,
	
	/** Debug messages that should be printed when compiled in debug mode. */
	K_DEBUG = 1,

	/** Normal info about what is going on. */
	K_LOW_INFO = 2,
	
	/** Only the most important information. */
	K_HIGH_INFO = 3,
	
	/** Warning messages. */
	K_WARNING = 4,
	
	/** Error messages, if it causes some failure. */
	K_ERROR = 5,

	/** Info that the user should read or respond to. */
	K_USER = 6,
	
	/** Unrecoverable error. */
	K_FATAL = 7
};




int vsprintf(char *buf, const char *fmt, va_list args);

int sprintf(char *buf, const char *fmt, ...);

int vprintf(const char *fmt, va_list args);

int printf(const char *fmt, ...);

int kprintf(enum KM_Level kl, const char *fmt, ...);

#endif
