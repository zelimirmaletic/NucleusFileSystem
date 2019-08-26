#ifndef SHELL_C_INCLUDED
#define SHELL_C_INCLUDED
#include "disk.h"
#include "shell.h"
//Global variables
char name[30];
char buffer[512];
unsigned short temp;
char password[10];
char rootPassword[]="Linux";

short getCommandNumber(const char *commandEntered)
{
    if(!strcmp(commandEntered, "exit"))
        return 0;
    else if(!strcmp(commandEntered,"format"))
        return 1;
    else if(!strcmp(commandEntered, "mount"))
        return 2;
    else if(!strcmp(commandEntered, "help"))
        return 3;
    else if(!strcmp(commandEntered, "ls"))
        return 4;
    else if(!strcmp(commandEntered,"mkdir"))
        return 5;
    else if(!strcmp(commandEntered, "sudo-format"))
        return 6;
    else if(!strcmp(commandEntered, "debug"))
        return 7;
    else if(!strcmp(commandEntered, "create"))
        return 8;
     else if(!strcmp(commandEntered, "rename"))
        return 9;
    else if(!strcmp(commandEntered, "cat"))
        return 10;
    else if(!strcmp(commandEntered, "stat"))
        return 11;
    else if(!strcmp(commandEntered, "cd"))
        return 12;
    else if(!strcmp(commandEntered,"cd..")|| !strcmp(commandEntered, "cd/.."))
        return 13;
    else if(!strcmp(commandEntered, "echo"))
        return 14;
    else if(!strcmp(commandEntered, "mv"))
        return 15;
    else if(!strcmp(commandEntered, "rm"))
        return 16;
    else if(!strcmp(commandEntered, "rm-r"))
        return 17;
    else if(!strcmp(commandEntered, "stat-f"))
        return 18;
    else if(!strcmp(commandEntered, "stat-d"))
        return 19;
    else if(!strcmp(commandEntered, "ls-r"))
        return 20;
    else if(!strcmp(commandEntered, "pwd"))
        return 21;
    else if(!strcmp(commandEntered, "get"))
        return 22;
    else if(!strcmp(commandEntered, "cp"))
        return 23;
    else if(!strcmp(commandEntered, "put"))
        return 24;
    else
        return -1;
}

