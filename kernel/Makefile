CFLAGS = -Wall -Wextra -Werror -Oz -ffreestanding -mno-red-zone -fno-exceptions -fno-asynchronous-unwind-tables
LDFLAGS = -m elf_x86_64 -T linker.ld -nostdlib
O = boot/multiboot.o

all: check_deps clean winuz.iso

check_deps:
	@echo -e "\033[1mChecking packages...\033[0m"
	@for pkg in \
		"mkdir:coreutils mkdir missing" "cp:coreutils cp missing" "rm:coreutils rm missing" \
		"echo:shell echo missing" "grep:grep is not installed" "file:file utility missing" \
		"readelf:readelf utility missing" "objdump:objdump utility missing" "nm:nm utility missing" \
		"grub-file:grub is not installed" "grub-mkrescue:grub utilities missing" \
		"xorriso:xorriso is not installed" "mtools:mtools is not installed" \
		"nasm:nasm is not installed" "clang:clang is not installed" "ld.lld:lld linker missing" \
	; do \
		cmd=$${pkg%%:*}; err=$${pkg#*:}; \
		command -v $$cmd >/dev/null 2>&1 || (echo -e "\033[1mbad: $$err\033[0m" && exit 1); \
		echo -e "\033[1mok, $$cmd found\033[0m"; \
	done
	@echo -e "\033[1mok\033[0m"

winuz.iso: winuz.bin
	grub-file --is-x86-multiboot2 winuz.bin || (echo -e "\033[1mbad: invalid multiboot2 header\033[0m" && exit 1)
	@echo -e "\033[1mok, multiboot2 header valid\033[0m"
	file winuz.bin | grep -q "ELF 64-bit" || (echo -e "\033[1mbad: file is not 64bit\033[0m" && exit 1)
	@echo -e "\033[1mok, file is 64bit\033[0m"
	readelf -h winuz.bin | grep -q "Class:.*ELF64" || (echo -e "\033[1mbad: bad elf64 class\033[0m" && exit 1)
	@echo -e "\033[1mok, elf64 class valid\033[0m"
	readelf -h winuz.bin | grep -q "Machine:.*Advanced Micro Devices X86-64" || (echo -e "\033[1mbad: wrong architecture\033[0m" && exit 1)
	@echo -e "\033[1mok, architecture target matched\033[0m"
	objdump -f winuz.bin | grep -q "architecture: x86_64" || (echo -e "\033[1mbad: target is not x86_64\033[0m" && exit 1)
	@echo -e "\033[1mok, target is x86_64\033[0m"
	nm winuz.bin | grep -q " kmain" || (echo -e "\033[1mbad: missing kmain symbol\033[0m" && exit 1)
	@echo -e "\033[1mok, kmain symbol found\033[0m"
	nm winuz.bin | grep -q " _start" || (echo -e "\033[1mbad: missing start entry\033[0m" && exit 1)
	@echo -e "\033[1mok, start entry found\033[0m"
	mkdir -p iso/boot/grub
	@echo -e "\033[1mok, staging directory created\033[0m"
	cp winuz.bin iso/boot/
	@echo -e "\033[1mok, winuz.bin copied to staging\033[0m"
	@printf '%s\n' \
		'serial --unit=0 --speed=115200' \
		'terminal_input serial' \
		'terminal_output serial' \
		'set timeout=0' \
		'set default=0' \
		'menuentry "Winuz Kernel" {' \
		'    multiboot2 /boot/winuz.bin' \
		'    boot' \
		'}' > iso/boot/grub/grub.cfg
	@echo -e "\033[1mok, grub.cfg generated\033[0m"
	grub-mkrescue -o winuz.iso iso
	@echo -e "\033[1mok, iso image created\033[0m"
	file winuz.iso | grep -q "ISO 9660" || (echo -e "\033[1mbad: target is not valid iso image\033[0m" && exit 1)
	@echo -e "\033[1mok, target is valid iso\033[0m"
	grub-file --is-x86-multiboot2 winuz.bin || (echo -e "\033[1mbad: iso integration check failed\033[0m" && exit 1)
	@echo -e "\033[1mok, integration check passed\033[0m"
	rm -rf iso kernel.o $(O)
	@echo -e "\033[1mok, build files cleaned\033[0m"
	@echo -e "\033[1mok created\033[0m"

winuz.bin: $(O) kernel.o
	ld.lld $(LDFLAGS) $(O) kernel.o -o winuz.bin
	@echo -e "\033[1mok, winuz.bin linked\033[0m"

boot/%.o: boot/%.asm
	mkdir -p boot
	@echo -e "\033[1mok, boot directory ready\033[0m"
	nasm -f elf64 $< -o $@
	@echo -e "\033[1mok, assembly compiled\033[0m"

kernel.o: kernel.c
	clang -target x86_64-unknown-none $(CFLAGS) -v -fverbose-asm -c kernel.c -o kernel.o
	@echo -e "\033[1mok, kernel.c compiled\033[0m"

clean:
	rm -rf *.o *.bin *.iso iso boot/*.o
	@echo -e "\033[1mok, workspace cleaned\033[0m"                                                           