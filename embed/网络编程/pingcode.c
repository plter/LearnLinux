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

# define        ICMP_ECHO       8       /* ����icmp echo */
# define        ICMP_ECHOREPLY  0       /* ����icmp echo reply */
# define        ICMP_HEADSIZE   8       /* ����icmp ��ͷ�Ĵ�С */
# define        IP_HEADSIZE     20      /* ����ip ��ͷ�Ĵ�С */

	/* �����Ƕ�������������Ƚϴ�С�ĺ� */
# define		  MAX(a,b)	    ((a) > (b))? (a):(b)
# define        MIN(a,b)        ((a) > (b))? (b):(a)

typedef struct  tagIpHead               /* ����icmp ��ͷ���ݽṹ */
{
        u_char          ip_verlen;      /* ip version �� ip header �ĳ��� */
        u_char          ip_tos;         /* service��ip���� */
        u_short         ip_len;         /* ip���ĳ��� */
        u_short         ip_id;          /* ip���ı�ʶ�� */
        u_short         ip_fragoff;     /* ip������ʼ��ַ */
        u_char          ip_ttl;         /* ip���Ĵ��ʱ�� */
        u_char          ip_proto;       /* ip����Э������ */
        u_short         ip_chksum;      /* ip��ͷ�ļ��鳤�� */
        u_long          ip_src_addr;    /* ip����Դ��ַ */
        u_long          ip_dst_addr;    /* ip����Ŀ���ַ */
} IPHEAD;

typedef struct tagIcmpHead              /* icmp ͷ�ṹ */
{
        u_char          icmp_type;      /* icmp �������� */
                                        /* 8 echo require, 0  echo reply */
        u_char          icmp_code;      /* icmp��ͷ���� */
        u_short         icmp_chksum;    /* icmp��ͷ�ļ���� */
        u_short         icmp_id;        /* icmp���ı�ʶ�� */
        u_short         icmp_seq;       /* icmp������ */
        u_char          icmp_data[1];   /* icmp�����ݣ�ָ������ */
} ICMPHEAD;

/*********************************************************************
		����ChkSum��������żУ�顣
	������
u_short * pIcmpData������ʼ��ַ
		int iDataLenΪ���ݳ���
	����ֵ��
		У����
*********************************************************************/
u_short ChkSum( u_short * pIcmpData, int iDataLen )
{
        u_short iSum;
        u_short iOddByte;

        iSum = 0;

        while ( iDataLen > 1 ) { 
		pIcmpData++;       
                iSum ^=  *pIcmpData;		/* ����һ�������������� */
                iDataLen -= 2;
        }

        if ( iDataLen == 1 ) {          /* ʣ�µ�����λ */
                iOddByte = 0;
                *((u_char *)&iOddByte) = *(u_char *)pIcmpData;
                iSum ^= iOddByte;
        }

        iSum ^= 0xffff;                 /* ȡ������ */
        return(iSum);
}
/*******************************************************************************
	time_now������������ϵͳʱ�䡣
	����ֵ��
		ϵͳ�ĵ�ǰʱ���1970.1.1 00:00:00֮���ʱ������
		��΢������ʾ��
********************************************************************************/
long time_now()
{
        struct timeval now;
        long    lPassed;
        gettimeofday(&now, 0);
        lPassed = now.tv_sec * 1000000 + now.tv_usec;
                                        /* now.tv_sec �б���������*/
                                        /* now.tv_usec �б�����΢���� */
        return lPassed;
}

char*   host;                   /* Ŀ������ */
char*   prog;                   /* �������� */
extern  errno;                  /* ϵͳȫ�ֱ��� */
long    lSendTime;              /* ���ݰ��ķ���ʱ�䣬�ڷ�����ʱ���ı� */
u_short seq;                    /* icmp������ */
int     iTimeOut;               /* ��ʱʱ�� */
int     sock, sent, recvd, max, min, total;
                                /* sent : ���ͳ�ȥ�İ�����Ŀ */
                                /* recvd: ���յ��İ�����Ŀ */
                                /* max, min: ������С������ʱ�� */
                                /* total: �ܹ���ʱ */
                                /* ��������ƽ��ֵ */
