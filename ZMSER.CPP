#include <CONIO.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include "DOSSER.H"
#include "SERIAL.H"
#include "UTILS.H"

int main(void)
{
	SerialManager serialmanager;
	int port_a = 0;
	int port_b = 0;
	int watch_zmodem = TRUE;
	int bridge_mode = BRIDGE_MODE_ZMODEM;
	long bridge_baud = 19200L;
	char input[10] = {"\0"};

	clrscr();
	println("ZMGR Serial Pass-through");
	println("");
	println("Enter first COM port number:");
	fgets(input, sizeof(input), stdin);
	port_a = atoi(input);

	memset(input, 0, sizeof(input));
	println("Enter second COM port number:");
	fgets(input, sizeof(input), stdin);
	port_b = atoi(input);

	memset(input, 0, sizeof(input));
	println("");
	println("Bridge mode:");
	println("1. ZModem optimized bridge");
	println("2. Simple null-modem bridge");
	println("Select mode [1]:");
	fgets(input, sizeof(input), stdin);
	if (atoi(input) == 2)
		bridge_mode = BRIDGE_MODE_SIMPLE;

	memset(input, 0, sizeof(input));
	println("");
	println("Baud rate:");
	println("1. 1200");
	println("2. 2400");
	println("3. 4800");
	println("4. 9600");
	println("5. 19200");
	println("Select baud [5]:");
	fgets(input, sizeof(input), stdin);
	switch (atoi(input))
	{
		case 1:
			bridge_baud = 1200L;
			break;
		case 2:
			bridge_baud = 2400L;
			break;
		case 3:
			bridge_baud = 4800L;
			break;
		case 4:
			bridge_baud = 9600L;
			break;
		default:
			bridge_baud = 19200L;
			break;
	}

	serialmanager.passthrough(port_a, port_b, watch_zmodem, bridge_mode, bridge_baud);

	println("");
	println("Returning to ZMGR");
	return 0;
}
