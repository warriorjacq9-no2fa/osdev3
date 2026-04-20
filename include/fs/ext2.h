#ifndef FS_EXT2_H
#define FS_EXT2_H

#include <stdint.h>
#include <fs/vfs.h>

#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_VALID_FS   1
#define EXT2_ERROR_FS   2

#define EXT2_ERRORS_CONTINUE    1
#define EXT2_ERRORS_RO          2
#define EXT2_ERRORS_PANIC       3

#define EXT2_OS_LINUX           0
#define EXT2_OS_HURD            1
#define EXT2_OS_MASIX           2
#define EXT2_OS_FREEBSD         3
#define EXT2_OS_LITES           4

#define EXT2_GOOD_OLD_REV       0
#define EXT2_DYNAMIC_REV        1

#define EXT2_FEATURE_COMPAT_DIR_PREALLOC    0x0001
#define EXT2_FEATURE_COMPAT_IMAGIC_INODES   0x0002
#define EXT2_FEATURE_COMPAT_HAS_JOURNAL     0x0004
#define EXT2_FEATURE_COMPAT_EXT_ATTR        0x0008
#define EXT2_FEATURE_COMPAT_RESIZE_INO      0x0010
#define EXT2_FEATURE_COMPAT_DIR_INDEX       0x0020

#define EXT2_FEATURE_INCOMPAT_COMPRESSION   0x0001
#define EXT2_FEATURE_INCOMPAT_FILETYPE      0x0002
#define EXT2_FEATURE_INCOMPAT_RECOVER       0x0004
#define EXT2_FEATURE_INCOMPAT_JOURNAL_DEV   0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG       0x0010

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    0x0004

#define EXT2_LZV1_ALG   0
#define EXT2_LZRW3A_ALG 1
#define EXT2_GZIP_ALG   2
#define EXT2_BZIP2_ALG  3
#define EXT2_LZO_ALG    4

#define EXT2_BAD_INO        1
#define EXT2_ROOT_INO       2
#define EXT2_ACL_IDX_INO    3
#define EXT2_ACL_DATA_INO   4
#define EXT2_BOOT_LOADER_INO 5
#define EXT2_UNDEL_DIR_INO  6

#define EXT2_S_IFSOCK   0xC000
#define EXT2_S_IFLNK    0xA000
#define EXT2_S_IFREG    0x8000
#define EXT2_S_IFBLK    0x6000
#define EXT2_S_IFDIR    0x4000
#define EXT2_S_IFCHR    0x2000
#define EXT2_S_IFIFO    0x1000

#define EXT2_S_ISUID    0x0800
#define EXT2_S_ISGID    0x0400
#define EXT2_S_ISVTX    0x0200

#define EXT2_S_IRUSR    0x0100
#define EXT2_S_IWUSR    0x0080
#define EXT2_S_IXUSR    0x0040
#define EXT2_S_IRGRP    0x0020
#define EXT2_S_IWGRP    0x0010
#define EXT2_S_IXGRP    0x0008
#define EXT2_S_IROTH    0x0004
#define EXT2_S_IWOTH    0x0002
#define EXT2_S_IXOTH    0x0001

#define EXT2_SECRM_FL       0x00001
#define EXT2_UNRM_FL        0x00002
#define EXT2_COMPR_FL       0x00004
#define EXT2_SYNC_FL        0x00008
#define EXT2_IMMUTABLE_FL   0x00010
#define EXT2_APPEND_FL      0x00020
#define EXT2_NODUMP_FL      0x00040
#define EXT2_NOATIME_FL     0x00080

#define EXT2_DIRTY_FL       0x00100
#define EXT2_COMPRBLK_FL    0x00200
#define EXT2_NOCOMPR_FL     0x00400
#define EXT2_ECOMPR_FL      0x00800

#define EXT2_BTREE_FL       0x01000
#define EXT2_INDEX_FL       0x01000
#define EXT2_IMAGIC_FL      0x02000
#define EXT2_JOURNAL_DATA_FL 0x04000
#define EXT2_RESERVED_FL 0x80000000

