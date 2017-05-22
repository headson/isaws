/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����: 
/************************************************************************/
#ifndef LIBPLATFORM_CVIDEOENCODE_H
#define LIBPLATFORM_CVIDEOENCODE_H

#include "vdefine.h"

#include "vpu.h"
#include "v4l2.h"

class CVideoEncode
{
public:
    CVideoEncode(int32_t nChn=0);
    virtual ~CVideoEncode();

    int32_t Start();
    void    Stop();

    void    ResetCamera();

    // �ж��Ƿ��Ѳ����Զ������
    bool    HaveSameHandle(string sName, int32_t nChn);

    // �����û�����
    int32_t SendUserData(int8_t* pData, int32_t nData);

public:
    virtual void GetIFrame() 
    {
        m_cVpu.bGetIFrame = true;
    }

public:
    virtual void SetExp(int32_t nMin, int32_t nMax)
    {
        m_nMinExp = nMin; m_nMaxExp = nMax;
    }
    virtual void SetGain(int32_t nMin, int32_t nMax)
    {
        m_nMinGain = nMin; m_nMaxGain = nMax;
    }
    virtual void OpenISPAddr(std::string sAddr, int32_t nSlave);
    virtual void SetISPValue(int32_t nReg, int32_t nValue);
    virtual int32_t GetISPValue(int32_t nReg, int32_t& nValue);

public:
    // ��Ƶ����
    virtual void SetVideo(std::string sVideo) 
    {
        m_cV4l2.sVideo = sVideo;
    }
    virtual void SetViSize(uint16_t nWidth, uint16_t nHeight) 
    {
        m_cV4l2.nWidth = nWidth; m_cV4l2.nHeight = nHeight;
    }
    virtual void SetInput(int32_t nInput) // MEM IC-MEM
    {
        m_cV4l2.nInput = nInput;
    }   
    virtual void SetFrmFreq(int32_t nFreq)// ֡��
    {
        m_cV4l2.nFrmFreq = nFreq;
    }   
    virtual void SetFrmLost(int32_t nLost)// ֡��ʧ
    {
        m_cV4l2.nFrmLost = nLost;
    }   

    // ��Ƶ����
    virtual void SetEncSize(uint16_t nWidth, uint16_t nHeight) 
    {
        m_cVpu.nWidth = nWidth; m_cVpu.nHeight = nHeight;
    }
    virtual void SetCodeFormat(int32_t eFormat) // �����ʽ
    {
        m_eCodec = eFormat;
        m_cVpu.eEncFormat = STD_AVC;
        if (m_eCodec == CODE_ID_MJPEG)
            m_cVpu.eEncFormat = STD_MJPG;
    }  
    virtual void SetBitrate(int32_t nBitrate)  // ����
    {
        m_cVpu.nBitrate = nBitrate;
    }   
    virtual void SetIFrmFreq(int32_t nIGop)        // I֡���
    {
        m_cVpu.nIGopSize = nIGop;
    }

protected:
    void Process();

    static void* ThreadImage(void* pArg);
    void NotifyImage();

    void CamISPInit();

protected:
  int32_t     n_chn_;
    int         m_i2c;

    int32_t     m_nMinExp;
    int32_t     m_nMaxExp;
    int32_t     m_nMinGain;
    int32_t     m_nMaxGain;

protected:
    int32_t     m_nYuvScale;
    int8_t*     m_pPkgBuffer;

private:
    int32_t     m_eCodec;
    int32_t     m_nStartTime;   // ��������

private:
    CVpu        m_cVpu;         // ����
    CV4l2       m_cV4l2;        // V4L2
    bool        m_bResetCamera; // ����CAMERA

private:
    struct {
        VSignal vEvent;
        VThread vThread;

        int8_t* pVideo;
        int32_t nVideo;
    } m_cImage;
};
#endif  // LIBPLATFORM_CVIDEOENCODE_H