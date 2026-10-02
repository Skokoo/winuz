#ifndef KALLOC_H
#define KALLOC_H

#include "io.h"
#include "vga.h"

struct small_node {
    struct small_node *next;
};

struct __attribute__((aligned(8))) large_header {
    unsigned long long size;
    unsigned char is_free;
};

static unsigned char pool[8*1024*1024] __attribute__((aligned(4096)));
static struct small_node *bins[9];
static unsigned char *large_ptr;
static unsigned char *end_ptr;

void kalloc_init(void){
    int i;
    unsigned char *p = pool;
    unsigned long long j, block, count;
    struct small_node *cur;

    for(i = 0; i < 9; i++) {
        bins[i] = 0;
    }

    for(i = 0; i < 9; i++){
        block = 8ULL << i;
        count = (256 * 1024) / block;
        bins[i] = (struct small_node*)p;
        cur = bins[i];
        for(j = 1; j < count; j++){
            cur->next = (struct small_node*)(p + j * block);
            cur = cur->next;
        }
        cur->next = 0;
        p += count * block;
    }

    large_ptr = p;
    end_ptr = pool + (8 * 1024 * 1024);
    
    struct large_header *h = (struct large_header*)large_ptr;
    h->size = (end_ptr - large_ptr) - sizeof(struct large_header);
    h->is_free = 1;
}

void* kmalloc(unsigned long long need){
    int idx, i;
    unsigned long long sz;
    struct small_node *n;
    struct large_header *cur, *next;

    if(need == 0) return 0;
    
    need = (need + 7) & ~7ULL;

    if(need <= 2048){
        idx = 0;
        sz = 8;
        while(sz < need){ 
            sz <<= 1; 
            idx++; 
        }
        for(i = idx; i < 9; i++){
            if(bins[i]){
                n = bins[i];
                bins[i] = n->next;
                return (void*)n;
            }
        }
    }

    cur = (struct large_header*)large_ptr;
    while((unsigned char*)cur < end_ptr){
        if(cur->is_free && cur->size >= need){
            if(cur->size >= need + sizeof(struct large_header) + 8){
                next = (struct large_header*)((unsigned char*)cur + sizeof(struct large_header) + need);
                next->size = cur->size - need - sizeof(struct large_header);
                next->is_free = 1;
                cur->size = need;
            }
            cur->is_free = 0;
            return (void*)((unsigned char*)cur + sizeof(struct large_header));
        }
        cur = (struct large_header*)((unsigned char*)cur + sizeof(struct large_header) + cur->size);
    }
    return 0;
}

void kfree(void *ptr){
    int idx;
    unsigned char *p;
    struct small_node *n;
    struct large_header *hdr, *next;

    if(!ptr) return;
    
    p = (unsigned char*)ptr;
    
    if(p < large_ptr){
        idx = (int)((p - pool) / (256 * 1024));
        n = (struct small_node*)p;
        n->next = bins[idx];
        bins[idx] = n;
        return;
    }
    
    hdr = (struct large_header*)(p - sizeof(struct large_header));
    hdr->is_free = 1;
    
    while((unsigned char*)hdr + sizeof(struct large_header) + hdr->size < end_ptr){
        next = (struct large_header*)((unsigned char*)hdr + sizeof(struct large_header) + hdr->size);
        if(!next->is_free) break;
        hdr->size += sizeof(struct large_header) + next->size;
    }
}

#endif