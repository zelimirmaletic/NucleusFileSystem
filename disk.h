#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED

#define DISK_BLOCK_SIZE 5

int  diskInitialize( const char *fileName, int numberOfBlocks );
int  diskSize();
void diskRead( int blockNumber, char *data );
void diskWrite( int blockNumber, const char *data );
void diskFormat();//clear all data from disk!
void diskClose();

#endif // DISK_H_INCLUDED
