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

#ifndef ROOT_H
#define ROOT_H

#include "../kernel/io.h"
#include "../kernel/vga.h"

volatile unsigned char r_dev = 0;
volatile unsigned char r_wait = 0;

static inline void r_prompt(void) {
    pr("[sys] Entering root mode, you can do anything to this kernel.");
    newline();
    pr("Proceed at your own risk. Continue? (y/n) ");
    r_wait = 1;
}

static inline void r_check(char input) {
    r_wait = 0;
    if (input == 'y' || input == 'Y') {
        r_dev = 1;        
    } else {
        pr("[sys] Cancelled.");
    }
    newline();
}

#endif