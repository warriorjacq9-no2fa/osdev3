#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stddef_.h>
#include <stdint.h>

#define EFAIL       1
#define ENOENT      2
#define E2BIG       3
#define EMEM        4
#define EEXIST      5
#define EISDIR      6

#define O_RDONLY    0x0001
#define O_WRONLY    0x0002
#define O_RDWR      0x0003

#define O_CLOEXEC   0x0004
#define O_CREAT     0x0008
#define O_DIRECTORY 0x0010
#define O_EXCL      0x0020
#define O_NOFOLLOW  0x0040
#define O_TMPFILE   0x0080
#define O_TRUNC     0x0100
#define O_APPEND    0x0200

#define S_IRUSR     0x0100
#define S_IWUSR     0x0080
#define S_IXUSR     0x0040
#define S_IRWXU     (S_IRUSR | S_IWUSR | S_IXUSR)

#define S_IRGRP     0x0020
#define S_IWGRP     0x0010
#define S_IXGRP     0x0008
#define S_IRWXG     (S_IRGRP | S_IWGRP | S_IXGRP)

#define S_IROTH     0x0004
#define S_IWOTH     0x0002
#define S_IXOTH     0x0001
#define S_IRWXO     (S_IROTH | S_IWOTH | S_IXOTH)

#define S_ISSOCK(n) (n & 0xC000)
#define S_ISLNK(n)  (n & 0xA000)
#define S_ISREG(n)  (n & 0x8000)
#define S_ISBLK(n)  (n & 0x6000)
#define S_ISDIR(n)  (n & 0x4000)
#define S_ISCHR(n)  (n & 0x2000)
#define S_ISFIFO(n) (n & 0x1000)


typedef struct vnode vnode_t;
typedef struct stat stat_t;
typedef struct vnode_ops vops_t;

// Collection of driver-implemented functions
struct vnode_ops {
    // int open(vnode_t* node, const char* filename, int flags, [int mode])
    int (*open)(vnode_t*, const char*, int, ...);
    // int close(vnode_t* node)
    int (*close)(vnode_t*);
    // ssize_t read(vnode_t* node, void* buf, size_t off, size_t len)
    ssize_t (*read)(vnode_t*, void*, size_t, size_t);
    // ssize_t write(vnode_t* node, void* buf, size_t off, size_t len)
    ssize_t (*write)(vnode_t*, void*, size_t, size_t);
    // int stat(const char* filename, stat_t* buf)
    int (*stat)(const char*, stat_t*);
    // int fstat(vnode_t* node, stat_t* buf)
    int (*fstat)(vnode_t*, stat_t*);
};

struct vnode {
    vops_t *ops;
    int flags, mode;
    void* private;
};

struct stat {
    size_t st_ino;
    int st_mode;
    size_t st_nlink;
    uint32_t st_uid, st_gid;
    size_t st_size;
    size_t st_blksize;
    size_t st_blocks;
    int32_t st_atime, st_mtime, st_ctime;
};

#endif