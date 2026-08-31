/* 

   Winux kernel.
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

#ifndef PAGING_H
#define PAGING_H

__attribute__((aligned(4096))) unsigned long long l4[512];
__attribute__((aligned(4096))) unsigned long long dp[512];
__attribute__((aligned(4096))) unsigned long long pd[512];

void init(void) {
    __asm__ volatile (
        "xor %%rax, %%rax\n\t"
        "mov $512, %%rcx\n\t"
        "rep stosq"
        :
        : "D"(l4)
        : "rax", "rcx", "memory"
    );
    __asm__ volatile (
        "xor %%rax, %%rax\n\t"
        "mov $512, %%rcx\n\t"
        "rep stosq"
        :
        : "D"(dp)
        : "rax", "rcx", "memory"
    );
    __asm__ volatile (
        "xor %%rax, %%rax\n\t"
        "mov $512, %%rcx\n\t"
        "rep stosq"
        :
        : "D"(pd)
        : "rax", "rcx", "memory"
    );

    l4[0] = (((unsigned long long)dp) & 0x000FFFFFFFFFF000ULL) | 0x03ULL;
    dp[0] = (((unsigned long long)pd) & 0x000FFFFFFFFFF000ULL) | 0x03ULL;

    unsigned long long addr = 0x83ULL | (1ULL << 63); 
    for (int i = 0; i < 512; i++) {
        pd[i] = addr;
        addr += 0x200000ULL;
    }

    __asm__ volatile ("mov %0, %%cr3" :: "r"(l4) : "memory");

    __asm__ volatile (
        "mov %%cr4, %%rax\n\t"
        "or $0x003000B0, %%rax\n\t" 
        "mov %%rax, %%cr4\n\t"
        "mov $0xC0000080, %%ecx\n\t"
        "rdmsr\n\t"
        "or $0x00000900, %%eax\n\t" 
        "wrmsr\n\t"
        "mov %%cr0, %%rax\n\t"
        "movabs $0xFFFFFFFF9FFFFFFF, %%rcx\n\t"
        "and %%rcx, %%rax\n\t"
        "movabs $0x80010023, %%rcx\n\t"
        "or %%rcx, %%rax\n\t"
        "mov %%rax, %%cr0"
        ::: "rax", "rcx", "rdx", "memory"
    );
}

#endif