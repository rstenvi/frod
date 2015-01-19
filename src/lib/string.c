/**
* \file lib/string.c
* Implementation of functions from the string.h library.
*/

#include "string.h"
#include "stdio.h"
#include "../sys/kernel.h"

#define STRING_NULL_ERROR 0xF00


void* memcpy(void* dst, const void* src, size_t num)	{
	if(dst == NULL || src == NULL)	return dst;
	size_t i = 0;
	uint8_t* d = (uint8_t*)dst;
	uint8_t* s = (uint8_t*)src;
	for(; i < num; i++)	{
		d[i] = s[i];
	}
	return dst;
}

void* memmove(void* dst, const void* src, size_t num)	{
	// Not sure if this will cause any problems, see:
	// http://stackoverflow.com/questions/4415910/memcpy-vs-memmove
	return memcpy(dst, src, num);
}

char* strcpy(char* dst, const char* src)	{
	if(dst == NULL || src == NULL)	return dst;
	uint32_t i = 0;
	for(; src[i] != 0x00; i++)
		dst[i] = src[i];
	dst[i] = src[i];
	return dst;
}

char* strncpy(char* dst, const char* src, size_t num)	{
	if(dst == NULL || src == NULL)	return dst;
	uint32_t i = 0;
	for(i = 0; i < num && src[i] != 0x00; i++)
		dst[i] = src[i];
	// If we reach a null-byte before num, we pad with null-bytes
	while(i < num)	dst[i++] = 0x00;
	dst[num-1] = 0x00;	// Always null-terminate the string
	return dst;
}

char* strcat(char* dst, const char* src)	{
	if(dst == NULL || src == NULL)	return dst;
	uint32_t i = 0;
	while(dst[i] != 0x00)	i++;
	strcpy(&dst[i], src);
	return dst;
}

char* strncat(char* dst, const char* src, size_t num)	{
	if(dst == NULL || src == NULL)	return dst;
	size_t i = 0;
	while(dst[i] != 0x00)	i++;
	size_t j = 0;
	for(; j < num && src[j] != 0x00; j++)	{
		dst[i+j] = src[j];
	}
	dst[i+j] = 0x00;
	return dst;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num)	{
	if(ptr1 == NULL || ptr2 == NULL)	return 0xF00;
	uint8_t* p1 = (uint8_t*)ptr1;
	uint8_t* p2 = (uint8_t*)ptr2;
	size_t i = 0;
	for(; i < num; i++)	{
		if(p1[i] != p2[i])	{
			return (int)p1[i]-p2[i];
		}
	}
	return 0;
}


int strcmp(const char* str1, const char* str2)	{
	if(str1 == NULL || str2 == NULL)	return 0xF00;
	size_t i = 0;
	while( (str1[i] == str2[i]) && (str1[i] != 0x00) )	i++;
	return (int)(str1[i] - str2[i]);
}

int strncmp(const char* str1, const char* str2, size_t num)	{
	if(str1 == NULL || str2 == NULL)	return 0xF00;
	if(num == 0)	return 0;
	size_t i = 0;
	for(i = 0; i < num && str1[i] != 0x00 && str1[i] == str2[i]; i++);
	if(i >= num)	return 0;
	else				return (int)(str1[i]-str2[i]);
}


char* strchr(const char* str, int8_t ch)	{
	size_t i = 0;
	while(str[i] != 0x00 && (int8_t)str[i] != ch)	{
		i++;
	}
	if(str[i] == 0x00)	return NULL;
	return (char*)str;
}

void* memset(void* ptr, int8_t value, size_t num)	{
	if(ptr == NULL)	return ptr;
	size_t i = 0;
	uint8_t* p = (uint8_t*)ptr;
	for(; i < num; i++)
		p[i] = value;
	return ptr;
}

size_t strlen(const char* str)	{
	if(str == NULL)	return 0;
	size_t ret = 0;
	while(str[ret] != 0x00)	ret++;
	return ret;
}



//------ Testing code ------------------

#ifdef TEST_KERNEL

int string_test_cmp()	{
	char* str1 = "test1", * str2 = "test2", * str12 = "test1";
	#define TSTRLEN 5
	#define TDIFF   -1

	// Test that difference is correct
	if(strcmp(str1, str2) != TDIFF)		return 1;
	if(strcmp(str2, str1) != -(TDIFF))	return 2;

	// Handles NULL values
	if(strcmp(NULL, str1) != STRING_NULL_ERROR)		return 3;

	// Same pointer
	if(strcmp(str1, str1) != 0)			return 4;
	
	// Equal string
	if(strcmp(str1, str12) != 0)			return 5;

	// Move on to strncmp and memcmp

	// Unequal test
	if(strncmp(str1, str2, TSTRLEN) != TDIFF)	return 6;
	if(memcmp(str1, str2, TSTRLEN) != TDIFF)	return 7;

	// Equal test
	if(strncmp(str1, str2, TSTRLEN-1) != 0)	return 8;
	if(memcmp(str1, str2, TSTRLEN-1) != 0)		return 9;
	
	// NULL pointer
	if(strncmp(NULL, str2, TSTRLEN) != STRING_NULL_ERROR)		return 10;
	if(memcmp(str1, NULL, TSTRLEN) != STRING_NULL_ERROR)		return 11;


	return 0;
}

int string_test_len()	{
	char str1[10] = "test1";
	#define TSTRLEN 5

	if(strlen(NULL) != 0)	return 1;
	if(strlen(str1) != TSTRLEN)	return 2;

	str1[1] = 0x00;
	if(strlen(str1) != 1)	return 3;

	str1[0] = 0x00;
	if(strlen(str1) != 0)	return 4;
	return 0;
}

int string_test_set()	{
	char str1[10];
	int i;
	
	memset(str1, 'A', 10);
	if(memcmp(str1, "AAAAAAAAAA", 10) != 0)	return 1;

	memset(str1, 0x00, 10);
	for(i = 0; i < 10; i++)	{
		if(str1[i] != 0x00)	return 2;
	}

	if(memset(NULL, 0x00, 1024) != NULL)	return 3;


	return 0;
}

int string_test_cpy()	{
	char* str1 = "test1", str2[10];
	#define TSTRLEN 5

	strcpy(str2, str1);
	if(strcmp(str1, str2) != 0)	return 1;

	str2[0] = 0x00;
	memcpy(str2, str1, TSTRLEN);
	if(memcmp(str1, str2, TSTRLEN) != 0)	return 2;

	return 0;
}

bool string_run_all_tests()	{
	// Each test
	unit_test tests[5] = {
		string_test_cmp,
		string_test_len,
		string_test_set,
		string_test_cpy,
		NULL
	};

	int res = 0, count = 0;
	do	{
		if( (res = (*tests[count])()) != 0)	{
			kprintf(K_LOW_INFO, "Index: %i FAILED: %i\n", count, res);
			return false;
		}
		count++;
	} while(tests[count] != NULL);
	return true;
}



#endif
