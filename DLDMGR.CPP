#include "DLDMGR.H"

DownloadManager::DownloadManager()
{
    config = ConfigClass::getinstance();
    tranfer = new Transfer();
    forwardmanager = ForwardManager::getinstance();
}

int DownloadManager::checkforforwardfiles(char *sourcefolder)
{
    return forwardmanager->checkforforwardfiles(sourcefolder);
}

static int name_in_multi_list(char *list, const char *computername)
{
    char listcopy[MAX_FILE_SIZE] = {"\0"};
    char *token;

    strcpy(listcopy, list);
    token = strtok(listcopy, ",");
    while (token != NULL)
    {
        while (*token == ' ')
            token++;
        remove_char(token, '\n');
        remove_char(token, '\r');
        to_upper_case(token);
        if (strcmp(token, computername) == 0)
            return TRUE;
        token = strtok(NULL, ",");
    }

    return FALSE;
}

static void remove_name_from_multi_list(char *list, const char *computername, char *output)
{
    char listcopy[MAX_FILE_SIZE] = {"\0"};
    char *token;

    output[0] = '\0';
    strcpy(listcopy, list);
    token = strtok(listcopy, ",");
    while (token != NULL)
    {
        while (*token == ' ')
            token++;
        remove_char(token, '\n');
        remove_char(token, '\r');
        to_upper_case(token);
        if (token[0] != '\0' && strcmp(token, computername) != 0)
        {
            if (output[0] != '\0')
                strcat(output, ",");
            strcat(output, token);
        }
        token = strtok(NULL, ",");
    }
}

static int write_multi_file(char *filename, char *recipients, char *payload)
{
    FILE *file;

    file = fopen(filename, "w");
    if (file == NULL)
        return ERROR;

    fprintf(file, "%s\n", recipients);
    fprintf(file, "%s\n", payload);
    fclose(file);
    return TRUE;
}

int DownloadManager::processmultifile()
{
    FILE *file;
    char multifile[MAX_FILE_SIZE] = {"\0"};
    char recipients[MAX_FILE_SIZE] = {"\0"};
    char remaining[MAX_FILE_SIZE] = {"\0"};
    char payload[MAX_FILE_SIZE] = {"\0"};
    char payload_source[MAX_FILE_SIZE] = {"\0"};
    char payload_keep[MAX_FILE_SIZE] = {"\0"};
    char payload_upload[MAX_FILE_SIZE] = {"\0"};
    char multi_upload[MAX_FILE_SIZE] = {"\0"};
    char computername[MAX_FILENAME_LENGTH] = {"\0"};
    int ismine;

    sprintf(multifile, "%s\\MULTI.FIL", config->getconfigvalue(DOWNLOADFOLDER));
    file = fopen(multifile, "r");
    if (file == NULL)
        return ERROR;

    if (fgets(recipients, sizeof(recipients), file) == NULL ||
        fgets(payload, sizeof(payload), file) == NULL)
    {
        fclose(file);
        return ERROR;
    }
    fclose(file);

    remove_char(recipients, '\n');
    remove_char(recipients, '\r');
    remove_char(payload, '\n');
    remove_char(payload, '\r');
    to_upper_case(recipients);

    strcpy(computername, config->getconfigvalue(COMPUTERNAME));
    to_upper_case(computername);

    sprintf(payload_source, "%s\\%s", config->getconfigvalue(DOWNLOADFOLDER), getshortfilename(payload));
    sprintf(payload_keep, "%s\\%s", config->getconfigvalue(KEEPFOLDER), getshortfilename(payload));
    sprintf(payload_upload, "%s\\%s", config->getconfigvalue(UPLOADFOLDER), getshortfilename(payload));
    sprintf(multi_upload, "%s\\MULTI.FIL", config->getconfigvalue(UPLOADFOLDER));

    if (fileexists(payload_source) == ERROR)
    {
        println("MULTI payload missing: %s", payload_source);
        return ERROR;
    }

    ismine = name_in_multi_list(recipients, computername);
    if (ismine == TRUE)
    {
        println("MULTI package includes %s", computername);
        remove_name_from_multi_list(recipients, computername, remaining);

        if (remaining[0] != '\0')
        {
            if (copyfile(payload_source, payload_keep) == FALSE)
                return ERROR;
            if (movefile(payload_source, payload_upload) == ERROR)
                return ERROR;
            if (write_multi_file(multi_upload, remaining, payload_upload) == ERROR)
                return ERROR;
            println("MULTI package forwarded to %s", remaining);
        }
        else
        {
            if (movefile(payload_source, payload_keep) == ERROR)
                return ERROR;
            println("MULTI package complete");
        }
    }
    else
    {
        if (movefile(payload_source, payload_upload) == ERROR)
            return ERROR;
        if (write_multi_file(multi_upload, recipients, payload_upload) == ERROR)
            return ERROR;
        println("MULTI package forwarded unchanged");
    }

    remove(multifile);
    forwardmanager->clear_arrays();

    if (ismine == TRUE)
        return FALSE;

    return TRUE;
}

