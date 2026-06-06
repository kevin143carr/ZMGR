#include "UPLDMGR.H"

int UploadManager::checkforuploads()
{
    int returnval = TRUE;
    returnval = filesexists_in_dir(config->getconfigvalue(UPLOADFOLDER), "*.*");
    return returnval;
}

/***
 * This functions checks for .FIL files and return the name of the first one.
 * Other .FIL files will be picked up on subsequent loop actions.
 * Anyway, it checks to see if all the files in the .FIL are located in the upload
 * folder.  If not it returns a NULL.
 */
int UploadManager::processForwardFiles(char *fullfilename)
{
    char *myfwdfilename = NULL;

    myfwdfilename = forwardmanager->getforwardfilename();
    sprintf(fullfilename, "%s\\%s", config->getconfigvalue(UPLOADFOLDER), myfwdfilename);
    to_upper_case(fullfilename);

    if (forwardmanager->validateforwardfiles(fullfilename, config->getconfigvalue(UPLOADFOLDER), FALSE) == ERROR)
    {
        println("Problem happened in processing forward file!");
        remove(fullfilename);
        return ERROR;
    }

    return SUCCESS;
}

int UploadManager::beginuploads(int port)
{
    int returnval = TRUE;
    char uploadfolder[MAX_FILE_SIZE] = {"\0"};
    char forwardfilename[MAX_FILE_SIZE] = {"\0"};
    strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));

    if (forwardmanager->checkforforwardfiles(uploadfolder) == TRUE)
    {
        if (processForwardFiles(forwardfilename) == SUCCESS)
        {
            returnval = tranfer->upload_operations(forwardfilename, port);

            if (returnval == SUCCESS)
            {
                println("Removing .FIL file from the %s folder", config->getconfigvalue(UPLOADFOLDER));
                println("Based on .FIL %s file", forwardfilename);
                forwardmanager->deleteforwardfilesfromfolder(config->getconfigvalue(UPLOADFOLDER));
                remove(forwardfilename);
            }
            forwardmanager->clear_arrays();
        }
        else
            forwardmanager->clear_arrays();
    }
    else
    {
        char uploadfolder[MAX_FILE_SIZE] = {"\0"};
        char removefilename[MAX_FILE_SIZE] = {"\0"};
        MemoryManager *memmgr = new MemoryManager();
        strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));
        int uploadfilecount = count_files_of_type(uploadfolder, "*");
        char **filearray = memmgr->allocate_string_array(uploadfilecount, MAX_FILE_SIZE);
        uploadfilecount = list_files_of_type(uploadfolder, "*", filearray, uploadfilecount, MAX_FILE_SIZE);
        returnval = tranfer->upload_operations(NULL, port);

        if (returnval == SUCCESS)
        {
            for (int i = 0; i < uploadfilecount; i++)
            {
                sprintf(removefilename, "%s\\%s", uploadfolder, filearray[i]);
                remove(removefilename);
            }
        }
        memmgr->free_string_array(filearray, uploadfilecount);
        delete (memmgr);
    }

    return returnval;
}

UploadManager::UploadManager()
{
    tranfer = new Transfer();
    forwardmanager = ForwardManager::getinstance();
}

void UploadManager::setupconfig(ConfigClass *config)
{
    UploadManager::config = config;
}
