/************************************************************************
*Author      : Sober.Peng 17-07-25
*Description : ����popenʵ��system����,���system����ֵ�ж�����
************************************************************************/
#ifndef _MYSYSTEM_H_
#define _MYSYSTEM_H_

int my_system(const char * cmd) {
  if (cmd == NULL) {
    printf("my_system cmd is NULL!\n");
    return -1;
  }

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
    } else {
      printf("popen res is :%d\n", res);
      return res;
    }
  }

  perror("popen");
  printf("popen error: %s/n", strerror(errno));
  return -1;
}

#endif  // _MYSYSTEM_H_