#include "callback.h"
#include "ext4.h"
#include "defs.h"
#include "helper.h"

extern struct basic_io bio;

int ext4_mount(uint64 dev, struct ext4_io *method)
{
	// use 4KB by default to read superblock
	struct ext4_fs first = {.block_size = 0x1000};

	uint8 buf[0x1000] = {0};
	ext4_read_fs_block(&first, method, dev, (uint8 *)buf);

	const uint8 *sb = buf + EXT4_SUPER_OFFSET;
	uint16 magic = ext4_read_le16(sb + EXT4_SB_MAGIC);
	if (magic != EXT4_SUPER_MAGIC) {
		bio.kprintf("magic mismatch, should be 0xEF53, but got 0x%x\n",
			    magic);
		return -1;
	}
	bio.kprintf("magic: 0x%x\n", magic);

	return 0;
}
