/************************************************************************
*Author      : Sober.Peng 17-08-01
*Description : 
************************************************************************/
#ifndef _SPSDECODE_H_
#define _SPSDECODE_H_

#include <stdio.h>
#include <math.h>

unsigned int Ue(char *pBuff, unsigned int nLen, unsigned int &nStartBit) {
  //计算0bit的个数
  unsigned int nZeroNum  =  0;
  while (nStartBit < nLen * 8) {
    if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) { //&:按位与，%取余
      break;
    }
    nZeroNum++;
    nStartBit++;
  }
  nStartBit ++;

  //计算结果
  unsigned int dwRet  =  0;
  for (unsigned int i = 0; i<nZeroNum; i++) {
    dwRet <<=  1;
    if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
      dwRet +=  1;
    }
    nStartBit++;
  }
  return (1 << nZeroNum) - 1 + dwRet;
}

int Se(char *pBuff, unsigned int nLen, unsigned int &nStartBit) {
  int UeVal = Ue(pBuff,nLen,nStartBit);
  double k = UeVal;
  int nValue = ceil(k/2); //ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2) = ceil(1.2) = cei(1.5) = 2.00
  if (UeVal % 2 == 0)
    nValue = -nValue;
  return nValue;
}

unsigned int u(unsigned int BitCount, char *buf,unsigned int &nStartBit) {
  unsigned int dwRet  =  0;
  for (unsigned int i = 0; i < BitCount; i++) {
    dwRet <<=  1;
    if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
      dwRet +=  1;
    }
    nStartBit++;
  }
  return dwRet;
}

/************************************************************************
*Description : nal包分离
*Parameters  : ph264[IN] h264数据
*              nh264[IN] h264数据长度
*              frm_type[OUT] 帧类型
*              nal_start[OUT] nal前置长度
*Return      : nal包含nal_start起始指针
************************************************************************/
char *nal_split(const char *ph264, int nh264, int *frm_type, int *nal_start) {
  int   n_0_cnt = 0;    // 0x00 个数
  char *p_nal = const_cast<char*>(ph264);
  while (true) {
    if ((p_nal + 1) > (ph264 + nh264)) {
      return NULL;
    }

    if (*p_nal == 0x01 && n_0_cnt >= 2) {
      p_nal -= n_0_cnt;
      *frm_type = p_nal[n_0_cnt + 1] & 0x1f;
      *nal_start = n_0_cnt + 1;
      return p_nal;
    }

    if (*p_nal == 0x00) {
      n_0_cnt++;
    } else {
      n_0_cnt = 0;
    }
    p_nal++;
  }
  return NULL;
}

/************************************************************************
*Description : 解码SPS,获取视频宽,高
*Parameters  : psps[IN] sps数据 = nal包 - nal_start
*              nsps[IN] sps数据长度
*              width[OUT] 视频宽
*              height[OUT] 视频高
*Return      : nal包含nal_start起始指针
************************************************************************/
bool h264_decode_sps(char *psps, unsigned int nsps, int *width, int *height) {
  unsigned int StartBit = 0;
  int forbidden_zero_bit = u(1, psps, StartBit);
  int nal_ref_idc = u(2, psps, StartBit);
  int nal_unit_type = u(5, psps, StartBit);
  if(nal_unit_type == 7) {
    int profile_idc = u(8, psps, StartBit);
    int constraint_set0_flag = u(1, psps, StartBit);//(buf[1] & 0x80)>>7;
    int constraint_set1_flag = u(1, psps, StartBit);//(buf[1] & 0x40)>>6;
    int constraint_set2_flag = u(1, psps, StartBit);//(buf[1] & 0x20)>>5;
    int constraint_set3_flag = u(1, psps, StartBit);//(buf[1] & 0x10)>>4;
    int reserved_zero_4bits = u(4, psps, StartBit);
    int level_idc = u(8, psps, StartBit);

    int seq_parameter_set_id = Ue(psps, nsps, StartBit);

    if( profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 144) {
      int chroma_format_idc = Ue(psps, nsps, StartBit);
      if(chroma_format_idc == 3)
        int residual_colour_transform_flag = u(1, psps, StartBit);
      int bit_depth_luma_minus8 = Ue(psps, nsps, StartBit);
      int bit_depth_chroma_minus8 = Ue(psps, nsps, StartBit);
      int qpprime_y_zero_transform_bypass_flag = u(1, psps, StartBit);
      int seq_scaling_matrix_present_flag = u(1, psps, StartBit);

      int seq_scaling_list_present_flag[8];
      if(seq_scaling_matrix_present_flag) {
        for(int i = 0; i < 8; i++) {
          seq_scaling_list_present_flag[i] = u(1, psps, StartBit);
        }
      }
    }
    int log2_max_frame_num_minus4 = Ue(psps, nsps, StartBit);
    int pic_order_cnt_type = Ue(psps, nsps, StartBit);
    if( pic_order_cnt_type == 0)
      int log2_max_pic_order_cnt_lsb_minus4 = Ue(psps, nsps, StartBit);
    else if( pic_order_cnt_type == 1) {
      int delta_pic_order_always_zero_flag = u(1, psps, StartBit);
      int offset_for_non_ref_pic = Se(psps, nsps, StartBit);
      int offset_for_top_to_bottom_field = Se(psps, nsps, StartBit);
      int num_ref_frames_in_pic_order_cnt_cycle = Ue(psps, nsps, StartBit);

      int *offset_for_ref_frame = new int[num_ref_frames_in_pic_order_cnt_cycle];
      for(int i  =  0; i < num_ref_frames_in_pic_order_cnt_cycle; i++)
        offset_for_ref_frame[i] = Se(psps, nsps, StartBit);
      delete[] offset_for_ref_frame;
    }
    int num_ref_frames = Ue(psps, nsps, StartBit);
    int gaps_in_frame_num_value_allowed_flag = u(1, psps, StartBit);
    int pic_width_in_mbs_minus1 = Ue(psps, nsps, StartBit);
    int pic_height_in_map_units_minus1 = Ue(psps, nsps, StartBit);

    *width = (pic_width_in_mbs_minus1 + 1) * 16;
    *height = (pic_height_in_map_units_minus1 + 1) * 16;

    return true;
  }

  return false;
}

#endif  // _SPSDECODE_H_
