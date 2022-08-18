#if !defined(__UTILS_H)
#define __UTILS_H

#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>
#include <DIR.H>
#include <DOS.H>
#include <SYS/STAT.H>

#define MAX_FILES 256
#define MAX_FILE_LENGTH 12
#define MAX_ARGS 10

static int splitIntoArray(char *str, const char *delim, char *arg_list[])
{
	unsigned count = 0;

	/* First call to strtok should be done with string and delimiter as first and second parameter*/
	char *token = strtok(str, delim);
	if (token == NULL)
		return NULL;

	/* Consecutive calls to the strtok should be with first parameter as NULL and second parameter as delimiter
	 * * return value of the strtok will be the split string based on delimiter*/
	while (token != NULL)
	{
		arg_list[count] = strdup(token);
		token = strtok(NULL, delim);
		count++;
	}

	arg_list[count] = NULL;

	return count;
}

static void removeChar(char *str, char charToRemmove)
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

static int get_files_in_directory(char *path, char *file_list[])
{
	struct find_t ffblk;
	int done;
	unsigned rtncount = 0;

	/* First call to strtok should be done with string and delimiter as first and second parameter*/
	done = _dos_findfirst(path, _A_NORMAL, &ffblk);
	if (done != 0)
		return NULL;

	while (!done)
	{
		file_list[rtncount] = strdup(ffblk.name);
		done = _dos_findnext(&ffblk);
		rtncount++;
	}

	file_list[rtncount] = NULL;

	return rtncount;
}

/***
 * Check for file and return size of file.
 * If the file is not found, it will return -1
 */
static int check_for_file(char *filename)
{
	struct stat info;
	int rtn;
	FILE *fptr;

	fptr = fopen(filename, "r");
	if (fptr == NULL)
	{
		perror(filename);
		rtn = -1;
	}
	else
		rtn = 1;

	return rtn;
}

static char *replaceWord(const char *s, const char *oldW,
						 const char *newW)
{
	char *result;
	int i, cnt = 0;
	int newWlen = strlen(newW);
	int oldWlen = strlen(oldW);

	// Counting the number of times old word
	// occur in the string
	for (i = 0; s[i] != '\0'; i++)
	{
		if (strstr(&s[i], oldW) == &s[i])
		{
			cnt++;

			// Jumping to index after the old word.
			i += oldWlen - 1;
		}
	}

	// Making new string of enough length
	result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

	i = 0;
	while (*s)
	{
		// compare the substring with the result
		if (strstr(s, oldW) == s)
		{
			strcpy(&result[i], newW);
			i += newWlen;
			s += oldWlen;
		}
		else
			result[i++] = *s++;
	}

	result[i] = NULL;
	return result;
}
#endif /* __UTILS_H */