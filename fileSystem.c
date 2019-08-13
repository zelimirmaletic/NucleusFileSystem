#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "fileSystem.h"
#define POINTERS_PER_INODE 5
#define INODE_TABLE_SIZE 20 // this size can hold aprox. 512 files/directories

//GLOBAL VARIABLES
static SUPERBLOCK superblock;
static char *freeBlocksBitmap;
static INODE currentInode;
//Functions for managing superBlock
void printSuperblock(SUPERBLOCK *superblock)
{
    printf("------------------------------------\n");
    printf("             SUPERBLOCK\n");
    printf("------------------------------------\n");
    printf("Magic number: %s\n", superblock->magicNumber);
    printf("Operating system: %s\n", superblock->operatingSystem);
    printf("Block size: %d [B]\n", superblock->blockSize);
    printf("Number of blocks: %d\n", superblock->numberOfBlocks);
    printf("Number of free blocks: %d\n", superblock->numberOfFreeBlocks);
    //inode data
    printf("Number of inode blocks: %d\n", superblock->numberOfInodeBlocks);
    printf("Number of free inodes: %d\n", superblock->numberOfFreeInodes);
    printf("Inode segment pointer: %d\n", superblock->inodeSegmentPointer);
    printf("Pointers per inode: %d\n", superblock->pointersPerInode);
    //data segment
    printf("Bitmap size: %d  blocks\n", superblock->bitmapLength);
    printf("Data segment pointer: %d\n", superblock->dataSegmentPointer);
    printf("Free blocks bitmap pointer: %d\n", superblock->freeBlocksBitmapPointer);
    printf("------------------------------------\n");
}

int fsFormat()
{
    diskOpen("disk.bin");
    char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
    diskRead(0,temp);
    int isMounted = 1;

    //Now we have to check whether the disk is mounted or not
    //We do that by checking the existance of magic number in the superblock
    char magicNumber[] = "0xf0f034";
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
        strcat(superblock.magicNumber,"\0");
        strcpy(superblock.operatingSystem, "Linux");
        superblock.blockSize = DISK_BLOCK_SIZE;
        superblock.numberOfFreeBlocks = superblock.numberOfBlocks-1; // -1 because superblock is taken after this function
        superblock.numberOfInodeBlocks = (unsigned short)((float)superblock.numberOfBlocks * 0.1);
        superblock.pointersPerInode = POINTERS_PER_INODE;
        superblock.bitmapLength = (superblock.numberOfBlocks)/512;
        superblock.freeBlocksBitmapPointer = 1;
        superblock.numberOfFreeInodes = superblock.numberOfInodeBlocks;
        superblock.inodeSegmentPointer = superblock.bitmapLength + 1;
        superblock.dataSegmentPointer = superblock.inodeSegmentPointer + superblock.numberOfInodeBlocks + 1;
        //Write superblock to disk
        diskFormat();
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
    setNumberOfBlocks(superblock.numberOfBlocks);
    if(!strcmp(superblock.magicNumber, "0xf0f034"))
    {
        printSuperblock(&superblock);
        //Disk is formated and has FS, create bitmap
        //Here we should read the bitmap from disk!
        freeBlocksBitmap = calloc(superblock.bitmapLength*DISK_BLOCK_SIZE, sizeof(char));
            if(!freeBlocksBitmap)
                printf("ALLOCATION FAILURE!\n");
        for(int i=0;i<superblock.bitmapLength*DISK_BLOCK_SIZE;++i)
            *(freeBlocksBitmap + i) = '0';
        //readFreeNodeBitmapFromDisk();
        updateFreeBlockBitmap('w');
        updateFreeBlockBitmap('w');updateFreeBlockBitmap('w');
        updateFreeBlockBitmap('w');
        updateFreeBlockBitmap('w');

        printFreeNodeBitmap();
    }
    else
    {
        printf("ERROR: fsMount()---> Disk is not formated! You should format it first!\n");
        return 0;
    }

}

