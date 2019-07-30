#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
int main()
{

    diskInitialize("disk.bin",5);
    diskFormat();

    char letters[]="Alice was beginning to get very tired of sitting by her sisteron the bank, and of having nothing to do:  once or twice she hadeeped into the book her sister was reading, but it had no pictures or conversations in it, `and what is the use of a book,thought Alice `without pictures or conversation?";
    diskWrite(0,letters);
    diskWrite(1,letters);
    diskWrite(3,letters);
    diskWrite(4,letters);


    char *data = calloc(512, sizeof(char));
    diskRead(0, data);

    for(int i=0;i<512;i++)
        printf("%c", data[i]);
    free(data);
    printf("Disk size: %d [blocks]\n",diskSize());
    printf("Disk usage is: %6.3f[percent]\n", diskUsage());
    //diskFormat();

    diskClose();

    return 0;
}
