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

#ifndef VGA_H
#define VGA_H

#include "io.h"

#define VGA 0xB8000

int p = 0;
unsigned char current_color = 0x0A;
unsigned char ansi_state = 0;
unsigned int ansi_num = 0;

void mv(int pos) {
    if ((unsigned int)pos >= 2000) return;
    outb(0x3D4, 0x0F); outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E); outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void scroll(void) {
    void* dest = (void*)VGA;
    const void* src = (const void*)(VGA + (80 * 2));
    mcpy64(dest, src, (24 * 80 * 2) / 8);

    void* clear_line = (void*)(VGA + (24 * 80 * 2));
    unsigned long long empty_val = 0x0720072007200720ULL;
    mset64(clear_line, empty_val, (80 * 2) / 8);

    p = 24 * 80;
}

void newline(void) {
    p = ((p / 80) + 1) * 80;
    if (p >= 2000) {
        scroll();
    }
    mv(p);
}

void vga_backspace(void) {
    if (p > 0) {
        p--;
        volatile unsigned short* b = (volatile unsigned short*)VGA;
        b[p] = 0x0700;
        mv(p);
    }
}

void pr(const char* s) {
    volatile unsigned short* b = (volatile unsigned short*)VGA;
    int cur = p;
    static const unsigned char ansi_vga_map[] = {0x00, 0x04, 0x02, 0x06, 0x01, 0x05, 0x03, 0x07};

    while (*s) {
        unsigned char c = (unsigned char)*s++;

        if (ansi_state == 0) {
            if (c == 0x1B) {
                ansi_state = 1;
                continue;
            }
            b[cur++] = (current_color << 8) | c;
            
            if ((unsigned int)cur >= 2000) {
                p = cur;
                scroll();
                cur = p;
                b = (volatile unsigned short*)VGA;
            }
        } else if (ansi_state == 1) {
            if (c == '[') {
                ansi_state = 2;
                ansi_num = 0;
            } else {
                ansi_state = 0;
            }
        } else if (ansi_state == 2) {
            if (c >= '0' && c <= '9') {
                ansi_num = (ansi_num * 10) + (c - '0');
            } else if (c == 'm') {
                if (ansi_num == 0) {
                    current_color = 0x07;
                } else if (ansi_num >= 30 && ansi_num <= 37) {
                    current_color = ansi_vga_map[ansi_num - 30];
                }
                ansi_state = 0;
            } else {
                ansi_state = 0;
            }
        }
    }
    p = cur;
    mv(p);
}

#endif