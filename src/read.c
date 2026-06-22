#include "callback.h"
#include "ext4.h"
#include "helper.h"
#include "kalloc.h"
#include "defs.h"

// --- basic methods ---
extern struct basic_io bio;
/**
 * ext4_read_inode_table_block - read inode table from group descriptor table
 *
 * Context: When reading an inode, you need to first search for the
 * corresponding group in the group table, and then go to the corresponding
 * group to find the corresponding inode table.
 *
 * @group: group numbe
 *
 * Return: 0
 * */
int ext4_read_inode_table_block(struct ext4_fs *fs,
				struct ext4_backend *backend, uint32 group,
				uint64 *block)
{
	uint64 desc_offset = fs->desc_size * group;
	uint64 byte_offset =
	    (fs->group_desc_block * fs->block_size) + desc_offset;

	uint64 cache_offset = byte_offset / (uint64)backend->block_size;
	uint64 offset_in_block = byte_offset % (uint64)backend->block_size;

	uint8 *dst = kalloc();
	ext4_read_fs_block(fs, backend, cache_offset, dst);

	const uint8 *gd = dst + offset_in_block;
	*block = ext4_read_le64_split(gd + EXT4_GD_INODE_TABLE_LO,
				      gd + EXT4_GD_INODE_TABLE_HI);
	kfree(dst);
	return 0;
}

/**
 * ext4_read_inode - read inode from inode table
 *
 * Context: After finding the inode table block by using
 * ext4_read_inode_table_block, find the corresponding inode and write the data
 * to the inode.
 *
 * @ino: Need to read inode
 * @inode: Data to be written
 *
 * Return: 0
 * */
int ext4_read_inode(struct ext4_fs *fs, struct ext4_backend *backend,
		    struct ext4_inode *inode, uint64 ino)
{
	uint64 group = (ino - 1) / fs->inodes_per_group;
	uint64 index = (ino - 1) % fs->inodes_per_group;

	uint64 inode_table_block;
	ext4_read_inode_table_block(fs, backend, group, &inode_table_block);

	uint64 inode_table_byte_offset =
	    (inode_table_block * fs->block_size) + index * fs->inode_size;
	uint64 cache_offset = inode_table_byte_offset / backend->block_size;
	uint64 offset_in_block =
	    (inode_table_block * fs->block_size) % backend->block_size;

	uint8 *buf = kalloc();
	ext4_read_fs_block(fs, backend, cache_offset, buf);
	const uint8 *raw_inode = buf + offset_in_block;

	inode->mode = ext4_read_le16(raw_inode + EXT4_INODE_MODE);
	inode->size =
	    ((uint64)ext4_read_le32(raw_inode + EXT4_INODE_SIZE_HI) << 32) |
	    ext4_read_le32(raw_inode + EXT4_INODE_SIZE_LO);
	inode->blocks_lo = ext4_read_le32(raw_inode + EXT4_INODE_BLOCKS_LO);
	inode->flags = ext4_read_le32(raw_inode + EXT4_INODE_FLAGS);
	memcpy(inode->block, raw_inode + EXT4_INODE_BLOCK,
	       EXT4_INODE_BLOCK_BYTES);

	kfree(buf);
	return 0;
}

/**
 * ext4_read_extent_header - read extent header from inode
 *
 * Context: https://docs.kernel.org/filesystems/ext4/ifork.html
 * The header in the inode block is stored in the first 12 bytes of the 60 bytes
 *
 * Return: 0 on success else -1
 * */
int ext4_read_extent_header(struct ext4_inode *inode,
			    struct ext4_extent_header *header)
{
	const uint8 *raw_data = inode->block;
	header->magic = ext4_read_le16(raw_data + EXT4_EXT_HEADER_MAGIC);
	header->entries = ext4_read_le16(raw_data + EXT4_EXT_HEADER_ENTRIES);
	header->max = ext4_read_le16(raw_data + EXT4_EXT_HEADER_MAX);
	header->depth = ext4_read_le16(raw_data + EXT4_EXT_HEADER_DEPTH);
	header->generation =
	    ext4_read_le32(raw_data + EXT4_EXT_HEADER_GENERATION);

	if (header->magic != EXT4_EXT_MAGIC) {
		bio.kprintf("ext4_read_inode_header: magic mismatch\n");
		bio.kprintf("ext4_read_inode_header: expected 0x%x, got 0x%x\n",
			    EXT4_EXT_MAGIC, header->magic);
		return -1;
	}

	return 0;
}

/**
 * ext4_read_extent - read extent from inode
 *
 * Context: https://docs.kernel.org/filesystems/ext4/ifork.html
 * The extent in the inode block is stored in 12 bytes of the 60 bytes
 *
 * Return: 0 on success else -1
 * */
int ext4_read_extent(const struct ext4_inode *inode, uint16 index,
		     struct ext4_extent *extent)
{
	// In extent mode, inode.i_block starts with a 12-byte extent header.
	// Leaf extent entries are packed immediately after that header.
	const uint8 *raw_extent = inode->block + EXT4_EXT_HEADER_SIZE +
				  ((uint64)index * EXT4_EXTENT_SIZE);

	extent->block = ext4_read_le32(raw_extent + EXT4_EXTENT_BLOCK);
	extent->len = ext4_read_le16(raw_extent + EXT4_EXTENT_LEN);
	extent->start =
	    ((uint64)ext4_read_le16(raw_extent + EXT4_EXTENT_START_HI) << 32) |
	    ext4_read_le32(raw_extent + EXT4_EXTENT_START_LO);

	return 0;
}

// --- extent functions ---
