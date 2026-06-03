#include <kernel/dllist.h>
#include <kernel/kmalloc.h>

void dllist_init(dllist_t* list) {
    list = kmalloc(sizeof(dllist_t), 0);
    list->next = NULL;
    list->prev = NULL;
}

dllist_t* dllist_append(dllist_t* list, void* data) {
    dllist_t* entry = kmalloc(sizeof(dllist_t), 0);
    entry->data = data;
    entry->next = list->next;
    entry->prev = list;
    if(list->next) list->next->prev = entry;
    list->next = entry;
    return entry;
}

dllist_t* dllist_prepend(dllist_t* list, void* data) {
    dllist_t* entry = kmalloc(sizeof(dllist_t), 0);
    entry->data = data;
    entry->next = list;
    entry->prev = list->prev;
    if(list->prev) list->prev->next = entry;
    list->prev = entry;
    return entry;
}

dllist_t* dllist_remove(dllist_t* list, bool shouldFree) {
    if(list->next)
        list->next->prev = list->prev;
    if(list->prev)
        list->prev->next = list->next;
    
    dllist_t* ret;
    if(list->next)
        ret = list->next;
    else if(list->prev)
        ret = list->prev;
    else ret = NULL;

    if(shouldFree) kfree(list->data);
    kfree(list);
    return ret;
}

dllist_t* dllist_prev(dllist_t* list) {
    return list->prev;
}

dllist_t* dllist_next(dllist_t* list) {
    return list->next;
}

void* dllist_get(dllist_t* list) {
    return list->data;
}