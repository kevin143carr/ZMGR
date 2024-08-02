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
char *UploadManager::processForwardFiles()
{
    char *myfwdfilename = NULL;
    char fullfilename[MAX_FILE_SIZE] = {"\0"};

    myfwdfilename = forwardmanager->getforwardfilename();
    sprintf(fullfilename, "%s\\%s", config->getconfigvalue(UPLOADFOLDER), myfwdfilename);
    to_upper_case(fullfilename);

    if (forwardmanager->validateforwardfiles(fullfilename, config->getconfigvalue(UPLOADFOLDER), TRUE) == ERROR)
    {
        println("Problem happened in processing forward files, deleting files!");
        forwardmanager->deleteforwardfiles(config->getconfigvalue(UPLOADFOLDER));
        remove(fullfilename);
    }

    listdirectory2file(config->getconfigvalue(UPLOADFOLDER), fullfilename, TRUE, ".FIL");
    appendstring2file(fullfilename, fullfilename);

    return fullfilename;
}

int UploadManager::beginuploads()
{
    int returnval = TRUE;
    char uploadfolder[MAX_FILE_SIZE] = {"\0"};
    char forwardfilename[MAX_FILE_SIZE] = {"\0"};
    strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));

    if (forwardmanager->checkforforwardfiles(uploadfolder) == TRUE)
    {
        strcpy(forwardfilename, processForwardFiles());

        if (forwardfilename != NULL)
        {
            returnval = tranfer->upload_operations(forwardfilename);

            if (returnval == SUCCESS)
            {
                int morefiles = FALSE;

                println("Removing files from the %s folder", config->getconfigvalue(UPLOADFOLDER));
                println("Based on .FIL %s file", forwardfilename);
                remove(forwardfilename);
                morefiles = filesexists_in_dir(config->getconfigvalue(UPLOADFOLDER), "*.FIL");
                if(morefiles == FALSE)
                    forwardmanager->deleteforwardfiles(config->getconfigvalue(UPLOADFOLDER));
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
        returnval = tranfer->upload_operations(NULL);

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
