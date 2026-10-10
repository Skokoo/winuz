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

#ifndef VFS_H
#define VFS_H

#include "io.h"

struct file
{
    char name[32];
    unsigned int cluster;
    unsigned int size;
    unsigned char is_dir;
};

struct sfs_inode
{
    unsigned int size;
    unsigned int type;
    unsigned int blocks[12];
};

struct sfs_dirent
{
    char name[28];
    unsigned int inode_id;
};

struct sfs_superblock
{
    unsigned int magic;
    unsigned int total_blocks;
    unsigned int inode_count;
    unsigned int inode_start_block;
    unsigned int data_start_block;
};

struct vfs_root
{
    struct file files[64];
    unsigned int file_count;
};

struct vfs_root root;
static unsigned int cached_inode_start = 0;
__attribute__((aligned(64))) static unsigned char static_scratchpad[1536];

int storage_explore(unsigned int lba_root_dir)
{
    unsigned char* b1 = static_scratchpad;
    struct sfs_dirent* de;
    int i = 0;

    root.file_count = 0;

    if(!ata_read_sector(lba_root_dir, (unsigned short*)b1))
    {
        return -1;
    }

    if(((struct sfs_superblock*)b1)->magic!= 0x47494C41)
    {
        return -2;
    }

    cached_inode_start = ((struct sfs_superblock*)b1)->inode_start_block;

    if(!ata_read_sector(cached_inode_start, (unsigned short*)b1))
    {
        cached_inode_start = 0;
        return -3;
    }

    struct sfs_inode ri;
    mcpy64(&ri, b1, 7);

    if(ri.type!= 2)
    {
        return -4;
    }

    if(!ata_read_sector(ri.blocks[0], (unsigned short*)b1))
    {
        return -5;
    }

    de = (struct sfs_dirent*)b1;

next_entry:
    if(i >= 16)
    {
        goto done;
    }

    if(de[i].inode_id == 0)
    {
        i++;
        goto next_entry;
    }

    if(root.file_count >= 64)
    {
        goto done;
    }

    {
        unsigned int blk = cached_inode_start + (de[i].inode_id * 56) / 512;
        unsigned int off = (de[i].inode_id * 56) & 511;

        if(!ata_read_sectors(blk, 2, (unsigned short*)b1))
        {
            i++;
            goto next_entry;
        }

        struct sfs_inode fi;
        mcpy64(&fi, b1 + off, 7);

        struct file* f = &root.files[root.file_count];
        *(unsigned long long*)(f->name) = *(unsigned long long*)(de[i].name);
        *(unsigned long long*)(f->name+8) = *(unsigned long long*)(de[i].name+8);
        *(unsigned long long*)(f->name+16) = *(unsigned long long*)(de[i].name+16);
        *(unsigned long long*)(f->name+24) = 0;
        f->name[31] = 0;

        f->is_dir = (fi.type == 2);
        f->cluster = de[i].inode_id;
        f->size = fi.size;
        root.file_count++;
    }

    i++;
    goto next_entry;

done:
    return root.file_count;
}

void* storage_read_file(unsigned int inode_id, unsigned char* out_buf)
{
    unsigned char* b2 = static_scratchpad;
    struct sfs_inode in;
    unsigned char* dst = out_buf;
    int i = 0;

    if(!cached_inode_start)
    {
        if(!ata_read_sector(0, (unsigned short*)b2))
        {
            return 0;
        }
        cached_inode_start = ((struct sfs_superblock*)b2)->inode_start_block;
    }

    unsigned int blk = cached_inode_start + (inode_id * 56) / 512;
    unsigned int off = (inode_id * 56) & 511;

    if(!ata_read_sectors(blk, 2, (unsigned short*)b2))
    {
        return 0;
    }

    mcpy64(&in, b2 + off, 7);

    int need = (in.size + 511) >> 9;

read_next:
    if(i >= 12)
    {
        goto read_done;
    }
    if(i >= need)
    {
        goto read_done;
    }
    if(in.blocks[i] == 0)
    {
        goto read_done;
    }

    {
        int cnt = 1;

cnt_loop:
        if(i + cnt >= 12)
        {
            goto cnt_done;
        }
        if(i + cnt >= need)
        {
            goto cnt_done;
        }
        if(in.blocks[i + cnt]!= in.blocks[i] + cnt)
        {
            goto cnt_done;
        }
        cnt++;
        goto cnt_loop;

cnt_done:
        if(!ata_read_sectors(in.blocks[i], cnt, (unsigned short*)dst))
        {
            return 0;
        }
        dst += cnt << 9;
        i += cnt;
    }

    goto read_next;

read_done:
    return out_buf;
}

#endif