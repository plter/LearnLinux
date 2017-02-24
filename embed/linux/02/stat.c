#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	struct stat info;
	if (argc != 2)
	{
		printf ("enter like:stat filename\n");
		exit (1);
	}
	if (stat(argv[1], &info) == 0)
	{
	printf ("%s uid is %d\n", argv[1], info.st_uid);
	printf ("%s size is %d\n", argv[1], info.st_size);
	printf ("%s lasttime is %s\n", argv[1], ctime(&info.st_ctime));
	}
	else
	{
		perror("stat file:");
	}
	exit (0);
}

