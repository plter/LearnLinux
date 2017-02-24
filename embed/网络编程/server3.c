#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *cmds[]={ "help", "say", "hello", "bye", "exit", NULL};

int getcmd(char *cmd)
{
	int n=0;
	while (cmds[n]!=NULL)
	{
		if (strncmp(cmd,cmds[n],strlen(cmds[n]))==0)
			return n;
		n++;
	}
	return -1;
}

int main(void)
{
	char buf[1024];
	int ok;

	printf("Welcome! This is hello service!\n");
	fflush(stdout);
	ok=0;
	do
	{
		while (fgets(buf,1023,stdin)==NULL) ;
		switch (getcmd(buf))
		{
			case -1: 
				printf("Unknown command!\n"); 
				break;
			case 0: 
				printf("How may I help you, sir?\n"); 
				break;
			case 1: 
				printf("I will say %s",&buf[3]); 
				break;
			case 2: 
				printf("How¡®re you doing today?\n"); 
				break;
			case 3: 
				printf("Si ya, mate!\n"); ok=1; 
				break;
			case 4: 
				printf("Go ahead!\n"); ok=1; 
				break;
		}
		fflush(stdout);
	} while(!ok);
}
