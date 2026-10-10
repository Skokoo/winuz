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

/* oh man i was so LAZY WHEN WRITING THIS CODE */

static inline unsigned char inb(unsigned short port){
    unsigned char r;
    __asm__ volatile("inb %w1, %b0" : "=a"(r) : "Nd"(port));
    return r;
}

static inline void outb(unsigned short port, unsigned char v){
    __asm__ volatile("outb %b0, %w1" :: "a"(v), "Nd"(port) : "memory");
}

static inline void mcpy64(void* dest, const void* src, unsigned int count)
{
    if(!count) return;
    __asm__ volatile("cld; rep movsq" :: "D"(dest), "S"(src), "c"(count) : "memory");
}

static inline void mset64(void* dest, unsigned long long val, unsigned int count)
{
    if(!count) return;
    __asm__ volatile("cld; rep stosq" :: "D"(dest), "a"(val), "c"(count) : "memory");
}

static inline int ata_read_sectors(unsigned int lba, unsigned char count, unsigned short* buf)
{
    unsigned int* p = (unsigned int*)buf;
    unsigned int total = count;
    if(total == 0) total = 256;

    unsigned int weird;
    __asm__ volatile(
        "andl $0xFFFFFF, %1\n\t"
        "shll $8, %1\n\t"
        "orl %1, %0\n\t"
        : "=a"(weird)
        : "r"(lba), "0"((unsigned int)count)
        : "cc"
    );

    unsigned int to = 1000000;
bzy:
    if(!(inb(0x1F7) & 0x80)) goto ok;
    if(--to == 0) return 0;
    goto bzy;
ok:
    __asm__ volatile("movw $0x1F2, %%dx; outl %0, %%dx" :: "a"(weird) : "dx", "memory");
    outb(0x1F6, 0xE0 | ((lba>>24) & 0x0F));
    outb(0x1F7, 0x20);

    for(unsigned int i = 0; i < total; i++){
        to = 1000000;
drq:
        if((inb(0x1F7) & 0x88) == 0x08) goto go;
        if(--to == 0) return 0;
        goto drq;
go:;
        unsigned int* cur = p + (i*128);
        __asm__ volatile(
            "cld\n\t"
            "rep insl" 
            : "+D"(cur) 
            : "d"(0x1F0), "c"(128) 
            : "memory"
        );
    }
    return 1;
}

static inline int ata_read_sector(unsigned int lba, unsigned short* buf){
    return ata_read_sectors(lba, 1, buf);
}

#endif