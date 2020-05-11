
/*
 *  Copyright 2016-2020 Alejandro Calderon Mateos (ARCOS.INF.UC3M.ES)
 *
 *  This file is part of nanofs (nano-filesystem).
 *
 *  nanofs is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  nanofs is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with nanofs.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*
 *  Block Disk
 */

#define DISK "tmpfile.dat"
#define BLOCK_SIZE 1024

int bread ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   fd1 = fopen(devname, "r") ;
   fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
   ret = fread(buffer, BLOCK_SIZE, 1, fd1) ;
   fclose(fd1) ;
   return ret ;
}

int bwrite ( char *devname, int bid, void *buffer )
{
   FILE *fd1 ;
   int ret ;

   fd1 = fopen(devname, "r+") ;
   fseek(fd1, bid*BLOCK_SIZE, SEEK_SET) ;
   ret = fwrite(buffer, BLOCK_SIZE, 1, fd1) ;
   fclose(fd1) ;
   return ret ;
}


/*
 *  Disk layout
 */

#define NUM_INODES 10
#define NUM_DATA_BLOCKS 20

typedef struct {
    unsigned int magicNumber;	            /* Superblock magic number: 0x12345 */
    unsigned int numInodeMapBlocks;         /* Number of block for the inode map */
    unsigned int numDataMapBlocks;          /* Number of block for the data map */
    unsigned int numInodes; 	            /* Number of inodes in the device */
    unsigned int firstInode;	            /* Block number of the first inode (root inode) */
    unsigned int numDataBlocks;             /* Number of data blocks in the device */
    unsigned int firstDataBlock;            /* Block number of the first data block */
    unsigned int devSize;	            /* Total size of the device in bytes */
    char padding[BLOCK_SIZE-8*sizeof(int)]; /* Padding (to fit in a block) */
} Superblock_t;

typedef struct {
    unsigned int type;	                  /* T_FILE o T_DIRECTORY */
    char nombre[200];	                  /* Nombre del fichero/ directorio asociado */
    unsigned int inodesInDir[200];        /* if (type==dir) -> list of the inodes in directory */
    unsigned int size;	                  /* Size in bytes */
    unsigned int directBlock;	          /* Number of the   direct block */
    unsigned int indirectBlock;	          /* Number of the indirect block */
    char padding[BLOCK_SIZE-204*sizeof(int)-200]; /* Padding (to fit in a block) */
} DiskInode_t;
#define PADDING_INODO (BLOCK_SIZE - sizeof(DiskInode_t))

typedef char  inode_map[NUM_INODES] ;         /* 100…0 (used: i_map[x]=1 | free: i_map[x]=0) */
typedef char block_map[NUM_DATA_BLOCKS] ;     /* 000…0 (used: b_map[x]=1 | free: b_map[x]=0) */

#define T_FILE      1
#define T_DIRECTORY 2


/*
 *  Memory layout
 */

// Metadata from disk
Superblock_t sbloques[1] ;
           char i_map[BLOCK_SIZE] ;
           char b_map[BLOCK_SIZE] ;
 DiskInode_t inodos[NUM_INODES] ;

// Extra support metadata (not to be stored on disk)
struct {
    int position ; // read/write seek position
    int oppened  ; // 0: false, 1: true
} inodos_x [NUM_INODES] ;

int is_mounted = 0 ; // 0: false, 1: true


/*
 *
 */

int nanofs_ialloc ( void )
{
    int i;

    // search for a free i-node
    for (i=0; i<sbloques[0].numInodes; i++)
    {
          if (i_map[i] == 0)
          {
              // set inode to used
              i_map[i] = 1;
              // set default values for the inode
              memset(&(inodos[i]),0, sizeof(DiskInode_t));
              // return the inode id.
              return i;
          }
    }

    return -1;
}

int nanofs_alloc ( void )
{
    char b[BLOCK_SIZE];
    int i;

    // search for a free data block
    for (i=0; i<sbloques[0].numDataBlocks; i++)
    {
          if (b_map[i] == 0)
          {
              // data block used now
              b_map[i] = 1;
              // default values for the block
              memset(b, 0, BLOCK_SIZE);
              bwrite(DISK, sbloques[0].firstDataBlock + i, b);
              // return the block id.
              return i;
          }
    }

    return -1;
}

int nanofs_ifree ( int inodo_id )
{
    // check inode id.
    if (inodo_id > sbloques[0].numInodes) {
        return -1;
    }

    // free i-node
    i_map[inodo_id] = 0;

    return -1;
}

int nanofs_free ( int block_id )
{
    // check block id.
    if (block_id > sbloques[0].numDataBlocks) {
        return -1;
    }

    // free block
    b_map[block_id] = 0;

    return -1;
}

int nanofs_namei ( char *fname )
{
   int i;

   // search an i-node with name <fname>
   for (i=0; i<sbloques[0].numInodes; i++)
   {
         if (! strcmp(inodos[i].nombre, fname)) {
               return i;
         }
   }

   return -1;
}

