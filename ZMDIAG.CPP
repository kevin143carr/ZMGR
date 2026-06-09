#include <CONIO.H>
#include <STDIO.H>
#include <STRING.H>
#include "APPINFO.H"
#include "SERIAL.H"

#define MENU_WIDTH 40
#define MENU_LEFT 20
#define MENU_TOP 5

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

void draw_menu(void)
{
	clrscr();
	draw_border(MENU_LEFT, MENU_TOP, MENU_WIDTH, 12);
	print_centered(MENU_TOP + 1, "ZMGR Diagnostics");
	print_centered(MENU_TOP + 2, ZMGR_BUILD);
	draw_separator(MENU_LEFT, MENU_TOP + 3, MENU_WIDTH);

	gotoxy(MENU_LEFT + 4, MENU_TOP + 5);
	cprintf("1. COM Port Information");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 6);
	cprintf("2. Who Is There");
	gotoxy(MENU_LEFT + 4, MENU_TOP + 7);
	cprintf("X. Exit");

	draw_separator(MENU_LEFT, MENU_TOP + 9, MENU_WIDTH);
	gotoxy(MENU_LEFT + 4, MENU_TOP + 10);
	cprintf("Select an option");
}

int main(void)
{
	SerialManager serialmanager;
	char key;

	while (1)
	{
		draw_menu();

		key = getch();
		if (key == '1')
			serialmanager.show_diagnostics();
		else if (key == '2')
			serialmanager.run_who_is_there();
		else if (key == 'x' || key == 'X')
			break;
	}

	return 0;
}
