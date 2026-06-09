#include <CONIO.H>
#include <DOS.H>
#include <PROCESS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include "APPINFO.H"

#define MENU_WIDTH 40
#define MENU_LEFT 20
#define MENU_TOP 4
#define CFG_LINE_LEN 180

void strip_newline(char *value)
{
	char *pos;

	pos = strchr(value, '\n');
	if (pos != NULL)
		*pos = '\0';
	pos = strchr(value, '\r');
	if (pos != NULL)
		*pos = '\0';
}

void trim_value(char *value)
{
	char *start;
	char *end;

	start = value;
	while (*start == ' ')
		start++;
	if (start != value)
		memmove(value, start, strlen(start) + 1);

	end = value + strlen(value);
	while (end > value && (*(end - 1) == ' ' || *(end - 1) == '\n' || *(end - 1) == '\r'))
	{
		*(end - 1) = '\0';
		end--;
	}
}

void print_centered(int row, const char *text)
{
	int col = MENU_LEFT + ((MENU_WIDTH - strlen(text)) / 2);
	gotoxy(col, row);
	cprintf("%s", text);
}

void draw_border(int left, int top, int width, int height)
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

void draw_separator(int left, int row, int width)
{
	int i;

	gotoxy(left, row);
	putch(204);
	for (i = 0; i < width - 2; i++)
		putch(205);
	putch(185);
}

void titlescreen()
{
	_setcursortype(_NOCURSOR);
	clrscr();
}

void printhelp()
{
	titlescreen();
	printf("[Public Domain Zmodem] [Manager]\n");
	printf("Use [ZMGR.EXE run] to open the mode menu\n");
	printf("ZMGR launches helper EXEs and returns here when they exit\n");
}

void draw_main_menu(int countdown, int countdown_active)
{
	titlescreen();
	draw_border(MENU_LEFT, MENU_TOP, MENU_WIDTH, 17);
	print_centered(MENU_TOP + 1, "ZMGR " ZMGR_VERSION);
	print_centered(MENU_TOP + 2, "ZModem Manager");
	print_centered(MENU_TOP + 3, ZMGR_BUILD);
	draw_separator(MENU_LEFT, MENU_TOP + 4, MENU_WIDTH);

	gotoxy(MENU_LEFT + 4, MENU_TOP + 6);
	cprintf("1. File Transfer");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 7);
	cprintf("2. Forward File");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 8);
	cprintf("3. Serial Pass-through");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 9);
	cprintf("4. Port Diagnostics");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 10);
	cprintf("5. Configuration");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 11);
	cprintf("X. Exit");

	draw_separator(MENU_LEFT, MENU_TOP + 12, MENU_WIDTH);
	gotoxy(MENU_LEFT + 3, MENU_TOP + 13);
	if (countdown_active)
		cprintf("Auto file transfer in %2d sec", countdown);
	else
		cprintf("Countdown cancelled");
	gotoxy(MENU_LEFT + 3, MENU_TOP + 14);
	if (countdown_active)
		cprintf("Press ESC to cancel countdown");
	else
		cprintf("Select an option");
}

void update_countdown_text(int countdown)
{
	gotoxy(MENU_LEFT + 3, MENU_TOP + 13);
	cprintf("Auto file transfer in %2d sec", countdown);
}

int read_config_value(const char *key, char *value, int max_len)
{
	FILE *file;
	char line[180];
	int key_len = strlen(key);

	value[0] = '\0';
	file = fopen("ZMMGR.CFG", "r");
	if (file == NULL)
		return 0;

	while (fgets(line, sizeof(line), file) != NULL)
	{
		if (line[0] == '#')
			continue;
		if (strncmp(line, key, key_len) == 0)
		{
			char *equal = strchr(line, '=');
			char *start;
			if (equal == NULL)
				continue;
			start = equal + 1;
			while (*start == ' ')
				start++;
			strncpy(value, start, max_len - 1);
			value[max_len - 1] = '\0';
			strip_newline(value);
			trim_value(value);
			fclose(file);
			return 1;
		}
	}

	fclose(file);
	return 0;
}

int line_matches_config_key(char *line, const char *key)
{
	char work[CFG_LINE_LEN];
	char *equal;

	if (line[0] == '#')
		return 0;

	strncpy(work, line, sizeof(work) - 1);
	work[sizeof(work) - 1] = '\0';
	equal = strchr(work, '=');
	if (equal == NULL)
		return 0;

	*equal = '\0';
	trim_value(work);
	strupr(work);

	return (strcmp(work, key) == 0);
}

