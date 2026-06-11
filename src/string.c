#include "types.h"

void *memset(void *s, int c, uint n)
{
	uchar *p = (uchar *)s;
	uchar v = (uchar)c;

	for (uint i = 0; i < n; i++) {
		p[i] = v;
	}

	return s;
}

void *memcpy(void *dest, const void *src, uint n)
{
	uchar *d = (uchar *)dest;
	const uchar *s = (const uchar *)src;

	for (uint i = 0; i < n; ++i) {
		*d++ = *s++;
	}

	return dest;
}

void *memmove(void *dest, const void *src, uint n)
{
	uchar *d = (uchar *)dest;
	const uchar *s = (const uchar *)src;

	if (d == s)
		return dest;

	if (d < s) {
		for (uint i = 0; i < n; ++i) {
			*d++ = *s++;
		}
		return dest;
	}

	for (uint i = n; i > 0; --i) {
		d[i - 1] = s[i - 1];
	}
	return dest;
}

char *strcpy(char *s1, const char *s2)
{
	char *os = s1;

	while ((*s1++ = *s2++) != '\0')
		;
	return os;
}

char *strncpy(char *s, const char *t, uint n)
{
	char *os = s;

	while (n > 0 && (*s++ = *t++) != '\0')
		n--;
	while (n > 0) {
		*s++ = '\0';
		n--;
	}
	return os;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 != '\0' && *s1 == *s2) {
		s1++;
		s2++;
	}
	return (uchar)*s1 - (uchar)*s2;
}

uint strlen(const char *str)
{
	uint cnt = 0;
	while (*str++ != '\0')
		cnt++;
	return cnt;
}

int strncmp(const char *p, const char *q, uint n)
{
	while (n > 0 && *p && *p == *q)
		n--, p++, q++;
	if (n == 0)
		return 0;
	return (uchar)*p - (uchar)*q;
}
