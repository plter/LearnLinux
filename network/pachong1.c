#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include "pachong.h"

/* 建立一个HTTP TCP 连接的辅助函数 */
int htconnect (char *domain,int port )
{
	int white_sock;
	struct hostent * site;
	struct sockaddr_in me;
	site = gethostbyname(domain);
	if (site==NULL)
	{
		return -2;
	}
	white_sock = socket (AF_INET, SOCK_STREAM, 0 ) ;
	if (white_sock<0)
	{
		return -1;
	}
	memset(&me,0,sizeof(struct sockaddr_in));
	memcpy(&me.sin_addr,site->h_addr_list[0],site->h_length);
	me.sin_family = AF_INET;
	me.sin_port = htons(port);
	return (connect (
			white_sock,
			(struct sockaddr*)&me,
			sizeof(struct sockaddr)
			)<0 ) ?1 : white_sock ;
}

/* 发送HTTP 信息头的辅助函数 */
int htsend(int sock,char *fmt,...)
{
	char BUF[1024];
	va_list argptr;
	va_start(argptr,fmt);
	vsprintf(BUF,fmt,argptr);
	va_end(argptr);
	return send(sock,BUF,strlen(BUF),0) ;
}

int geturl(char *host, char *url)
{
	int black_sock;
	char bugs_bunny[3];
	struct html_db htmldb;
	struct html_db_idx htmldbidx;
	int htmldb_text_len = 0;
	char tmpfile_buf[1024];
	char tmpfile_name[128];
	char newurl[1024];

	FILE *fp_htmldb;
	FILE *fp_htmldbidx;
	FILE *fp_tmpfile;

	printf("host=%s, url=%s\n", host, url);

	sprintf(tmpfile_name, "tmpfile.%d.%d", rand(), rand());
#ifdef DEBUG
	printf("tmpfile_name: %s\n", tmpfile_name);
#endif
	if(!(fp_tmpfile=fopen(tmpfile_name, "w")))
	{
		printf("open %s error!\n", tmpfile_name);
		return 1;
	}

	black_sock = htconnect(host, 80);
	if (black_sock<0)
	{
		printf ( "Socket Connect Error!\n" ) ;
		fclose(fp_tmpfile);
		unlink(tmpfile_name);
		return 1;
	}
	htsend(black_sock,"GET %s HTTP/1.1%c", url, 10);
	htsend(black_sock,"Host: %s%c", host, 10);
	htsend(black_sock,"%c",10);
	while (read(black_sock,bugs_bunny,1)>0 
			&& htmldb_text_len<(128*1024-1))
	{
#ifdef DEBUG
		printf("%c",bugs_bunny[0]);
#endif
		htmldb.text[htmldb_text_len++] = bugs_bunny[0];
		fputc(bugs_bunny[0], fp_tmpfile);
	}
	htmldb.text[htmldb_text_len] = 0;
	close(black_sock);
	fclose(fp_tmpfile);
	strcpy(htmldb.url, url);

	if(!(fp_htmldb=fopen("html.db", "a")))
	{
		printf("open html.db error!\n");
		unlink(tmpfile_name);
		return 1;
	}
	if(!(fp_htmldbidx=fopen("html.idx", "a")))
	{
		printf("open html.idx error!\n");
		fclose(fp_htmldb);
		unlink(tmpfile_name);
		return 1; 
	}

	strcpy(htmldbidx.url, url);
	htmldbidx.offset = ftell(fp_htmldb);
	fwrite(&htmldbidx, sizeof(htmldbidx), 1, fp_htmldbidx);
	fwrite(&htmldb, sizeof(htmldb), 1, fp_htmldb);

	fclose(fp_htmldbidx);
	fclose(fp_htmldb);

	fp_tmpfile=fopen(tmpfile_name, "r");
	fgets(tmpfile_buf, 1024, fp_tmpfile);
	while(!feof(fp_tmpfile))
	{
		if(strstr(tmpfile_buf, "<a href=\""))
		{
			strcpy(newurl, "/");
			strcat(newurl,
				strstr(tmpfile_buf, "<a href=\"")+9);
			newurl[
				strlen(newurl)
				-strlen(strstr(newurl, "\""))
			] = 0;

			fp_htmldbidx = fopen("html.idx", "r");
			fread(&htmldbidx, 
				sizeof(htmldbidx),
				1, 
				fp_htmldbidx);
			while(!feof(fp_htmldbidx))
			{
				if(strcmp(htmldbidx.url, newurl)==0)
				{
					fclose(fp_htmldbidx);
					goto next;
				}
				fread(&htmldbidx,
					sizeof(htmldbidx),
					1,
					fp_htmldbidx);
			}
			fclose(fp_htmldbidx);
			geturl(host, newurl);
next:;
		} 
		fgets(tmpfile_buf, 1024, fp_tmpfile);
	}
	fclose(fp_tmpfile);
	unlink(tmpfile_name);
	return 0;
}