int executeCommand(char *commandEntered)
{
    switch(getCommandNumber(commandEntered))
        {
            case 0:
                closeFileSystem();
                diskClose();
                return -1;
            case 1:
                fsFormat('0');
                break;
            case 2:
                fsMount();
                if(!folderExists("root"))
                    createInode("root",'d', "root/");
                break;
            case 3:
                printHelp();
                break;
            case 4:
                printAllFilesAndFolders(currentAbsoluteAdress, 'x');
                break;
            case 5:
                scanf("%s", name);
                if(treeDepth==2)
                    printf("ERROR: You cannot have folders here!\n");
                else if(folderExists(name))
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
                if(fileExists(name, currentAbsoluteAdress)==2)
                {
                    printf("ERROR: You have to mount file system first!\n");
                    break;
                }
                if(fileExists(name, currentAbsoluteAdress))
                {
                    printf("ERROR: File aready exists!\n");
                    break;
                }
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
                if(!fileExists(name, currentAbsoluteAdress))
                {
                    printf("ERROR: File not found. Use command [ls] to see all files and folders\n");
                    break;
                }
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
                if(folderExists(name))
                {
                    if(!strcmp(name,"root"))
                        break;
                    strcpy(currentAbsoluteAdress,"root/");
                    strcat(currentAbsoluteAdress, name);
                    strcat(currentAbsoluteAdress, "/");
                    treeDepth++;
                }
                else
                    printf("ERROR: Folder not found. Use command [ls] to see all folders on current absolute adress.\n");
                break;
            case 13:
                if(strcmp(currentAbsoluteAdress, "root/"))
                {
                    treeDepth--;
                    strcpy(currentAbsoluteAdress, "root/");
                }
                break;
            case 14:
                scanf("%s", name);
                if(!fileExists(name,currentAbsoluteAdress))
                    printf("ERROR: File not found. Use command [ls] to see all files and folders\n");
                else
                {
                    printf("");
                    gets(buffer);
                    fsWrite(findInodeByFIleName(name), buffer);
                }
                break;
            case 15:
                scanf("%s", name);
                if(!fileExists(name,currentAbsoluteAdress))
                    printf("ERROR: File not found. Use command [ls] to see all files and folders\n");
                else
                {
                    gets(buffer);
                    int i=0;
                    while(buffer[i])
                        buffer[i++]=buffer[i];
                    if(!folderExists(buffer))
                        printf("ERROR: Could not find folder. Please chech your spelling or use command [ls] to see all files and folders\n");
                    else
                        updateAbsoluteAdress(name, buffer);
                }
                break;
            case 16:
                scanf("%s", name);
                if(fileExists(name, currentAbsoluteAdress))
                {
                    fsDeleteInode(findInodeByFIleName(name));
                }
                else
                {
                    if(!folderExists(name))
                        printf("ERROR: To remove folders use [rm -r] command.\n");
                    else
                        printf("ERROR: Could not find file. Please chech your spelling or use command [ls] to see all files and folders\n");
                }
                break;
            case 17:
                scanf("%s", name);
                if(!strcmp(name, "root"))
                    printf("ERROR: You cannot delete root folder!\n");
                else if(folderExists(name))
                    removeFolder(name);
                else
                    printf("ERROR: Could not find folder. Please chech your spelling or use command [ls] to see all files and folders\n");
                break;
            case 18:
                printFreeBlocksBitmap();
                break;
            case 19:
                printf("\nDISK USAGE: %6.3f[percent]\n",diskUsage());
                break;
            case 20:
                printAllFilesAndFolders(currentAbsoluteAdress,'r');
                break;
            case 21:
                printf("%s\n", currentAbsoluteAdress);
                break;
            case 22:
                scanf("%s", name);
                downloadFile(name);
                break;
            case 23:
                scanf("%s", name);
                gets(buffer);//into absolute adress
                int i=0;
                while(buffer[i])
                    buffer[i++]=buffer[i];
                copyFile(name, buffer);
                break;
            case 24:
                scanf("%s", name);
                gets(buffer);
                i=0;
                while(buffer[i])
                    buffer[i++]=buffer[i];
                putFile(name,buffer,'x');
                break;
            case -1:
                printf("ERROR:Command is not valid. \nPlease, take a look at help section by entering command: help\n");
                break;
            strcpy(buffer,"\0");
            strcpy(name,"\0");
        }
}

void printHelp(void)
{
    printf("**********************************HELP******************************************\n");
    printf("[format]        Formats empty disk and prepares it for mounting a file system.   \t\tReturns an error if disk is already mounted\n");
    printf("[sudo-format]   Formats disk no matter what is on it. Requres root privileges\n");
    printf("[mount]         Mounts file system on already formatted disk\n");
    printf("[help]          Prints help menu with all available commands and their descripti\n\t\tons\n");
    printf("[exit]          Exits file system shell, and saves all changes to the disk\n");

    printf("[debug]         Shows internal structure of file system.\n");
    printf("[ls]            Lists all files and folders\n");
    printf("[ls-r]          Lists all files, folders and folder contence.\n");
    printf("[mkdir]         Makes new directory on current absolute adress. It should be fol\n\t\tlowed by space and folder name(max 30 char)\n");
    printf("[create]        Makes a new file on current absolute adress. It should be follow\n\t\ted by space and file name (max 30 char)\n");

    printf("[rename]        Renames file/folder. It should be followed by:space, old file/fo\n\t\tlder name, space, new file/folder name\n");
    printf("[cat]           Prints file contence. It should be followed by a valid file name\n");
    printf("[stat]          Prints inode that resresents selected file/folder with all meta\n\t\t data. Should be followed by: space, valid file/folder name\n");
    printf("[cd]            Changes the folder position.Should be followed by a valid folder\n\t\tmname\n");
    printf("[cd..]or[cd/..] Returns to root/ folder\n");

    printf("[echo]          Writes text to file.Should be followed by: valid file name, text\n\t\tdata\n");
    printf("[rm]            Removes a file from file system at the current position.\n");
    printf("[rm-r]          Removes folder with all files inside it.\n");
    printf("[pwd]           Show current working directory.\n");
    printf("[get]           Downloads fil to main filesystem.\n");

    printf("[put]           Uploads file to Nucleus FileSystem.\n");
    printf("[cp]            Copy file to entered absolute adress.\n");
    printf("********************************************************************************\n");
}















#endif // SHELL_C_INCLUDED
