#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>

#include "fileSystem.h"
//GLOBAL VARIABLES
static SUPERBLOCK superblock;
static char *freeBlocksBitmap;
static INODE currentInode;
unsigned short isMounted;
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

int fsFormat(char sudoRight)
{
    diskOpen("disk.bin");
    diskReadStructure(0,1,'s',&superblock);
    int isMounted = 0;
    //Now we have to check whether the disk is mounted or not
    //We do that by checking the existance of magic number in the superblock
    char magicNumber[] = "0xf0f034";
    if(strcmp(superblock.magicNumber, magicNumber)==0)
        isMounted = 1;
    if(sudoRight == 's')
        isMounted = 0;
    if(isMounted==1)
    {
        printf("WARNING: Disk is already formatted! You cannot format it again.\n");
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
        writeFreeNodeBitmapToDisk();
        free(freeBlocksBitmap);
        freeBlocksBitmap = NULL;
    }
    return 0;
}

int fsMount()
{
    if(isMounted)
        return 2;
    isMounted=1;
    diskOpen("disk.bin");
    diskReadStructure(0,1,'s',&superblock);
    getCurrentTime(&superblock.mountTime);
    setNumberOfBlocks(superblock.numberOfBlocks);
    if(!strcmp(superblock.magicNumber, "0xf0f034"))
    {
        //Disk is formated and has FS, create bitmap
        //Here we should read the bitmap from disk!
        freeBlocksBitmap = calloc(superblock.bitmapLength*DISK_BLOCK_SIZE, sizeof(char));
            if(!freeBlocksBitmap)
                printf("FREEBLOCK BITMAP ALLOCATION FAILURE!\n");
        readFreeNodeBitmapFromDisk();
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
    if(inode.fileType == 'f')
    {
        printf("\tSize: %d [B]\n",inode.fileSize );
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
        inode.isExtent==1?printf("true\n"):printf("false\n");
        if(inode.isExtent==1)
            printf("\tExtent length: %d\n",inode.extentLength);
    }
    else
    {
        //Here should be implemented a size for directory------------------->MODIFY
    }
    printf("\tAccess permisions: %s\n",inode.accessPermisions);
    //Make directorySize() function
    //Print here file or directory size
    printf("\tType: %s\n",inode.fileType=='f'?"file":(inode.fileType=='d'?"directory":"not specified")); //nested ternary operator OMG!
    printf("\tCreation time: %s",inode.creationTime);
    printf("\tAccess time: %s",inode.accessTime);
    printf("\tModification time: %s",inode.modificationTime);
    printf("\tAbsolute adress: %s\n", inode.absoluteAdress);
}

void fsDebug(void)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    printSuperblock(&superblock);
    for(int i=0;i<superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]=='1')
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
    for(int i=0;i<strlen(temp);++i)
        printf("%c", temp[i]);
    free(temp);
    temp = NULL;
}

void printAllFilesAndFolders(char *currentAbsoluteAdress, char mode)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    short flagIsEmpty = 1;
    for(unsigned short i=0;i<superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]=='1')
        {
            diskReadStructure(superblock.inodeSegmentPointer + i, 1, 'i', &currentInode);
            if(!strcmp(currentInode.absoluteAdress, currentAbsoluteAdress))
            {
                flagIsEmpty = 0;
                if(currentInode.fileType=='d')
                {
                    printf("[dir] %s\n", currentInode.fileName);
                    if(mode == 'r')
                    {
                        char adress[50];
                        strcat(adress, currentInode.absoluteAdress);
                        strcat(adress, currentInode.fileName);
                        strcat(adress,"/");
                        printAllFilesAndFolders(adress, 'r');
                        strcpy(adress,"\0");
                    }
                }
                else
                {
                    if(mode=='r' && strcmp(currentAbsoluteAdress, "root/"))
                        printf("\t");
                    printf("%s\n", currentInode.fileName);
                }
            }
        }
    }
    //if(flagIsEmpty)
        //printf("NOTIFICATION: No files or folders to be shown\n");
}

void printFreeBlocksBitmap(void)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    printf("=============================FREE BLOCKS BITMAP=================================\n");
    for(int i=0;i<(superblock.bitmapLength)*DISK_BLOCK_SIZE;++i)
    {
        if(i==0)
            printf("INODES:\n");
        if(i==superblock.dataSegmentPointer)
            printf("\nDATA BLOCKS:\n");
        if(*(freeBlocksBitmap+i)=='1')
           printf("[%d]",i);
        else
            printf("%c",*(freeBlocksBitmap+i));
    }
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

