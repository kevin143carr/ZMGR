#ifndef PROCMAN_H_INCLUDED
#define PROCMAN_H_INCLUDED

#include "DEFINES.H"
#include "DLDMGR.H"
#include "CONFIGCL.H"
#include "EXECPGM.H"
#include "PARSER.H"
#include "SECRETS.H"
#include "UPLDMGR.H"
#include "UTILS.H"


class ProcessManager
{
private:
    ConfigClass* config;
    SecretThings* secretThings;
	ExecPgm* epgm;
    UploadManager* upmanager;
    DownloadManager* downmanager;
    ForwardManager *fwdmanager;
            
    int setbreak;
    int keychr;

public:
    void execute_main_loop(void);
    ProcessManager();
    void set_local_path(char *path);
};
#endif