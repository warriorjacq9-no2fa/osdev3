#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>

// int bdev_read_t(void* buf, size_t seek, size_t len)
typedef int (*bdev_read_t)(void*, size_t, size_t);

typedef struct vfs_file file_t;

// Collection of driver-implemented functions
typedef struct vfs_driver {
    // int open(char* filename, file_t* file)
    int (*open)(char*, file_t*);
    // int close(file_t* file)
    int (*close)(file_t*);
    // int read(file_t* file, void* buf, size_t off, size_t len)
    int (*read)(file_t*, void*, size_t, size_t);
    // int write(file_t* file, void* buf, size_t off, size_t len)
    int (*write)(file_t*, void*, size_t, size_t);
} driver_t;

typedef struct vfs_file {
    driver_t *driver;
    uintptr_t driver_fp;
    uint8_t state;
} file_t;

#endif