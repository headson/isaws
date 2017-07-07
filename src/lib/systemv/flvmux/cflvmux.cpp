#include "cflvmux.h"

static void avio_w8(FLV_DATA *s, uint32_t b) {
  s->data[s->w_pos] = b;
  s->w_pos++;
}

static void avio_wb16(FLV_DATA *s, uint32_t val) {
  avio_w8(s, (int)val >> 8);
  avio_w8(s, (uint8_t)val);
}

static void avio_wb24(FLV_DATA *s, uint32_t val) {
  avio_wb16(s, (int)val >> 8);
  avio_w8(s, (uint8_t)val);
}

static void avio_wb32(FLV_DATA *s, uint32_t val) {
  avio_w8(s, val >> 24);
  avio_w8(s, (uint8_t)(val >> 16));
  avio_w8(s, (uint8_t)(val >> 8));
  avio_w8(s, (uint8_t)val);
}

static void avio_wb64(FLV_DATA *s, uint64_t val) {
  avio_wb32(s, (uint32_t)(val >> 32));
  avio_wb32(s, (uint32_t)(val & 0xffffffff));
}

static void avio_write(FLV_DATA *s, const char *buf, uint32_t size) {
  while (size > 0) {
    uint32_t len = size;
    memcpy(&s->data[s->w_pos], buf, len);
    s->w_pos += len;

    buf += len;
    size -= len;
  }
}

static void put_amf_string(FLV_DATA *pb, const char *str) {
  avio_wb16(pb, strlen(str));
  avio_write(pb, str, strlen(str));
}

/**
 * Reinterpret a double as a 64-bit integer.
 */

static uint64_t av_double2int(double f) {
  union av_intfloat64 {
    uint64_t i;
    double   f;
  };

  union av_intfloat64 v;
  v.f = f;
  return v.i;
}

static void put_amf_double(FLV_DATA *pb, double d) {
  avio_w8(pb, AMF_DATA_TYPE_NUMBER);
  avio_wb64(pb, av_double2int(d));
}

static void put_amf_bool(FLV_DATA *pb, uint32_t b) {
  avio_w8(pb, AMF_DATA_TYPE_BOOL);
  avio_w8(pb, b > 0);
}

//////////////////////////////////////////////////////////////////////////
CFlvMux::CFlvMux(FILE *file)
  : p_file(file) {
}

CFlvMux::~CFlvMux() {
  if (p_file) {
    fclose(p_file);
    p_file = NULL;
  }
}

int32_t CFlvMux::InitHeadTag0(uint8_t *p_dst, uint32_t n_dst,
                           uint32_t n_width, uint32_t n_height) {
  FLV_DATA *flv_head_ = (FLV_DATA*)p_dst;
  if (flv_head_ == NULL) {
    printf("malloc flv meta failed.\n");
    return -1;
  }
  flv_head_->w_pos = 0;
  flv_head_->n_max = n_dst - sizeof(FLV_DATA);

  avio_write(flv_head_, "FLV", 3);
  avio_w8(flv_head_, 0x01);         // version
  avio_w8(flv_head_, (FLV_HEADER_FLAG_HASVIDEO
                      | FLV_HEADER_FLAG_HASAUDIO)); // flag;enable video audio
  avio_wb32(flv_head_, 0x09);
  avio_wb32(flv_head_, 0x00);       // pre tag size

  /* write meta_tag */
  avio_w8(flv_head_,   0x12);   // 0x12=SCRIPT META,0x08=AUDIO,0x09=VIDEO
  // Length of the message.Number of bytes after StreamID to end of tag
  // (Equal to length of the tag �C 11)
  avio_wb24(flv_head_, 279-11); // size of data part(sum of all parts below)
  avio_wb24(flv_head_, 0);      // timestamp
  avio_wb32(flv_head_, 0);      // reserved

  /* first event name as a string */
  avio_w8(flv_head_, AMF_DATA_TYPE_STRING);
  put_amf_string(flv_head_, "onMetaData"); // 12 bytes

  /* mixed array (hash) with size and string/type/data tuples */
  avio_w8(flv_head_,   AMF_DATA_TYPE_MIXEDARRAY);
  avio_wb32(flv_head_, 2 + 5 + 5);

  // fill in the guessed duration, it'll be corrected later if incorrect
  put_amf_string(flv_head_, "duration");
  put_amf_double(flv_head_, 0x52);

  //////////////////////////////////////////////////////////////////////////
  put_amf_string(flv_head_, "width");
  put_amf_double(flv_head_, n_width);

  put_amf_string(flv_head_, "height");
  put_amf_double(flv_head_, n_height);

  // ��Ƶ����
  put_amf_string(flv_head_, "videodatarate");
  put_amf_double(flv_head_, 1024);

  // ��Ƶ֡��
  put_amf_string(flv_head_, "framerate");
  put_amf_double(flv_head_, 25);

  // ��Ƶ���뷽ʽ
  put_amf_string(flv_head_, "videocodecid");
  put_amf_double(flv_head_, FLV_CODECID_H264);

  //////////////////////////////////////////////////////////////////////////
  put_amf_string(flv_head_, "audiodatarate");   // ������
  put_amf_double(flv_head_, 16000);
    
  // ��Ƶ������
  put_amf_string(flv_head_, "audiosamplerate");
  put_amf_double(flv_head_, 8000);

  // ��Ƶ��������
  put_amf_string(flv_head_, "audiosamplesize");
  put_amf_double(flv_head_, 16);

  put_amf_string(flv_head_, "stereo");
  put_amf_bool(flv_head_,   0);

  put_amf_string(flv_head_, "audiocodecid");
  put_amf_double(flv_head_, FLV_CODECID_PCM_ALAW);

  // data_enc
  //put_amf_string(flv_meta_, "datastream");
  //put_amf_double(flv_meta_, 0.0);

  put_amf_string(flv_head_, "filesize");
  put_amf_double(flv_head_, 0); // delayed write

  put_amf_string(flv_head_, "");
  avio_w8(flv_head_, AMF_END_OF_OBJECT);
  avio_wb32(flv_head_, flv_head_->w_pos - 13);

  if (p_file) {
    fwrite(flv_head_->data, 1, flv_head_->w_pos, p_file);
    fflush(p_file);
  }
  return (int32_t)flv_head_->w_pos;
}