#define EXT2_FT_UNKNOWN     0
#define EXT2_FT_REG_FILE    1
#define EXT2_FT_DIR         2
#define EXT2_FT_CHRDEV      3
#define EXT2_FT_BLKDEV      4
#define EXT2_FT_FIFO        5
#define EXT2_FT_SOCK        6
#define EXT2_FT_SYMLINK     7

#define DX_HASH_LEGACY      0
#define DX_HASH_HALF_MD4    1
#define DX_HASH_TEA         2

typedef struct ext2_superblock {
    uint32_t    inodes_count; // Total number of used and free inodes
    uint32_t    blocks_count; // Total number of used, free, and reserved blocks
    uint32_t    r_blocks_count; // Total number of blocks reserved for the super user
    uint32_t    free_blocks_count; // Total number of free blocks (including reserved)
    uint32_t    free_inodes_count; // Total number of free inodes across block groups
    uint32_t    first_data_block; // ID of the block containing the superblock structure
    uint32_t    log_block_size; // Block size in bytes = 1024 << log_block_size
    int32_t     log_frag_size; // Fragment size in bytes = (log_frag_size > 0) ? 1024 << log_frag_size : 1024 >> log_frag_size
    uint32_t    blocks_per_group; // Total number of blocks per group, can be used with first_data_block to determine block group boundaries
    uint32_t    frags_per_group; // Total number of fragments per group, also used to determine the size of the block bitmap
    uint32_t    inodes_per_group; // Total inodes per group, also used to determine the size of the inode bitmap
    uint32_t    mtime; // Unix time of last mount
    uint32_t    wtime; // Unix time of last write
    uint16_t    mnt_count; // How many times the filesystem was mounted since the last fsck
    uint16_t    max_mnt_count; // Max times the filesystem can be mounted before a fsck
    uint16_t    magic; // Identifies the file system as ext2, should be 0xEF53
    uint16_t    state; // When mounted, set to EXT2_ERROR_FS, when unmounted, set to EXT2_VALID_FS
    uint16_t    errors; // What to do on error, see enum ext2_error
    uint16_t    minor_rev_level; // Minor revision level
    uint32_t    lastcheck; // Unix time of last fsck
    uint32_t    checkinterval; // Maximum Unix time between fscks
    uint32_t    creator_os; // Who created the file system, see enum ext2_os
    uint32_t    rev_level; // Revision level, if > 0 the extended superblock exists
    uint16_t    def_resuid; // Default uid for reserved blocks
    uint16_t    def_resgid; // Default gid for reserved blocks
} __attribute__((packed)) ext2_sb_t;

typedef struct ext2_superblock_ext {
    uint32_t    first_ino; // Index of the first useable inode for standard files
    uint16_t    inode_size; // Size in bytes of the inode structure
    uint16_t    block_group_nr; // Block group number holding the superblock
    uint32_t    feature_compat; // Bitmask of compatible features
    uint32_t    feature_incompat; // Bitmask of incompatible features, don't mount if any are unsupported
    uint32_t    feature_ro_compat; // Bitmask of read-only features, mount as read-only if any are unsupported
    uint8_t     uuid[16]; // Volume ID outputted by blkid
    char        volume_name[16]; // Volume name, only ISO-Latin-1 characters
    char        last_mounted[64]; // Directory path where the file system was last mounted
    uint32_t    algo_bitmap; // Used to determine the compression methods used
    uint8_t     prealloc_blocks; // Number of blocks to preallocate when creating a file
    uint8_t     prealloc_dir_blocks; // Number of blocks to preallocate when creating a directory
    uint16_t    unused;
    uint8_t     journal_uuid[16]; // UUID of the journal superblock
    uint32_t    journal_inum; // Inode number of the journal file
    uint32_t    journal_dev; // Device number of the journal file
    uint32_t    last_orphan; // First inode in the list of inodes to delete
    uint32_t    hash_seed[4]; // Seeds for the hash algorithm
    uint8_t     def_hash_version; // Default hash version
    uint32_t    default_mount_options; // Default mount options
    uint32_t    first_meta_bg; // Block group ID of the first meta block group
} __attribute__((packed)) ext2_sb_ext_t;

