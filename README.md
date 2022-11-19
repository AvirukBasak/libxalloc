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

## Test results

Test command `make testdbg`.

Tested in `gdb` file `tests/test.c`.

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

### Observation:

Difference in `sbrk(0)` at line 11 i.e. beginning of each loop = `0x555555a000` - `0x555555a018` = `24B`.

### Conclusion
Difference in `sbrk(0)` before and after run = `24B`.

Hence, allocator is functioning as expected.
