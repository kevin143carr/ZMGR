#include <CONIO.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#define CFG_LINE_COUNT 160
#define CFG_LINE_LEN 180
#define CFG_KEY_COUNT 9
#define BOX_LEFT 4
#define BOX_TOP 2
#define BOX_WIDTH 72
#define BOX_HEIGHT 20
#define EDIT_LEFT 10
#define EDIT_TOP 7
#define EDIT_WIDTH 60
#define EDIT_HEIGHT 9

const char *cfg_keys[CFG_KEY_COUNT] = {
	"COMPUTERNAME",
	"DOWNLOADFOLDER",
	"UPLOADFOLDER",
	"KEEPFOLDER",
	"EXECPATH",
	"PORTS",
	"ZMOPTIONS",
	"STARTUPSTATE",
	"MENUCOUNTDOWN"
};

char cfg_values[CFG_KEY_COUNT][CFG_LINE_LEN];
char cfg_lines[CFG_LINE_COUNT][CFG_LINE_LEN];
int cfg_line_count = 0;
int changed = 0;

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

int line_matches_key(const char *line, const char *key)
{
	int key_len = strlen(key);

	if (line[0] == '#')
		return 0;
	if (strncmp(line, key, key_len) != 0)
		return 0;
	if (line[key_len] != ' ' && line[key_len] != '=')
		return 0;

	return (strchr(line, '=') != NULL);
}

void read_value_from_line(char *line, char *value)
{
	char *equal = strchr(line, '=');
	char *start;

	value[0] = '\0';
	if (equal == NULL)
		return;

	start = equal + 1;
	while (*start == ' ')
		start++;

	strncpy(value, start, CFG_LINE_LEN - 1);
	value[CFG_LINE_LEN - 1] = '\0';
	strip_newline(value);
}

void clear_values()
{
	int i;

	for (i = 0; i < CFG_KEY_COUNT; i++)
		cfg_values[i][0] = '\0';
}

int load_config()
{
	FILE *file;
	int i;

	clear_values();
	cfg_line_count = 0;

	file = fopen("ZMMGR.CFG", "r");
	if (file == NULL)
		return 0;

	while (cfg_line_count < CFG_LINE_COUNT &&
		   fgets(cfg_lines[cfg_line_count], CFG_LINE_LEN, file) != NULL)
	{
		for (i = 0; i < CFG_KEY_COUNT; i++)
		{
			if (line_matches_key(cfg_lines[cfg_line_count], cfg_keys[i]))
				read_value_from_line(cfg_lines[cfg_line_count], cfg_values[i]);
		}
		cfg_line_count++;
	}

	fclose(file);
	changed = 0;
	return 1;
}

void print_trimmed_value(int row, const char *value)
{
	char display[36];

	strncpy(display, value, sizeof(display) - 1);
	display[sizeof(display) - 1] = '\0';

	gotoxy(BOX_LEFT + 31, row);
	cprintf("%-35s", display);
}

void draw_config_screen()
{
	int i;
	int row;

	_setcursortype(_NOCURSOR);
	clrscr();
	draw_border(BOX_LEFT, BOX_TOP, BOX_WIDTH, BOX_HEIGHT);
	gotoxy(BOX_LEFT + 23, BOX_TOP + 1);
	cprintf("ZMGR Configuration");
	draw_separator(BOX_LEFT, BOX_TOP + 2, BOX_WIDTH);

	for (i = 0; i < CFG_KEY_COUNT; i++)
	{
		row = BOX_TOP + 4 + i;
		gotoxy(BOX_LEFT + 4, row);
		cprintf("%d. %-16s", i + 1, cfg_keys[i]);
		print_trimmed_value(row, cfg_values[i]);
	}

	draw_separator(BOX_LEFT, BOX_TOP + 15, BOX_WIDTH);
	gotoxy(BOX_LEFT + 4, BOX_TOP + 16);
	if (changed)
		cprintf("Unsaved changes");
	else
		cprintf("No unsaved changes");
	gotoxy(BOX_LEFT + 4, BOX_TOP + 17);
	cprintf("1-9 Edit    S Save    X Exit");
}

