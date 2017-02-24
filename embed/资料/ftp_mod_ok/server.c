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
#include <dirent.h>
#define N 32
#define err_log(log)\
	do{\
	perror(log);\
	exit(1);\
	}while(0)
typedef struct sockaddr SA;

void process_list(int clientfd);
void process_get(int clientfd,const char *filename);
void process_put(int clientfd,const char *filename);
int main(int argc, const char *argv[])
{
	

	int serverfd,clientfd;
	struct sockaddr_in serveraddr,clientaddr;
	socklen_t len=sizeof(SA);
	bzero(&serveraddr,len);
	ssize_t bytes;
	char buf[N]={0};
	if(argc!=3)
	{
		fprintf(stderr,"User:%s IP PORT",argv[0]);
		return -1;
	}
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr=inet_addr(argv[1]);

	if((serverfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		err_log("fail to socket");
	}
	if(bind(serverfd,(SA*)&serveraddr,len)<0)
	{
		close(serverfd);
		err_log("fail to bind");
	}
	if(listen(serverfd,15)<0)
	{
		close(serverfd);
		err_log("fail to listen");
	}
	while(1)
	{
		if((clientfd=accept(serverfd,(SA*)&clientaddr,&len))<0)
		{
			perror("fail to accept");
			continue;
		}

		bytes=recv(clientfd,buf,N,0);
		if(bytes<=0)
		{
			break;
		}
		switch(buf[0])
		{
		case 'L':
			process_list(clientfd);
			break;
		case 'G':
			process_get(clientfd,buf+2);
			break;
		case 'P':
			process_put(clientfd,buf+2);
			break;
		}
		close(clientfd);

	}
	close(serverfd);


	return 0;
}

void process_list(int clientfd)
{

	
}
void process_get(int clientfd,const char *filename)
{

	

}
void process_put(int clientfd,const char *filename)
{
}
