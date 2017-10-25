/************************************************************************/
/* Author      : SoberPeng 2017-07-26
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "alg_hs/basedefine.h"

#include "json/json.h"
#include "vzbase/base/vmessage.h"
#include "vzbase/helper/stdafx.h"

void alg::CListenMessage::AlgActionCallback(IVA_ACTION_OUTPUT *pAction) {
  Json::Value jroot;

  switch(pAction->action_code) {
  case LOCAL_ACTION_VIDEO_START_SAVING: { // 环境和门户摄像机开始保存视频
    jroot[MSG_CMD] = "";
    LOG_INFO("record video and audio start %u-%d ", pAction->unique_code, pAction->event_type);
  }
  break;

  case LOCAL_ACTION_VIDEO_STOP_SAVING: {  // 环境和门户摄像机停止保存视频
    LOG_INFO("record video and audio stop %u-%d", pAction->unique_code, pAction->event_type);
  }
  break;

  case LOCAL_ACTION_TURN_ON_LIGHT:                // 开启照明
  case LOCAL_ACTION_TURN_OFF_LIGHT: {             // 关闭照明
#ifdef IMX6Q
    TAG_EXT_CTRL cLight= {0};   // 默认关闭
    cLight.eDev  = E_ZhaoMing;
    if (LOCAL_ACTION_TURN_ON_LIGHT == pAction->action_code) {
      cLight.nState = 1;
      LOG_INFO("开起照明 %d-%d-%d ", cLight.eDev, cLight.nState, cLight.nValue);
    } else if (LOCAL_ACTION_TURN_OFF_LIGHT == pAction->action_code) {
      cLight.nState = 0;
      LOG_INFO("关闭照明 %d-%d-%d ", cLight.eDev, cLight.nState, cLight.nValue);
    }
    cLight.nValue = m_cIva.nLight;
    if (PLATFORM()->get_ext_dev())
      PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cLight);
#endif
  }
  break;

  // case LOCAL_ACTION_TURN_ON_IR_LIGHT:
  case LOCAL_ACTION_ADJUST_IR_LIGHT_INTENSITY:        // 开启红外照明
  case LOCAL_ACTION_TURN_OFF_IR_LIGHT: {              // 关闭红外照明
#ifdef IMX6Q
    TAG_EXT_CTRL cLight= {0};
    cLight.eDev  = E_Infrared;
    if (LOCAL_ACTION_ADJUST_IR_LIGHT_INTENSITY == pAction->action_code) {
      cLight.nState = 1;
      LOG_INFO("开启红外照明 %d-%d-%d ", cLight.eDev, cLight.nState, cLight.nValue);
    } else if(LOCAL_ACTION_TURN_OFF_IR_LIGHT == pAction->action_code) {
      cLight.nState = 0;
      LOG_INFO("关闭红外照明 %d-%d-%d ", cLight.eDev, cLight.nState, cLight.nValue);
    }

    cLight.nValue = m_cIva.nInfrared * pAction->gain / DEF_MAX_GAIN;
    if (PLATFORM()->get_ext_dev())
      PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cLight);
#endif
  }
  break;

  case LOCAL_ACTION_UNLOCK_STOP_ALARM: {          // 开锁异常停止报警
  }
  break;

  case LOCAL_ACTION_ADJUST_DOOR_CAMERA_EXPOSUER: { // 调整门户相机曝光时间
    int32_t nCurrent = 0;
    int32_t nChn = FRAME_TYPE_DOOR_VIDEO;
    if (PLATFORM()->get_video(nChn)) {
      if (pAction->exposure_time <= DEF_MAX_EXP) {
        nCurrent = (m_cIva.cArg[nChn].nMaxExp-m_cIva.cArg[nChn].nMinExp)
                   * pAction->exposure_time / DEF_MAX_EXP + m_cIva.cArg[nChn].nMinExp;

        if (nCurrent > m_cIva.cArg[nChn].nMaxExp)
          nCurrent = m_cIva.cArg[nChn].nMaxExp;
        if (nCurrent < m_cIva.cArg[nChn].nMinExp)
          nCurrent = m_cIva.cArg[nChn].nMinExp;

        /*PLATFORM()->get_video(nChn)->SetISPValue(0x3502, (nCurrent & 0xff));
        PLATFORM()->get_video(nChn)->SetISPValue(0x3501, ((nCurrent & 0xff00) >> 8));*/
        PLATFORM()->get_video(nChn)->SetISPValue(1, nCurrent);
      }

      if (pAction->gain <= DEF_MAX_GAIN) {
        nCurrent = m_cIva.cArg[nChn].nMaxGain * pAction->gain / DEF_MAX_GAIN;

        PLATFORM()->get_video(nChn)->SetISPValue(2, nCurrent);
        /*PLATFORM()->get_video(nChn)->SetISPValue(0x350b, (nCurrent & 0xff));
        PLATFORM()->get_video(nChn)->SetISPValue(0x350a, ((nCurrent & 0xff00) >> 8));*/
      }
    }
    LOG_INFO("调整门户相机曝光时间 exp %d, gain %d, nCurrent %d.", pAction->exposure_time, pAction->gain, nCurrent);
  }
  break;

  case LOCAL_ACTION_ADJUST_FACE_CAMERA_EXPOSUER: { // 调整人像相机曝光时间
    int32_t nCurrent = 0;
    int32_t nChn = FRAME_TYPE_FACE_VIDEO;
    if (PLATFORM()->get_video(nChn)) {
      if (pAction->exposure_time <= DEF_MAX_EXP) {
        nCurrent = (m_cIva.cArg[nChn].nMaxExp-m_cIva.cArg[nChn].nMinExp)
                   * pAction->exposure_time / DEF_MAX_EXP + m_cIva.cArg[nChn].nMinExp;

        if (nCurrent > m_cIva.cArg[nChn].nMaxExp)
          nCurrent = m_cIva.cArg[nChn].nMaxExp;
        if (nCurrent < m_cIva.cArg[nChn].nMinExp)
          nCurrent = m_cIva.cArg[nChn].nMinExp;

        //PLATFORM()->get_video(nChn)->SetISPValue(0x3502, (nCurrent & 0xff));
        //PLATFORM()->get_video(nChn)->SetISPValue(0x3501, ((nCurrent & 0xff00) >> 8));
        PLATFORM()->get_video(nChn)->SetISPValue(1, nCurrent);
      }

      if (pAction->gain <= DEF_MAX_GAIN) {
        nCurrent = m_cIva.cArg[nChn].nMaxGain * pAction->gain / DEF_MAX_GAIN;

        PLATFORM()->get_video(nChn)->SetISPValue(2, nCurrent);
        //PLATFORM()->get_video(nChn)->SetISPValue(0x350b, (nCurrent & 0xff));
        //PLATFORM()->get_video(nChn)->SetISPValue(0x350a, ((nCurrent & 0xff00) >> 8));
      }
    }
    LOG_INFO("调整人像相机曝光时间 exp %d, gain %d.", pAction->exposure_time, pAction->gain);
  }
  break;

  case LOCAL_ACTION_ADJUST_LIGHT_INTENSITY: { // 调整环境照明强度
#ifdef IMX6Q
    TAG_EXT_CTRL cLight= {0};
    cLight.eDev   = E_ZhaoMing;
    cLight.nState = 1;
    cLight.nValue = m_cIva.nLight * pAction->gain/DEF_MAX_GAIN;

    if (PLATFORM()->get_ext_dev())
      PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cLight);

    LOG_INFO("调整环境照明强度 %d-%d-%d; light %d gain %d",
             cLight.eDev, cLight.nState, cLight.nValue,
             m_cIva.nLight, pAction->gain);
