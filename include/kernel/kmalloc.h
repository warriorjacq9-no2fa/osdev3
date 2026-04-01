#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>

#define MEM_PRESENT 0x01
#define MEM_RW      0x02
#define MEM_USER    0x04

void kheap_init();
void* kmalloc(size_t size, char flags);
int __kmem_is_user(void* addr);
void kfree(void* addr);

#endif