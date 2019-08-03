#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED
#define DISK_BLOCK_SIZE 512

int diskOpen(const char *fileName);
int  diskInitialize(int numberOfBlocks );
float diskUsage();
//Functions for determining disk size
int  diskSize();
int  diskSizeB();
float diskSizeKB();
float diskSizeKiB();

//Basic I/O functions
void diskRead( int blockNumber, char *data );
void diskWrite( int blockNumber, const char *data );
void diskUpdate(int blockNumber, int blockPosition, char *data);
//There are special functions for writing/reading superblock and inodes
void diskWriteStructure(int blockNumber, int numberOfArguments, char mode, ...);
void diskReadStructure(int blockNumber, int numberOfArguments, char mode, ...);
//Formating and closing disk functions
void diskFormat();//ROOT RIGHTS
void formatBlock(int blockNumber);
void diskClose();

//RAID1 Implementation
void switchToBackupDisk();//ROOT RIGHTS

//Full-disk-encryption implementation
//Here is implemented so called OTFE (On-The-Fly-Encryption)
//These functions are not visible from outside of disk.h and disk.c
static void encrypt(char *data);
static void decrypt(char *data);

#endif // DISK_H_INCLUDED
