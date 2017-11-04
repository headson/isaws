/************************************************************************
* Author      : SoberPeng 2017-10-31
* Description :
************************************************************************/
#ifndef LIBCLIENT_HS_BASEDEFINE_H
#define LIBCLIENT_HS_BASEDEFINE_H

#include "vzbase/base/basictypes.h"

#define LEN_UUID    (32)                // UUID(�豸���š��ֻ���)
#define LEN_NAME    (32)                // ����(�û������豸��)
#define LEN_PWD     (32)                // ����

#define LEN_TIME    (20)                // ʱ��

#define LEN_HOST    (20)                // IP��ַ
#define LEN_MAC     (20)                // MAC��ַ

#define LEN_MD5     (36)                // MD5����

///��������////////////////////////////////////////////////////////////////////////////////
typedef enum {
  CODE_ID_G711A = 19,
  CODE_ID_JPEG  = 26,
  CODE_ID_AAC   = 37,
  CODE_ID_H264  = 96,
  CODE_ID_MJPEG = 1002,

  CODE_ID_FEOF  = 4444,  // �ļ�����
  CODE_ID_FACE  = 4446,  // ����ע��״̬
} E_CODE_ID;
#define EFRM_I      3       // I֡
#define EFRM_P      2       // P֡
#define EFRM_B      1       // B֡

//////////////////////////////////////////////////////////////////////////
#define _SYSTEM_STRING      ("thinkwatch_iSaw_term")

#define _SYS_UDP_PORT       (20002) // UDP ̽��˿�
#define _SYS_RTSP_PORT      (8554)  // RTSP ���ʶ˿�

#define _DEF_REPLAY_MINOR   0x123456    // �ط�
#define _DEF_RESEND_MINOR   0xabcdef    // �ش��������

#pragma pack(2)
///���ṹ����/////////////////////////////////////////////////////////////
const uint32_t HEAD_MARK = 257;  // ��ͷ

// ��ͷ
typedef struct TAG_PkgHead {
  uint32 nMark;      // 257
  int32  nPkgLen;    // ���ݳ���(��������ͷ)
  int32  eCmdMain;   // ����������
  int32  eCmdMinor;  // ����������
  TAG_PkgHead() {
    nMark = 0;
    nPkgLen = 0;
    eCmdMain = 0;
    eCmdMinor = 0;
  }
} TAG_PkgHead;
const uint32 HEAD_LENGTH = (uint32)sizeof(struct TAG_PkgHead);

struct TAG_PkgHeadReq : public TAG_PkgHead {
  char sid[LEN_UUID];  //
  char sdst[LEN_UUID];  //

  TAG_PkgHeadReq()
    : TAG_PkgHead() {
    memset(sid, 0, LEN_UUID);
    memset(sdst, 0, LEN_UUID);
  }
};
const uint32 HEAD_LEN_REQ = (int32)sizeof(struct TAG_PkgHeadReq);
/// �������ͷ
inline int32 PkgHeadReq(int8* pPacket, int32 eCmd, int32 nMinor,
                         const char* pData, uint32 nData,
                         const char* pSrc = NULL, const char* pDst = NULL) {
  if (!pPacket)
    return 0;

  TAG_PkgHeadReq* pHead = (TAG_PkgHeadReq*)pPacket;
  memset(pHead, 0, HEAD_LEN_REQ);
  pHead->nMark = HEAD_MARK;
  pHead->eCmdMain = eCmd;
  pHead->eCmdMinor = nMinor;
  pHead->nPkgLen = nData;

  if (pSrc && strlen(pSrc) < LEN_UUID)
    memcpy(pHead->sid, pSrc, LEN_UUID);

  if (pDst && strlen(pDst) < LEN_UUID)
    memcpy(pHead->sdst, pDst, LEN_UUID);

  if (pData)
    memcpy(pPacket + HEAD_LEN_REQ, pData, nData);

  return HEAD_LEN_REQ + nData;
}

/// ��ִ����ͷ
struct TAG_PkgHeadRet : public TAG_PkgHead {
  int32 nResult;

  TAG_PkgHeadRet()
    : TAG_PkgHead() {
    nResult = 0;
  }
};
const uint32 HEAD_LEN_RET = (int32)sizeof(struct TAG_PkgHeadRet);
inline int32 PkgHeadRet(int8* pPacket, int32 eCmd,
                         const char* pData, uint32 nData,
                         int32 nRet, int32 eMinor) {
  if (!pPacket)
    return 0;

  TAG_PkgHeadRet* pHead = (TAG_PkgHeadRet*)pPacket;
  memset(pHead, 0, HEAD_LEN_RET);
  pHead->nMark = HEAD_MARK;
  pHead->eCmdMain = eCmd;
  pHead->eCmdMinor = eMinor;
  pHead->nPkgLen = nData;
  pHead->nResult = nRet;

  if (pData)
    memcpy(pPacket + HEAD_LEN_RET, pData, nData);

  return HEAD_LEN_RET + nData;
}
//////////////////////////////////////////////////////////////////////////
// ��������ͷ
struct TAG_PkgHeadStream : public TAG_PkgHead {
  int32         nResult;

