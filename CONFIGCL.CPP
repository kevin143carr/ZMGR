#include <CONIO.H>
#include "CONFIGCL.H"
#include "UTILS.H"
#include "PARSER.H"

ConfigClass::ConfigClass()
{
	updateConfig();
}

void ConfigClass::updateConfig()
{
	char *replaced_word;

	get_config_file_value("ZM.CFG", "EXECPATH", path);
	get_config_file_value("ZM.CFG", "ONCEEXEC", onceexec);
	get_config_file_value("ZM.CFG", "DOWNLOADPARAMS", downloadparams);
	get_config_file_value("ZM.CFG", "UPLOADPARAMS", uploadparams);
	get_config_file_value("ZM.CFG", "STARTUPSTATE", startupstate);
	get_config_file_value("ZM.CFG", "PAUSETIME", pausetimestr);
	get_config_file_value("ZM.CFG", "ZMGRSECRET", zmgrsecret);
	get_config_file_value("ZM.CFG", "ONCEDOSCOMMAND", oncedoscommand);
	get_config_file_value("ZM.CFG", "DOWNLOADFOLDER", downloadfolder);
	get_config_file_value("ZM.CFG", "UPLOADFOLDER", uploadfolder);

	removeChar(path, '\n');
	removeChar(onceexec, '\n');
	removeChar(downloadparams, '\n');
	removeChar(uploadparams, '\n');
	removeChar(startupstate, '\n');
	removeChar(pausetimestr, '\n');
	removeChar(zmgrsecret, '\n');
	removeChar(oncedoscommand, '\n');
	removeChar(downloadfolder, '\n');
	removeChar(uploadfolder, '\n');

	pausetime = atoi(pausetimestr);
	replaceSubstring(downloadparams, "{DOWNLOAD}", downloadfolder);
	replaceSubstring(uploadparams, "{UPLOAD}", uploadfolder);
}

void ConfigClass::setLocalPath(char *path)
{
	replaceSubstring(path, "ZMGR.EXE", "");
	strcpy(localpath, path);
}