void menu()
{
	printf("\033c");
	printf("\033[1;1H┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓");
	printf("\033[2;1H┃0,退出\t1,抓取数据\t2,分析数据\t3,搜索\t4,生成手机号库    ┃");
	printf("\033[3;1H┃5,群发短信\t6,生成Email库\t7,群发Email                               ┃");
	printf("\033[4;1H┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛");
	printf("\033[3;72H:_\b");
}

int feature()
{
	struct html_db htmldb;
	struct feature_db featuredb;

	FILE *fp_htmldb;
	FILE *fp_featuredb;

	if(!(fp_htmldb=fopen("html.db", "r")))
	{
		printf("open html.db error!\n");
		return 1;
	}
	if(!(fp_featuredb=fopen("feature.db", "w")))
	{
		printf("open feature.db error!\n");
		fclose(fp_htmldb);
		return 1;
	}

	fread(&htmldb, sizeof(htmldb), 1, fp_htmldb);
	while(!feof(fp_htmldb))
	{
		int count = 0;
		featuredb.offset 
			= ftell(fp_htmldb)-(128*1024+URL_LEN);
		for(count=0; count<(128*1024); count++)
		{
			if(htmldb.text[count]=='>')
			{
				int featuredb_name_len = 0;
				count++;
				while(htmldb.text[count]!='<' 
					&& count<(128*1024)
					&& featuredb_name_len<(64-1))
				{
					featuredb.name[featuredb_name_len++]
						= htmldb.text[count];
					count++;
				}
				featuredb.name[featuredb_name_len] = 0;
				if(featuredb.name[0]!=0)
				{
					fwrite(&featuredb, 
						sizeof(featuredb),
						1, 
						fp_featuredb);
					printf("name=%s, offset=%d\n", 
						featuredb.name, 
						featuredb.offset);
				}
			}
		}
		fread(&htmldb, sizeof(htmldb), 1, fp_htmldb);
	}
	fclose(fp_featuredb);
	fclose(fp_htmldb);
	return 0;
}

int search()
{
	struct html_db htmldb;
	struct feature_db featuredb;
	char buf[1024];
	char tmphtml_name[1024];
	int  tmphtml_count = 0;

	FILE *fp_htmldb;
	FILE *fp_featuredb;
	FILE *fp_tmphtml;
	FILE *fp_tmpindexhtml;

	printf("\033[4;1H");
	if(!(fp_htmldb=fopen("html.db", "r")))
	{
		printf("open html.db error!\n");
		return 1;
	}
	if(!(fp_featuredb=fopen("feature.db", "r")))
	{
		printf("open feature.db error!\n");
		fclose(fp_htmldb);
		return 1;
	}

	printf("请输入搜索内容：");
	fgets(buf, 1024, stdin);
	buf[strlen(buf)-1] = 0;

	if(!(fp_tmpindexhtml=fopen("tmpindex.html", "w")))
	{
		printf("open tmpindex.html error!\n");
		fclose(fp_featuredb);
		fclose(fp_htmldb);
		return 1;
	}
	fprintf(fp_tmpindexhtml, "<html><head>");
	fprintf(fp_tmpindexhtml, "<title>%s</title>", buf);
	fprintf(fp_tmpindexhtml, "</head><body>\n");

	fread(&featuredb, sizeof(featuredb), 1, fp_featuredb);
	while(!feof(fp_featuredb))
	{
		if(strstr(featuredb.name, buf))
		{
			printf("feature=%s\n", featuredb.name);
#ifdef DEBUG
	printf("offset=%d\n", featuredb.offset);
#endif
			fseek(fp_htmldb, featuredb.offset, 0);
			fread(&htmldb, sizeof(htmldb), 1, fp_htmldb);
			printf("URL=%s\n", htmldb.url);

			sprintf(tmphtml_name,
				"tmp%d.html", tmphtml_count++);
			if(!(fp_tmphtml=fopen(tmphtml_name, "w")))
			{
				printf("open %s error!\n", tmphtml_name);
			} else {
				fwrite(htmldb.text, 
					strlen(htmldb.text), 
					1, 
					fp_tmphtml);
				fclose(fp_tmphtml);

				fprintf(fp_tmpindexhtml, 
					"%s<br>\n", featuredb.name);
				fprintf(fp_tmpindexhtml,
					"%s<a href=\"%s\">网页快照</a><br><br>\n", 
					htmldb.url,
					tmphtml_name); 
			}
		}
		fread(&featuredb, sizeof(featuredb), 1, fp_featuredb); 
	}
	fclose(fp_featuredb);
	fclose(fp_htmldb);

	fprintf(fp_tmpindexhtml, "</body></html>");
	fclose(fp_tmpindexhtml);

	system("firefox tmpindex.html");

	while(tmphtml_count)
	{
		sprintf(tmphtml_name, 
			"tmp%d.html", --tmphtml_count);
		unlink(tmphtml_name);
	}
	unlink("tmpindex.html");

	return 0;
}

