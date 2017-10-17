#ifdef HISI_R

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "vzhardwaredog.h"

namespace vzlog {

PWMWatchDog::PWM_DOG_FD PWMWatchDog::pwm_dog_fd_ = -1;
char PWMWatchDog::DEV_DES_NAME[16] = "/dev/pwmWDog";

bool PWMWatchDog::GetDogState() {
  return pwm_dog_fd_ < 0 ? false:true;
}

bool PWMWatchDog::OpenWatchDogFd() {
  if(pwm_dog_fd_ < 0) {
    pwm_dog_fd_ = open(DEV_DES_NAME, O_WRONLY);
    if(pwm_dog_fd_ < 0) {
      return false;
    }
  }
  return true;
}

bool PWMWatchDog::StartPwmWatchDog() {
  bool ret = OpenWatchDogFd();
  if(ret) {
    int res = ioctl(pwm_dog_fd_, PWM_WDOG_START, INTERAL_FEED_TIME);
    if(res != 0) {
      ret = false;
    }
  }
  return ret;
}

void PWMWatchDog::ClosePwmDog() {
  if(OpenWatchDogFd()) {
    int res = ioctl(pwm_dog_fd_, PWM_WDOG_STOP);
    if(res == 0) {
      close(pwm_dog_fd_);
      pwm_dog_fd_ = -1;
    }
  }
}

bool PWMWatchDog::FeedPwmWatchDog() {
  bool ret = OpenWatchDogFd();
  if(ret) {
    int res = ioctl(pwm_dog_fd_, PWM_WDOG_FEED);
    if(res != 0) {
      ret = false;
    }
  }
  return ret;
}

}  // namespace vzlog

#endif  // HISI_R