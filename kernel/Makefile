CC = clang -target x86_64-pc-elf
LD = ld.lld
CFLAGS = -Wall -Wextra -Werror -Oz -ffreestanding -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables
LDFLAGS = -T linker.ld -nostdlib --oformat=binary --strip-all

all: winuz.bin

winuz.bin: kernel.o
	$(LD) $(LDFLAGS) kernel.o -o winuz.bin
	rm -f kernel.o

kernel.o: kernel.c io.h paging.h vfs.h vga.h shell/shell.h shell/root.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

run: winuz.bin
	qemu-system-x86_64 -kernel winuz.bin -m 64M

clean:
	rm -f *.o winuz.bin