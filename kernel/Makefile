#    Winuz kernel.
#    Copyright (C) 2026 Skokoo
# 
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
# 
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
# 
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. 

CFLAGS = -Wall -Wextra -Werror -Oz -ffreestanding -mno-red-zone \
         -mno-mmx -mno-sse -mno-sse2 \
         -fno-exceptions -fno-asynchronous-unwind-tables \
         -fno-pic -fno-pie -fno-plt -fno-stack-protector -ffunction-sections -fdata-sections

LDFLAGS = -m elf_x86_64 -T linker.ld -nostdlib --gc-sections -static --no-dynamic-linker \
          -z nodynamic-undefined-weak -z max-page-size=0x1000 -z common-page-size=0x1000
O = boot/multiboot.o

.PHONY: all check_deps clean run

all: check_deps clean winuz.iso

check_deps:
	@echo -e "\033[1;32mchecking packages.\033[0m"
	@for pkg in \
		"mkdir:coreutils mkdir missing" "cp:coreutils cp missing" "rm:coreutils rm missing" \
		"echo:shell echo missing" "grep:grep is not installed" "file:file utility missing" \
		"readelf:readelf utility missing" "objdump:objdump utility missing" "nm:nm utility missing" \
		"grub-file:grub is not installed" "grub-mkrescue:grub utilities missing" \
		"xorriso:xorriso is not installed" "mtools:mtools is not installed" \
		"nasm:nasm is not installed" "clang:clang is not installed" "ld.lld:lld linker missing" \
		"qemu-system-x86_64:qemu emulator missing" "stat:coreutils stat missing" \
		"wc:coreutils wc missing" "dd:coreutils dd missing" "cksum:coreutils cksum missing" \
		"size:binutils size missing" "strings:binutils strings missing" \
		"sha256sum:coreutils sha256sum missing" \
	; do \
		cmd=$${pkg%%:*}; err=$${pkg#*:}; \
		command -v $$cmd >/dev/null 2>&1 || (echo -e "\033[1;31mbad: $$err\033[0m" && exit 1); \
	done
	@echo -e "\033[32mall packages good\033[0m"

winuz.iso: winuz.bin
	@mkdir -p iso/boot/grub
	@cp winuz.bin iso/boot/
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
	@grub-mkrescue -o winuz.iso iso >/dev/null 2>&1
	@rm -rf iso
	@if [ -f "winuz.iso" ]; then \
		echo -e "\033[1;32mcreated, path: $$(pwd)/winuz.iso\033[0m"; \
	fi

winuz.bin: $(O) kernel.o
	@ld.lld $(LDFLAGS) $(O) kernel.o -o winuz.bin
	@echo -e "\033[32mlinked\033[0m"

boot/%.o: boot/%.asm
	@mkdir -p boot
	@nasm -f elf64 $< -o $@
	@echo -e "\033[32massembly compiled\033[0m"

kernel.o: kernel.c
	@echo -e "\033[1;32mcompiling\033[0m"
	@clang -target x86_64-unknown-none $(CFLAGS) -c kernel.c -o kernel.o
	@echo -e "\033[32mcompiled\033[0m"

clean:
	@rm -rf *.o *.bin *.iso iso boot/*.o
	@echo -e "\033[32mcleaned\033[0m"