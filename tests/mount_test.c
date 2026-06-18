#include "callback.h"
#include "ext4.h"
#include "kalloc.h"
#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int ext4_mount(uint64 dev, struct ext4_backend *backend, struct ext4_fs *fs);

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

static int kalloc_alloc_calls;
static int kalloc_free_calls;
static uint64 kalloc_last_size;
static void *kalloc_last_ptr;
static void *kalloc_freed_ptr;

static void *test_kalloc(uint64 size)
{
	kalloc_alloc_calls++;
	kalloc_last_size = size;
	kalloc_last_ptr = malloc(size);
	return kalloc_last_ptr;
}

static void test_kfree(void *ptr)
{
	kalloc_free_calls++;
	kalloc_freed_ptr = ptr;
	free(ptr);
}

static int expect_u32(const char *name, uint32 actual, uint32 expected)
{
	printf("%s: %u\n", name, actual);
	if (actual != expected) {
		fprintf(stderr, "%s mismatch: got %u, expected %u\n", name,
			actual, expected);
		return -1;
	}
	return 0;
}

static int expect_u64(const char *name, uint64 actual, uint64 expected)
{
	printf("%s: %lu\n", name, actual);
	if (actual != expected) {
		fprintf(stderr, "%s mismatch: got %lu, expected %lu\n", name,
			actual, expected);
		return -1;
	}
	return 0;
}

static int expect_ptr(const char *name, void *actual, void *expected)
{
	printf("%s: %p\n", name, actual);
	if (actual != expected) {
		fprintf(stderr, "%s mismatch: got %p, expected %p\n", name,
			actual, expected);
		return -1;
	}
	return 0;
}

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
	struct ext4_fs fs;
	struct ext4_backend backend;
	int ret;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <image>\n", argv[0]);
		return 2;
	}

	kalloc_set_allocator(test_kalloc, test_kfree);
	disk.fp = fopen(argv[1], "rb");
	if (disk.fp == NULL) {
		perror("fopen");
		return 2;
	}
	disk.block_size = 512;

	backend.dev = 0;
	backend.superblock_block = 0;
	backend.block_size = disk.block_size;
	backend.priv = &disk;
	backend.getblk = file_getblk;
	backend.putblk = file_putblk;

	ret = ext4_mount(0, &backend, &fs);
	fclose(disk.fp);

	if (ret != 0) {
		fprintf(stderr, "ext4_mount failed: %d\n", ret);
		return 1;
	}
	if (fs.block_size == 0 || fs.inodes_count == 0 ||
	    fs.blocks_count == 0) {
		fprintf(stderr, "ext4_mount returned incomplete fs state\n");
		return 1;
	}
	if (expect_u32("block_size", fs.block_size, 4096) < 0 ||
	    expect_u64("blocks_count", fs.blocks_count, 1048576) < 0 ||
	    expect_u32("inodes_count", fs.inodes_count, 262144) < 0 ||
	    expect_u32("blocks_per_group", fs.blocks_per_group, 32768) < 0 ||
	    expect_u32("inodes_per_group", fs.inodes_per_group, 8192) < 0 ||
	    expect_u32("inode_size", fs.inode_size, 256) < 0 ||
	    expect_u32("desc_size", fs.desc_size, 64) < 0 ||
	    expect_u64("group_desc_block", fs.group_desc_block, 1) < 0) {
		return 1;
	}
	if (expect_u32("kalloc_alloc_calls", kalloc_alloc_calls, 1) < 0 ||
	    expect_u32("kalloc_free_calls", kalloc_free_calls, 1) < 0 ||
	    expect_u64("kalloc_last_size", kalloc_last_size, KALLOC_SIZE) < 0 ||
	    expect_ptr("kalloc_freed_ptr", kalloc_freed_ptr, kalloc_last_ptr) <
		0) {
		return 1;
	}

	return 0;
}
