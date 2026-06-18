#include "callback.h"
#include "ext4.h"
#include "defs.h"
#include "helper.h"
#include "kalloc.h"

extern struct basic_io bio;

// This reader is intentionally read-only and small. Unknown incompatible
// features are rejected before later code interprets unsupported disk layouts.
static int ext4_check_features(struct ext4_fs *fs)
{
	uint32 supported_incompat =
	    EXT4_FEATURE_INCOMPAT_FILETYPE | EXT4_FEATURE_INCOMPAT_EXTENTS |
	    EXT4_FEATURE_INCOMPAT_64BIT | EXT4_FEATURE_INCOMPAT_FLEX_BG;
	uint32 supported_ro_compat = EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER |
				     EXT4_FEATURE_RO_COMPAT_LARGE_FILE |
				     EXT4_FEATURE_RO_COMPAT_HUGE_FILE |
				     EXT4_FEATURE_RO_COMPAT_DIR_NLINK |
				     EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE |
				     EXT4_FEATURE_RO_COMPAT_METADATA_CSUM;
	uint32 unknown_incompat = fs->feature_incompat & ~supported_incompat;
	uint32 unknown_ro_compat = fs->feature_ro_compat & ~supported_ro_compat;

	if (unknown_incompat != 0) {
		bio.kprintf("ext4: unsupported incompat features: %x",
			    unknown_incompat);
		return -1;
	}

	if (unknown_ro_compat != 0) {
		bio.kprintf("ext4: unsupported ro_compat features: %x",
			    unknown_ro_compat);
		return -1;
	}

	if ((fs->feature_incompat & EXT4_FEATURE_INCOMPAT_EXTENTS) == 0) {
		bio.kprintf("ext4: non-extent inode data is not supported");
		return -1;
	}

	bio.kprintf("ext4: supported features: 0x%x\n", supported_incompat);
	return 0;
}

int ext4_mount(uint64 dev, struct ext4_io *method, struct ext4_fs *fs)
{
	// use 4KB by default to read superblock
	struct ext4_fs first = {.block_size = 0x1000};

	uint8 *buf = kalloc(); // allocate 4KB
	memset(buf, 0, KALLOC_SIZE);

	ext4_read_fs_block(&first, method, dev, (uint8 *)buf);

	const uint8 *sb = buf + EXT4_SUPER_OFFSET;
	uint16 magic = ext4_read_le16(sb + EXT4_SB_MAGIC);
	if (magic != EXT4_SUPER_MAGIC) {
		bio.kprintf("magic mismatch, should be 0xEF53, but got 0x%x\n",
			    magic);
		return -1;
	}
	bio.kprintf("magic: 0x%x\n", magic);

	uint32 log_block_size = ext4_read_le32(sb + EXT4_SB_LOG_BLOCK_SIZE);
	uint32 blocks_lo = ext4_read_le32(sb + EXT4_SB_BLOCKS_COUNT_LO);
	uint32 blocks_hi = ext4_read_le32(sb + EXT4_SB_BLOCKS_COUNT_HI);

	fs->dev = dev;
	fs->block_size = 1024U << log_block_size;
	fs->blocks_count = ((uint64)blocks_hi << 32) | blocks_lo;
	fs->inodes_count = ext4_read_le32(sb + EXT4_SB_INODES_COUNT);
	fs->blocks_per_group = ext4_read_le32(sb + EXT4_SB_BLOCKS_PER_GROUP);
	fs->inodes_per_group = ext4_read_le32(sb + EXT4_SB_INODES_PER_GROUP);
	fs->inode_size = ext4_read_le16(sb + EXT4_SB_INODE_SIZE);
	fs->desc_size = ext4_read_le16(sb + EXT4_SB_DESC_SIZE);
	// For 1 KiB block filesystems, block 0 is before the superblock and the
	// GDT starts at block 2. For larger block sizes, the GDT starts at
	// block 1.
	fs->group_desc_block = fs->block_size == 1024 ? 2 : 1;
	fs->feature_compat = ext4_read_le32(sb + EXT4_SB_FEATURE_COMPAT);
	fs->feature_incompat = ext4_read_le32(sb + EXT4_SB_FEATURE_INCOMPAT);
	fs->feature_ro_compat = ext4_read_le32(sb + EXT4_SB_FEATURE_RO_COMPAT);

	return ext4_check_features(fs);
}
