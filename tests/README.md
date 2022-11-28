## Test results
Test platform `Termux Linux 4.19.157 aarch64 Android`.

Test command `make testdbg`.

Tested in `gdb`, file [test.c](test.c).
The test runs `7` iterations, each iteration allocating a total of `1 GB`, writing a few bytes to it, and then deallocating it.

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

Difference in `sbrk(0)` in same iteration b/w lines 29 and 12 = `0x559555a0e0` - `0x555555a000` = `1 GB`.
This indicates the allocator is properly allocating blocks.

Difference in `sbrk(0)` in same iteration b/w lines 32 and 29 = `0x559555a0e0` - `0x559555a0e0` = `0 B`.
This indicates allocator is not updating `brk` unless the last allocated block is freed.

Difference in `sbrk(0)` b/w two iterations at line 12 of each iteration = `0x555555a000` - `0x555555a000` = `0 B`.
This indicates the allocator is properly deallocating blocks.

#### Conclusion:
Difference in `sbrk(0)` before and after run = `0 B`.

Hence, allocator is functioning as expected.

#### Notes:
On testing in a `Linux 5.10.147+ x86_64`, difference in `sbrk(0)` before and after run = `132 KB`

It was observed that this allocation happened before the first call to `xmalloc`.

Most likely this was allocated by `libc` as `printf` uses `malloc` and that in turn uses `sbrk` (see next section).

- Address of `sbrk(0)` before run = `0x555555559000`
- Address of 1st allocation of 0th iteration = `0x55555557a000`
- Difference = `0x55555557a000` - `0x555555559000` = `132 KB`

We still can conclude that deallocation is successful as address of 1st allocation of 1st iteration happened after `0 B` of 0th iteration.

## Test Fail Results
Running `make test-fail-dbg`
```
brk init = 0x555555b000
libxalloc: aborted: buffer at '0x555559b050' overflowed

Program received signal SIGABRT, Aborted
```

The address `0x555559b050` is the address of the variable `s1`.

It can be checked by running `p s1` at frame of `main` from `gdb`.

