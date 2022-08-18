#include <CONIO.H>
#include <DOS.H>
#include <STDIO.H>
#include "PARSER.H"
#include "SECRETS.H"
#include "UTILS.H"
#include "CONFIGCL.H"
#include "TRANSFER.H"
#include "EXECPGM.H"

int main(int argc, char *argv[])
{
	SecretThings *secretThings;
	int keychr;
	ConfigClass *config;
	Transfer *tranfer;
	ExecPgm *epgm;
	int setbreak = 0;

	secretThings = new SecretThings();
	config = new ConfigClass();
	tranfer = new Transfer();
	epgm = new ExecPgm();

	if (argc > 0)
		config->setLocalPath(argv[0]);

	clrscr();
	printf("[Public Domain Zmodem] [Manager]\n");
	printf(" Public Domain Zmodem by Peter Mandrella, 1994-1998\n");
	printf(" Manager by Kevin Carr, 2022\n\n");
	printf(" This is a MS-DOS program\n");
	printf(" All actions handled by the ZM.CFG file\n\n");
	delay(3000);

	while (1)
	{
		/***
		 * This is for the PDZM program
		 */
		if (strcmp(config->startupstate, DOWNLOAD) == 0)
		{
			printf("Entering Download Processing\n");
			tranfer->DownloadOperations(config);
			printf("Exiting Download Processing\n");
		}
		else // We are UPLOADING
		{
			tranfer->UploadOperations(config);
		}

		if ((config->onceexec[0] != '\0'))
		{
			printf("Executing onceexec [%s] with [%s]\n", config->onceexec, config->downloadparams);
			epgm->execprogram(config->onceexec, config->downloadparams);
		}

		if (config->oncedoscommand[0] != '\0')
		{
			if (epgm->execdoscommand(config->oncedoscommand) == -1)
				perror("Error Executing DOS Command\n");
			remove_config_file_value("ZM.CFG", "ONCEDOSCOMMAND");
		}

		if (config->zmgrsecret != '\0')
		{
			secretThings->dosecretthings(config->zmgrsecret);
		}

		//		MEMORY DEBUG STUFF:
		//		if (epgm->execdoscommand("mem.exe /f") == -1)
		//			perror("Error Executing DOS Command\n");

		printf("PAUSING %d SECONDS\n", config->pausetime);
		printf("'X' WILL END THIS MADNESS\n");
		setbreak = 0;
		for (int i = 0; i < (config->pausetime * 2); i++)
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
			}

			if (setbreak == 1)
				break;
		}

		if (setbreak == 1)
			break;
	}

	return 0;
}
