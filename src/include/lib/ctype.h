/**
* \file ctype.h
* Macros for checking and manipulating single characters.
*/

#ifndef __CTYPE_H
#define __CTYPE_H

#define isalnum(c) ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9'))
#define isalpha(c) ((c>='A'&&c<='Z')||(c>='a'&&c<='z'))
#define iscntrl(c) ((c>=0x00 && c<=0x1f)||(c==0x7f))
#define isdigit(c) (c>='0'&&c<='9')
#define isgraph(c) (c>=0x21&&c<=0x7e)
#define islower(c) (c>='a'&&c<='z')
#define isprint(c) (c>=' '&&c<='~')
#define ispunct(c) ((c>=0x21&&c<=2f)||(c>=0x3a&&c<=40)||(c>=0x5b&&c<=60)||(c>=0x7b&&c<=0x7e))
#define isspace(c) ((c>=0x09&&c<=0x0d)||(c==0x20))
#define isupper(c) (c>='A'&&c<='Z')
#define isxdigit(c) ((c>=0&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f'))

#define isascii(c)	((unsigned)(c) <= 0x7F)
#define toascii(c)	((unsigned)(c) & 0x7F)
#define tolower(c)	(isupper(c) ? c + 'a' - 'A' : c)
#define toupper(c)	(islower(c) ? c + 'A' - 'a' : c)


#endif
