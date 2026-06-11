# libext4

A minimal, libc-free, read-only EXT4 library.

No `malloc`, no `FILE*`, no standard headers. Designed for freestanding
environments (kernels, bootloaders, embedded systems).

## Build

```sh
make           # → build/libext4.a
make compdb    # → compile_commands.json (for clangd/LSP)
make clean     # remove build/ and compile_commands.json
```

Out-of-tree build: all artifacts go to `build/`.

## Integration

```c
#include "ext4/ext4.h"

// caller provides block read callback
int bread(uint32 dev, uint64 block, uint8 *buf, void *priv)
{
    // read block from disk ...
    return 0;
}

struct ext4_fs fs;
ext4_mount(&fs, bread, NULL, 0);
```

## Layout

| Directory/File | Purpose |
|----------------|---------|
| `include/ext4/` | Public headers (on-disk layout offsets, API types) |
| `src/` | Library source (string functions, ext4 logic) |
| `githooks/` | Git hooks (pre-commit clang-format) |
| `Makefile` | Build system with compdb generation |

## License

MIT
