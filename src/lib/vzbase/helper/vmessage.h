/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

#define DEF_SHM_VIDEO_0       "/dev/shm/video_0"
#define DEF_SHM_VIDEO_0_SIZE  ((352*288*3)/2)

#define DEF_SHM_AUDIO_0       "/dev/shm/audio_0"
#define DEF_SHM_AUDIO_0_SIZE  1024

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
#define MSG_SYS_PARAM         "msg_sys_param"     // 系统参数

#define MSG_IVA_PARAM         "msg_iva_param"     // 算法参数
#define MSG_IVA_EVENT         "msg_iva_event"     // 算法事件



#endif  // VMESSAGE_H
