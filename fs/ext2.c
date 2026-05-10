#include <fs/ext2.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static ext2_sb_t *sb;
static ext2_sb_ext_t *ext_sb;
static ext2_bgdesc_t *bgdesc_table;
static size_t vol_start;
static bdev_read_t read;

static vops_t ops;

#define BLOCK_SIZE (1024 << sb->log_block_size)

ext2_inode_t* get_fp(char* filepath);

static inline size_t block_offset(size_t block) {
    return vol_start + block * BLOCK_SIZE;
}

ext2_inode_t* get_inode(size_t in) {
    size_t bg = (in - 1) / sb->inodes_per_group;
    size_t l_id = (in - 1) % sb->inodes_per_group;

    size_t inode_table = bgdesc_table[bg].inode_table;

    ext2_inode_t* inode = kmalloc(sizeof(ext2_inode_t), 0);

    size_t inode_size = (sb->rev_level >= 1) ? ext_sb->inode_size : sizeof(ext2_inode_t);

    size_t off = block_offset(inode_table) + (l_id * inode_size);

    if(read((void*)inode, off, inode_size)) return NULL;
    return inode;
}

// TODO: better inode_get_data
void* inode_get_data(ext2_inode_t* inode) {
    void* buf = kmalloc(inode->blocks * BLOCK_SIZE, 0);
    for(int i = 0; i < 12 && inode->block[i]; i++) {
        if(read(buf + i * BLOCK_SIZE, block_offset(inode->block[i]), BLOCK_SIZE)) return NULL;
    }
    return buf;
}

ext2_inode_t* lookup_inode(ext2_inode_t* i_dir, char* name) {
    if(i_dir == NULL) return NULL;
    ext2_dir_entry_t* dir = (ext2_dir_entry_t*) inode_get_data(i_dir);
    ext2_dir_entry_t* d = dir;
    while(d->inode) {
        char* d_name = kmalloc(d->name_len, 0);
        strncpy(d_name, d->name, d->name_len);
        if(strcmp(d_name, name) == 0) {
            ext2_inode_t* res = get_inode(d->inode);
            kfree(dir);
            return res;
        } else {
            d = (ext2_dir_entry_t*)((uint8_t*)d + d->rec_len);
            if((uint8_t*)d >= (uint8_t*)dir + i_dir->r0_size) {
                return NULL;
            }
        }
    }
    return NULL;
}

ext2_inode_t* get_fp(char* filepath) {
    char* path = strdup(filepath);
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

int ext2_open(vnode_t* node, const char* filename, int flags, ...) {
    ext2_inode_t* inode = get_fp(filename);

    const bool create = (flags & (O_CREAT | O_TMPFILE)) != 0;

    // File does not exist
    if (inode == NULL) {
        if (!create)
            return -1;

        // TODO: writing
    } else if (flags & O_EXCL) {
        return -1;
    }

    node->flags = flags;
    node->private = inode;
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
    if(node->private) kfree(node->private); // TODO: ext2_free
    return 0;
}

int ext2_init(bdev_read_t _read, size_t _vol_start) {
    vol_start = _vol_start;
    read = _read;
    sb = kmalloc(sizeof(ext2_sb_t), 0);
    if(read((void*)sb, vol_start + 1024, sizeof(ext2_sb_t))) return 1;
    if(sb->magic != EXT2_SUPER_MAGIC) {
        kprintf(LOG_ERR, "ext2", "Invalid magic\r\n");
        return 1;
    }
    if(sb->rev_level >= 1) {
        kprintf(LOG_INFO, "ext2", "Extended superblock is present\r\n");
        ext_sb = kmalloc(sizeof(ext2_sb_ext_t), 0);
        if(read((void*)ext_sb, vol_start + 1024 + sizeof(ext2_sb_t), sizeof(ext2_sb_ext_t)))
            return 1;
    }
    kprintf(
        LOG_INFO, "ext2", "%u blocks total (%u remaining), size is %u\r\n",
        sb->blocks_count, sb->free_blocks_count, 1024 << sb->log_block_size
    );
    kprintf(
        LOG_INFO, "ext2", "%u blocks per group, %u inodes per group, starting block at %u\r\n",
        sb->blocks_per_group, sb->inodes_per_group, sb->first_data_block
    );

    size_t bgdt_block = (sb->log_block_size == 0) ? 2 : 1;
    size_t bgdt_size = sizeof(ext2_bgdesc_t) * ((sb->blocks_count + sb->blocks_per_group) / sb->blocks_per_group);
    
    bgdesc_table = kmalloc(bgdt_size, 0);
    if(read((void*)bgdesc_table, block_offset(bgdt_block), bgdt_size))
        return 1;
    return 0;
}
