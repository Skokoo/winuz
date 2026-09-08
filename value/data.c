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

/*
 kvar_t (winuz custom variant var): isolated 8byte dynamic data object capable of selfidentifying its own internal data type.

 kbitset_t (winuz hardware bitset): custom data type designed from scratch for manipulating individual memory bits.

 Implementation:

 struct kvar_t dynamic_var;
    kvar_set_int(&dynamic_var, 12345);       
    unsigned long long bit_buffer[4] = {0};
    struct kbitset_t page_bitmap;
    page_bitmap.storage = bit_buffer;
    page_bitmap.bit_capacity = 256;
    
    kbitset_set(&page_bitmap, 45);
 */

/* Code for future */

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char      winuz_byte_t;
typedef unsigned long long winuz_word_t;

typedef enum {
    TYPE_VOID    = 0x00,
    TYPE_INT     = 0x01,
    TYPE_PTR     = 0x02,
    TYPE_STATUS  = 0x03,
    TYPE_RAW     = 0xFF
} vtype_t;

struct kvar_t {
    vtype_t type;
    winuz_byte_t reserved[7];
    union {
        long long int_val;
        void*     ptr_val;
        int       status_val;
        winuz_byte_t raw_bytes[8];
    } payload;
};

struct kbitset_t {
    winuz_word_t* storage;
    winuz_word_t  bit_capacity;
};

static inline void kvar_set_int(struct kvar_t* var, long long val) {
    if (__builtin_expect(var == (void*)0, 0)) return;
    var->type = TYPE_INT;
    var->payload.int_val = val;
}

static inline long long kvar_get_int(const struct kvar_t* var) {
    if (__builtin_expect(var == (void*)0, 0) || var->type != TYPE_INT) return 0;
    return var->payload.int_val;
}

static inline void kvar_set_ptr(struct kvar_t* var, void* ptr) {
    if (__builtin_expect(var == (void*)0, 0)) return;
    var->type = TYPE_PTR;
    var->payload.ptr_val = ptr;
}

static inline void* kvar_get_ptr(const struct kvar_t* var) {
    if (__builtin_expect(var == (void*)0, 0) || var->type != TYPE_PTR) return (void*)0;
    return var->payload.ptr_val;
}

static inline void kbitset_set(struct kbitset_t* set, winuz_word_t bit_index) {
    if (__builtin_expect(set == (void*)0 || set->storage == (void*)0, 0)) return;
    if (bit_index >= set->bit_capacity) return;
    set->storage[bit_index / 64] |= (1ULL << (bit_index % 64));
}

static inline void kbitset_clear(struct kbitset_t* set, winuz_word_t bit_index) {
    if (__builtin_expect(set == (void*)0 || set->storage == (void*)0, 0)) return;
    if (bit_index >= set->bit_capacity) return;
    set->storage[bit_index / 64] &= ~(1ULL << (bit_index % 64));
}

static inline int kbitset_test(const struct kbitset_t* set, winuz_word_t bit_index) {
    if (__builtin_expect(set == (void*)0 || set->storage == (void*)0, 0)) return 0;
    if (bit_index >= set->bit_capacity) return 0;
    return (set->storage[bit_index / 64] & (1ULL << (bit_index % 64))) ? 1 : 0;
}

#endif