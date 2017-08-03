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
#ifndef _WIN32
  system("killall watchdog");
  sleep(1);
  int fd;
  fd = open("/dev/watchdog", O_WRONLY);
  if (fd < 0) {
    printf("open error!!\n");
  }
  ioctl(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
  ioctl(fd, WDIOC_SETTIMEOUT, 6);

  while (1) {
    ioctl(fd, WDIOC_KEEPALIVE, 1);
    sleep(1);
  }
  close(fd);
#endif

  return 0;
}
