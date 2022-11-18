# Alloc
Custom heap memory allocator using `sbrk` and `mmap`.

## Usage
- Build with `make`.
- Copy library from `bin/`.
- Visit [test.c](tests/test.c) for example use.

## Warning
- Writing to invalid memory allocated by `liballoc` will break the memory cleanup function.
- segmentation faults during execution can be attributed to `libc`.
- `libc` will break functionality of library.
