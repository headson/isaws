/************************************************************************
*Author      : Sober.Peng 17-09-16
*Description :
************************************************************************/
#ifndef LIBPLATFORM_BASEDEFINES_H_
#define LIBPLATFORM_BASEDEFINES_H_

typedef enum EXT_DEV_TYPE {
  IO_IRCUT = 1,        // IRCUT
  IO_IR_LED = 2,       // 红外补光
};

typedef struct {
  int etype;       // IO类型
  int ntimes;      // 循环次数
  int nstate0;     // 状态1
  int nduration0;  // 持续时长1
  int nstate1;     // 状态1
  int nduration1;  // 持续时长1
} TAG_GPIO;

class CDeviceInterface {
 public:
  CDeviceInterface(int etype) { }
  virtual ~CDeviceInterface() { }

  int Initinal();
  
  int Write(const void *ptag);
  int Read(const void *ptag);

  int Status();

 protected:
  int etype_;
};

#define ALG_EVT_OUT_TIMET     "evt_out_timet"
#define ALG_POSITIVE_NUMBER   "positive_number"
#define ALG_NEGATIVE_NUMBER   "negative_number"

#endif  // LIBPLATFORM_BASEDEFINES_H_
