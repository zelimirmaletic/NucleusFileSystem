#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED
#include "disk.h"

//DATA STRUCTURES
typedef struct superblock
{
    char magicNumber[8];

    int blockSize;
    int numberOfBlocks;
    int numberOfFreeBlocks;
    int numberOfInodeBlocks;
    int pointersPerInode;

}SUPERBLOCK;

typedef struct inode
{
    int something;
}INODE;

//FUNCTIONS
int fsFormat();
void printSuperblock(SUPERBLOCK *superblock);
#endif // FILESYSTEM_H_INCLUDED
