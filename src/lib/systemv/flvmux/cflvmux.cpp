#include "cflvmux.h"

static void avio_w8(FLV_DATA *s, uint32_t b) {
  s->buffer[s->w_pos] = b;
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
    memcpy(&s->buffer[s->w_pos], buf, len);
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
  FLV_DATA flv;
  flv.buffer = p_dst;
  flv.w_pos = 0;
  flv.n_max = n_dst - sizeof(FLV_DATA);

  avio_write(&flv, "FLV", 3);
  avio_w8(&flv, 0x01);         // version
  avio_w8(&flv, (FLV_HEADER_FLAG_HASVIDEO
                 | FLV_HEADER_FLAG_HASAUDIO)); // flag;enable video audio
  avio_wb32(&flv, 0x09);
  avio_wb32(&flv, 0x00);       // pre tag size

  /* write meta_tag */
  avio_w8(&flv,   0x12);   // 0x12=SCRIPT META,0x08=AUDIO,0x09=VIDEO
  // Length of the message.Number of bytes after StreamID to end of tag
  // (Equal to length of the tag – 11)
  avio_wb24(&flv, 279-11); // size of data part(sum of all parts below)
  avio_wb24(&flv, 0);      // timestamp
  avio_wb32(&flv, 0);      // reserved

  /* first event name as a string */
  avio_w8(&flv, AMF_DATA_TYPE_STRING);
  put_amf_string(&flv, "onMetaData"); // 12 bytes

  /* mixed array (hash) with size and string/type/data tuples */
  avio_w8(&flv,   AMF_DATA_TYPE_MIXEDARRAY);
  avio_wb32(&flv, 2 + 5 + 5);

  // fill in the guessed duration, it'll be corrected later if incorrect
  put_amf_string(&flv, "duration");
  put_amf_double(&flv, 0x52);

  //////////////////////////////////////////////////////////////////////////
  put_amf_string(&flv, "width");
  put_amf_double(&flv, n_width);

  put_amf_string(&flv, "height");
  put_amf_double(&flv, n_height);

  // 视频码率
  put_amf_string(&flv, "videodatarate");
  put_amf_double(&flv, 1024);

  // 视频帧率
  put_amf_string(&flv, "framerate");
  put_amf_double(&flv, 25);

  // 视频编码方式
  put_amf_string(&flv, "videocodecid");
  put_amf_double(&flv, FLV_CODECID_H264);

  //////////////////////////////////////////////////////////////////////////
  put_amf_string(&flv, "audiodatarate");   // 比特率
  put_amf_double(&flv, 16000);

  // 音频采样率
  put_amf_string(&flv, "audiosamplerate");
  put_amf_double(&flv, 8000);

  // 音频采样精度
  put_amf_string(&flv, "audiosamplesize");
  put_amf_double(&flv, 16);

  put_amf_string(&flv, "stereo");
  put_amf_bool(&flv,   0);

  put_amf_string(&flv, "audiocodecid");
  put_amf_double(&flv, FLV_CODECID_PCM_ALAW);

  // data_enc
  //put_amf_string(flv_meta_, "datastream");
  //put_amf_double(flv_meta_, 0.0);

  put_amf_string(&flv, "filesize");
  put_amf_double(&flv, 0); // delayed write

  put_amf_string(&flv, "");
  avio_w8(&flv, AMF_END_OF_OBJECT);
  avio_wb32(&flv, flv.w_pos - 13);

  if (p_file) {
    fwrite(&flv.buffer, 1, flv.w_pos, p_file);
    fflush(p_file);
  }
  return flv.w_pos;
}

int32_t CFlvMux::VideoPacket(uint8_t *p_dst, uint32_t n_dst,
                             bool b_key_frame, uint64_t n_pts,
                             const uint8_t *p_src, uint32_t n_src) {
  FLV_DATA flv;
  flv.buffer = p_dst;
  flv.w_pos = 0;
  flv.n_max = n_dst - sizeof(FLV_DATA);

  avio_w8(&flv,    FLV_TAG_TYPE_VIDEO);
  avio_wb24(&flv,  n_src + 5);
  avio_wb24(&flv,  n_pts);
  avio_w8(&flv,    (n_pts >> 24) & 0x7F); // timestamps are 32 bits _signed_
  avio_wb24(&flv,  0);

  // 5
  avio_w8(&flv,    ((b_key_frame ? 1 : 0) << 4) | 7);
  avio_w8(&flv,    0x01);   // 0=AVC sequence header 1=AVC NALU
  avio_wb24(&flv,  20);     // composition time

  //
  avio_write(&flv, (char*)p_src, n_src);    // data
  avio_wb32(&flv,  n_src + 5 + 11);  // previous tag size

  if (p_file) {
    fwrite(flv.buffer, 1, flv.w_pos, p_file);
    fflush(p_file);
  }
  return flv.w_pos;
}

int32_t CFlvMux::AudioPacket(uint8_t *p_dst, uint32_t n_dst,
                             uint64_t n_pts,
                             const uint8_t *p_src, uint32_t n_src) {
  FLV_DATA flv;
  flv.buffer = p_dst;
  flv.w_pos = 0;
  flv.n_max = n_dst - sizeof(FLV_DATA);

  avio_w8(&flv,   FLV_TAG_TYPE_AUDIO);
  avio_wb24(&flv, n_src + 1);
  avio_wb24(&flv, n_pts);
  avio_w8(&flv,   (n_pts >> 24) & 0x7F); // timestamps are 32 bits _signed_
  avio_wb24(&flv, 0);

  // 1
  avio_w8(&flv, (FLV_CODECID_PCM_ALAW |
                      FLV_SAMPLERATE_SPECIAL | FLV_SAMPLESSIZE_16BIT));

  //
  avio_write(&flv, (char*)p_src, n_src);    // data
  avio_wb32(&flv,  n_src + 1 + 11);  // previous tag size

  if (p_file) {
    fwrite(flv.buffer, 1, flv.w_pos, p_file);
    fflush(p_file);
  }
  return flv.w_pos;
}
