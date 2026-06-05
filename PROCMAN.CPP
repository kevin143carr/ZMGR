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

void ProcessManager::execute_main_loop(void)
{
	int returnval = 0;
	int uploadhandled = FALSE;
	while (1)
	{
		returnval = 0;
		uploadhandled = FALSE;
		clrscr();
		printf("I AM \"%s\" :: ", config->getconfigvalue(COMPUTERNAME));
		monitormemory();

		/***
		 * First Check for any uploads
		 */
		if (upmanager->checkforuploads() > 0)
		{
			println("Uploads Found");
			uploadhandled = TRUE;
			if (upmanager->beginuploads() == -1)
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
				downmanager->begindownload();
		}

		if ((config->getconfigvalue(ONCEEXEC)[0] != '\0'))
		{
			println("Executing onceexec [%s] with [%s]", config->getconfigvalue(ONCEEXEC), config->getconfigvalue(DOWNLOADPARAMS));
			epgm->execprogram(config->getconfigvalue(ONCEEXEC), config->getconfigvalue(DOWNLOADPARAMS));
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

		// *****************************

		/***
		 * This portion of code checks for keyboard input
		 * and then releases to the main loop.
		 */

		/* 		// Trying randomized pausing to stagger UL/DL processes
				// between connected machines
				int randpause = getrandomnumber(1000, (atoi(config->getconfigvalue(PAUSETIME)) * 1000));
				println("PAUSING %d MS ...", randpause);
				delay(randpause); */
		if (pause_or_exit() == TRUE)
			break;
	}
}

void ProcessManager::execute_auto_download_loop(void)
{
	while (1)
	{
		clrscr();
		printf("AUTO DOWNLOAD :: I AM \"%s\" :: ", config->getconfigvalue(COMPUTERNAME));
		monitormemory();
		downmanager->begindownload();

		if (config->getconfigvalue(ZMGRSECRET)[0] != '\0')
			secretThings->dosecretthings(config->getconfigvalue(ZMGRSECRET));

		if (pause_or_exit() == TRUE)
			break;
	}
}

void ProcessManager::execute_passthrough_mode(void)
{
	int port_a = 0;
	int port_b = 0;
	int watch_zmodem = TRUE;
	char input[10] = {"\0"};

	clrscr();
	println("Serial Pass-through Mode");
	println("");
	println("Enter first COM port number:");
	gets(input);
	port_a = atoi(input);

	memset(input, 0, sizeof(input));
	println("Enter second COM port number:");
	gets(input);
	port_b = atoi(input);

	serialmanager->passthrough(port_a, port_b, watch_zmodem);
}

void ProcessManager::show_port_diagnostics(void)
{
	serialmanager->show_diagnostics();
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
	serialmanager = new SerialManager();
}

void ProcessManager::set_local_path(char *path)
{
	config->set_local_path(path);
}
