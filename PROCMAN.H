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

    int pause_or_exit(void);
    int get_port_list(int *ports, int max_ports);
    void execute_once_hooks(void);
    void execute_external_tool(char *toolname);

public:
    void execute_file_transfer_loop(void);
    void create_forward_file_wizard(void);
    void execute_passthrough_mode(void);
    void show_port_diagnostics(void);
    ProcessManager();
    void set_local_path(char *path);
};
#endif
