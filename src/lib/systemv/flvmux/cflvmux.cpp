/************************************************************************/
/* Author      : SoberPeng 2017-07-05
/* Description :
/************************************************************************/
#include "cflvmux.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else

#endif

/* offsets for packed values */
#define FLV_AUDIO_SAMPLESSIZE_OFFSET 1
#define FLV_AUDIO_SAMPLERATE_OFFSET  2
#define FLV_AUDIO_CODECID_OFFSET     4

#define FLV_VIDEO_FRAMETYPE_OFFSET   4

/* bitmasks to isolate specific values */
#define FLV_AUDIO_CHANNEL_MASK    0x01
#define FLV_AUDIO_SAMPLESIZE_MASK 0x02
#define FLV_AUDIO_SAMPLERATE_MASK 0x0c
#define FLV_AUDIO_CODECID_MASK    0xf0

#define FLV_VIDEO_CODECID_MASK    0x0f
#define FLV_VIDEO_FRAMETYPE_MASK  0xf0

#define AMF_END_OF_OBJECT         0x09

#define KEYFRAMES_TAG            "keyframes"
#define KEYFRAMES_TIMESTAMP_TAG  "times"
#define KEYFRAMES_BYTEOFFSET_TAG "filepositions"

enum {
  FLV_HEADER_FLAG_HASVIDEO = 1,
  FLV_HEADER_FLAG_HASAUDIO = 4,
};

enum {
  FLV_STREAM_TYPE_VIDEO,
  FLV_STREAM_TYPE_AUDIO,
  FLV_STREAM_TYPE_DATA,
  FLV_STREAM_TYPE_NB,
};

enum {
  FLV_MONO   = 0,
  FLV_STEREO = 1,
};

enum {
  FLV_SAMPLESSIZE_8BIT  = 0,
  FLV_SAMPLESSIZE_16BIT = 1 << FLV_AUDIO_SAMPLESSIZE_OFFSET,
};

enum {
  FLV_SAMPLERATE_SPECIAL = 0, /**< signifies 5512Hz and 8000Hz in the case of NELLYMOSER */
  FLV_SAMPLERATE_11025HZ = 1 << FLV_AUDIO_SAMPLERATE_OFFSET,
  FLV_SAMPLERATE_22050HZ = 2 << FLV_AUDIO_SAMPLERATE_OFFSET,
  FLV_SAMPLERATE_44100HZ = 3 << FLV_AUDIO_SAMPLERATE_OFFSET,
};

enum {
  FLV_CODECID_PCM = 0,
  FLV_CODECID_ADPCM                 = 1 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_MP3                   = 2 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_PCM_LE                = 3 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_NELLYMOSER_16KHZ_MONO = 4 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_NELLYMOSER_8KHZ_MONO  = 5 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_NELLYMOSER            = 6 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_PCM_ALAW              = 7 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_PCM_MULAW             = 8 << FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_AAC                   = 10<< FLV_AUDIO_CODECID_OFFSET,
  FLV_CODECID_SPEEX                 = 11<< FLV_AUDIO_CODECID_OFFSET,
};

enum {
  FLV_CODECID_H263    = 2,
  FLV_CODECID_SCREEN  = 3,
  FLV_CODECID_VP6     = 4,
  FLV_CODECID_VP6A    = 5,
  FLV_CODECID_SCREEN2 = 6,
  FLV_CODECID_H264    = 7,
  FLV_CODECID_REALH263= 8,
  FLV_CODECID_MPEG4   = 9,
};

enum {
  FLV_FRAME_KEY            = 1 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< key frame (for AVC, a seekable frame)
  FLV_FRAME_INTER          = 2 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< inter frame (for AVC, a non-seekable frame)
  FLV_FRAME_DISP_INTER     = 3 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< disposable inter frame (H.263 only)
  FLV_FRAME_GENERATED_KEY  = 4 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< generated key frame (reserved for server use only)
  FLV_FRAME_VIDEO_INFO_CMD = 5 << FLV_VIDEO_FRAMETYPE_OFFSET, ///< video info/command frame
};

typedef enum {
  AMF_DATA_TYPE_NUMBER      = 0x00,
  AMF_DATA_TYPE_BOOL        = 0x01,
  AMF_DATA_TYPE_STRING      = 0x02,
  AMF_DATA_TYPE_OBJECT      = 0x03,
  AMF_DATA_TYPE_NULL        = 0x05,
  AMF_DATA_TYPE_UNDEFINED   = 0x06,
  AMF_DATA_TYPE_REFERENCE   = 0x07,
  AMF_DATA_TYPE_MIXEDARRAY  = 0x08,
  AMF_DATA_TYPE_OBJECT_END  = 0x09,
  AMF_DATA_TYPE_ARRAY       = 0x0a,
  AMF_DATA_TYPE_DATE        = 0x0b,
  AMF_DATA_TYPE_LONG_STRING = 0x0c,
  AMF_DATA_TYPE_UNSUPPORTED = 0x0d,
} AMFDataType;

