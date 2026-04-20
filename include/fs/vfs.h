#ifndef VFS_H
#define VFS_H

#include <stddef.h>

// int bdev_read_t(void* buf, size_t seek, size_t len)
typedef int (*bdev_read_t)(void*, size_t, size_t);

#endif