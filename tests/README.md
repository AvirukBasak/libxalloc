## Test results
Test platform `Termux Linux 4.19.157 aarch64 Android`.

Test command `make testdbg`.

Tested in `gdb`, file [test.c](test.c).
The test runs `7` iterations, each iteration allocating a total of `1GB`, writing a few bytes to it, and then deallocating it.

Breakpoints at lines `12`, `29` and `32`.

#### 0th iteration:
 - at line 12: `sbrk(0)` = `0x555555a000`
 - at line 29: `sbrk(0)` = `0x559555a0e0`
 - at line 32: `sbrk(0)` = `0x559555a0e0`

#### 1st iteration:
 - at line 12: `sbrk(0)` = `0x555555a000`
 - at line 29: `sbrk(0)` = `0x559555a0e0`
 - at line 32: `sbrk(0)` = `0x559555a0e0`

#### Subsequent iterations:
Similar to 1st iteration

#### Observations:

Difference in `sbrk(0)` in same iteration b/w lines 29 and 12 = `0x559555a0e0` - `0x555555a000` = `1GB`.
This indicates the allocator is properly allocating blocks.

Difference in `sbrk(0)` in same iteration b/w lines 32 and 29 = `0x559555a0e0` - `0x559555a0e0` = `0B`.
This indicates allocator is not updating `brk` unless the last allocated block is freed.

Difference in `sbrk(0)` b/w two iterations at line 12 of each iteration = `0x555555a000` - `0x555555a000` = `0B`.
This indicates the allocator is properly deallocating blocks.

#### Conclusion
Difference in `sbrk(0)` before and after run = `0B`.

Hence, allocator is functioning as expected.

#### Notes
On testing in a `Linux 5.10.147+ x86_64`, difference in `sbrk(0)` before and after run = `132KB`

It was observed that this allocation happened somewhere before the first call to `xmalloc`.

It's possible this was allocated by `libc` as `printf` uses `malloc` and that in turn uses `sbrk` (see next section).

- Address of `sbrk(0)` before run = `0x555555559000`
- Address of 1st allocation of 0th iteration = `0x55555557a000`
- Difference = `0x55555557a000` - `0x555555559000` = `132KB`

We still can conclude that deallocation is successful as address of 1st allocation of 1st iteration happened after `0B` of 0th iteration.

In any case, `132KB` couldn't be traced.

## Test Fail
Running `make test-fail-dbg`
```
brk init = 0x555555b000
libxalloc: aborted: buffer at '0x555559b050' overflowed

Program received signal SIGABRT, Aborted
```

The address `0x555559b050` is the address of the variable `s1`.

It can be checked by running `p s1` at frame of `main` from `gdb`.

Looking at the code at [`test-fail.c:23`](test-fail.c#L23), note that we are indeed overflowing the buffer of `s1` by `1B`.

As a result, memory is corrupted, and `xmalloc` at [`test-fail.c:26`](test-fail.c#L26) fails.

## Test (no malloc) Results
File [test-no-malloc.c](test-no-malloc.c) in `gdb`.

The idea is to modify [test.c](test.c), replacing `*alloc` and `free` functions with custom overrides.

The overrides provide the following dump.
Inspection of the dump is largely unnecessary.

It is observed that the difference in `sbrk(0)` at the end of execution is `1064 B`.
That is untraceable.

Inspection of the dump gives no clear clues, but we do notice an allocation of `1080 B` during first `printf` call.

Dump of `make test-no-malloc-dbg`:
```
    malloc: ptr = '0x555555b080', size = 0x08 B
    malloc: ptr = '0x555555b0b0', size = 0x30 B
    malloc: ptr = '0x555555b108', size = 0x0400 B
brk init = 0x555555b0e0
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    malloc: ptr = '0x555555b530', size = 0x10000000 B
    malloc: ptr = '0x556555b558', size = 0x10000000 B
    malloc: ptr = '0x557555b580', size = 0x10000000 B
    malloc: ptr = '0x558555b5a8', size = 0x10000000 B
0: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    free: ptr = '0x555555b530', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x556555b558', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x557555b580', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x558555b5a8', size = 0x10000000 B, freed = 0x40000000 B
    malloc: ptr = '0x555555b530', size = 0x10000000 B
    malloc: ptr = '0x556555b558', size = 0x10000000 B
    malloc: ptr = '0x557555b580', size = 0x10000000 B
    malloc: ptr = '0x558555b5a8', size = 0x10000000 B
1: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    free: ptr = '0x555555b530', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x556555b558', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x557555b580', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x558555b5a8', size = 0x10000000 B, freed = 0x40000000 B
    malloc: ptr = '0x555555b530', size = 0x10000000 B
    malloc: ptr = '0x556555b558', size = 0x10000000 B
    malloc: ptr = '0x557555b580', size = 0x10000000 B
    malloc: ptr = '0x558555b5a8', size = 0x10000000 B
2: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    free: ptr = '0x555555b530', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x556555b558', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x557555b580', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x558555b5a8', size = 0x10000000 B, freed = 0x40000000 B
    malloc: ptr = '0x555555b530', size = 0x10000000 B
    malloc: ptr = '0x556555b558', size = 0x10000000 B
    malloc: ptr = '0x557555b580', size = 0x10000000 B
    malloc: ptr = '0x558555b5a8', size = 0x10000000 B
3: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    free: ptr = '0x555555b530', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x556555b558', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x557555b580', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x558555b5a8', size = 0x10000000 B, freed = 0x40000000 B
    malloc: ptr = '0x555555b530', size = 0x10000000 B
    malloc: ptr = '0x556555b558', size = 0x10000000 B
    malloc: ptr = '0x557555b580', size = 0x10000000 B
    malloc: ptr = '0x558555b5a8', size = 0x10000000 B
4: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    free: ptr = '0x555555b530', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x556555b558', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x557555b580', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x558555b5a8', size = 0x10000000 B, freed = 0x40000000 B
    malloc: ptr = '0x555555b530', size = 0x10000000 B
    malloc: ptr = '0x556555b558', size = 0x10000000 B
    malloc: ptr = '0x557555b580', size = 0x10000000 B
    malloc: ptr = '0x558555b5a8', size = 0x10000000 B
5: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    free: ptr = '0x555555b530', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x556555b558', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x557555b580', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x558555b5a8', size = 0x10000000 B, freed = 0x40000000 B
    malloc: ptr = '0x555555b530', size = 0x10000000 B
    malloc: ptr = '0x556555b558', size = 0x10000000 B
    malloc: ptr = '0x557555b580', size = 0x10000000 B
    malloc: ptr = '0x558555b5a8', size = 0x10000000 B
6: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
    free: ptr = '0x555555b530', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x556555b558', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x557555b580', size = 0x10000000 B, freed = 0x00 B
    free: ptr = '0x558555b5a8', size = 0x10000000 B, freed = 0x40000000 B
brk exit = 0x555555b508
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
brk difference = 1064 B
    free: ptr = '0x00', size = 0x00 B, freed = 0x00 B
```
