#include <fs/ext2.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

typedef struct ext2_private {
    ext2_inode_t* inode;
    size_t i_num;
} ext2_priv_t;

static ext2_sb_t *sb;
static ext2_sb_ext_t *ext_sb;
static ext2_bgdesc_t *bgdesc_table;
static size_t vol_start;
static bdev_read_t read;

static vops_t ops;

static size_t block_size;

ext2_inode_t* get_fp(const char* filepath);

static inline size_t block_offset(size_t block) {
    return vol_start + block * block_size;
}

size_t get_block_iter(size_t log_block, size_t bnum, size_t iter) {
    if(bnum == 0 || iter == 0) return 0;
    const size_t pointers = block_size / sizeof(uint32_t);
    uint32_t* ptrs = kmalloc(block_size, 0);
    if(ptrs == NULL) return 0;
    if(read((void*)ptrs, block_offset(bnum), block_size)) {
        kfree(ptrs);
        return 0;
    }
    if(iter == 1) {
        if(log_block >= pointers) {
            kfree(ptrs);
            return 0;
        }
        size_t ptr = ptrs[log_block];
        kfree(ptrs);
        return ptr;
    } else {
        size_t div = 1;
        for(size_t i = 1; i < iter; i++) div *= pointers;
        if(log_block / div >= pointers) {
            kfree(ptrs);
            return 0;
        }
        size_t ptr = ptrs[log_block / div];
        kfree(ptrs);
        return get_block_iter(log_block % div, ptr, iter - 1);
    }
}

size_t get_block(ext2_inode_t* inode, size_t log_block) {
    const size_t pointers = block_size / sizeof(uint32_t);

    // Direct blocks
    if(log_block < 12)
        return inode->block[log_block];

    log_block -= 12;

    // Singly indirect
    if(log_block < pointers)
        return get_block_iter(log_block, inode->block[12], 1);

    log_block -= pointers;

    // Doubly indirect
    if(log_block < pointers * pointers)
        return get_block_iter(log_block, inode->block[13], 2);

    log_block -= pointers * pointers;

    // Triply indirect
    if(log_block < pointers * pointers * pointers)
        return get_block_iter(log_block, inode->block[14], 3);

    return 0;
}

ext2_inode_t* get_inode(size_t in) {
    if(in == 0) return NULL;
    size_t bg = (in - 1) / sb->inodes_per_group;
    size_t l_id = (in - 1) % sb->inodes_per_group;

    size_t inode_table = bgdesc_table[bg].inode_table;
    size_t inode_size = (sb->rev_level >= 1) ? ext_sb->inode_size : sizeof(ext2_inode_t);

    ext2_inode_t* inode = kmalloc(inode_size, 0);

    size_t off = block_offset(inode_table) + (l_id * inode_size);

    if(read((void*)inode, off, inode_size)) {
        kfree(inode);
        return NULL;
    }
    return inode;
}

ssize_t ext2_read_inode(ext2_inode_t* inode, void* buf, size_t off, size_t len) {
    if(off >= inode->r0_size) return 0;
    if(len > inode->r0_size - off) len = inode->r0_size - off;

    size_t b_off = off / block_size;
    size_t r_off = off % block_size;

    if (len > SIZE_MAX - r_off - (block_size - 1)) return -E2BIG;
    size_t b_len = (len + r_off + block_size - 1) / block_size;

    void* b_buf = kmalloc(b_len * block_size, 0);    
    if(b_buf == NULL) return -EMEM;

    for(size_t i = 0; i < b_len; i++) {
        size_t block;
        if((block = get_block(inode, i + b_off)) == 0) {
            memset(b_buf + i * block_size, 0, block_size);
            continue;
        }
        if(read(b_buf + i * block_size, block_offset(block), block_size)) {
            len = i < 1 ? 0 : min(i * block_size - r_off, len);
            break;
        }
    }
    memcpy(buf, b_buf + r_off, len);
    kfree(b_buf);
    return (ssize_t)len;
}

