#include <CONIO.H>
#include <DOS.H>
#include <IOSTREAM.H>
#include <STDIO.H>
#include "TRANSFER.H"
#include "UTILS.H"

Transfer::Transfer()
{
	forwarding = 0;
	epgm = new ExecPgm();
	config = ConfigClass::getinstance();
}

int Transfer::download_operations()
{
	int returnval = 0;

	char movefile[MAX_CONFIG_SIZE] = {'\0'};
	char downloadparams[MAX_CONFIG_SIZE] = {'\0'};
	char downloadfolder[MAX_CONFIG_SIZE] = {'\0'};

	strcpy(downloadparams, config->getconfigvalue(DOWNLOADPARAMS));
	strcpy(downloadfolder, config->getconfigvalue(DOWNLOADFOLDER));

	replacesubstring(downloadparams, "{DOWNLOAD}", downloadfolder);

	if ((config->get_local_path()[0] != '\0') && (config->getconfigvalue(DOWNLOADPARAMS)[0] != '\0'))
	{
		// Execute the ZM program for download

		returnval = epgm->execprogram(config->getconfigvalue(EXECPATH), downloadparams);
		if (returnval == -1)
		{
			returnval = -1;
			println("Execution Error");
		}

		// check to see if a ZMMGR.CFG was send and
		// move it to the executable folder to be executed
		sprintf(movefile, "%s\\ZMMGR.CFG", config->getconfigvalue(DOWNLOADPARAMS));
		if (fileexists(movefile) != -1)
		{
			sprintf(movefile, "move %s %s /Y", movefile, config->get_local_path());
			println("ZMMGR.CFG Found moving file: %s", movefile);
			epgm->execdoscommand(movefile);
		}
	}

	return returnval;
}

int Transfer::uploadfiles(char *uploadparams)
{
	int returnval = 0;

	//println("upload params = %s", uploadparams);

	returnval = epgm->execprogram(config->getconfigvalue(EXECPATH), uploadparams);
	// Execute the ZM program for upload
	if (returnval == -1)
	{
		perror("Error executing download\n");
		perror("the files will be deleted in the upload folder\n");
	}

	return returnval;
}

int Transfer::upload_operations(char *filename)
{
	char uploadparams[MAX_CONFIG_SIZE] = {"\0"};
	char uploadfolder[MAX_CONFIG_SIZE] = {"\0"};
	int returnval = TRUE;

	strcpy(uploadparams, config->getconfigvalue(UPLOADPARAMS));

	if (filename == NULL)
	{ // No files listed in a .FIL text file
		strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));
		replacesubstring(uploadparams, "{UPLOAD}", uploadfolder);
		returnval = uploadfiles(uploadparams);
	}
	else
	{ // Files listed in a .FIL text file
		memset(uploadparams, 0, MAX_CONFIG_SIZE);
		strcpy(uploadparams, config->getconfigvalue(FORWARDPARAMS));
		replacesubstring(uploadparams, "{FORWARD}", filename);

		returnval = uploadfiles(uploadparams);
	}

	return returnval;
}