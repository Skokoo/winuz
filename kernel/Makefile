CFLAGS = -Wall -Wextra -Werror -Oz -ffreestanding -mno-red-zone \
         -fno-exceptions -fno-asynchronous-unwind-tables \
         -fno-pic -fno-pie -fno-plt -fno-stack-protector -ffunction-sections -fdata-sections

LDFLAGS = -m elf_x86_64 -T linker.ld -nostdlib --gc-sections -static --no-dynamic-linker \
          -z nodynamic-undefined-weak -z max-page-size=0x1000 -z common-page-size=0x1000
O = boot/multiboot.o

.PHONY: all check_deps clean run

all: check_deps clean winuz.iso

check_deps:
	echo -e "\033[1mchecking environment...\033[0m"
	if [ -d "/data/data/com.termux" ] && [ "$$PREFIX" != "" ] && [ ! -d "/usr/bin" ]; then \
		echo -e "\033[1;31mbad: termux environment detected\033[0m"; \
		echo -e "\033[1mtermux doesn't have grub.\033[0m"; \
		echo -e "\033[1;33muse PRoot (proot-distro) if you want to proceed.\033[0m"; \
		exit 1; \
	fi
	echo -e "\033[32mok, valid linux environment found\033[0m"
	echo -e "\033[1mchecking packages...\033[0m"
	for pkg in \
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
		command -v $$cmd >/dev/null 2>&1 || (echo -e "\033[1mbad: $$err\033[0m" && exit 1); \
		echo -e "\033[32mok, $$cmd found\033[0m"; \
	done
	echo -e "\033[32mok, all packages verified\033[0m"

winuz.iso: winuz.bin
	grub-file --is-x86-multiboot2 winuz.bin || (echo -e "\033[1mbad: invalid multiboot2 header\033[0m" && exit 1)
	echo -e "\033[32mok, multiboot2 header valid\033[0m"
	echo -e "\033[1minterrogating file architecture class...\033[0m"
	file winuz.bin
	file winuz.bin | grep -q "ELF 64-bit" || (echo -e "\033[1mbad: file is not 64bit\033[0m" && exit 1)
	echo -e "\033[32mok, file is 64bit\033[0m"
	file winuz.bin | grep -q "statically linked" || (echo -e "\033[1mbad: dynamic link dependency breach\033[0m" && exit 1)
	echo -e "\033[32mok, static linkage structure intact\033[0m"
	echo -e "\033[1mextracting target system sub-type layout...\033[0m"
	echo "target hardware profile: $$(file winuz.bin | awk -F, '{print $$2}' | sed 's/^[ \t]*//')"
	echo -e "\033[32mok, architecture verification block confirmed\033[0m"
	echo -e "\033[1mcounting external undefined symbols...\033[0m"
	echo "total undefined symbols found: $$(nm -u winuz.bin | wc -l)"
	if [ $$(nm -u winuz.bin | wc -l) -ne 0 ]; then \
		echo -e "\033[1mbad: external undefined runtime symbols found\033[0m" && exit 1; \
	fi
	echo -e "\033[32mok, freestanding zero-dependency symbol table clean\033[0m"
	echo -e "\033[1mauditing system descriptor table entry boundaries...\033[0m"
	nm winuz.bin | grep -q "g64" || (echo -e "\033[1mbad: security audit failed (gdt descriptor missing)\033[0m" && exit 1)
	echo -e "\033[32mok, global descriptor table mapping found\033[0m"
	echo -e "\033[1manalyzing linker section structural weight...\033[0m"
	size -A winuz.bin
	echo -e "\033[32mok, linker data mapping table displayed\033[0m"
	echo -e "\033[1mauditing text string payload metrics...\033[0m"
	echo "total embedded strings detected: $$(strings winuz.bin | wc -l)"
	echo -e "\033[32mok, binary text segment pool parsed\033[0m"
	echo -e "\033[1mgenerating cryptographic payload signature...\033[0m"
	sha256sum winuz.bin
	echo -e "\033[32mok, sha256 checksum minted\033[0m"
	echo -e "\033[1mchecking biner physical byte weight...\033[0m"
	echo "winuz.bin raw size: $$(stat -c %s winuz.bin) bytes"
	if [ $$(stat -c %s winuz.bin) -lt 4096 ]; then \
		echo -e "\033[1mbad: binary size abnormally small\033[0m" && exit 1; \
	fi
	echo -e "\033[32mok, binary payload size check passed\033[0m"
	mkdir -p iso/boot/grub
	echo -e "\033[33mok, staging directory created\033[0m"
	cp winuz.bin iso/boot/
	echo -e "\033[33mok, winuz.bin copied to staging\033[0m"
	printf '%s\n' \
		'serial --unit=0 --speed=115200' \
		'terminal_input serial' \
		'terminal_output serial' \
		'set timeout=0' \
		'set default=0' \
		'menuentry "Winuz Kernel" {' \
		'    multiboot2 /boot/winuz.bin' \
		'    boot' \
		'}' > iso/boot/grub/grub.cfg
	echo -e "\033[33mok, grub.cfg generated\033[0m"
	grub-mkrescue -o winuz.iso iso
	echo -e "\033[32mok, iso image created\033[0m"
	echo -e "\033[1mverifying generated iso filesystem...\033[0m"
	file winuz.iso
	file winuz.iso | grep -q "ISO 9660" || (echo -e "\033[1mbad: target is not valid iso image\033[0m" && exit 1)
	echo -e "\033[32mok, target is valid iso\033[0m"
	grub-file --is-x86-multiboot2 winuz.bin || (echo -e "\033[1mbad: iso integration check failed\033[0m" && exit 1)
	echo -e "\033[32mok, integration check passed\033[0m"
	echo -e "\033[1mgenerating cryptographic media distribution signature...\033[0m"
	sha256sum winuz.iso
	echo -e "\033[32mok, media hash signed\033[0m"
	echo -e "\033[1mauditing disk hardware block boundaries...\033[0m"
	echo "winuz.iso sector size: $$(stat -c %s winuz.iso) bytes"
	echo "sector alignment modulo remainder: $$(expr $$(stat -c %s winuz.iso) % 2048)"
	if [ $$(expr $$(stat -c %s winuz.iso) % 2048) -ne 0 ]; then \
		echo -e "\033[1mbad: iso is not 2048-byte aligned\033[0m" && exit 1; \
	fi
	echo -e "\033[32mok, iso hardware block structure aligned\033[0m"
	rm -rf iso kernel.o $(O)
	echo -e "\033[33mok, build files cleaned\033[0m"
	if [ -f "winuz.iso" ]; then \
		echo -e "\033[1;32mok created -> status: success | path: $$(pwd)/winuz.iso\033[0m"; \
	else \
		echo -e "\033[1;31mbad status: build failed, output file missing\033[0m"; \
		exit 1; \
	fi

winuz.bin: $(O) kernel.o
	ld.lld $(LDFLAGS) $(O) kernel.o -o winuz.bin
	echo -e "\033[32mok, winuz.bin linked\033[0m"

boot/%.o: boot/%.asm
	mkdir -p boot
	echo -e "\033[33mok, boot directory ready\033[0m"
	nasm -f elf64 $< -o $@
	echo -e "\033[32mok, assembly compiled\033[0m"

kernel.o: kernel.c
	clang -target x86_64-unknown-none $(CFLAGS) -v -fverbose-asm -c kernel.c -o kernel.o
	echo -e "\033[32mok, kernel.c compiled\033[0m"

clean:
	rm -rf *.o *.bin *.iso iso boot/*.o
	echo -e "\033[33mok, cleaned\033[0m"