#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "disk.h"

//Here we define global constants
static FILE *stream;
static int numberOfBlocks = 0;
static int numberOfReads = 0;
static int numberOfWrites = 0;


int diskInitialize(const char *fileName, int number)
{
    if( (number<0) | (number>20e6) )
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
    //ftruncate() - function that shortend file to desired length
    //fileno() -  it shall return the integer file descriptor associated with the stream pointed to by stream
    ftruncate(fileno(stream), number*DISK_BLOCK_SIZE);
    numberOfBlocks = number;
    return 1;
}

int diskSize()
{
    return numberOfBlocks;
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

void diskRead(int blockNumber, char *data)
{
    validityCheck(blockNumber, data);
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

    fseek(stream, blockNumber*DISK_BLOCK_SIZE, SEEK_SET);

    if(fwrite(data, DISK_BLOCK_SIZE, 1, stream)==1)
        numberOfWrites++;
    else
    {
        printf("ERROR: Could not access emulated disk!\n Aborting...\n");
        abort();
    }
}

void diskFormat()
{
    if(stream)
    {
        rewind(stream);
        char *buffer = calloc(numberOfBlocks*DISK_BLOCK_SIZE, sizeof(char));
        fwrite(buffer,sizeof(char),(numberOfBlocks+1)*DISK_BLOCK_SIZE,stream);
    }
}

void diskClose()
{
    if(stream)
    {
        printf("=======DISK STATISTICS=======\n");
        printf("NUMBER OF READS: %d\n", numberOfReads);
        printf("NUMBER OF WRITES: %d\n", numberOfWrites);
        printf("=============================\n");
        fclose(stream);
        stream = 0;
    }
}
