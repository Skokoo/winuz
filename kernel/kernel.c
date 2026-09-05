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

__attribute__((section(".multiboot"), used)) 
const unsigned int multiboot_header[] = {
    0xE85250D6,                
    0,                         
    16,                        
    -(0xE85250D6 + 0 + 16),    
    0,                         
    0,                         
    8                          
};

#include "io.h"
#include "paging.h"
#include "vfs.h"
#include "vga.h"
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
        ring_head = next;
    }
}

static inline unsigned char dequeue_scancode(void) {
    if (ring_head == ring_tail) return 0;
    unsigned char code = ring_buffer[ring_tail & 255];
    ring_tail = (ring_tail + 1) & 255;
    return code;
}

void kmain(unsigned int magic1, unsigned int magic2) {
    if (__builtin_expect(magic1 != 1686005835, 0)) { while(1); }
    if (__builtin_expect(magic2 != 26985, 0)) { while(1); }

    init();
    mv(0);

    root.file_count = 0;
    pr("yey, kernel.");
    newline();
    newline();
    pr("> ");

    unsigned char ctrl_pressed = 0;
    unsigned char shift_pressed = 0;
    unsigned long long loop_counter = 0;

    static const char m[128] = {
        [0x1E] = 'A', [0x30] = 'B', [0x2E] = 'C', [0x20] = 'D', [0x12] = 'E',
        [0x21] = 'F', [0x22] = 'G', [0x23] = 'H', [0x17] = 'I', [0x24] = 'J',
        [0x25] = 'K', [0x26] = 'L', [0x32] = 'M', [0x31] = 'N', [0x18] = 'O',
        [0x19] = 'P', [0x10] = 'Q', [0x13] = 'R', [0x1F] = 'S', [0x14] = 'T',
        [0x16] = 'U', [0x2F] = 'V', [0x11] = 'W', [0x2D] = 'X', [0x15] = 'Y',
        [0x2C] = 'Z', [0x39] = ' '
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
            pr("[sig] Keyboard Interrupted.");
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
                vga_backspace();
            }
            continue;
        }

        if (__builtin_expect(c >= 128, 0)) continue;

        char tgt = m[c];
        if (!tgt) continue;

        if ((unsigned int)p >= 2000) p = 0;
        if (!shift_pressed && tgt >= 'A' && tgt <= 'Z') tgt += 32;
        if (__builtin_expect(cmd_idx < 254, 1)) cmd_buffer[cmd_idx++] = tgt;

        char stream[2] = {tgt, 0};
        pr(stream);
    }
}