CC = clang -target x86_64-pc-elf
LD = ld.lld
CFLAGS = -Wall -Wextra -Werror -Oz -ffreestanding -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables
LDFLAGS = -T linker.ld -nostdlib --oformat=binary --strip-all

all: winux.bin

winux.bin: kernel.o
	$(LD) $(LDFLAGS) kernel.o -o winux.bin
	rm -f kernel.o

kernel.o: kernel.c io.h paging.h vfs.h vga.h shell/shell.h shell/root.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

run: winux.bin
	qemu-system-x86_64 -kernel winux.bin -m 64M

clean:
	rm -f *.o winux.bin