# Alloc
Custom heap memory allocator using `brk` and `sbrk`.

## Usage
- Build with `make`.
- Copy library from `bin/`.
- Visit [test.c](tests/test.c) for example use.

## Warning
- Writing to invalid memory allocated by `liballoc` will break the memory cleanup function.
- Segmentation faults during execution can be attributed to `libc`.
- `libc` will break functionality of library.

## How libc affects liballoc
`libc` uses `brk` and `sbrk` calls to allocate memory, and `liballoc` also uses the same.

As a result, the two libraries will end up corrupting the memory tracking data structures of each other.

This will result in undefined behaviour.

To know how `liballoc` works, visit [working.md](docs/working.md).

## Test results

Test command `make testdbg`.

Tested in `gdb` file `tests/test.c`.
The test runs `7` iterations, each iteration allocating a total of `1GB`, writing a few bytes to it, and then deallocating it.

Breakpoints at lines `11`, `28` and `31`.

### 0th iteration:
 - at line 11: `sbrk(0)` = `0x555555a000`
 - at line 28: `sbrk(0)` = `0x559555a0b8`
 - at line 32: `sbrk(0)` = `0x559555a0b8`

### 1st iteration:
 - at line 11: `sbrk(0)` = `0x555555a018`
 - at line 28: `sbrk(0)` = `0x559555a0b8`
 - at line 31: `sbrk(0)` = `0x559555a0b8`

### Subsequent iterations:
Similar to 1st iteration

### Observations:

Difference in `sbrk(0)` in same iteration b/w lines 28 and 11 = `0x559555a0b8` - `0x555555a018` = `1GB`.
This indicates the allocator is properly allocating blocks.

Difference in `sbrk(0)` in same iteration b/w lines 31 and 28 = `0x559555a0b8` - `0x559555a0b8` = `0B`.
This indicates allocator is not updating `brk` unless the last allocated block is freed.

Difference in `sbrk(0)` b/w two iterations at line 11 of each iteration = `0x555555a018` - `0x555555a000` = `24B` or `0x555555a018` - `0x555555a018` = `0B`.
This indicates the allocator is properly deallocating blocks.

The extra `24B` couldn't be traced.

### Conclusion
Difference in `sbrk(0)` before and after run = `24B`.

Hence, allocator is functioning as expected.