int DownloadManager::processforwardfiles()
{
    int returnval = TRUE;
    char forwardfilename[MAX_FILE_SIZE] = {"\0"}; // contains full path to the .FIL found in DL folder
    char downloadfolder[MAX_FILE_SIZE] = {"\0"};
    char keepfolder[MAX_FILE_SIZE] = {"\0"};
    char copyfilename[MAX_FILE_SIZE] = {"\0"};

    sprintf(downloadfolder, "%s", config->getconfigvalue(DOWNLOADFOLDER));
    sprintf(keepfolder, "%s", config->getconfigvalue(KEEPFOLDER));
    sprintf(forwardfilename, "%s\\MULTI.FIL", downloadfolder);

    if (fileexists(forwardfilename) != ERROR)
        return processmultifile();

    sprintf(forwardfilename, "%s\\%s", config->getconfigvalue(DOWNLOADFOLDER), forwardmanager->getforwardfilename());


    /***
     * I need to check for my files first and move them off
     */
    if (forwardmanager->ismyforwardfile() == TRUE)
    {
        sprintf(forwardfilename, "%s\\%s", downloadfolder, forwardmanager->getforwardfilename());
        println("I Got Files!!, Copying %s files to %s", forwardfilename, keepfolder);
        returnval = forwardmanager->validateforwardfiles(forwardfilename, downloadfolder, TRUE);

        if (returnval == ERROR)
        {
            println("Error in validating .FIL file");
            exit(0);
        }

        if(forwardmanager->getforwardfilelistcount() > 1)
            returnval = forwardmanager->copyforwardfiles(downloadfolder, keepfolder);
        else
            returnval = forwardmanager->moveforwardfiles(downloadfolder, keepfolder);
        if (returnval == FALSE)
        {
            println("Error occured while getting my own files");
        }

        remove(forwardfilename);
        forwardmanager->clear_arrays();
        return FALSE;
    }

    /***
     * Now I can handle other .FIL files that have the same values of
     * myfilfile.
     */
    if (returnval == TRUE)
    {
        returnval = forwardmanager->validateforwardfiles(forwardfilename, downloadfolder, TRUE);

        if (returnval == ERROR)
        {
            println("Error in validating .FIL file");
            exit(0);
        }

        if (forwardmanager->moveforwardfiles(downloadfolder, config->getconfigvalue(UPLOADFOLDER)) == ERROR)
        {
            println("An error occured while moving forward files");
            println("Deleting forward files and .FIL file");
            forwardmanager->deleteforwardfiles(downloadfolder);
            returnval = ERROR;
        }

        sprintf(copyfilename, "%s\\%s", config->getconfigvalue(DOWNLOADFOLDER), forwardmanager->getforwardfilename());
        remove(copyfilename);
        sprintf(copyfilename, "%s\\%s", config->getconfigvalue(UPLOADFOLDER), forwardmanager->getforwardfilename());
        to_upper_case(copyfilename);
        listdirectory2file(config->getconfigvalue(UPLOADFOLDER), copyfilename, TRUE, ".FIL");
        appendstring2file(copyfilename, copyfilename);

        forwardmanager->clear_arrays();
    }

    return returnval;
}

int DownloadManager::begindownload(int port)
{
    println("Waiting for Downloads on COM%d - Hit 'ESC' to skip!", port);
    tranfer->download_operations(port);
    return 0;
}
