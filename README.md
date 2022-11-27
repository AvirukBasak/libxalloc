# X Alloc
Custom heap memory allocator using `brk` and `sbrk`.

## Usage
- Build with `make`.
- Copy library from `target/`.
- Visit [test.c](tests/test.c) for example use.

## Warning
- Writing to invalid memory allocated by `libxalloc` will break the memory cleanup function.
- Segmentation faults during execution can be attributed to `libc`.
- `libc` will break functionality of library.

## How libc affects libxalloc
`libc` uses `brk` and `sbrk` calls to allocate memory, and `libxalloc` also uses the same.

As a result, the two libraries will end up corrupting the memory tracking data structures of each other.

This will result in undefined behaviour.

To know how `libxalloc` works, visit [working.md](docs/working.md).

## Test results
Visit [tests](tests/README.md) for details.
