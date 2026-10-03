# winuz

> Source code

winuz is a 64bit x86_64 monolithic kernel written in C (and of course, some inline asm).

## deployment & compilation

to build the iso file, execute the following commands:

```bash
git clone https://github.com/Skokoo/winuz
cd winuz/kernel
make clean && make
```

it will flood your terminal with some logs, that's normal. it just checks the binary and then gives you the .iso.

## source code

if you wanna know how it really works, just open the source code. it's all there.

## community

* contribution guidelines: [.github/CONTRIBUTING.md](.github/CONTRIBUTING.md)
* code of conduct: [.github/CODE_OF_CONDUCT.md](.github/CODE_OF_CONDUCT.md)