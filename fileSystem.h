#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED
#include "disk.h"

//DATA STRUCTURES
typedef struct superblock
{
    // Magic number is a way to tell that the superblock is present,
    // and that the file system is mounted on given disk
    char magicNumber[8];

    int blockSize;
    int numberOfBlocks;
    int numberOfFreeBlocks;
    int numberOfInodeBlocks;
    int pointersPerInode;
    int bitmapLength;
    int dataSegmentPointer;

}SUPERBLOCK;

typedef struct inode
{
    int something;
}INODE;

//FUNCTIONS
/*NOTES fsFormat(): Creates new file system on disk, writes superblock and clears all previous data.
If disk is already mounted, should do nothing.*/
int fsFormat();
void printSuperblock(SUPERBLOCK *superblock);
/*NOTES fsMount(): Examines the disk for a file system. If file system is present, reads superblock and creates
free block bitmap-array*/
int fsMount();



void closeFileSystem();//Here we should release freeBlockBitmap and other dinamically allocated variables
#endif // FILESYSTEM_H_INCLUDED
