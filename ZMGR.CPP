#include <CONIO.H>
#include <DOS.H>
#include <PROCESS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#define MENU_WIDTH 40
#define MENU_LEFT 20
#define MENU_TOP 4

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
	print_centered(MENU_TOP + 1, "ZMGR v2");
	print_centered(MENU_TOP + 2, "ZModem Manager");
	draw_separator(MENU_LEFT, MENU_TOP + 3, MENU_WIDTH);

	gotoxy(MENU_LEFT + 4, MENU_TOP + 5);
	cprintf("1. File Transfer");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 6);
	cprintf("2. Forward File");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 7);
	cprintf("3. Serial Pass-through");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 8);
	cprintf("4. Port Diagnostics");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 9);
	cprintf("5. Configuration");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 10);
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
			if (strchr(value, '\n') != NULL)
				*strchr(value, '\n') = '\0';
			if (strchr(value, '\r') != NULL)
				*strchr(value, '\r') = '\0';
			fclose(file);
			return 1;
		}
	}

	fclose(file);
	return 0;
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
			runzmgr(argv[0]);
		else if (strcmp(argval, "-I") == 0)
			execute_tool(argv[0], "ZMFT.EXE", "FORWARD");
		else if (strcmp(argval, "-F") == 0 || strcmp(argval, "-FF") == 0)
			execute_tool(argv[0], "ZMFT.EXE", "FORWARD");
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
