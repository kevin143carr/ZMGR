#ifndef CONFIGCL_H_INCLUDED
#define CONFIGCL_H_INCLUDED

#include <IOSTREAM.H>
#include "DEFINES.H"

enum Category
{
	EXECPATH,
	COMPUTERNAME,
	FORWARDTO,
	ONCEEXEC,
	DOWNLOADPARAMS,
	UPLOADPARAMS,
	FORWARDPARAMS,
	STARTUPSTATE,
	PAUSETIME,
	ZMGRSECRET,
	ONCEDOSCOMMAND,
	DOWNLOADFOLDER,
	UPLOADFOLDER,
	KEEPFOLDER,
	NOTFOUND
};

class ConfigClass
{
private:
	char configitem[MAX_CONFIG_SIZE];
	struct CategoryString
	{
		Category category;
		const char *name;
	};
	ConfigClass() {};

	static const CategoryString categoryStrings[];
	static ConfigClass *instance;

	const char *getCategoryName(Category category);
	Category getCategoryFromName(const char *name);
	char localpath[MAX_FILE_SIZE];

public:
	static ConfigClass *getinstance()
	{
		if (instance == NULL)
			instance = new ConfigClass();

		return instance;
	}

	char *getconfigvalue(Category category);
	void set_local_path(char *path);
	char *get_local_path();
};
#endif
