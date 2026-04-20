#include <fs/ext2.h>
#include <kernel/kmalloc.h>
#include <kernel/klog.h>

static ext2_sb_t *sb;
static ext2_sb_ext_t *ext_sb;
static ext2_bgdesc_t *bgdesc_table;

inline size_t block_offset(size_t vol_start, size_t block) {
    return vol_start + (1024 << sb->log_block_size);
}

int ext2_init(bdev_read_t read, size_t vol_start) {
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
    if(read((void*)bgdesc_table, block_offset(vol_start, bgdt_block), bgdt_size))
        return 1;
    return 0;
}

ext2_inode_t* get_inode(size_t id) {
    size_t bg = (id - 1) / sb->inodes_per_group;
    size_t l_id = (id - 1) % sb->inodes_per_group;

    size_t inode_table = bgdesc_table[bg].inode_table;

    
}