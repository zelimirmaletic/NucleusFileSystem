#ifndef SHELL_H_INCLUDED
#define SHELL_H_INCLUDED
#include "fileSystem.h"
#include "disk.h"
#include "shell.c"
//Global variables
char currentAbsoluteAdress[35];

short getCommandNumber(const char *commandEntered);
void printHelp(void);

#endif // SHELL_H_INCLUDED
