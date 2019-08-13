#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED
#include "disk.h"

//DATA STRUCTURES
typedef struct superblock
{
    // Magic number is a way to tell that the superblock is present,
    // and that the file system is mounted on given disk
    char magicNumber[9];
    char operatingSystem[10];
    unsigned short blockSize;
    unsigned short numberOfBlocks;
    unsigned short numberOfFreeBlocks;
    //inode segment
    unsigned short numberOfInodeBlocks;
    unsigned short numberOfFreeInodes;
    unsigned short inodeSegmentPointer;
    unsigned short pointersPerInode;
    //data segment
    unsigned short bitmapLength;
    unsigned short dataSegmentPointer;
    unsigned short freeBlocksBitmapPointer;
}SUPERBLOCK;

typedef struct inode
{
    char fileName[20]; //file or folder name
    unsigned short inodePosition;
    unsigned short inodeSize;
    unsigned short directInodePointers[5];
    //Extent implementation
    unsigned short isExtent;
    unsigned short extentLength;
    char fileType; //can be f- for file and d-for directory
    char accessPermisions[6]; //in format r-w-x
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
void printInode(unsigned short inodeNumber);
/*NOTES fsMount(): Examines the disk for a file system. If file system is present, reads superblock and creates
free block bitmap-array*/
int fsMount();
void printFreeNodeBitmap(void);
void writeFreeNodeBitmapToDisk(void);
void readFreeNodeBitmapFromDisk(void);
void clearInode(unsigned short inodeNumber);
void clearCurrentInode(void);  //NOT IMPLEMENTED!
unsigned short getInodeSize(unsigned short inodeNumber);
int createInode(void); //NOT IMPLEMENTED!
int updateFreeBlockBitmap(char mode, ...);
void closeFileSystem();//Here we should release freeBlockBitmap and other dinamically allocated variables
#endif // FILESYSTEM_H_INCLUDED
