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
	replaced_word = replaceWord(downloadparams, "{DOWNLOAD}", downloadfolder);
	strcpy(downloadparams, replaced_word);
	free(replaced_word);
	replaced_word = replaceWord(uploadparams, "{UPLOAD}", uploadfolder);
	strcpy(uploadparams, replaced_word);
	free(replaced_word);
}

void ConfigClass::setLocalPath(char *path)
{
	strcpy(localpath, replaceWord(path, "ZMGR.EXE", ""));
}