#include <CONIO.H>
#include <DOS.H>
#include <STDIO.H>
#include "TRANSFER.H"
#include "UTILS.H"

Transfer::Transfer()
{
	epgm = new ExecPgm();
}

void Transfer::DownloadOperations(ConfigClass *config)
{
	config->updateConfig();

	if ((config->path[0] != '\0') && (config->downloadparams[0] != '\0'))
	{
		// Execute the ZM program for download
		if (epgm->execprogram(config->path, config->downloadparams) == -1)
			printf("Execution Error\n");
		else
		{
			char checkfile[100] = {'\0'};
			char movefile[100] = {'\0'};
			// check to see if a ZM.CFG was send and
			// move it to the executable folder to be executed
			sprintf(checkfile, "%s\\ZM.CFG", config->downloadfolder);
			if (check_for_file(checkfile) > -1)
			{
				sprintf(movefile, "move %s %s /Y", checkfile, config->localpath);
				printf("ZM.CFG Found moving file: %s\n", movefile);
				epgm->execdoscommand(movefile);
			}
		}
	}
	else
		printf("One must fill in the gaps, in order to not fail\n");
}

void Transfer::UploadOperations(ConfigClass *config)
{
	char checkfile[100] = {'\0'};
	char *file_list[MAX_FILES];
	int count = 0;

	config->updateConfig();
	sprintf(checkfile, "%s\\*.*", config->uploadfolder);
	count = get_files_in_directory(checkfile, file_list);
	(count > 0) ? printf("[Upload File Count = %d]\n", count) : printf("[Skipping Upload Operations]\n", count);

	if (count > 0)
	{

		// Execute the ZM program for upload
		if (epgm->execprogram(config->path, config->uploadparams) == -1)
			perror("Error executing download\n");
		else
		{
			for (int i = 0; i < count; i++)
			{
				sprintf(checkfile, "%s\\%s", config->uploadfolder, file_list[i]);
				printf("Removing File %s\n", checkfile);
				if (remove(checkfile) == -1)
					perror(checkfile);
			}
		}
	}

	for (int i = 0; i < count; i++)
		free(file_list[i]);

	free(file_list);
}