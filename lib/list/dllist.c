#include <kernel/dllist.h>
#include <kernel/kmalloc.h>

dllist_t* dllist_create() {
    dllist_t* list = kmalloc(sizeof(dllist_t), 0);
    if(list) {
        list->next = NULL;
        list->prev = NULL;
    }
    return list;
}

void dllist_append(dllist_t* list, void* data) {
    dllist_t* entry = kmalloc(sizeof(dllist_t), 0);
    entry->data = data;
    entry->next = NULL;
    dllist_t* ptr = list;
    while(ptr->next) {
        ptr = ptr->next;
    }
    entry->prev = ptr;
    ptr->next = entry;
}

dllist_t* dllist_prepend(dllist_t* list, void* data) {
    dllist_t* entry = kmalloc(sizeof(dllist_t), 0);
    entry->data = data;
    entry->prev = NULL;
    dllist_t* ptr = list;
    while(ptr->prev) {
        ptr = ptr->prev;
    }
    entry->next = ptr;
    ptr->prev = entry;
    return entry;
}

void dllist_remove(dllist_t* list, size_t idx, bool shouldFree) {
    size_t cur = 0;
    idx += 1; // Account for the root list element
    dllist_t* ptr = list;
    while(cur < idx && ptr) {
        ptr = ptr->next;
        cur++;
    }

    ptr->prev->next = ptr->next;
    ptr->next->prev = ptr->prev;
    if(shouldFree) kfree(ptr->data);
    kfree(ptr);
}

void* dllist_get(dllist_t* list, size_t idx) {
    size_t cur = 0;
    idx += 1; // Account for the root list element
    dllist_t* ptr = list;
    while(cur < idx && ptr) {
        ptr = ptr->next;
        cur++;
    }
    if(ptr) return ptr->data;
    else return NULL;
}

size_t dllist_len(dllist_t* list) {
    size_t len = 0;
    dllist_t* cur = list->next;
    while(cur) {
        len++;
        cur = cur->next;
    }
    cur = list->prev;
    while(cur) {
        len++;
        cur = cur->prev;
    }
    return len;
}