#endif
  }
  break;

  case LOCAL_ACTION_PLAY_SOUND: {             // 播放音频
    VoiceProcess(pAction);

    LOG_INFO("播放音频 %d - speaker %d ", pAction->event_type-IVA_PLAY_SOUND_EVENT_START, m_cVoice.bEnable);
  }
  break;

  case LOCAL_ACTION_FACE_ENROLLMENT_CANCLLING: { // 取消人脸注册
    LOG_INFO("取消人脸注册.");
  }
  break;

  case LOCAL_ACTION_HEART_BEAT_RESPONDENCE: { //心跳回应
    inc_living();

    if (pAction->speech != NULL) {
      memcpy(g_app.nAIState, pAction->speech, 5);
    }

    // 系统状态灯
    static uint32_t nState = 0;
    nState = nState>0 ? 0 : 1;

    TAG_EXT_CTRL cLight= {0};
#ifdef IMX6Q
#if (HARD_VER == DEV_TYPE_SW1)
    cLight.eDev   = E_WriteLED3;
#else
    cLight.eDev   = E_WriteLED1;
#endif
#endif
    cLight.nState = nState;
    if (PLATFORM()->get_ext_dev()) {
      PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cLight);
    }

    // 发送人数统计
    //UDP_SRV()->Notify();
  }
  break;

  case LOCAL_ACTION_PLAY_EDITABLE_SOUND: {    // 播放可编辑声音
    if (PLATFORM()->get_speech())
      PLATFORM()->get_speech()->Decode((int8_t*)pAction->speech, strlen((char*)pAction->speech));
    LOG_INFO("播放可编辑的音频 string %s-%d.", pAction->speech, strlen((char*)pAction->speech));
  }
  break;

