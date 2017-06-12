/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef LIBPLATFORM_CALSAAUDIOENCODE_H
#define LIBPLATFORM_CALSAAUDIOENCODE_H

#include "inc/vtypes.h"

#include <alsa/asoundlib.h>

class CAlsaAudioEncode : public CAudioEncode
{
public:
    CAlsaAudioEncode();
    virtual ~CAlsaAudioEncode();

    // 模块运行
    int32_t Start();

    // 模块停止
    void    Stop();

    virtual bool    HaveSameHandle(string sName, bool bPcm=false);   // 判断是否已插入此远程连接
    // 插入观察者
    virtual int32_t Attach(VOBS_SP& cObs, int32_t nChn=0, bool bPcm=false);
    // 释放观察者
    virtual int32_t Detach(VOBS_SP& cObs, int32_t nChn=0, bool bPcm=false);

protected:
    // 音频采集打开
    int32_t AudioOpen();
    // 音频采集关闭
    void    AudioClose();

    // 音频采集线程
    void         Process();

    // 音频采集初始化
    int32_t      Sgtl5000Init();

private:
    VSubject                m_cPcmSubj;  // PCM
    VSubject                m_cAdoSubj;  // ADO

private:
    uint16_t                m_nCount;             // 
    int32_t                 m_nBitSize;      // 

private:
    snd_pcm_t*              m_pHandle;
    snd_pcm_uframes_t       m_nFrames;
    snd_pcm_hw_params_t*    m_pParams;

private:
    VSignal                 m_vEvent;
    VRingBuffer             m_vBuffer;
};

#endif  // LIBPLATFORM_CALSAAUDIOENCODE_H
