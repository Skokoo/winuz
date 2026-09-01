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

static inline int m_str_cmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

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
    0x41BF7CBE is the pre-calculated FNV-1a 32bit hash value for "REBOOT"
    0xE710FA4A is the pre-calculated FNV-1a 32bit hash value for "SHWDIR"
    0x7C9861DC is the pre-calculated FNV-1a 32bit hash value for "SAY" 
    0x70EC43EF is the pre-calculated FNV-1a 32bit hash value for "GOTO"
    0xB67AA316 is the pre-calculated FNV-1a 32bit hash value for "REMOVE"
    */    
    switch (cmd_hash) {
        case 0x41BF7CBE:
            outb(0x64, 0xFE);
            while(1) { __asm__ volatile ("hlt"); }
            break;

        case 0xE710FA4A:
            if (root.file_count == 0) {
                pr("[sys] Directory is empty.");
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

        case 0x70EC43EF:           
            if (cmd_idx > 5) {
                char* target_dir = &cmd_buffer[5];
                int found = 0;
                struct file* file_ptr = root.files;
                struct file* end_ptr = root.files + root.file_count;

                while (file_ptr < end_ptr) {
                    if (file_ptr->is_dir && m_str_cmp(file_ptr->name, target_dir) == 0) {
                        pr("[sys] Moved to directory: ");
                        pr(file_ptr->name);
                        found = 1;
                        break;
                    }
                    file_ptr++;
                }
                if (!found) {
                    pr("[sys] Directory not found.");
                }
            } else {
                pr("[sys] Usage: GOTO [dir_name]");
            }
            newline();
            break;

        case 0xB67AA316:
            if (cmd_idx > 7) {
                char* target_file = &cmd_buffer[7];
                int found = 0;
                struct file* file_ptr = root.files;
                struct file* end_ptr = root.files + root.file_count;

                while (file_ptr < end_ptr) {
                    if (m_str_cmp(file_ptr->name, target_file) == 0) {
                        found = 1;                       
                        struct file* next_file = file_ptr + 1;
                        while (next_file < end_ptr) {
                            *file_ptr = *next_file;
                            file_ptr++;
                            next_file++;
                        }
                        root.file_count--;                       
                        break;
                    }
                    file_ptr++;
                }
                if (!found) {
                    pr("[sys] Object not found.");
                }
            } else {
                pr("[sys] Usage: REMOVE [file_name]");
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