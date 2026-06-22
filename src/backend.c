#include "callback.h"
#include "ext4.h"
#include "types.h"
#include "string.h"

/**
 * ext4_read_fs_block - read a block from an EXT4 image
 *
 * Context: Independent of backend block size
 *
 * Return: 0 on success
 * */
int ext4_read_fs_block(const struct ext4_fs *fs,
		       const struct ext4_backend *backend, uint64 block,
		       uint8 *dst)
{
	uint64 bytes = block * fs->block_size;
	uint64 copied = 0;

	while (copied < fs->block_size) {
		uint64 backend_block = bytes / backend->block_size;
		uint32 offset = bytes % backend->block_size;
		uint32 n = backend->block_size - offset;

		if (n > fs->block_size - copied) {
			n = fs->block_size - copied;
		}
		struct ext4_block blk = {0};

		int ret = backend->getblk(backend->dev, backend_block, &blk,
					  backend->priv);
		if (ret < 0) {
			return ret;
		}
		memcpy(dst + copied, blk.data + offset, n);
		backend->putblk(&blk, backend->priv);

		bytes += n;
		copied += n;
	}

	return 0;
};
