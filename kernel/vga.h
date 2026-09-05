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

int p = 0;
unsigned char current_color = 0x1F;
unsigned char ansi_state = 0;
unsigned int ansi_num = 0;

static inline void putc_serial(char c) {
    unsigned short port_status = 0x3FD;
    unsigned char status;
    
    do {
        __asm__ volatile (
            "inb %1, %0"
            : "=a"(status)
            : "Nd"(port_status)
        );
    } while ((status & 0x20) == 0);

    unsigned short port_data = 0x3F8;
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"((unsigned char)c), "Nd"(port_data)
    );
}

void mv(int pos) {
    if ((unsigned int)pos >= 2000) return;
    static unsigned char is_init = 0;
    if (__builtin_expect(!is_init, 0)) {
        unsigned short ports[] = {0x3F9, 0x3FB, 0x3F8, 0x3F9, 0x3FB, 0x3FA, 0x3FC};
        unsigned char vals[]   = {0x00,  0x80,  0x01,  0x00,  0x03,  0xC7,  0x0B};

        for (int i = 0; i < 7; i++) {
            __asm__ volatile (
                "outb %0, %1"
                :
                : "a"(vals[i]), "Nd"(ports[i])
            );
        }
        is_init = 1;
    }
}

void scroll(void) {
    putc_serial('\n');
    putc_serial('\r');
}

void newline(void) {
    putc_serial('\n');
    putc_serial('\r');
}

void vga_backspace(void) {
    putc_serial('\b');
    putc_serial(' ');
    putc_serial('\b');
}

void pr(const char* s) {
    if (__builtin_expect(s == 0, 0)) return;
    
    while (*s) {
        putc_serial(*s++);
    }
}

#endif