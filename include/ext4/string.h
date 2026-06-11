#ifndef _EXT4_STRING_H
#define _EXT4_STRING_H

#include "types.h"

void *memset(void *s, int c, uint n);
void *memcpy(void *dest, const void *src, uint n);
void *memmove(void *dest, const void *src, uint n);
char *strcpy(char *s1, const char *s2);
char *strncpy(char *s, const char *t, uint n);
int strcmp(const char *s1, const char *s2);
uint strlen(const char *str);
int strncmp(const char *p, const char *q, uint n);

#endif