int write_config_backup()
{
	FILE *infile;
	FILE *outfile;
	char line[CFG_LINE_LEN];

	infile = fopen("ZMMGR.CFG", "r");
	if (infile == NULL)
		return 0;

	outfile = fopen("ZMMGR.BAK", "w");
	if (outfile == NULL)
	{
		fclose(infile);
		return 0;
	}

	while (fgets(line, sizeof(line), infile) != NULL)
		fputs(line, outfile);

	fclose(infile);
	fclose(outfile);
	return 1;
}

int update_config_value(const char *key, const char *new_value)
{
	FILE *infile;
	FILE *outfile;
	char line[CFG_LINE_LEN];
	int found = 0;

	write_config_backup();

	infile = fopen("ZMMGR.CFG", "r");
	if (infile == NULL)
		return 0;

	outfile = fopen("ZMMGR.TMP", "w");
	if (outfile == NULL)
	{
		fclose(infile);
		return 0;
	}

	while (fgets(line, sizeof(line), infile) != NULL)
	{
		if (line_matches_config_key(line, key))
		{
			fprintf(outfile, "%s = %s\n", key, new_value);
			found = 1;
		}
		else
			fputs(line, outfile);
	}

	if (!found)
		fprintf(outfile, "%s = %s\n", key, new_value);

	fclose(infile);
	fclose(outfile);

	remove("ZMMGR.CFG");
	if (rename("ZMMGR.TMP", "ZMMGR.CFG") != 0)
		return 0;

	return 1;
}

void prompt_for_required_config(void)
{
	char computername[CFG_LINE_LEN] = {"\0"};
	char ports[CFG_LINE_LEN] = {"\0"};
	int have_computername;
	int have_ports;
	int row;

	have_computername = read_config_value("COMPUTERNAME", computername, sizeof(computername));
	have_ports = read_config_value("PORTS", ports, sizeof(ports));

	if (have_computername && computername[0] != '\0' && have_ports && ports[0] != '\0')
		return;

	_setcursortype(_NORMALCURSOR);
	clrscr();
	draw_border(8, 4, 64, 17);
	gotoxy(27, 5);
	cprintf("ZMGR First-Time Setup");
	draw_separator(8, 6, 64);
	gotoxy(12, 8);
	cprintf("Every machine needs a unique COMPUTERNAME.");
	gotoxy(12, 9);
	cprintf("PORTS can be blank if you want diagnostics to find them.");

	while (computername[0] == '\0')
	{
		gotoxy(12, 11);
		cprintf("Computer name:                                      ");
		gotoxy(27, 11);
		fgets(computername, sizeof(computername), stdin);
		strip_newline(computername);
		trim_value(computername);
		strupr(computername);
		if (computername[0] == '\0')
		{
			gotoxy(12, 13);
			cprintf("COMPUTERNAME is required.                         ");
		}
	}

	if (!update_config_value("COMPUTERNAME", computername))
	{
		gotoxy(12, 13);
		cprintf("Could not update COMPUTERNAME in ZMMGR.CFG");
		gotoxy(12, 14);
		cprintf("Press any key to continue");
		getch();
	}

	if (!have_ports || ports[0] == '\0')
	{
		for (row = 12; row <= 16; row++)
		{
			gotoxy(12, row);
			cprintf("                                                        ");
		}
		gotoxy(12, 12);
		cprintf("COM ports, like 1 or 1,2:");
		gotoxy(12, 13);
		cprintf("Leave blank if unsure.");
		gotoxy(12, 15);
		cprintf("PORTS: ");
		fgets(ports, sizeof(ports), stdin);
		strip_newline(ports);
		trim_value(ports);

		if (!update_config_value("PORTS", ports))
		{
			gotoxy(12, 17);
			cprintf("Could not update PORTS in ZMMGR.CFG");
			gotoxy(12, 18);
			cprintf("Press any key to continue");
			getch();
		}
		else if (ports[0] == '\0')
		{
			gotoxy(12, 17);
			cprintf("Use Port Diagnostics to find and update COM ports.");
			gotoxy(12, 18);
			cprintf("Press any key to continue");
			getch();
		}
	}

	_setcursortype(_NOCURSOR);
}

