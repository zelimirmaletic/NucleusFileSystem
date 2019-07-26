#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "disk.h"

//GLOBAL CONSTANTS
static FILE *stream = 0;
static FILE *streamBackup = 0;
//Kepp track of basic disk data and I/O operations
static int numberOfBlocks = 0;
static int numberOfReads = 0;
static int numberOfWrites = 0;

/*
*******************NOTE ABOUT EMULATED DISK SECURITY******************************
-This emulation of physical hard drive has a built-in security.
-Security is achieved using RAID1.
-In this implementation we have only one backup drive(binary file), whereas in
practice there can be multiple backup hard drvies.
-This security implementation has more explanatory than practical purpose.
-In implemetation of functions there will be commented which part of each function
is responsible for RAID1 implementation.
**********************************************************************************
*/


int diskInitialize(const char *fileName, int number)
{
    if( (number<0) | (number>4e6) )
    {
        printf("ERROR: Maximal disk size is 20 MB!\n");
        printf("ERROR: Allocation failure!\n");
        return 0;
    }
    stream = fopen(fileName,"r+");
    if (!stream)
        stream = fopen(fileName,"w+");
    if(!stream)
        return 0;
    numberOfBlocks = number;
    //ftruncate() - Function that shortens file to desired length
    //fileno() -  It shall return the integer file descriptor associated with the stream pointed to by stream
    ftruncate(fileno(stream), numberOfBlocks*DISK_BLOCK_SIZE);

    //******************RAID1 IMPLEMENTATION*************************
    streamBackup = fopen("BackupDisk.bin","r+");
    if (!streamBackup)
        streamBackup = fopen("BackupDisk.bin","w+");
    if(!streamBackup)
        return 0;
    ftruncate(fileno(streamBackup), numberOfBlocks*DISK_BLOCK_SIZE);
    //***************************************************************
    return 1;
}

//Functions for determining disk size
int diskSize()
{
    return numberOfBlocks;
}

int diskSizeB()
{
    return numberOfBlocks*5;
}
float diskSizeKB()
{
    return (numberOfBlocks*5)/10e3;
}
float diskSizeKiB()
{
    return (numberOfBlocks*5)/1024.00;
}

static void validityCheck(int blockNumber, const char *data)
{
    if(blockNumber<0)
    {
        printf("ERROR: Block number %d is negative!\n", blockNumber);
        abort();
    }
    else if(blockNumber>numberOfBlocks)
    {
        printf("ERROR: Block number %d is out of range!(Maximal block number is %d)\n", blockNumber, numberOfBlocks);
        abort();
    }
    else if(!data)
    {
        printf("ERROR: Input data pointer is NULL!\n");
        abort();
    }
}

//Basic I/O functions

void diskRead(int blockNumber, char *data)
{
    validityCheck(blockNumber, data);
    rewind(stream);
    fseek(stream,blockNumber*DISK_BLOCK_SIZE,SEEK_SET);
    if(fread(data,DISK_BLOCK_SIZE,1,stream)==1)
        numberOfReads++;
    else
    {
        printf("ERROR: Could not access emulated disk!\n Aborting...\n");
        abort();
    }
}

void diskWrite(int blockNumber,const char *data)
{
    validityCheck(blockNumber, data);
    rewind(stream);
    fseek(stream, blockNumber*DISK_BLOCK_SIZE, SEEK_SET);
    if(fwrite(data, DISK_BLOCK_SIZE, 1, stream)==1)
        numberOfWrites++;
    else
    {
        printf("ERROR: Could not access emulated disk!\n Aborting...\n");
        abort();
    }

    //*****************RAID1 IMPLEMENTATION******************************************************
    rewind(streamBackup);
    fseek(streamBackup,blockNumber*DISK_BLOCK_SIZE,SEEK_SET);
    if(fwrite(data,DISK_BLOCK_SIZE,1,streamBackup)==1)
        printf("");
    else
    {
        printf("ERROR: Could not access emulated backup disk!\n Aborting...\n");
    }
    //*******************************************************************************************

}

//Formating and closing disk functions
void diskFormat()
{
    if(stream)
    {
        rewind(stream);
        char *buffer = calloc(numberOfBlocks*DISK_BLOCK_SIZE, sizeof(char));
        fwrite(buffer,sizeof(char), numberOfBlocks*DISK_BLOCK_SIZE,stream);
    }
    //NOTE: diskFormat() function does not remove data from backup drive!
}

void diskClose()
{
    if(stream)
    {
        printf("==========DISK STATISTICS==========\n");
        printf("NUMBER OF READS: %d\n", numberOfReads);
        printf("NUMBER OF WRITES: %d\n", numberOfWrites);
        printf("===================================\n");
        fclose(stream);
        stream = NULL;
    }
    //*******RAID1 IMPLEMENTATION*******************************
    if(streamBackup)
    {
        fclose(streamBackup);
        streamBackup = NULL;
    }
    //**********************************************************
}