void printInode(unsigned short inodeNumber)
{
    INODE inode;
    diskReadStructure(superblock.inodeSegmentPointer+inodeNumber, 1, 'i', &inode);
    printf("\tINODE %d\n", inode.inodePosition);
    printf("\t---------------------------------------------\n");
    if(inode.fileType == 'f')
        printf("\tFIle name: %s\n", inode.fileName);
    else
        printf("\tDirectory name: %s\n", inode.fileName);
    printf("\tPosition: %d\n", inode.inodePosition);
    printf("\tSize: %d\n",inode.inodeSize );
    printf("\tDirect pointers: ");
    for(int i=0;i<POINTERS_PER_INODE;i++)
        printf("%d ",(inode.directInodePointers)[i]);
    printf("\n");
    printf("\tIs extent: ");
    inode.isExtent=='1'?printf("true\n"):printf("false");
    if(inode.isExtent=='1')
        printf("\tExtent length: %d\n",inode.extentLength);
    printf("\tAccess permisions: %s\n",inode.accessPermisions);
    //Make directorySize() function
    //Print here file or directory size
    printf("\tAccess time: %s\n",inode.accessTime);
    printf("\tModification time: %s\n",inode.modificationTime);
    printf("\tCreation time: %s\n",inode.modificationTime);
    printf("\tOwner: %s\n", inode.owner);
}

void printFreeNodeBitmap(void)
{
    printf("=============================FREE NODES BITMAP==================================\n");
    for(int i=0;i<(superblock.bitmapLength)*DISK_BLOCK_SIZE;++i)
            printf("%c",freeBlocksBitmap[i]);
    printf("\n================================================================================\n");
}

void writeFreeNodeBitmapToDisk(void)
{
    if(superblock.bitmapLength == 1)
        diskWrite(superblock.freeBlocksBitmapPointer, freeBlocksBitmap);
    else
    {
        char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
        for(int i=0;i<superblock.bitmapLength;++i) //number of blocks to be written
        {
            for(int j=0+i*DISK_BLOCK_SIZE, k=0; j < i*DISK_BLOCK_SIZE + DISK_BLOCK_SIZE; ++j,++k)
            {
                temp[k] = freeBlocksBitmap[j];
            }
            diskWrite(superblock.freeBlocksBitmapPointer + i,temp);
        }
        free(temp);
        temp = NULL;
    }
}
void readFreeNodeBitmapFromDisk(void)
{
    if(superblock.bitmapLength == 1)
        diskRead(superblock.freeBlocksBitmapPointer, freeBlocksBitmap);
    else
    {
        char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
        for(int i=0;i<superblock.bitmapLength;++i) //number of blocks to be written
        {
            diskRead(superblock.freeBlocksBitmapPointer + i,temp);
            for(int j=0+i*DISK_BLOCK_SIZE, k=0; j < i*DISK_BLOCK_SIZE + DISK_BLOCK_SIZE; ++j,++k)
            {
                freeBlocksBitmap[j] = temp[k];
            }
        }
        free(temp);
        temp = NULL;
    }
}

void clearInode(unsigned short inodeNumber)
{
    formatBlock(superblock.inodeSegmentPointer + inodeNumber);
}

unsigned short getInodeSize(unsigned short inodeNumber)
{
    diskReadStructure(inodeNumber,1,'i',&currentInode);
    return currentInode.inodeSize;
}



int updateFreeBlockBitmap(char mode, ...)
{
    if(mode == 'd') //free up bitmap position
    {
        va_list funcitonArguments;
        va_start(funcitonArguments,1);
        int inodeNumber = va_arg(funcitonArguments, int);
            if(inodeNumber<1)
                return -1; //we cannot delete superblock flag!
        freeBlocksBitmap[inodeNumber] = '0';
        va_end(funcitonArguments);
        return 0;
    }
    else if(mode == 'w') //write 1 to first free position - mark one inode/block
    {
        for(int i=0; i<(superblock.bitmapLength*DISK_BLOCK_SIZE); ++i)
        {
            if(freeBlocksBitmap[i] == '0' )
            {
                printf("Inode Number--->%d\n", i);
                freeBlocksBitmap[i] = '1';
                printf("***********%c\n", freeBlocksBitmap[i]);
                return i;//return number of inode taken
            }
        }
        //Error handle, when there are no more free blocks
    }
    else
    {
        return -1; //There is an error in arguments
    }
}










void closeFileSystem()
{
    //write superblock and other data
    writeFreeNodeBitmapToDisk();
    diskWriteStructure(0,1,'s',superblock);
    free(freeBlocksBitmap);
    freeBlocksBitmap = NULL;
}
