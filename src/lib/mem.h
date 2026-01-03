#ifndef MEM_H
#define MEM_H
#include <stdint.h>

#define NULL ((void *)0)

void init_mem();
void *malloc(uint32_t size);
void free(void *ptr);
void mem_dump();

#endif