#ifndef TRANSFER_H_INCLUDED
#define TRANSFER_H_INCLUDED
#include "EXECPGM.H"
#include "CONFIGCL.H"

class Transfer
{
private:
    ExecPgm *epgm;
    ConfigClass* config;

public:
    int forwarding;
    Transfer();
    int uploadfiles(char* uploadparams);
    int upload_operations(char* filename);
    int download_operations();
};
#endif