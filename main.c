#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "fileSystem.h"
#include "shell.h"
char name[30];
char buffer[512];
unsigned short temp;
char password[10];
char rootPassword[]="Linux";
int main()
{

    printf("===============================================================================\n");
    printf("|                           NUCLEUS FILE SYSTEM                               |\n");
    printf("===============================================================================\n");
    strcpy(currentAbsoluteAdress,"root/");
    treeDepth=1;
    while(1)
    {
        char *commandEntered[15];
        printf("%s", currentAbsoluteAdress);
        scanf("%s",commandEntered);
        switch(getCommandNumber(commandEntered))
        {
            case 0:
                closeFileSystem();
                diskClose();
                return 0;
            case 1:
                fsFormat('0');
                break;
            case 2:
                fsMount();
                printf("NOTIFICATION: File system mounted.\n");
                break;
            case 3:
                printHelp();
                break;
            case 4:
                printAllFilesAndFolders(currentAbsoluteAdress);
                break;
            case 5:
                scanf("%s", name);
                if(treeDepth==2)
                    printf("ERROR: You cannot have folders here!\n");
                else if(folderExists(name, currentAbsoluteAdress))
                    printf("ERROR: Folder already exists!\n");
                else
                    createInode(name,'d', currentAbsoluteAdress);
                break;
            case 6:
                printf("Password: ");
                scanf("%s", password);
                if(!strcmp(rootPassword, password))
                {
                    fsFormat('s');
                    printf("Formated!\n");
                }
                else
                    printf("ERROR: Wrong password\n");
                break;
            case 7:
                fsDebug();
                break;
            case 8:
                scanf("%s",name);
                if(fileExists(name, currentAbsoluteAdress))
                    printf("ERROR: File aready exists!\n");
                else
                    createInode(name,'f', currentAbsoluteAdress);
                break;
            case 9:
                scanf("%s",name);
                scanf("%s", buffer);
                renameInode(name, buffer);
                break;
            case 10:
                scanf("%s", name);
                fsRead(findInodeByFIleName(name));
                break;
            case 11:
                scanf("%s", name);
                temp = findInodeByFIleName(name);
                if(!temp)
                    printf("ERROR: Entered name is not valid. Please check spelling\n");
                else
                    printInode(temp);
                break;
            case 12:
                scanf("%s", name);
                if(folderExists(name, currentAbsoluteAdress))
                {
                    strcat(currentAbsoluteAdress, name);
                    strcat(currentAbsoluteAdress, "/");
                    treeDepth++;
                }
                else
                {
                    printf("ERROR: Folder not found. Use command [ls] to see all folders on current absolute adress.\n");
                }
                break;
            case 13:
                treeDepth--;
                strcpy(currentAbsoluteAdress, "root/");
                break;
            case 14:
                scanf("%s", name);
                if(!fileExists(name,currentAbsoluteAdress))
                    printf("ERROR: File not found. Use command [ls] to see all files and folders\n");
                else
                    fsWrite(findInodeByFIleName(name), buffer);
            case -1:
                printf("ERROR:Command is not valid. \nPlease, take a look at help section by entering command: help\n");
                break;
        }
        strcpy(commandEntered,"\n");
    }


    return 0;
}
