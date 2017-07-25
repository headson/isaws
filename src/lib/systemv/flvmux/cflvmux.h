/************************************************************************/
/* Author      : SoberPeng 2017-07-05
/* Description :
/************************************************************************/
#ifndef SHM_CAMERA_H264_CFLVMUX_H
#define SHM_CAMERA_H264_CFLVMUX_H

enum FlvTagType {
  FLV_TAG_TYPE_AUDIO = 0x08,
  FLV_TAG_TYPE_VIDEO = 0x09,
  FLV_TAG_TYPE_META  = 0x12,
};

class CFlvMux {
 public:
  static char *Packet(char *p_dst,
                      const char *p_head, int n_head,
                      const char *p_data, int n_data,
                      int type, unsigned int timestamp);


  static char *HeaderAndMetaDataTag(char *p_dst,
                                    int width, int height,
                                    int audiodatarate, int audiosamplerate, int audiosamplesize, int audiochannels);

  static int   VdoHeadSize() {
    return 20;
  }
  static char *PacketVideo(char *p_dst, char *p_vdo, int n_vdo, bool is_key_frame, unsigned int timestamp);
  static char *PacketAudio(char *p_dst, char *p_ado, int n_ado, unsigned int timestamp);

 public:
  CFlvMux();

  int MakeAVCc(char* sps_pps, int sps_size, int pps_size);

 public:
  int  avcc_size_;
  char avcc_[128];
};


#endif  // SHM_CAMERA_H264_CFLVMUX_H