ext2_inode_t* lookup_inode(ext2_inode_t* i_dir, const char* name) {
    if(i_dir == NULL) return NULL;

    uint8_t* block_buf = kmalloc(block_size, 0);
    if(block_buf == NULL) return NULL;

    size_t name_len = strlen(name);
    size_t offset = 0;

    while(offset < i_dir->r0_size) {
        // Read one block at a time
        size_t to_read = min(block_size, i_dir->r0_size - offset);
        ssize_t got = ext2_read_inode(i_dir, block_buf, offset, to_read);
        if(got <= 0) break;

        uint8_t* ptr = block_buf;
        uint8_t* end = block_buf + got;

        while(ptr < end) {
            ext2_dir_entry_t* d = (ext2_dir_entry_t*)ptr;

            if(d->rec_len == 0) {
                kprintf(LOG_WARN, "ext2", "Invalid entry\r\n");
                break;
            }

            if(d->inode != 0 &&
                d->name_len == name_len &&
                memcmp(d->name, name, name_len) == 0)
            {
                ext2_inode_t* res = get_inode(d->inode);
                kfree(block_buf);
                return res;
            }

            ptr += d->rec_len;
        }

        offset += got;
    }

    kfree(block_buf);
    return NULL;
}

ext2_inode_t* get_fp(const char* filepath) {
    char* path = strdup((char*)filepath);
    ext2_inode_t* i = get_inode(EXT2_ROOT_INO);
    ext2_inode_t* i_next;
    char* save;
    char* tok = strtok_r(path, "/", &save);
    while(tok != NULL && i != NULL) {
        char* next = strtok_r(NULL, "/", &save);
        i_next = lookup_inode(i, tok);
        kfree(i);
        i = i_next;
        tok = next;
    }

    kfree(path);
    return i;
}

size_t lookup_inode_num(ext2_inode_t* i_dir, const char* name) {
    if(i_dir == NULL) return 0;

    uint8_t* block_buf = kmalloc(block_size, 0);
    if(block_buf == NULL) return 0;

    size_t name_len = strlen(name);
    size_t offset = 0;

    while(offset < i_dir->r0_size) {
        // Read one block at a time
        size_t to_read = min(block_size, i_dir->r0_size - offset);
        ssize_t got = ext2_read_inode(i_dir, block_buf, offset, to_read);
        if(got <= 0) break;

        uint8_t* ptr = block_buf;
        uint8_t* end = block_buf + got;

        while(ptr < end) {
            ext2_dir_entry_t* d = (ext2_dir_entry_t*)ptr;

            if(d->rec_len == 0) {
                kprintf(LOG_WARN, "ext2", "Invalid entry\r\n");
                break;
            }

            if(d->inode != 0 &&
                d->name_len == name_len &&
                memcmp(d->name, name, name_len) == 0)
            {
                kfree(block_buf);
                return d->inode;
            }

            ptr += d->rec_len;
        }

        offset += got;
    }
    kfree(block_buf);
    return 0;
}

size_t get_fp_num(const char* filepath) {
    char* path = strdup((char*)filepath);
    size_t i = EXT2_ROOT_INO;
    size_t i_next;
    char* save;
    char* tok = strtok_r(path, "/", &save);
    while(tok != NULL && i != 0) {
        char* next = strtok_r(NULL, "/", &save);
        ext2_inode_t* in = get_inode(i);
        i_next = lookup_inode_num(in, tok);
        kfree(in);
        i = i_next;
        tok = next;
    }

    kfree(path);
    return i;
}

int ext2_open(vnode_t* node, const char* filename, int flags, ...) {
    size_t i_num = get_fp_num(filename);
    ext2_inode_t* inode = get_inode(i_num);

    const bool create = (flags & (O_CREAT | O_TMPFILE)) != 0;

    // File does not exist
    if (inode == NULL) {
        if (!create)
            return -ENOENT;

        // TODO: writing
    } else if (flags & O_EXCL) {
        return -EEXIST;
    }

    if((inode->mode & EXT2_S_IFDIR) && (flags & O_WRONLY)) return -EISDIR;

    ext2_priv_t* private = kmalloc(sizeof(ext2_priv_t), 0);
    private->inode = inode;
    private->i_num = i_num;

    node->flags = flags;
    node->private = private;
    node->ops = &ops;

    if (create) {
        va_list ap;
        va_start(ap, flags);
        node->mode = va_arg(ap, int);
        va_end(ap);
    } else {
        node->mode = inode->mode;
    }

    return 0;
}

