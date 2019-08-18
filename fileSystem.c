#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>

#include "fileSystem.h"
#define POINTERS_PER_INODE 5

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
    printf("Number of files/folders: %d\n", superblock->numberOfInodeBlocks - superblock->numberOfFreeInodes);
    //time information
    printf("Format time: %s", superblock->formatTime);
    printf("Mount time: %s", superblock->mountTime);
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
        if(*(temp+i) != *(magicNumber+i))
        {
            isMounted = 0;
            break;
        }
    }
    if(isMounted==1)
    {
        printf("WARNING: Disk is already mounted! You cannot format it.\n");
        free(temp);
        temp = NULL;
        return 1;
    }
    else //create new File System
    {
            printf("NOTIFICATION: Disk is not mounted!\nNOTE: Current disk size is %d[MB]\n", DISK_SIZE);
            printf("NOTIFICATOIN: Formating disk----->DONE!\n");

        //Now we have to calculate number of blcoks and initialize new disk
        superblock.numberOfBlocks = (DISK_SIZE*1024*1024)/DISK_BLOCK_SIZE;
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
        superblock.bitmapLength = (superblock.numberOfBlocks)/DISK_BLOCK_SIZE;
        superblock.freeBlocksBitmapPointer = 1;
        superblock.numberOfFreeInodes = superblock.numberOfInodeBlocks;
        superblock.inodeSegmentPointer = superblock.bitmapLength + 1;
        superblock.dataSegmentPointer = superblock.inodeSegmentPointer + superblock.numberOfInodeBlocks + 1;
        getCurrentTime(&superblock.formatTime);
        //Write superblock to disk
        diskFormat();
        diskWriteStructure(0,1,'s',superblock);
        //Initialize free block bitmap
        freeBlocksBitmap = calloc(superblock.bitmapLength*DISK_BLOCK_SIZE, sizeof(char));
            if(!freeBlocksBitmap)
                printf("FREEBLOCK BITMAP ALLOCATION FAILURE!\n");
        for(int i=0;i<superblock.bitmapLength*DISK_BLOCK_SIZE;++i)
            *(freeBlocksBitmap + i) = '0';
        *freeBlocksBitmap = '1';
        writeFreeNodeBitmapToDisk();
        free(freeBlocksBitmap);
        freeBlocksBitmap = NULL;
    }
    free(temp);
    temp = NULL;
    return 0;
}

int fsMount()
{
    diskOpen("disk.bin");
    diskReadStructure(0,1,'s',&superblock);
    getCurrentTime(&superblock.mountTime);
    setNumberOfBlocks(superblock.numberOfBlocks);
    if(!strcmp(superblock.magicNumber, "0xf0f034"))
    {
       // printSuperblock(&superblock); //--------------------------> COMMENT OUT THIS LATER!
        //Disk is formated and has FS, create bitmap
        //Here we should read the bitmap from disk!
        freeBlocksBitmap = calloc(superblock.bitmapLength*DISK_BLOCK_SIZE, sizeof(char));
            if(!freeBlocksBitmap)
                printf("FREEBLOCK BITMAP ALLOCATION FAILURE!\n");
        readFreeNodeBitmapFromDisk();
       // printFreeNodeBitmap(); //--------------------------> COMMENT OUT THIS LATER!
        return 0;
    }
    printf("ERROR: Disk is not formated! You should format it first!\n");
    return -1;
}
void printInode(unsigned short inodeNumber)
{
    INODE inode;
    diskReadStructure(superblock.inodeSegmentPointer+inodeNumber, 1, 'i', &inode);
    printf("---------------------------------------------\n");
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
    {
        if( (inode.directInodePointers)[i] == 0 )
            printf("x ");
        else
            printf("%d ",(inode.directInodePointers)[i]);
    }
    printf("\n");
    printf("\tIs extent: ");
    inode.isExtent=='1'?printf("true\n"):printf("false\n");
    if(inode.isExtent=='1')
        printf("\tExtent length: %d\n",inode.extentLength);
    printf("\tAccess permisions: %s\n",inode.accessPermisions);
    //Make directorySize() function
    //Print here file or directory size
    printf("\tType: %s\n",inode.fileType=='f'?"file":(inode.fileType=='d'?"directory":"not specified")); //double ternary operator OMG!
    printf("\tAccess time: %s",inode.accessTime);
    printf("\tModification time: %s",inode.modificationTime);
    printf("\tCreation time: %s",inode.modificationTime);
    printf("\tAbsolute adress: %s\n", inode.absoluteAdress);
}

