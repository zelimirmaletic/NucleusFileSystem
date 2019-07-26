#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
int main()
{
    diskInitialize("disk.bin",50);
    char letters[]={'a','b','c','d','e'};

    diskWrite(0,letters);
    diskWrite(50,letters);
    diskWrite(10,letters);
    diskWrite(30,letters);

    char *data = calloc(5, sizeof(char));
    diskRead(40, data);

    for(int i=0;i<5;i++)
        printf("%c\n", data[i]);

    diskFormat();
    diskFormat();
    diskClose();

    return 0;
}
