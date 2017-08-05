#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <asm/types.h>
#endif

int main() {
  system("killall watchdog");

  int fd;

#if 0
  sleep(1);

  fd = open("/dev/watchdog", O_WRONLY);
  if (fd < 0) {
    printf("open error!!\n");
  }
  ioctl(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
  ioctl(fd, WDIOC_SETTIMEOUT, 6);

  while (true) {
    ioctl(fd, WDIOC_KEEPALIVE, 1);
    sleep(1);
  }
  close(fd);
#endif

#ifdef IMX6Q
  sleep(1);

  typedef struct {
    int eType;     // 类型
    int nState;    // 状态
    int nParam;    // 占空比
  } TAG_EXT_GPIO;

  TAG_EXT_GPIO cExt;
  fd = open("/dev/ihs_gpio", O_RDWR);
  if (fd < 0) {
    perror("open");
    return -1;
  }

  while (true) {
    static int nWatchdog = 0;
    nWatchdog = ((nWatchdog>0) ? 0 : 1);

    cExt.eType = 106;
    cExt.nState = nWatchdog;

    write(fd, &cExt, sizeof(cExt));
    usleep(500 * 1000);
  }

  close(fd);
#endif

  return 0;
}
