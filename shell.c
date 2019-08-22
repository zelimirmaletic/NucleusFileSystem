#ifndef SHELL_C_INCLUDED
#define SHELL_C_INCLUDED
#include "shell.h"
#include "disk.h"

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
    else
        return -1;
}
void printHelp(void)
{
    printf("**********************************HELP******************************************\n");
    printf("[format]  Formats empty disk and prepares it for mounting a file system.Returns an error if disk is already mounted\n");
    printf("[sudo-format] Formats disk no matter what is on it. Requres root privileges\n");
    printf("[mount] Mounts file system on already formatted disk\n");
    printf("[help] Prints help menu with all available commands and their descriptions\n");
    printf("[exit] Exits file system shell, and saves all changes to the disk\n");
    printf("[debug] Shows internal structure of file system. Superblock and all inodes\n");
    printf("[ls] Lists all files and folders\n");
    printf("[mkdir] Makes new directory on current absolute adress. It should be followed by space and folder name which can have at most 30 characters\n");
    printf("[create] Makes a new file on current absolute adress. It should be followed by space and file name which can have at most 30 characters\n");
    printf("[rename] Renames file/folder. It should be followed by:space, old file/folder name, space, new file/folder name\n");
    printf("[cat]  Prints file contence. It should be followed by a valid file name\n");
    printf("[stat] Prints inode that resresents selected file/folder with all meta data. Should be followed by: space, valid file/folder name\n");
    printf("[cd]   Changes the folder position. Should be followed by a valid folder name\n");
    printf("[cd..] or [cd/..] Returns to root/ folder\n");
    printf("[echo] Writes text to file. Should be followed by: valid file name, text data\n");
    printf("********************************************************************************\n");
}















#endif // SHELL_C_INCLUDED
