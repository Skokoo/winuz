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

    for (int i = 0; i < 256; i++) {
        buf[i] = inb(0x1F0) | (inb(0x1F0) << 8);
    }
    ata_wait();
}

int storage_explore(unsigned int lba_root_dir) {
    unsigned short sector_buf[256];
    unsigned char* byte_buf = (unsigned char*)sector_buf;
    
    root.file_count = 0;
    ata_read_sector(lba_root_dir, sector_buf);

    for (int offset = 0; offset < 512; offset += 32) {
        if (byte_buf[offset] == 0x00) break;
        if (byte_buf[offset] == 0xE5) continue;
        if (byte_buf[offset + 11] == 0x0F) continue;

        struct file* f = &root.files[root.file_count];
        
        int name_idx = 0;
        for (int i = 0; i < 8; i++) {
            if (byte_buf[offset + i] != ' ') {
                f->name[name_idx++] = byte_buf[offset + i];
            }
        }
        
        if (!(byte_buf[offset + 11] & 0x10)) {
            f->name[name_idx++] = '.';
            for (int i = 0; i < 3; i++) {
                if (byte_buf[offset + 8 + i] != ' ') {
                    f->name[name_idx++] = byte_buf[offset + 8 + i];
                }
            }
        }
        f->name[name_idx] = '\0';

        f->is_dir = (byte_buf[offset + 11] & 0x10) ? 1 : 0;
        f->cluster = byte_buf[offset + 26] | (byte_buf[offset + 27] << 8);
        f->size = byte_buf[offset + 28] | (byte_buf[offset + 29] << 8) | 
                  (byte_buf[offset + 30] << 16) | (byte_buf[offset + 31] << 24);

        root.file_count++;
        if (root.file_count >= 64) break;
    }
    return root.file_count;
}

#endif