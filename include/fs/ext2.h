#ifndef FS_EXT2_H
#define FS_EXT2_H

#include <stdint.h>
#include <fs/vfs.h>

#define FEAT_PREALLOC   0x0001  // Optional
#define FEAT_IMAGIC     0x0002  // Optional
#define FEAT_JOURNAL    0x0004  // Optional
#define FEAT_EXT_ATTRS  0x0008  // Optional
#define FEAT_RESIZE     0x0010  // Optional
#define FEAT_HASH       0x0020  // Optional

#define FEAT_COMP       0x0001  // Required
#define FEAT_DIRTYPE    0x0002  // Required
#define FEAT_REPLAY     0x0004  // Required
#define FEAT_JDEV       0x0008  // Required

#define FEAT_SPARSE     0x0001  // Required for writing
#define FEAT_LL         0x0002  // Required for writing
#define FEAT_BTREE      0x0004  // Required for writing

#define EXT2_SUPER_MAGIC 0xEF53

enum ext2_state {
    EXT2_VALID_FS = 1,
    EXT2_ERROR_FS
};

enum ext2_error {
    EXT2_ERRORS_CONTINUE = 1,
    EXT2_ERRORS_RO,
    EXT2_ERRORS_PANIC
};

enum ext2_os {
    EXT2_OS_LINUX,
    EXT2_OS_HURD,
    EXT2_OS_MASIX,
    EXT2_OS_FREEBSD,
    EXT2_OS_LITES
};

enum ext2_compression {
    EXT2_LZV1,
    EXT2_LZZRW3A,
    EXT2_GZIP,
    EXT2_BZIP2,
    EXT2_LZO
};

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