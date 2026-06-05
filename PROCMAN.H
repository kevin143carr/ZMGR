#ifndef PROCMAN_H_INCLUDED
#define PROCMAN_H_INCLUDED

#include "DEFINES.H"
#include "DLDMGR.H"
#include "CONFIGCL.H"
#include "EXECPGM.H"
#include "PARSER.H"
#include "SECRETS.H"
#include "SERIAL.H"
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
    SerialManager *serialmanager;
            
    int setbreak;
    int keychr;

    int pause_or_exit(void);

public:
    void execute_main_loop(void);
    void execute_auto_download_loop(void);
    void execute_passthrough_mode(void);
    void show_port_diagnostics(void);
    ProcessManager();
    void set_local_path(char *path);
};
#endif