int nanofs_bmap ( int inodo_id, int offset )
{
    int b[BLOCK_SIZE/4] ;
    int logical_block ;

    // check inode id.
    if (inodo_id > sbloques[0].numInodes) {
        return -1;
    }

    // logical block
    logical_block = offset / BLOCK_SIZE ;
    if (logical_block > (BLOCK_SIZE/4)) {
        return -1 ;
    }

    // return direct block
    if (0 == logical_block) {
        return inodos[inodo_id].directBlock;
    }

    // return indirect block
    bread(DISK, sbloques[0].firstDataBlock + inodos[inodo_id].indirectBlock, b);
    return b[logical_block - 1] ;
}


/*
 *
 */

int nanofs_meta_readFromDisk ( void )
{
    // read block 0 from disk to sbloques[0]
    bread(DISK, 0, &(sbloques[0]) );

    // read the blocks where the i-node map is stored
    for (int i=0; i<sbloques[0].numInodeMapBlocks; i++) {
           bread(DISK, 2+i, ((char *)i_map + i*BLOCK_SIZE)) ;
    }

    // read the blocks where the block map is stored
    for (int i=0; i<sbloques[0].numDataMapBlocks; i++) {
          bread(DISK, 2+i+sbloques[0].numInodeMapBlocks, ((char *)b_map + i*BLOCK_SIZE));
    }

    // read i-nodes to memory
    for (int i=0; i<(sbloques[0].numInodes*sizeof(DiskInode_t)/BLOCK_SIZE); i++) {
          bread(DISK, i+sbloques[0].firstInode, ((char *)inodos + i*BLOCK_SIZE));
    }

    return 1;
}

int nanofs_meta_writeToDisk ( void )
{
    // write block 0 to disk from sbloques[0]
    bwrite(DISK, 0, &(sbloques[0]) );

    // write the blocks where the i-node map is stored
    for (int i=0; i<sbloques[0].numInodeMapBlocks; i++) {
           bwrite(DISK, 2+i, ((char *)i_map + i*BLOCK_SIZE)) ;
    }

    // write the blocks where the block map is stored
    for (int i=0; i<sbloques[0].numDataMapBlocks; i++) {
          bwrite(DISK, 2+i+sbloques[0].numInodeMapBlocks, ((char *)b_map + i*BLOCK_SIZE));
    }

    // write i-nodes to disk
    for (int i=0; i<(sbloques[0].numInodes*sizeof(DiskInode_t)/BLOCK_SIZE); i++) {
          bwrite(DISK, i+sbloques[0].firstInode, ((char *)inodos + i*BLOCK_SIZE));
    }

    return 1;
}

int nanofs_meta_setDefault ( void )
{
    // set the default values of the superblock, inode map, etc.
    sbloques[0].magicNumber        = 0x12345; // help to check our mkfs was used
    sbloques[0].numInodes          = NUM_INODES;
    sbloques[0].numInodeMapBlocks  = 1;
    sbloques[0].numDataMapBlocks   = 1;
    sbloques[0].firstInode         = 1;
    sbloques[0].numDataBlocks      = NUM_DATA_BLOCKS;
    sbloques[0].firstDataBlock     = 12;
    sbloques[0].devSize            = 20;

    for (int i=0; i<sbloques[0].numInodes; i++) {
         i_map[i] = 0; // free
    }

    for (int i=0; i<sbloques[0].numDataBlocks; i++) {
         b_map[i] = 0; // free
    }

    for (int i=0; i<sbloques[0].numInodes; i++) {
         memset(&(inodos[i]), 0, sizeof(DiskInode_t) );
    }

    return 1;
}

int nanofs_mount ( void )
{
    if (1 == is_mounted) {
        return -1 ;
    }

    // read the metadata file system from disk
    nanofs_meta_readFromDisk() ;

    // mounted
    is_mounted = 1 ; // 0: false, 1: true

    return 1 ;
}

int nanofs_umount ( void )
{
    // if NOT mounted -> error
    if (0 == is_mounted) {
        return -1 ;
    }

    // if any file is open -> error
    for (int i=0; i<sbloques[0].numInodes; i++) {
    if (1 == inodos_x[i].oppened)
        return -1 ;
    }

    // write the metadata file system into disk
    nanofs_meta_writeToDisk() ;

    // unmounted
    is_mounted = 0 ; // 0: false, 1: true

    return 1 ;
}

int nanofs_mkfs ( void )
{
    char b[BLOCK_SIZE];

    // set default values in memory
    nanofs_meta_setDefault() ;

    // write the default file system into disk
    nanofs_meta_writeToDisk() ;

    // write empty data blocks
    memset(b, 0, BLOCK_SIZE);
    for (int i=0; i < sbloques[0].numDataBlocks; i++) {
         bwrite(DISK, sbloques[0].firstDataBlock + i, b);
    }

    return 1;
}


