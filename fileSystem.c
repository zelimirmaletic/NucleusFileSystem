#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "fileSystem.h"
#define POINTERS_PER_INODE 5

//GLOBAL VARIABLES
static SUPERBLOCK superblock;
static char *freeBlocksBitmap;
//Functions for managing superBlock
void printSuperblock(SUPERBLOCK *superblock)
{
    printf("------------------------------------\n");
    printf("             SUPERBLOCK\n");
    printf("------------------------------------\n");
    printf("Magic number: %s\n", superblock->magicNumber);
    printf("Block size: %d [B]\n", superblock->blockSize);
    printf("Number of blocks: %d\n", superblock->numberOfBlocks);
    printf("Number of free blocks: %d\n", superblock->numberOfFreeBlocks);
    printf("Number of inode blocks: %d\n", superblock->numberOfInodeBlocks);
    printf("Pointers per inode: %d\n", superblock->pointersPerInode);
    printf("Bitmap size: %d  blocks\n", superblock->bitmapLength);
    printf("Data segment pointer: %d\n", superblock->dataSegmentPointer);
    printf("------------------------------------\n");
}

int fsFormat()
{
    diskOpen("disk.bin");
    char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
    diskRead(0,temp);
    int isMounted = 1;

    //Now we have to check wether the disk is mounted or not
    //We do that by checking the existance of magic number in the superblock
    char magicNumber[] = "0xf0f03410";
    for(int i=0;i<8;i++)
    {
        if(temp[i] != magicNumber[i])
        {
            isMounted = 0;
            break;
        }
    }
    if(isMounted==1)
    {
        printf("Disk is already mounted! You cannot format it.\n");
        free(temp);
        temp = NULL;
        return 1;
    }
    else //create new File System
    {
        int diskSize = 0;//disk size in [MiB]
        while(1)
        {
            printf("Disk is not mounted! To mount it enter disk size in [MiB].\nNOTE: You cannot have disk size greater than 20[MB]\n");
            printf("Enter disk size (it has to be integer): ");
            scanf("%d",&diskSize);
            if(diskSize<0 || diskSize>20)
                continue;
            break;
        }
        //Now we have to calculate number of blcoks and initialize new disk
        superblock.numberOfBlocks = (diskSize*1024*1024)/512;
        diskInitialize(superblock.numberOfBlocks);
        diskFormat();
        //WRITE DATA TO SUPERBLOCK
        strcpy(superblock.magicNumber, magicNumber);
        superblock.blockSize = DISK_BLOCK_SIZE;
        superblock.numberOfFreeBlocks = superblock.numberOfBlocks-1; // -1 because superblock is taken after this function
        superblock.numberOfInodeBlocks = (int)((float)superblock.numberOfBlocks * 0.1);
        superblock.pointersPerInode = POINTERS_PER_INODE;
        superblock.bitmapLength = superblock.numberOfBlocks/512;
        superblock.dataSegmentPointer = 1 + superblock.numberOfInodeBlocks + superblock.bitmapLength;
        //Write superblock to disk
        diskWriteStructure(0,1,'s',superblock);
    }
    free(temp);
    temp = NULL;
    return 0;
}

int fsMount()
{
    diskOpen("disk.bin");
    diskReadStructure(0,1,'s',&superblock);
    printSuperblock(&superblock);
    if(!strcmp(superblock.magicNumber, "0xf0f034"))
    {
        //Disk is formated and has FS, create bitmap
        freeBlocksBitmap = calloc(2*DISK_BLOCK_SIZE, sizeof(char));
        freeBlocksBitmap[0] = 1;//for superblock
        diskRead(1,freeBlocksBitmap);
        diskRead(2, freeBlocksBitmap+DISK_BLOCK_SIZE);
        return 1;
    }
    else
    {
        printf("ERROR: fsMount()---> Disk is not formated! You should format it first!\n");
        return 0;
    }
}


void closeFileSystem()
{
    free(freeBlocksBitmap);
    freeBlocksBitmap = NULL;
}
