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

__attribute__((section(".text"))) 
const unsigned int multiboot_header[] = {
    0x1BADB002,               
    0x00010003,               
    -(0x1BADB002 + 0x00010003),
    0x00100000,               
    0x00100000,               
    0x00000000,               
    0x00000000,               
    0x00100000                
};

#include "io.h"
#include "paging.h"
#include "vfs.h"
#include "vga.h"
#include "/shell/shell.h"

volatile unsigned char proc_active = 0;

volatile unsigned char ring_buffer[256];
volatile unsigned int ring_head = 0;
volatile unsigned int ring_tail = 0;

void enqueue_scancode(unsigned char code) {
    unsigned int next = (ring_head + 1) % 256;
    if (next != ring_tail) {
        ring_buffer[ring_head] = code;
        ring_head = next;
    }
}

unsigned char dequeue_scancode(void) {
    if (ring_head == ring_tail) return 0;
    unsigned char code = ring_buffer[ring_tail];
    ring_tail = (ring_tail + 1) % 256;
    return code;
}

void kmain(void) {
    init();
    volatile unsigned short* vga_b = (volatile unsigned short*)VGA;

    unsigned long long* b64 = (unsigned long long*)VGA;
    unsigned long long cv = 0x0720072007200720ULL;
    for (int i = 0; i < 500; i++) b64[i] = cv;

    root.file_count = 0;

    pr("yey, kernel.");
    newline();
    newline();
    pr("> ");

    unsigned char ctrl_pressed = 0;
    unsigned char shift_pressed = 0;
    unsigned long long loop_counter = 0;

    static const char m[] = {
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
            unsigned char code = inb(0x60);
            enqueue_scancode(code);
        }

        unsigned char c = dequeue_scancode();
        if (c != 0) {
            if (c == 0x1D) ctrl_pressed = 1;
            else if (c == 0x9D) ctrl_pressed = 0;
            else if (c == 0x2A || c == 0x36) shift_pressed = 1;
            else if (c == 0xAA || c == 0xB6) shift_pressed = 0;

            if (!(c & 0x80)) {
                if (ctrl_pressed && c == 0x2E) {
                    if (proc_active) {
                        proc_active = 0;
                        newline();
                        pr("[sig] Keyboard Interrupted.");
                        newline();
                        pr("> ");
                    }
                    continue;
                }             

                if (c == 0x1C) {
                    execute_command();
                    continue;
                }

                if (!proc_active) {
                    if (c == 0x0E) {
                        if (cmd_idx > 0) {
                            cmd_idx--;
                            vga_backspace();
                        }
                        continue;
                    }

                    if (c < 128) {
                        char tgt = m[c];
                        if (tgt) {
                            if ((unsigned int)p >= 2000) p = 0;
                            if (!shift_pressed && tgt >= 'A' && tgt <= 'Z') {
                                tgt = tgt + 32;
                            }
                            if (cmd_idx < 254) {
                                cmd_buffer[cmd_idx++] = tgt;
                            }
                            vga_b[p++] = (current_color << 8) | (unsigned char)tgt;
                            mv(p);
                        }
                    }
                }
            }
        }

        if (proc_active) {
            loop_counter++;
            if (loop_counter % 5000000 == 0) {
                pr(".");
            }
        }

        __asm__ volatile ("pause");
    }
}