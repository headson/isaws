/************************************************************************
*Author      : Sober.Peng 17-07-20
*Description :
************************************************************************/
#ifndef LIBVZLOGSERVER_CVZLOGMANAGE_H_
#define LIBVZLOGSERVER_CVZLOGMANAGE_H_


class CVzLogManage {
 protected:
  CVzLogManage();
  virtual ~CVzLogManage();

 public:
  static CVzLogManage *Instance();

 protected:

};

#endif  // LIBVZLOGSERVER_CVZLOGMANAGE_H_
