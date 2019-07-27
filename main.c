#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
int main()
{

    diskInitialize("disk.bin",10);

    char letters[]={'a','b','c','d','e'};

    diskWrite(0,letters);
    diskWrite(1,letters);
    diskWrite(3,letters);
    diskWrite(4,letters);


    char *data = calloc(5, sizeof(char));
    diskRead(0, data);

    for(int i=0;i<5;i++)
        printf("%c\n", data[i]);
    printf("Disk size: %d [blocks]\n",diskSize());
    printf("Disk usage is: %6.3f[%]\n", diskUsage());
    //diskFormat();
    diskClose();

    return 0;
}
