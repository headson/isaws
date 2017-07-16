/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

#define SW_VERSION_ "V100.00.00 "
#define HW_VERSION_ "V100.00.00 "

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
#define DEF_WEB_SRV_PORT      8000
#ifdef WIN32
#define DEF_WEB_SRV_PATH      "E:\\workspace\\git_work\\isaws\\src\\web"
#else
#define DEF_WEB_SRV_PATH      "mnt/etc/web"
#endif

#define DEF_MCAST_IP          "228.5.6.2"
#define DEF_MCAST_DEV_PORT    20003     // 设备监听端口,修改IP地址
#define DEF_MCAST_CLI_PORT    20004     // 客户端监听端口,获取设备信息

//////////////////////////////////////////////////////////////////////////
/*
请求的协议
{
  "cmd":"xxx",
  "id":xxx,
  "body": {
  }
}
cmd  字段表明消息的类型，由服务端与客户端约定，这是一个字符串，需要遵守命名规则。
id   字段是一个由客户端发起的字段，针对每一个请求，都必须有一个唯一的请求id，id只能够是一个数字，服务端会将id在回复中返回，如果一个请求没有id字段，服务端应该直接返回失败的命令。
body 是由客户端发送的具体命令的数据，根据不同的请求，有不同的`body`数据，原则上，`body`里面的内容不应该有超过3层的JSON嵌套。

回复的协议
{
  "cmd":"xxx",
  "id":xxx,
  "state":200,
  "body": {
  }
}
注意:所有字段都为小写
*/
#define MSG_CMD               "cmd"
#define MSG_ID                "id"
#define MSG_STATE             "state"
#define MSG_BODY              "body"

#define MSG_CMD_SIZE          32

typedef enum _MSG_RET {
  RET_SUCCESS       = 0,
  RET_FAILED        = -1000,
  RET_JSON_PARSE,
  RET_USERNAME,
  RET_PASSWORD,
} MSG_RET;
//////////////////////////////////////////////////////////////////////////

#define DP_SYS_CONFIG         "dp_sys_config"     // 系统参数

#define MSG_SYSC_GET_INFO     "get_devinfo"       // 获取设备信息
#define MSG_SYSC_SET_INFO     "set_devinfo"       // 设置设备信息
#define MSG_SYSC_ADDR_CHANGE  "addr_change"       // 地址改变

#define MSG_SYSC_HWCLOCK      "set_hwclock"       // 设置硬件时间
#define MSG_SYSC_TIME_CHANGE  "time_change"       // 时间改变

#define DP_NET_MODIFY         "dp_net_modify"     // 网络修改消息

//////////////////////////////////////////////////////////////////////////
#define DP_IVA_CONFIG         "dp_iva_config"     // 算法参数
#define MSG_IVAC_GET_INFO     "get_iva_info"      // 

#define DP_IVA_EVENT          "dp_iva_event"      // 算法事件

//////////////////////////////////////////////////////////////////////////
#define KVDB_KEY_USER         "key_user"     // kvdb获取用户
#endif  // VMESSAGE_H
