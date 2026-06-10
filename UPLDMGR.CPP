#include "UPLDMGR.H"

static int is_internal_upload_file(char *filename)
{
    char uppername[MAX_FILENAME_LENGTH] = {"\0"};

    strcpy(uppername, filename);
    to_upper_case(uppername);

    if (strcmp(uppername, "ZMSEND.LST") == 0)
        return TRUE;

    return FALSE;
}

static void remove_upload_owned_file(char *uploadfolder, char *filename)
{
    char candidate[MAX_FILE_SIZE] = {"\0"};
    char candidateupper[MAX_FILE_SIZE] = {"\0"};
    char entryupper[MAX_FILE_SIZE] = {"\0"};

    sprintf(candidate, "%s\\%s", uploadfolder, getshortfilename(filename));
    strcpy(candidateupper, candidate);
    strcpy(entryupper, filename);
    to_upper_case(candidateupper);
    to_upper_case(entryupper);

    if (strchr(filename, '\\') == NULL || strcmp(candidateupper, entryupper) == 0)
        remove(candidate);
}

int UploadManager::checkforuploads()
{
    char uploadfolder[MAX_FILE_SIZE] = {"\0"};
    int filecount;
    int allocated_count;
    int uploadcount = 0;
    char **filearray;
    MemoryManager *memmgr;

    strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));
    filecount = count_files_of_type(uploadfolder, "*");
    if (filecount <= 0)
        return FALSE;

    allocated_count = filecount;
    memmgr = new MemoryManager();
    filearray = memmgr->allocate_string_array(allocated_count, MAX_FILE_SIZE);
    filecount = list_files_of_type(uploadfolder, "*", filearray, allocated_count, MAX_FILE_SIZE);
    for (int i = 0; i < filecount; i++)
    {
        if (is_internal_upload_file(filearray[i]) == FALSE)
            uploadcount++;
    }

    memmgr->free_string_array(filearray, allocated_count);
    delete (memmgr);
    return uploadcount;
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

int UploadManager::processMultiForwardFile(char *fullfilename, char *payloadfile, char *multifile)
{
    FILE *infile;
    FILE *sendlist;
    char recipients[MAX_FILE_SIZE] = {"\0"};
    char payload[MAX_FILE_SIZE] = {"\0"};
    char sendlistname[MAX_FILE_SIZE] = {"\0"};

    sprintf(multifile, "%s\\MULTI.FIL", config->getconfigvalue(UPLOADFOLDER));
    infile = fopen(multifile, "r");
    if (infile == NULL)
    {
        println("Could not open MULTI.FIL");
        return ERROR;
    }

    if (fgets(recipients, sizeof(recipients), infile) == NULL ||
        fgets(payload, sizeof(payload), infile) == NULL)
    {
        fclose(infile);
        println("MULTI.FIL is incomplete");
        return ERROR;
    }
    fclose(infile);

    remove_char(payload, '\n');
    remove_char(payload, '\r');
    strcpy(payloadfile, payload);

    if (fileexists(payloadfile) == ERROR)
    {
        println("MULTI payload missing: %s", payloadfile);
        return ERROR;
    }

    sprintf(sendlistname, "%s\\ZMSEND.LST", config->getconfigvalue(UPLOADFOLDER));
    sendlist = fopen(sendlistname, "w");
    if (sendlist == NULL)
    {
        println("Could not create %s", sendlistname);
        return ERROR;
    }

    fprintf(sendlist, "%s\n", payloadfile);
    fprintf(sendlist, "%s\n", multifile);
    fclose(sendlist);

    strcpy(fullfilename, sendlistname);
    return SUCCESS;
}

int UploadManager::processControlFile(char *fullfilename, char *controlfile, char *extension)
{
    FILE *sendlist;
    char uploadfolder[MAX_FILE_SIZE] = {"\0"};
    int filecount;
    int allocated_count;
    char **filearray;
    MemoryManager *memmgr;

    strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));
    filecount = count_files_of_type(uploadfolder, extension);
    if (filecount <= 0)
        return ABORT;

    allocated_count = filecount;
    memmgr = new MemoryManager();
    filearray = memmgr->allocate_string_array(allocated_count, MAX_FILE_SIZE);
    filecount = list_files_of_type(uploadfolder, extension, filearray, allocated_count, MAX_FILE_SIZE);
    if (filecount <= 0)
    {
        memmgr->free_string_array(filearray, allocated_count);
        delete (memmgr);
        return ABORT;
    }

    sprintf(controlfile, "%s\\%s", uploadfolder, filearray[0]);
    to_upper_case(controlfile);
    memmgr->free_string_array(filearray, allocated_count);
    delete (memmgr);

    sprintf(fullfilename, "%s\\ZMSEND.LST", uploadfolder);
    sendlist = fopen(fullfilename, "w");
    if (sendlist == NULL)
    {
        println("Could not create %s", fullfilename);
        return ERROR;
    }

    fprintf(sendlist, "%s\n", controlfile);
    fclose(sendlist);
    return SUCCESS;
}

int UploadManager::beginuploads(int port)
{
    int returnval = TRUE;
    char uploadfolder[MAX_FILE_SIZE] = {"\0"};
    char forwardfilename[MAX_FILE_SIZE] = {"\0"};
    char payloadfile[MAX_FILE_SIZE] = {"\0"};
    char multifile[MAX_FILE_SIZE] = {"\0"};
    char controlfile[MAX_FILE_SIZE] = {"\0"};
    strcpy(uploadfolder, config->getconfigvalue(UPLOADFOLDER));
    sprintf(multifile, "%s\\MULTI.FIL", uploadfolder);

    if (fileexists(multifile) != ERROR)
    {
        if (processMultiForwardFile(forwardfilename, payloadfile, multifile) == SUCCESS)
        {
            returnval = tranfer->upload_operations(forwardfilename, port);

            if (returnval == SUCCESS)
            {
                remove_upload_owned_file(uploadfolder, payloadfile);
                remove(multifile);
                remove(forwardfilename);
            }
        }
        else
            returnval = ERROR;
    }
    else if (processControlFile(forwardfilename, controlfile, "XFL") == SUCCESS)
    {
        returnval = tranfer->upload_operations(forwardfilename, port);

        remove(forwardfilename);
        if (returnval == SUCCESS)
            remove(controlfile);
    }
    else if (processControlFile(forwardfilename, controlfile, "RDY") == SUCCESS)
    {
        returnval = tranfer->upload_operations(forwardfilename, port);

        remove(forwardfilename);
        if (returnval == SUCCESS)
            remove(controlfile);
    }
    else if (forwardmanager->checkforforwardfiles(uploadfolder) == TRUE)
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
