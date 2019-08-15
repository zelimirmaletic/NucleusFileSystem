#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fileSystem.h"
#include <time.h>
#include "supportLibrary.c"


int main()
{
    fsFormat();
    fsMount();
    closeFileSystem();

    printf("BLOCK 1 CONTENCE--->\n");
    printDataBlock(10);

    diskClose();


    return 0;
}
