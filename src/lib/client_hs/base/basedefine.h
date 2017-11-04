/************************************************************************
* Author      : SoberPeng 2017-10-31
* Description :
************************************************************************/
#ifndef LIBCLIENT_HS_BASEDEFINE_H
#define LIBCLIENT_HS_BASEDEFINE_H

#include "vzbase/base/basictypes.h"

#define LEN_UUID    (32)                // UUID(设备串号、手机号)
#define LEN_NAME    (32)                // 名字(用户名、设备名)
#define LEN_PWD     (32)                // 密码

#define LEN_TIME    (20)                // 时间

#define LEN_HOST    (20)                // IP地址
#define LEN_MAC     (20)                // MAC地址

#define LEN_MD5     (36)                // MD5长度

///编码类型////////////////////////////////////////////////////////////////////////////////
typedef enum {
  CODE_ID_G711A = 19,
  CODE_ID_JPEG  = 26,
  CODE_ID_AAC   = 37,
  CODE_ID_H264  = 96,
  CODE_ID_MJPEG = 1002,

  CODE_ID_FEOF  = 4444,  // 文件结束
  CODE_ID_FACE  = 4446,  // 人脸注册状态
} E_CODE_ID;
#define EFRM_I      3       // I帧
#define EFRM_P      2       // P帧
#define EFRM_B      1       // B帧

//////////////////////////////////////////////////////////////////////////
#define _SYSTEM_STRING      ("thinkwatch_iSaw_term")

#define _SYS_UDP_PORT       (20002) // UDP 探测端口
#define _SYS_RTSP_PORT      (8554)  // RTSP 访问端口

#define _DEF_REPLAY_MINOR   0x123456    // 回放
#define _DEF_RESEND_MINOR   0xabcdef    // 重传次命令号

#pragma pack(2)
///包结构定义/////////////////////////////////////////////////////////////
const uint32_t HEAD_MARK = 257;  // 包头

