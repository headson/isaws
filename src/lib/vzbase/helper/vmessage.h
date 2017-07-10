/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

#define DEF_SHM_VIDEO_0       "/dev/shm/video_0"
#define DEF_SHM_VIDEO_0_SIZE  ((352*288*3)/2 + 1024)

#define DEF_SHM_AUDIO_0       "/dev/shm/audio_0"
#define DEF_SHM_AUDIO_0_SIZE  (1024)

// DpClient_Init设置dispatcher_server dp地址
#define DEF_DP_SRV_IP         "127.0.0.1"
#define DEF_DP_SRV_PORT       5291

// KvdbClient_Init设置dispatcher_server kvdb地址
#define DEF_KVDB_SRV_IP       "127.0.0.1"
#define DEF_KVDB_SRV_PORT     5299

// WEB Server 监听端口和网页存储路径
#define DEF_WEB_SRV_PORT      "8000"
#define DEF_WEB_SRV_PATH      "c:/tools/web"

#define DEF_MCAST_IP          "228.5.6.2"
#define DEF_MCAST_DEV_PORT    20003     // 设备监听端口,修改IP地址
#define DEF_MCAST_CLI_PORT    20004     // 客户端监听端口,获取设备信息

//////////////////////////////////////////////////////////////////////////
/*
请求的协议
{
  "type":"set_xxx",
  "body":{
    ......
  }
}

回复的协议
{
  "type":"set_xxx",
  "state":200,
  "err_msg":"all done",
  "body":{
    ......
  }
}
body:内部是各种不同的协议的不同定义
type:是必须值，请求和回复的都应该携带这个值
state:回复的状态码，在正常的情况下为200，也有一个额外的定义。具体与http的错误码相似。
err_msg:在state的值不为200的情况下，应该将error_msg的值赋值，以便用户能够更好的明白错误在什么地方。
          错误描述里面，只能够用英文，不能够用中文。

注意:所有字段都为小写
*/
#define MSG_TYPE              "type"
#define MSG_STATE             "state"
#define MSG_ERR_MSG           "err_msg"
#define MSG_BODY              "body"

#define MSG_TYPE_MAX          32

//////////////////////////////////////////////////////////////////////////

#define DP_SYS_CONFIG         "dp_sys_config"   // 系统参数

#define MSG_SYSC_GET_DEVINFO  "get_devinfo"     // 获取设备信息
#define MSG_SYSC_SET_DEVINFO  "set_devinfo"     // 设置设备信息

#define MSG_SYSC_SET_HWCLOCK  "set_hwclock"     // 设置硬件时间

#define DP_NET_MODIFY         "dp_net_modify"   // 网络修改消息

//////////////////////////////////////////////////////////////////////////
#define DP_IVA_CONFIG         "dp_iva_config"   // 算法参数

#define DP_IVA_EVENT          "dp_iva_event"    // 算法事件

//////////////////////////////////////////////////////////////////////////
#define KVDB_
#endif  // VMESSAGE_H