int mobilenum()
{
	struct feature_db featuredb;
	struct mobilenum_db mobilenumdb;

	FILE *fp_featuredb;
	FILE *fp_mobilenumdb;

	printf("\033[4;1H");
	if(!(fp_featuredb=fopen("feature.db", "r")))
	{
		printf("open feature.db error!\n");
		return 1;
	}
	if(!(fp_mobilenumdb=fopen("mobilenum.db", "w")))
	{
		printf("open mobilenum.db error!\n");
		fclose(fp_featuredb);
		return 1;
	}

	fread(&featuredb, sizeof(featuredb), 1, fp_featuredb);
	while(!feof(fp_featuredb))
	{
		int count = 0;
		int count1 = 0;
		for(count=0; count<53&&featuredb.name[count]; count++)
		{
			if(featuredb.name[count]>='0' && featuredb.name[count]<='9')
			{
				for(count1=0; count1<11; count1++)
				{
					if(featuredb.name[count+count1]<'0'
						|| featuredb.name[count+count1]>'9')
					{
						break;
					}
				}
				if(count1==11)
				{
					strncpy(mobilenumdb.num, featuredb.name+count, 11);
					strcpy(mobilenumdb.text, featuredb.name);
					mobilenumdb.offset = featuredb.offset;
					printf("num=%s %s\n", mobilenumdb.num, featuredb.name);
					fwrite(&mobilenumdb, sizeof(mobilenumdb), 1, fp_mobilenumdb);
				}
			}
		}
		fread(&featuredb, sizeof(featuredb), 1, fp_featuredb); 
	}

	fclose(fp_mobilenumdb);
	fclose(fp_featuredb);
	return 0;
}

int sms()
{
	int fd_ttys = 0;
	struct mobilenum_db mobilenumdb;
	char text[256];
	char atcmd[128];
	char atret[128];

	FILE *fp_mobilenumdb;

	printf("\033[5;1H");
	if(!(fp_mobilenumdb=fopen("mobilenum.db", "r")))
	{
		printf("open mobilenum.db error!\n");
		return 1;
	}

	printf("输入短信内容：");
	fgets(text, 256, stdin);
	text[strlen(text)-1] = 0;

	fread(&mobilenumdb, sizeof(mobilenumdb), 1, fp_mobilenumdb);
	while(!feof(fp_mobilenumdb))
	{
#ifdef DEBUG
	printf("mobilenum=%s\n", mobilenumdb.num);
#endif
		fd_ttys=open("/dev/ttyS0", O_RDWR);
// /dev/ttySAC0
		if(fd_ttys>0)
		{
			sprintf(atcmd, "AT+CMGF=1\n");
			write(fd_ttys, atcmd, strlen(atcmd));
			read(fd_ttys, atret, 128);
			sprintf(atcmd, "AT+CMGS=\"%s\"\n", mobilenumdb.num);
			write(fd_ttys, atcmd, strlen(atcmd));
			read(fd_ttys, atret, 128);
			sprintf(atcmd, "%s%c", text, 26);
			write(fd_ttys, atcmd, strlen(atcmd));
			read(fd_ttys, atret, 128);
			close(fd_ttys);
		}
		fread(&mobilenumdb, sizeof(mobilenumdb), 1, fp_mobilenumdb); 
	}
	fclose(fp_mobilenumdb);
	return 0;
}

