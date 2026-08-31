CC = clang -target x86_64-pc-elf
LD = ld.lld
CFLAGS = -Wall -Wextra -Werror -O3 -ffreestanding -mno-red-zone -fno-exceptions
LDFLAGS = -T linker.ld -nostdlib

all: winux.bin

winux.bin: kernel.o
	$(LD) $(LDFLAGS) kernel.o -o winuz.elf
	llvm-objcopy -O binary winuz.elf winuz.bin

kernel.o: kernel.c io.h paging.h vfs.h vga.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

run: winux.bin
	qemu-system-x86_64 -kernel winuz.bin

clean:
	rm -f *.o *.bin