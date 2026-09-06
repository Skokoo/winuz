# winuz
> Source code for the winuz kernel.

winuz is a lightweight, fast, freestanding, monolithic 64bit x86_64 kernel. This kernel features an integrated synchronous execution context loop, and grants users primitive access to the kernel.

## deployment & compilation

to build the ISO file, execute this following command:

```bash
git clone https://github.com/Skokoo/winuz
cd winuz/kernel

# compile the iso
make clean && make
```
dont worry, the make execution is fast, even though the auditing logs will flood your terminal pipeline.

you should obtain .bin and .iso files once the execution is complete.

## community

contribution:

**[.github/CONTRIBUTING.md](.github/CONTRIBUTING.md)**

code of conduct:

**[.github/CODE_OF_CONDUCT.md](.github/CODE_OF_CONDUCT.md)**
