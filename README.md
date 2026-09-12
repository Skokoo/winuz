# winuz
> Source code for the winuz kernel.

winuz is a lightweight, fast, freestanding, monolithic 64bit x86_64 kernel. This kernel features an integrated synchronous execution context loop, and grants users primitive access to the kernel. 

your hardware is the only speed limit.

## deployment & compilation

to build the iso file, execute this following command:

```bash
git clone https://github.com/Skokoo/winuz
cd winuz/kernel

# compile the iso
make clean && make
```
dont worry, the make execution is fast, even though the auditing logs will flood your terminal pipeline.

you should obtain .bin and .iso files once the execution is complete.

## what's actually going on (for now)

> hey you, yes you, is this thing formal?

### bare metal stuff
- no dynamic memory layout here. it's standing free.
- talks directly to the hardware using raw port i/o and manual cr0/cr3/cr4 register tweaks.
- fast isr handlers mixed with inline assembly and direct ata disk i/o.
- drops overhead as much as possible using cpu pause polling and precomputed constant folding.
- super fast o(1) shell routing thanks to fnv1a hashing and __builtin_expect branch hints.
- completely lock-free ring buffers powered by __sync_synchronize atomic barriers.
- everything is kept neat with 4k page-aligned layouts and single-instruction gdt/idt setups.
- uses 64bit bulk ops (like rep movsq, stosq, insw) and bitwise modulo (& 255) to speed things up.
- uses simple linear scanning with increments and zero function calls wherever it makes sense.
- features custom data types like kvar_t (8-byte variants) and kbitset_t for bit-level operations.

### how it's compiled
- optimized for size and cache efficiency via -oz, plus -mno-red-zone for stack reduction.
- stripped to the bone: -mno-mmx/sse/sse2 drops floating registers, and -static kills dynamic linking.
- strictly isolated: compiled with -ffreestanding, -fno-exceptions, and -fno-asynchronous-unwind-tables.
- strips out standard overhead: no stack protectors, and zero got/plt generation using -fno-pic/-fno-pie/-fno-plt.
- linker tuning: forces a 4k page size (-z max-page-size=0x1000) and deletes dead code using --gc-sections, -ffunction-sections, and -fdata-sections.
- absolutely zero undefined symbols and no stdlib pollution (even memset and memcpy are custom-built).

### automated build audits
- instantly checks for multiboot2 magic headers and validates the 64bit elf binary.
- hard blocks against vector instructions (xmm/ymm) and redzone violations.
- super clean binaries: no nop sleds, no sketchy w+x permissions, and no dynamic interpreters.
- zero relocation artifacts (like r_x86_64_relative) and absolutely no .got/.plt tables.
- proper alignment: 32bit word alignment, 4k .text alignment, and forces the entry point within the 1mb mark.
- keeps things ultra-light by capping local stack frames at 255 bytes max.
- storage rules: requires at least a 4kb payload and adheres to standard iso 2048-byte disk block alignment.
- security check: automatically runs a sha256 verification on both the raw binary and the final iso image.

you know, if i added more things in this kernel, i guess, just guess. Hint: this thing above.

## community

contribution:

**[.github/CONTRIBUTING.md](.github/CONTRIBUTING.md)**

code of conduct:

**[.github/CODE_OF_CONDUCT.md](.github/CODE_OF_CONDUCT.md)**