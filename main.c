#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileSystem.h"
#include "shell.c"
int main()
{

    printf("===============================================================================\n");
    printf("|                           NUCLEUS FILE SYSTEM                               |\n");
    printf("===============================================================================\n");
    strcpy(currentAbsoluteAdress,"root/");
    treeDepth=1;
    int exitFlag = 0;
    while(exitFlag!=-1)
    {
        char *commandEntered[30];
        printf("%s", currentAbsoluteAdress);
        scanf("%s",commandEntered);
        exitFlag =  executeCommand(commandEntered);
        strcpy(commandEntered,"\0");
    }
    printf("===============================================================================\n");
    printf("|                           Zelimir Maletic 2019.                              |\n");
    printf("===============================================================================\n");
    return 0;
}
