#include "execpgm.h"

int ExecPgm::execprogram(const char *path, char *args)
{
	int status = -1;
	char *arglist[MAX_ARGS];
	int argcount = 0;
	char *argholder = strdup(args);

	argcount = splitIntoArray(args, " ", arglist);
	if (argcount > 0)
	{
		// DEBUG
		//	printf("Executing Spawn with %d args: \n", argcount);
		//	for(int i =0; i < argcount; i++)
		//		printf("%s ", arglist[i]);
		//	printf("\n");
		//	getch();
		status = spawnv(P_WAIT, (char *)path, arglist);
		if (status == -1)
		{
			char errorstr[100];
			sprintf(errorstr, "Error Spawning %s with %s\n", path, args);
			perror(errorstr);
		}
	}

	for (int i = 0; i < argcount; i++)
	{
		free(arglist[i]);
	}

	free(arglist);
	strcpy(args, argholder);
	free(argholder);

	return status;
}

int ExecPgm::execdoscommand(char *command)
{
	int status = -1;
	if (system(command) == 0)
		status = 0;

	return status;
}