int get_menu_countdown()
{
	char value[20];
	int seconds;

	if (!read_config_value("MENUCOUNTDOWN", value, sizeof(value)))
		return 20;

	seconds = atoi(value);
	if (seconds < 0)
		seconds = 0;

	return seconds;
}

int startup_filetransfer()
{
	char value[40];

	if (!read_config_value("STARTUPSTATE", value, sizeof(value)))
		return 0;

	strupr(value);
	return (strcmp(value, "FILETRANSFER") == 0);
}

int show_main_menu(int use_countdown)
{
	char key;
	int countdown = get_menu_countdown();
	int countdown_active = (use_countdown && countdown > 0);
	int tick;

	draw_main_menu(countdown, countdown_active);

	while (countdown_active)
	{
		for (tick = 0; tick < 10; tick++)
		{
			if (kbhit())
			{
				key = getch();
				if (key == 27)
				{
					countdown_active = 0;
					draw_main_menu(countdown, countdown_active);
					break;
				}
				if (key == '1' || key == '2' || key == '3' || key == '4' ||
					key == '5' || key == 'x' || key == 'X')
					return key;
			}
			delay(100);
		}

		if (countdown_active)
		{
			countdown--;
			if (countdown <= 0)
				return '1';
			update_countdown_text(countdown);
		}
	}

	while (1)
	{
		key = getch();
		if (key == '1' || key == '2' || key == '3' || key == '4' ||
			key == '5' || key == 'x' || key == 'X')
			return key;
	}
}

void build_tool_path(char *program_path, char *toolname, char *toolpath)
{
	char *lastslash;

	strcpy(toolpath, program_path);
	lastslash = strrchr(toolpath, '\\');

	if (lastslash == NULL)
	{
		strcpy(toolpath, toolname);
		return;
	}

	strcpy(lastslash + 1, toolname);
}

int execute_tool(char *program_path, char *toolname, char *args)
{
	char toolpath[160] = {"\0"};
	int status;

	_setcursortype(_NORMALCURSOR);
	build_tool_path(program_path, toolname, toolpath);
	status = spawnl(P_WAIT, toolpath, toolpath, args, NULL);
	if (status == -1)
		status = spawnl(P_WAIT, toolname, toolname, args, NULL);

	if (status == -1)
	{
		printf("Unable to run %s\n", toolname);
		printf("Press any key to continue\n");
		getch();
	}

	_setcursortype(_NOCURSOR);
	return status;
}

int runzmgr(char *program_path)
{
	int choice = 0;
	int first_menu = 1;

	if (startup_filetransfer())
		execute_tool(program_path, "ZMFT.EXE", "TRANSFER");

	while (1)
	{
		choice = show_main_menu(first_menu);
		first_menu = 0;

		if (choice == '1')
			execute_tool(program_path, "ZMFT.EXE", "TRANSFER");
		else if (choice == '2')
			execute_tool(program_path, "ZMFT.EXE", "FORWARD");
		else if (choice == '3')
			execute_tool(program_path, "ZMSER.EXE", "");
		else if (choice == '4')
			execute_tool(program_path, "ZMDIAG.EXE", "");
		else if (choice == '5')
			execute_tool(program_path, "ZMCFG.EXE", "");
		else if (choice == 'x' || choice == 'X')
		{
			_setcursortype(_NORMALCURSOR);
			clrscr();
			break;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	char argval[100] = {"\0"};

	if (argc >= 2)
	{
		_setcursortype(_NOCURSOR);
		strcpy(argval, argv[1]);
		strupr(argval);

		if (strcmp(argval, "RUN") == 0)
		{
			prompt_for_required_config();
			runzmgr(argv[0]);
		}
		else if (strcmp(argval, "-I") == 0)
		{
			prompt_for_required_config();
			execute_tool(argv[0], "ZMFT.EXE", "FORWARD");
		}
		else if (strcmp(argval, "-F") == 0 || strcmp(argval, "-FF") == 0)
		{
			prompt_for_required_config();
			execute_tool(argv[0], "ZMFT.EXE", "FORWARD");
		}
		else
		{
			printhelp();
			printf("Invalid Option: %s\n", argval);
			return 1;
		}
	}
	else
	{
		_setcursortype(_NORMALCURSOR);
		printhelp();
	}

	_setcursortype(_NORMALCURSOR);
	return 0;
}