u_long  lHostIp;                /* ������ip��ַ */
struct sockaddr_in it;          /* Ŀ����������Ϣ */

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

        if (argc < 2) {         /* ÿһ����ʱʱ���pingһ��ָ�������� */
                                /* ȱʡ�ĳ�ʱʱ��Ϊ1�� */

                printf("usage: %s [-timeout] host|IP\n", argv[0]);
                exit(0);
        }
        prog = argv[0];
        host = argc == 2 ? argv[1] : argv[2];
        iTimeOut = argc == 2 ? 1 : atoi(argv[1]);

        /* ����icmp�׽��� */

        if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
                perror("socket");
                exit(2);
        }

        /* ����Ŀ��������Ϣ */

        bzero(&it, sizeof(it));
        it.sin_family = AF_INET;

        /* ���������ַ���� */

        if ( ( lHostIp = inet_addr(host) ) != INADDR_NONE ) {
                                                /* �Ϸ���ip��ַ */
                it.sin_addr.s_addr = lHostIp;
                strcpy( dst_host, host );
        } else if ( h = gethostbyname(host) ) {
                                                /* ��ȡ�Ϸ����������� */
                                                /* �ӱ��ػ��ϻ�ȡ*/
                                                /* ���DNS�������ϻ�ȡ */
                bcopy(h->h_addr, &it.sin_addr, h->h_length);
                sprintf( dst_host, "%s (%s)", host,
                         inet_ntoa(it.sin_addr) );
        } else {
                                                /* �����ip��ַ���������� */
                                                /* ����������³�����˳� */
                fprintf( stderr, "bad IP or host\n" );
                exit(3);
        }
        namelen = sizeof(it);
 
        printf("\nDigger pinging  %s, send %d bytes\n",
                dst_host,
                IP_HEADSIZE + ICMP_HEADSIZE + sizeof(long)
                );
 
        seq = 0;                /* ��һ�� icmp_seq = 0 */
        signal(SIGINT, stat);   /* ������Ӧ���������յ�del �� ctrl+c */
    /*�������������� stat���� */
                                /* ��ʾ���������˳� */
        signal(SIGALRM, ping);  /* ���ó�ʱ������ */
                                /* �յ���ʱ�źź��ȥ����ping���� */
        alarm(iTimeOut);        /* ������ʱ�� */
                                /* ����Ϊ��λ */
        ping();
        for ( ;; ) {            /* �ȴ�ÿ�����ص� */
                                /* icmp�������Ҵ����� */
                register size;
                register u_char ttl;
                register delta;
                register iIpHeadLen;
 
                /* ��������ȡ���ص����ݰ� */
 
                size = recvfrom(sock, buf, sizeof(buf), 0,
                                (struct sockaddr *)&it, &namelen);
                if (size == -1  &&  errno == EINTR) {
                                /* ��ȡ����ʱ���������ϵͳ���ñ��ź����ж� */
                        continue;
                }
 
                /* �������ݰ�������ʱ�� */
                /* �õ�ǰϵͳʱ���ȥ���ʹ˰�ʱ��ϵͳʱ������� */
 
                delta = (int)((time_now() - lSendTime)/1000);
 
                /* ��ȡ���ص����ݰ����������ipͷ */
 
                pIpHead = (IPHEAD *)buf;
 
                /* �����Ĵ�С�������̫С����ô�Ͳ��Ƿ��ص�icmp���ݰ� */
                /* ��ʱ���㶪������� */
 
                iIpHeadLen = (int)((pIpHead->ip_verlen & 0x0f) << 2);
                if (size < iIpHeadLen + ICMP_HEADSIZE) {
                        continue;
                }
                ttl = pIpHead->ip_ttl;          /* ���ݰ��Ĵ��ʱ�� */
 
                /* ��ȡicmp ͷ��Ϣ */
                pIcmpHead = (ICMPHEAD *)(buf + iIpHeadLen);
 
                /* ����icmp���ذ��������� */
                if (pIcmpHead->icmp_type != ICMP_ECHOREPLY) {
                        continue;
                }
 
                /* ������ȷ��icmp���кţ������� */
                if (pIcmpHead->icmp_id != seq || pIcmpHead->icmp_seq != seq) {
                        continue;
                }
 
                /* ��ÿ������icmp����ӡ������Ϣ */
                sprintf( buf, "icmp_seq=%u bytes=%d ttl=%d",
                        pIcmpHead->icmp_seq, size, ttl );
                fprintf(stderr, "reply from %s: %s time=%d ms\n",
                            host, buf, delta);
 
                /* ����ping�Ľ���������� */
                /* ���ݰ��������С��ƽ������ʱ�� */
                /* �յ��ķ���icmp���ݰ�����Ŀ */
                max = MAX(delta, max);
                min = min ? MIN(delta, min) : delta;
                total += delta;
                ++ recvd;
 
                /* ��������һ����icmp���ݰ� */
 
                ++ seq;
        }
}
 
/*******************************************************************************
	ping����
		��ָ����Ŀ���ַ����icmp���ݰ���
	�޲������޷���ֵ��
	ʹ��ȫ�ֱ�����
*******************************************************************************/
ping()
{
        char    buf[200];
        int     iPacketSize;
 
        /* ����icmp���ݰ���ͷ��Ϣ */
 
        ICMPHEAD *pIcmpHead = (ICMPHEAD *)buf;
        pIcmpHead->icmp_type = ICMP_ECHO;
        pIcmpHead->icmp_code = 0;
        pIcmpHead->icmp_id = seq;
        pIcmpHead->icmp_seq = seq;
        pIcmpHead->icmp_chksum = 0;
 
        /* store time information as icmp packet content, 4 bytes */
        /* ��Ҳ�����ڴ˴洢һЩ�����Ϣ */
 
        *((long *)pIcmpHead->icmp_data) = time_now();
 
        iPacketSize = ICMP_HEADSIZE + 4;        /* icmp���ĳ��� */
 
        /* icmp�����ݵ���żУ�� */
 
        pIcmpHead->icmp_chksum = ChkSum((u_short *)pIcmpHead,
                                                iPacketSize );
 
        /* �洢���Ͱ���ʱ�䣬�Ա������������ݰ�������ʱ�� */
        lSendTime = time_now();
 
        /* ��ָ����Ŀ���ַ����icmp���ݰ� */
        if ( sendto(sock, buf, iPacketSize, 0, (struct sockaddr *)&it,
                sizeof(it) ) < 0) {
                perror("send failed");
                exit(6);
        }
 
        /* �ܹ����͵����ݰ���Ŀ */
        ++sent;
 
        /* �������ü�ʱ�� */
        alarm(iTimeOut);
}
 /*******************************************************************************
	stat������
		��ӡ�����Ϣ��
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
