#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#define N 32
#define err_log(log)\
	do{\
	perror(log);\
	exit(1);\
	}while(0)
typedef struct sockaddr SA;
void do_list(struct sockaddr_in serveraddr);
void do_get(struct sockaddr_in serveraddr,const char *filename);
void do_put(struct sockaddr_in serveraddr,const char *filename);
int main(int argc, const char *argv[])
{
	
	


	char buf[N]={0};
	struct sockaddr_in serveraddr;
	socklen_t len=sizeof(SA);
	bzero(&serveraddr,len);

	if(argc!=3)
	{
		fprintf(stderr,"User:%s IP PORT",argv[0]);
		return -1;
	}
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr=inet_addr(argv[1]);

	while(1)
	{
		puts("***********************************************");
		puts("1.list   2.get filename  3.put filename  4.quit");
		puts("***********************************************");
		puts("input cmd>>>");
		fgets(buf,N,stdin);
		buf[strlen(buf)-1]='\0';
		switch(buf[0])
		{
		case 'l':
			do_list(serveraddr);
			break;
		case 'g':
			do_get(serveraddr,buf+4);
			break;
		case 'p':
			do_put(serveraddr,buf+4);
			break;
		case 'q':
			exit(0);
			break;
		default:
			puts("cmd error");
			break;
		}

	}
	return 0;
}

void do_list(struct sockaddr_in serveraddr)
{

}
void do_get(struct sockaddr_in serveraddr,const char *filename)
{
}
void do_put(struct sockaddr_in serveraddr,const char *filename)
{
}