/*
 *
 */

int nanofs_open ( char *nombre )
{
    int inodo_id ;

    inodo_id = nanofs_namei(nombre) ;
    if (inodo_id < 0) {
        return inodo_id ;
    }

    inodos_x[inodo_id].position = 0;
    inodos_x[inodo_id].oppened  = 1;

    return inodo_id;
}

int nanofs_close ( int fd )
{
     if (fd < 0) {
         return fd ;
     }

     inodos_x[fd].position = 0;
     inodos_x[fd].oppened  = 0;

     return 1;
}

int nanofs_creat ( char *nombre )
{
    int b_id, inodo_id ;

    inodo_id = nanofs_ialloc() ;
    if (inodo_id < 0) {
        return inodo_id ;
    }

    b_id = nanofs_alloc();
    if (b_id < 0) {
        nanofs_ifree(inodo_id);
        return b_id ;
    }

    inodos[inodo_id].type = T_FILE ;
    strcpy(inodos[inodo_id].nombre, nombre);
    inodos[inodo_id].directBlock = b_id ;
    inodos_x[inodo_id].position    = 0;
    inodos_x[inodo_id].oppened     = 1;

    return inodo_id ;
}

int nanofs_unlink ( char * nombre )
{
     int inodo_id ;

     inodo_id = nanofs_namei(nombre) ;
     if (inodo_id < 0) {
         return inodo_id ;
     }

     nanofs_free(inodos[inodo_id].directBlock);
     memset(&(inodos[inodo_id]), 0, sizeof(DiskInode_t));
     nanofs_ifree(inodo_id) ;

    return 1;
}

int nanofs_read ( int fd, char *buffer, int size )
{
     char b[BLOCK_SIZE] ;
     int b_id ;

     if (inodos_x[fd].position+size > inodos[fd].size) {
         size = inodos[fd].size - inodos_x[fd].position;
     }
     if (size <= 0) {
         return 0;
     }

     b_id = nanofs_bmap(fd, inodos_x[fd].position);
     bread(DISK, sbloques[0].firstDataBlock+b_id, b);
     memmove(buffer, b+inodos_x[fd].position, size);
     inodos_x[fd].position += size;

     return size;
}

int nanofs_write ( int fd, char *buffer, int size )
{
     char b[BLOCK_SIZE] ;
     int b_id ;

     if (inodos_x[fd].position+size > BLOCK_SIZE) {
         size = BLOCK_SIZE - inodos_x[fd].position;
     }
     if (size <= 0) {
         return 0;
     }

     b_id = nanofs_bmap(fd, inodos_x[fd].position);
     bread(DISK, sbloques[0].firstDataBlock+b_id, b);
     memmove(b+inodos_x[fd].position, buffer, size);
     bwrite(DISK, sbloques[0].firstDataBlock+b_id, b);
     inodos_x[fd].position += size;
       inodos[fd].size     += size;

     return size;
}


/*
 *  Example Test
 */

int main()
{
   int   ret = 1 ;
   int   fd  = 1 ;
   char *str1 = "hello world..." ;
   char  str2[20] ;

   //
   // mkfs-mount
   //
   if (ret != -1)
   {
       printf(" * nanofs_mkfs() -> ") ;
       ret = nanofs_mkfs() ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_mount() -> ") ;
       ret = nanofs_mount() ;
       printf("%d\n", ret) ;
   }

   //
   // creat-write-close
   //
   if (ret != -1)
   {
       printf(" * nanofs_creat('test1.txt') -> ") ;
       ret = fd = nanofs_creat("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_write(%d,'%s',%ld) -> ", ret, str1, strlen(str1)) ;
       ret = nanofs_write(fd, str1, strlen(str1)) ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_close(%d) -> ", ret) ;
       ret = nanofs_close(fd) ;
       printf("%d\n", ret) ;
   }

   //
   // open-read-close
   //
   if (ret != -1)
   {
       printf(" * nanofs_open('test1.txt') -> ") ;
       ret = fd = nanofs_open("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       memset(str2, 0, 20) ;
       printf(" * nanofs_read(%d,'%s',%d) -> ", ret, str2, 13) ;
       ret = nanofs_read(fd, str2, 13) ;
       printf("%d (%s)\n", ret, str2) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_close(%d) -> ", ret) ;
       ret = nanofs_close(fd) ;
       printf("%d\n", ret) ;
   }

   //
   // unlink-umount
   //
   if (ret != -1)
   {
       printf(" * nanofs_unlink('test1.txt') -> ") ;
       ret = nanofs_unlink("test1.txt") ;
       printf("%d\n", ret) ;
   }

   if (ret != -1)
   {
       printf(" * nanofs_umount() -> ") ;
       ret = nanofs_umount() ;
       printf("%d\n", ret) ;
   }

   return 0 ;
}