#define QWORD_BE(val) (((val>>56)&0xFF) | (((val>>48)&0xFF)<<8) | (((val>>40)&0xFF)<<16) | (((val>>32)&0xFF)<<24) | \
  (((val>>24)&0xFF)<<32) | (((val>>16)&0xFF)<<40) | (((val>>8)&0xFF)<<48) | ((val&0xFF)<<56))
#define DWORD_BE(val) (((val>>24)&0xFF) | (((val>>16)&0xFF)<<8) | (((val>>8)&0xFF)<<16) | ((val&0xFF)<<24))
#define WORD_BE(val)  (((val>>8)&0xFF) | ((val&0xFF)<<8))

unsigned int fastHtonl(unsigned int dw) {
  return DWORD_BE(dw);
}
unsigned short fastHtons(unsigned short  w) {
  return  WORD_BE(w);
}

char *AMF_EncodeInt8(char *output, char nVal) {
  output[0] = nVal;
  return output+1;
}

char *AMF_EncodeInt16(char *output, short nVal) {
  output[1] = nVal & 0xff;
  output[0] = nVal >> 8;
  return output+2;
}

char *AMF_EncodeInt24(char *output, int nVal) {
  output[2] = nVal & 0xff;
  output[1] = nVal >> 8;
  output[0] = nVal >> 16;
  return output+3;
}

char *AMF_EncodeInt32(char *output, int nVal) {
  output[3] = nVal & 0xff;
  output[2] = nVal >> 8;
  output[1] = nVal >> 16;
  output[0] = nVal >> 24;
  return output+4;
}

char *AMF_EncodeString(char *output, const char *bv) {
  int nbv = strlen(bv);
  if (nbv < 65536) {
    *output++ = AMF_DATA_TYPE_STRING;
    output = AMF_EncodeInt16(output, nbv);
  } else {
    *output++ = AMF_DATA_TYPE_LONG_STRING;
    output = AMF_EncodeInt32(output, nbv);
  }
  memcpy(output, bv, nbv);
  output += nbv;

  return output;
}

char *AMF_EncodeNumber(char *output, double dVal) {
  *output++ = AMF_DATA_TYPE_NUMBER; /* type: Number */

  unsigned char *ci, *co;
  ci = (unsigned char *)&dVal;
  co = (unsigned char *)output;
  co[0] = ci[7];
  co[1] = ci[6];
  co[2] = ci[5];
  co[3] = ci[4];
  co[4] = ci[3];
  co[5] = ci[2];
  co[6] = ci[1];
  co[7] = ci[0];

  return output+8;
}

char *AMF_EncodeBoolean(char *output, int bVal) {
  *output++ = AMF_DATA_TYPE_BOOL;
  *output++ = bVal ? 0x01 : 0x00;
  return output;
}

char *AMF_EncodeNamedString(char *output, const char *strName, const char *strValue) {
  int nname = strlen(strName);
  output = AMF_EncodeInt16(output, nname);

  memcpy(output, strName, nname);
  output += nname;

  return AMF_EncodeString(output, strValue);
}

char *AMF_EncodeNamedNumber(char *output, const char *strName, double dVal) {
  int nname = strlen(strName);
  output = AMF_EncodeInt16(output, nname);

  memcpy(output, strName, nname);
  output += nname;

  return AMF_EncodeNumber(output, dVal);
}

char *AMF_EncodeNamedBoolean(char *output, const char *strName, int bVal) {
  int nname = strlen(strName);
  output = AMF_EncodeInt16(output, nname);

  memcpy(output, strName, nname);
  output += nname;

  return AMF_EncodeBoolean(output, bVal);
}

