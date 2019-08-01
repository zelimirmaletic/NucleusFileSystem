#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
int main()
{
    diskInitialize("disk.bin",1);

    char letters[]="Alice was...";
    diskWrite(0,letters);
    //diskWrite(1,letters);
    //diskWrite(3,letters);
    //diskWrite(4,letters);



    char *data = calloc(DISK_BLOCK_SIZE, sizeof(char));
    diskRead(0, data);

    for(int i=0;i<DISK_BLOCK_SIZE;i++)
        printf("%c", data[i]);
    printf("\n");
    free(data);
    printf("Disk size: %f [KiB]\n",diskSizeKiB());
    printf("Disk usage is: %6.3f[percent]\n", diskUsage());
    //diskFormat();

    diskClose();

    return 0;
}
