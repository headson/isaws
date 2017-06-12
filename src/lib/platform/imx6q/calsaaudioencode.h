/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����: 
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

    // ģ������
    int32_t Start();

    // ģ��ֹͣ
    void    Stop();

    virtual bool    HaveSameHandle(string sName, bool bPcm=false);   // �ж��Ƿ��Ѳ����Զ������
    // ����۲���
    virtual int32_t Attach(VOBS_SP& cObs, int32_t nChn=0, bool bPcm=false);
    // �ͷŹ۲���
    virtual int32_t Detach(VOBS_SP& cObs, int32_t nChn=0, bool bPcm=false);

protected:
    // ��Ƶ�ɼ���
    int32_t AudioOpen();
    // ��Ƶ�ɼ��ر�
    void    AudioClose();

    // ��Ƶ�ɼ��߳�
    void         Process();

    // ��Ƶ�ɼ���ʼ��
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