Looking at the code at [`test-fail.c:23`](test-fail.c#L23), note that we are indeed overflowing the buffer of `s1` by `1 B`.

As a result, memory is corrupted, and `xmalloc` at [`test-fail.c:26`](test-fail.c#L26) fails.

## Test (no malloc) Results
File [test-no-malloc.c](test-no-malloc.c) in `gdb`.

The idea is to modify [test.c](test.c), replacing `*alloc` and `free` functions with custom overrides.
This is to prevent `libc` allocators from interfering with `libxalloc`.

The allocator then provides with the allocation [dump](#allocation-dump).

It is observed that the difference in `sbrk(0)` at the end of execution is `0 B`.

#### Observations:
Test platform `Termux Linux 4.19.157 aarch64 Android`:
- First `48 B`, `8 B` and `48 B` allocations are not by `printf`.
- First `48 B` allocation causes allocator initialization.
- Calling `malloc(0)` doesn't do anything in this case.
- Brk init is calculated at this point, before 1st `printf`.
- Brk init is ending address of initial `128 KB` + `40 B` bloc.
- First `printf` causes allocation of `1024 B`.
- After every print, `printf` calls `free(NULL)` for some reason.
- `printf` never frees the initial `1024 B`.
- Brk exit is calculated before 2nd last `printf`.
- In the end, difference in `sbrk(0)` is `0 B`.
- Initial `128 KB` + `40 B` bloc is never freed.

On removal of `printf` calls, difference in `sbrk(0)` is `0 B` as expected.

Test platform `Linux 5.10.147+ x86_64`:
- First `48 B`, `8 B` and `48 B` allocations never happen.
- So, `malloc(0)` causes allocator initialization in this case.
- Otherwise, brk init will end up `128 KB` + `40 B` ahead of brk end.
- Brk init is calculated at this point, before 1st `printf`.
- Brk init is ending address of initial `128 KB` + `40 B` bloc.
- First `printf` causes allocation of `1024 B`.
- `printf` never calls `free(NULL)`.
- `printf` never frees the initial `1024 B`.
- Brk exit is calculated before 2nd last `printf`.
- In the end, difference in `sbrk(0)` is `0 B`.
- Initial `128 KB` + `40 B` bloc is never freed.

#### Conclusion:
- First `printf` allocates `1024 B`.
- `libxalloc` allocates `128 KB` + `40 B` to reduce syscalls.
- `printf` never frees the `1024 B`.
- Extra occupied space at exit = `128 KB` + `40 B`.
- This occupied space can be inspected via `gdb`.

#### References:
- `1 GB` = `1073741824 B`
- `256 MB` = `268435456 B`

#### Allocation dump:
Dump of `make test-no-malloc-dbg`.

Indened stuff is by the allocator, unintended stuff is by printf.
```
    malloc: ptr = '0x5a33187028', size = 48 B
    malloc: ptr = '0x555555b080', size = 8 B
    malloc: ptr = '0x555555b0b0', size = 48 B
    malloc: ptr = '0x555555b108', size = 1024 B
brk init = 0x555555b0e0
    free: ptr = '0x00', size = 0 B, freed = 0 B
    malloc: ptr = '0x555555b530', size = 268435456 B
    malloc: ptr = '0x556555b558', size = 268435456 B
    malloc: ptr = '0x557555b580', size = 268435456 B
    malloc: ptr = '0x558555b5a8', size = 268435456 B
0: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0 B, freed = 0 B
    free: ptr = '0x555555b530', size = 268435456 B, freed = 0 B
    free: ptr = '0x556555b558', size = 268435456 B, freed = 0 B
    free: ptr = '0x557555b580', size = 268435456 B, freed = 0 B
    free: ptr = '0x558555b5a8', size = 268435456 B, freed = 1073741824 B
    malloc: ptr = '0x555555b530', size = 268435456 B
    malloc: ptr = '0x556555b558', size = 268435456 B
    malloc: ptr = '0x557555b580', size = 268435456 B
    malloc: ptr = '0x558555b5a8', size = 268435456 B
1: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0 B, freed = 0 B
    free: ptr = '0x555555b530', size = 268435456 B, freed = 0 B
    free: ptr = '0x556555b558', size = 268435456 B, freed = 0 B
    free: ptr = '0x557555b580', size = 268435456 B, freed = 0 B
    free: ptr = '0x558555b5a8', size = 268435456 B, freed = 1073741824 B
    malloc: ptr = '0x555555b530', size = 268435456 B
    malloc: ptr = '0x556555b558', size = 268435456 B
    malloc: ptr = '0x557555b580', size = 268435456 B
    malloc: ptr = '0x558555b5a8', size = 268435456 B
2: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0 B, freed = 0 B
    free: ptr = '0x555555b530', size = 268435456 B, freed = 0 B
    free: ptr = '0x556555b558', size = 268435456 B, freed = 0 B
    free: ptr = '0x557555b580', size = 268435456 B, freed = 0 B
    free: ptr = '0x558555b5a8', size = 268435456 B, freed = 1073741824 B
    malloc: ptr = '0x555555b530', size = 268435456 B
    malloc: ptr = '0x556555b558', size = 268435456 B
    malloc: ptr = '0x557555b580', size = 268435456 B
    malloc: ptr = '0x558555b5a8', size = 268435456 B
3: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0 B, freed = 0 B
    free: ptr = '0x555555b530', size = 268435456 B, freed = 0 B
    free: ptr = '0x556555b558', size = 268435456 B, freed = 0 B
    free: ptr = '0x557555b580', size = 268435456 B, freed = 0 B
    free: ptr = '0x558555b5a8', size = 268435456 B, freed = 1073741824 B
    malloc: ptr = '0x555555b530', size = 268435456 B
    malloc: ptr = '0x556555b558', size = 268435456 B
    malloc: ptr = '0x557555b580', size = 268435456 B
    malloc: ptr = '0x558555b5a8', size = 268435456 B
4: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0 B, freed = 0 B
    free: ptr = '0x555555b530', size = 268435456 B, freed = 0 B
    free: ptr = '0x556555b558', size = 268435456 B, freed = 0 B
    free: ptr = '0x557555b580', size = 268435456 B, freed = 0 B
    free: ptr = '0x558555b5a8', size = 268435456 B, freed = 1073741824 B
    malloc: ptr = '0x555555b530', size = 268435456 B
    malloc: ptr = '0x556555b558', size = 268435456 B
    malloc: ptr = '0x557555b580', size = 268435456 B
    malloc: ptr = '0x558555b5a8', size = 268435456 B
5: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0 B, freed = 0 B
    free: ptr = '0x555555b530', size = 268435456 B, freed = 0 B
    free: ptr = '0x556555b558', size = 268435456 B, freed = 0 B
    free: ptr = '0x557555b580', size = 268435456 B, freed = 0 B
    free: ptr = '0x558555b5a8', size = 268435456 B, freed = 1073741824 B
    malloc: ptr = '0x555555b530', size = 268435456 B
    malloc: ptr = '0x556555b558', size = 268435456 B
    malloc: ptr = '0x557555b580', size = 268435456 B
    malloc: ptr = '0x558555b5a8', size = 268435456 B
6: abcdefghijklmnopqrstuvwxyz
    free: ptr = '0x00', size = 0 B, freed = 0 B
    free: ptr = '0x555555b530', size = 268435456 B, freed = 0 B
    free: ptr = '0x556555b558', size = 268435456 B, freed = 0 B
    free: ptr = '0x557555b580', size = 268435456 B, freed = 0 B
    free: ptr = '0x558555b5a8', size = 268435456 B, freed = 1073741824 B
brk exit = 0x555555b508
    free: ptr = '0x00', size = 0 B, freed = 0 B
brk difference = 1064 B
    free: ptr = '0x00', size = 0 B, freed = 0 B
```
