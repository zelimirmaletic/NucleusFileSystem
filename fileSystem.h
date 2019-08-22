#ifndef FILESYSTEM_H_INCLUDED
#define FILESYSTEM_H_INCLUDED
#include "disk.h"
#define POINTERS_PER_INODE 5

unsigned short treeDepth; //maximal is 2

//DATA STRUCTURES
typedef struct superblock
{
    // Magic number is a way to tell that the superblock is present,
    // and that the file system is mounted on given disk
    char magicNumber[9];
    char operatingSystem[6];
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
    //time information
    char formatTime[30];
    char mountTime[30];
    //logindata ---> to be implemented later!
    //char username[30];
    //char password[30];
}SUPERBLOCK;

typedef struct inode
{
    char fileName[30]; //file or folder name
    unsigned short inodePosition;
    unsigned short inodeSize;
    unsigned short directInodePointers[POINTERS_PER_INODE];
    //Extent implementation
    unsigned short isExtent;
    unsigned short extentLength;
    char fileType; //can be f- for file and d-for directory
    char accessPermisions[6]; //in format r-w-x
    unsigned int fileSize;
    char absoluteAdress[35];
    char creationTime[30];
    char accessTime[30];
    char modificationTime[30];
}INODE;
//FUNCTIONS
/*NOTES fsFormat(): Creates new file system on disk, writes superblock and clears all previous data.
If disk is already mounted, should do nothing.*/
int fsFormat(char sudoRight);
/*NOTES fsMount(): Examines the disk for a file system. If file system is present, reads superblock and creates
free block bitmap-array*/
int fsMount();
//Output functions
void printSuperblock(SUPERBLOCK *superblock);
void printInode(unsigned short inodeNumber);
void printDataBlock(unsigned short blockNumber);
void printFreeNodeBitmap(void);
void printAllFilesAndFolders(char *currentAbsoluteAdress);
void fsDebug(void);
//Functions for INODE manipulation
void writeFreeNodeBitmapToDisk(void);
void readFreeNodeBitmapFromDisk(void);
void clearInode(unsigned short inodeNumber);
void clearCurrentInode(void);
unsigned short fsDeleteInode(unsigned short inodeNumber);
short fsWrite(unsigned short inodeNumber, const char *data);
short fsUpdate(unsigned short inodeNumber, const char *data);
void fsRead(unsigned short inodeNumber);
unsigned short getInodeSize(unsigned short inodeNumber);
short createInode(const char *name, char type, char *currentAbsoluteAdress);
int updateFreeBlockBitmap(char mode, ...); //This function is for inodes
unsigned short getFreeDataBlock(void);
int getBlocksForExtent(unsigned short extentLength);
unsigned short findInodeByFIleName(const char *fileName);
void renameInode(char *oldName, char *newName);
short folderExists(char *folderName, char *currentAbsoluteAdress);
short fileExists(char *fileName, char *currentAbsoluteAdress);
//TIME FUNCTION
void getCurrentTime(char *outputPointer);
void closeFileSystem();//Here we should release freeBlockBitmap and other dinamically allocated variables
#endif // FILESYSTEM_H_INCLUDED
