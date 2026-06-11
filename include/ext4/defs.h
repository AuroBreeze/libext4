#ifndef _EXT4_DEFS_H
#define _EXT4_DEFS_H

#include "types.h"

struct ext4_fs;
struct ext4_io;

// io.c
int ext4_read_fs_block(const struct ext4_fs *fs, const struct ext4_io *io,
		       uint64 block, uint8 *dst);
#endif