  uint16        nSeq;   // �ۼ�
  uint8         eFrm;   // ֡����
  uint8         nLast;  // ʣ���
  int64         nTime;  // ֡ʱ��(MS)

  uint16        eCode;  // E_CODE_ID;96=h264,19=G711A
  uint16        nChnx;  // channel
  uint16        nRvd1;  // sample-width
  uint16        nRvd2;  // bitrate-height

  TAG_PkgHeadStream()
    : TAG_PkgHead() {
    nResult = 0;

    nSeq = 0;
    eFrm = 0;
    nLast = 0;
    nTime = 0;

    eCode = 0;
    nChnx = 0;
    nRvd1 = 0;
    nRvd2 = 0;
  }
};
const uint32 HEAD_LEN_STR = HEAD_LENGTH + 4;
const uint32 STREAM_BEGIN = (uint32)sizeof(struct TAG_PkgHeadStream);
inline void PkgHeadStream(int8* pData, int32 eCmd, int32 nRet = 0) {
  if (!pData) {
    return;
  }
  TAG_PkgHeadStream* pHead = (TAG_PkgHeadStream*)pData;

  memset(pHead, 0, sizeof(TAG_PkgHeadStream));
  pHead->nMark = HEAD_MARK;
  pHead->eCmdMain = eCmd;
  pHead->eCmdMinor = 0x7c;
  pHead->nPkgLen = STREAM_BEGIN - HEAD_LEN_STR;

  pHead->nResult = nRet;

  pHead->nSeq = 0;
  pHead->eFrm = (uint8)3;
  pHead->nLast = (uint8)0;
  pHead->nTime = 0;//VTime::get_msec();
}

// ���������
enum E_COMMAND {
  //////////////////////////////////////////////////////////////////////////
  CMD_HEART_BAET = 2,                // ����
  CMD_DISCONNECT = 4,                // �Ͽ�����

  CMD_REG_CODE = 10,               // ��ȡע����

  CMD_USR_LOGIN = 14,               // �û���¼
  CMD_USR_VERIFY = 1012,             // �û���֤
  CMD_USR_P_VERIFY = 42,               // �û�Ȩ����֤
  CMD_ADMIN_VERIFY = 44,               // ����Ա��֤

  CMD_BIND_DEV = 16,               // �û����豸 
  CMD_SYNC_DEV = 500,              // UDP̽���ն�

  // ���������
  //////////////////////////////////////////////////////////////////////////
  CMD_SYS_GET = 1002,             // ϵͳ��ȡ 
  CMD_SYS_SET = 1004,             // ϵͳ����

  CMD_IVA_CTL = 1006,             // �㷨����
  CMD_IVA_STA = 1008,             // �㷨״̬

  CMD_TALK_REQ = 1010,             // �Խ�����

  CMD_USER_CMD = 8888,             // �û�����

  ///�ն�///////////////////////////////////////////////////////////////////
  CMD_TERM_LOGIN = 2002,             // ע���ն˵����������
  CMD_TERM_EVENT = 2004,             // ����
  CMD_PEOPLE_CNT = 2006,             // ����ͳ��

  CMD_TERM_LOGOUT = 4008,             // �ն˵���

  // ת��������
  CMD_DISP_LOGIN = 3002,             // ת��������ע��
  CMD_CHN_NOTIFY = 3006,             // ������Ƶת��ͨ��
  CMD_LINK_VDSRV = 3008,             // ������Ƶ���������

  CMD_ACCESS_REG = 4002,             // ���������ע��

  // ת��������
  CMD_STORE_LOGIN = 6002,             // �洢������ע��

  CMD_LOG_WEBSRV = 4006,             // ��־
  CMD_DOWN_FILE = 5000,             // �ļ�����

  // MAKE HOLE
  CMD_MAKE_HOLE = 5012,             // ��

  // Ԥ��
  CMD_STR_TICKET = 5010,             // ��ȡԤ����ַ
  CMD_STR_TRANS = 8000,             // ��ȡԤ������

