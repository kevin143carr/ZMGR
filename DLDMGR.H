#ifndef DLDMGR_H_INCLUDED
#define DLDMGR_H_INCLUDED
#include "DEFINES.H"
#include "CONFIGCL.H"
#include "TRANSFER.H"
#include "UTILS.H"
#include "FWDMGR.H"

class DownloadManager
{
private:
    ConfigClass *config;
    ForwardManager* forwardmanager;
	Transfer *tranfer;

public:
    DownloadManager();
    int checkforforwardfiles(char *sourcefolder);
    int processforwardfiles();
    int begindownload();
};
#endif