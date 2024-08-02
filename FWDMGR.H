#ifndef FWDMGRCL_H_INCLUDED
#define FWDMGRCL_H_INCLUDED

#include <IOSTREAM.H>
#include "CONFIGCL.H"
#include <CONIO.H>
#include "DEFINES.H"
#include <STDIO.H>
#include <STDLIB.H>
#include "MEMMGR.H"
#include "UTILS.H"

class ForwardFileArray
{
public:
    char **array;
    int count;
    int max_size_of_strings;
};

class ForwardManager
{
private:
    ConfigClass *config;
    MemoryManager *memmgr;
    ForwardFileArray *forwardfilelist;          // .FIL files
    ForwardFileArray *forwardfilecontentsarray; // files inside the .FIL files
    ForwardManager()
    {
        config = ConfigClass::getinstance();
        memmgr = new MemoryManager();
        initialize_arrays();
    }
    ForwardManager(const ForwardManager &other);
    ForwardManager &operator=(const ForwardManager &other);
    char fullfilename[MAX_FILE_SIZE];
    static ForwardManager *instance;

public:
    static ForwardManager *getinstance()
    {
        if (instance == NULL)
            instance = new ForwardManager();

        return instance;
    }
    int checkforforwardfiles(char *sourcefolder);
    int validateforwardfiles(char *forwardfilename, char *filefolder, int usesourcefolder = FALSE);
    int getforwardfilelistcount();
    void clear_arrays();
    void initialize_arrays();
    void cleanupforwardmanager();
    int ismyforwardfile();
    int deleteforwardfiles(char *sourcefolder);
    int copyforwardfiles(char *sourcefolder, char *destinationfolder);
    int moveforwardfiles(char *sourcefolder, char *destinationfolder);
    char *getforwardfilename();
};

#endif