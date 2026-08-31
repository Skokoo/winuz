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

static inline unsigned char inb(unsigned short p) {
    unsigned char r;
    __asm__ volatile ("inb %1, %0" : "=a"(r) : "Nd"(p) : "memory");
    return r;
}

static inline void outb(unsigned short p, unsigned char v) {
    __asm__ volatile ("outb %0, %1" :: "a"(v), "Nd"(p) : "memory");
}

static inline void io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" :: "a"(0) : "memory");
}

static inline void mcpy64(void* dest, const void* src, unsigned int count) {
    __asm__ volatile ("cld\n\t" "rep movsq" : "+D"(dest), "+S"(src), "+c"(count) :: "memory");
}

static inline void mset64(void* dest, unsigned long long val, unsigned int count) {
    __asm__ volatile ("cld\n\t" "rep stosq" : "+D"(dest), "+c"(count) : "a"(val) : "memory");
}

#endif