#include "PROCMAN.H"
#include "SERIAL.H"

#define EXPRESS_BRIDGE_IDLE_SECONDS 30

void draw_process_border(int left, int top, int width, int height)
{
	int i;

	gotoxy(left, top);
	putch(201);
	for (i = 0; i < width - 2; i++)
		putch(205);
	putch(187);

	for (i = 1; i < height - 1; i++)
	{
		gotoxy(left, top + i);
		putch(186);
		gotoxy(left + width - 1, top + i);
		putch(186);
	}

	gotoxy(left, top + height - 1);
	putch(200);
	for (i = 0; i < width - 2; i++)
		putch(205);
	putch(188);
}

void draw_process_separator(int left, int row, int width)
{
	int i;

	gotoxy(left, row);
	putch(204);
	for (i = 0; i < width - 2; i++)
		putch(205);
	putch(185);
}

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

int ProcessManager::get_route_port_for_machine(char *machine)
{
	FILE *file;
	char routepath[MAX_FILE_SIZE] = {"\0"};
	char line[MAX_CONFIG_SIZE];
	char cleanmachine[MAX_FILENAME_LENGTH] = {"\0"};

	strcpy(cleanmachine, machine);
	while (cleanmachine[0] == ' ')
		memmove(cleanmachine, cleanmachine + 1, strlen(cleanmachine));
	to_upper_case(cleanmachine);

	if (config->get_local_path()[0] != '\0')
	{
		strcpy(routepath, config->get_local_path());
		strcat(routepath, "ZMROUTES.TXT");
		file = fopen(routepath, "r");
	}
	else
		file = NULL;

	if (file == NULL)
		file = fopen("ZMROUTES.TXT", "r");
	if (file == NULL)
		return 0;

	while (fgets(line, sizeof(line), file) != NULL)
	{
		char *equal;
		char name[MAX_FILENAME_LENGTH] = {"\0"};
		char value[MAX_FILENAME_LENGTH] = {"\0"};

		if (line[0] == '#')
			continue;

		equal = strchr(line, '=');
		if (equal == NULL)
			continue;

		*equal = '\0';
		strcpy(name, line);
		strcpy(value, equal + 1);
		remove_char(value, '\n');
		remove_char(value, '\r');
		while (name[0] == ' ')
			memmove(name, name + 1, strlen(name));
		while (value[0] == ' ')
			memmove(value, value + 1, strlen(value));
		while (strlen(name) > 0 && name[strlen(name) - 1] == ' ')
			name[strlen(name) - 1] = '\0';
		while (strlen(value) > 0 && value[strlen(value) - 1] == ' ')
			value[strlen(value) - 1] = '\0';
		to_upper_case(name);
		to_upper_case(value);

		if (strcmp(name, cleanmachine) == 0)
		{
			fclose(file);
			if (strcmp(value, "LOCAL") == 0)
				return 0;
			return atoi(value);
		}
	}

	fclose(file);
	return 0;
}

static void trim_process_value(char *value)
{
	while (value[0] == ' ')
		memmove(value, value + 1, strlen(value));
	while (strlen(value) > 0 && value[strlen(value) - 1] == ' ')
		value[strlen(value) - 1] = '\0';
	remove_char(value, '\n');
	remove_char(value, '\r');
}

static int first_process_file_of_type(char *folder, char *extension, char *fullpath)
{
	int filecount;
	int allocated_count;
	char **filearray;
	MemoryManager *memmgr;

	filecount = count_files_of_type(folder, extension);
	if (filecount <= 0)
		return FALSE;

	allocated_count = filecount;
	memmgr = new MemoryManager();
	filearray = memmgr->allocate_string_array(allocated_count, MAX_FILE_SIZE);
	filecount = list_files_of_type(folder, extension, filearray, allocated_count, MAX_FILE_SIZE);
	if (filecount <= 0)
	{
		memmgr->free_string_array(filearray, allocated_count);
		delete (memmgr);
		return FALSE;
	}

	sprintf(fullpath, "%s\\%s", folder, filearray[0]);
	to_upper_case(fullpath);
	memmgr->free_string_array(filearray, allocated_count);
	delete (memmgr);
	return TRUE;
}