//    case LOCAL_ACTION_TURN_ON_LASER_LIGHT:      // 开启激光
//    case LOCAL_ACTION_TURN_OFF_LASER_LIGHT:     // 关闭激光
//        {
//            TAG_EXT_CTRL cLight={0};
//#ifdef IMX6Q
//            cLight.eDev  = E_LaserLight;
//#endif
//            if (pAction->action_code == LOCAL_ACTION_TURN_ON_LASER_LIGHT)
//            {
//                cLight.nState = 1;
//                LOG_INFO("开启激光 %d %d.", cLight.eDev, cLight.nState);
//            }
//            else
//            {
//                cLight.nState = 0;
//                LOG_INFO("关闭激光 %d %d.", cLight.eDev, cLight.nState);
//            }
//            if (m_cIva.nMode <= 4) // 考勤
//            {
//                if (PLATFORM()->get_ext_dev())
//                    PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cLight);
//            }
//        }
//        break;

  case LOCAL_ACTION_TURN_ON_DOOR_GUARD:       // 打开门禁开关
  case LOCAL_ACTION_TURN_OFF_DOOR_GUARD: {    // 关闭门禁开关
    static TAG_CfgExt cExt = {-1};
    if (cExt.nDefDoor == -1) {
      CONFIG()->GetExt(cExt);
    }

    TAG_EXT_CTRL cSwitch= {0};
#ifdef IMX6Q
    if ((pAction->command & 0x1))
      cSwitch.eDev  = E_WriteSwitch1;
    else if (pAction->command & 0x2)
      cSwitch.eDev  = E_WriteSwitch2;
#endif
    if(LOCAL_ACTION_TURN_ON_DOOR_GUARD == pAction->action_code) {

#ifdef IMX6Q
      cSwitch.nState = 0;
      if (cSwitch.eDev == E_WriteSwitch1
          && PLATFORM()->get_ext_dev())
        PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cSwitch);

      VThread::msleep(30);
      cSwitch.nState = 1;
      if (cSwitch.eDev == E_WriteSwitch1
          && PLATFORM()->get_ext_dev())
        PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cSwitch);
#endif

      open_door();

      cSwitch.nState = ((cExt.nDefDoor > 0) ? 1: 0);    // 0
      LOG_INFO("打开门禁开关.");
    } else if(LOCAL_ACTION_TURN_OFF_DOOR_GUARD == pAction->action_code) {
      cSwitch.nState = ((cExt.nDefDoor > 0) ? 0: 1);    // 0
      LOG_INFO("关闭门禁开关.");
    }

#ifdef IMX6Q
    if (cSwitch.eDev  == E_WriteSwitch2
        && PLATFORM()->get_ext_dev())
      PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cSwitch);
