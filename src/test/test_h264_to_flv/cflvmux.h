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
  static char *Packet(char *p_packet,
                      const char *p_head, int n_head,
                      const char *p_data, int n_data,
                      int type, unsigned int timestamp);


  static char *HeaderAndMetaDataTag(char *p_packet,
                                    int width, int height,
                                    int audiodatarate, int audiosamplerate, int audiosamplesize, int audiochannels);



  static char *PacketSpsPps(char *p_packet, char *p_vdo, int n_vdo, bool is_key_frame, unsigned int timestamp);
  static char *PacketVideo(char *p_packet, char *p_vdo, int n_vdo, bool is_key_frame, unsigned int timestamp);


 public:
  CFlvMux();
  int SetSps(const char *p_sps, int n_sps);
  int SetPps(const char *p_pps, int n_pps);

 public:
  int  sps_size_;
  int  pps_size_;
  char sps_pps_[128];
};


#endif  // SHM_CAMERA_H264_CFLVMUX_H
