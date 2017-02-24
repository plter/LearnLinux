#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

const char *test_file = "/tmp/test_lock";

int main(int argc,char *argv[]) 
{
	int fd;
	int byte_count;
	char *byte_to_write = "A";
	struct flock region_1;
	struct flock region_2;
	int res;

	if((fd = open(test_file, O_RDWR | O_CREAT, 0644))==-1)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	for(byte_count = 0; byte_count < 100; byte_count++) 
	{
		if((write(fd, byte_to_write, 1))==-1)
		{
			perror("write");
			exit(EXIT_FAILURE);
		}
	}

	region_1.l_type = F_RDLCK;
	region_1.l_whence = SEEK_SET;
	region_1.l_start = 10;
	region_1.l_len = 20; 
	    
	region_2.l_type = F_WRLCK;
	region_2.l_whence = SEEK_SET;
	region_2.l_start = 40;
	region_2.l_len = 10;

	/* now lock the file */
	printf("Process %d locking file\n", getpid());

	res = fcntl(fd, F_SETLK, &region_1);
	if (res == -1) 
		fprintf(stderr, "Failed to lock region 1\n");
	res = fcntl(fd, F_SETLK, &region_2);
	if (res == -1) 
		fprintf(stderr, "Failed to lock region 2\n");    
	/* and wait for a while */
	sleep(10);
	printf("Process %d closing file\n", getpid());    
	close(fd);
	exit(EXIT_SUCCESS);
}