void edit_value(int index)
{
	char input[CFG_LINE_LEN];
	int row;

	clrscr();
	_setcursortype(_NOCURSOR);
	draw_border(EDIT_LEFT, EDIT_TOP, EDIT_WIDTH, EDIT_HEIGHT);
	gotoxy(EDIT_LEFT + 20, EDIT_TOP + 1);
	cprintf("Edit Configuration");
	gotoxy(EDIT_LEFT + 3, EDIT_TOP + 3);
	cprintf("%s", cfg_keys[index]);
	gotoxy(EDIT_LEFT + 3, EDIT_TOP + 4);
	cprintf("Current:");
	gotoxy(EDIT_LEFT + 12, EDIT_TOP + 4);
	cprintf("%-42s", cfg_values[index]);
	gotoxy(EDIT_LEFT + 3, EDIT_TOP + 6);
	cprintf("New value:");
	gotoxy(EDIT_LEFT + 14, EDIT_TOP + 6);
	for (row = 0; row < EDIT_WIDTH - 17; row++)
		putch(' ');
	gotoxy(EDIT_LEFT + 14, EDIT_TOP + 6);
	_setcursortype(_NORMALCURSOR);
	fgets(input, sizeof(input), stdin);
	_setcursortype(_NOCURSOR);
	strip_newline(input);

	if (input[0] != '\0')
	{
		strcpy(cfg_values[index], input);
		changed = 1;
	}
}

int write_backup()
{
	FILE *infile;
	FILE *outfile;
	char buffer[CFG_LINE_LEN];

	infile = fopen("ZMMGR.CFG", "r");
	if (infile == NULL)
		return 0;

	outfile = fopen("ZMMGR.BAK", "w");
	if (outfile == NULL)
	{
		fclose(infile);
		return 0;
	}

	while (fgets(buffer, sizeof(buffer), infile) != NULL)
		fputs(buffer, outfile);

	fclose(infile);
	fclose(outfile);
	return 1;
}

int save_config()
{
	FILE *file;
	int i;
	int key_index;
	int found[CFG_KEY_COUNT];

	for (i = 0; i < CFG_KEY_COUNT; i++)
		found[i] = 0;

	write_backup();

	file = fopen("ZMMGR.CFG", "w");
	if (file == NULL)
		return 0;

	for (i = 0; i < cfg_line_count; i++)
	{
		int matched = 0;
		for (key_index = 0; key_index < CFG_KEY_COUNT; key_index++)
		{
			if (line_matches_key(cfg_lines[i], cfg_keys[key_index]))
			{
				fprintf(file, "%s = %s\n", cfg_keys[key_index], cfg_values[key_index]);
				found[key_index] = 1;
				matched = 1;
				break;
			}
		}

		if (!matched)
			fputs(cfg_lines[i], file);
	}

	for (key_index = 0; key_index < CFG_KEY_COUNT; key_index++)
	{
		if (!found[key_index])
			fprintf(file, "%s = %s\n", cfg_keys[key_index], cfg_values[key_index]);
	}

	fclose(file);
	load_config();
	return 1;
}

int confirm_exit()
{
	char key;

	if (!changed)
		return 1;

	gotoxy(BOX_LEFT + 4, BOX_TOP + 18);
	cprintf("Save changes before exit? Y/N/C ");

	while (1)
	{
		key = getch();
		if (key == 'y' || key == 'Y')
		{
			save_config();
			return 1;
		}
		if (key == 'n' || key == 'N')
			return 1;
		if (key == 'c' || key == 'C')
			return 0;
	}
}

int main(void)
{
	char key;

	_setcursortype(_NOCURSOR);
	if (!load_config())
	{
		_setcursortype(_NORMALCURSOR);
		cprintf("Unable to load ZMMGR.CFG\r\n");
		cprintf("Press any key to continue\r\n");
		getch();
		return 1;
	}

	while (1)
	{
		draw_config_screen();
		key = getch();

		if (key >= '1' && key <= '9')
			edit_value(key - '1');
		else if (key == 's' || key == 'S')
			save_config();
		else if (key == 'x' || key == 'X')
		{
			if (confirm_exit())
				break;
		}
	}

	_setcursortype(_NORMALCURSOR);
	return 0;
}
