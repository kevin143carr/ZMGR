#include "FWDMGR.H"

ForwardManager *ForwardManager::instance = 0;

void ForwardManager::clear_arrays()
{
    int i;
    for (i = 0; i < MAX_LINES; i++)
    {
        memset(forwardfilecontentsarray->array[i], 0, MAX_FILENAME_LENGTH);
    }
    forwardfilecontentsarray->count = 0;

    for (i = 0; i < MAX_LINES; i++)
    {
        memset(forwardfilelist->array[i], 0, MAX_FILENAME_LENGTH);
    }
    forwardfilelist->count = 0;
}

void ForwardManager::initialize_arrays()
{
    forwardfilelist = new ForwardFileArray();
    forwardfilelist->count = MAX_LINES;
    forwardfilelist->max_size_of_strings = MAX_FILENAME_LENGTH;
    forwardfilelist->array = memmgr->allocate_string_array(forwardfilelist->count, forwardfilelist->max_size_of_strings);

    forwardfilecontentsarray = new ForwardFileArray();
    forwardfilecontentsarray->count = MAX_LINES;
    forwardfilecontentsarray->max_size_of_strings = MAX_FILENAME_LENGTH;
    forwardfilecontentsarray->array = memmgr->allocate_string_array(forwardfilecontentsarray->count, forwardfilecontentsarray->max_size_of_strings);

    println("Arrays Allocated");

    clear_arrays();
}

void ForwardManager::cleanupforwardmanager()
{
    if (forwardfilecontentsarray->array != NULL)
    {
        memmgr->free_string_array(forwardfilecontentsarray->array, MAX_LINES);
        forwardfilecontentsarray->array = NULL;
    }

    if (forwardfilelist->array != NULL)
    {
        memmgr->free_string_array(forwardfilelist->array, MAX_LINES);
        forwardfilelist->array = NULL;
    }

    println("Cleared Memory, Count = %d", memmgr->get_pointer_count());
}

int ForwardManager::checkforforwardfiles(char *sourcefolder)
{
    int returnval = FALSE;

    int numfiles = count_files_of_type(sourcefolder, "FIL");

    if (numfiles > 0)
    {
        forwardfilelist->count = numfiles;
        numfiles = list_files_of_type(sourcefolder, "FIL", forwardfilelist->array, numfiles, MAX_FILE_SIZE);

        if (numfiles > 0)
        {
            if (forwardfilelist->array[0] == NULL)
            {
                println("Forward file value is null found in [%s]", sourcefolder);
                return ERROR;
            }
            println("Forward file [%s] found in [%s]", forwardfilelist->array[0], sourcefolder);
        }

        if (forwardfilelist->array == NULL)
            returnval = ERROR;

        returnval = TRUE;
    }

    return returnval;
}

/***
 * THIS SHOULD BE CALLED FIRST, after checkforforwardfiles() - initializes the forwardfilecontentsarray.
 * This function compares the .FIL file with the files in the directory
 * to make sure they exists.  If they do, then it will return TRUE
 * otherwise it could return ERROR on unknown filename or FALSE
 * if there are no files or some are missing.
 *
 */
int ForwardManager::validateforwardfiles(char *forwardfilename, char *sourcefilefolder, int usesourcefolder)
{
    int filecheck = 0;
    int linecount = 0;

    linecount = count_lines_in_file(forwardfilename);

    read_lines_from_file(forwardfilename, forwardfilecontentsarray->array, linecount, MAX_FILENAME_LENGTH, FALSE);
    forwardfilecontentsarray->count = linecount;
    for (int i = 0; i < forwardfilecontentsarray->count; i++)
    {
        memset(fullfilename, 0, MAX_FILE_SIZE);
        if (usesourcefolder == TRUE)
            sprintf(fullfilename, "%s\\%s", sourcefilefolder, forwardfilecontentsarray->array[i]);
        else
            sprintf(fullfilename, "%s", forwardfilecontentsarray->array[i]);

        filecheck = fileexists(fullfilename);
        if (filecheck == ERROR)
        {
            println("The [%s] file was not found in DIR [%s]", fullfilename, sourcefilefolder);
            return ERROR;
        }
    }

    return TRUE;
}

int ForwardManager::getforwardfilelistcount()
{
    return forwardfilelist->count;
}

int ForwardManager::ismyforwardfile()
{
    int returnval = FALSE;

    if (forwardfilelist != NULL)
    {

        for (int i = 0; i < forwardfilelist->count; i++)
        {
            memset(fullfilename, 0, MAX_FILE_SIZE);
            sprintf(fullfilename, "%s.FIL", config->getconfigvalue(COMPUTERNAME));
            if (strcmp(fullfilename, forwardfilelist->array[i]) == 0)
            {
                returnval = TRUE;
                // copy over the name to sub-zero since we handle .FIL
                // files one at a time per iteration.
                strcpy(forwardfilelist->array[0], fullfilename);
                break;
            }
        }
    }
    else
        println("forward file list is not populated");

    return returnval;
}

int ForwardManager::deleteforwardfiles(char *sourcefolder)
{
    int returnval = TRUE;

    for (int i = 0; i < forwardfilecontentsarray->count; i++)
    {
        memset(fullfilename, 0, MAX_FILE_SIZE);
        sprintf(fullfilename, "%s\\%s", sourcefolder, getshortfilename(forwardfilecontentsarray->array[i]));
        remove(fullfilename);
    }
    return returnval;
}

int ForwardManager::copyforwardfiles(char *sourcefolder, char *destinationfolder)
{
    char sourcefile[MAX_FILE_SIZE] = {"\0"};
    int returnval = TRUE;

    for (int i = 0; i < forwardfilecontentsarray->count; i++)
    {
        memset(fullfilename, 0, MAX_FILE_SIZE);
        // create ad destination for it.
        sprintf(fullfilename, "%s\\%s", destinationfolder, getshortfilename(forwardfilecontentsarray->array[i]));
        sprintf(sourcefile, "%s\\%s", sourcefolder, getshortfilename(forwardfilecontentsarray->array[i]));

        returnval = copyfile(sourcefile, fullfilename);
        if (returnval == FALSE)
            println("ERROR: Copying file %s to %s", sourcefile, fullfilename);
    }

    return returnval;
}

/***
 * This function takes the files inside the forwardfile.FIL and checks
 * if they are in the sourcefolder and then moves them to the folder
 * that is specified in the forwardfile.FIL file.
 */
int ForwardManager::moveforwardfiles(char *sourcefolder, char *destinationfolder)
{
    char destination[MAX_FILE_SIZE] = {"\0"};
    int returnval = TRUE;

    for (int i = 0; i < forwardfilecontentsarray->count; i++)
    {
        memset(fullfilename, 0, MAX_FILE_SIZE);
        sprintf(fullfilename, "%s\\%s", sourcefolder, getshortfilename(forwardfilecontentsarray->array[i]));
        sprintf(destination, "%s\\%s", destinationfolder, getshortfilename(forwardfilecontentsarray->array[i]));
        movefile(fullfilename, destination);
    }

    return returnval;
}

/***
 * returns the sub-zero [0] of the entire file list
 * since we handle .FIL files on at a time
 */
char *ForwardManager::getforwardfilename()
{
    return forwardfilelist->array[0];
}
