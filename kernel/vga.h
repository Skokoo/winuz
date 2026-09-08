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

void pr(const char* s) {
    if (__builtin_expect(s == 0, 0)) return;

    while (*s) {
        putc_serial(*s++);
    }
}

void sc_bios(void) {
    pr("\033[37;44m");
    pr("\033[2J");
    pr("\033[H");
}

void serial_init(void) {
    static unsigned char is_init = 0;
    if (__builtin_expect(!is_init, 0)) {
        unsigned short ports[] = {0x3F9, 0x3FB, 0x3F8, 0x3F9, 0x3FB, 0x3FA, 0x3FC};
        unsigned char vals[]   = {0x00,  0x80,  0x01,  0x00,  0x03,  0xC7,  0x0B};

        for (int i = 0; i < 7; i++) {
            unsigned char status;
            do {
                __asm__ volatile (
                    "inb %1, %0"
                    : "=a"(status)
                    : "Nd"((unsigned short)0x3FD)
                );
            } while ((status & 0x20) == 0);

            __asm__ volatile (
                "outb %0, %1"
                :
                : "a"(vals[i]), "Nd"(ports[i])
            );
        }
        is_init = 1;
        sc_bios();
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

void serial_backspace(void) {
    putc_serial('\b');
    putc_serial(' ');
    putc_serial('\b');
}

#endif