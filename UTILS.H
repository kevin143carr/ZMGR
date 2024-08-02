#if !defined(__UTILS_H)
#define __UTILS_H

#include <CONIO.H>
#include <CTYPE.H>
#include <STDARG.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <DIR.H>
#include <DOS.H>
#include <SYS/STAT.H>
#include <IOSTREAM.H>
#include <ALLOC.H>
#include <IO.H>
#include "DEFINES.H"

#include <iostream.h>
#include <string.h>
#include <FSTREAM.H>

/***
 * *************** STRING OPERATIONS *******************
 */

const int MAX_SIZE = 100; // Max number of substrings

static void println(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // Print the formatted string
    vprintf(format, args);

    // Print a newline character
    printf("\n");

    va_end(args);
}

static void to_upper_case(char *str)
{
    for (int i = 0; i < strlen(str); ++i)
    {
        str[i] = toupper(str[i]);
    }
}

static int splitstring(const char *input, char delimiter, char *result[])
{
    char *token;
    int i = 0;

    token = strtok((char *)input, &delimiter); // Initial tokenization

    while (token != NULL && i < MAX_SIZE - 1)
    {
        result[i++] = token;              // Store token in result array
        token = strtok(NULL, &delimiter); // Get next token
    }

    result[i] = NULL; // Null-terminate the array of strings

    return i;
}

