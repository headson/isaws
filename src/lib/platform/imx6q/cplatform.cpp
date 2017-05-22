/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : cplatform.cpp
* Author        : Sober.Peng
* Date          : 7:2:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#include "cplatform.h"

void CPlatform::Initinal()
{
#if defined(IMX6Q)
    RetCode ret;
    vpu_versioninfo ver;
    ret = vpu_Init(NULL);
    if (ret)
    {
        loge("VPU Init Failure.");
        return;
    }

    ret = vpu_GetVersionInfo(&ver);
    if (ret)
    {
        loge("Cannot get version info, err:%d", ret);
        vpu_UnInit();
        return;
    }

    printf("VPU firmware version: %d.%d.%d_r%d.\n", ver.fw_major, ver.fw_minor, ver.fw_release, ver.fw_code);
    printf("VPU library version: %d.%d.%d.\n", ver.lib_major, ver.lib_minor, ver.lib_release);
#endif
}

void CPlatform::Release()
{
#if defined(IMX6Q)
    vpu_UnInit();
#endif
}

CPlatform::CPlatform()
    : VObject()
{
    for (int32_t i = VI_MIN; i < VI_MAX; i++) {
        _pVdoEnc[i] = NULL;
    }

    _pAdoEnc  = NULL;
    _pAdoDec  = NULL;
    _pKDXFDec = NULL;

    _pExtDev  = NULL;
}

CPlatform::~CPlatform()
{
    Stop();
}

int32_t CPlatform::Start()
{
    int32_t nRet = 0;

    nRet = new_video();
    if (nRet != RET_SUCCESS) {
        loge("create video module failed.%d.", nRet);
    }

    nRet = new_audio();
    if (nRet != RET_SUCCESS) {
        loge("create audio module failed.%d.", nRet);
    }

    nRet = new_ext_dev();
    if (nRet != RET_SUCCESS) {
        loge("create extern device module failed.%d.", nRet);
    }
    return RET_SUCCESS;
}

void CPlatform::Stop()
{
    del_video();
    del_audio();
    del_ext_dev();
}

CVideoEncode* CPlatform::get_video(int32_t nChn)
{
    if (nChn >= VI_MIN && nChn < VI_MAX) {
        return _pVdoEnc[nChn];
    }
    
    return NULL;
}

CAudioEncode* CPlatform::get_audio_capture()
{
    return _pAdoEnc;
}

CAudioDecode* CPlatform::get_audio_playback()
{
    return _pAdoDec;
}

CAudioDecode* CPlatform::get_speech()
{
    return _pKDXFDec;
}

CExternDevice* CPlatform::get_ext_dev()
{
    return _pExtDev;
}

int32_t CPlatform::new_video()
{
    struct {
        int32_t nNum;       // i2c
        int32_t nAddr;      // i2c-addr
        int32_t nVideo;     // video[0-2]
        int32_t nInput;     // 0-1
    } vdo[] = {
#if 1
        {1, 0x3c, 0, 1},    // VIDEO0 i2c-1 ov5640 input1 左
        {0, 0x3c, 2, 0},    // VIDEO2 i2c-0 ov5640 input0 中
        {2, 0x3c, 1, 1},    // VIDEO1 i2c-2 ov5640 input1 右
#else
        {1, 0x3c, 1, 1},    // VIDEO0 i2c-1 ov5640 input1 左
        {0, 0x3c, 3, 0},    // VIDEO2 i2c-0 ov5640 input0 中
        {2, 0x3c, 2, 1},    // VIDEO1 i2c-2 ov5640 input1 右
#endif
    };

    // 通道              0, 1, 2； // sensor顺序
#if (HARD_VER == 1101)
    int32_t nVdoChn[] = {2, 1, 0}; // 右、中、左
#elif (HARD_VER == 1201)
    int32_t nVdoChn[] = {2, 1, 0}; // 右、中、左
#elif (HARD_VER == 1102)
    int32_t nVdoChn[] = {2, 0, 1}; // 右、中、左
#elif (HARD_VER == 1202)
    int32_t nVdoChn[] = {2, 0, 1}; // 右、中、左
#elif (HARD_VER == 1100)
    int32_t nVdoChn[] = {1, 0, 2}; // 右，左，中
#elif (HARD_VER == 1200)
    int32_t nVdoChn[] = {0, 1, 2}; // 左、中、右
#elif (HARD_VER == 1300)
    int32_t nVdoChn[] = {1, 0, 2}; // 右，左，中
#elif (HARD_VER == 1400)
    int32_t nVdoChn[] = {2, 0, 1}; // 右，左，中
#elif (HARD_VER == 1502)
    int32_t nVdoChn[] = {2, 0, 1}; // 右，左，中
#else
    int32_t nVdoChn[] = {2, 1, 0}; // 右、中、左
#endif

    int32_t nRet = 0;
    bool bResave = false;
    TAG_CfgVdo cVdo; CONFIG()->GetVdo(cVdo);
    for (int32_t i = VI_MIN; i < VI_MAX; i++) 
    {
        int32_t chn = nVdoChn[i];
        // 使用配置文件的视频通道
        if (cVdo.chn[i].nVdoChn < VI_MIN
            || cVdo.chn[i].nVdoChn >= VI_MAX) 
        {
            cVdo.chn[i].nVdoChn = chn;
            bResave             = true;
        } else {
            chn = cVdo.chn[i].nVdoChn;
        }
        if (!cVdo.chn[i].bEnable)
            continue;

        _pVdoEnc[i] = CVideoCenter::CreateVideoEncode(i);
        if (_pVdoEnc[i])
        {
            // 视频输入
            char sNVdo[128]={0}, sNI2c[128]={0};

            snprintf(sNI2c, 127, "/dev/i2c-%d", vdo[chn].nNum);
#ifndef WIN32
            snprintf(sNVdo, 127, "/dev/video%d", vdo[chn].nVideo);
#else
            snprintf(sNVdo, 127, "D:\\tools\\stream\\video_%d.yuv", i);
#endif
            _pVdoEnc[i]->SetVideo(sNVdo);
            _pVdoEnc[i]->SetInput(vdo[chn].nInput);
            if (cVdo.chn[i].bSetISP != 0) {
                _pVdoEnc[i]->OpenISPAddr(sNI2c, vdo[chn].nAddr);
            }

            int32_t nVWidth = 0, nVHeight = 0;
            CConfig::VideoPixel(cVdo.chn[i].nYuvSize, nVWidth, nVHeight);
            _pVdoEnc[i]->SetViSize(nVWidth, nVHeight);            // 输入分辨率
            _pVdoEnc[i]->SetFrmLost(cVdo.chn[i].nFrmLost);        // 丢帧率
            _pVdoEnc[i]->SetFrmFreq(cVdo.chn[i].nFrmFreq);        // 帧率15-30

            // 视频编码
            int32_t nEWidth = 0, nEHeight = 0;
            CConfig::VideoPixel(cVdo.chn[i].nEncSize, nEWidth, nEHeight);
            _pVdoEnc[i]->SetEncSize(nEWidth, nEHeight);           // 编码分辨率
            _pVdoEnc[i]->SetBitrate(cVdo.chn[i].nBitrate);        // 码流
            _pVdoEnc[i]->SetIFrmFreq(cVdo.chn[i].nIFrmFreq);      // I帧频率
            _pVdoEnc[i]->SetCodeFormat(cVdo.chn[i].nCodeFmt);     // 编码格式 2=STD_AVC

            int32_t nRet = _pVdoEnc[i]->Start();
            if (RET_SUCCESS != nRet){ 
                delete _pVdoEnc[i]; _pVdoEnc[i] = NULL; 
            }
            _pVdoEnc[i]->set_living(&g_app.nLiving[EM_VI_0+i]);

            logi("Create video %s encode %d vi %d-%d, enc %d-%d. result %d", 
                sNVdo, i, nVWidth, nVHeight, nEWidth, nEHeight, nRet)
        }
    }

    if (bResave) {
        CONFIG()->SetVdo(cVdo);
    }
    return nRet;
}

