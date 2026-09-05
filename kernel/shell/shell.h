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
extern volatile unsigned char r_dev;

__attribute__((always_inline)) static inline int m_str_cmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

#include "root.h"

static inline struct file* find_file_node(const char* name) {
    struct file* file_ptr = root.files;
    const struct file* const end_ptr = root.files + root.file_count;
    while (file_ptr < end_ptr) {
        if (m_str_cmp(file_ptr->name, name) == 0) {
            return file_ptr;
        }
        file_ptr++;
    }
    return (void*)0;
}

__attribute__((always_inline)) static inline unsigned int hash_fnv1a(const char* str, unsigned int max_len) {
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
    * Note:
    * 0x41BF7CBE is the pre-calculated FNV-1a 32bit hash value for "REBOOT"
    * 0xE710FA4A is the pre-calculated FNV-1a 32bit hash value for "SHWDIR"
    * 0x7C9861DC is the pre-calculated FNV-1a 32bit hash value for "SAY" 
    * 0x70EC43EF is the pre-calculated FNV-1a 32bit hash value for "GOTO"
    * 0xB67AA316 is the pre-calculated FNV-1a 32bit hash value for "REMOVE"
    * 0x7C81A169 is the pre-calculated FNV-1a 32bit hash value for "CPUID"
    * 0x5B5E05AC is the pre-calculated FNV-1a 32bit hash value for "ROOTED"
    * 0x5B5E0D5B is the pre-calculated FNV-1a 32bit hash value for "ROOTID"
    */    
    switch (cmd_hash) {
        case 0x41BF7CBE:
            outb(0x64, 0xFE);
            while(1) { __asm__ volatile ("hlt"); }
            break;

        case 0xE710FA4A:
            if (root.file_count == 0) {
                pr("directory is empty.");
            } else {
                struct file* file_ptr = root.files;
                const struct file* const end_ptr = root.files + root.file_count;

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

        case 0x7C81A169:
            {
                unsigned int regs[4] = {0};
                __asm__ volatile (
                    "cpuid"
                    : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
                    : "a"(0)
                    : "memory"
                );

                char vendor_string[13];
                char* const raw_ptr = (char* const)&regs[1];

                void* const dest_ptr = (void* const)vendor_string;
                const void* const src_ptr = (const void* const)raw_ptr;
                mcpy64(dest_ptr, src_ptr, 1);

                char* const raw_edx = (char* const)&regs[3];
                void* const dest_edx = (void* const)&vendor_string[4];
                const void* const src_edx = (const void* const)raw_edx;
                mcpy64(dest_edx, src_edx, 1);

                char* const raw_ecx = (char* const)&regs[2];
                void* const dest_ecx = (void* const)&vendor_string[8];
                const void* const src_ecx = (const void* const)raw_ecx;
                mcpy64(dest_ecx, src_ecx, 1);

                vendor_string[12] = '\0';

                pr("[sys] CPU vendor string: ");
                pr(vendor_string);
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
                const struct file* const target = find_file_node(&cmd_buffer[5]);
                if (target && target->is_dir) {
                    pr("moved to directory: ");
                    pr(target->name);
                } else {
                    pr("directory not found.");
                }
            } else {
                pr("usage: GOTO [dir_name]");
            }
            newline();
            break;

        case 0xB67AA316:
            if (cmd_idx > 7) {
                struct file* file_ptr = find_file_node(&cmd_buffer[7]);
                if (file_ptr) {
                    const struct file* const end_ptr = root.files + root.file_count;
                    struct file* next_file = file_ptr + 1;
                    while (next_file < end_ptr) {
                        *file_ptr = *next_file;
                        file_ptr++;
                        next_file++;
                    }
                    root.file_count--;                       
                } else {
                    pr("object not found.");
                }
            } else {
                pr("usage: REMOVE [file_name]");
            }
            newline();
            break;

        case 0x5B5E05AC:
            if (r_dev == 1) {
                pr("system is already running in ROOT mode.");
                newline();
            } else {
                if (cmd_idx > 7) {
                    r_toggle(&cmd_buffer[7]);
                } else {
                    pr("usage: ROOTED YES");
                    newline();
                }
            }
            break;                

        case 0x5B5E0D5B:            
            if (r_dev == 1) {
                pr("1");
            } else {
                pr("0");
            }
            newline();
            break;

        default:
            pr("unknown command: ");
            pr(cmd_buffer);
            newline();
            break;
    }

    cmd_idx = 0;
}

#endif