void clearCurrentInode(void)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
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
    strcpy(currentInode.accessTime, "\0");
    strcpy(currentInode.modificationTime, "\0");
    strcpy(currentInode.creationTime, "\0");
    strcpy(currentInode.absoluteAdress, "root/\0");
}
unsigned short getInodeSize(unsigned short inodeNumber)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    clearCurrentInode();
    diskReadStructure(inodeNumber,1,'i',&currentInode);
    return currentInode.inodeSize;
}
short createInode(const char *name, char type, char *currentAbsoluteAdress)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    clearCurrentInode();
    currentInode.inodePosition = updateFreeBlockBitmap('w');
    if(currentInode.inodePosition == -1)//if there is an error in inode allocation(in bitmap)
        return -1;
    strcpy(currentInode.fileName, name);
    //Here we should write time and date to inode structure
    getCurrentTime(currentInode.creationTime);
    strcpy(currentInode.accessTime, "Not accessed yet\n");
    strcpy(currentInode.modificationTime, "Not modified yet\n");
    strcpy(currentInode.accessPermisions, "r----");
    strcpy(currentInode.absoluteAdress, currentAbsoluteAdress);
    currentInode.fileType = type;
    diskWriteStructure(superblock.inodeSegmentPointer + currentInode.inodePosition, 1, 'i', currentInode);
    superblock.numberOfFreeInodes--;
    superblock.numberOfFreeBlocks--;
    return 0;
}
unsigned short fsDeleteInode(unsigned short inodeNumber)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    if(freeBlocksBitmap[inodeNumber]=='0')
    {
        printf("ERROR: There is nothing to be deleted!\n");
        return;
    }
    clearCurrentInode();
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
        formatBlock(superblock.inodeSegmentPointer + inodeNumber);
        //Finally, release inode from free block bitmap
        updateFreeBlockBitmap('d', inodeNumber);
        superblock.numberOfFreeInodes++;
        superblock.numberOfFreeBlocks++;
        return 0;
    }
    else //This part is for releasing extent from file system
    {
        for(unsigned short i = 0; i < currentInode.extentLength; ++i)
        {
            formatBlock(currentInode.directInodePointers[0] + i);
            updateFreeBlockBitmap('d', ((currentInode.directInodePointers)[0])+i);
        }
        //Then, clear inode in disk
        formatBlock(superblock.inodeSegmentPointer + inodeNumber);
        //Finally, release inode from free block bitmap
        updateFreeBlockBitmap('d', inodeNumber);
        superblock.numberOfFreeInodes++;
        superblock.numberOfFreeBlocks+= currentInode.extentLength;
        return 0;
    }
    return -1; //If error occurs return -1
}