static int read_express_route_name(char *filename, char *keyname, char *machine)
{
	FILE *file;
	char line[MAX_FILE_SIZE] = {"\0"};

	machine[0] = '\0';
	file = fopen(filename, "r");
	if (file == NULL)
		return ERROR;

	while (fgets(line, sizeof(line), file) != NULL)
	{
		char *equal = strchr(line, '=');
		char key[MAX_FILENAME_LENGTH] = {"\0"};
		char value[MAX_FILE_SIZE] = {"\0"};

		if (equal == NULL)
			continue;

		*equal = '\0';
		strcpy(key, line);
		strcpy(value, equal + 1);
		trim_process_value(key);
		trim_process_value(value);
		to_upper_case(key);

		if (strcmp(key, keyname) == 0)
		{
			strcpy(machine, value);
			to_upper_case(machine);
			fclose(file);
			return TRUE;
		}
	}

	fclose(file);
	return ERROR;
}

int ProcessManager::get_routed_upload_port(void)
{
	char uploadfolder[MAX_CONFIG_SIZE] = {"\0"};
	char machine[MAX_FILENAME_LENGTH] = {"\0"};
	char multifile[MAX_FILE_SIZE] = {"\0"};
	char controlfile[MAX_FILE_SIZE] = {"\0"};
	char recipients[MAX_FILE_SIZE] = {"\0"};
	int filecount;
	int allocated_count;
	char **filearray;
	MemoryManager *memmgr;
	int routed_port = 0;
	char *dot;
	char *comma;
	FILE *file;

	strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));
	sprintf(multifile, "%s\\MULTI.FIL", uploadfolder);
	if (fileexists(multifile) != ERROR)
	{
		file = fopen(multifile, "r");
		if (file != NULL)
		{
			if (fgets(recipients, sizeof(recipients), file) != NULL)
			{
				remove_char(recipients, '\n');
				remove_char(recipients, '\r');
				comma = strchr(recipients, ',');
				if (comma != NULL)
					*comma = '\0';
				while (recipients[0] == ' ')
					memmove(recipients, recipients + 1, strlen(recipients));
				to_upper_case(recipients);
				routed_port = get_route_port_for_machine(recipients);
				if (routed_port > 0)
					println("Route %s via COM%d", recipients, routed_port);
			}
			fclose(file);
		}
		return routed_port;
	}

	if (first_process_file_of_type(uploadfolder, "XFL", controlfile) == TRUE)
	{
		if (read_express_route_name(controlfile, "TARGET", machine) == TRUE)
		{
			routed_port = get_route_port_for_machine(machine);
			if (routed_port > 0)
				println("Route %s via COM%d", machine, routed_port);
		}
		return routed_port;
	}

	filecount = count_files_of_type(uploadfolder, "FIL");
	if (filecount <= 0)
		return 0;

	allocated_count = filecount;
	memmgr = new MemoryManager();
	filearray = memmgr->allocate_string_array(allocated_count, MAX_FILE_SIZE);
	filecount = list_files_of_type(uploadfolder, "FIL", filearray, allocated_count, MAX_FILE_SIZE);

	if (filecount > 0)
	{
		int i;

		for (i = 0; i < filecount; i++)
		{
			strcpy(machine, filearray[i]);
			to_upper_case(machine);
			dot = strchr(machine, '.');
			if (dot != NULL)
				*dot = '\0';
			routed_port = get_route_port_for_machine(machine);
			if (routed_port > 0)
			{
				println("Route %s via COM%d", machine, routed_port);
				break;
			}
		}
	}

	memmgr->free_string_array(filearray, allocated_count);
	delete (memmgr);
	return routed_port;
}

int ProcessManager::build_port_order(int *ports, int portcount, int routed_port, int avoid_port, int *ordered_ports, int max_ports)
{
	int ordered_count = 0;
	int i;

	if (routed_port > 0 && ordered_count < max_ports)
	{
		ordered_ports[ordered_count] = routed_port;
		ordered_count++;
	}

	for (i = 0; i < portcount && ordered_count < max_ports; i++)
	{
		if (ports[i] == routed_port)
			continue;
		if (routed_port == 0 && ports[i] == avoid_port)
			continue;
		ordered_ports[ordered_count] = ports[i];
		ordered_count++;
	}

	if (routed_port == 0 && avoid_port > 0 && ordered_count < max_ports)
	{
		ordered_ports[ordered_count] = avoid_port;
		ordered_count++;
	}

	return ordered_count;
}

