#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fileSystem.h"
#include <time.h>
#include "supportLibrary.c"


int main()
{

    char Data[] = "This is an article about Coco Chanel. You may definitely recognize a double CC logo...\0";

    fsFormat();
    fsMount();

    //createInode("CocoChanel.txt");
    /*
    createInode("Versace.jpg");
    createInode("Valentino.txt");
    createInode("Gucci.txt");
    */
    //fsDeleteInode(1);
    //fsDeleteInode(3);
    //fsDeleteInode(3);

    //fsWrite(1,Data);
    fsRead(1);


    fsDebug();
    closeFileSystem();
    diskClose();
    return 0;
}
