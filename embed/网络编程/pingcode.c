# include       <stdio.h>
# include       <fcntl.h>
# include       <errno.h>
# include       <signal.h>
# include       <sys/types.h>
# include       <sys/socket.h>
# include       <sys/time.h>
# include       <netinet/in.h>
# include       <arpa/inet.h>
# include       <netdb.h>

# define        ICMP_ECHO       8       /* 定义icmp echo */
# define        ICMP_ECHOREPLY  0       /* 定义icmp echo reply */
# define        ICMP_HEADSIZE   8       /* 定义icmp 包头的大小 */
# define        IP_HEADSIZE     20      /* 定义ip 包头的大小 */

	/* 以下是定义的两个用来比较大小的宏 */
# define		  MAX(a,b)	    ((a) > (b))? (a):(b)
# define        MIN(a,b)        ((a) > (b))? (b):(a)

typedef struct  tagIpHead               /* 定义icmp 包头数据结构 */
{
        u_char          ip_verlen;      /* ip version 和 ip header 的长度 */
        u_char          ip_tos;         /* service的ip类型 */
        u_short         ip_len;         /* ip包的长度 */
        u_short         ip_id;          /* ip包的标识号 */
        u_short         ip_fragoff;     /* ip包的起始地址 */
        u_char          ip_ttl;         /* ip包的存活时间 */
        u_char          ip_proto;       /* ip包的协议类型 */
        u_short         ip_chksum;      /* ip包头的检验长度 */
        u_long          ip_src_addr;    /* ip包的源地址 */
        u_long          ip_dst_addr;    /* ip包的目标地址 */
} IPHEAD;

typedef struct tagIcmpHead              /* icmp 头结构 */
{
        u_char          icmp_type;      /* icmp 服务类型 */
                                        /* 8 echo require, 0  echo reply */
        u_char          icmp_code;      /* icmp包头代码 */
        u_short         icmp_chksum;    /* icmp包头的检验和 */
        u_short         icmp_id;        /* icmp包的标识号 */
        u_short         icmp_seq;       /* icmp包队列 */
        u_char          icmp_data[1];   /* icmp包数据，指针类型 */
} ICMPHEAD;

/*********************************************************************
		函数ChkSum用来作奇偶校验。
	参数：
u_short * pIcmpData数据起始地址
		int iDataLen为数据长度
	返回值：
		校验结果
*********************************************************************/
u_short ChkSum( u_short * pIcmpData, int iDataLen )
{
        u_short iSum;
        u_short iOddByte;

        iSum = 0;

        while ( iDataLen > 1 ) { 
		pIcmpData++;       
                iSum ^=  *pIcmpData;		/* 和下一个数据作异或操作 */
                iDataLen -= 2;
        }

        if ( iDataLen == 1 ) {          /* 剩下的奇数位 */
                iOddByte = 0;
                *((u_char *)&iOddByte) = *(u_char *)pIcmpData;
                iSum ^= iOddByte;
        }

        iSum ^= 0xffff;                 /* 取反操作 */
        return(iSum);
}
/*******************************************************************************
	time_now函数用来返回系统时间。
	返回值：
		系统的当前时间和1970.1.1 00:00:00之间的时间间隔，
		用微妙来表示。
********************************************************************************/
long time_now()
{
        struct timeval now;
        long    lPassed;
        gettimeofday(&now, 0);
        lPassed = now.tv_sec * 1000000 + now.tv_usec;
                                        /* now.tv_sec 中保存了秒数*/
                                        /* now.tv_usec 中保存了微妙数 */
        return lPassed;
}

char*   host;                   /* 目标主机 */
char*   prog;                   /* 程序名称 */
extern  errno;                  /* 系统全局变量 */
long    lSendTime;              /* 数据包的发送时间，在发数据时被改变 */
u_short seq;                    /* icmp包队列 */
int     iTimeOut;               /* 超时时间 */
int     sock, sent, recvd, max, min, total;
                                /* sent : 发送出去的包的数目 */
                                /* recvd: 接收到的包的数目 */
                                /* max, min: 最大和最小的往返时间 */
                                /* total: 总共用时 */
                                /* 用来计算平均值 */
u_long  lHostIp;                /* 主机的ip地址 */
struct sockaddr_in it;          /* 目标主机的信息 */

