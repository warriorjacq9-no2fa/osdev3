#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>

void kheap_init();
void* kmalloc(size_t size);

#endif