int32_t CFlvMux::VideoPacket(uint8_t *p_dst, uint32_t n_dst,
                          bool b_key_frame, uint64_t n_pts,
                          const uint8_t *p_src, uint32_t n_src) {
  FLV_DATA *flv_avdo_ = (FLV_DATA *)p_dst;
  if (flv_avdo_ == NULL) {
    printf("malloc flv avdo failed.\n");
    return -1;
  }
  flv_avdo_->w_pos = 0;
  flv_avdo_->n_max = n_dst - sizeof(FLV_DATA);

  avio_w8(flv_avdo_,    FLV_TAG_TYPE_VIDEO);
  avio_wb24(flv_avdo_,  n_src + 5);
  avio_wb24(flv_avdo_,  n_pts);
  avio_w8(flv_avdo_,    (n_pts >> 24) & 0x7F); // timestamps are 32 bits _signed_
  avio_wb24(flv_avdo_,  0);

  // 5
  avio_w8(flv_avdo_,    ((b_key_frame ? 1 : 0) << 4) | 7);
  avio_w8(flv_avdo_,    0x01);   // 0=AVC sequence header 1=AVC NALU
  avio_wb24(flv_avdo_,  20);     // composition time

  // 
  avio_write(flv_avdo_, (char*)p_src, n_src);    // data
  avio_wb32(flv_avdo_,  n_src + 5 + 11);  // previous tag size

  if (p_file) {
    fwrite(flv_avdo_->data, 1, flv_avdo_->w_pos, p_file);
    fflush(p_file);
  }
  return flv_avdo_->w_pos;
}

int32_t CFlvMux::AudioPacket(uint8_t *p_dst, uint32_t n_dst,
                          uint64_t n_pts, 
                          const uint8_t *p_src, uint32_t n_src) {
  FLV_DATA *flv_avdo_ = (FLV_DATA *)p_dst;
  if (flv_avdo_ == NULL) {
    printf("malloc flv avdo failed.\n");
    return -1;
  }
  flv_avdo_->w_pos = 0;
  flv_avdo_->n_max = n_dst - sizeof(FLV_DATA);

  avio_w8(flv_avdo_,   FLV_TAG_TYPE_AUDIO);
  avio_wb24(flv_avdo_, n_src + 1);
  avio_wb24(flv_avdo_, n_pts);
  avio_w8(flv_avdo_,   (n_pts >> 24) & 0x7F); // timestamps are 32 bits _signed_
  avio_wb24(flv_avdo_, 0);

  // 1
  avio_w8(flv_avdo_, (FLV_CODECID_PCM_ALAW |
          FLV_SAMPLERATE_SPECIAL | FLV_SAMPLESSIZE_16BIT));

  // 
  avio_write(flv_avdo_, (char*)p_src, n_src);    // data
  avio_wb32(flv_avdo_,  n_src + 1 + 11);  // previous tag size

  if (p_file) {
    fwrite(flv_avdo_->data, 1, flv_avdo_->w_pos, p_file);
    fflush(p_file);
  }
  return flv_avdo_->w_pos;
}