int ping();
void stat();
main(int argc, char** argv)
{
        struct hostent* h;
        char            buf[200];
        char            dst_host[32];
        int             i, namelen;
        IPHEAD*         pIpHead;
        ICMPHEAD*       pIcmpHead;

        if (argc < 2) {         /* 每一个超时时间后ping一次指定的主机 */
                                /* 缺省的超时时间为1秒 */

                printf("usage: %s [-timeout] host|IP\n", argv[0]);
                exit(0);
        }
        prog = argv[0];
        host = argc == 2 ? argv[1] : argv[2];
        iTimeOut = argc == 2 ? 1 : atoi(argv[1]);

        /* 创建icmp套接字 */

        if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
                perror("socket");
                exit(2);
        }

        /* 设置目标主机信息 */

        bzero(&it, sizeof(it));
        it.sin_family = AF_INET;

        /* 检查主机地址类型 */

        if ( ( lHostIp = inet_addr(host) ) != INADDR_NONE ) {
                                                /* 合法的ip地址 */
                it.sin_addr.s_addr = lHostIp;
                strcpy( dst_host, host );
        } else if ( h = gethostbyname(host) ) {
                                                /* 获取合法的主机名称 */
                                                /* 从本地机上获取*/
                                                /* 或从DNS服务器上获取 */
                bcopy(h->h_addr, &it.sin_addr, h->h_length);
                sprintf( dst_host, "%s (%s)", host,
                         inet_ntoa(it.sin_addr) );
        } else {
                                                /* 错误的ip地址或主机名称 */
                                                /* 在这种情况下程序便退出 */
                fprintf( stderr, "bad IP or host\n" );
                exit(3);
        }
        namelen = sizeof(it);
 
        printf("\nDigger pinging  %s, send %d bytes\n",
                dst_host,
                IP_HEADSIZE + ICMP_HEADSIZE + sizeof(long)
                );
 
        seq = 0;                /* 第一个 icmp_seq = 0 */
        signal(SIGINT, stat);   /* 按键响应函数，当收到del 或 ctrl+c */
    /*按键操作后便调用 stat函数 */
                                /* 显示完结果，便退出 */
        signal(SIGALRM, ping);  /* 设置超时处理函数 */
                                /* 收到超时信号后便去调用ping函数 */
        alarm(iTimeOut);        /* 启动计时器 */
                                /* 以秒为单位 */
        ping();
        for ( ;; ) {            /* 等待每个返回的 */
                                /* icmp包，并且处理它 */
                register size;
                register u_char ttl;
                register delta;
                register iIpHeadLen;
 
                /* 阻塞并获取返回的数据包 */
 
                size = recvfrom(sock, buf, sizeof(buf), 0,
                                (struct sockaddr *)&it, &namelen);
                if (size == -1  &&  errno == EINTR) {
                                /* 获取数据时出错或者是系统调用被信号所中断 */
                        continue;
                }
 
                /* 计算数据包的往返时间 */
                /* 用当前系统时间减去发送此包时的系统时间来获得 */
 
                delta = (int)((time_now() - lSendTime)/1000);
 
                /* 获取返回的数据包并检查它的ip头 */
 
                pIpHead = (IPHEAD *)buf;
 
                /* 检查包的大小，如果包太小，那么就不是返回的icmp数据包 */
                /* 这时，便丢弃这个包 */
 
                iIpHeadLen = (int)((pIpHead->ip_verlen & 0x0f) << 2);
                if (size < iIpHeadLen + ICMP_HEADSIZE) {
                        continue;
                }
                ttl = pIpHead->ip_ttl;          /* 数据包的存活时间 */
 
                /* 获取icmp 头信息 */
                pIcmpHead = (ICMPHEAD *)(buf + iIpHeadLen);
 
                /* 不是icmp返回包，丢弃它 */
                if (pIcmpHead->icmp_type != ICMP_ECHOREPLY) {
                        continue;
                }
 
                /* 不是正确的icmp队列号，丢弃它 */
                if (pIcmpHead->icmp_id != seq || pIcmpHead->icmp_seq != seq) {
                        continue;
                }
 
                /* 给每个返回icmp包打印返回信息 */
                sprintf( buf, "icmp_seq=%u bytes=%d ttl=%d",
                        pIcmpHead->icmp_seq, size, ttl );
                fprintf(stderr, "reply from %s: %s time=%d ms\n",
                            host, buf, delta);
 
                /* 计算ping的结果，包括： */
                /* 数据包的最大、最小、平均往返时间 */
                /* 收到的返回icmp数据包的数目 */
                max = MAX(delta, max);
                min = min ? MIN(delta, min) : delta;
                total += delta;
                ++ recvd;
 
                /* 紧接着下一个的icmp数据包 */
 
                ++ seq;
        }
}
 
/*******************************************************************************
	ping函数
		向指定的目标地址发送icmp数据包。
	无参数，无返回值。
	使用全局变量。
*******************************************************************************/
ping()
{
        char    buf[200];
        int     iPacketSize;
 
        /* 设置icmp数据包的头信息 */
 
        ICMPHEAD *pIcmpHead = (ICMPHEAD *)buf;
        pIcmpHead->icmp_type = ICMP_ECHO;
        pIcmpHead->icmp_code = 0;
        pIcmpHead->icmp_id = seq;
        pIcmpHead->icmp_seq = seq;
        pIcmpHead->icmp_chksum = 0;
 
        /* store time information as icmp packet content, 4 bytes */
        /* 你也可以在此存储一些别的信息 */
 
        *((long *)pIcmpHead->icmp_data) = time_now();
 
        iPacketSize = ICMP_HEADSIZE + 4;        /* icmp包的长度 */
 
        /* icmp包数据的奇偶校验 */
 
        pIcmpHead->icmp_chksum = ChkSum((u_short *)pIcmpHead,
                                                iPacketSize );
 
        /* 存储发送包的时间，以便用来计算数据包的往返时间 */
        lSendTime = time_now();
 
        /* 向指定的目标地址发送icmp数据包 */
        if ( sendto(sock, buf, iPacketSize, 0, (struct sockaddr *)&it,
                sizeof(it) ) < 0) {
                perror("send failed");
                exit(6);
        }
 
        /* 总共发送的数据包数目 */
        ++sent;
 
        /* 重新设置计时器 */
        alarm(iTimeOut);
}
 /*******************************************************************************
	stat函数：
		打印结果信息。
*********************************************************************************/
void stat()
{
        if (sent) {
                printf("\n----- %s ping statistics summerized by Digger-----\n"
                        , host );
                printf("%d packets sent, %d packets received, %.2f%% lost\n",
                        sent, recvd, (float)(sent-recvd)/(float)sent*100 );
        }
        if (recvd) {
                printf("round_trip min/avg/max: %d/%d/%d ms\n\n",
                        min, total/recvd, max );
        }
        exit(0);
}
