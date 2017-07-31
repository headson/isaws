/************************************************************************
*Author      : Sober.Peng 17-07-25
*Description : 利用popen实现system功能,规避system返回值判断问题
************************************************************************/
#ifndef _MYSYSTEM_H_
#define _MYSYSTEM_H_

#include <stdio.h>

int my_system(const char * cmd) {
  if (cmd == NULL) {
    printf("my_system cmd is NULL!\n");
    return -1;
  }
#ifdef _LIMUX
  FILE * fp;
  if ((fp = popen(cmd, "r")) != NULL) {
    int res;
    char buf[1024];
    while(fgets(buf, sizeof(buf), fp)) {
      printf("%s", buf);
    }

    if ((res = pclose(fp)) == -1) {
      printf("close popen file pointer fp error!\n");
      return res;
    } else if (res == 0) {
      return res;
    }
    printf("popen res is :%d\n", res);
    return res;
  }

  perror("popen");
  printf("popen error: %s error %s./n",
         cmd, strerror(errno));
#endif  // _LIMUX
  return -1;
}

#endif  // _MYSYSTEM_H_
