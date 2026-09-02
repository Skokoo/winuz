/* 

   Winuz kernel.
   Copyright (C) 2026 Skokoo

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. 

*/

#ifndef VFS_H
#define VFS_H

#include "io.h"

struct file {
    char name[32];
    unsigned int cluster;
    unsigned int size;
    unsigned char is_dir;
};

struct vfs_root {
    struct file files[64];
    unsigned int file_count;
};

struct vfs_root root;

static inline void ata_wait(void) {
    inb(0x1F7); inb(0x1F7); inb(0x1F7); inb(0x1F7);
}

static inline void ata_read_sector(unsigned int lba, unsigned short* buf) {
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    while (!(inb(0x1F7) & 0x08)) {
        __asm__ volatile ("pause");
    }

    __asm__ volatile (
        "cld\n\t"
        "rep insw"
        : "+D"(buf)
        : "d"(0x1F0), "c"(256)
        : "memory"
    );
    ata_wait();
}

int storage_explore(unsigned int lba_root_dir) {
    unsigned short sector_buf[256];
    unsigned char* byte_buf = (unsigned char*)sector_buf;

    root.file_count = 0;
    ata_read_sector(lba_root_dir, sector_buf);

    unsigned char* entry_ptr = byte_buf;
    const unsigned char* const end_entry_ptr = byte_buf + 512;

    while (entry_ptr < end_entry_ptr) {
        if (*entry_ptr == 0x00) break;
        
        unsigned char attr = *(entry_ptr + 11);
        if (*entry_ptr == 0xE5 || attr == 0x0F) {
            entry_ptr += 32;
            continue;
        }

        struct file* f = root.files + root.file_count;
        int name_idx = 0;

        const char* name_part = (const char*)entry_ptr;
        int i = 0;
        while (i < 8 && name_part[i] != ' ') {
            f->name[name_idx++] = name_part[i++];
        }

        if (!(attr & 0x10)) {
            f->name[name_idx++] = '.';
            const char* ext_part = (const char*)(entry_ptr + 8);
            int j = 0;
            while (j < 3 && ext_part[j] != ' ') {
                f->name[name_idx++] = ext_part[j++];
            }
            if (f->name[name_idx - 1] == '.') {
                name_idx--;
            }
        }
        f->name[name_idx] = '\0';

        f->is_dir = (attr & 0x10) ? 1 : 0;
        f->cluster = *(unsigned short*)(entry_ptr + 26);
        f->size = *(unsigned int*)(entry_ptr + 28);

        root.file_count++;
        if (root.file_count >= 64) break;

        entry_ptr += 32;
    }
    return root.file_count;
}

#endif