  // �洢
  CMD_FILE_OPEN = 5020,             // �洢���ļ�
  CMD_FILE_CLOSE = 5022,             // �洢�ر��ļ�

  // �ѷ���
  CMD_FACE_EXPORT = 5024,             // ס����������
  CMD_FACE_IMPORT = 5026,             // ע����������

  CMD_FILE_TRANS = 6000,             // �����ļ�����
  CMD_FILE_INFO = 6002,             // �����ļ���Ϣ
  CMD_FILE_PACKET = 6004,             // �����ļ���

  //////////////////////////////////////////////////////////////////////////
  PKG_VIDEO_HEAD = 8002,             // ý��ͷ

  PKG_ENC_STREAM = 8004,             // ��������
  PKG_YUV_STREAM = 8006,             // ԭʼ����

  PKG_EVT_STREAM = 8008,             // ��������
  PKG_REC_STREAM = 8010,             // �洢����

  PKG_ADO_STREAM = 8012,             // ��Ƶ����

  PKG_TALK_STREAM = 9000,             // �Խ���Ƶ
};

enum E_IVA_CTL { // �㷨����
  CMD_MASTER_REQ = 10000,    // ס��ע������ 
  CMD_MASTER_LIST = 10002,    // ס���б����� 

  CMD_SYNC_EVENT = 10006,    // ͬ���¼��б�
  CMD_EVT_PROCESS = 10008,    // �¼��������� 

  CMD_USER_COMMAND = 10012,    // �㷨�û����� 
  CMD_LISTEN_RESULT = 10014,    // �������

  CMD_LIGHT_ADJUST = 10016,    // ������������
  CMD_LIGHT_MODIFY = 10018,    // ���������޸�

  CMD_CATCH_1_IMAGE = 10020,    // ��ȡһ�ż����Ƭ

  CMD_GET_WORK_SCHED = 10022,    // ��ȡ����ʱ��
  CMD_SET_WORK_SCHED = 10024,    // ���ù���ʱ��

  CMD_GET_EDIT_VOICE = 10026,    // ��ȡ�ɱ༭����
  CMD_SET_EDIT_VOICE = 10028,    // ���ÿɱ༭����
};

enum E_CMD_CFG {
  CMD_SET_REBOOT = 444,      // ��������
  CMD_SET_DEF_CFG = 555,      // �ָ�Ĭ������

  CMD_MAKE_CONFIG = 666,      // ��������

  CMD_SET_UPDATE = 777,      // HTTP����

  CMD_SYS_GET_DEV = 1000,     // ��ȡ�豸����
  CMD_SYS_SET_DEV = 1002,     // �����豸����

  CMD_SYS_GET_NET = 1004,     // ��ȡ�������
  CMD_SYS_SET_NET = 1006,     // �����������

  CMD_SYS_GET_VDO = 1008,
  CMD_SYS_SET_VDO = 1010,

  CMD_SYS_GET_ADO = 1012,
  CMD_SYS_SET_ADO = 1014,

  CMD_SYS_GET_EFS = 1016,
  CMD_SYS_SET_EFS = 1018,

  CMD_SYS_GET_USR = 1020,
  CMD_SYS_SET_USR = 1022,

  CMD_SYS_GET_IVA = 1024,
  CMD_SYS_SET_IVA = 1026,

  CMD_SYS_GET_SRV = 1028,
  CMD_SYS_SET_SRV = 1030,

  CMD_SET_TIMZONE = 1032,     // ����ʱ��
  CMD_REVISE_TIME = 1034,     // �ֶ�Уʱ

  CMD_SYS_GET_CODE = 1036,     // ��ȡ�豸����
  CMD_SYS_SET_CODE = 1038,     // �����豸����
  ///�㷨///////////////////////////////////////////
  CMD_GET_IVA_CFG = 2000,     // ��ȡ�㷨����
  CMD_SET_IVA_CFG = 2002,     // �����㷨����

  CMD_GET_AUX_CFG = 2004,     // ��ȡ�㷨����
  CMD_SET_AUX_CFG = 2006,     // �����㷨����

  CMD_SET_IVA_DEF = 2008,     // �㷨�ָ�Ĭ�ϲ���
  CMD_SET_AUX_DEF = 2010,     // �澯�ָ�Ĭ�ϲ���

  CMD_GET_EVT_EN = 2012,     // ��ȡ�澯�ƻ�
  CMD_SET_EVT_EN = 2014,     // ���ø澯�ƻ�
};


#define XML_ROOT    ("Message")
#define XML_HEAD    ("<?xml version=\"1.0\" encoding=\"GBK\"?>\r\n")

#endif  // LIBCLIENT_HS_BASEDEFINE_H
