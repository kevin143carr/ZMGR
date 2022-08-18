#ifndef EXECPGM_H_INCLUDED
#define EXECPGM_H_INCLUDED

#include <CONIO.H>
#include <STDIO.H>
#include <PROCESS.H>
#include "UTILS.H"

class ExecPgm
{
public:
	int execprogram(const char *path, char *args);
	int execdoscommand(char *command);
};
#endif