static void remove_char(char *str, char charToRemmove)
{
    int i, j;
    int len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (str[i] == charToRemmove)
        {
            for (j = i; j < len; j++)
            {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
}

static void replacesubstring(char *string, const char *sub, const char *new_str)
{
    int stringLen, subLen, newLen;
    int i = 0, j, k;
    int flag = 0, start, end;
    stringLen = strlen(string);
    subLen = strlen(sub);
    newLen = strlen(new_str);

    for (i = 0; i < stringLen; i++)
    {
        flag = 0;
        start = i;
        for (j = 0; string[i] == sub[j]; j++, i++)
            if (j == subLen - 1)
                flag = 1;
        end = i;
        if (flag == 0)
            i -= j;
        else
        {
            for (j = start; j < end; j++)
            {
                for (k = start; k < stringLen; k++)
                    string[k] = string[k + 1];
                stringLen--;
                i--;
            }

            for (j = start; j < start + newLen; j++)
            {
                for (k = stringLen; k >= j; k--)
                    string[k + 1] = string[k];
                string[j] = new_str[j - start];
                stringLen++;
                i++;
            }
        }
    }
}

/***
 * ********** FILE OPERATIONS *****************
 */

// fileExists
/** EXAMPLE
    const char* filename = "example.txt";

    if (fileExists(filename)) {
        std::cout << "File exists!" << std::endl;
    } else {
        std::cout << "File does not exist!" << std::endl;
    }
*/
static int fileexists(const char *filename)
{
    // Check if the file exists
    if (access(filename, 0) == 0)
    {
        return 1; // File exists
    }
    else
    {
        return -1; // File does not exist
    }
}

/***
 *  const char* directory = "C:\\path\\to\\directory";
    const char* fileType = "*.txt";  // Change this to the file type you're looking for

    if (fileExistsInDirectory(directory, fileType)) {
        cout << "File or file type exists in the directory." << endl;
    } else {
        cout << "File or file type does not exist in the directory." << endl;
    }
 */
static int filesexists_in_dir(const char *directory, const char *fileType)
{
    struct ffblk fileInfo;
    int done;

    // Create a search pattern
    char searchPattern[100];
    strcpy(searchPattern, directory);
    strcat(searchPattern, "\\");
    strcat(searchPattern, fileType);

    done = findfirst(searchPattern, &fileInfo, 0);
    if (done == 0)
    {
        // If a file is found, return true
        return TRUE;
    }

    // If no file is found, return false
    return FALSE;
}

/***
 * count_files_of_type returns number of files of type from a diectory.
 * Example:
 *  // Parameters for counting files
    const char* directory = "."; // Change to the desired directory
    const char* file_extension = "txt"; // Change to the desired file extension

    // Count files of the specified type
    int file_count = count_files_of_type(directory, file_extension);
 */
static int count_files_of_type(const char *directory, const char *file_extension)
{
    struct ffblk fileinfo;
    char search_path[256];

    // Create the search path
    sprintf(search_path, "%s\\*.%s", directory, file_extension);

    int file_count = 0;
    int done = findfirst(search_path, &fileinfo, 0);
    while (!done)
    {
        file_count++;
        done = findnext(&fileinfo);
    }

    return file_count;
}

static int list_files_of_type(const char *directory, const char *file_extension, char **files_array, size_t max_files, size_t max_filename_length)
{
    struct ffblk fileinfo;
    char search_path[256];

    // Create the search path
    sprintf(search_path, "%s\\*.%s", directory, file_extension);

    int file_count = 0;
    int done = findfirst(search_path, &fileinfo, 0);
    while (!done && file_count < max_files)
    {
        strncpy(files_array[file_count], fileinfo.ff_name, max_filename_length - 1);
        files_array[file_count][max_filename_length - 1] = '\0'; // Ensure null-termination
        file_count++;
        done = findnext(&fileinfo);
    }

    return file_count;
}

/***
 * create directory
 * Example:
 * const char* directoryPath = "test_directory/subdirectory";
    createDirectory(directoryPath);
 */
static void createdirectory(const char *path)
{
    // Check if the directory already exists
    if (access(path, 0) == 0)
    {
        return;
    }

    // Create the directory recursively
    int status = mkdir(path);
    if (status == 0)
    {
    }
    else
    {
        println("Unable to create %s directory", path);
    }
}

/***
 * copyfile
 * Example:
 *  const char* sourcePath = "source_directory/source_file.txt";
    const char* destinationPath = "destination_directory/destination_file.txt";

    if (copyFile(sourcePath, destinationPath)) {
        cout << "File copied successfully!" << endl;
    } else {
        cout << "File copy failed!" << endl;
    }

    returns FALSE if there is a problem
 */
static int copyfile(const char *source, const char *destination)
{
    FILE *srcFile, *destFile;
    char buffer[1024];
    size_t bytesRead;

    // Open the source file in binary read mode
    srcFile = fopen(source, "rb");
    if (srcFile == NULL)
    {
        println("Copy File Error: [%s] Source file not found.", source);
        return FALSE;
    }

    // Open the destination file in binary write mode
    destFile = fopen(destination, "wb");
    if (destFile == NULL)
    {
        println("Copy File Error: Unable to create [%s] destination file.", destination);
        fclose(srcFile);
        return FALSE;
    }

    // Copy the contents from source to destination
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0)
    {
        fwrite(buffer, 1, bytesRead, destFile);
    }

    // Close both files
    fclose(srcFile);
    fclose(destFile);

    return TRUE;
}

/***
 * EXAMPLE OF MoveFile
 * const char* sourcePath = "C:\\source\\file.txt";
    const char* destPath = "C:\\destination\\file.txt";

    if (MoveFile(sourcePath, destPath)) {
        cout << "File moved successfully." << endl;
    } else {
        cout << "File move failed." << endl;
    }

 */
static int movefile(const char *source, const char *destination)
{
    // Try to copy the file first
    FILE *sourceFile, *destFile;
    char buffer[512];
    size_t bytesRead;

    sourceFile = fopen(source, "rb");
    if (sourceFile == NULL)
    {
        println("Move File Error: [%s] Source file not found.", source);
        return -1;
    }

    destFile = fopen(destination, "wb");
    if (destFile == NULL)
    {
        println("Move File Error: Unable to create destination file.");
        fclose(sourceFile);
        return -1;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0)
    {
        fwrite(buffer, 1, bytesRead, destFile);
    }

    fclose(sourceFile);
    fclose(destFile);

    // Now delete the source file
    if (remove(source) != 0)
    {
        println("Error: Unable to delete the source file.");
        return -1;
    }

    return 0;
}

/***
 * getFileName - returns a filename from a full filepath
 *
 * Example:
 *  const char* fullPath = "C:\\path\\to\\your\\file.txt";
    const char* fileName = getFileName(fullPath);

    printf("Full path: %s\n", fullPath);
    printf("File name: %s\n", fileName);
 */

static const char *getshortfilename(const char *fullPath)
{
    // Find the last occurrence of the backslash character
    const char *lastBackslash = strrchr(fullPath, '\\');

    if (lastBackslash)
    {
        // Return the substring following the last backslash
        return lastBackslash + 1;
    }
    else
    {
        // If no backslash is found, the input is already a file name
        return fullPath;
    }
}

static int read_lines_from_file(const char *filename, char **lines_array, size_t max_lines, size_t max_length, int usefullpath)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        println("Could not open file %s", filename);
        return -1; // Indicate an error opening the file
    }

    size_t line_count = 0;
    while (line_count < max_lines && fgets(lines_array[line_count], max_length, file) != NULL)
    {
        // This equates to just the filename without the path
        if (usefullpath == FALSE)
            strcpy(lines_array[line_count], getshortfilename(lines_array[line_count]));

        // Remove newline character if present
        size_t len = strlen(lines_array[line_count]);
        if (len > 0 && lines_array[line_count][len - 1] == '\n')
        {
            lines_array[line_count][len - 1] = '\0';
        }
        line_count++;
    }

    fclose(file);
    return line_count;
}

