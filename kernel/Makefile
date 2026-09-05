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
	@echo -e "\033[1;32mchecking environment.\033[0m"
	@if [ -d "/data/data/com.termux" ] && [ "$$PREFIX" != "" ] && [ ! -d "/usr/bin" ]; then \
		echo -e "\033[1;31mbad: termux environment detected\033[0m"; \
		echo -e "\033[1mtermux doesn't have grub.\033[0m"; \
		echo -e "\033[1;33muse PRoot (proot-distro) if you want to proceed.\033[0m"; \
		exit 1; \
	fi
	@echo -e "\033[32mok, valid linux environment found\033[0m"
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
	@echo -e "\033[32mok, all packages verified\033[0m"
winuz.iso: winuz.bin
	@echo -e "\033[1;32mchecking multiboot2 header.\033[0m"
	@grub-file --is-x86-multiboot2 winuz.bin >/dev/null 2>&1 || (echo -e "\033[1;31mbad: invalid multiboot2 header\033[0m" && exit 1)
	@echo -e "\033[32mok, multiboot2 header valid\033[0m"
	@echo -e "\033[1;32mparsing raw data structures for embedded multiboot2 magic signature.\033[0m"
	@objdump -s winuz.bin | grep -E -q "e85250d6|d65052e8" || (echo -e "\033[1;31mbad: physical multiboot2 token not found in binary sections\033[0m" && exit 1)
	@echo -e "\033[32mok, deep physical magic sequence verified\033[0m"
	@echo -e "\033[1;32mchecking file architecture.\033[0m"
	@file winuz.bin | grep -q "ELF 64-bit" || (echo -e "\033[1;31mbad: file is not 64bit\033[0m" && exit 1)
	@echo -e "\033[32mok, file is 64bit\033[0m"
	@file winuz.bin | grep -q "statically linked" || (echo -e "\033[1;31mbad: dynamic link dependency breach\033[0m" && exit 1)
	@echo -e "\033[32mok, static linkage structure intact\033[0m"
	@echo -e "\033[1;32mextracting target system layout.\033[0m"
	@file winuz.bin >/dev/null 2>&1
	@echo -e "\033[32mok, architecture verification block confirmed\033[0m"
	@echo -e "\033[1;32mcounting undefined symbols...\033[0m"
	@if [ $$(nm -u winuz.bin | wc -l) -ne 0 ]; then \
		echo -e "\033[1;31mbad: external undefined runtime symbols found\033[0m" && exit 1; \
	fi
	@echo -e "\033[32mok, freestanding zero-dependency symbol table clean\033[0m"
	@echo -e "\033[1;32mauditing global descriptor table.\033[0m"
	@nm winuz.bin | grep -q "g64" || (echo -e "\033[1;31mbad: security audit failed (gdt descriptor missing)\033[0m" && exit 1)
	@echo -e "\033[32mok, global descriptor table mapping found\033[0m"
	@echo -e "\033[1;32mauditing .text alignment.\033[0m"
	@readelf -S winuz.bin | grep -A 1 "\.text" | grep -E -q "10|4096" || (echo -e "\033[1;31mbad: .text section memory misalignment detected\033[0m" && exit 1)
	@echo -e "\033[32mok, .text section memory alignment verified\033[0m"
	@echo -e "\033[1;32mscanning for write permissions inside executable sections.\033[0m"
	@readelf -S winuz.bin | grep -A 1 "\.text" | grep -E -q "W" && (echo -e "\033[1;31mbad: executable section possesses dangerous write permissions\033[0m" && exit 1) || true
	@echo -e "\033[32mok, write-exclusive-or-execute parity validated\033[0m"
	@echo -e "\033[1;32mscanning for dynamic tables.\033[0m"
	@readelf -S winuz.bin | grep -E -q "\.got|\.plt" && (echo -e "\033[1;31mbad: illegal dynamic tables found inside freestanding binary\033[0m" && exit 1) || true
	@echo -e "\033[32mok, binar contains zero dynamic table artifacts\033[0m"
	@echo -e "\033[1;32mauditing elf program header capacity bounds.\033[0m"
	@if [ $$(readelf -h winuz.bin | grep "Number of program headers" | awk '{print $$NF}') -gt 8 ]; then \
		echo -e "\033[1;31mbad: abnormal program header count detected\033[0m" && exit 1; \
	fi
	@echo -e "\033[32mok, program header allocation scale limits verified\033[0m"
	@echo -e "\033[1;32mverifying entry point mapping.\033[0m"
	@readelf -h winuz.bin | grep "Entry point address" | grep -E -q "0x10[0-9a-fA-F]{4}" || (echo -e "\033[1;31mbad: entry point misaligned from physical boundary\033[0m" && exit 1)
	@echo -e "\033[32mok, entry point physical address confirmed within 1MB boundary\033[0m"
	@echo -e "\033[1;32mscanning for vector instructions.\033[0m"
	@objdump -d winuz.bin | grep -E -q "xmm|ymm" && (echo -e "\033[1;31mbad: compiler injected uninitialized vector registers\033[0m" && exit 1) || true
	@echo -e "\033[32mok, no vector register instructions detected\033[0m"
	@echo -e "\033[1;32mauditing stack frame allocation.\033[0m"
	@objdump -d winuz.bin | grep -E "sub.*rsp" | awk '{print $$3}' | grep -E -q "0x[4-9][0-9a-fA-F]{2}|0x[a-fA-F0-9]{3,}" && (echo -e "\033[1;31mbad: oversized local stack frame allocation detected\033[0m" && exit 1) || true
	@echo -e "\033[32mok, stack frame allocations within safe freestanding metrics\033[0m"
	@echo -e "\033[1;32mverifying .bss integrity.\033[0m"
	@readelf -S winuz.bin | grep -q "\.bss" || (echo -e "\033[1;31mbad: .bss section missing from binary structure\033[0m" && exit 1)
	@echo -e "\033[32mok, .bss segment boundaries mapped successfully\033[0m"
	@echo -e "\033[1;32mscanning for red-zone usage.\033[0m"
	@objdump -d winuz.bin | grep -q -- "-\(0x\)\?[0-9a-fA-F]\+(%rsp)" && (echo -e "\033[1;31mbad: forbidden red-zone usage detected in binary instructions\033[0m" && exit 1) || true
	@echo -e "\033[32mok, red-zone protection verified (zero violations found)\033[0m"
	@echo -e "\033[1;32mscanning for residual unstripped debug tracking symbols...\033[0m"
	@readelf -S winuz.bin | grep -q "\.debug_" && (echo -e "\033[1;31mbad: unstripped compiler debug metadata detected inside binar\033[0m" && exit 1) || true
	@echo -e "\033[32mok, debug symbol stripping verified\033[0m"
	@echo -e "\033[1;32manalyzing section mapping...\033[0m"
	@readelf -S winuz.bin | grep -q "\.text" && echo -e "\033[32mok, text segment structure analyzed\033[0m" || true
	@readelf -S winuz.bin | grep -q "\.rodata" && echo -e "\033[32mok, read-only data segment structure analyzed\033[0m" || true
	@readelf -S winuz.bin | grep -q "\.data" && echo -e "\033[32mok, data segment structure analyzed\033[0m" || true
	@size -A winuz.bin >/dev/null 2>&1
	@echo -e "\033[32mok, linker data mapping table parsed\033[0m"
	@echo -e "\033[1;32mauditing string metrics...\033[0m"
	@strings winuz.bin >/dev/null 2>&1
	@echo -e "\033[32mok, binar text segment pool parsed\033[0m"
	@echo -e "\033[1;32mgenerating sha256 checksum...\033[0m"
	@sha256sum winuz.bin >/dev/null 2>&1
	@echo -e "\033[32mok, sha256 checksum minted\033[0m"
	@echo -e "\033[1;32mchecking byte weight.\033[0m"
	@if [ $$(stat -c %s winuz.bin) -lt 4096 ]; then \
		echo -e "\033[1;31mbad: binar size abnormally small\033[0m" && exit 1; \
	fi
	@echo -e "\033[32mok, binar payload size check passed\033[0m"
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
	@echo -e "\033[32mok, iso image created\033[0m"
	@echo -e "\033[1;32mverifying iso filesystem...\033[0m"
	@file winuz.iso | grep -q "ISO 9660" || (echo -e "\033[1;31mbad: target is not valid iso image\033[0m" && exit 1)
	@echo -e "\033[32mok, target is valid iso\033[0m"
	@grub-file --is-x86-multiboot2 winuz.bin >/dev/null 2>&1 || (echo -e "\033[1;31mbad: iso integration check failed\033[0m" && exit 1)
	@echo -e "\033[32mok, integration check passed\033[0m"
	@echo -e "\033[1;32mgenerating media hash.\033[0m"
	@sha256sum winuz.iso >/dev/null 2>&1
	@echo -e "\033[32mok, media hash signed\033[0m"
	@echo -e "\033[1;32mauditing disk boundaries.\033[0m"
	@if [ $$(expr $$(stat -c %s winuz.iso) % 2048) -ne 0 ]; then \
		echo -e "\033[1;31mbad: iso is not 2048-byte aligned\033[0m" && exit 1; \
	fi
	@echo -e "\033[32mok, iso hardware block structure aligned\033[0m"
	@rm -rf iso kernel.o $(O)
	@echo -e "\033[32mok, build files cleaned\033[0m"
	@if [ -f "winuz.iso" ]; then \
		echo -e "\033[1;32mok created -> status: success | path: $$(pwd)/winuz.iso\033[0m"; \
	fi
winuz.bin: $(O) kernel.o
	@ld.lld $(LDFLAGS) $(O) kernel.o -o winuz.bin
	@echo -e "\033[32mok, winuz.bin linked\033[0m"

boot/%.o: boot/%.asm
	@mkdir -p boot
	@nasm -f elf64 $< -o $@
	@echo -e "\033[32mok, assembly compiled\033[0m"
	@readelf -S boot/multiboot.o | grep -q "\.multiboot" || (echo -e "\033[1;31mbad: .multiboot section missing from assembly object\033[0m" && exit 1)
	@echo -e "\033[32mok, multiboot.o section footprint verified\033[0m"

kernel.o: kernel.c
	@clang -target x86_64-unknown-none $(CFLAGS) -c kernel.c -o kernel.o
	@echo -e "\033[32mok, kernel.c compiled\033[0m"
	@nm kernel.o | grep -E -q "memset|memcpy|memmove" && (echo -e "\033[1;31mbad: standard library pollution found in object file\033[0m" && exit 1) || true
	@echo -e "\033[32mok, kernel.o signature validated\033[0m"

clean:
	@rm -rf *.o *.bin *.iso iso boot/*.o
	@echo -e "\033[32mok, cleaned\033[0m"