typedef struct ext2_bg_descriptor {
    uint32_t    block_bitmap; // Block ID of the first block of the block bitmap
    uint32_t    inode_bitmap; // Block ID of the first block of the inode bitmap
    uint32_t    inode_table; // Block ID ot the first block of the inode table
    uint16_t    free_blocks_count; // Total number of free blocks in this group
    uint16_t    free_inodes_count; // Total number of free inodes in this group
    uint16_t    used_dirs_count; // Total number of inodes allocated for directories in this group
    uint16_t    pad;
    uint8_t     reserved[12];
} ext2_bgdesc_t;

#define EIT_FIFO    0x1000
#define EIT_CHAR    0x2000
#define EIT_DIR     0x4000
#define EIT_BDEV    0x6000
#define EIT_FILE    0x8000
#define EIT_LINK    0xA000
#define EIT_SOCK    0xC000

#define EIP_OX      0x0001
#define EIP_OW      0x0002
#define EIP_OR      0x0004
#define EIP_GX      0x0008
#define EIP_GW      0x0010
#define EIP_GR      0x0020
#define EIP_UX      0x0040
#define EIP_UW      0x0080
#define EIP_UR      0x0100
#define EIP_STICKY  0x0200
#define EIP_SETGID  0x0400
#define EIP_SETUID  0x0800

#define EIF_SDEL    0x00001 // Secure deletion
#define EIF_COPYDEL 0x00002 // Keep a copy of data when deleted
#define EIF_COMP    0x00004 // File compression
#define EIF_SYNC    0x00008 // New data is written immediately
#define EIF_CONST   0x00010 // File data cannot be changed
#define EIF_APPEND  0x00020 // Append only
#define EIF_NODUMP  0x00040 // Don't back up
#define EIF_NOATIME 0x00080 // Don't update access time
#define EIF_HASH    0x01000 // Hash indexed dir
#define EIF_AFS     0x02000 // AFS dir
#define EIF_JOURNAL 0x04000 // Journal data

typedef struct ext2_inode {
    uint16_t    mode; // Format and access rights
    uint16_t    uid; // UID associated with the file
    union {
        int32_t     r0_size; // Revision 0, file size
        uint32_t    r1_size_low; // Revision 1, low 32 of file size
    };
    uint32_t    atime; // Unix time of last access
    uint32_t    ctime; // Unix time of creation
    uint32_t    mtime; // Unix time of last modify
    uint32_t    dtime; // Unix time of deletion
    uint16_t    gid; // GID associated with the file
    uint16_t    links_count; // How many times this inode is linked to. Most files have a link count of 1. Symlinks do not affect link count. When this reaches 0, free the inode
    uint32_t    blocks; // Total number of 512-byte blocks reserved for the inode. To index the block array, use blocks/(2<<ext2_sb_t::log_block_size)
    uint32_t    flags; // How the ext2 implementation should behave when accessing data, see the EIF_* defs
    uint32_t    osd1;
    uint32_t    block[15]; // Block IDs for the data blocks of this inode. First 12 are direct, last 3 are progressively more indirect
    uint32_t    generation; // Indicates the file version, used by NFS
    union {
        uint32_t    r1_size_high; // Revision 1, high 32 of file size (for 64-bit file size)
        uint32_t    file_acl; // Block ID for the block containing extended file attrubites
        uint32_t    dir_acl; // Block ID for the block containing extended directory attributes
    };
    uint32_t    faddr; // Location of the file fragment
    uint32_t    osd2[3];
} __attribute__((packed)) ext2_inode_t;

enum ext2_dir_type {
    EDT_UNKNOWN,
    EDT_FILE,
    EDT_DIR,
    EDT_CHAR,
    EDT_BDEV,
    EDT_FIFO,
    EDT_SOCK,
    EDT_LINK
};

typedef struct ext2_dir_entry {
    uint32_t    inode;
    uint16_t    rec_len;
    uint8_t     name_len;
    union {
        uint8_t type;
        uint8_t name_len_high;
    };
    char        name[];
} __attribute__((packed)) ext2_dir_entry_t;

int ext2_init(bdev_read_t read, size_t vol_start);

#endif