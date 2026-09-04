#ifndef HEAP_H
#define HEAP_H

#include "types.h"

void heap_initialize(void);

void *kmalloc(uint32_t size);
void kfree(void *ptr);

#endif // HEAP_H