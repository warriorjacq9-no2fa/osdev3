#ifndef DLLIST_H
#define DLLIST_H

#include <stdbool.h>

typedef struct doubly_linked_list {
    void* data; // Must be a kmalloc'd pointer
    struct doubly_linked_list* next;
    struct doubly_linked_list* prev;
} dllist_t;

void dllist_init(dllist_t* list);
dllist_t* dllist_append(dllist_t* list, void* data);
dllist_t* dllist_prepend(dllist_t* list, void* data);
dllist_t* dllist_remove(dllist_t* list, bool shouldFree);
dllist_t* dllist_prev(dllist_t* list);
dllist_t* dllist_next(dllist_t* list);
void* dllist_get(dllist_t* list);

#endif