/************************************************************************
*Author      : Sober.Peng 17-09-16
*Description :
************************************************************************/
#ifndef LIBPLATFORM_BASEDEFINES_H_
#define LIBPLATFORM_BASEDEFINES_H_

typedef enum EXT_DEV_TYPE {
  IO_IRCUT = 1,        // IRCUT
  IO_IR_LED = 2,       // ���ⲹ��
};

typedef struct {
  int etype;       // IO����
  int ntimes;      // ѭ������
  int nstate0;     // ״̬1
  int nduration0;  // ����ʱ��1
  int nstate1;     // ״̬1
  int nduration1;  // ����ʱ��1
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
