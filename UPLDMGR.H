#ifndef UPLDMGR_H_INCLUDED
#define UPLDMGR_H_INCLUDED
#include "CONFIGCL.H"
#include "FWDMGR.H"
#include "TRANSFER.H"
#include "UTILS.H"

class UploadManager
{
private:
    ConfigClass *config;
	Transfer *tranfer;
    ForwardManager *forwardmanager;

public:
    UploadManager();
    void setupconfig(ConfigClass *config);
    int checkforuploads();
    char* processForwardFiles();
    int beginuploads();
};

#endif