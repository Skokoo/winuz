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

#include "../io.h"
#include "../vga.h"

extern volatile unsigned char r_dev;

static inline void r_toggle(const char* param) {
    if (m_str_cmp(param, "YES") == 0) {
        r_dev = 1;
        pr("[sys] Entering root mode, you can do anything to this kernel.");
        newline();
        pr("Proceed at your own risk.");       
    } else {
        pr("[sys] Invalid parameter. Usage: ROOTED YES");
    }
    newline();
}

#endif