short fsWrite(unsigned short inodeNumber, const char *data)//----------------->NOT FINISHED!
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    if(!data)
        return -1; //there is an error in data pointer!
    //First we have to determine the size of data array
    unsigned int dataSize = strlen(data);
    char *temp = calloc(DISK_BLOCK_SIZE, sizeof(char));
    clearCurrentInode();
    diskReadStructure(superblock.inodeSegmentPointer+inodeNumber, 1, 'i', &currentInode);

    if(currentInode.directInodePointers[0]==0)
    {
        //Here we have to write, there are no allocated data blicks!
        if(dataSize<DISK_BLOCK_SIZE)//if we only write to one block
        {
            for(int i=0;i<dataSize;++i)
                temp[i] = data[i];
            currentInode.directInodePointers[0] = getFreeDataBlock();
            diskWrite(currentInode.directInodePointers[0], temp);
        }
        else
        {
            short numberOfBlocksNeeded = (short)(ceil((float)dataSize/(float)DISK_BLOCK_SIZE));
            if(dataSize< POINTERS_PER_INODE*DISK_BLOCK_SIZE) // if not extent, but has more than 512 B
            {
                for(int i=0;i<numberOfBlocksNeeded;++i)
                {
                    int j=i*DISK_BLOCK_SIZE;
                    for(int k=0;j<(i+1)*DISK_BLOCK_SIZE; ++j,++k)
                        temp[k] = data[j];
                    currentInode.directInodePointers[i] = getFreeDataBlock();
                    diskWrite(currentInode.directInodePointers[i],temp);
                }
            }
            else // this is for extent
            {
                currentInode.isExtent = 1;
                currentInode.extentLength = numberOfBlocksNeeded;
                currentInode.directInodePointers[0] = getBlocksForExtent(currentInode.extentLength);
                for(int i=0; i<numberOfBlocksNeeded;++i)
                {
                    int j=i*DISK_BLOCK_SIZE;
                    for(int k=0;j<(i+1)*DISK_BLOCK_SIZE; ++j,++k)
                        temp[k] = data[j];
                    diskWrite(currentInode.directInodePointers[0]+i,temp);
                    freeBlocksBitmap[currentInode.directInodePointers[0]+i] = '1';
                }
                superblock.numberOfFreeBlocks-=numberOfBlocksNeeded;
            }
        }
    }
    else//we update or concatenate contence to existing data blocks
    {
        printf("NOTIFICATION:%s already has contence. Updating file contence is not supported yet.\n",currentInode.fileName);
        fsUpdate(inodeNumber, data);

    }
    currentInode.fileSize = dataSize;
    getCurrentTime(currentInode.modificationTime);
    diskWriteStructure(superblock.inodeSegmentPointer+inodeNumber,1,'i',currentInode);
    free(temp);
    temp = NULL;
    return 0;
}
short fsUpdate(unsigned short inodeNumber, const char *data)
{
    //Code code code
    return 0;
}
void fsRead(unsigned short inodeNumber)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    //Check inode validity
    if(freeBlocksBitmap[inodeNumber] == '0' || inodeNumber<0 || inodeNumber > superblock.numberOfInodeBlocks)
    {
        printf("ERROR: Invalid inode number in fsRead() function!\n");
        return;
    }
    clearCurrentInode();
    diskReadStructure(superblock.inodeSegmentPointer+inodeNumber, 1, 'i', &currentInode);
    if(currentInode.fileType=='d')
    {
        printf("ERROR: You cannot use this command on a folder!\n");
        return;
    }
    if(currentInode.directInodePointers[0]==0)
    {
        printf("WARNING: %s is empty, there is nothing to be printed!\n", currentInode.fileName);
        getCurrentTime(currentInode.accessTime);
        diskWriteStructure(superblock.inodeSegmentPointer + inodeNumber,1,'i',currentInode);
        return;
    }
    printf("\n");
    printf("-------------------------------------------------------------------------------\n");
    printf("%s\n", currentInode.fileName);
    printf("-------------------------------------------------------------------------------\n");
    if(currentInode.isExtent != 1)//if not an extent
    {
        short counter = 0;
        for(short i=0;i<POINTERS_PER_INODE;++i)
            if(currentInode.directInodePointers[i]!= 0)
                counter++;
        for(unsigned short i=0;i < counter;++i)
                printDataBlock((currentInode.directInodePointers)[i]);
    }
    else //if extent
    {
        for(unsigned short i=0; i < currentInode.extentLength; ++i)
            printDataBlock( *(currentInode.directInodePointers) + i);
    }
    //Update access time
    getCurrentTime(currentInode.accessTime);
    diskWriteStructure(superblock.inodeSegmentPointer + inodeNumber,1,'i',currentInode);
    printf("\n-------------------------------------------------------------------------------\n");
    printf("\n");
}

int updateFreeBlockBitmap(char mode, ...)//write part here is only for inodes!
{
    if(mode == 'd') //free up bitmap position
    {
        va_list funcitonArguments;
        va_start(funcitonArguments,1);
        int blockNumber = va_arg(funcitonArguments, int);
        freeBlocksBitmap[blockNumber] = '0';
        va_end(funcitonArguments);
        return 0;
    }
    else if(mode == 'w') //write 1 to first free position - mark one inode
    {
        for(int i=0; i<(superblock.bitmapLength*DISK_BLOCK_SIZE); ++i)
        {
            if(*(freeBlocksBitmap+i) == '0' )
            {
                *(freeBlocksBitmap+i) = '1';
                return i;//return number of inode taken
            }
        }
        //Error handle, when there are no more free blocks
        printf("ERROR: No more free inode blocks!\n");
        return -1;
    }
    return -1; //There is an error in char argument
}

