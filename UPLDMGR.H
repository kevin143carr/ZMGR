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
    int processForwardFiles(char *fullfilename);
    int processMultiForwardFile(char *fullfilename, char *payloadfile, char *multifile);
    int processControlFile(char *fullfilename, char *controlfile, char *extension);
    int beginuploads(int port);
};

#endif