int ext2_close(vnode_t* node) {
    if(node->private) { // TODO: ext2_free
        ext2_priv_t* private = (ext2_priv_t*)node->private;
        kfree(private->inode);
        kfree(private);
    }
    return 0;
}

ssize_t ext2_read(vnode_t* node, void* buf, size_t off, size_t len) {
    if(len == 0) return 0;
    return ext2_read_inode(((ext2_priv_t*)node->private)->inode, buf, off, len);
}

int ext2_stat(const char* filename, stat_t* buf) {
    size_t in = get_fp_num(filename);
    if(in == 0) return -ENOENT;
    ext2_inode_t* inode = get_inode(in);
    buf->st_ino = in;
    buf->st_mode = inode->mode;
    buf->st_nlink = inode->links_count;
    buf->st_uid = inode->uid;
    buf->st_gid = inode->gid;
    buf->st_size = inode->r0_size;
    buf->st_blksize = block_size;
    buf->st_blocks = inode->blocks;
    buf->st_atime = inode->atime;
    buf->st_mtime = inode->mtime;
    buf->st_ctime = inode->ctime;
    kfree(inode);
    return 0;
}

int ext2_fstat(vnode_t* node, stat_t* buf) {
    ext2_priv_t* private = (ext2_priv_t*)node->private;
    ext2_inode_t* inode = private->inode;
    buf->st_ino = private->i_num;
    buf->st_mode = inode->mode;
    buf->st_nlink = inode->links_count;
    buf->st_uid = inode->uid;
    buf->st_gid = inode->gid;
    buf->st_size = inode->r0_size;
    buf->st_blksize = block_size;
    buf->st_blocks = inode->blocks;
    buf->st_atime = inode->atime;
    buf->st_mtime = inode->mtime;
    buf->st_ctime = inode->ctime;
    return 0;
}

int ext2_init(bdev_read_t _read, size_t _vol_start) {
    vol_start = _vol_start;
    read = _read;
    sb = kmalloc(sizeof(ext2_sb_t), 0);
    if(read((void*)sb, vol_start + 1024, sizeof(ext2_sb_t))) return -1;
    if(sb->magic != EXT2_SUPER_MAGIC) {
        kprintf(LOG_ERR, "ext2", "Invalid magic\r\n");
        return -1;
    }
    if(sb->rev_level >= 1) {
        kprintf(LOG_INFO, "ext2", "Extended superblock is present\r\n");
        ext_sb = kmalloc(sizeof(ext2_sb_ext_t), 0);
        if(read((void*)ext_sb, vol_start + 1024 + sizeof(ext2_sb_t), sizeof(ext2_sb_ext_t)))
            return -1;
    }
    block_size  = (1024 << sb->log_block_size);
    kprintf(
        LOG_INFO, "ext2", "%u blocks total (%u remaining), size is %u\r\n",
        sb->blocks_count, sb->free_blocks_count, block_size
    );
    kprintf(
        LOG_INFO, "ext2", "%u blocks per group, %u inodes per group, starting block at %u\r\n",
        sb->blocks_per_group, sb->inodes_per_group, sb->first_data_block
    );

    size_t bgdt_block = (sb->log_block_size == 0) ? 2 : 1;
    size_t bgdt_size = sizeof(ext2_bgdesc_t) * ((sb->blocks_count + sb->blocks_per_group - 1) / sb->blocks_per_group);
    
    bgdesc_table = kmalloc(bgdt_size, 0);
    if(read((void*)bgdesc_table, block_offset(bgdt_block), bgdt_size))
        return -1;
    
    ops.open = ext2_open;
    ops.close = ext2_close;
    ops.read = ext2_read;
    ops.stat = ext2_stat;
    ops.fstat = ext2_fstat;
    return 0;
}