unsigned short getFreeDataBlock(void)
{
    unsigned short noMoreFreeFlag = 1;
    for(int i=0; i<(superblock.numberOfBlocks-superblock.numberOfInodeBlocks-1);++i)
    {
        if( *(freeBlocksBitmap + superblock.dataSegmentPointer + i) == '0')
        {
            *(freeBlocksBitmap + superblock.dataSegmentPointer + i) = '1';
            noMoreFreeFlag = 0;
            return superblock.dataSegmentPointer+i;
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

int getBlocksForExtent(unsigned short extentLength)
{
    printFreeBlocksBitmap();
    for(int i=0;i< superblock.bitmapLength*DISK_BLOCK_SIZE;++i)
    {
        if(freeBlocksBitmap[superblock.dataSegmentPointer + i] == '0')
        {
            short flag=1;
            for(int j=i;j<i+extentLength;++j)
            {
                if(freeBlocksBitmap[superblock.dataSegmentPointer+i+j]=='1')
                    flag = 0;
            }
            if(flag==1)
            {
                freeBlocksBitmap[superblock.dataSegmentPointer + i] = '1';
                return (superblock.dataSegmentPointer + i);
            }
        }
    }
    return -1;// We dnon't have enough space to fit extent!
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
    for(int i=0;i<=superblock.numberOfInodeBlocks;++i)
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

unsigned short findInodeByFolderName(const char *folderName)
{
    char folderNotFound = '1';
    for(int i=0;i<=superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]=='1')
        {
            clearCurrentInode();
            diskReadStructure(superblock.inodeSegmentPointer + i,1,'i', &currentInode);
            if(strcmp(currentInode.fileName, folderName) == 0 || currentInode.fileType=='d')
            {
                //file/folder is found
                folderNotFound = '0';
                return i;
            }
        }
    }
    if(folderNotFound == '1')
        return 0; //file is not found
}

short folderExists(char *folderName)
{
    for(int i=0;i<=superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]=='1')
        {
            clearCurrentInode();
            diskReadStructure(superblock.inodeSegmentPointer + i,1,'i', &currentInode);
            if(!strcmp(currentInode.fileName, folderName) && currentInode.fileType=='d')
                    return 1;
        }
    }
    return 0; //folder is not found
}

short fileExists(char *fileName, char *currentAbsoluteAdress)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return 2;
    }
    for(int i=0;i<=superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]=='1')
        {
            clearCurrentInode();
            diskReadStructure(superblock.inodeSegmentPointer + i,1,'i', &currentInode);
            if(strcmp(currentInode.fileName, fileName) == 0 && currentInode.fileType=='f')
            {
                if(!strcmp(currentInode.absoluteAdress, currentAbsoluteAdress))
                    return 1;
            }
        }
    }
    return 0;
}

void renameInode(char *oldName,char *newName)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    clearCurrentInode();
    unsigned short inodeNumber = findInodeByFIleName(oldName);
    if(!inodeNumber)
    {
        printf("ERROR: Could not find file. Check your spelling\n");
        return;
    }
    diskReadStructure(superblock.inodeSegmentPointer + inodeNumber,1,'i', &currentInode);
    strcpy(currentInode.fileName, newName);
    diskWriteStructure(superblock.inodeSegmentPointer + inodeNumber,1,'i', currentInode);
}

void removeFolder(char *folderName)
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    clearCurrentInode();
    char absoluteAdress[30];
    strcpy(absoluteAdress, "\0");
    strcat(absoluteAdress, "root/");
    strcat(absoluteAdress, folderName);
    strcat(absoluteAdress, "/");
    for(int i=0;i<superblock.numberOfInodeBlocks;++i)
    {
        if(freeBlocksBitmap[i]=='1')
        {
            clearCurrentInode();
            diskReadStructure(superblock.inodeSegmentPointer + i,1,'i', &currentInode);
            if(currentInode.fileType=='f')
            {
                if(!strcmp(currentInode.absoluteAdress, absoluteAdress))
                    fsDeleteInode(i);
            }
        }
    }
    fsDeleteInode(findInodeByFolderName(folderName));
}

