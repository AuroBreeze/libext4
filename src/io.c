#include "callback.h"
#include "ext4.h"
#include "types.h"
#include "string.h"

/**
 * ext4_read_fs_block - read a block from an EXT4 image
 *
 * Context: Independent of io_block_size
 *
 * Return: 0 on success
 * */
int ext4_read_fs_block(const struct ext4_fs *fs, const struct ext4_io *io,
		       uint64 block, uint8 *dst)
{
	uint64 bytes = block * fs->block_size;
	uint64 copied = 0;

	while (copied < fs->block_size) {
		uint64 io_block = bytes / io->io_block_size;
		uint32 offset = bytes % io->io_block_size;
		uint32 n = io->io_block_size - offset;

		if (n > fs->block_size - copied) {
			n = fs->block_size - copied;
		}
		struct ext4_block blk = {0};

		int ret = io->getblk(io->dev, io_block, &blk, io->priv);
		if (ret < 0) {
			return ret;
		}
		memcpy(dst + copied, blk.data + offset, n);
		io->putblk(&blk, io->priv);

		bytes += n;
		copied += n;
	}

	return 0;
};
