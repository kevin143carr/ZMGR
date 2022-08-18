#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <STDLIB.H>
#include <STDIO.H>
#include <MEM.H>
#include <STRING.H>
#include <SYS/STAT.H>

#define MAX_LINES 100
#define MAX_LEN 100

static void remove_config_file_value(char *path, char *key)
{
    FILE *fptr;
    char line[MAX_LINES][MAX_LEN];
    int i = 0;
    int count = 0;

    if ((fptr = fopen(path, "r+")) == NULL)
    {
        perror("fopen()");
    }

    while (!feof(fptr) && !ferror(fptr))
        if (fgets(line[i], 100, fptr) != NULL)
        {
            if (strstr(line[i], key) != NULL)
                if (line[i][0] != '#')
                {
                    sprintf(line[i], "%s =\n", key);
                }
            i++;
        }

    fclose(fptr);

    if ((fptr = fopen(path, "w")) == NULL)
    {
        perror("fopen()");
    }
    for (count = 0; count < i; count++)
        fputs(line[count], fptr);

    fclose(fptr);
}

static void get_config_file_value(char *path, char *key, char *value)
{
    FILE *fptr;
    char line[100];

    memset(line, 0, sizeof(line));

    if ((fptr = fopen(path, "r+")) == NULL)
    {
        perror("fopen()");
    }

    while (fgets(line, 100, fptr) != NULL)
    {
        if (strstr(line, key) != NULL)
        {
            if (line[0] != '#')
            {
                strcpy(value, &line[strlen(key) + strlen(" = ")]);
                break;
            }
        }
    }

    fclose(fptr);
}

#endif