#ifndef _EXT4_CALLBACK_H
#define _EXT4_CALLBACK_H

#include "types.h"

struct ext4_block {
	/* Read-only view of a block returned by ext4_io.getblk. */
	const uint8 *data;
	/* Backend-owned handle, such as a block-cache buffer. */
	void *priv;
};

typedef int (*ext4_getblk_t)(uint32 dev, uint64 block, struct ext4_block *out,
			     void *priv);
typedef int (*ext4_putblk_t)(struct ext4_block *blk, void *priv);

struct ext4_io {
	/* Backend device identifier passed to getblk. */
	uint32 dev;
	/* Superblock block number. */
	uint32 superb_block;
	/* I/O block size: the unit of getblk's block argument (e.g. 512). */
	uint32 io_block_size;
	/* Opaque pointer passed through to getblk and putblk. */
	void *priv;
	/* Borrow a block; out->data must stay valid until putblk is called. */
	ext4_getblk_t getblk;
	/* Release a block previously returned by getblk. */
	ext4_putblk_t putblk;
};

// The most basic d,x,p,s,c are needed
typedef void (*out)(const char *fmt, ...);

struct basic_io {
	out kprintf;
};

#endif
