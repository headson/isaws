#include <stdio.h>
#include <string>
#include "json/json.h"
#include "string.h"
#include "onvif/cfgpro/onvif_kvdb.h"
#include "dispatcher/sync/dpclient_c.h"
//#include <ipc_comm/SystemServerComm.h>
#include "onvif/base/basedefine.h"

using namespace onvifserver;
//#define DEBUG
#ifdef DEBUG
#define DBG(fmt, args...)	printf("Debug " fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define ERR(fmt, args...)	printf("Error " fmt, ##args)

static int ParserStr2Json(std::string &inputstr, Json::Value &outjson) {
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(inputstr, outjson);
  if (!parsingSuccessful) {
		ERR("parsing Json string error\n");
    return -1;
  }
  return 0;
}

static int ParserHWInfo(std::string &data,hw_info_t *phwi) {
  Json::Value jsonvalue;
  int res = ParserStr2Json(data, jsonvalue);
  if (res < 0) {
    return -1;
  }

  int i = 0;
  memset(phwi, 0, sizeof(hw_info_t));
  phwi->write_status = jsonvalue[HW_WRITE_STATUS].asUInt();
  phwi->device_type = jsonvalue[HW_DEV_TYPE].asUInt();
  phwi->exdata_checker = jsonvalue[HW_EXDATA_CHECKER].asInt();

  sscanf(jsonvalue[HW_MAC].asCString(),"%02x:%02x:%02x:%02x:%02x:%02x",
    &phwi->mac[0], &phwi->mac[1], &phwi->mac[2],
    &phwi->mac[3], &phwi->mac[4], &phwi->mac[5]);
 
  sscanf(jsonvalue[HW_SERIALNO].asCString(),"%08x-%08x",
    &phwi->serialno[0], &phwi->serialno[1]);
  
  sscanf(jsonvalue[HW_OEM_INFO].asCString(), "%02x.%02x.%02x.%02x.%02x.%02x.%02x",
    &phwi->oem_info[0], &phwi->oem_info[1], &phwi->oem_info[2], &phwi->oem_info[3],
    &phwi->oem_info[4], &phwi->oem_info[5], &phwi->oem_info[6]);
 
  sscanf(jsonvalue[HW_VERSION].asCString(), "%02x.%02x.%02x.%02x",
    &phwi->hw_version[0], &phwi->hw_version[1],
    &phwi->hw_version[2], &phwi->hw_version[3]);

  phwi->hw_flag = jsonvalue[HW_FLAG].asUInt();
  phwi->exdataSize = jsonvalue[HW_EXDATA_SIZE].asUInt();
  phwi->board_version = jsonvalue[HW_BOARD_VERSION].asInt();
  /*
  for (i = 0; i < 8; i++) {
		phwi->mac[i] = jsonvalue[HW_MAC][i].asInt();
    printf("mac[%d]:%d\n",i,phwi->mac[i]);
  }
  for (i = 0; i < 2; i++) {
    phwi->serialno[i] = jsonvalue[HW_SERIALNO][i].asUInt();
  }
  for (i = 0; i < OEM_INFO_SIZE; i++) {
    phwi->oem_info[i] = jsonvalue[HW_OEM_INFO][i].asInt();
  }
  for (i = 0; i < HW_VERSION_SIZE; i++) {
    phwi->hw_version[i] = jsonvalue[HW_VERSION][i].asInt();
  }*/

  return 0;
}

int GetHwi(hw_info_t *phwi) {
  if (phwi == NULL)
    return -1; 
  char buffer[512]= {0};
  int returnsize = Kvdb_GetKeyAbsolutelyToBuffer(HW_INFO,strlen(HW_INFO),buffer,
                                       sizeof(buffer));
  if(returnsize <= 0) {
    return -1;
  }
	
  std::string result(buffer,returnsize);
	
	return ParserHWInfo(result, phwi);
}


