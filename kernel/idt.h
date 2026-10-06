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

#ifndef IDT_H
#define IDT_H
#include "vga.h"

/* man, if computer was fast at computation, why wouldn't we use that opportunity? */

struct idt_entry {
    unsigned short off_low;
    unsigned short sel;
    unsigned char ist;
    unsigned char flags;
    unsigned short off_mid;
    unsigned int off_high;
    unsigned int rsv;
} __attribute__((packed));

struct {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed)) idtr;

__attribute__((aligned(64))) struct idt_entry idt[256];

static inline void idt_set(int vec, void *isr){
    unsigned long long a = (unsigned long long)isr;
    *(unsigned __int128*)&idt[vec] = (unsigned __int128)(a & 0xFFFF) | (unsigned __int128)8 << 16 | (unsigned __int128)0x8E << 40 | (unsigned __int128)(a & 0xFFFF0000) << 32 | (unsigned __int128)(a >> 32) << 64;
}

__attribute__((noreturn)) void fault_c(unsigned long long vec, unsigned long long err){
    (void)err;
    __asm__ volatile("mov $0xE9, %%dx; mov %0, %%al; out %%al, %%dx" :: "r"((char)('0'+vec)) : "dx","al");
    __asm__ volatile("cli; 1: hlt; jmp 1b" ::: "memory");
    __builtin_unreachable();
}

__attribute__((naked)) void isr0(){ __asm__ volatile("push $0; push $0; jmp isr_common"); }
__attribute__((naked)) void isr13(){ __asm__ volatile("push $13; jmp isr_common"); }
__attribute__((naked)) void isr14(){ __asm__ volatile("push $14; jmp isr_common"); }
__attribute__((naked)) void isr_common(){ __asm__ volatile("mov 16(%%rsp), %%rdi; mov 24(%%rsp), %%rsi; call fault_c" ::: "memory"); }

void idt_init(void){
    void *p = idt;
    __asm__ volatile("xor %%eax, %%eax; mov $512, %%ecx; rep stosq" : "+D"(p) : : "rax","rcx","memory");
    idt_set(0, isr0);
    idt_set(13, isr13);
    idt_set(14, isr14);
    __asm__ volatile("sub $10, %%rsp; movw $4095, (%%rsp); mov %0, 2(%%rsp); lidt (%%rsp); add $10, %%rsp; sti" :: "r"(idt) : "memory");
}
#endif