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

int DownloadManager::processforwardfiles()
{
    int returnval = TRUE;
    char forwardfilename[MAX_FILE_SIZE] = {"\0"}; // contains full path to the .FIL found in DL folder
    char downloadfolder[MAX_FILE_SIZE] = {"\0"};
    char keepfolder[MAX_FILE_SIZE] = {"\0"};
    char copyfilename[MAX_FILE_SIZE] = {"\0"};

    sprintf(forwardfilename, "%s\\%s", config->getconfigvalue(DOWNLOADFOLDER), forwardmanager->getforwardfilename());
    sprintf(downloadfolder, "%s", config->getconfigvalue(DOWNLOADFOLDER));
    sprintf(keepfolder, "%s", config->getconfigvalue(KEEPFOLDER));

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

int DownloadManager::begindownload()
{
    println("Waiting for Downloads - Hit 'ESC' to skip!");
    tranfer->download_operations();
    return 0;
}
