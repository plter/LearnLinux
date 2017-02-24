#ifndef PACHONG_H
#define PACHONG_H

#define URL_LEN 256		// URL长度

struct html_db {	// 原始数据库
  char url[URL_LEN];		// 来源URL
  char text[128*1024];	// 来源URL对应HTML
};

struct html_db_idx {	// 索引库
  char url[URL_LEN];		// 来源URL
  unsigned long offset;	// 原始数据库中对应偏移
};

struct email_db {
  char email[128];
};

struct mobilenum_db {	// 手机号数据库
  char num[12];		// 手机号
  char text[64];		// 手机号所在文字区域
//  char url[URL_LEN];		// 来源URL
  unsigned long offset;	// 原始数据库中对应偏移
};

struct feature_db {	// 特征库
  char name[64];		// 特征名
  unsigned long offset;	// 原始数据库中对应偏移
};

#endif