// 包头
typedef struct TAG_PkgHead {
  uint32 nMark;      // 257
  int32  nPkgLen;    // 数据长度(不包括包头)
  int32  eCmdMain;   // 主命令类型
  int32  eCmdMinor;  // 次命令类型
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
/// 请求包包头
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

/// 回执包包头
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
// 数据流包头
struct TAG_PkgHeadStream : public TAG_PkgHead {
  int32         nResult;

  uint16        nSeq;   // 累计
  uint8         eFrm;   // 帧类型
  uint8         nLast;  // 剩余包
  int64         nTime;  // 帧时间(MS)

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

// 命令次类型
enum E_COMMAND {
  //////////////////////////////////////////////////////////////////////////
  CMD_HEART_BAET = 2,                // 心跳
  CMD_DISCONNECT = 4,                // 断开连接

  CMD_REG_CODE = 10,               // 获取注册码

  CMD_USR_LOGIN = 14,               // 用户登录
  CMD_USR_VERIFY = 1012,             // 用户验证
  CMD_USR_P_VERIFY = 42,               // 用户权限验证
  CMD_ADMIN_VERIFY = 44,               // 管理员验证

  CMD_BIND_DEV = 16,               // 用户绑定设备 
  CMD_SYNC_DEV = 500,              // UDP探测终端

  // 接入服务器
  //////////////////////////////////////////////////////////////////////////
  CMD_SYS_GET = 1002,             // 系统获取 
  CMD_SYS_SET = 1004,             // 系统配置

  CMD_IVA_CTL = 1006,             // 算法控制
  CMD_IVA_STA = 1008,             // 算法状态

  CMD_TALK_REQ = 1010,             // 对讲请求

  CMD_USER_CMD = 8888,             // 用户命令

  ///终端///////////////////////////////////////////////////////////////////
  CMD_TERM_LOGIN = 2002,             // 注册终端到接入服务器
  CMD_TERM_EVENT = 2004,             // 报警
  CMD_PEOPLE_CNT = 2006,             // 人数统计

  CMD_TERM_LOGOUT = 4008,             // 终端掉线

  // 转发服务器
  CMD_DISP_LOGIN = 3002,             // 转发服务器注册
  CMD_CHN_NOTIFY = 3006,             // 创建视频转发通道
  CMD_LINK_VDSRV = 3008,             // 连接视频服务服务器

  CMD_ACCESS_REG = 4002,             // 接入服务器注册

  // 转发服务器
  CMD_STORE_LOGIN = 6002,             // 存储服务器注册

  CMD_LOG_WEBSRV = 4006,             // 日志
  CMD_DOWN_FILE = 5000,             // 文件更新

  // MAKE HOLE
  CMD_MAKE_HOLE = 5012,             // 打洞

  // 预览
  CMD_STR_TICKET = 5010,             // 获取预览地址
  CMD_STR_TRANS = 8000,             // 获取预览数据

  // 存储
  CMD_FILE_OPEN = 5020,             // 存储打开文件
  CMD_FILE_CLOSE = 5022,             // 存储关闭文件

  // 已废弃
  CMD_FACE_EXPORT = 5024,             // 住户人脸导出
  CMD_FACE_IMPORT = 5026,             // 注册人脸导入

  CMD_FILE_TRANS = 6000,             // 传输文件请求
  CMD_FILE_INFO = 6002,             // 传输文件信息
  CMD_FILE_PACKET = 6004,             // 传输文件包

  //////////////////////////////////////////////////////////////////////////
  PKG_VIDEO_HEAD = 8002,             // 媒体头

  PKG_ENC_STREAM = 8004,             // 编码数据
  PKG_YUV_STREAM = 8006,             // 原始数据

  PKG_EVT_STREAM = 8008,             // 报警数据
  PKG_REC_STREAM = 8010,             // 存储数据

  PKG_ADO_STREAM = 8012,             // 音频数据

  PKG_TALK_STREAM = 9000,             // 对讲音频
};

enum E_IVA_CTL { // 算法控制
  CMD_MASTER_REQ = 10000,    // 住户注册请求 
  CMD_MASTER_LIST = 10002,    // 住户列表请求 

  CMD_SYNC_EVENT = 10006,    // 同步事件列表
  CMD_EVT_PROCESS = 10008,    // 事件处理请求 

  CMD_USER_COMMAND = 10012,    // 算法用户命令 
  CMD_LISTEN_RESULT = 10014,    // 结果监听

  CMD_LIGHT_ADJUST = 10016,    // 环境照明调整
  CMD_LIGHT_MODIFY = 10018,    // 环境照明修改

  CMD_CATCH_1_IMAGE = 10020,    // 获取一张监控照片

  CMD_GET_WORK_SCHED = 10022,    // 获取工作时段
  CMD_SET_WORK_SCHED = 10024,    // 设置工作时段

  CMD_GET_EDIT_VOICE = 10026,    // 获取可编辑语音
  CMD_SET_EDIT_VOICE = 10028,    // 设置可编辑语音
};

enum E_CMD_CFG {
  CMD_SET_REBOOT = 444,      // 机器重启
  CMD_SET_DEF_CFG = 555,      // 恢复默认配置

  CMD_MAKE_CONFIG = 666,      // 生产配置

  CMD_SET_UPDATE = 777,      // HTTP升级

  CMD_SYS_GET_DEV = 1000,     // 获取设备参数
  CMD_SYS_SET_DEV = 1002,     // 配置设备参数

  CMD_SYS_GET_NET = 1004,     // 获取网络参数
  CMD_SYS_SET_NET = 1006,     // 配置网络参数

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

  CMD_SET_TIMZONE = 1032,     // 设置时区
  CMD_REVISE_TIME = 1034,     // 手动校时

  CMD_SYS_GET_CODE = 1036,     // 获取设备密码
  CMD_SYS_SET_CODE = 1038,     // 配置设备密码
  ///算法///////////////////////////////////////////
  CMD_GET_IVA_CFG = 2000,     // 获取算法参数
  CMD_SET_IVA_CFG = 2002,     // 配置算法参数

  CMD_GET_AUX_CFG = 2004,     // 获取算法参数
  CMD_SET_AUX_CFG = 2006,     // 配置算法参数

  CMD_SET_IVA_DEF = 2008,     // 算法恢复默认参数
  CMD_SET_AUX_DEF = 2010,     // 告警恢复默认参数

  CMD_GET_EVT_EN = 2012,     // 获取告警计划
  CMD_SET_EVT_EN = 2014,     // 配置告警计划
};


#define XML_ROOT    ("Message")
#define XML_HEAD    ("<?xml version=\"1.0\" encoding=\"GBK\"?>\r\n")

#endif  // LIBCLIENT_HS_BASEDEFINE_H
