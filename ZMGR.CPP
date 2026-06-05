#include <CONIO.H>
#include <DOS.H>
#include <STDIO.H>
#include "CONFIGCL.H"
#include "PROCMAN.H"

void titlescreen()
{
	clrscr();
	println("[Public Domain Zmodem] [Manager]");
	println(" Public Domain Zmodem by Peter Mandrella, 1994-1998");
	println(" ZMGR Version v0.1.0 ZModem - Manager");
	println(" Written in Borland C++ 3.1 by Kevin Carr, 2022");
	println(" Updated July 2024: Added auto up/down automation");
	println(" This is a MS-DOS program");
	println(" All actions handled by the ZMMGR.CFG file\n");
}

void printhelp()
{
	titlescreen();
	println("#### - FORWARDING FILES GO INTO UPLOAD FOLDER! - ####");
	println("#### - Use [ZMGR.EXE run] to open the mode menu");
	println("#### - Use [ZMGR.EXE -f] <NAMEOFCOMPUTER> to create forward file");
	println("#### - Use [ZMGR.EXE -i] (interactive) asks for <NAMEOFCOMPUTER>");
}

int show_main_menu()
{
	char key;

	while (1)
	{
		titlescreen();
		println("Select operating mode:");
		println("");
		println("1. Manager Forwarding");
		println("2. Auto Download");
		println("3. Serial Pass-through");
		println("4. Port Diagnostics");
		println("X. Exit");
		println("");

		key = getch();
		if (key == '1' || key == '2' || key == '3' || key == '4' ||
			key == 'x' || key == 'X')
			return key;
	}
}

void createfilelistfile()
{
	ConfigClass *config = ConfigClass::getinstance();
	char filename[MAX_FILE_SIZE] = {"\0"};
	char fullfilename[MAX_FILE_SIZE] = {"\0"};

	titlescreen();
	println("");
	println("This will create a forwarding file for a computer");
	println("in the %s upload directory",
			config->getconfigvalue(UPLOADFOLDER));

	println("Enter the name of computer which will receive the files:");
	gets(filename);
	sprintf(fullfilename, "%s\\%s.FIL", config->getconfigvalue(UPLOADFOLDER), filename);
	to_upper_case(fullfilename);
	listdirectory2file(config->getconfigvalue(UPLOADFOLDER), fullfilename, FALSE, ".FIL");
	appendstring2file(fullfilename, fullfilename);
}

int runzmgr(char *path)
{
	ProcessManager *procman = new ProcessManager();
	ConfigClass *config = ConfigClass::getinstance();
	int choice = 0;

	procman->set_local_path(path);

	createdirectory(config->getconfigvalue(DOWNLOADFOLDER));
	createdirectory(config->getconfigvalue(UPLOADFOLDER));
	createdirectory(config->getconfigvalue(KEEPFOLDER));

	while (1)
	{
		choice = show_main_menu();

		if (choice == '1')
			procman->execute_main_loop();
		else if (choice == '2')
			procman->execute_auto_download_loop();
		else if (choice == '3')
			procman->execute_passthrough_mode();
		else if (choice == '4')
			procman->show_port_diagnostics();
		else if (choice == 'x' || choice == 'X')
			break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	char filename[MAX_FILE_SIZE] = {"\0"};
	char argval[MAX_FILENAME_LENGTH] = {"\0"};
	ConfigClass *config = ConfigClass::getinstance();

	if (argc >= 2)
	{
		strcpy(argval, argv[1]);
		to_upper_case(argval);

		// Check for the -f flag and call processFile
		if (strcmp(argval, "-F") == 0)
		{
			if (argv[2] != NULL)
			{
				sprintf(filename, "%s\\%s.FIL", config->getconfigvalue(UPLOADFOLDER), argv[2]);
				to_upper_case(filename);
				printf("CREATED FORWARDING FILE [%s]", filename);
				listdirectory2file(config->getconfigvalue(UPLOADFOLDER), filename, TRUE, ".FIL");
				appendstring2file(filename, filename);
			}
			else
				println("ERROR IN COMMANDLINE - MISSING NAME");
			return 0;
		}
		if (strcmp(argval, "-FF") == 0)
		{
			if (argv[2] != NULL)
			{
				sprintf(filename, "%s\\%s.FIL", config->getconfigvalue(UPLOADFOLDER), argv[2]);
				to_upper_case(filename);
				printf("CREATED FORWARDING FILE [%s]", filename);
				listdirectory2file(config->getconfigvalue(UPLOADFOLDER), filename, FALSE, ".FIL");
				appendstring2file(filename, filename);
			}
			else
				println("ERROR IN COMMANDLINE - MISSING NAME");
			return 0;
		}
		else if (strcmp(argval, "-I") == 0)
		{
			createfilelistfile();
			runzmgr(argv[0]);
		}
		else if (strcmp(argval, "RUN") == 0)
		{
			runzmgr(argv[0]);
		}
		else
		{
			void printhelp();
			println("Invalid Option: %s", argval);
			return 1;
		}
	}
	else
		printhelp();

	return 0;
}
