#include "PROCMAN.H"

int ProcessManager::pause_or_exit(void)
{
	println("PAUSING %s SECONDS", config->getconfigvalue(PAUSETIME));
	println("'X' WILL END THIS MADNESS");
	println("'P' WILL PAUSE");
	println("'C' WILL CONTINUE FROM 'PAUSE'");

	setbreak = 0;
	int setpause = FALSE;
	for (int i = 0; i < (atoi(config->getconfigvalue(PAUSETIME)) * 2); i++)
	{
		delay(500);

		keychr = kbhit();
		if (keychr != 0)
		{
			char key = getch();
			if (key == 'x' || key == 'X')
			{
				printf("Break Key Hit\n");
				setbreak = 1;
				break;
			}
			if (key == 'p' || key == 'P')
			{
				println("PAUSED!!");
				setpause = TRUE;
			}
			if (key == 'c' || key == 'C')
			{
				println("UN-PAUSED!!");
				setpause = FALSE;
			}
		}

		if (setpause == TRUE)
			i = 0;

		if (setbreak == 1)
			break;
	}

	return setbreak;
}

int ProcessManager::get_port_list(int *ports, int max_ports)
{
	char portconfig[MAX_CONFIG_SIZE] = {"\0"};
	char *token;
	int count = 0;

	strcpy(portconfig, config->getconfigvalue(PORTS));
	token = strtok(portconfig, ",");

	while (token != NULL && count < max_ports)
	{
		ports[count] = atoi(token);
		if (ports[count] > 0)
			count++;
		token = strtok(NULL, ",");
	}

	if (count == 0)
	{
		ports[0] = 1;
		count = 1;
	}

	return count;
}

void ProcessManager::execute_once_hooks(void)
{
	if ((config->getconfigvalue(ONCEEXEC)[0] != '\0'))
	{
		println("Executing onceexec [%s] with [%s]", config->getconfigvalue(ONCEEXEC), config->getconfigvalue(ZMOPTIONS));
		epgm->execprogram(config->getconfigvalue(ONCEEXEC), config->getconfigvalue(ZMOPTIONS));
	}

	if (config->getconfigvalue(ONCEDOSCOMMAND)[0] != '\0')
	{
		if (epgm->execdoscommand(config->getconfigvalue(ONCEDOSCOMMAND)) == -1)
			perror("Error Executing DOS Command\n");

		// remove_config_file_value("ZMMGR.CFG", "ONCEDOSCOMMAND");
	}

	if (config->getconfigvalue(ZMGRSECRET)[0] != '\0')
	{
		secretThings->dosecretthings(config->getconfigvalue(ZMGRSECRET));
	}
}

void ProcessManager::execute_external_tool(char *toolname)
{
	char toolpath[MAX_FILE_SIZE] = {"\0"};

	strcpy(toolpath, config->get_local_path());
	strcat(toolpath, toolname);

	if (fileexists(toolpath) == ERROR)
		strcpy(toolpath, toolname);

	epgm->execprogram(toolpath, "");
}

void ProcessManager::execute_file_transfer_loop(void)
{
	int returnval = 0;
	int uploadhandled = FALSE;
	int ports[MAX_ARGS];
	int portcount = 0;
	int i;

	portcount = get_port_list(ports, MAX_ARGS);

	while (1)
	{
		for (i = 0; i < portcount; i++)
		{
			returnval = 0;
			uploadhandled = FALSE;
			clrscr();
			printf("FILE TRANSFER COM%d :: I AM \"%s\" :: ", ports[i], config->getconfigvalue(COMPUTERNAME));
			monitormemory();

			/***
			 * First Check for any uploads
			 */
			if (upmanager->checkforuploads() > 0)
			{
				println("Uploads Found");
				uploadhandled = TRUE;
				if (upmanager->beginuploads(ports[i]) == -1)
				{
					println("Error during uploads");
				}
			}

			/***
			 * Check for forward files in the download folder
			 */
			if (uploadhandled == FALSE)
			{
				returnval = downmanager->checkforforwardfiles(config->getconfigvalue(DOWNLOADFOLDER));

				if (returnval == TRUE)
				{
					int rtnval = downmanager->processforwardfiles();
					if (rtnval == ERROR)
						println("Error in processing forward files from %s", config->getconfigvalue(DOWNLOADFOLDER));
					else if (rtnval == TRUE)
						println("forward files found and moved to %s folder", config->getconfigvalue(UPLOADFOLDER));
					else
						println("Forward Files found for ME! Moved to the %s folder", config->getconfigvalue(KEEPFOLDER));
				}
				else if (returnval == ERROR)
					println("Error in checking for forward files");
				else
					downmanager->begindownload(ports[i]);
			}

			execute_once_hooks();

			// *****************************

			/***
			 * This portion of code checks for keyboard input
			 * and then releases to the main loop.
			 */

			if (pause_or_exit() == TRUE)
				return;
		}
	}
}

void ProcessManager::create_forward_file_wizard(void)
{
	char computername[MAX_FILENAME_LENGTH] = {"\0"};
	char filepath[MAX_FILE_SIZE] = {"\0"};
	char forwardfilename[MAX_FILE_SIZE] = {"\0"};
	FILE *forwardfile;

	clrscr();
	println("Forward File");
	println("");
	println("Forward To:");
	fgets(computername, sizeof(computername), stdin);
	remove_char(computername, '\n');
	remove_char(computername, '\r');
	to_upper_case(computername);

	println("Enter full path to file:");
	fgets(filepath, sizeof(filepath), stdin);
	remove_char(filepath, '\n');
	remove_char(filepath, '\r');

	if (computername[0] == '\0' || filepath[0] == '\0')
	{
		println("Forward file cancelled");
		println("Press any key to continue");
		getch();
		return;
	}

	if (fileexists(filepath) == ERROR)
	{
		println("File not found: %s", filepath);
		println("Press any key to continue");
		getch();
		return;
	}

	sprintf(forwardfilename, "%s\\%s.FIL", config->getconfigvalue(UPLOADFOLDER), computername);
	to_upper_case(forwardfilename);

	forwardfile = fopen(forwardfilename, "w");
	if (forwardfile == NULL)
	{
		println("Unable to create %s", forwardfilename);
		println("Press any key to continue");
		getch();
		return;
	}

	fprintf(forwardfile, "%s\n", filepath);
	fprintf(forwardfile, "%s\n", forwardfilename);
	fclose(forwardfile);

	println("Created forwarding file [%s]", forwardfilename);
	println("Press any key to continue");
	getch();
}

void ProcessManager::execute_passthrough_mode(void)
{
	execute_external_tool("ZMSER.EXE");
}

void ProcessManager::show_port_diagnostics(void)
{
	execute_external_tool("ZMDIAG.EXE");
}

ProcessManager::ProcessManager()
{
	config = ConfigClass::getinstance();
	secretThings = new SecretThings();
	epgm = new ExecPgm();
	upmanager = new UploadManager();
	upmanager->setupconfig(config);
	downmanager = new DownloadManager();
	fwdmanager = ForwardManager::getinstance();
}

void ProcessManager::set_local_path(char *path)
{
	config->set_local_path(path);
}