int emaildb()
{
	struct html_db htmldb;
	struct email_db emaildb;
	char email[1024];

	FILE *fp_htmldb;
	FILE *fp_emaildb;

	printf("\033[5;1H");
	if(!(fp_htmldb=fopen("html.db", "r")))
	{
		printf("open html.db error!\n");
		return 1;
	}
	if(!(fp_emaildb=fopen("email.db", "w+")))
	{
		printf("open email.db error!\n");
		fclose(fp_htmldb);
		return 1;
	}

	fread(&htmldb, sizeof(htmldb), 1, fp_htmldb);
	while(!feof(fp_htmldb))
	{
		if(strstr(htmldb.text, "<a href=\"mailto:"))
		{
			strncpy(email, 
				strstr(htmldb.text, "<a href=\"mailto:"
				)+16, 
				1024-1);
			email[strlen(email)-strlen(strstr(email, "\""))] = 0;
			fseek(fp_emaildb, 0, 0);
			fread(&emaildb, sizeof(emaildb), 1, fp_emaildb);
			while(!feof(fp_emaildb))
			{
				if(strcmp(emaildb.email, email)==0)
				{
					break;
				}
				fread(&emaildb, sizeof(emaildb), 1, fp_emaildb); 
			}
			if(strcmp(emaildb.email, email)!=0)
			{
				strcpy(emaildb.email, email);
				printf("email=%s\n", emaildb.email);
				fseek(fp_emaildb, 0, 2);
				fwrite(&emaildb, sizeof(emaildb), 1, fp_emaildb);
			}
		}
		fread(&htmldb, sizeof(htmldb), 1, fp_htmldb);
	}

	fclose(fp_emaildb);
	fclose(fp_htmldb);
	return 0; 
}

int emailsend()
{
	struct email_db emaildb;
	char from[1024];
	char subject[1024];
	char text[1024];
	char sendmailcmd[1024];

	FILE *fp_emaildb;
	FILE *fp_mailtext;

	printf("\033[5;1H");
	if(!(fp_emaildb=fopen("email.db", "r")))
	{
		printf("open email.db error!\n");
		return 1;
	}

	printf("请输入发件人：");
	fgets(from, 1024, stdin);
	from[strlen(from)-1] = 0;
	printf("请输入邮件标题：");
	fgets(subject, 1024, stdin);
	subject[strlen(subject)-1] = 0;
	printf("请输入邮件正文：");
	fgets(text, 1024, stdin);
	text[strlen(text)-1] = 0;

	fread(&emaildb, sizeof(emaildb), 1, fp_emaildb);
	while(!feof(fp_emaildb))
	{
		if(!(fp_mailtext=fopen("mailtext.tmp", "w")))
		{
			printf("open mailtext.tmp error!\n");
			fclose(fp_emaildb);
			return 1;
		}
		fprintf(fp_mailtext, "From: %s\n", from);
		fprintf(fp_mailtext, "To: %s\n", emaildb.email);
		fprintf(fp_mailtext, "Subject: %s\n", subject);
		fprintf(fp_mailtext, "%s\n", text);
		fclose(fp_mailtext);
		printf("email=%s\n", emaildb.email);
		sprintf(sendmailcmd, 
			"cat mailtext.tmp | sendmail %s", 
			emaildb.email);
//		system(sendmailcmd);
		unlink("mailtext.tmp");

		fread(&emaildb, sizeof(emaildb), 1, fp_emaildb);
	}

	fclose(fp_emaildb);
	return 0;
}

int main(int argc, char **argv)
{
	int key;
	char host[256];
	char url[1024];

	if(argc==3)
	{
		strcpy(host, argv[1]);
		strcpy(url, argv[2]);
	}

	while(1){ 
		menu();
		scanf("%d", &key);
		getchar();
		switch(key)
		{
			case 1:
					if(argc!=3)
					{
						printf("\033[4;1H请输入主机名：");
						fgets(host, 256, stdin);
						host[strlen(host)-1] = 0;
						printf("\033[5;1H请输入URL：");
						fgets(url, 1024, stdin);
						url[strlen(url)-1] = 0;
					}
					geturl(host, url);
					printf("按回车键继续... ...");
					getchar();
					break;
			case 2:
					feature();
					printf("按回车键继续... ...");
					getchar();
					break;
			case 3:
					search();
					printf("按回车键继续... ...");
					getchar();
					break;
			case 4:
					mobilenum();
					printf("按回车键继续... ...");
					getchar();
					break;
			case 5:
					sms();
					printf("按回车键继续... ...");
					getchar();
					break;
			case 6:
					emaildb();
					printf("按回车键继续... ...");
					getchar();
					break;
			case 7:
					emailsend();
					printf("按回车键继续... ...");
					getchar();
					break;
			case 0:
					printf("\033c");
					return 0;
			default:
					printf("\033[4;1H选择错误！");
		}
	}
	return 0;
}

