#include <STRING.H>
#include "CONFIGCL.H"
#include "PROCMAN.H"
#include "UTILS.H"

void prepare_transfer_folders()
{
	ConfigClass *config = ConfigClass::getinstance();

	createdirectory(config->getconfigvalue(DOWNLOADFOLDER));
	createdirectory(config->getconfigvalue(UPLOADFOLDER));
	createdirectory(config->getconfigvalue(KEEPFOLDER));
}

int main(int argc, char *argv[])
{
	char mode[MAX_FILENAME_LENGTH] = {"\0"};
	ProcessManager *procman = new ProcessManager();

	procman->set_local_path(argv[0]);
	prepare_transfer_folders();

	if (argc >= 2)
	{
		strcpy(mode, argv[1]);
		to_upper_case(mode);
	}

	if (strcmp(mode, "FORWARD") == 0)
		procman->create_forward_file_wizard();
	else
		procman->execute_file_transfer_loop();

	return 0;
}
