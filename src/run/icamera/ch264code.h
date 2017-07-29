#pragma once
extern "C" {
#include "x264/x264.h"
#include "x264/x264_config.h"
};

class CH264Coder {
 private:
  x264_param_t param;
  x264_t      *x264;

  int  width;
  int  height;

  char sps_pps_data[300];
  int  sps_pps_length;

  char sps_pps_data_ts[300];// have nal start code ，标准的h264数据
  int  sps_pps_length_ts;

  char sei_data[200];
  int  sei_length;

 public:
  CH264Coder(int width, int height, int maxBitrate);
  ~CH264Coder();

  void    SaveSei(char* data, int length);

  x264_t* x264encoder();
  int     get_sps_pps(char* buffer);
  int     get_sps_pps_ts(char* buffer);
  char*   get_sei(char* buffer);
};

