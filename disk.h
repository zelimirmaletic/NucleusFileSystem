#ifndef DISK_H_INCLUDED
#define DISK_H_INCLUDED


#define DISK_BLOCK_SIZE 5
#define NULL 0

int  diskInitialize( const char *fileName, int numberOfBlocks );
float diskUsage();
//Functions for determining disk size
int  diskSize();
int  diskSizeB();
float diskSizeKB();
float diskSizeKiB();
//Basic I/O functions
void diskRead( int blockNumber, char *data );
void diskWrite( int blockNumber, const char *data );
//Formating and closing disk functions
void diskFormat();//ROOT RIGHTS
void diskClose();

//RAID1 Implementation
void switchToBackupDisk();//ROOT RIGHTS

//Full-disk-encryption implementation
//Here is implemented so called OTFE (On-The-Fly-Encryption)
void encrypt(char *data);
void decrypt(char *dara);

#endif // DISK_H_INCLUDED
