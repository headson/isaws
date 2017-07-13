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

#include "json/json.h"

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

 signals:
  void UpdateDevice();

 protected:
  void run();

 protected:
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
  void OnGetDevInfo(Json::Value &jroot);

 protected:
  bool                   running_;

 protected:
  QMutex                        mutex_;
  QMap<std::string, CDevInfo*>  devs_info_; // dev_id dev_info

 protected:
  vzconn::CMCastSocket         *mcast_sock_;
};


#endif  // PC_MANAGE_CDEVICEDETECT_H