int ProcessManager::get_pending_xfl_endpoints(char *source, char *target)
{
	char xflfile[MAX_FILE_SIZE] = {"\0"};

	source[0] = '\0';
	target[0] = '\0';

	if (first_process_file_of_type(config->getconfigvalue(UPLOADFOLDER), "XFL", xflfile) == FALSE)
		return FALSE;

	if (read_express_route_name(xflfile, "SOURCE", source) != TRUE)
		return FALSE;
	if (read_express_route_name(xflfile, "TARGET", target) != TRUE)
		return FALSE;

	return TRUE;
}

int ProcessManager::is_express_relay_for(char *source, char *target)
{
	char computername[MAX_FILENAME_LENGTH] = {"\0"};

	strcpy(computername, config->getconfigvalue(COMPUTERNAME));
	to_upper_case(computername);

	if (strcmp(computername, source) == 0)
		return FALSE;
	if (strcmp(computername, target) == 0)
		return FALSE;

	return TRUE;
}

int ProcessManager::get_bridge_peer_port(int outbound_port, int inbound_port, int *ports, int portcount)
{
	int i;

	if (inbound_port > 0 && inbound_port != outbound_port)
		return inbound_port;

	if (portcount == 2)
	{
		for (i = 0; i < portcount; i++)
		{
			if (ports[i] != outbound_port)
				return ports[i];
		}
	}

	return 0;
}

