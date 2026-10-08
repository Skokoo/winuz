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

extern int ata_read_sector(unsigned int lba, unsigned short* buf);
extern int ata_read_sectors(unsigned int lba, unsigned char count, unsigned short* buf);

int storage_explore(unsigned int lba_root_dir)
{
    int i, k, count;
    unsigned int inode_block, offset, data_block;
    struct sfs_superblock *sb;
    struct sfs_inode ri;
    struct sfs_dirent de[512 / sizeof(struct sfs_dirent)];
    struct file *f;
    unsigned char *buf1;

    buf1 = static_scratchpad;
    root.file_count = 0;

    if(!ata_read_sector(lba_root_dir, (unsigned short*)buf1))
    {
        return -1;
    }

    sb = (struct sfs_superblock*)buf1;
    if(sb->magic!= 0x47494C41)
    {
        return -2;
    }

    cached_inode_start = sb->inode_start_block;

    if(!ata_read_sector(cached_inode_start, (unsigned short*)buf1))
    {
        cached_inode_start = 0;
        return -3;
    }

    ri = *(struct sfs_inode*)buf1;
    if(ri.type!= 2)
    {
        return -4;
    }

    data_block = ri.blocks[0];

    if(!ata_read_sector(data_block, (unsigned short*)buf1))
    {
        return -5;
    }

    count = 512 / sizeof(struct sfs_dirent);

    for(i=0; i<count; i++)
    {
        de[i] = ((struct sfs_dirent*)buf1)[i];
    }

    for(i=0; i<count; i++)
    {
        if(de[i].inode_id == 0) continue;
        if(root.file_count >= 64) break;

        inode_block = cached_inode_start + (de[i].inode_id * sizeof(struct sfs_inode)) / 512;
        offset = (de[i].inode_id * sizeof(struct sfs_inode)) % 512;

        if(!ata_read_sectors(inode_block, 2, (unsigned short*)buf1)) continue;

        struct sfs_inode fi = *(struct sfs_inode*)(buf1 + offset);
        f = &root.files[root.file_count];

        for(k=0; k<28; k++)
        {
            f->name[k] = de[i].name[k];
            if(de[i].name[k] == 0) break;
        }
        f->name[31] = 0;

        f->is_dir = (fi.type == 2)? 1 : 0;
        f->cluster = de[i].inode_id;
        f->size = fi.size;
        root.file_count++;
    }

    return root.file_count;
}

void* storage_read_file(unsigned int inode_id, unsigned char* out_buf)
{
    int i;
    unsigned int b, off, need, burst_cnt;
    struct sfs_superblock *sb;
    struct sfs_inode in;
    unsigned char *dst;
    unsigned char *buf2;

    if(cached_inode_start == 0)
    {
        if(!ata_read_sector(0, (unsigned short*)static_scratchpad)) return 0;
        sb = (struct sfs_superblock*)static_scratchpad;
        cached_inode_start = sb->inode_start_block;
    }

    buf2 = static_scratchpad;
    b = cached_inode_start + (inode_id * sizeof(struct sfs_inode)) / 512;
    off = (inode_id * sizeof(struct sfs_inode)) % 512;

    if(!ata_read_sectors(b, 2, (unsigned short*)buf2)) return 0;
    in = *(struct sfs_inode*)(buf2 + off);

    need = (in.size + 511) / 512;
    dst = out_buf;

    i = 0;
    while(i < 12 && i < (int)need)
    {
        if(in.blocks[i] == 0) break;

        burst_cnt = 1;
        while((i + burst_cnt < 12) &&
              (i + burst_cnt < need) &&
              (in.blocks[i + burst_cnt] == in.blocks[i] + burst_cnt))
        {
            burst_cnt++;
        }

        if(burst_cnt > 1)
        {
            if(!ata_read_sectors(in.blocks[i], (unsigned char)burst_cnt, (unsigned short*)dst)) return 0;
            dst = dst + (512 * burst_cnt);
            i = i + burst_cnt;
        }
        else
        {
            if(!ata_read_sector(in.blocks[i], (unsigned short*)dst)) return 0;
            dst = dst + 512;
            i = i + 1;
        }
    }

    return out_buf;
}

#endif