#endif
  }
  break;

  case LOCAL_ACTION_SET_MOUNTING_HEIGHT: {    // 设置安装高度
    g_app.nInstallHeight = pAction->command;

    uint16_t nChn = FRAME_TYPE_FACE_VIDEO;
    if (PLATFORM()->get_video(nChn)) {
      PLATFORM()->get_video(nChn)->SetISPValue(-1, pAction->command); // 安装高度
    }
    logi("set install height %d.", g_app.nInstallHeight);
  }
  break;

  case LOCAL_ACTION_RESTART_DEVICE: {                                         // 重启设备
    loge("-------------- IVA need reboot device.%d.", pAction->action_code);

    //applet_reboot();
  }
  break;

  case LOCAL_ACTION_CHANGE_TO_DOOR_RECORDING: //切换为门户录像
  case LOCAL_ACTION_CHANGE_TO_ENV_RECORDING: { //切换为环境录像
    int32_t nNewChn = VI_0; // 切换为门户录像
    if(pAction->action_code == LOCAL_ACTION_CHANGE_TO_ENV_RECORDING) {
      nNewChn = VI_2;     // 切换为环境录像
    }
    m_cRecord.ExRecordChannel(nNewChn);
    LOG_INFO("change the save channel new %d.", nNewChn);
  }
  break;

  case LOCAL_ACTION_TURN_ON_INDICATOR_LIGHT:      //开启指示灯
  case LOCAL_ACTION_TURN_OFF_INDICATOR_LIGHT: {   //关闭指示灯
    TAG_EXT_CTRL cLight= {0};
#ifdef IMX6Q
    cLight.eDev  = E_WPilotLamp;
#endif
    if (pAction->action_code == LOCAL_ACTION_TURN_ON_INDICATOR_LIGHT) {
      cLight.nState = 0;
      LOG_INFO("开启指示灯 %d %d %d.", cLight.eDev, cLight.nState, m_cIva.nMode);
    } else {
      cLight.nState = 1;
      LOG_INFO("关闭指示灯 %d %d %d.", cLight.eDev, cLight.nState, m_cIva.nMode);
    }
    if (m_cIva.nMode > 4) {
      if (PLATFORM()->get_ext_dev()) {
        PLATFORM()->get_ext_dev()->Ioctrl(EXT_WRITE, &cLight);
      }
    }
  }
  break;

  case LOCAL_ACTION_RESET_NETWORK: {                                // 固定IP时设置默认IP
    TAG_CfgNet cNet;
    CONFIG()->GetNet(cNet);
    if (cNet.bDhcp == false) {
      const char sCmd[] = "ifconfig eth0 192.168.254.254 netmask 255.255.255.0;" \
                          "route del default;"
                          "route add default gateway 192.168.254.1 eth0";

      VThread::run_cmd(sCmd);
      logi("system \t%s\n", sCmd);
    }
    LOG_INFO("固定IP时设置默认IP.");
  }
  break;
  case LOCAL_ACTION_RESET_DOOR_SENSOR: {                                      // 图像漆黑
    if (PLATFORM()->get_video(VI_0)) {
      PLATFORM()->get_video(VI_0)->ResetCamera();
    }
    loge("a camera of door need reset by iva.");
  }
  break;
  ///远程动作指令(输出接口)///////////////////////////////////////////////////////////////////////
  case REMOTE_ACTION_FACE_ENROLLMENT_START:       // 人脸注册开始
  case REMOTE_ACTION_FACE_ENROLLMENT_SUCCESS:     // 人脸注册成功
  case REMOTE_ACTION_FACE_ENROLLMENT_TIMEDOUT: {  // 人脸注册超时
    // 通过转发服务器发送状态
    for (int32_t i = 0; i < 2; i++) {
      if (PLATFORM()->get_video(VI_1)) {
        TAG_PkgHeadStream cHead;
        PkgHeadStream((int8_t*)&cHead, PKG_ENC_STREAM, 0);

        cHead.eCode = CODE_ID_FACE;
        cHead.nRvd1 = (uint16_t)pAction->command;
        cHead.nChnx = (uint16_t)pAction->action_code;

        PLATFORM()->get_video(VI_1)->SendUserData((int8_t*)&cHead, sizeof(cHead));
      }
    }
    LOG_INFO("人脸注册 %d-%d.", pAction->command, pAction->action_code);
  }
  break;

  case REMOTE_ACTION_CONNECTING_VIDEO_AND_AUDIO:      //开启音视频连线
  case REMOTE_ACTION_DISCONNECTING_VIDEO_AND_AUDIO: { //结束音视频连线
    /*TAG_CfgUsr cUsr;
    CFG_HDL()->ReadUsr(cUsr);

    CCmdIvaState cState;
    cState.sUsrId = "";
    cState.sCaptainId = cUsr.sId;
    cState.nCommand = pAction->command;
    cState.nState = IVA_BASIC_EVENT_VIDEO_AND_AUDIO_CONNECTION;
                    IVA_BASIC_EVENT_VIDEO_AND_AUDIO_DISCONNECTION
    string sXml = "";
    if (cState.Encoder(sXml))
    {
    CNetClient* pSock = (CNetClient*)pAction->address;
    if (pSock) {
    int8_t sPacket[4*SZ_1K];
    int32_t nRet = PkgHeadReq(sPacket, CMD_IVA_STA, 0, sXml.c_str(), sXml.size());
    VSubject::Notify(CMD_IVA_STA, sPacket, nRet, NULL);
    }
    }*/
    LOG_INFO("开启音视频连线.");
  }
  break;

  case REMOTE_ACTION_ALARM_MODE_SYNC: {        // 报警模式同步
    LOG_INFO("报警模式同步.");
    //string sXml = "";
    //if (pEngine->GetAux(sXml, true) == RET_SUCCESS)
    //{
    //    TAG_CfgSys cSys; CFG_HDL()->ReadSystem(cSys);
    //    TAG_CfgSrv cSrv; CFG_HDL()->ReadServer(cSrv);

    //    string sNaming = cSys.sDevId; sNaming += ":";
    //    sNaming += cSrv.sHost; sNaming += ":"; sNaming += cSys.sDevId;

    //    CMarkupSTL cXml;
    //    cXml.SetDoc(sXml.c_str());
    //    if (!cXml.IsWellFormed())
    //        break;

    //    if (cXml.FindElem())
    //    {
    //        cXml.AddAttrib("isSync", "true");   // 设备同步
    //        cXml.AddAttrib("Naming",  sNaming.c_str());
    //        cXml.AddAttrib("CtlType", CMD_SET_IVA_AUX);
    //    }
    //    sXml = cXml.GetDoc();

    //    int8_t  sPacket[4*SZ_1K] = {0};
    //    int32_t nPacket = PkgHeadReq(sPacket, CMD_SET_CFG, 0, sXml.c_str(), sXml.size());

    //    VOBS_SP vObs;
    //    bool bFind = pEngine->m_cServer.Find("ACCESS", vObs);
    //    if (bFind && vObs.get()) vObs->Update(CMD_SET_CFG, sPacket, nPacket);
    //}
  }
  break;

  ///图片抓拍指令(输出接口)///////////////////////////////////////////////////////////////////////
  case SAVING_FACE_CAMERA_SNAPSHOT_IMAGE: {           //保存人像摄像机抓拍图像
    /*uint32_t nImgSize = pAction->img_w * pAction->img_h * 3 / 2;
    TAG_IVA_FaceImage *pIva = (TAG_IVA_FaceImage*)pAction->imgbuf;
    if (pIva == NULL)
    break;

    pIva->nWidth  = (uint16_t)pAction->img_w;
    pIva->nHeight = (uint16_t)pAction->img_h;
    if (pAction->address)
    {
    ((CNetClient*)pAction->address)->SetNonBlockMode(0);
    NET_COMMAND((CNetClient*)pAction->address, E_MINOR_IVA_FACE_IMAGE, (uint8_t*)pAction->imgbuf, nImgSize);
    ((CNetClient*)pAction->address)->SetNonBlockMode(1);
    }*/
    LOG_INFO("保存人像摄像机抓拍图像 %d, %d.", pAction->img_w, pAction->img_h);
  }
  break;

  case SAVING_DOOR_CAMERA_SNAPSHOT_IMAGE:             //保存门户摄像机抓拍图像
  case SAVING_ENV_CAMERA_SNAPSHOT_IMAGE: {            //保存环境摄像机抓拍图像
    printf("###########################保存摄像机抓拍图像 %d %d %d, %d.\n",
           pAction->event_type, pAction->event_priority, pAction->img_w, pAction->img_h);

    PhotoProcess(pAction);
  }
  break;
  }
}
