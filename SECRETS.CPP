#include <CONIO.H>
#include <STRING.H>
#include <STDLIB.H>
#include <DOS.H>
#include <STDIO.H>
#include "SECRETS.H"

void SecretThings::dosecretwait(char *zmgrsecret)
{
	int waittime = 0;
	char errorcode[50];
	strcpy(zmgrsecret, &zmgrsecret[strlen("WAIT")]);
	waittime = atoi(zmgrsecret);
	if ((waittime > 0) && (waittime < 1000))
	{
		printf("Waiting for %s seconds\n", zmgrsecret);
		delay(waittime * 1000);
	}
}

void SecretThings::dosecretthings(char *zmgrsecret)
{
	if (strstr(zmgrsecret, "WAIT"))
		dosecretwait(zmgrsecret);
}