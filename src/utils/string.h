#ifndef STRING_H
#define STRING_H

typedef unsigned int size_t;

int strcmp(const char *a, const char *b);
size_t strlen(const char *str);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int value, size_t n);

int memcmp(const void *a, const void *b, size_t n);

void itoa(unsigned int value, char *buffer);
void itoa_hex(unsigned int value, char *buffer);

#endif