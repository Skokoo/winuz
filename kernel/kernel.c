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

#include "io.h"
#include "paging.h"
#include "vfs.h"
#include "vga.h"
#include "idt.h"
#include "shell/shell.h"

volatile unsigned char proc_active = 0;
volatile unsigned char ring_buffer[256];
volatile unsigned int ring_head = 0;
volatile unsigned int ring_tail = 0;
volatile unsigned char r_dev = 0;

static inline void enqueue_scancode(unsigned char code) {
    unsigned int next = (ring_head + 1) & 255;
    if (__builtin_expect(next != ring_tail, 1)) {
        ring_buffer[ring_head & 255] = code;
        __sync_synchronize();
        ring_head = next;
        __sync_synchronize();
    }
}

static inline unsigned char dequeue_scancode(void) {
    if (ring_head == ring_tail) return 0;
    unsigned char code = ring_buffer[ring_tail & 255];
    __sync_synchronize();
    ring_tail = (ring_tail + 1) & 255;
    __sync_synchronize();
    return code;
}

void kmain(unsigned int magic1, unsigned int magic2) {
    serial_init();

    if (__builtin_expect(magic1 != 0x36D76289, 0)) {
        pr("multiboot magic mismatch");
        newline();
        while(1) { __asm__ volatile ("hlt"); }
    }

    idt_init();
    init();

    root.file_count = 0;
    pr("yey, kernel");
    newline();
    newline();
    pr("> ");

    unsigned char ctrl_pressed = 0;
    unsigned char shift_pressed = 0;
    unsigned long long loop_counter = 0;

    static const char m[128] = {
        [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
        [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
        [0x0C] = '-', [0x0D] = '=', [0x0E] = '\b', [0x1C] = '\n',
        [0x1E] = 'A', [0x30] = 'B', [0x2E] = 'C', [0x20] = 'D', [0x12] = 'E',
        [0x21] = 'F', [0x22] = 'G', [0x23] = 'H', [0x17] = 'I', [0x24] = 'J',
        [0x25] = 'K', [0x26] = 'L', [0x32] = 'M', [0x31] = 'N', [0x18] = 'O',
        [0x19] = 'P', [0x10] = 'Q', [0x13] = 'R', [0x1F] = 'S', [0x14] = 'T',
        [0x16] = 'U', [0x2F] = 'V', [0x11] = 'W', [0x2D] = 'X', [0x15] = 'Y',
        [0x2C] = 'Z', [0x39] = ' ', [0x33] = ',', [0x34] = '.', [0x35] = '/'
    };

    while (1) {
        unsigned char status = inb(0x64);
        if (status & 0x01) {
            enqueue_scancode(inb(0x60));
        }

        unsigned char c = dequeue_scancode();

        if (c == 0) {
            if (proc_active && ((++loop_counter & 0x3FFFFF) == 0)) pr(".");
            __asm__ volatile ("pause");
            continue;
        }

        if (c == 0x1D) ctrl_pressed = 1;
        if (c == 0x9D) ctrl_pressed = 0;
        if (c == 0x2A || c == 0x36) shift_pressed = 1;
        if (c == 0xAA || c == 0xB6) shift_pressed = 0;

        if (c & 0x80) continue;

        if (ctrl_pressed && c == 0x2E && proc_active) {
            proc_active = 0;
            newline();
            pr("keyboard Interrupted");
            newline();
            pr("> ");
            continue;
        }
        if (ctrl_pressed && c == 0x2E) continue;

        if (c == 0x1C) {
            execute_command();
            continue;
        }

        if (proc_active) continue;

        if (c == 0x0E) {
            if (cmd_idx > 0) {
                cmd_idx--;
                serial_backspace();
            }
            continue;
        }

        if (__builtin_expect(c >= 128, 0)) continue;
        char tgt = m[c];
        if (!tgt) continue;

        if (!shift_pressed && tgt >= 'A' && tgt <= 'Z') tgt += 32;
        if (__builtin_expect(cmd_idx < 255, 1)) cmd_buffer[cmd_idx++] = tgt;

        char stream[2] = {tgt, 0};
        pr(stream);
    }
}