void CPlatform::del_video()
{
    for (int32_t i = VI_MIN; i < VI_MAX; i++) {
        if (_pVdoEnc[i]) { delete _pVdoEnc[i]; _pVdoEnc[i] = NULL; }
    }
}

int32_t CPlatform::new_audio()
{
    int32_t nRet = 0;

    // 科大讯飞
    _pKDXFDec = CAudioCenter::CreateAudioDecode(CAudioDecode::ADO_DEC_KDXF);
    if (_pKDXFDec == NULL) {
        loge("create kdxf failed.");
    } else {
        nRet = _pKDXFDec->Start();
        if (RET_SUCCESS != nRet) {
            if (_pKDXFDec) { delete _pKDXFDec;  _pKDXFDec = NULL; }
            loge("KDXF decode start failed.");
        }
        _pKDXFDec->set_living(&g_app.nLiving[EM_AO_KDXF]);
    }

    // ALSA解码
    _pAdoDec = CAudioCenter::CreateAudioDecode(CAudioDecode::ADO_DEC_ALSA);
    if (_pAdoDec == NULL) {
        loge("create audio decode failed.");
    } else {
        nRet = _pAdoDec->Start();
        if (RET_SUCCESS != nRet)
        {
            if (_pAdoDec) { delete _pAdoDec; _pAdoDec = NULL; }
            loge("audio decode start failed.");
        }
        _pAdoDec->set_living(&g_app.nLiving[EM_AO_ALSA]);
    }

    // ALSA编码
    _pAdoEnc = CAudioCenter::CreateAudioEncode();
    if (_pAdoEnc == NULL) {
       loge("create audio encode failed.");
    } else {
        nRet = _pAdoEnc->Start();
        if (RET_SUCCESS != nRet)
        {
            if (_pAdoEnc) { delete _pAdoEnc; _pAdoEnc = NULL; }
            loge("audio encode start failed");
        }
        _pAdoEnc->set_living(&g_app.nLiving[EM_AI_ALSA]);
    }
    return RET_SUCCESS;
}

void CPlatform::del_audio()
{
    if (_pAdoEnc)  { delete _pAdoEnc;  _pAdoEnc = NULL; }
    if (_pAdoDec)  { delete _pAdoDec;  _pAdoDec = NULL; }
    if (_pKDXFDec) { delete _pKDXFDec; _pKDXFDec = NULL; }
}

int32_t CPlatform::new_ext_dev()
{
    int32_t nRet = 0;

    _pExtDev = CExternCenter::CreateExternDevice();
    if (_pExtDev == NULL) {
        loge("create extern device failed.");
    } else {
        nRet = _pExtDev->Start();
        if (RET_SUCCESS != nRet)
        {
            if (_pExtDev) { delete _pExtDev; _pExtDev = NULL; }
            loge("extern device start failed");
        }
    }
    logi("extern device open success 0x%x.", (uint32_t)_pExtDev);
    return nRet;
}

void CPlatform::del_ext_dev()
{
    if (_pExtDev) { delete _pExtDev; _pExtDev = NULL; }
}

