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

struct idt_entry {
    unsigned short low_offset;
    unsigned short selector;
    unsigned char ist;
    unsigned char type_attr;
    unsigned short mid_offset;
    unsigned int high_offset;
    unsigned int reserved;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr idtr;

void idt_set_gate(unsigned char num, unsigned long long base, unsigned short sel, unsigned char flags) {
    idt[num].low_offset = (unsigned short)(base & 0xFFFF);
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].type_attr = flags;
    idt[num].mid_offset = (unsigned short)((base >> 16) & 0xFFFF);
    idt[num].high_offset = (unsigned int)((base >> 32) & 0xFFFFFFFF);
    idt[num].reserved = 0;
}

void exception_handler(unsigned long long vector) {
    pr("cpu exception detected");
    newline();
    pr("vector: ");
    if (vector == 0) pr("0");
    else if (vector == 13) pr("13");
    else if (vector == 14) pr("14");
    else pr("unknown exception");
    newline();
    while (1) { __asm__ volatile ("hlt"); }
}

__attribute__((naked)) void isr0(void) {
    __asm__ volatile (
        "push %%rax\n\t"
        "push %%rcx\n\t"
        "push %%rdx\n\t"
        "push %%rsi\n\t"
        "push %%rdi\n\t"
        "push %%r8\n\t"
        "push %%r9\n\t"
        "push %%r10\n\t"
        "push %%r11\n\t"
        "mov %%rsp, %%rbp\n\t"
        "and $-16, %%rsp\n\t"
        "mov $0, %%rdi\n\t"
        "call exception_handler\n\t"
        "mov %%rbp, %%rsp\n\t"
        "pop %%r11\n\t"
        "pop %%r10\n\t"
        "pop %%r9\n\t"
        "pop %%r8\n\t"
        "pop %%rdi\n\t"
        "pop %%rsi\n\t"
        "pop %%rdx\n\t"
        "pop %%rcx\n\t"
        "pop %%rax\n\t"
        "iretq"
        :
        :
        : "memory"
    );
}

__attribute__((naked)) void isr13(void) {
    __asm__ volatile (
        "add $8, %%rsp\n\t"
        "push %%rax\n\t"
        "push %%rcx\n\t"
        "push %%rdx\n\t"
        "push %%rsi\n\t"
        "push %%rdi\n\t"
        "push %%r8\n\t"
        "push %%r9\n\t"
        "push %%r10\n\t"
        "push %%r11\n\t"
        "mov %%rsp, %%rbp\n\t"
        "and $-16, %%rsp\n\t"
        "mov $13, %%rdi\n\t"
        "call exception_handler\n\t"
        "mov %%rbp, %%rsp\n\t"
        "pop %%r11\n\t"
        "pop %%r10\n\t"
        "pop %%r9\n\t"
        "pop %%r8\n\t"
        "pop %%rdi\n\t"
        "pop %%rsi\n\t"
        "pop %%rdx\n\t"
        "pop %%rcx\n\t"
        "pop %%rax\n\t"
        "iretq"
        :
        :
        : "memory"
    );
}

__attribute__((naked)) void isr14(void) {
    __asm__ volatile (
        "add $8, %%rsp\n\t"
        "push %%rax\n\t"
        "push %%rcx\n\t"
        "push %%rdx\n\t"
        "push %%rsi\n\t"
        "push %%rdi\n\t"
        "push %%r8\n\t"
        "push %%r9\n\t"
        "push %%r10\n\t"
        "push %%r11\n\t"
        "mov %%rsp, %%rbp\n\t"
        "and $-16, %%rsp\n\t"
        "mov $14, %%rdi\n\t"
        "call exception_handler\n\t"
        "mov %%rbp, %%rsp\n\t"
        "pop %%r11\n\t"
        "pop %%r10\n\t"
        "pop %%r9\n\t"
        "pop %%r8\n\t"
        "pop %%rdi\n\t"
        "pop %%rsi\n\t"
        "pop %%rdx\n\t"
        "pop %%rcx\n\t"
        "pop %%rax\n\t"
        "iretq"
        :
        :
        : "memory"
    );
}

void idt_init(void) {
    idtr.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtr.base = (unsigned long long)&idt;

    for (int i = 0; i < 256; i++) {
        idt[i].low_offset = 0;
        idt[i].selector = 0;
        idt[i].ist = 0;
        idt[i].type_attr = 0;
        idt[i].mid_offset = 0;
        idt[i].high_offset = 0;
        idt[i].reserved = 0;
    }

    idt_set_gate(0, (unsigned long long)isr0, 0x08, 0x8E);
    idt_set_gate(13, (unsigned long long)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned long long)isr14, 0x08, 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

#endif