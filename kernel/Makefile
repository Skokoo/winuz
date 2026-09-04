CFLAGS = -Wall -Wextra -Werror -Oz -ffreestanding -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables
LDFLAGS = -T linker.ld -nostdlib --oformat=binary --strip-all

O = boot/header.o boot/entry.o boot/long_mode.o

all: winuz.bin

winuz.bin: $(O) kernel.o
	ld.lld $(LDFLAGS) $(O) kernel.o -o winuz.bin
	rm -f $(O) kernel.o

boot/%.o: boot/%.asm
	nasm -f elf32 $< -o $@

kernel.o: kernel.c io.h paging.h vfs.h vga.h shell/shell.h shell/root.h
	clang -target x86_64-pc-elf $(CFLAGS) -c kernel.c -o kernel.o

clean:
	rm -f boot/*.o *.o winuz.bin