#include <fs/ext2.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>
#include <string.h>
#include <stdio.h>

static ext2_sb_t *sb;
static ext2_sb_ext_t *ext_sb;
static ext2_bgdesc_t *bgdesc_table;
static size_t vol_start;
static bdev_read_t read;

#define BLOCK_SIZE (1024 << sb->log_block_size)

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

void* inode_get_data(ext2_inode_t* inode, size_t *size) {
    *size = inode->r0_size;
    void* buf = kmalloc(*size, 0);
    for(int i = 0; i < 12 && inode->block[i]; i++) {
        if(read(buf + i * BLOCK_SIZE, block_offset(inode->block[i]), BLOCK_SIZE)) return NULL;
    }
    return buf;
}

void mode_to_string(unsigned short mode, char str[11]) {
    // File type
    if (mode & EXT2_S_IFREG)  str[0] = '-';
    else if (mode & EXT2_S_IFDIR) str[0] = 'd';
    else if (mode & EXT2_S_IFLNK) str[0] = 'l';
    else if (mode & EXT2_S_IFCHR) str[0] = 'c';
    else if (mode & EXT2_S_IFBLK) str[0] = 'b';
    else if (mode & EXT2_S_IFIFO) str[0] = 'p';
    else if (mode & EXT2_S_IFSOCK) str[0] = 's';
    else str[0] = '?';

    // Owner
    str[1] = (mode & EXT2_S_IRUSR) ? 'r' : '-';
    str[2] = (mode & EXT2_S_IWUSR) ? 'w' : '-';
    str[3] = (mode & EXT2_S_IXUSR) ? 'x' : '-';

    // Group
    str[4] = (mode & EXT2_S_IRGRP) ? 'r' : '-';
    str[5] = (mode & EXT2_S_IWGRP) ? 'w' : '-';
    str[6] = (mode & EXT2_S_IXGRP) ? 'x' : '-';

    // Others
    str[7] = (mode & EXT2_S_IROTH) ? 'r' : '-';
    str[8] = (mode & EXT2_S_IWOTH) ? 'w' : '-';
    str[9] = (mode & EXT2_S_IXOTH) ? 'x' : '-';

    // Special bits
    if (mode & EXT2_S_ISUID)
        str[3] = (str[3] == 'x') ? 's' : 'S';
    if (mode & EXT2_S_ISGID)
        str[6] = (str[6] == 'x') ? 's' : 'S';
    if (mode & EXT2_S_ISVTX)
        str[9] = (str[9] == 'x') ? 't' : 'T';

    str[10] = '\0';
}

void ext2_print(size_t in) {
    ext2_inode_t* inode = get_inode(in);

    if(!(inode->mode & EIT_DIR)) {
        kprintf(LOG_WARN, "ext2", "Not a directory: %u\r\n", in, inode->mode);
        return;
    }
    size_t size;
    ext2_dir_entry_t* dir = (ext2_dir_entry_t*)inode_get_data(inode, &size);
    if(dir == NULL) return;
    while(dir->inode) {
        ext2_inode_t* i = get_inode(dir->inode);
        char mode[11];
        mode_to_string(i->mode, mode);
        printf("%s %u %04u %04u % 8u %.*s\r\n",
            mode, i->links_count, i->uid, i->gid, i->blocks * 512,
            dir->name_len, dir->name
        );
        kfree(i);
        dir = (ext2_dir_entry_t*)((uint8_t*)dir + dir->rec_len);
    }
}

void ext2_print_tree(size_t in, int d) {
    ext2_inode_t* inode = get_inode(in);

    if(!(inode->mode & EIT_DIR)) {
        kprintf(LOG_WARN, "ext2", "Not a directory: %u\r\n", in, inode->mode);
        return;
    }
    size_t size;
    ext2_dir_entry_t* dir = (ext2_dir_entry_t*)inode_get_data(inode, &size);
    if(dir == NULL) return;
    while(dir->inode) {
        if(dir->name[0] != '.') {
            for(int i = 0; i < d; i++) printf("    ");
            ext2_inode_t* i = get_inode(dir->inode);
            char mode[11];
            mode_to_string(i->mode, mode);
            printf("%s %u %04u %04u % 8u %.*s\r\n",
                mode, i->links_count, i->uid, i->gid, i->blocks * 512,
                dir->name_len, dir->name
            );
            if(i->mode & EXT2_S_IFDIR) {
                kfree(i);
                ext2_print_tree(dir->inode, d + 1);
            }
        }
        dir = (ext2_dir_entry_t*)((uint8_t*)dir + dir->rec_len);
    }
    kfree(dir);
}

int ext2_init(bdev_read_t _read, size_t _vol_start) {
    vol_start = _vol_start;
    read = _read;
    sb = kmalloc(sizeof(ext2_sb_t), 0);
    if(read((void*)sb, vol_start + 1024, sizeof(ext2_sb_t))) return 1;
    if(sb->magic != EXT2_SUPER_MAGIC) {
        kprintf(LOG_ERR, "ext2", "Invalid magic");
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
    ext2_print_tree(EXT2_ROOT_INO, 0);
    return 0;
}