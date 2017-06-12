/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef LIBPLATFORM_CALSAAUDIODECODE_H
#define LIBPLATFORM_CALSAAUDIODECODE_H

#include "inc/vtypes.h"

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>       // asoundlib.h pcm.h不能乱顺序

class CAlsaAudioDecode
{
public:
    CAlsaAudioDecode();
    virtual ~CAlsaAudioDecode();

    int32_t     Start();
    void        Stop();

    int32_t     Decode(const int8_t* pData, uint32_t nData, const void* pHdlPlay=NULL, bool bClearAudio=false);

protected:
    /* ALSA PCM Play*/
    int32_t     PcmOpen();
    void        PcmClose();
    int32_t     PcmPlay(int8_t *pData, int32_t nData, bool bReset=false);

protected:
    void        Process();

private:
    uint16_t            m_nFormat;

private:
    // PCM播放
    snd_pcm_t*           m_pHandle;
    snd_pcm_uframes_t    m_nFrames;
    snd_pcm_hw_params_t* m_pParams;

    VSignal              m_vEvent;
    VRingBuffer          m_vBuffer;     // 解码数据
};

#endif  // LIBPLATFORM_CALSAAUDIODECODE_H
