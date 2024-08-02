#include <CONIO.H>
#include "CONFIGCL.H"
#include "UTILS.H"
#include "PARSER.H"

ConfigClass *ConfigClass::instance = 0;

// Define the array to hold the mapping of enums to strings
const ConfigClass::CategoryString ConfigClass::categoryStrings[] = {
    { COMPUTERNAME, "COMPUTERNAME" },
    { FORWARDTO, "FORWARDTO" },
    { STARTUPSTATE, "STARTUPSTATE" },
    { PAUSETIME, "PAUSETIME" },
    { DOWNLOADFOLDER, "DOWNLOADFOLDER" },
    { UPLOADFOLDER, "UPLOADFOLDER" },
    { KEEPFOLDER, "KEEPFOLDER" },
    { EXECPATH, "EXECPATH" },
    { DOWNLOADPARAMS, "DOWNLOADPARAMS" },
    { UPLOADPARAMS, "UPLOADPARAMS" },
    { FORWARDPARAMS, "FORWARDPARAMS" },
    { ONCEEXEC, "ONCEEXEC" },
    { ONCEDOSCOMMAND, "ONCEDOSCOMMAND" },
    { ZMGRSECRET, "ZMGRSECRET" }
};

const char *ConfigClass::getCategoryName(Category category)
{
    for (int i = 0; i < sizeof(categoryStrings) / sizeof(CategoryString); ++i) {
        if (categoryStrings[i].category == category) {
            return categoryStrings[i].name;
        }
    }
    return "Unknown";
}

Category ConfigClass::getCategoryFromName(const char *name)
{
    for (int i = 0; i < sizeof(categoryStrings) / sizeof(CategoryString); ++i) {
        if (strcmp(categoryStrings[i].name, name) == 0) {
            return categoryStrings[i].category;
        }
    }
    return NOTFOUND;
}

char *ConfigClass::getconfigvalue(Category category)
{
    memset(configitem, 0, MAX_CONFIG_SIZE);
	get_config_file_value("ZMMGR.CFG", getCategoryName(category), configitem);
    remove_char(configitem,'\n');
    return configitem;
}

void ConfigClass::set_local_path(char *path)
{
	replacesubstring(path, "ZMGR.EXE", "");
	strcpy(localpath, path);
}

char *ConfigClass::get_local_path()
{
    return localpath;
}
