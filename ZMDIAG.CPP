#include <CONIO.H>
#include <STDIO.H>
#include "SERIAL.H"

int main(void)
{
	SerialManager serialmanager;
	char key;

	while (1)
	{
		clrscr();
		cprintf("ZMGR Diagnostics\r\n");
		cprintf("\r\n");
		cprintf("1. COM Port Information\r\n");
		cprintf("2. Who Is There\r\n");
		cprintf("X. Exit\r\n");
		cprintf("\r\n");

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
