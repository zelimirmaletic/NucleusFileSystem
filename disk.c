#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "disk.h"
#include "fileSystem.h"

//GLOBAL CONSTANTS
static FILE *stream = NULL;
static FILE *streamBackup = NULL;
//Keep track of basic disk data and I/O operations
static unsigned int numberOfBlocks = 0;
static unsigned int numberOfReads = 0;
static unsigned int numberOfWrites = 0;

void setNumberOfBlocks(unsigned short number)
{
    numberOfBlocks = number;
}
/*
*******************NOTE ABOUT EMULATED DISK-FAILURE SECURITY***********************
-This emulation of a physical hard drive has a built-in disk-failure security.
-Security is achieved using RAID1.
-In this implementation we have only one backup drive(binary file), whereas in
practice there can be multiple backup hard drvies.
-This security implementation has more explanatory than practical purpose.
-In implemetation of functions there will be commented which part of each function
is responsible for RAID1 implementation.
**********************************************************************************
*/
int diskOpen(const char *fileName)
{
    stream = fopen(fileName,"rb+");
    if (!stream)
        stream = fopen(fileName,"w+");
    if(!stream)
        return 0;
    //************RAID1 IMPLEMENTATION****************
    streamBackup = fopen("BackupDisk.bin","rb+");
    if (!streamBackup)
        streamBackup = fopen("BackupDisk.bin","wb+");
    if(!streamBackup)
        return 0;
    //************************************************
}

//Called only at the first use of file system
int diskInitialize(int number)
{
    if( (number<=0) || (number>40960) )//maximal size of disk is 20MiB
    {
        printf("ERROR: Maximal disk size is 20 MB and minimal size cannot be zero!\n");
        printf("ERROR: Allocation failure!\n");
        return 0;
    }
    numberOfBlocks = number;
    //ftruncate() - Function that shortens file to desired length
    //fileno() -  It shall return the integer file descriptor associated with the stream pointed to by stream
    ftruncate(fileno(stream), numberOfBlocks*DISK_BLOCK_SIZE);

    //******************RAID1 IMPLEMENTATION*************************
    ftruncate(fileno(streamBackup), numberOfBlocks*DISK_BLOCK_SIZE);
    //***************************************************************
    return 1;
}

//Determines true byte usage on emulated hard-drive expressed in percentiles
float diskUsage()
{
    unsigned int counter = 0;
    int temp;
    for(int i=0;i<numberOfBlocks*DISK_BLOCK_SIZE;i++)
    {
        temp = getc(stream);
        if(temp!=0)
            counter++;
    }
    rewind(stream);
    //Counter is now number of used b on hard-drive
    float percentage = (float)counter/(numberOfBlocks*DISK_BLOCK_SIZE);
    return percentage*100;
}

//Functions for determining disk size
int diskSize(){ return numberOfBlocks; }
int diskSizeB(){ return numberOfBlocks*DISK_BLOCK_SIZE; }
float diskSizeKB(){ return (numberOfBlocks*DISK_BLOCK_SIZE)/10e3; }
float diskSizeKiB(){ return (numberOfBlocks*DISK_BLOCK_SIZE)/1024.00; }

static void validityCheck(register int blockNumber, const char *data)
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
void diskRead(register int blockNumber,register char *data)
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
    //decrypt(data);
    rewind(stream);
}

void diskWrite(register int blockNumber,register const char *data)
{
    validityCheck(blockNumber, data);

    //Find position in disk
    if(blockNumber == 0)
        rewind(stream);
    else
        fseek(stream, blockNumber*DISK_BLOCK_SIZE, SEEK_SET);

    //Determine input data length
    unsigned int length = 0;
    for (length = 0; *(data+length) != '\0'; ++length);

    //Allocate new temporary array
    char *temp = calloc(DISK_BLOCK_SIZE,sizeof(char));
    for(int i =0;i<length;++i)
        temp[i] = data[i];
    //encrypt(temp);

    if(fwrite(temp, DISK_BLOCK_SIZE, 1, stream)==1)
        numberOfWrites++;
    else
    {
        printf("ERROR: Could not access emulated disk!\n Aborting...\n");
        abort();
    }
    //*****************RAID1 IMPLEMENTATION******************************************************
    rewind(streamBackup);
    fseek(streamBackup,blockNumber*DISK_BLOCK_SIZE,SEEK_SET);
    fwrite(temp, DISK_BLOCK_SIZE, 1, streamBackup);
    //*******************************************************************************************
    free(temp);
    temp = NULL;
}

