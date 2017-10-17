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

#include "vzlogging/logging/vzlogging.h"

#ifdef HISI_R
#include "hisi_dog/vzhardwaredog.h"

void FeedHardwareDog() {
  vzlog::PWMWatchDog::FeedPwmWatchDog();
}
#else
void FeedHardwareDog() {
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
}
#endif

int main(int argc, char *argv[]) {
  if (argc >= 2 && argv[1][0] == 's') {
    InitVzLogging(argc, argv);
#ifdef _WIN32
    ShowVzLoggingAlways();
#endif

    LOG_RESET("reset all process timeout.");
    LOG_RESET("reset all process timeout.");
  } else {
    system("killall watchdog");
    //VZ_ERROR("killall watchdog");
    // replace watchdog to feed hardware
    while (true) {
      FeedHardwareDog();
#ifdef _WIN32
#else
      sleep(1);
#endif
    }
  }
  return 0;
}
