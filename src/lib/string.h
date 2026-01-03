#ifndef STRING_H
#define STRING_H
#include <stdint.h>

int strlen(const char *str);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, uint32_t n);
void *memcpy(void *dest, const void *src, uint32_t n);

#endif