/*
NOTE: This is a function with variable number of arguments. It is used for writing superblock to disk as
well as writing inodes to disk. Parameter mode can have values 's' for writing a superblock and 'i' for
writing inode to disk.
*/
void diskWriteStructure(int blockNumber, int numberOfArguments, char mode, ...)
{
    va_list functionArguments;
    va_start(functionArguments, numberOfArguments);

    if(mode == 's')
    {
        SUPERBLOCK superBlock = va_arg(functionArguments, SUPERBLOCK);
        rewind(stream);
        fwrite(&superBlock, sizeof(SUPERBLOCK), 1, stream);
        //********RAID1******************************************
        rewind(streamBackup);
        fwrite(&superBlock, sizeof(SUPERBLOCK), 1, streamBackup);
        //*******************************************************
        numberOfWrites++;
    }
    else if(mode == 'i')
    {
        INODE inode =  va_arg(functionArguments, INODE);
        rewind(stream);
        fseek(stream, blockNumber*DISK_BLOCK_SIZE, SEEK_SET);
        fwrite(&inode, sizeof(INODE), 1, stream);
        //********RAID1********************************************
        rewind(streamBackup);
        fseek(streamBackup, blockNumber*DISK_BLOCK_SIZE, SEEK_SET);
        fwrite(&inode, sizeof(INODE), 1, streamBackup);
        //*********************************************************
        numberOfWrites++;
    }
    else
    {
        printf("ERROR: function-->diskWriteStructure(...) : Wrong mode parameter!\n");
    }
    va_end(functionArguments);
    return;
}
void diskReadStructure(int blockNumber, int numberOfArguments, char mode, ...)
{
    va_list functionArguments;
    va_start(functionArguments, numberOfArguments);

    if(mode == 's')
    {
        SUPERBLOCK *superBlock = va_arg(functionArguments, SUPERBLOCK *);
        if(!superBlock)
        {
            printf("ERROR:diskReadStructure() ----> Invalid pointer!\n");
            return;
        }
        rewind(stream);
        fread(superBlock, sizeof(SUPERBLOCK), 1, stream);
        numberOfReads++;
    }
    else if(mode == 'i')
    {
        INODE *inode =  va_arg(functionArguments, INODE*);
        if(!inode)
        {
            printf("ERROR:diskReadStructure() ----> Invalid pointer!\n");
            return;
        }
        rewind(stream);
        fseek(stream, blockNumber*DISK_BLOCK_SIZE, SEEK_SET);
        fread(inode, sizeof(INODE), 1, stream);
        numberOfReads++;
    }
    else
    {
        printf("ERROR: function-->diskReadStructure(...) : Wrong mode parameter!\n");
    }
    va_end(functionArguments);
    return;
}
//Formating and closing disk functions
void formatBlock(int blockNumber)
{
    char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
    diskWrite(blockNumber, temp);
    free(temp);
    temp = NULL;
}
void diskFormat()
{
    char *buffer = calloc(numberOfBlocks*DISK_BLOCK_SIZE, sizeof(char));
    if(stream)
    {
        rewind(stream);
        fwrite(buffer,sizeof(char), numberOfBlocks*DISK_BLOCK_SIZE,stream);
    }
    if(streamBackup)
    {
        rewind(streamBackup);
        fwrite(buffer,sizeof(char), numberOfBlocks*DISK_BLOCK_SIZE,streamBackup);
    }
    free(buffer);
    buffer = NULL;
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


//****************RAID1 IMPLEMENTATION********************
void switchToBackupDisk(){ stream = streamBackup; }
//********************************************************


/*
********************** NOTE ABOUT ON-THE-FLY-ENCRYPTION IMPLEMENTATION ******************************
-On the fly encryption is an additional feature, again, just to demonstrate and to more realistically
emulate real hard drive.
-Real hard drives have multiple layers of encryption including full-disk-encryption,
on-the-fly-encryption, etc.
-This encryption is acomplished through simple pertubation encryption algorithm.
*****************************************************************************************************
*/
static void encrypt(register char *data)
{
    register char temp;
    //Simple perturbation encryption algorithm
    for(int i=0;i<DISK_BLOCK_SIZE;++i)
    {
        if(i%2==0)
        {
            //pointer access is implemented for the sake of speed
            temp = *data;
            *data = *(data+i);
            *(data+i) = temp;
        }
    }
}
static void decrypt(register char *data)
{
    register char temp;
    //Simple perturbation decryption algorithm
    for(int i=DISK_BLOCK_SIZE-1;i>=0;i--)
    {
        if(i%2==0)
        {
            temp = *data;
            *data = *(data+i);
            *(data+i) = temp;
        }
    }
}
//*******************************************************************************************************
