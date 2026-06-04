#ifndef DLLIST_H
#define DLLIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct doubly_linked_list {
    void* data; // Must be a kmalloc'd pointer
    struct doubly_linked_list* next;
    struct doubly_linked_list* prev;
} dllist_t;

dllist_t* dllist_create();
void dllist_append(dllist_t* list, void* data);
dllist_t* dllist_prepend(dllist_t* list, void* data);
void dllist_remove(dllist_t* list, size_t idx, bool shouldFree);
void* dllist_get(dllist_t* list, size_t idx);
size_t dllist_len(dllist_t* list);

#endif