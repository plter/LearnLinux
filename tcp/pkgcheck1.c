#include <stdio.h>
#include "stdlib.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <sys/types.h>

/* 定义tpc/ip数据包的数据结构 */
struct ip_tcp {
    struct  iphdr *iph;
    struct  tcphdr *tcph;
    unsigned char buffer[500];
};

int main()
{
	int sock, bytes_recieved, fromlen,n,id=1;
	unsigned char buffer[65535];
	struct sockaddr_in from,ff;
	struct ip  *ip;
	struct tcphdr *tcp;
	struct ip_tcp *iptcp;

	/* 建立原始TCP包方式 收到IP+TCP信息包 */
	sock = socket(AF_INET, SOCK_RAW,IPPROTO_TCP);
	printf("IPPROTO_TCP= %d \n",IPPROTO_TCP);

	if (sock<=0) exit(0);

	id=1;
	while(1)
	{
	fromlen = sizeof(from);
		/* 接收包 */
	bytes_recieved = recvfrom(sock, buffer, sizeof(buffer),0,(struct sockaddr *)&from, &fromlen);

	if (bytes_recieved>0)
	{
		printf("ok");
		/* IP头为最小160位 8位一个字节 共20个字节 （0x14） 后 */
		/* 为添加内容TCP，UDP等信息 */
		ip = (struct ip *)buffer;
		/* tcp信息包 从整个IP/TCP包 buffer + (4*ip->ip_length) 地址处开始 */
		tcp = (struct tcphdr *)(buffer + (4*ip->ip_hl));
		if (ntohs(tcp->dest)!=23 )  /* 可以改为其它(说明不显示TELNET的信息) */
		{
			printf("\n ID=::: %d\n",id);
			printf("Bytes received ::: %5d\n",bytes_recieved);
			printf("---- IP info begin ---- \n");
			printf("IP header length ::: %d\n",ip->ip_hl);
			printf("IP sum      size ::: %d\n",ntohs(ip->ip_len));
			printf("Protocol ::: %d\n",ip->ip_p);
			printf("IP_source address ::: %s \n",inet_ntoa(ip->ip_src));
			printf("IP_dest address ::: %s \n",inet_ntoa(ip->ip_dst));
			for (n=0;n<4*ip->ip_hl;n++)
			{
				printf("%02X ",buffer[n]);
			}
			printf("\n");
			printf("----  IP info end  ----  \n");
			printf("----TCP info begin ----  \n");
			/* ntohs  网络的 short int 转为本地的 short int  */
			printf("Source port ::: %d\n",ntohs(tcp->source));
			printf("Dest port  ::: %d\n",ntohs(tcp->dest));
			printf("Source address ::: %s\n",inet_ntoa(from.sin_addr));
			/* 列出收到的TCP信息内容 一般TCP头20个字节 */
			/* 所以TCP后20个字节是传送的信息 */
			for (n=(4*ip->ip_hl+20);n<ntohs(ip->ip_len);n++)
			{
				printf("%c",buffer[n]);
				if (n>65)
				{
					break;
				}
			}
		  	printf("----TCP info end    ----  \n");
          		id=id+1;        
         	}  /*>23 end */
	   } /*>0 end */
	} /*while end */
}
