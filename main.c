#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fileSystem.h"

int main()
{
    //diskOpen("disk.bin");
    //diskInitialize(1);

    //char letters[]="Alice was...";
    //diskWrite(0,letters);
    //diskWrite(1,letters);
    //diskWrite(3,letters);
    //diskWrite(4,letters);

    //fsFormat();

    fsMount();
    for(int i=0;i<2*DISK_BLOCK_SIZE;i++)
        //printf("%d", freeBlocksBitmap[i]);
    /*
    char *data = calloc(DISK_BLOCK_SIZE, sizeof(char));
    diskRead(0, data);
    for(int i=0;i<DISK_BLOCK_SIZE;i++)
        printf("%c", data[i]);
    printf("\n");

    printf("Disk size: %f [KiB]\n",diskSizeKiB());
    printf("Disk usage is: %6.3f[percent]\n", diskUsage());
    //diskFormat();
    //formatBlock(0);
    */
    closeFileSystem();
    diskClose();
    //free(data);
    //data = NULL;


    /*
    int a = 4056;
    char buffer[20];
    itoa(a, buffer, 10);
    printf("%s", buffer);
*/
    return 0;
}
