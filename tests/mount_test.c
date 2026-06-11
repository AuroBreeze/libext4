#include "callback.h"
#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int ext4_mount(uint64 dev, struct ext4_io *method);

static void test_kprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

struct basic_io bio = {
    .kprintf = test_kprintf,
};

struct file_disk {
	FILE *fp;
	uint32 block_size;
};

static int file_getblk(uint32 dev, uint64 block, struct ext4_block *out,
		       void *priv)
{
	struct file_disk *disk = priv;
	uint8 *data = malloc(disk->block_size);

	(void)dev;
	if (data == NULL) {
		return -1;
	}
	if (fseek(disk->fp, block * disk->block_size, SEEK_SET) != 0) {
		free(data);
		return -1;
	}
	if (fread(data, 1, disk->block_size, disk->fp) != disk->block_size) {
		free(data);
		return -1;
	}

	out->data = data;
	out->priv = data;
	return 0;
}

static int file_putblk(struct ext4_block *blk, void *priv)
{
	(void)priv;
	free(blk->priv);
	blk->data = NULL;
	blk->priv = NULL;
	return 0;
}

int main(int argc, char **argv)
{
	struct file_disk disk;
	struct ext4_io io;
	int ret;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <image>\n", argv[0]);
		return 2;
	}

	disk.fp = fopen(argv[1], "rb");
	if (disk.fp == NULL) {
		perror("fopen");
		return 2;
	}
	disk.block_size = 512;

	io.dev = 0;
	io.superb_block = 0;
	io.io_block_size = disk.block_size;
	io.priv = &disk;
	io.getblk = file_getblk;
	io.putblk = file_putblk;

	ret = ext4_mount(0, &io);
	fclose(disk.fp);

	if (ret != 0) {
		fprintf(stderr, "ext4_mount failed: %d\n", ret);
		return 1;
	}

	return 0;
}
