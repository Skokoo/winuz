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

#ifndef IO_H
#define IO_H

static inline unsigned char inb(unsigned short port){
    unsigned char r;
    __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void outb(unsigned short port, unsigned char v){
    __asm__ volatile("outb %0, %1" :: "a"(v), "Nd"(port));
}

static inline void io_wait(void){
    __asm__ volatile("outb %%al, $0x80" :: "a"(0));
}

static inline void mcpy64(void* dest, const void* src, unsigned int count){
    if(!count) return;
    __asm__ volatile("cld; rep movsq" :: "D"(dest), "S"(src), "c"(count) : "memory");
}

static inline void mset64(void* dest, unsigned long long val, unsigned int count){
    if(!count) return;
    __asm__ volatile("cld; rep stosq" :: "D"(dest), "a"(val), "c"(count) : "memory");
}

static inline void ata_wait(void){
    inb(0x1F7); inb(0x1F7); inb(0x1F7); inb(0x1F7);
}

static inline int ata_read_sector(unsigned int lba, unsigned short* buf){
    unsigned int timeout = 0;

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    while(!(inb(0x1F7) & 0x08)){
        timeout++;
        if(timeout > 10000000) return 0;
        __asm__ volatile("pause");
    }

    __asm__ volatile("cld; rep insl" :: "D"(buf), "d"(0x1F0), "c"(128) : "memory");
    ata_wait();
    return 1;
}

static inline int ata_read_sectors(unsigned int lba, unsigned char count, unsigned short* buf){
    unsigned int timeout;
    unsigned char i;

    if(!count) return 1;

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, count);
    outb(0x1F3, (unsigned char)lba);
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    for(i=0; i<count; i++){
        timeout = 0;
        while(!(inb(0x1F7) & 0x08)){
            timeout++;
            if(timeout > 10000000) return 0;
            __asm__ volatile("pause");
        }
        __asm__ volatile("cld; rep insl" :: "D"(buf), "d"(0x1F0), "c"(128) : "memory");
        buf += 256;
    }

    ata_wait();
    return 1;
}

#endif