void fsDebug(void)
{
    printSuperblock(&superblock);
    for(int i=1;i<superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]!='0')
        {
            printf("\t");
            printInode(i);
        }
    }
}

void printDataBlock(unsigned short blockNumber)
{
    char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
    diskRead(blockNumber, temp);
    printf("%s", temp);
    free(temp);
    temp = NULL;
}

void printFreeNodeBitmap(void)
{
    printf("=============================FREE BLOCKS BITMAP=================================\n");
    for(int i=0;i<(superblock.bitmapLength)*DISK_BLOCK_SIZE;++i)
            printf("%c",*(freeBlocksBitmap+i));
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
                *(temp+k) = *(freeBlocksBitmap+j);
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
                *(freeBlocksBitmap+j) = *(temp+k);
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
void clearCurrentInode(void)
{
    strcpy(currentInode.fileName, " ");
    currentInode.inodePosition = 0;
    currentInode.inodeSize = 0;
    for(short i=0;i< POINTERS_PER_INODE; ++i)
        currentInode.directInodePointers[i] = 0;
    currentInode.isExtent = 0;
    currentInode.extentLength = 0;
    currentInode.fileType = ' ';
    strcpy(currentInode.accessPermisions, "-----");
    currentInode.fileSize = 0;
    strcpy(currentInode.accessTime, "none");
    strcpy(currentInode.modificationTime, "none");
    strcpy(currentInode.creationTime, "none");
    strcpy(currentInode.absoluteAdress, "root/");
}
unsigned short getInodeSize(unsigned short inodeNumber)
{
    diskReadStructure(inodeNumber,1,'i',&currentInode);
    return currentInode.inodeSize;
}
unsigned short createInode(const char *name)
{
    clearCurrentInode();
    currentInode.inodePosition = updateFreeBlockBitmap('w');
    if(currentInode.inodePosition == -1)//if there is an error in inode allocation(in bitmap)
        return -1;
    //Here we should write time and date to inode structure
    strcpy(currentInode.fileName, name);
    char buffer[30];
    getCurrentTime(buffer);
    strcpy(currentInode.modificationTime, buffer);
    strcpy(currentInode.creationTime, buffer);
    strcpy(currentInode.accessTime, buffer);
    strcpy(currentInode.accessPermisions, "r-w-x");
    strcpy(currentInode.absoluteAdress, "root/");
    diskWriteStructure(superblock.inodeSegmentPointer + currentInode.inodePosition, 1, 'i', currentInode);
    superblock.numberOfFreeInodes--;
    superblock.numberOfFreeBlocks--;
    return 0;
}
unsigned short fsDeleteInode(unsigned short inodeNumber)
{
    diskReadStructure(superblock.inodeSegmentPointer + inodeNumber, 1, 'i', &currentInode);
    if(currentInode.isExtent == 0) //If not an extent regularly free all data
    {
         //First, clear data blocks on disk!
        for(int i=0;i<POINTERS_PER_INODE;++i)
        {
            if((currentInode.directInodePointers)[i] != 0)
            {
                formatBlock((currentInode.directInodePointers)[i]);
                updateFreeBlockBitmap('d', (currentInode.directInodePointers)[i]);
            }
        }
        //Then, clear inode in disk
        clearInode(inodeNumber);
        //Finally, release inode from free block bitmap
        updateFreeBlockBitmap('d', inodeNumber);
        superblock.numberOfFreeInodes++;
        superblock.numberOfFreeBlocks++;
        return 0;
    }
    else //This part is for releasing extent from file system
    {
        for(int i = 0; i < currentInode.extentLength; ++i)
        {
            formatBlock(currentInode.directInodePointers[0] + i);
            updateFreeBlockBitmap('d', (currentInode.directInodePointers)[0]+i);
        }
        //Then, clear inode in disk
        clearInode(inodeNumber);
        //Finally, release inode from free block bitmap
        updateFreeBlockBitmap('d', inodeNumber);
        superblock.numberOfFreeInodes++;
        return 0;
    }
    return -1; //If error occurs return -1
}

void fsWrite(unsigned short inodeNumber, const char *data)//----------------->NOT FINISHED!
{
    //First we have to determine the size of data array
    unsigned int dataSize = strlen(data);
    char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
    clearCurrentInode();
    diskReadStructure(superblock.inodeSegmentPointer+inodeNumber, 1, 'i', &currentInode);

    if(dataSize < DISK_BLOCK_SIZE)
    {
        for(int i=0;i<dataSize;++i)
            *(temp + i) = *(data + i);
        currentInode.directInodePointers[0] = getFreeDataBlock();
        diskWrite(currentInode.directInodePointers[0], temp);
        diskWriteStructure(superblock.inodeSegmentPointer + inodeNumber, 1, 'i', currentInode);
        return;
    }
    free(temp);
    temp = NULL;
}

void fsRead(unsigned short inodeNumber)
{
    diskReadStructure(superblock.inodeSegmentPointer+inodeNumber, 1, 'i', &currentInode);
    if(currentInode.isExtent != 1)//if not an extent
    {
        for(unsigned short i=0;(currentInode.directInodePointers)[i] == 'x' || i < 5;++i)
        {
            printDataBlock((currentInode.directInodePointers)[i]);
        }
    }
    else //if extent
    {
        for(unsigned short i=0; i < currentInode.extentLength; ++i)
        {
            printDataBlock( *(currentInode.directInodePointers) + i);
        }
    }
}

int updateFreeBlockBitmap(char mode, ...)//write part here is only for inodes!
{
    if(mode == 'd') //free up bitmap position
    {
        va_list funcitonArguments;
        va_start(funcitonArguments,1);
        int blockNumber = va_arg(funcitonArguments, int);
            if(blockNumber<1)
                return -1; //we cannot delete superblock flag!
        *(freeBlocksBitmap+blockNumber) = '0';
        va_end(funcitonArguments);
        return 0;
    }
    else if(mode == 'w') //write 1 to first free position - mark one inode
    {
        unsigned short noMoreFreeFlag = 1;
        for(int i=1; i<=(superblock.bitmapLength*DISK_BLOCK_SIZE); ++i)
        {
            if(*(freeBlocksBitmap+i) == '0' )
            {
                *(freeBlocksBitmap+i) = '1';
                noMoreFreeFlag = 0;
                return i;//return number of inode taken
            }
        }
        //Error handle, when there are no more free blocks
        if(noMoreFreeFlag == 1)
        {
            printf("ERROR: No more free inode blocks!\n");
            return -1;
        }
    }
    return -1; //There is an error in char argument
}

unsigned short getFreeDataBlock(void)
{
    unsigned short noMoreFreeFlag = 1;
    for(int i=superblock.dataSegmentPointer; i<=(superblock.numberOfBlocks-superblock.numberOfInodeBlocks-1);++i)
    {
        if( *(freeBlocksBitmap + superblock.dataSegmentPointer + i) == '0')
        {
            *(freeBlocksBitmap + superblock.dataSegmentPointer + i) = '1';
            noMoreFreeFlag = 0;
            return i;
        }
    }
    //Error handle, when there are no more free blocks
    if(noMoreFreeFlag == 1)
    {
        printf("ERROR: No more free data blocks!\n");
        return -1;
    }
    return -1; //There was an error
}

//TIME FUNCTION
void getCurrentTime(char *outputPointer)
{
    timer_t currentTime;
    struct tm *localTime;
    //Get the current time
    currentTime = time(NULL);
    //Convert it to local time representation
    localTime = localtime(&currentTime);
    //Copy date and time string to output pointer
    strcpy(outputPointer, asctime(localTime));
}

//Search functions
unsigned short findInodeByFIleName(const char *fileName)
{
    char fileNotFound = '1';
    for(int i=1;i<=superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]=='1')
        {
            clearCurrentInode();
            diskReadStructure(superblock.inodeSegmentPointer + i,1,'i', &currentInode);
            if(strcmp(currentInode.fileName, fileName) == 0)
            {
                //file/folder is found
                fileNotFound = '0';
                return i;
            }
        }
    }
    if(fileNotFound == '1')
        return 0; //file is not found
}

void closeFileSystem()
{
    //Make sure to save all relevant structures and data to disk!
    writeFreeNodeBitmapToDisk();
    diskWriteStructure(0,1,'s',superblock);
    free(freeBlocksBitmap);
    freeBlocksBitmap = NULL;
}
