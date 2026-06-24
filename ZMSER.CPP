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
	int handshaking = SER_HANDSHAKING_RTSCTS;
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
	println("Handshaking mode:");
	println("1. RTS/CTS hardware flow control");
	println("2. None - direct null modem/game mode");
	println("Select mode [1]:");
	fgets(input, sizeof(input), stdin);
	if (atoi(input) == 2)
		handshaking = SER_HANDSHAKING_NONE;

	serialmanager.passthrough(port_a, port_b, watch_zmodem, handshaking);

	println("");
	println("Returning to ZMGR");
	return 0;
}
