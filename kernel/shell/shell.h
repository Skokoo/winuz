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

#ifndef SHELL_H
#define SHELL_H

#include "../io.h"
#include "../vga.h"
#include "../vfs.h"

char cmd_buffer[256];
unsigned int cmd_idx = 0;

static inline unsigned int hash_fnv1a(const char* str, unsigned int max_len) {
    unsigned int hash = 0x811C9DC5;
    for (unsigned int i = 0; i < max_len; i++) {
        if (str[i] == '\0' || str[i] == ' ') break;
        hash ^= (unsigned char)str[i];
        hash *= 0x01000193;
    }
    return hash;
}

static inline void execute_command(void) {
    cmd_buffer[cmd_idx] = '\0';
    newline();

    if (cmd_idx == 0) {
        pr("> ");
        return;
    }

    unsigned int cmd_hash = hash_fnv1a(cmd_buffer, cmd_idx);

    /*
    Note:
    0x41BF7CBE is the pre-calculated FNV-1a 32bit hash value for the string "REBOOT"
    0xE710FA4A is the pre-calculated FNV-1a 32bit hash value for the string "SHWDIR"
    0x7C9861DC is the pre-calculated FNV-1a 32bit hash value for the string "SAY" 
    */    
    switch (cmd_hash) {
        case 0x41BF7CBE:
            outb(0x64, 0xFE);
            while(1) { __asm__ volatile ("hlt"); }
            break;

        case 0xE710FA4A:
            if (root.file_count == 0) {
                pr("[Sys] Directory is empty.");
            } else {
                struct file* file_ptr = root.files;
                struct file* end_ptr = root.files + root.file_count;
                
                while (file_ptr < end_ptr) {
                    pr(file_ptr->name);
                    if (file_ptr->is_dir) {
                        pr("/");
                    }
                    pr("   ");
                    file_ptr++;
                }
            }
            newline();
            break;

        case 0x7C9861DC:
            if (cmd_idx > 4) {
                pr(&cmd_buffer[4]);
            }
            newline();
            break;

        default:
            pr("[sys] Unknown command: ");
            pr(cmd_buffer);
            newline();
            break;
    }

    cmd_idx = 0;
    pr("> ");
}

#endif