static int count_lines_in_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        println("Could not open file %s", filename);
        return -1; // Indicate an error opening the file
    }

    int line_count = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        line_count++;
    }

    fclose(file);
    return line_count;
}

/*
Removes a line from a file
Example:
    const char* filename = "example.txt";
    const char* lineToRemove = "This is the line to remove.";

    removeLineFromFile(filename, lineToRemove);
*/
static void removelinefromfile(const char *filename, const char *lineToRemove)
{
    FILE *infile = fopen(filename, "r");     // Open the input file for reading
    FILE *tempFile = fopen("temp.txt", "w"); // Open a temporary file for writing

    if (!infile || !tempFile)
    {
        println("Error opening files!");
        return;
    }

    char buffer[256]; // Buffer to hold lines from the file

    while (fgets(buffer, sizeof(buffer), infile))
    {
        // Remove the newline character at the end of the line if it exists
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }

        // If the current line does not match the line to remove, write it to the temporary file
        if (strcmp(buffer, lineToRemove) != 0)
        {
            fputs(buffer, tempFile);
            fputs("\n", tempFile); // Add the newline character back
        }
    }

    fclose(infile);   // Close the input file
    fclose(tempFile); // Close the temporary file

    // Remove the original file and rename the temporary file to the original filename
    remove(filename);
    rename("temp.txt", filename);
}

/*** listDirectoryToFile list a folder and write its contents
 * into a text file to include full path
 *
 * Example:
 *  const char* directory = "C:\\example";
    const char* outputFile = "output.txt";
    int usefullpath = TRUE/FALSE (1/0)

    listDirectory(directory, outputFile, FALSE);
 */
static void listdirectory2file(const char *directory, const char *outputFile, int usefullpath, const char *excepttype)
{
    FILE *outFile = fopen(outputFile, "w+");
    if (!outFile)
    {
        println("Error opening output file!");
        return;
    }

    struct ffblk fileInfo;
    int done;
    char searchPath[256];
    char fullPath[512];

    // Create the search path for all files in the directory
    strcpy(searchPath, directory);
    strcat(searchPath, "\\*.*");

    done = findfirst(searchPath, &fileInfo, FA_DIREC);
    while (!done)
    {
        if (strcmp(fileInfo.ff_name, ".") != 0 && strcmp(fileInfo.ff_name, "..") != 0)
        {
            if (usefullpath == TRUE)
            {
                // Construct the full path
                strcpy(fullPath, directory);
                strcat(fullPath, "\\");
                strcat(fullPath, fileInfo.ff_name);
            }
            else
            {
                strcpy(fullPath, fileInfo.ff_name);
            }

            if (excepttype != NULL)
            {
                if (strstr(fullPath, excepttype) == NULL)
                    fprintf(outFile, "%s\n", fullPath);
            }
            else
            {
                // Write the full path to the output file
                fprintf(outFile, "%s\n", fullPath);
            }

            // If the current item is a directory, recurse into it
            if (fileInfo.ff_attrib & FA_DIREC)
            {
                listdirectory2file(fullPath, outputFile, usefullpath, excepttype);
            }
        }
        done = findnext(&fileInfo);
    }

    fclose(outFile);
}

static void appendstring2file(const char* fileName, const char* strToAppend) {
    FILE* file;

    // Open the file in append mode
    file = fopen(fileName, "a");

    // Check if the file opened successfully
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    // Append the string to the file
    fputs(strToAppend, file);
    fputs("\n", file); // Add a newline character for readability

    // Close the file
    fclose(file);
}

/***
 *  ****************** MEMORY AND CPU OPERATIONS ***************
 */

static void printstopexit(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    // Print the formatted string
    vprintf(format, args);

    // Print a newline character
    printf("\n");

    va_end(args);
    getch();
    exit(0);
}

/***
Monitors the memory
*/

static void monitormemory()
{
    struct SREGS sregs;
    // unsigned long totalMemory;
    unsigned long freeMemory;

    // Get free memory
    freeMemory = coreleft();
    // totalMemory = farcoreleft();

    // Print memory information
    println("Memory Free: %lu", freeMemory);
}

/// @brief freearray frees memory from an array
/// @param array
/// @param count
static void freearray(char **array, int count)
{
    if (array != NULL)
    {
        // Free the allocated memory
        for (int i = 0; i < count; i++)
        { // Only free up to numLines
            free(array[i]);
        }
        free(array);
    }
}

#endif /* __UTILS_H */