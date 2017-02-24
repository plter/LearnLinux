#include "stdio.h"
#include "stdlib.h"

main()
{
    int fd;
    int count;
    char buf[8192];
    if((fd=creat("wrtest.tmp", 0600))>0)
    {
       for(count=0; count<10000; count++)
          write(fd, buf, sizeof(buf));
    }
}


