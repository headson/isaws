/************************************************************************
*Author      : Sober.Peng 17-07-13
*Description :
************************************************************************/
#ifndef PC_MANAGE_CDEVICEDETECT_H
#define PC_MANAGE_CDEVICEDETECT_H

#include "appinc.h"

#include <QMutex>
#include <QThread>
#include <QMutexLocker>

#include "vzconn/multicast/cmcastsocket.h"

class CDeviceDetect : public QThread,
  public vzconn::CClientInterface {
  Q_OBJECT

 public:
  CDeviceDetect(QObject *parent=NULL);
  virtual ~CDeviceDetect();

 public:
  static CDeviceDetect *Instance();

  bool Start();
  bool Detect();

 protected:
  void run();

  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8      *p_data,
                                 uint32            n_data,
                                 uint16            n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }

  virtual void  HandleClose(vzconn::VSocket *p_cli) {
  }

 protected:
  bool                   running_;

 protected:
  QMutex                 mutex_;
  QList<CDevInfo*>       devs_info_;

 protected:
  vzconn::CMCastSocket  *mcast_sock_;
};


#endif  // PC_MANAGE_CDEVICEDETECT_H
