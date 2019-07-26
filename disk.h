#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED


#define DISK_BLOCK_SIZE 5
#define NULL 0

int  diskInitialize( const char *fileName, int numberOfBlocks );
//Functions for determining disk size
int  diskSize();
int  diskSizeB();
float diskSizeKB();
float diskSizeKiB();
//Basic I/O functions
void diskRead( int blockNumber, char *data );
void diskWrite( int blockNumber, const char *data );
//Formating and closing disk functions
void diskFormat();//clear all data from disk--->HARD FORMAT!
void diskClose();

#endif // DISK_H_INCLUDED
