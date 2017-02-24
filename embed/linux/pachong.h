#ifndef PACHONG_H
#define PACHONG_H

#define URL_LEN 256		// URL����

struct html_db {	// ԭʼ���ݿ�
  char url[URL_LEN];		// ��ԴURL
  char text[128*1024];	// ��ԴURL��ӦHTML
};

struct html_db_idx {	// ������
  char url[URL_LEN];		// ��ԴURL
  unsigned long offset;	// ԭʼ���ݿ��ж�Ӧƫ��
};

struct email_db {
  char email[128];
};

struct mobilenum_db {	// �ֻ������ݿ�
  char num[12];		// �ֻ���
  char text[64];		// �ֻ���������������
//  char url[URL_LEN];		// ��ԴURL
  unsigned long offset;	// ԭʼ���ݿ��ж�Ӧƫ��
};

struct feature_db {	// ������
  char name[64];		// ������
  unsigned long offset;	// ԭʼ���ݿ��ж�Ӧƫ��
};

#endif
