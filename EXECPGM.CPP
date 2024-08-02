#include "execpgm.h"

int ExecPgm::execprogram(const char *path, char *args)
{
	int status = -1;

	//println("Executing %s with %s", path, args);

	status = spawnl(P_WAIT, (char *)path, (char *)path, args, NULL);
	if (status == -1)
	{
		char errorstr[100];
		sprintf(errorstr, "Error Spawning %s with %s\n", path, args);
		perror(errorstr);
	}

	return status;
}

int ExecPgm::execdoscommand(char *command)
{
	int status = -1;
	if (system(command) == 0)
		status = 0;

	return status;
}
