#ifndef TRANSFER_H_INCLUDED
#define TRANSFER_H_INCLUDED
#include "EXECPGM.H"
#include "CONFIGCL.H"

class Transfer
{
private:
    ExecPgm *epgm;

public:
    Transfer();
    void UploadOperations(ConfigClass *config);
    void DownloadOperations(ConfigClass *config);
};
#endif