void updateAbsoluteAdress(char *name, char *destinationFolder)
{
    if(!isMounted)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    clearCurrentInode();
    unsigned short inodeNumber = findInodeByFIleName(name);
    diskReadStructure(superblock.inodeSegmentPointer + inodeNumber,1,'i', &currentInode);
    if(!strcmp(destinationFolder, "root"))
        strcpy(currentInode.absoluteAdress,"root/");
    else
    {
        strcat(currentInode.absoluteAdress, destinationFolder);
        strcat(currentInode.absoluteAdress, "/");
    }
    getCurrentTime(currentInode.modificationTime);
    diskWriteStructure(superblock.inodeSegmentPointer + inodeNumber,1,'i', currentInode);
}


void downloadFile(char *fileName)
{
    if(!isMounted)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    char newName[30];
    strcat(newName, fileName);
    strcat(newName,"(copy)\0");
    if(fileExists(newName, currentAbsoluteAdress))
        printf("ERROR: File does not exist in current working directory.\n");
    else
    {
        clearCurrentInode();
        unsigned short inodeNumber = findInodeByFIleName(fileName);
        diskReadStructure(superblock.inodeSegmentPointer+inodeNumber,1,'i',&currentInode);
        FILE *outputStream = fopen(currentInode.fileName, "a");
        char *temp=calloc(DISK_BLOCK_SIZE, sizeof(char));
        if(currentInode.isExtent!=1)
        {
            for(short i=0;i<POINTERS_PER_INODE;i++)
            {
                if((currentInode.directInodePointers)[i] != 0)
                {
                    diskRead((currentInode.directInodePointers)[i], temp);
                    fputs(temp, outputStream);
                }
            }
        }
        else //for extent
        {
            for(short i=0;i<currentInode.extentLength;++i)
            {
                diskRead(*(currentInode.directInodePointers)+i,temp);
                fputs(temp, outputStream);
            }
        }
        free(temp);temp=NULL;
        fclose(outputStream);
    }
}

void copyFile(char *fileName, char *adress)
{
    if(!isMounted)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    if(fileExists(fileName, adress))
        printf("ERROR: File copy alredy exists in current working directory.\n");
    else
    {
        downloadFile(fileName);
        putFile(fileName, adress,'c');
    }
}

void putFile(char *fileName, char *adress, char isCopy)
{
    if(!isMounted)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    FILE *inputStream = fopen(fileName, "r");
    if(!inputStream)
    {
        printf("ERROR: File does not exist on main file system\n");
        return;
    }
    else
    {
        unsigned int counter = 0;
        char c;
        while(c!=EOF)
        {
            c=fgetc(inputStream);
            ++counter;
        }
        if(counter>64*1024)
        {
            printf("ERROR: Maximal file size on Nucleus File System is 64 KiB\n");
            return;
        }
        char *temp = calloc(counter, sizeof(char));
        if(fileExists(fileName, adress))
        {
            printf("ERROR: File already exists\n");
            return;
        }
        rewind(inputStream);
        for(int i=0;i<counter;i++)
            temp[i]=fgetc(inputStream);
        if(isCopy=='c')
        {
            char *newName=calloc(30,sizeof(char));
            strcat(newName, fileName);
            strcat(newName,"(copy)\0");
            createInode(newName,'f', adress);
            fsWrite(findInodeByFIleName(newName), temp);
            free(newName); newName =NULL;
        }
        else
        {
            createInode(fileName,'f', adress);
            fsWrite(findInodeByFIleName(fileName), temp);
        }
        free(temp);
        rewind(inputStream);
    }
    fclose(inputStream);
}

//Determines block usage on emulated hard-drive expressed in percentiles
float diskUsage()
{
    unsigned int counter = 0;
    int temp;
    for(int i=0;i<superblock.numberOfBlocks;i++)
    {
        if(*(freeBlocksBitmap+i)=='1')
            counter++;
    }
    //Counter is now number of used blocks on hard-drive
    float percentage = (float)counter/(superblock.numberOfBlocks);
    return percentage*100;
}

void closeFileSystem()
{
    if(isMounted!=1)
    {
        printf("ERROR: You have to mount file system first!\n");
        return;
    }
    //Make sure to save all relevant structures and data to disk!
    writeFreeNodeBitmapToDisk();
    diskWriteStructure(0,1,'s',superblock);
    free(freeBlocksBitmap);
    freeBlocksBitmap = NULL;
}
