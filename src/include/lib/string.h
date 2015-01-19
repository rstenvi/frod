#ifndef ___STRING_H
#define ___STRING_H

#include <stddef.h>
#include <stdint.h>

/**
* \file string.h
* Attempts to mimic the functionality in string.h library in glibc
* \todo Implement all functions.
* \addtogroup c_string
* @{
*/

/**
* Copies "num" bytes from src to dst.
* \param[out] dst Where data is copied to
* \param[in] src Where data is copied from
* \param[in] num Number of bytes to copy
* \returns Returns the address of dst.
* \remark If dst or src is NULL, dst is returned untouched.
*/
void* memcpy(void* dst, const void* src, size_t num);

/**
* As of now, this is just a wrapper to memcpy, nothing has been optimized, so
* this is ok.
* \param[out] dst Where data is placed.
* \param[in] src Where data is copied from
* \param[in] num Number of bytes to copy
* \returns Returns the address of dst.
* \remark If dst or src is NULL, dst is returned untouched.
*/
void* memmove(void* dst, const void* src, size_t num);

/**
* Copy a string from src to dst.
* \param[out] dst Where data is copied to.
* \param[in] src Where data is copied from.
* \returns Returns the address of dst.
* \remark If dst or src is NULL, dst is returned untouched.
*/
char* strcpy(char* dst, const char* src);

/**
* Copy num characters from src to dst
* \param[out] dst Where characters are copied to
* \param[in] src Where characters are copied from
* \param[in] num Max number of characters top copy, will stop at 0x00 if that
* comes before num characters have been reached.
* \returns Returns the address of dst.
* \remark If dst or src is NULL, dst is returned untouched.
* \remark The string is always null-terminated, unlike the equivalent function in
* the C-library.
*/
char* strncpy(char* dst, const char* src, size_t num);

/**
* Adds the string from src onto the end of dst.
* \param[in,out] dst 
* \param[in] src
* \returns Returns the address of dst.
* \remark If dst or src is NULL, dst is returned untouched.
*/
char* strcat(char* dst, const char* src);

/**
* 
* \param[in,out] dst
* \param[in] src
* \param[in] num
* \returns Returns the address of dst.
* \remark If dst or src is NULL, dst is returned untouched.
*/
char* strncat(char* dst, const char* src, size_t num);

/**
* 
* \param[in] ptr2
* \param[in] ptr2
* \param[in] num
* \returns 
* \remark If ptr1 or src is NULL, 0xF00(256) is returned. That is the only way
* that value can be returned.
*/
int memcmp(const void* ptr1, const void* ptr2, size_t num);

/**
* 
* \param[in] str1
* \param[in] str2
* \returns 
* \remark If ptr1 or src is NULL, 0xF00(256) is returned. That is the only way
* that value can be returned.
*/
int strcmp(const char* str1, const char* str2);

/**
* 
* \param[in] str1
* \param[in] str2
* \param[in] num
* \returns 
* \remark If ptr1 or src is NULL, 0xF00(256) is returned. That is the only way
* that value can be returned.
*/
int strncmp(const char* str1, const char* str2, size_t num);

/**
* Sets all the bytes of a memory region to a certain value.
* \param[in,out] ptr The starting address of the memory region.
* \param[in] value The value to be placed.
* \param[in] num Number of bytes to set, starting from ptr.
* \return The same address as ptr.
* \remark If ptr is NULL, NULL is returned and nothing is done.
*/
void* memset(void* ptr, int8_t value, size_t num);

/**
* Calculates the number of characters before hitting a null-byte.
* \param[in] str A null-terminated string.
* \remark if str is NULL, 0 is returned
* \returns The number of characters in the string.
*/
size_t strlen(const char* str);




// Stuff not implemented, but part of the string library
/*
void* memchr(void* ptr, int8_t value, size_t num);
char* strchr(const char* str, int8_t ch);
size_t strcspn(const char* str1, const char* str2);
char* strpbrk(const char* str1, const char* str2);
char* strrchr(const char* str, int8_t ch);
size_t strspn(const char* str1, const char* str2);
char* strstr(const char* str1, const char* str2);
*/

/** @} */

#endif
