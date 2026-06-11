CC := gcc
CFLAGS := -ffreestanding -nostdlib -fno-builtin -Iinclude -Iinclude/ext4 -Wall
AR := ar

BUILD_DIR := build

SRCS := $(sort $(wildcard src/*.c))
HDRS := $(sort $(wildcard include/ext4/*.h))
OBJS := $(SRCS:src/%.c=$(BUILD_DIR)/src/%.o)

.PHONY: all clean compdb format test

all: $(BUILD_DIR)/libext4.a

$(BUILD_DIR)/libext4.a: $(OBJS) | $(BUILD_DIR)
	$(AR) rcs $@ $(OBJS)

$(BUILD_DIR)/src/%.o: src/%.c | $(BUILD_DIR)/src
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR) $(BUILD_DIR)/src:
	mkdir -p $@

compile_commands.json: Makefile
	@printf '[\n' > $@
	@first=1; for src in $(SRCS); do \
	    obj="$(BUILD_DIR)/$${src%.c}.o"; \
	    cmd="$(CC) $(CFLAGS) -c $$src -o $$obj"; \
	    if [ $$first -eq 1 ]; then first=0; else printf ',\n' >> $@; fi; \
	    printf '  { "directory": "$(CURDIR)", "command": "%s", "file": "%s" }' "$$cmd" "$$src" >> $@; \
	done; \
	printf '\n]\n' >> $@

compdb: compile_commands.json

format:
	clang-format -i $(SRCS) $(HDRS)

test: $(BUILD_DIR)/libext4.a | $(BUILD_DIR)/tests
	$(CC) -Iinclude -Iinclude/ext4 tests/mount_test.c $(BUILD_DIR)/libext4.a -o $(BUILD_DIR)/tests/mount_test
	$(BUILD_DIR)/tests/mount_test sdcard-rv.img

clean:
	rm -rf $(BUILD_DIR) compile_commands.json

$(BUILD_DIR)/tests:
	mkdir -p $@
