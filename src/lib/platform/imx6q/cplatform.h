/************************************************************************/
/* ×÷Õß: SoberPeng 17-05-23
/* ÃèÊö: 
/************************************************************************/
#ifndef LIBPLATFORM_CPLATFORM_H
#define LIBPLATFORM_CPLATFORM_H

#include "cvideoencode.h"
#include "cphotoencode.h"

#include "caudioencode.h"
#include "caudiodecode.h"

#include "cexterndevice.h"

#include "yuv420.h"

#ifdef IMX6Q
#include "imx6q/cfslvideoencode.h"
#include "imx6q/cfslphotoencode.h"

#include "imx6q/calsaaudioencode.h"
#include "imx6q/calsaaudiodecode.h"
#include "imx6q/ckdxfaudiodecode.h"

#include "imx6q/cfslexterndevice.h"
#elif defined(HI3516A)
#include "hi3516a/chisivideoencode.h"
#elif defined(WIN32)
#include "file/cfilevideoencode.h"
#include "file/cfilephotoencode.h"
#endif

class CVideoCenter
{
public:
    static CVideoEncode* CreateVideoEncode(int32_t nChn)
    {
        CVideoEncode *pEnc = NULL;
#ifdef IMX6Q
        pEnc = new CVideoEncode(nChn);
#elif defined(HI3516A)
        pEnc = new CHisiVideoEncode(nChn);
#elif defined(WIN32)
        pEnc = new CFileVideoEncode(nChn);
#endif
        return pEnc;
    }
};

class CPhotoCenter
{
public:
    static CPhotoEncode* CreatePhotoEncode(int32_t eCode=CODE_ID_MJPEG)
    {
        CPhotoEncode *pEnc = NULL;
#ifdef IMX6Q
        pEnc = new CPhotoEncode(eCode);
#elif defined(WIN32)
        pEnc = new CFilePhotoEncode(eCode);
#endif
        return pEnc;
    }
};

class CAudioCenter
{
public:
    static CAudioEncode* CreateAudioEncode()
    {
        CAudioEncode* pEnc = NULL;
#ifdef IMX6Q
        pEnc = new CAlsaAudioEncode();
#elif defined(WIN32)
        pEnc = NULL;
#endif
        return pEnc;
    }

    static CAudioDecode* CreateAudioDecode(int32_t eType=CAudioDecode::ADO_DEC_ALSA)
    {
        CAudioDecode* pDec = NULL;

#ifdef IMX6Q
        if (eType == CAudioDecode::ADO_DEC_ALSA)
        {
            pDec = new CAlsaAudioDecode();
        }
        else if(eType == CAudioDecode::ADO_DEC_KDXF)
        {
            pDec = new CKDXFAudioDecode();
        }
#elif defined(WIN32)
        pDec = NULL;
#endif

        return pDec;
    }
};

class CExternCenter
{
public:
    static CExternDevice* CreateExternDevice()
    {
        CExternDevice* pDev = NULL;
#ifdef IMX6Q
        pDev = new CFslExternDevice();
#elif defined(WIN32)
        pDev = NULL;
#endif
        return pDev;
    }
};

class CPlatform : public VObject
{
public:
    static void Initinal();
    static void Release();

public:
    CPlatform();
    virtual ~CPlatform();

    int32_t         Start();
    void            Stop();

    CVideoEncode*   get_video(int32_t nChn);

    CAudioEncode*   get_audio_capture();
    CAudioDecode*   get_audio_playback();
    CAudioDecode*   get_speech();

    CExternDevice*  get_ext_dev();

protected:
    int32_t         new_video();
    void            del_video();

    int32_t         new_audio();
    void            del_audio();

    int32_t         new_ext_dev();
    void            del_ext_dev();

protected:
    CVideoEncode*   _pVdoEnc[VI_MAX];   // 

    CAudioEncode*   _pAdoEnc;           // 
    CAudioDecode*   _pAdoDec;           // 
    CAudioDecode*   _pKDXFDec;          // 

    CExternDevice*  _pExtDev;           // 
};

#endif  // LIBPLATFORM_CPLATFORM_H