char *CFlvMux::HeaderAndMetaDataTag(char *p_packet,
                                    int width, int height,
                                    int audiodatarate, int audiosamplerate, int audiosamplesize, int audiochannels) {
  // write the file header
  static unsigned char header[13] = {
    'F', 'L', 'V',  // FLV file signature
    0x01,           // FLV file version = 1
    0x5,            // Flags - modified later
    0, 0, 0, 9,     // size of the header
    0, 0, 0, 0      // pre packet size
  };
  static unsigned int header_size = 13;

  memcpy(p_packet, header, header_size);
  p_packet += header_size;

  char  metaData[2048] = {0};
  char *dst = metaData;
  /* first event name as a string */
  dst = AMF_EncodeString(dst, "onMetaData");

  /* mixed array (hash) with size and string/type/data tuples */
  *dst++ = AMF_DATA_TYPE_MIXEDARRAY;
  dst = AMF_EncodeInt32(dst, 13);    // 1 + 1 + 4 + 6 + 1

  dst = AMF_EncodeNamedNumber(dst, "duration",        0.0);
  dst = AMF_EncodeNamedNumber(dst, "filesize",        0.0);
  dst = AMF_EncodeNamedNumber(dst, "width",           width);
  dst = AMF_EncodeNamedNumber(dst, "height",          height);

  dst = AMF_EncodeNamedNumber(dst, "videocodecid",    FLV_CODECID_H264);

  dst = AMF_EncodeNamedNumber(dst, "videodatarate",   4000);

  dst = AMF_EncodeNamedNumber(dst, "audiocodecid",    FLV_CODECID_PCM_MULAW);

  dst = AMF_EncodeNamedNumber(dst, "audiodatarate",   audiodatarate);
  dst = AMF_EncodeNamedNumber(dst, "audiosamplerate", audiosamplerate);
  dst = AMF_EncodeNamedNumber(dst, "audiosamplesize", audiosamplesize);
  dst = AMF_EncodeNamedNumber(dst, "audiochannels",   audiochannels);

  dst = AMF_EncodeNamedBoolean(dst, "stereo", audiochannels == 2);

  dst = AMF_EncodeNamedString(dst, "encoder", "desktopwebshare 1.0");

  *dst++ = 0;
  *dst++ = 0;
  *dst++ = AMF_END_OF_OBJECT;

  return Packet(p_packet, NULL, 0, metaData, dst - metaData, 0x12, 0);
}

char * CFlvMux::PacketVideo(char *p_packet, char *p_vdo, int n_vdo, bool is_key_frame, unsigned int timestamp) {
  char *p_enc = p_packet;

  *p_enc++ = 0x09;
  
  p_enc = AMF_EncodeInt24(p_enc, n_vdo+5+4); //3

  unsigned long netTimestamp = fastHtonl(timestamp);
  memcpy(p_enc, ((char*)(&netTimestamp)) + 1, 3);
  p_enc += 3;

  memcpy(p_enc, &netTimestamp, 1);
  p_enc += 1;

  unsigned int  streamID = 0;
  memcpy(p_enc, &streamID, 3);
  p_enc += 3;

  if (is_key_frame) {
    *p_enc++ = 0x17;
  } else {
    *p_enc++ = 0x27;
  }

  *p_enc++ = 0x01;
  p_enc = AMF_EncodeInt24(p_enc, 0);

  p_enc = AMF_EncodeInt32(p_enc, n_vdo);

  memcpy(p_enc, p_vdo, n_vdo);
  p_enc += n_vdo;

  return AMF_EncodeInt32(p_enc, n_vdo + 11 + 9);
}

char* CFlvMux::Packet(char *p_packet,
                      const char *p_head, int n_head,
                      const char *p_data, int n_data,
                      int type, unsigned int timestamp) {
  char *p_enc = p_packet;

  *p_enc++ = type;

  p_enc = AMF_EncodeInt24(p_enc, n_head+n_data); //3

  //time tamp-> [time tamp 3b,time tamp ex 1b]
  unsigned long netTimestamp = fastHtonl(timestamp);
  memcpy(p_enc, ((char*)(&netTimestamp)) + 1, 3);
  p_enc += 3;

  memcpy(p_enc, &netTimestamp, 1);
  p_enc += 1;

  unsigned int  streamID = 0;
  memcpy(p_enc, &streamID, 3);
  p_enc += 3;

  if (p_head) {
    memcpy(p_enc, p_head, n_head);
    p_enc += n_head;
  }
  memcpy(p_enc, p_data, n_data);
  p_enc += n_data;

  return AMF_EncodeInt32(p_enc, n_head + n_data + 11);
}

CFlvMux::CFlvMux() {
  avcc_size_ = 0;
}

int CFlvMux::MakeAVCc(char* sps_pps, int sps_size, int pps_size) {
  if (!sps_pps ||
      sps_size <= 0 ||
      pps_size <= 0) {
    return -1;
  }
  if (avcc_size_ > 0) {
    return avcc_size_;
  }

  char *p_sps = sps_pps;
  char *p_pps = sps_pps + sps_size;

  char* p_out = avcc_;

  p_out[0] = 0x17;
  p_out[1] = 0;
  p_out[2] = 0;
  p_out[3] = 0;
  p_out[4] = 0;
  p_out[5] = 1;
  p_out[6] = p_sps[5];
  p_out[7] = p_sps[6];
  p_out[8] = p_sps[7];
  p_out[9] = 0xff;
  p_out[10] = 0xe1;
  p_out += 11;

  unsigned short size = 0;
  size = fastHtons(sps_size - 4);
  memcpy(p_out, &size, 2);
  p_out += 2;

  memcpy(p_out, p_sps + 4, sps_size - 4);
  p_out += (sps_size - 4);

  p_out[0] = 1;
  p_out += 1;

  size = fastHtons(pps_size - 4);
  memcpy(p_out, &size, 2);
  p_out += 2;

  memcpy(p_out, p_pps + 4, pps_size - 4);
  p_out += (pps_size - 4);

  avcc_size_ = p_out - avcc_;

  return (p_out - avcc_);
}
