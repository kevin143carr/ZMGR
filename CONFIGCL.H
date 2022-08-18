#ifndef CONFIGCL_H_INCLUDED
#define CONFIGCL_H_INCLUDED

#define DOWNLOAD "DOWNLOAD"
#define UPLOAD "UPLOAD"

class ConfigClass
{
private:
	char pausetimestr[4];

public:
	char path[100];
	char localpath[100];
	char onceexec[100];
	char downloadparams[100];
	char uploadparams[100];
	char startupstate[20];
	char oncedoscommand[100];
	char downloadfolder[100];
	char uploadfolder[100];
	int pausetime;
	char zmgrsecret[10];

	ConfigClass();
	void updateConfig();
	void setLocalPath(char *path);
};
#endif
