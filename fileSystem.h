#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED
#include "disk.h"

//DATA STRUCTURES
typedef struct superblock
{
    // Magic number is a way to tell that the superblock is present,
    // and that the file system is mounted on given disk
    char magicNumber[8];
    char operatingSystem[10];
    unsigned short blockSize;
    unsigned short numberOfBlocks;
    unsigned short numberOfFreeBlocks;
    unsigned short numberOfInodeBlocks;
    unsigned short inodeSize;
    unsigned short numberOfFreeInodes;
    unsigned short pointersPerInode;
    unsigned short bitmapLength;
    unsigned short dataSegmentPointer;
    unsigned short inodeSegmentPointer;
    unsigned short inodeBitmapPointer;
    unsigned short inodeTablePointer;
    unsigned short dataBitmapPointer;
    char creationTime[15];
    char mountTime[15];
}SUPERBLOCK;

typedef struct inode
{
    char magicNumber[8];
    unsigned short inodePosition;
    unsigned short inodeSize;
    unsigned short directInodePointers[5];
    //Extent implementation
    unsigned short isExtent;
    unsigned short extentLength;
    char fileType; //can be f- for file and d-for directory
    char accessPermisions[5]; //in format r-w-x
    unsigned int fileSize;
    char accessTime[15];
    char modificationTime[15];
    char creationTime[15];
    char owner[20];


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
