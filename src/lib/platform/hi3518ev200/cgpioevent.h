/************************************************************************
*Author      : Sober.Peng 17-09-16
*Description :
************************************************************************/
#ifndef LIBPLATFORM_CGPIOEVENT_H_
#define LIBPLATFORM_CGPIOEVENT_H_

class CGpioEvent {
 public:
  static void GpioInit();
  static void IRCutOpen();
  static void IRCutClose();

};

#endif  // LIBPLATFORM_CGPIOEVENT_H_