int ProcessManager::get_express_bridge_wait_seconds(void)
{
	int value = atoi(config->getconfigvalue(EXPRESSBRIDGEWAIT));

	if (value <= 0)
		return 30;

	return value;
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
	int ordered_ports[MAX_ARGS];
	int portcount = 0;
	int ordered_portcount = 0;
	int routed_port = 0;
	int avoid_upload_port = 0;
	int i;

	portcount = get_port_list(ports, MAX_ARGS);

	while (1)
	{
		routed_port = get_routed_upload_port();
		ordered_portcount = build_port_order(ports, portcount, routed_port, avoid_upload_port, ordered_ports, MAX_ARGS);

		for (i = 0; i < ordered_portcount; i++)
		{
			returnval = 0;
			uploadhandled = FALSE;
			clrscr();
			printf("FILE TRANSFER COM%d :: I AM \"%s\" :: ", ordered_ports[i], config->getconfigvalue(COMPUTERNAME));
			if (routed_port == ordered_ports[i])
				printf("ROUTED :: ");
			else if (routed_port == 0 && i == 0)
				printf("NO ROUTE :: ");
			monitormemory();

			/***
			 * First Check for any uploads
			 */
			if (upmanager->checkforuploads() > 0)
			{
				char xfl_source[MAX_FILENAME_LENGTH] = {"\0"};
				char xfl_target[MAX_FILENAME_LENGTH] = {"\0"};
				int pending_xfl_relay = FALSE;
				int inbound_port = avoid_upload_port;
				int outbound_port = ordered_ports[i];
				int bridge_port = 0;

				println("Uploads Found");
				uploadhandled = TRUE;
				if (get_pending_xfl_endpoints(xfl_source, xfl_target) == TRUE)
					pending_xfl_relay = is_express_relay_for(xfl_source, xfl_target);

				if (routed_port == 0 && avoid_upload_port == outbound_port)
				{
					println("No route yet; not sending back on COM%d", outbound_port);
				}
				else if (upmanager->beginuploads(outbound_port) == -1)
				{
					println("Error during uploads");
				}
				else
				{
					if (pending_xfl_relay == TRUE)
					{
						bridge_port = get_bridge_peer_port(outbound_port, inbound_port, ports, portcount);
						if (bridge_port > 0)
						{
							SerialManager serialmanager;
							int bridge_wait_seconds = get_express_bridge_wait_seconds();

							println("Express .XFL relayed; bridge mode COM%d <-> COM%d", bridge_port, outbound_port);
							serialmanager.passthrough_until_idle_after_traffic(bridge_port,
																			outbound_port,
																			TRUE,
																			EXPRESS_BRIDGE_IDLE_SECONDS,
																			bridge_wait_seconds);
						}
					}
					avoid_upload_port = 0;
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
					{
						println("forward files found and moved to %s folder", config->getconfigvalue(UPLOADFOLDER));
						avoid_upload_port = ordered_ports[i];
						i = ordered_portcount;
					}
					else
						println("Forward Files found for ME! Moved to the %s folder", config->getconfigvalue(KEEPFOLDER));
				}
				else if (returnval == ERROR)
					println("Error in checking for forward files");
				else
					downmanager->begindownload(ordered_ports[i]);
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
	char sourcecomputer[MAX_FILENAME_LENGTH] = {"\0"};
	char filepath[MAX_FILE_SIZE] = {"\0"};
	char forwardfilename[MAX_FILE_SIZE] = {"\0"};
	int is_multi = FALSE;
	int use_express = FALSE;
	FILE *forwardfile;

	clrscr();
	draw_process_border(6, 4, 68, 16);
	gotoxy(31, 5);
	cprintf("Forward File");
	draw_process_separator(6, 6, 68);
	gotoxy(10, 8);
	cprintf("Forward to machine:");
	gotoxy(30, 8);
	fgets(computername, sizeof(computername), stdin);
	remove_char(computername, '\n');
	remove_char(computername, '\r');
	to_upper_case(computername);

	gotoxy(10, 10);
	cprintf("Full path to file:");
	gotoxy(29, 10);
	fgets(filepath, sizeof(filepath), stdin);
	remove_char(filepath, '\n');
	remove_char(filepath, '\r');

	if (computername[0] == '\0' || filepath[0] == '\0')
	{
		gotoxy(10, 13);
		cprintf("Forward file cancelled");
		gotoxy(10, 15);
		cprintf("Press any key to continue");
		getch();
		return;
	}

	if (fileexists(filepath) == ERROR)
	{
		gotoxy(10, 13);
		cprintf("File not found");
		gotoxy(10, 14);
		cprintf("%.58s", filepath);
		gotoxy(10, 15);
		cprintf("Press any key to continue");
		getch();
		return;
	}

	is_multi = strchr(computername, ',') != NULL;
	if (is_multi == FALSE)
	{
		char answer;
		gotoxy(10, 12);
		cprintf("Express handshake? (Y/N):");
		answer = getch();
		if (answer == 'y' || answer == 'Y')
			use_express = TRUE;
		gotoxy(35, 12);
		if (use_express == TRUE)
			cprintf("Y");
		else
			cprintf("N");
	}

	if (is_multi == TRUE)
		sprintf(forwardfilename, "%s\\MULTI.FIL", config->getconfigvalue(UPLOADFOLDER));
	else if (use_express == TRUE)
		sprintf(forwardfilename, "%s\\%s.XFL", config->getconfigvalue(UPLOADFOLDER), computername);
	else
		sprintf(forwardfilename, "%s\\%s.FIL", config->getconfigvalue(UPLOADFOLDER), computername);
	to_upper_case(forwardfilename);

	forwardfile = fopen(forwardfilename, "w");
	if (forwardfile == NULL)
	{
		gotoxy(10, 13);
		cprintf("Unable to create forward file");
		gotoxy(10, 14);
		cprintf("%.58s", forwardfilename);
		gotoxy(10, 15);
		cprintf("Press any key to continue");
		getch();
		return;
	}

	if (is_multi == TRUE)
	{
		fprintf(forwardfile, "%s\n", computername);
		fprintf(forwardfile, "%s\n", filepath);
	}
	else if (use_express == TRUE)
	{
		strcpy(sourcecomputer, config->getconfigvalue(COMPUTERNAME));
		to_upper_case(sourcecomputer);
		fprintf(forwardfile, "SOURCE=%s\n", sourcecomputer);
		fprintf(forwardfile, "TARGET=%s\n", computername);
		fprintf(forwardfile, "PAYLOAD=%s\n", filepath);
	}
	else
	{
		fprintf(forwardfile, "%s\n", filepath);
		fprintf(forwardfile, "%s\n", forwardfilename);
	}
	fclose(forwardfile);

	gotoxy(10, 14);
	cprintf("Created forwarding file");
	gotoxy(10, 15);
	cprintf("%.58s", forwardfilename);
	gotoxy(10, 17);
	cprintf("Press any key to continue");
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
