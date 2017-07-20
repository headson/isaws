/************************************************************************
*Author      : Sober.Peng 17-07-05
*Description :
************************************************************************/
#include "httpsender/dataparse/dataparse.h"
#include "vzbase/helper/stdafx.h"

#include "vzbase/base/helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fstream>

#include <iconv.h>

namespace hs {

// Get Cahced File callback function
void FileCache_Callback(const char *s_path, int n_path,
                        const char *p_data, int n_data,
                        void       *p_usr_arg) {
  if (!s_path || !p_data || !p_usr_arg) {
    LOG(L_ERROR) << "param is null.";
    return;
  }

  ((std::string*)p_usr_arg)->append(p_data, n_data);
}

DataParse::DataParse() {
}

DataParse::~DataParse() {

}

int DataParse::h2i(char* s) {

  int value;
  int c;

  c = ((unsigned char *)s)[0];
  if (isupper(c))
    c = tolower(c);
  value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

  c = ((unsigned char *)s)[1];
  if (isupper(c))
    c = tolower(c);
  value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

  return (value);
}

int DataParse::url_decode(char *str, int len) {

  char *dest = str;
  char *data = str;

  while (len--) {
    if (*data == '+') {
      *dest = ' ';
    } else if (*data == '%' && len >= 2 && isxdigit((int)*(data + 1)) && isxdigit((int)*(data + 2))) {
      *dest = (char)h2i(data + 1);
      data += 2;
      len -= 2;
    } else {
      *dest = *data;
    }
    data++;
    dest++;
  }
  *dest = '\0';
  return dest - str;
}

char* DataParse::url_encode(
  char const *s,
  int len,
  int *new_length) {

  register unsigned char c;
  unsigned char *to, *start;
  unsigned char const *from, *end;

  from = (unsigned char *)s;
  end = (unsigned char *)s + len;
  start = to = (unsigned char *)calloc(1, 3 * len + 1);

  while (from < end) {
    c = *from++;

    if (c == ' ') {
      *to++ = '+';
    } else if ((c < '0' && c != '-' && c != '.') ||
               (c < 'A' && c > '9') ||
               (c > 'Z' && c < 'a' && c != '_') ||
               (c > 'z')) {
      to[0] = '%';
      to[1] = hexchars[c >> 4];
      to[2] = hexchars[c & 15];
      to += 3;
    } else {
      *to++ = c;
    }
  }
  *to = 0;
  if (new_length) {
    *new_length = to - start;
  }
  return (char *)start;
}

void DataParse::OnCachedGetFileCallback(
  const char *path,
  int path_size,
  const char *data,
  int data_size,
  void* user_data) {
  if (NULL != data) {
    std::string datestr = data;
    //((IMAGE_DATA*)user_data)->imagedata = base64_encode((const unsigned char*)data, data_size);
    ((IMAGE_DATA*)user_data)->imagedata =
      const_cast<char*>(vzbase::Base64::Encode(datestr).c_str());
    ((IMAGE_DATA*)user_data)->imagelen = data_size;
  }
  ((IMAGE_DATA*)user_data)->datagetend = true;
}

int DataParse::ReadJsonFile(const char* file_path,Json::Value &value) {
  Json::Reader reader;
  std::ifstream is;
  is.open(file_path,std::ios::binary);
  if(!is.is_open()) {
    return -1;
  }
  if(!reader.parse(is,value))return -1;
  return 0;
}

bool DataParse::JsonReadSysInfoValue(Json::Value &sysInfoJson) {
  Json::Value root;
  if(ReadJsonFile(SYS_JSON_FILE, root) !=0)
    return false;
  if(root[SYSINFO_STR].isNull()) {
    sysInfoJson = root;
  } else {
    sysInfoJson = root[SYSINFO_STR];
  }
  return true;
}

void DataParse::GetSnapImage(const char *path,
                             Json::Value& snapimage_js) {
  std::string imagedata;
  Json::Value triggerimage_js;
  int n_ret = Cached_GetFile(path, strlen(path),
                             FileCache_Callback,
                             &imagedata);
  if (CACHED_RET_SUCCEED == n_ret) {
    std::string imagefile = vzbase::Base64::Encode(imagedata);
    triggerimage_js[JSON_IMAGEFILE] = imagefile.c_str();
    triggerimage_js[JSON_IMAGEFILE_BASE64_LEN] = imagefile.length();
    triggerimage_js[JSON_IMAGEFILE_LEN] = imagedata.length();
    snapimage_js[JSON_IP_ADDRESS] = device_ip_;
    snapimage_js[JSON_TRIGGERIMAGE] = triggerimage_js;
  }
}

void DataParse::GetPlateJson(IVS_RESULT_PARAM *result,
                             Json::Value& platejson,
                             VZ_CenterServer_PlateResult *plateresult,
                             char* devicename) {

  Json::Value info;
  Json::Value alarmInfoPlate;
  Json::Value plateResult;
  alarmInfoPlate[JSON_SERIALNO] = serial_no_;
  alarmInfoPlate[JSON_DEVICENAME] = devicename;
  alarmInfoPlate[JSON_IP_ADDRESS] = device_ip_;
  alarmInfoPlate[JSON_CHANNEL] = 0;

  plateResult[JSON_LICENSE] = vzbase::Gb2312ToUtf8(result->plateInfo.license);

  if (plateresult->plateResultLevel < 2) {
    plateResult[JSON_COLOR_VALUE] = (int)strtol(result->plateInfo.color, NULL, 16);
    plateResult[JSON_CONFIDENCE] = result->plateInfo.nConfidence;
    plateResult[JSON_BRIGHT] = result->plateInfo.nBright;
    plateResult[JSON_CAR_BRIGHT] = result->plateInfo.nCarBright;
    plateResult[JSON_CAR_COLOR] = result->plateInfo.nCarColor;
  }

  plateResult[JSON_COLOR_TYPE] = result->plateInfo.nColor;
  plateResult[JSON_TYPE] = result->plateInfo.nType;
  plateResult[JSON_DIRECTION] = result->plateInfo.nDirection;
  if (plateresult->plateResultLevel < 1) {
    Json::Value location;
    Json::Value rect;
    rect[JSON_LEFT] = result->plateInfo.rcLocation.left;
    rect[JSON_TOP] = result->plateInfo.rcLocation.top;
    rect[JSON_RIGHT] = result->plateInfo.rcLocation.right;
    rect[JSON_BOTTOM] = result->plateInfo.rcLocation.bottom;
    location[JSON_RECT] = rect;
    plateResult[JSON_LOCATION] = location;

    plateResult[JSON_TIMEUSED] = result->plateInfo.nTime;
  }

  if (plateresult->plateResultLevel < 3) {
    Json::Value timeStamp;
    Json::Value timeVal;
    timeVal[JSON_SEC] = result->plateInfo.tvPTS.uTVSec;
    timeVal[JSON_USEC] = result->plateInfo.tvPTS.uTVUSec;
    timeVal[JSON_DECYEAR] = result->plateInfo.struBDTime.bdt_year;
    timeVal[JSON_DECMON] = result->plateInfo.struBDTime.bdt_mon;
    timeVal[JSON_DECDAY] = result->plateInfo.struBDTime.bdt_mday;
    timeVal[JSON_DECHOUR] = result->plateInfo.struBDTime.bdt_hour;
    timeVal[JSON_DECMIN] = result->plateInfo.struBDTime.bdt_min;
    timeVal[JSON_DECSEC] = result->plateInfo.struBDTime.bdt_sec;
    timeStamp[JSON_TIMEVAL] = timeVal;
    plateResult[JSON_TIMESTAMP] = timeStamp;
  }
  plateResult[JSON_TRIGGERTYPE] = result->plateInfo.uBitsTrigType;

  if (plateresult->is_send_image) {
    std::string imagedata = "";
    int n_ret = Cached_GetFile(result->imageSDPath,
                               strlen(result->imageSDPath),
                               FileCache_Callback,
                               &imagedata);
    if (imagedata.size() > 0) {
      std::string imagefile = vzbase::Base64::Encode(imagedata);
      plateResult[JSON_IMAGEFILE] = imagefile;
      plateResult[JSON_IMAGEFILE_LEN] = imagefile.length();
    }
    LOG(L_INFO) << "get image "<<imagedata.length();
  } else {
    char file_path[256] = { 0 };
    strcat(file_path, "/mmc");
    char* p = strstr(result->imageSDPath, "/media/mmcblk0p1");
    if (p) {
      p += strlen("/media/mmcblk0p1");
      strcat(file_path, p);
      printf("%s\n", file_path);

      int rlen;
      char* escaped_path = url_encode(file_path, strlen(file_path), &rlen);
      plateResult[JSON_IMAGEPATH] = escaped_path;
      if (escaped_path) free(escaped_path);
    }
  }

  if (plateresult->is_send_small_image) {
    char file_path[256] = { 0 };
    strcat(file_path, result->imageFragmentPath);

    std::string smallimagedata;
    int n_ret = Cached_GetFile(file_path,
                               strlen(file_path),
                               FileCache_Callback,
                               &smallimagedata);
    if (CACHED_RET_SUCCEED == n_ret) {
      std::string smallimagefile = vzbase::Base64::Encode(smallimagedata);
      plateResult[JSON_IMAGEFRAGMENT_FILE] = smallimagefile.c_str();
      plateResult[JSON_IMAGEFRAGENMENT_FILELEN] = smallimagefile.length();
    }
  } else {
  }

  Json::Value prResult;
  prResult[JSON_PLATE_RESULT] = plateResult;
  alarmInfoPlate[JSON_RESULT] = prResult;
  platejson[JSON_ALARMINFO_PLATE] = alarmInfoPlate;
}

void DataParse::GetDeviceRegJson(
  Json::Value& deviceregjson,
  char* devicename,
  char* username,
  char* passwd,
  char* httpport) {

  deviceregjson[JSON_DEVICE_NAME] = devicename;
  deviceregjson[JSON_IP_ADDRESS] = device_ip_;
  deviceregjson[JSON_PORT] = httpport;
  deviceregjson[JSON_USERNAME] = username;
  deviceregjson[JSON_PASSWORD] = passwd;
  deviceregjson[JSON_SERIALNO] = serial_no_;
  deviceregjson[JSON_CHANNEL_NUM] = 1;
}

//sprintf(cFullRmtPath, "ftp://%s/%s/%s(%s)/channel_%d/%s/%s/", pFTPCfg->ServerIP,
//	pFTPCfg->ServerFoldPath, pSYSCfg->MyTitle, pSYSCfg->MyIP, Channel,
//	strRecType, strDate);

static char *GetFileNameFormFullPath(char *pPath) {
  if(pPath == NULL)
    return NULL;

  char *p,*q;
  p = strchr(pPath, '/');
  if(p == NULL) {
    return pPath;
  } else {
    p++;
    while(1) {
      q = strchr(p, '/');
      if(q == NULL) {
        return p;
      } else {
        q++;
        p = q;
      }
    }
  }
}

int DataParse::Code_Convert(char *from_charset, char *to_charset,
                            char *in_buf, int in_len, char *out_buf, int out_len) {
  iconv_t cd;
  int rc = 0;
  char **pin = &in_buf;
  char **pout = &out_buf;

  cd = iconv_open(to_charset, from_charset);
  if (cd == (iconv_t)-1) {
    return -1;
  }
  memset(out_buf, 0, out_len);
  if (iconv(cd, (const char**)pin, (size_t *)&in_len,
            pout, (size_t *)&out_len) == (size_t)-1) {
    rc = -1;
  }
  iconv_close(cd);

  return rc;
}

int DataParse::UTF8_To_GB2312(char *in_buf, int in_len, char *out_buf,
                              int out_len) {
  if (in_buf == NULL || out_buf == NULL) {
    return -1;
  }
  return Code_Convert((char *)"utf-8", (char *)"gb2312", in_buf, in_len,
                      out_buf, out_len);
}

void DataParse::GetFtpPostJson(UserGetImgInfo &imginfo,
                               Json::Value& ftppostjson,
                               char* ftpip,
                               char* userfilepath,
                               char* postfilepath,
                               std::string title) {
  std::string ftppostimagedata;
  int n_ret = Cached_GetFile(postfilepath,
                             strlen(postfilepath),
                             FileCache_Callback,
                             &ftppostimagedata);
  if (CACHED_RET_FAILURE == n_ret) {
    return;
  }

  memcpy(imginfo.pdata,ftppostimagedata.c_str(),ftppostimagedata.length());
  imginfo.datasize = ftppostimagedata.length();
  //ftppostjson[JSON_FTP_FILE_DATA] = ftppostimagedata;
  //ftppostjson[JSON_FTP_FILE_SIZE] = ftppostimagedata.length();
  char ftpfilepath[256] = {0};
  char datestr[128] = {0};
  struct tm *pTm = NULL;
  time_t itime;
  itime = time(NULL);
  pTm = localtime(&itime);
  sprintf(datestr,"%04d-%02d-%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday);

  char *pName = GetFileNameFormFullPath(postfilepath);
  LOG(L_INFO) << "title : " << title;
  char title_gb2312[512] = {0};
  if(title.length() <= 256)
    UTF8_To_GB2312((char*)title.c_str(), title.length(), title_gb2312,512);
  else
    strcpy(title_gb2312,"IVS");
  //utf8 -> gb2312
  sprintf(ftpfilepath, "ftp://%s/%s/%s(%s)/channel_0/plate/%s/%s",
          ftpip,userfilepath,title_gb2312,device_ip_,datestr,pName);
  ftppostjson[JSON_FTP_URL] = ftpfilepath;
}

void DataParse::GetRs485Json(
  Json::Value& rs485json,
  int serialchannel,
  const char* serialdata,
  int datalen) {

  Json::Value serialData;

  serialData[JSON_SERIALNO] = serial_no_;
  serialData[JSON_CHANNEL] = 0;
  serialData[JSON_IP_ADDRESS] = device_ip_;

  //char* base64 = base64_encode((unsigned char*)serialdata, datalen);

  std::string serialstr = serialdata;

  //char*base64 = const_cast<char*>(vzbase::Base64::Encode(serialstr).c_str());
  //if (!base64) return;
  std::string base64 = vzbase::Base64::Encode(serialstr);

  serialData[JSON_SERIAL_CHANNEL] = serialchannel;
  serialData[JSON_DATA] = base64;
  serialData[JSON_DATA_LEN] = datalen;

  rs485json[JSON_SERIAL_DATA_HEAD] = serialData;
}

void DataParse::GetGIOJson(
  Json::Value& giojson,
  TRIGGER_REULST* result,
  char* devicename) {

  Json::Value info;
  Json::Value alarmGioIn;
  Json::Value triggerResult;

  alarmGioIn[JSON_SERIALNO] = serial_no_;
  alarmGioIn[JSON_DEVICENAME] = devicename;
  alarmGioIn[JSON_IP_ADDRESS] = device_ip_;

  triggerResult[JSON_VALUE] = result->result;
  triggerResult[JSON_SOURSE] = result->source;
  Json::Value ioResult;
  ioResult[JSON_TRIGGER_RESULT] = triggerResult;
  alarmGioIn[JSON_RESULT] = ioResult;

  giojson[JSON_ALARM_GIOIN] = alarmGioIn;
}

void DataParse::iInitHwiInfo(Json::Value hw_info) {

  LOG(L_INFO) << "serial_no_";
  if(hw_info["serialno"].isNull()) {
    sprintf(serial_no_, "00000000-00000000");
    return;
  }
  sprintf(serial_no_, "%s",
          hw_info["serialno"].asCString());
  LOG(L_INFO) << "serial_no_ : " << serial_no_;
//#ifdef WIN32
//  unsigned int serialno1 = 255;
//  unsigned int serialno2 = 255;
//
//  sprintf(serial_no_, "%08x-%08x", serialno1, serialno2);
//#else
//  InitHwiShareMem();
//
//  hw_info_t hwi;
//  GetHwi(&hwi);
//  if (hwi.write_status == HW_INFO_WRITTED) {
//
//    sprintf(serial_no_, "%08x-%08x", hwi.serialno[0], hwi.serialno[1]);
//  }
//#endif
}
void DataParse::SetDeviceIp(char* deviceip) {

  strcpy(device_ip_, deviceip);
}
};