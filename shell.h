#ifndef SHELL_H_INCLUDED
#define SHELL_H_INCLUDED
//Global variables
char currentAbsoluteAdress[35];

short getCommandNumber(const char *commandEntered);
int executeCommand(char *commandEntered);
void printHelp(void);

#endif // SHELL_H_INCLUDED
