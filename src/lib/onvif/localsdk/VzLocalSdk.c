#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include "onvif/localsdk/VzLocalSdk.h"
#include "time.h"
//#include "net_config.h"
#include "system_default.h"
#include "onvif/cfgpro/onvif_kvdb.h"
#include <unistd.h>
#ifdef WIFI_LAN
#include "ipc_comm/SystemServerComm.h"
#endif
#include <ipc_comm/WisStreamerComm.h>
#include "ipc_comm/AVServerComm.h"
#include "cacheserver/client/cachedclient_c.h"
#include "onvif/cfgpro/onvif_cfgfile_mng.h"
#include "onvif/cfgpro/onvif_dispatcher.h"
#include "onvif/bm/util.h"

//#include "VzRuleAlarm.h" 

#define RESOLV_CONF	"/etc/resolv.conf"
#define ETH0_NAME "eth0"
#define MAX_SDLIST_LENGTH	(64*1024)
#define LOCAL_CHANNEL_NUM	2
#define MAX_GOVLENGTH	400
#define MIN_GOVLENGTH	1
#define MAX_VIDEOSOURCE_WITH	1280
#define MAX_VIDEOSOURCE_HEIGHT	720

#ifdef DHCP_SUPPORT
#ifdef EMBED
#define NTP_CONF		"/etc/config/ntp.conf"
#else
#define NTP_CONF		"/mnt/usr/etc/ntp.conf"
#endif
#endif

typedef struct {
  int datasize;
  int bufsize;
  char *pdata;
} GetSnapShotInfo;

//extern Onvif_Info* GetOnvifInfo();


/**
*	@brief 初始化SDK
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK_init() {
  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief 释放SDK
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK_GetErr() {
  return gLocalErr;
}

int LocalSDK_CleanUp() {
  return VZ_LOCALAPI_SUCCESS;
}



Onvif_Info* LocalSDK_GetOnvifInfo() {
  return GetOnvifInfo();
}


static int LocalSDK__AddVideoAnalytics2Profile(Onvif_Info* pOnvifInfo,
                                               char *cfgToken,int channel,int index) {
  int i = 0;
  int addindex = 0;

  if(cfgToken == NULL) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  for( i = 0; i<MAX_VIDEO_ANALYTICS_NUM; i++) {
    if(pOnvifInfo->VideoAnalytics[i].used
       && (strcmp(pOnvifInfo->VideoAnalytics[i].VACfg.token,cfgToken)==0)) {
      addindex = i;
      break;
    }
  }

  if(i >= MAX_VIDEO_ANALYTICS_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  if (pOnvifInfo->Profiles[channel].meta_profile[index].VA_ID != addindex) {
    int deindex = pOnvifInfo->Profiles[channel].meta_profile[index].VA_ID;

    if ((deindex >= 0)&& (pOnvifInfo->VideoAnalytics[deindex].VACfg.UseCount > 0)) {
      vz_onvif_VideoAnalytics value;
      memcpy(&value,&pOnvifInfo->VideoAnalytics[deindex],
             sizeof(vz_onvif_VideoAnalytics));
      value.VACfg.UseCount --;

      if(DoOnvifSetVideoAnalytics(&value,deindex) <0 ) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }

    vz_onvif_VideoAnalytics add_value;
    memcpy(&add_value,&pOnvifInfo->VideoAnalytics[addindex],
           sizeof(vz_onvif_VideoAnalytics));
    add_value.VACfg.UseCount ++;

    if(DoOnvifSetVideoAnalytics(&add_value,addindex) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    vz_onvif_Profiles pro_value;
    memcpy(&pro_value,&pOnvifInfo->Profiles[channel],sizeof(pro_value));
    pro_value.meta_profile[index].VA_ID = addindex;

    if(DoOnvifSetProfiles(&pro_value,channel) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

static int LocalSDK__DelVideoAnalyticsFromProfile(Onvif_Info* pOnvifInfo,
                                                  int channel,int index) {
  int deindex = pOnvifInfo->Profiles[channel].meta_profile[index].VA_ID;

  if ((deindex >= 0)&& pOnvifInfo->VideoAnalytics[deindex].VACfg.UseCount > 0) {
    vz_onvif_VideoAnalytics value;
    memcpy(&value,&pOnvifInfo->VideoAnalytics[deindex],
           sizeof(vz_onvif_VideoAnalytics));
    value.VACfg.UseCount --;

    if(DoOnvifSetVideoAnalytics(&value,deindex) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  vz_onvif_Profiles pro_value;
  memcpy(&pro_value,&pOnvifInfo->Profiles[channel],sizeof(pro_value));
  pro_value.meta_profile[index].VA_ID = -1;

  if(DoOnvifSetProfiles(&pro_value,channel) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  //如果要移除的通道开启算法分析,则先关闭分析
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[channel].enableAlg) {
    int ret=0;
    int enable =0;
    int curchannel = sys_source_cfg[channel].userID;

    if(sys_source_cfg[channel].streamType == AVS_CAP_IPC) {
      ret=ControlSystemDataRemoteChannel(curchannel,SYS_MSG_SET_REMOTE_VIDEOANALYTICS,
                                         &enable, sizeof(enable));

    } else {
      ret=ControlSystemDataLocalChannel(curchannel,SYS_MSG_SET_LOCAL_VIDEOANALYTICS,
                                        &enable, sizeof(enable));
    }

    if(ret < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}


static int LocalSDK__LookUp_VideoSourceToken(char *VideoSourceToken,
                                             Onvif_Info* pOnvifInfo,int *channel,int *index) {
  int k=0;
  int j=0;
  int flag=0;

  if(!VideoSourceToken || !pOnvifInfo)
    return VZ_LOCALAPI_FAILED;

  for(j=0; j<MAX_CHANNEL; j++) {
    for(k=0; k<MAX_PROFILE_NUM; k++) {
      if(strlen(VideoSourceToken)
         &&(strcmp(VideoSourceToken,
                   pOnvifInfo->Profiles[j].meta_profile[k].VideoSourceConfig.token)==0)) {
        flag =1;
        *channel=j;
        *index=k;
        break;
      }
    }

    if(flag)
      break;
  }

  if(j != MAX_CHANNEL)
    return VZ_LOCALAPI_SUCCESS;

  else
    return VZ_LOCALAPI_FAILED;
}

static int LocalSDK__LookUp_ProfileToken(char *ProfileToken,
                                         Onvif_Info* pOnvifInfo,int *channel,int *index) {
  int k=0;
  int j=0;
  int flag=0;

  if(!ProfileToken || !pOnvifInfo)
    return VZ_LOCALAPI_FAILED;

  for(j=0; j<MAX_CHANNEL; j++) {
    for(k=0; k<MAX_PROFILE_NUM; k++) {
      if(strlen(ProfileToken)
         && (strcmp(ProfileToken,pOnvifInfo->Profiles[j].meta_profile[k].token)==0)) {
        flag =1;
        *channel=j;
        *index=k;
        break;
      }
    }

    if(flag)
      break;
  }

  if(j != MAX_CHANNEL)
    return VZ_LOCALAPI_SUCCESS;

  else
    return VZ_LOCALAPI_FAILED;
}

static int LocalSDK__LookUp_VideoEncoderToken(char *VideoEncoderToken,
                                              Onvif_Info* pOnvifInfo,int *channel,int *index) {
  int k=0;
  int j=0;
  int flag=0;

  if(!VideoEncoderToken || !pOnvifInfo)
    return VZ_LOCALAPI_FAILED;

  for(j=0; j<MAX_CHANNEL; j++) {
    for(k=0; k<MAX_PROFILE_NUM; k++) {
      if(strlen(VideoEncoderToken)
         && (strcmp(VideoEncoderToken,
                    pOnvifInfo->Profiles[j].meta_profile[k].VideoEncoderConfig.token)==0)) {
        flag =1;
        *channel=j;
        *index=k;
        break;
      }
    }

    if(flag)
      break;
  }

  if(j != MAX_CHANNEL)
    return VZ_LOCALAPI_SUCCESS;

  else
    return VZ_LOCALAPI_FAILED;
}


static int LocalSDK_CheckOut_TimeZone(char *TimeZone) {
  char *temp = NULL;
  char flag = 0;

  if(TimeZone[0] ==
     ':') { //the time zone information should be read from a file:
    /*i do not know how to judge the TimeZone right or not when the time zone information was read from a file */
    /*
    :[filespec]
    If the file specification filespec is omitted, the time zone information is read from
    the file localtime in the system timezone directory, which nowadays usually is
    /usr/share/zoneinfo. This file is in tzfile(5) format. If filespec is given, it specifies
    another tzfile(5)-format file to read the time zone information from.
    If filespec does not begin with a `/', the file specification is relative
    to the system timezone directory.
    */
  } else {
    temp = TimeZone;
    char time[3][3]= {{0},{0},{0}};
    int i = 0;
    int j =0;

    while(*temp != '\0') { //judge the number exist or not
      if(*temp>='0'&&*temp<='9') {
        if(i >= 3) {
          fprintf( stderr,"only check hour minute second\n");
          break;
        }

        time[i][j++]=*temp;

        if(*(temp+1)>='0'&&*(temp+1)<='9') {
          temp++;
          time[i][j++]=*temp;

          if(*(temp+1)>='0'&&*(temp+1)<='9') {
            flag =1;
            break;
          }
        }

        i++;
        j =0;
      }

      temp++;

      if(i > 0 &&*temp!=':') {
        break;
      }

    }

    if(flag) {
      return VZ_LOCALAPI_TZ_ERR;

    } else {
      if(!strlen(time[0])) { //no number
        return VZ_LOCALAPI_TZ_ERR;

      } else {
        int hour = atoi(time[0]);

        if((hour<0)||(hour>24)) {
          return VZ_LOCALAPI_TZ_ERR;
        }

        if(strlen(time[1])) {
          int minute = atoi(time[1]);

          if(minute<0 ||minute >60)
            return VZ_LOCALAPI_TZ_ERR;
        }

        if(strlen(time[2])) {
          int second = atoi(time[2]);

          if(second<0 ||second >60)
            return VZ_LOCALAPI_TZ_ERR;
        }
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief set system Date and Time
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/

int LocalSDK_SetSystemDateAndTime(LocalSDK_SystemDateAndTime
                                  *SystemDateAndTime) {
  /*
  	int SdkResult = VZ_LOCALAPI_SUCCESS;
  	Onvif_Info* pOnvifInfo = GetOnvifInfo();
  	if(SystemDateAndTime->DateTimeType == (__u8)Vz__SetDateTimeType__NTP)
  	{
  		SysInfo* pSysInfo = GetSysInfo();
  		if(strlen(pSysInfo->device.net.ntp.server_name) == 0)
  		{
  			gLocalErr = VZ_LOCALAPI_NTP_ERR;
  			return VZ_LOCALAPI_FAILED;
  		}

  		if((pSysInfo->device.net.ntp.enable == 0)||(pSysInfo->device.net.ntp.frequency == 0))
  		{
  			Ntp_Config_Data ntp;
  			memset(&ntp,0x0,sizeof(Ntp_Config_Data));
  			strcpy(ntp.server_name,pSysInfo->device.net.ntp.server_name);
  			ntp.frequency = NTP_FREQUENCY_DEFAULT;
  			ntp.enable =1;
  			if (ControlSystemData(SYS_MSG_SET_SNTP, (void *)&ntp, sizeof(ntp)) < 0)
  			{
  				gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
  				return VZ_LOCALAPI_FAILED;
  			}
  		}
  		pOnvifInfo->time_cfg.DateTimeType = vz__SetDateTimeType__NTP;
  		SdkResult = LocalSDK_SetTimeZone(SystemDateAndTime->TimeZone,&(pOnvifInfo->time_cfg));
  		if(SdkResult != VZ_LOCALAPI_SUCCESS)
  		{
  			gLocalErr = SdkResult;
  			return VZ_LOCALAPI_FAILED;
  		}
  		return VZ_LOCALAPI_SUCCESS;

  		//NTP不支持,需要返回错误
  	}
  	else
  	{
  		SdkResult = LocalSDK_CheckOut_SystemDateTime(&SystemDateAndTime->UTCDateTime);
  		if(SdkResult != VZ_LOCALAPI_SUCCESS)
  		{
  			gLocalErr = SdkResult;
  			return VZ_LOCALAPI_FAILED;
  		}
  		pOnvifInfo->time_cfg.DateTimeType = Vz__SetDateTimeType__Manual;

  		struct tm *tm;
  		struct tm tmlocal;
  		struct tm tmutc;
  		time_t now;
  		SdkResult = LocalSDK_SetTimeZone(SystemDateAndTime->TimeZone,&(pOnvifInfo->time_cfg));
  		if(SdkResult != VZ_LOCALAPI_SUCCESS)
  		{
  			gLocalErr = SdkResult;
  			return VZ_LOCALAPI_FAILED;
  		}

  		now = time(NULL);
  		tm = gmtime(&now);
  		memcpy(&tmutc,tm,sizeof(struct tm ));
  		tm = localtime(&now);
  		memcpy(&tmlocal,tm,sizeof(struct tm ));
  		int localbig = 0;
  		if((tmlocal.tm_year > tmutc.tm_year)
  			||(tmlocal.tm_mon > tmutc.tm_mon)
  			||(tmlocal.tm_mday > tmutc.tm_mday)
  			||(tmlocal.tm_hour > tmutc.tm_hour)
  			||(tmlocal.tm_min > tmutc.tm_min)
  			||(tmlocal.tm_sec > tmutc.tm_sec))
  		{
  			localbig = 1;
  		}
  		int year = SystemDateAndTime->UTCDateTime.Date.Year;
  		int month = SystemDateAndTime->UTCDateTime.Date.Month;
  		int day = SystemDateAndTime->UTCDateTime.Date.Day;
  		int hour = SystemDateAndTime->UTCDateTime.Time.Hour;
  		int min = SystemDateAndTime->UTCDateTime.Time.Minute;
  		int sec = SystemDateAndTime->UTCDateTime.Time.Second;

  		tm->tm_sec = sec + 3;
  		if((sec + 3) >=60)
  		{
  			tm->tm_sec -= 60;
  			min += 1;
  		}
  		tm->tm_min = min;
  		if(tm->tm_min >= 60)
  		{
  			tm->tm_min -= 60;
  			hour += 1;
  		}
  		tm->tm_hour = hour;
  		if(tm->tm_hour >= 24)
  		{
  			tm->tm_hour -= 24;
  			day += 1;
  		}
  		tm->tm_mday = day;
  		if(tm->tm_mday > 31)
  		{
  			tm->tm_mday -= 31;
  			month += 1;
  		}
  		if(month > 12)
  		{
  			month -= 12;
  			year += 1;
  		}
  		year = (year>1900) ? year-1900 : 0;
  		tm->tm_year = year;
  		month = (month>0) ? month-1 : 0;
  		tm->tm_mon = month;

  		tm->tm_isdst = SystemDateAndTime->DaylightSavings;
  		now=0;
  		if ((now = mktime(tm)) < 0)
  			return -1;

  		if(localbig)
  			now = now + abs(timezone);
  		else
  			now = now - abs(timezone);

  		//ControlSystemData(SFIELD_SCHEDULE_STOP, (void *)&v2, sizeof(v2));
  #ifdef SESSION
  		session_update();
  #endif
  		stime(&now);
  		system("hwclock -uw");
  #ifdef SESSION
  		session_update_time();
  #endif
  		//ControlSystemData(SFIELD_SCHEDULE_RESUME, (void *)&v2, sizeof(v2));
  		return VZ_LOCALAPI_SUCCESS;
  	}
  	*/
  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief time format transform
*  @return null
*/
void tm2datetime(struct tm *t,LocalSDK_DateTime *datetime) {
  datetime->Date.Year = t->tm_year+1900;
  datetime->Date.Month = t->tm_mon+1;
  datetime->Date.Day = t->tm_mday;
  datetime->Time.Hour = t->tm_hour;
  datetime->Time.Minute = t->tm_min;
  datetime->Time.Second = t->tm_sec;
  //printf("%s,%d,(%d-%d-%d %d:%d:%d)!!!!!!!!!!!\n",__FUNCTION__,__LINE__,t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
}

static void LocalSDK_get_timezone(char * tz, int len) {
  int time_zone;
  int local_hour, utc_hour;
  time_t time_utc;
  struct tm *tm_local;
  struct tm *tm_utc;

  // Get the UTC time
  time(&time_utc);

  // Get the local time
  tm_local = localtime(&time_utc);
  local_hour = tm_local->tm_hour;

  // Change it to GMT tm
  tm_utc = gmtime(&time_utc);
  utc_hour = tm_utc->tm_hour;

  time_zone = local_hour - utc_hour;

  if (time_zone < -12) {
    time_zone += 24;

  } else if (time_zone > 12) {
    time_zone -= 24;
  }

  snprintf(tz, len, "PST%dPDT", time_zone);
}

/**
*	@brief get system Date and Time
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
void LocalSDK__GetSystemDateAndTime(LocalSDK_SystemDateAndTime
                                    *SystemDateAndTime) {
  time_t tnow;
  struct tm *gmtmnow;
  struct tm *loctmnow;
  time(&tnow);

  gmtmnow = gmtime(&tnow);
  tm2datetime(gmtmnow,&SystemDateAndTime->UTCDateTime);
  loctmnow = localtime(&tnow);
  tm2datetime(loctmnow,&SystemDateAndTime->LocalDateTime);
  SystemDateAndTime->DaylightSavings = loctmnow->tm_isdst;
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  //get the TimeZone info
  //if(strlen(pOnvifInfo->time_cfg.TZ) == 0)
  {
    LocalSDK_get_timezone(pOnvifInfo->time_cfg.TZ,sizeof(pOnvifInfo->time_cfg.TZ));
  }
  strcpy(SystemDateAndTime->TimeZone , pOnvifInfo->time_cfg.TZ);
  SystemDateAndTime->DateTimeType = pOnvifInfo->time_cfg.DateTimeType;
}

/**
*	@brief set system Factory Default
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/

int LocalSDK__SetSystemFactoryDefault(__u8 value) {
  if(ControlSystemData(SYS_MSG_SET_DEMOCFG, (void *)&value, sizeof(value)) <0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief System Reboot
*  @return null
*/

void LocalSDK__SystemReboot(void) {
  system("/sbin/reboot\n");
}

/**
*	@brief Set Scopes
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/

int LocalSDK__SetScopes(LocalSDK_ScopesInfo* ScopesInfo) {
  if(ScopesInfo->sizeScopes > ADDTIONAL_SCOPE_LEN) {
    gLocalErr = VZ_LOCALAPI_SCOPELIST_EXCEED;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_Scopes scopes;
  memset(&scopes,0,sizeof(scopes));
  int i=0;

  if(ScopesInfo->sizeScopes > 0) {
    for(; i<ScopesInfo->sizeScopes && i<ADDTIONAL_SCOPE_LEN ; i++) {
      //一些不能改变和添加的scope,现在假定为hardware
      if(strncmp(ScopesInfo->Scopes[i], "onvif://www.onvif.org/hardware",
                 strlen("onvif://www.onvif.org/hardware"))==0) {
        gLocalErr = VZ_LOCALAPI_SCOPE_OVERWRITE;
        return VZ_LOCALAPI_FAILED;
      }

      int len = strlen(ScopesInfo->Scopes[i]);

      if(len>0 && len < SCOPE_MAX_LEN) {
        strcpy(scopes.addtional_scopes[scopes.add_len],ScopesInfo->Scopes[i]);
        scopes.add_len++;

      } else if(len==0) {
        continue;

      } else {
        gLocalErr = VZ_LOCALAPI_SCOPE_EXCEED;
        return VZ_LOCALAPI_FAILED;
      }
    }

    if(DoOnvifSetScopes(&scopes) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief add Scopes
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/

int LocalSDK__AddScopes(LocalSDK_ScopesInfo* ScopesInfo) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  if(ScopesInfo->sizeScopes + pOnvifInfo->scopes.add_len > ADDTIONAL_SCOPE_LEN) {
    gLocalErr = VZ_LOCALAPI_SCOPELIST_EXCEED;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_Scopes scopes;
  memcpy(&scopes,&pOnvifInfo->scopes,sizeof(scopes));

  int i=0;

  for(; i<ScopesInfo->sizeScopes && scopes.add_len<=ADDTIONAL_SCOPE_LEN ; i++) {
    //一些不能改变和添加的scope,现在假定为hardware
    if(strncmp(ScopesInfo->Scopes[i], "onvif://www.onvif.org/hardware",
               strlen("onvif://www.onvif.org/hardware"))==0) {
      gLocalErr = VZ_LOCALAPI_SCOPE_OVERWRITE;
      return VZ_LOCALAPI_FAILED;
    }

    int len = strlen(ScopesInfo->Scopes[i]);

    if(len>0 && len < SCOPE_MAX_LEN) {
      //start: checkout scopes were exist or not(no checkout the fixed partion)
      int j = 0;

      for(; j<scopes.add_len; j++) {
        if(strcmp(scopes.addtional_scopes[j],ScopesInfo->Scopes[i])== 0) {
          gLocalErr = VZ_LOCALAPI_SCOPE_OVERWRITE;
          return VZ_LOCALAPI_FAILED;
        }
      }

      //end: checkout scopes were exist or nit
      strcpy(scopes.addtional_scopes[scopes.add_len],ScopesInfo->Scopes[i]);
      scopes.add_len++;

    } else if(len==0) {
      continue;

    } else {
      gLocalErr = VZ_LOCALAPI_SCOPE_EXCEED;
      return VZ_LOCALAPI_FAILED;
    }
  }

  if(DoOnvifSetScopes(&scopes) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief Remove Scopes
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK__RemoveScopes(LocalSDK_ScopesInfo* ScopesInfo) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_Scopes scopes;
  memcpy(&scopes,&pOnvifInfo->scopes,sizeof(scopes));

  int i=0,j=0;
  int add_len = 0;
  int FoundFlag;

  for(; i<ScopesInfo->sizeScopes; i++) {
    for(j=0; j<scopes.fixed_len; j++) {
      if(strcmp(scopes.fixed_scopes[j],ScopesInfo->Scopes[i])==0) {
        gLocalErr = VZ_LOCALAPI_SCOPE_OVERWRITE;
        return VZ_LOCALAPI_FAILED;
      }
    }

    //删除
    FoundFlag = 0;

    for(j=0; j<scopes.add_len; j++) {
      if(strcmp(scopes.addtional_scopes[j],ScopesInfo->Scopes[i])==0) {
        strcpy(scopes.addtional_scopes[j],"");
        FoundFlag = 1;
      }
    }

    if(!FoundFlag) {
      gLocalErr = VZ_LOCALAPI_SCOPE_NONEXIST;
      return VZ_LOCALAPI_FAILED;
    }
  }

  int last_empty_str_index=scopes.add_len;

  //排序
  for(j=0; j<scopes.add_len; j++) {
    if(strlen(scopes.addtional_scopes[j])!=0) {
      if(last_empty_str_index <j) {
        strcpy(scopes.addtional_scopes[last_empty_str_index],
               scopes.addtional_scopes[j]);
        strcpy(scopes.addtional_scopes[j],"");
        last_empty_str_index++;
      }

      add_len++;

    } else {
      if(last_empty_str_index >j)
        last_empty_str_index = j;
    }
  }

  //长度修正
  scopes.add_len = add_len;

  if(DoOnvifSetScopes(&scopes) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief Create Users
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK__CreateUsers( LocalSDK_UsersInfo *CreateUsers) {
  Acount_t sysacounts[ACOUNT_NUM];

  if(ControlSystemData(MSG_GET_USER, sysacounts,
                       sizeof(Acount_t)*ACOUNT_NUM) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  add_user_t acount;
  int i,j;
  int acount_avaible = 0;

  for(i=0; i<ACOUNT_NUM; i++) {
    if(!strlen(sysacounts[i].user)) {
      acount_avaible++;
    }
  }

  for(i=0; i<CreateUsers->sizeUser; i++) {
    for(j=0; j<ACOUNT_NUM; j++) {
      if(strlen(sysacounts[j].user)!=0
         && strcmp(sysacounts[j].user,CreateUsers->UserInfo[i].Username)==0) {
        gLocalErr = VZ_LOCALAPI_USERNAME_CLASH;
        return VZ_LOCALAPI_FAILED;
      }
    }

    if(acount_avaible==0) {
      gLocalErr = VZ_LOCALAPI_USER_EXCEED;
      return VZ_LOCALAPI_FAILED;
    }

    if(CreateUsers->UserInfo[i].UserLevel>2) {
      gLocalErr = VZ_LOCALAPI_USER_ANONYMOUS;
      return VZ_LOCALAPI_FAILED;
    }

    int user_len = strlen(CreateUsers->UserInfo[i].Username);

    if(user_len>=USR_LEN ) {
      gLocalErr = VZ_LOCALAPI_USERNAME_LONG;
      return VZ_LOCALAPI_FAILED;

    } else if(user_len == 0 ) {
      gLocalErr = VZ_LOCALAPI_USERNAME_SHORT;
      return VZ_LOCALAPI_FAILED;
    }

    if(CreateUsers->UserInfo[i].Password) {
      int pass_len = strlen(CreateUsers->UserInfo[i].Password);

      if(pass_len>=PW_LEN ) {
        gLocalErr = VZ_LOCALAPI_PASSWORD_LONG;
        return VZ_LOCALAPI_FAILED;

      } else if(pass_len < VZ_PASSWORD_MIN_NUM) {
        gLocalErr = VZ_LOCALAPI_PASSWORD_SHORT;
        return VZ_LOCALAPI_FAILED;
      }

    } else {
      gLocalErr = VZ_LOCALAPI_PASSWORD_SHORT;
      return VZ_LOCALAPI_FAILED;
    }

    acount_avaible--;
  }

  for(i=0; i<CreateUsers->sizeUser; i++) {
    strcpy(acount.user_id,CreateUsers->UserInfo[i].Username);

    if(CreateUsers->UserInfo[i].Password) {
      strcpy(acount.password,CreateUsers->UserInfo[i].Password);

    } else {
      strcpy(acount.password,"");
    }

    acount.authority = CreateUsers->UserInfo[i].UserLevel;

    if(ControlSystemData(SYS_MSG_ADD_USER,&acount,sizeof(acount))<0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief Delete Users
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK__DeleteUsers( LocalSDK_UsersInfo *DeleteUsers) {
  Acount_t sysacounts[ACOUNT_NUM];

  if(ControlSystemData(MSG_GET_USER, sysacounts,
                       sizeof(Acount_t)*ACOUNT_NUM) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int i,j;
  int recognized;

  for(i=0; i<DeleteUsers->sizeUser; i++) {
    recognized = 0;

    for(j=0; j<ACOUNT_NUM; j++) {
      if(strlen(sysacounts[j].user)!=0) {
        if(strcmp(sysacounts[j].user,DeleteUsers->UserInfo[i].Username)==0) {
          if(j==0) {
            gLocalErr = VZ_LOCALAPI_USERNAME_FIXED;
            return VZ_LOCALAPI_FAILED;
          }

          recognized = 1;
          break;
        }
      }
    }

    if(!recognized) {
      gLocalErr = VZ_LOCALAPI_USERNAME_MISS;
      return VZ_LOCALAPI_FAILED;
    }
  }

  for(i=0; i<DeleteUsers->sizeUser; i++) {
    if(ControlSystemData(SYS_MSG_DEL_USER,DeleteUsers->UserInfo[i].Username,
                         strlen(DeleteUsers->UserInfo[i].Username))<0) {

      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief set Users
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK__SetUser( LocalSDK_UsersInfo *SetUsers) {
  Acount_t sysacounts[ACOUNT_NUM];

  if(ControlSystemData(MSG_GET_USER, sysacounts,
                       sizeof(Acount_t)*ACOUNT_NUM) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  add_user_t acount;
  int i,j;
  int recognized;

  for(i=0; i<SetUsers->sizeUser; i++) {
    recognized = 0;

    for(j=0; j<ACOUNT_NUM; j++) {
      if(strlen(sysacounts[j].user)!=0) {
        if(strcmp(sysacounts[j].user,SetUsers->UserInfo[i].Username)==0) {
          recognized = 1;
          break;
        }
      }
    }

    if(!recognized) {
      gLocalErr = VZ_LOCALAPI_USERNAME_MISS;
      return VZ_LOCALAPI_FAILED;
    }

    if(SetUsers->UserInfo[i].UserLevel>2) {
      gLocalErr = VZ_LOCALAPI_USER_ANONYMOUS;
      return VZ_LOCALAPI_FAILED;
    }

    int user_len = strlen(SetUsers->UserInfo[i].Username);

    if(user_len>=USR_LEN ) {
      gLocalErr = VZ_LOCALAPI_USERNAME_LONG;
      return VZ_LOCALAPI_FAILED;

    } else if(user_len == 0 ) {
      gLocalErr = VZ_LOCALAPI_USERNAME_SHORT;
      return VZ_LOCALAPI_FAILED;
    }

    if(SetUsers->UserInfo[i].Password) {
      int pass_len = strlen(SetUsers->UserInfo[i].Password);

      if(pass_len>=PW_LEN ) {
        gLocalErr = VZ_LOCALAPI_PASSWORD_LONG;
        return VZ_LOCALAPI_FAILED;

      } else if(pass_len < VZ_PASSWORD_MIN_NUM) {
        gLocalErr = VZ_LOCALAPI_PASSWORD_SHORT;
        return VZ_LOCALAPI_FAILED;
      }

    } else {
      gLocalErr = VZ_LOCALAPI_PASSWORD_SHORT;
      return VZ_LOCALAPI_FAILED;
    }
  }

  for(i=0; i<SetUsers->sizeUser; i++) {
    strcpy(acount.user_id,SetUsers->UserInfo[i].Username);

    if(SetUsers->UserInfo[i].Password) {
      strcpy(acount.password,SetUsers->UserInfo[i].Password);

    } else {
      strcpy(acount.password,"");
    }

    acount.authority = SetUsers->UserInfo[i].UserLevel;

    if(ControlSystemData(SYS_MSG_ADD_USER,&acount,sizeof(acount))<0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief get DNS
*  @return 0 或 -1
*/
int get_dns(char** dns1, char** dns2) {
  FILE *fp;
  static char dnsname[2][40];

  fp = fopen(RESOLV_CONF, "r");

  if ( !fp ) return -1;

  if (fscanf(fp, "nameserver %s\n", dnsname[0]) != EOF) {
    *dns1 = dnsname[0];

    if (fscanf(fp, "nameserver %s\n", dnsname[1]) != EOF) {
      *dns2 = dnsname[1];

    } else {
      *dns2 = NULL;
    }

  } else {
    *dns1 = NULL;
    *dns2 = NULL;
    fclose(fp);
    return -1;
  }

  fclose(fp);
  return 0;
}

/**
*	@brief SET DNS
*  @return 0 或 -1
*/
int set_dns(char* dns1, char* dns2) {
  FILE *fp;

  fp = fopen(RESOLV_CONF, "w");

  if ( fp ) {
    if(dns1) fprintf(fp, "nameserver %s\n", dns1);

    if(dns2) fprintf(fp, "nameserver %s\n", dns2);

    fclose(fp);
    return 0;
  }

  return -1;
}


static int LocalSDK__CheckOut_Ip(char *IP) {
  //需要检测合法性
  struct in_addr addr;
  char *temp =NULL;
  char *string =NULL;
  int count = 0;
  temp = strchr(IP,'.');

  while(temp) {
    count++;
    string = temp+1;
    temp = strchr(string,'.');
  }

  if( (count != 3) || (inet_aton(IP,&addr)==0)) {
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief get DNS
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK__GetDNS(LocalSDK_DNSInfo *DNSInfo) {
  __u8 status_3g;
  __u8 status_dhcp;

  if((ControlSystemData(SYS_MSG_GET_3G_STATUS,&status_3g,sizeof(status_3g))<0)
     ||(ControlSystemData(SYS_MSG_GET_DHCP_STATUS,&status_dhcp,
                          sizeof(status_dhcp))<0)) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  //if 3g or dhcp is enable dns will be set from dhcp

  char* dns[2];
  int dns_size=0;

  if(get_dns(&dns[0], &dns[1])<0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(dns[0])dns_size++;

  if(dns[1])dns_size++;

  if(dns_size > VZ_DNS_MAX_NUM) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  LocalSDK_IPAddress _dns[VZ_DNS_MAX_NUM];
  int i;

  for(i=0; i<dns_size; i++) {
    strcpy(_dns[i].IPv6Address,"");
    _dns[i].Type = Vz__IPType__IPv4;
    strcpy(_dns[i].IPv4Address,dns[i]);
  }

  DNSInfo->Extension.__size = 0;
  DNSInfo->Extension.__any = NULL;
  DNSInfo->__anyAttribute = NULL;
  DNSInfo->SearchDomain = NULL;
  DNSInfo->sizeSearchDomain = 0;

  if(status_3g || status_dhcp) {
    DNSInfo->FromDHCP = vz_true;
    DNSInfo->sizeDNSFromDHCP = dns_size;

    for(i=0; i<dns_size; i++) {
      strcpy(DNSInfo->DNSFromDHCP[i].IPv6Address,_dns[i].IPv6Address);
      strcpy(DNSInfo->DNSFromDHCP[i].IPv4Address,_dns[i].IPv4Address);
      DNSInfo->DNSFromDHCP[i].Type = _dns[i].Type;
    }

    DNSInfo->sizeDNSManual = 0;

  } else {

    DNSInfo->FromDHCP = vz_false;
    DNSInfo->sizeDNSFromDHCP = 0;
    DNSInfo->sizeDNSManual = dns_size;

    for(i=0; i<dns_size; i++) {
      strcpy(DNSInfo->DNSManual[i].IPv6Address,_dns[i].IPv6Address);
      strcpy(DNSInfo->DNSManual[i].IPv4Address,_dns[i].IPv4Address);
      DNSInfo->DNSManual[i].Type = _dns[i].Type;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

/**
*	@brief Set DNS
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK__SetDNS(LocalSDK_DNSInfo *DNSInfo) {
  __u8 status_3g;
  __u8 status_dhcp;

  if((ControlSystemData(SYS_MSG_GET_3G_STATUS,&status_3g,sizeof(status_3g))<0)
     ||(ControlSystemData(SYS_MSG_GET_DHCP_STATUS,&status_dhcp,
                          sizeof(status_dhcp))<0)) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(DNSInfo->sizeSearchDomain!=0) {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;
  }

  //现在对dns的控制还不太完善
  if(DNSInfo->FromDHCP) {
#ifdef DHCP_SUPPORT

    if(!status_dhcp) { //turn on dhcp
      int value =1;

      if(ControlSystemData(SYS_MSG_SET_DHCPC_ENABLE,&value,sizeof(value))<0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }

#else
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;
#endif

  } else {
    if(DNSInfo->sizeDNSManual >2 || DNSInfo->sizeDNSManual<=0) {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      return VZ_LOCALAPI_FAILED;
    }

    int i=0;
    char* dns[2] = {0};

    for(; i<DNSInfo->sizeDNSManual; i++) {
      if(DNSInfo->DNSManual[i].Type == Vz__IPType__IPv6) {
        gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
        return VZ_LOCALAPI_FAILED;

      } else {
        int result = 0;
        result =LocalSDK__CheckOut_Ip(DNSInfo->DNSManual[i].IPv4Address);

        if( result != VZ_LOCALAPI_SUCCESS) {
          gLocalErr = VZ_LOCALAPI_IPV4_INVALID;
          return VZ_LOCALAPI_FAILED;

        } else {
          dns[i] = DNSInfo->DNSManual[i].IPv4Address;
        }
      }
    }

    //当dhcp没有启用时，设置手动dns，若启用，关闭或者断开时使用（以后实现）
    if(!status_3g && !status_dhcp) {
      if(set_dns(dns[0], dns[1])<0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

int get_PrefixLength(struct in_addr netmask) {
  int len=0;

  while( netmask.s_addr & (1 << len)) {
    len++;
  }

  return len;
}
struct in_addr get_netmask_from_PrefixLength(int len) {
  struct in_addr netmask = {0};

  for(len--; len>=0; len--) {
    netmask.s_addr += (1<< len);
  }

  return netmask;
}
//先支持eth0
int LocalSDK__GetNetworkInterfaces(LocalSDK__NetworkInterfaceList
                                   *NetworkInterfaceList) {
  __u8 status_dhcp;

  if(ControlSystemData(SYS_MSG_GET_DHCP_STATUS,&status_dhcp,
                       sizeof(status_dhcp))<0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  NetworkInterfaceList->sizeNetworkInterfaces = 1;

  if(NetworkInterfaceList->sizeNetworkInterfaces > VZ_NETWORK_INTERFACE_MAX_NUM) {
    gLocalErr = VZ_LOCALAPI_NETWORK_INTERFACE_EXCEED;
  }

  strcpy(NetworkInterfaceList->NetworkInterfaces[0].Name,ETH0_NAME);
  const char* 		nicname = ETH0_NAME;
#ifdef WIFI_LAN
  __u8 netlinkstate = 0;

  if((OnvifDisp_SendRequestSync(SYS_SRV_GET_NETLINK_STATE, NULL, 0,&netlinkstate,
                                sizeof(netlinkstate))>0)
     &&(netlinkstate == 0)) { //netlink UNCONNECT
    nicname = WIFI_NAME;
  }

#endif
	hw_info_t hwi;
	GetHwi(&hwi);
  unsigned char *mac = hwi.mac;
  /*if( net_get_hwaddr(nicname,mac) <0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }*/

  sprintf(NetworkInterfaceList->NetworkInterfaces[0].HwAddress,
          "%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

  NetworkInterfaceList->NetworkInterfaces[0].MTU = 0;//不支持

  NetworkInterfaceList->NetworkInterfaces[0].Enabled = vz_true; //eth0 默认打开
  //NetworkInterfaceList->NetworkInterfaces[0].Extension = NULL;
  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Enabled = vz_true;
  //NetworkInterfaceList->NetworkInterfaces[0].IPv6 = NULL;
  //NetworkInterfaceList->NetworkInterfaces[0].Link = NULL;
  NetworkInterfaceList->NetworkInterfaces[0].token = ETH0_NAME;//token不能为空
  NetworkInterfaceList->NetworkInterfaces[0].anyAttribute = NULL;

  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.any = NULL;
  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.anyAttribute = NULL;
  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.size = 0;
  strcpy(NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_LINKLOCAL].Address,
         "");
  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_LINKLOCAL].PrefixLength
    = 0;
  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.DHCP = status_dhcp;
  //SysInfo *pSysInfo = GetSysInfo();
  struct in_addr ip_manual,netmask_manual;
  NetworkInfo *pnetwork = GetNetworkInfo();
  ip_manual = pnetwork->ip;
  netmask_manual = pnetwork->netmask;

  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.sizeManual = 1;
  strcpy(NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.Manual[0].Address,
         inet_ntoa(ip_manual));
  NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.Manual[0].PrefixLength =
    get_PrefixLength(netmask_manual);

  if(status_dhcp) {
    struct in_addr ip,netmask;

    if ((ControlSystemData(SYS_MSG_GET_IP, &ip, sizeof(struct in_addr)) < 0)
        ||(ControlSystemData(SYS_MSG_GET_NETMASK, &netmask,
                             sizeof(struct in_addr)) < 0)) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    // ip.s_addr = net_get_ifaddr(nicname);
    // netmask.s_addr = net_get_netmask(nicname);
    NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_FROMDHCP].PrefixLength
      = get_PrefixLength(netmask);
    strcpy(NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_FROMDHCP].Address,
           inet_ntoa(ip));

  } else {
    strcpy(NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_FROMDHCP].Address,
           "");
    NetworkInterfaceList->NetworkInterfaces[0].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_FROMDHCP].PrefixLength
      = 0;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetNetworkInterfaces(LocalSDK__NetworkInterface
                                   *NetworkInterface) {
  if(strcmp(NetworkInterface->Name,ETH0_NAME)!=0) {
    gLocalErr = VZ_LOCALAPI_NETWORK_INTERFACE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  //不支持关闭eth0
  if(!NetworkInterface->Enabled) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  //忽略以下配置
  /*if(NetworkInterface->NetworkInterface->IPv6 != NULL)
  	return soap_sender_fault_subcode(soap,"InvalidArgVal","InvalidInterfaceType","The suggested network interface type is not supported");
  if(NetworkInterface->NetworkInterface->Link != NULL)
  	return soap_sender_fault_subcode(soap,"InvalidArgVal","InvalidInterfaceType","The suggested network interface type is not supported");
  if(NetworkInterface->NetworkInterface->MTU != NULL)
  	return soap_sender_fault_subcode(soap,"InvalidArgVal","InvalidInterfaceType","The suggested network interface type is not supported");
  if(NetworkInterface->NetworkInterface->Extension != NULL)
  	return soap_sender_fault_subcode(soap,"InvalidArgVal","InvalidInterfaceType","The suggested network interface type is not supported");*/

  //不支持关闭ipv4
  if(!NetworkInterface->IPv4.Enabled) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

#ifdef DHCP_SUPPORT
  __u8 status_dhcp = 0;
  ControlSystemData(SYS_MSG_GET_DHCP_STATUS,&status_dhcp,sizeof(status_dhcp));
#endif

  //不支持dhcp
  if(NetworkInterface->IPv4.Config.DHCP) {
#ifdef DHCP_SUPPORT

    if(!status_dhcp) {
      int value =1;

      if(ControlSystemData(SYS_MSG_SET_DHCPC_ENABLE,&value,sizeof(value))<0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }

    return VZ_LOCALAPI_SUCCESS;

  } else {
    if(status_dhcp) {
      int value =0;

      if(ControlSystemData(SYS_MSG_SET_DHCPC_ENABLE,&value,sizeof(value))<0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }

#else
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
#endif
  }

  //不支持多个Manual地址
  if(NetworkInterface->IPv4.Config.sizeManual != 1) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  struct in_addr ip,netmask;

  if( inet_aton(NetworkInterface->IPv4.Config.Manual[0].Address, &ip) == 0) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  netmask = get_netmask_from_PrefixLength(
              NetworkInterface->IPv4.Config.Manual[0].PrefixLength);

  if(netmask.s_addr <= 0) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  char replybuffer[128] = {0};

  if((ControlSystemDataIpc(MSG_SET_NETIP, (void *)&ip, sizeof(ip), replybuffer,
                           sizeof(replybuffer)) < 0)
     ||(ControlSystemDataIpc(MSG_SET_NETMASK, (void *)&netmask, sizeof(netmask),
                             replybuffer, sizeof(replybuffer)) < 0)) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int  LocalSDK__GetNetworkProtocols(LocalSDK__NetworkProtocolList
                                   *NetworkProtocolList) {
  int sys_http_port = 0;
  int sys_rtsp_port = 0;

  if((ControlSystemData(MSG_GET_HTTPPORT, &sys_http_port,sizeof(int)) < 0)
     ||(ControlSystemData(MSG_GET_RTSPPORT, &sys_rtsp_port,sizeof(int)) < 0)) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  NetworkProtocolList->sizeNetworkProtocols = 2;
  NetworkProtocolList->NetworkProtocols[0].Enabled = vz_true;
  NetworkProtocolList->NetworkProtocols[0].sizePort = 1;
  NetworkProtocolList->NetworkProtocols[0].Name = NETWORK_PROTOCOL_TYPE_HTTP;
  NetworkProtocolList->NetworkProtocols[0].Extension.__size = 0;
  NetworkProtocolList->NetworkProtocols[0].Extension.__any = NULL;
  NetworkProtocolList->NetworkProtocols[0].anyAttribute = NULL;
  NetworkProtocolList->NetworkProtocols[0].Port = sys_http_port;

  NetworkProtocolList->NetworkProtocols[1].Enabled = vz_true;
  NetworkProtocolList->NetworkProtocols[1].sizePort = 1;
  NetworkProtocolList->NetworkProtocols[1].Name = NETWORK_PROTOCOL_TYPE_RTSP;
  NetworkProtocolList->NetworkProtocols[1].Extension.__size = 0;
  NetworkProtocolList->NetworkProtocols[1].Extension.__any = NULL;
  NetworkProtocolList->NetworkProtocols[1].anyAttribute = NULL;
  NetworkProtocolList->NetworkProtocols[1].Port = sys_rtsp_port;

  return VZ_LOCALAPI_SUCCESS;
}

int  LocalSDK__SetNetworkProtocols(LocalSDK__NetworkProtocolList
                                   *NetworkProtocolList) {
  int http_port = 0;
  int rtsp_port = 0;

  if((ControlSystemData(MSG_GET_HTTPPORT, &http_port,sizeof(int)) < 0)
     ||(ControlSystemData(MSG_GET_RTSPPORT, &rtsp_port,sizeof(int)) < 0)) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int i=0;
  int rtsp_index=-1,http_index=-1,https_index=-1;

  for(i=0; i<NetworkProtocolList->sizeNetworkProtocols; i++) {
    if(NetworkProtocolList->NetworkProtocols[i].Name ==
       NETWORK_PROTOCOL_TYPE_HTTP) {
      http_index = i;

      //不支持关闭http或者rtsp服务&不支持多个端口
      if((!NetworkProtocolList->NetworkProtocols[i].Enabled)
         ||(NetworkProtocolList->NetworkProtocols[i].sizePort!=1)) {
        gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
        return VZ_LOCALAPI_FAILED;
      }

    } else if(NetworkProtocolList->NetworkProtocols[i].Name ==
              Vz__NetworkProtocolType__RTSP) {
      rtsp_index = i;

      //不支持关闭http或者rtsp服务&不支持多个端口
      if((!NetworkProtocolList->NetworkProtocols[i].Enabled)
         ||(NetworkProtocolList->NetworkProtocols[i].sizePort!=1)) {
        gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
        return VZ_LOCALAPI_FAILED;
      }

    } else {
      https_index = i;

      if(NetworkProtocolList->NetworkProtocols[i].Enabled)

      {
        gLocalErr = VZ_LOCALAPI_ENABLE_TLS_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }
  }

  //先设置rtsp端口，再设置http端口（boa会重启）
  if(rtsp_index!=-1) {
    if(NetworkProtocolList->NetworkProtocols[rtsp_index].Port != rtsp_port) {
      rtsp_port = NetworkProtocolList->NetworkProtocols[rtsp_index].Port;

      if(ControlSystemData(SYS_MSG_SET_RTSPPORT, (void *)&rtsp_port,
                           sizeof(rtsp_port))<0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }
  }

  if(http_index!=-1) {
    if(NetworkProtocolList->NetworkProtocols[http_index].Port != http_port) {
      http_port = NetworkProtocolList->NetworkProtocols[http_index].Port;

      if(ControlSystemData(SYS_MSG_SET_HTTPPORT, (void *)&http_port,
                           sizeof(http_port))<0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

void LocalSDK__GetNetworkDefaultGateway( LocalSDK__NetworkGateway
                                         *NetworkGateway) {
  struct in_addr gateway;
  if (ControlSystemData(SYS_MSG_GET_GATEWAY, &gateway, sizeof(struct in_addr)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return ;
  }

  NetworkGateway->sizeIPv4Address = 1;
  strcpy(NetworkGateway->IPv4Address[0],inet_ntoa(gateway));
  strcpy(NetworkGateway->IPv6Address[0],"");
  NetworkGateway->sizeIPv6Address = 0;
}

int LocalSDK__SetNetworkDefaultGateway(LocalSDK__NetworkGateway
                                       *NetworkGateway) {
  if(NetworkGateway->sizeIPv6Address!=0) {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;
  }

  //现只支持设置一个gateway
  if(NetworkGateway->sizeIPv4Address!=1) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if(NetworkGateway->IPv4Address[0]) {

    struct in_addr gateway;

    if( inet_aton(NetworkGateway->IPv4Address[0], &gateway) == 0) {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      return VZ_LOCALAPI_FAILED;
    }

    char replybuffer[128] = {0};

    if(ControlSystemDataIpc(MSG_SET_GATEWAY, (void *)&gateway, sizeof(gateway),
                            replybuffer,sizeof(replybuffer)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

static int LocalSDK__CheckRuleToken(char * Token,int *AlgId) {
  int i=0;
  int j=0;
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  for(j=0; j<MAX_CHANNEL; j++) {
    if(sys_source_cfg[j].enableAlg) {
      for(i=0; i<MAX_PROFILE_NUM; i++) {
        int id = pOnvifInfo->Profiles[j].meta_profile[i].VA_ID;

        if(Token && (id >= 0)
           &&(strcmp(Token,pOnvifInfo->VideoAnalytics[id].VACfg.token)==0)) {
          *AlgId = (int)sys_source_cfg[j].algId;

          if((*AlgId<0)||(*AlgId>=MAX_ALG_CHANNEL))
            return VZ_LOCALAPI_FAILED;

          return VZ_LOCALAPI_SUCCESS;
        }
      }
    }
  }

  return VZ_LOCALAPI_FAILED;
}



int LocalSDK__GetSupportedRules(char * Token,
                                LocalSDK__SupportedRules *SupportedRules) {
  int AlgId =0;

  if(LocalSDK__CheckRuleToken(Token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    int i =0;
    VZ_IVS_RULE_CFG_EX ivs_rule_cfg_ex;

    if(ControlSystemData(MSG_GET_EVENTRULE, &ivs_rule_cfg_ex,
                         sizeof(VZ_IVS_RULE_CFG_EX)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    SupportedRules->sizeSchemaLocation =0;
    SupportedRules->sizeRuleDescription = ivs_rule_cfg_ex.dwRuleNum;

    for(i =0; i<SupportedRules->sizeRuleDescription; i++) {
      VZ_IVS_RULE_INFO *struRules = &(ivs_rule_cfg_ex.struRules[i]);
      strcpy(SupportedRules->RuleDescription[i].Name,struRules->byRuleName);

      //get SimpleItem:byRuleID,byEnable,byDrawbyDSP,byRes
      int k = 0;
      strcpy(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k].Name,
             "RuleID");
      sprintf(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k++].Value,
              "%d",struRules->byRuleID);

      strcpy(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k].Name,
             "Enable");
      sprintf(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k++].Value,
              "%d",struRules->byEnable);

      strcpy(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k].Name,
             "DrawbyDSP");
      sprintf(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k++].Value,
              "%d",struRules->byDrawbyDSP);

      strcpy(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k].Name,"Res");
      sprintf(SupportedRules->RuleDescription[i].Parameters.SimpleItem[k++].Value,
              "%d",struRules->byRes[0]);
      SupportedRules->RuleDescription[i].Parameters.__sizeSimpleItem = k;

      //get ElementItem(VZ_IVS_EVENT_UNION)
      k = 0;
      strcpy(SupportedRules->RuleDescription[i].Parameters.ElementItem[k].Name,
             "EventParam");
      strcpy(SupportedRules->RuleDescription[i].Parameters.ElementItem[k++].Value,
        (char *)&(struRules->uEventParam));
      strcpy(SupportedRules->RuleDescription[i].Parameters.ElementItem[k].Name,
             "RuleTime");
      strcpy(SupportedRules->RuleDescription[i].Parameters.ElementItem[k++].Value,
        (char *)&(ivs_rule_cfg_ex.struRuleTimes[i]));
      SupportedRules->RuleDescription[i].Parameters.__sizeElementItem = k;
    }

    return VZ_LOCALAPI_SUCCESS;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

}
int LocalSDK__CreateRules(LocalSDK__OneSetOfRule *CreatRules) {
  int AlgId =0;

  if(LocalSDK__CheckRuleToken(CreatRules->token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    int i =0;
    int RuleIndex=0;
    VZ_IVS_RULE_CFG_EX ivs_rule_cfg_ex;

    if(ControlSystemData(MSG_GET_EVENTRULE, &ivs_rule_cfg_ex,
                         sizeof(VZ_IVS_RULE_CFG_EX)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    if(CreatRules->sizeRule + ivs_rule_cfg_ex.dwRuleNum > MAX_RULE_NUM) {
      gLocalErr = VZ_LOCALAPI_RULE_EXCEED;
      return VZ_LOCALAPI_FAILED;
    }

    VZ_IVS_RULE_CFG_EX value;
    memcpy(&value,&ivs_rule_cfg_ex,sizeof(value));

    for(i =0; i<CreatRules->sizeRule && value.dwRuleNum <= MAX_RULE_NUM ; i++) {
      int j = 0;
      RuleIndex=value.dwRuleNum;

      for(j = 0; j<value.dwRuleNum; j++) {
        if(strcmp(CreatRules->Rule[i].Name,value.struRules[j].byRuleName)== 0) {
          RuleIndex=j;
          break;
          //gLocalErr = VZ_LOCALAPI_RULE_CLASH;
          //fprintf(stderr,"Rule Name exist!n");
          //return VZ_LOCALAPI_FAILED;
        }
      }

      if(strlen(CreatRules->Rule[i].Name)==0) {
        fprintf(stderr,"Need Rule Name!\n");
        gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
        return VZ_LOCALAPI_FAILED;
      }

      strcpy(value.struRules[RuleIndex].byRuleName,CreatRules->Rule[i].Name);
      fprintf(stderr,"Rule %d:%s\n",RuleIndex,CreatRules->Rule[i].Name);
      int k = 0;

      //for SimpleItem:byRuleID,byEnable,byDrawbyDSP,byRes
      for(k = 0; k<CreatRules->Rule[i].Parameters.__sizeSimpleItem; k++) {
        if(strcmp(CreatRules->Rule[i].Parameters.SimpleItem[k].Name,"RuleID")==0)
          value.struRules[RuleIndex].byRuleID= atoi(
                                                 CreatRules->Rule[i].Parameters.SimpleItem[k].Value);

        if(strcmp(CreatRules->Rule[i].Parameters.SimpleItem[k].Name,"Enable")==0)
          value.struRules[RuleIndex].byEnable= atoi(
                                                 CreatRules->Rule[i].Parameters.SimpleItem[k].Value);

        if(strcmp(CreatRules->Rule[i].Parameters.SimpleItem[k].Name,"DrawbyDSP")==0)
          value.struRules[RuleIndex].byDrawbyDSP= atoi(
                                                    CreatRules->Rule[i].Parameters.SimpleItem[k].Value);

        if(strcmp(CreatRules->Rule[i].Parameters.SimpleItem[k].Name,"Res")==0)
          value.struRules[RuleIndex].byRes[0] = atoi(
                                                  CreatRules->Rule[i].Parameters.SimpleItem[k].Value);

        if(strcmp(CreatRules->Rule[i].Parameters.SimpleItem[k].Name,"RuleType")==0)
          value.struRules[RuleIndex].dwRuleType = (IVS_RULE_TYPE)atoi(
                                                    CreatRules->Rule[i].Parameters.SimpleItem[k].Value);
      }

      memset(&(value.struRules[RuleIndex].uEventParam),0x0,
             sizeof(VZ_IVS_EVENT_UNION));
      memset(&(value.struRuleTimes[RuleIndex]),0x0,sizeof(VZ_IVS_RULE_TIME_INFO));

      for(k = 0; k<CreatRules->Rule[i].Parameters.__sizeElementItem; k++) {
        int rlen = 0;
        //unsigned char* decode = NULL;
        unsigned int buf_max_len = 2*1024;
        unsigned char decode[2*1024] = {0};

        //for ElementItem(VZ_IVS_EVENT_UNION)
        if(strcmp(CreatRules->Rule[i].Parameters.ElementItem[k].Name,"EventParam")==0) {
          memset(decode, 0, buf_max_len);
          rlen = base64_decode_onvif(CreatRules->Rule[i].Parameters.ElementItem[k].Value,
                                     decode, buf_max_len);
          //decode = base64_decode(CreatRules->Rule[i].Parameters.ElementItem[k].Value, &rlen);
          fprintf(stderr,"value:%s\n",
                  CreatRules->Rule[i].Parameters.ElementItem[k].Value);

          if(rlen != sizeof(VZ_IVS_EVENT_UNION)) {
            fprintf(stderr,"can't get VZ_IVS_EVENT_UNION[%d:%d]\n",rlen,
                    sizeof(VZ_IVS_EVENT_UNION));
            gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
            return VZ_LOCALAPI_FAILED;
          }

          memcpy(&(value.struRules[RuleIndex].uEventParam),decode,
                 sizeof(VZ_IVS_EVENT_UNION));

          if(value.struRules[RuleIndex].dwRuleType == IVS_RULE_AREA_EX)
            fprintf(stderr,"dwPointNum:%d\n",
                    value.struRules[RuleIndex].uEventParam.struAreaExt.struRegion.dwPointNum);
        }

        //for VZ_IVS_RULE_TIME_INFO
        if(strcmp(CreatRules->Rule[i].Parameters.ElementItem[k].Name,"RuleTime")==0) {
          memset(decode, 0, buf_max_len);
          rlen = base64_decode_onvif(CreatRules->Rule[i].Parameters.ElementItem[k].Value,
                                     decode, buf_max_len);

          //decode = base64_decode(CreatRules->Rule[i].Parameters.ElementItem[k].Value, &rlen);
          if(rlen != sizeof(VZ_IVS_RULE_TIME_INFO)) {
            fprintf(stderr,"decode RuleTime fail\n");
            gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
            return VZ_LOCALAPI_FAILED;

          } else {
            memcpy(&(value.struRuleTimes[RuleIndex]),decode,sizeof(VZ_IVS_RULE_TIME_INFO));
          }
        }
      }

      if(RuleIndex >= value.dwRuleNum) {
        value.dwRuleNum++;
      }
    }

    if(ControlSystemDataALGChannel(AlgId,SYS_MSG_SET_IVS_EVENTRULE1, &value,
                                   sizeof(value)) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    return VZ_LOCALAPI_SUCCESS;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

}

int LocalSDK__DeleteRules(LocalSDK__OneSetOfRule *DeleteRules) {
  int AlgId =0;

  if(LocalSDK__CheckRuleToken(DeleteRules->token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    int i=0,j=0;
    int add_len = 0;
    int FoundFlag;
    VZ_IVS_RULE_CFG_EX Rules;

    if(ControlSystemData(MSG_GET_EVENTRULE, &Rules,
                         sizeof(VZ_IVS_RULE_CFG_EX)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    for(i=0; i<DeleteRules->sizeRule; i++) {
      FoundFlag = 0;

      for(j=0; j<Rules.dwRuleNum; j++) {
        if(strcmp(Rules.struRules[j].byRuleName,DeleteRules->Rule[i].Name)==0) {
          strcpy(Rules.struRules[j].byRuleName,"");
          FoundFlag = 1;
        }
      }

      if(!FoundFlag) {
        gLocalErr = VZ_LOCALAPI_RULE_NONEXIST;
        return VZ_LOCALAPI_FAILED;
      }
    }

    int last_empty_str_index=Rules.dwRuleNum;

    //排序
    for(j=0; j<Rules.dwRuleNum; j++) {
      if(strlen(Rules.struRules[j].byRuleName)!=0) {
        if(last_empty_str_index <j) {
          //strcpy(Rules.struRules[last_empty_str_index].byRuleName,Rules.struRules[j].byRuleName);
          //strcpy(Rules.struRules[j].byRuleName,"");
          memcpy(&(Rules.struRules[last_empty_str_index]),&(Rules.struRules[j]),
                 sizeof(VZ_IVS_RULE_INFO));
          memset(&(Rules.struRules[j]),0x0,sizeof(VZ_IVS_RULE_INFO));
          memcpy(&(Rules.struRuleTimes[last_empty_str_index]),&(Rules.struRuleTimes[j]),
                 sizeof(VZ_IVS_RULE_INFO));
          memset(&(Rules.struRuleTimes[j]),0x0,sizeof(VZ_IVS_RULE_TIME_INFO));
          last_empty_str_index++;
        }

        add_len++;

      } else {
        if(last_empty_str_index >j)
          last_empty_str_index = j;

        memset(&(Rules.struRules[j]),0x0,sizeof(VZ_IVS_RULE_INFO));
        memset(&(Rules.struRuleTimes[j]),0x0,sizeof(VZ_IVS_RULE_TIME_INFO));
      }
    }

    //长度修正
    Rules.dwRuleNum = add_len;
    fprintf(stderr,"dwRuleNum:%d\n",add_len);

    if(ControlSystemDataALGChannel(AlgId,SYS_MSG_SET_IVS_EVENTRULE1, &Rules,
                                   sizeof(Rules)) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    return VZ_LOCALAPI_SUCCESS;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }
}


int LocalSDK__GetRules(LocalSDK__OneSetOfRule *OneSetOfRules) {
  int AlgId =0;

  if(LocalSDK__CheckRuleToken(OneSetOfRules->token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    int i =0;
    VZ_IVS_RULE_CFG_EX ivs_rule_cfg_ex;

    if(ControlSystemData(MSG_GET_EVENTRULE, &ivs_rule_cfg_ex,
                         sizeof(VZ_IVS_RULE_CFG_EX)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    OneSetOfRules->sizeRule=ivs_rule_cfg_ex.dwRuleNum;

    for(i =0; i<OneSetOfRules->sizeRule; i++) {
      VZ_IVS_RULE_INFO *struRules = &(ivs_rule_cfg_ex.struRules[i]);
      strcpy(OneSetOfRules->Rule[i].Name,struRules->byRuleName);
      fprintf(stderr,"Rule %d:%s\n",i,struRules->byRuleName);
      //get SimpleItem:byRuleID,byEnable,byDrawbyDSP,byRes
      int k = 0;
      strcpy(OneSetOfRules->Rule[i].Parameters.SimpleItem[k].Name,"RuleID");
      sprintf(OneSetOfRules->Rule[i].Parameters.SimpleItem[k++].Value,"%d",
              struRules->byRuleID);

      strcpy(OneSetOfRules->Rule[i].Parameters.SimpleItem[k].Name,"Enable");
      sprintf(OneSetOfRules->Rule[i].Parameters.SimpleItem[k++].Value,"%d",
              struRules->byEnable);

      strcpy(OneSetOfRules->Rule[i].Parameters.SimpleItem[k].Name,"DrawbyDSP");
      sprintf(OneSetOfRules->Rule[i].Parameters.SimpleItem[k++].Value,"%d",
              struRules->byDrawbyDSP);

      strcpy(OneSetOfRules->Rule[i].Parameters.SimpleItem[k].Name,"Res");
      sprintf(OneSetOfRules->Rule[i].Parameters.SimpleItem[k++].Value,"%d",
              struRules->byRes[0]);

      strcpy(OneSetOfRules->Rule[i].Parameters.SimpleItem[k].Name,"RuleType");
      sprintf(OneSetOfRules->Rule[i].Parameters.SimpleItem[k++].Value,"%d",
              struRules->dwRuleType);

      OneSetOfRules->Rule[i].Parameters.__sizeSimpleItem = k;

      //get ElementItem(VZ_IVS_EVENT_UNION)
      k = 0;
      strcpy(OneSetOfRules->Rule[i].Parameters.ElementItem[k].Name,"EventParam");

      base64_encode_onvif(( unsigned char *)&struRules->uEventParam,
        sizeof(VZ_IVS_EVENT_UNION), OneSetOfRules->Rule[i].Parameters.ElementItem[k].Value, VZ_EITEM_ANY_MAX_SIZE);
      k++;
      strcpy(OneSetOfRules->Rule[i].Parameters.ElementItem[k].Name,"RuleTime");
      base64_encode_onvif(( unsigned char *)&(ivs_rule_cfg_ex.struRuleTimes[i]),
        sizeof(VZ_IVS_RULE_TIME_INFO),OneSetOfRules->Rule[i].Parameters.ElementItem[k].Value,VZ_EITEM_ANY_MAX_SIZE);
      k++;

      OneSetOfRules->Rule[i].Parameters.__sizeElementItem = k;
    }

    return VZ_LOCALAPI_SUCCESS;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }
}


int LocalSDK__ModifyRules(LocalSDK__OneSetOfRule *ModifyRules) {
  int AlgId=0;

  if(LocalSDK__CheckRuleToken(ModifyRules->token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    VZ_IVS_RULE_CFG_EX Rules;

    if(ControlSystemData(MSG_GET_EVENTRULE, &Rules,
                         sizeof(VZ_IVS_RULE_CFG_EX)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    int i,j;
    int recognized;

    for(i=0; i<ModifyRules->sizeRule; i++) {
      recognized = 0;

      for(j=0; j<Rules.dwRuleNum; j++) {
        if(strlen(Rules.struRules[i].byRuleName)!=0) {
          if(strcmp(Rules.struRules[i].byRuleName,ModifyRules->Rule[i].Name)==0) {
            recognized = 1;

            int k;

            //for SimpleItem:byRuleID,byEnable,byDrawbyDSP,byRes
            for(k = 0; k<ModifyRules->Rule[i].Parameters.__sizeSimpleItem; k++) {
              if(strcmp(ModifyRules->Rule[i].Parameters.SimpleItem[k].Name,"RuleID")==0)
                Rules.struRules[i].byRuleID= atoi(
                                               ModifyRules->Rule[i].Parameters.SimpleItem[k].Value);

              if(strcmp(ModifyRules->Rule[i].Parameters.SimpleItem[k].Name,"Enable")==0)
                Rules.struRules[i].byEnable= atoi(
                                               ModifyRules->Rule[i].Parameters.SimpleItem[k].Value);

              if(strcmp(ModifyRules->Rule[i].Parameters.SimpleItem[k].Name,"DrawbyDSP")==0)
                Rules.struRules[i].byDrawbyDSP= atoi(
                                                  ModifyRules->Rule[i].Parameters.SimpleItem[k].Value);

              if(strcmp(ModifyRules->Rule[i].Parameters.SimpleItem[k].Name,"Res")==0)
                Rules.struRules[i].byRes[0] = atoi(
                                                ModifyRules->Rule[i].Parameters.SimpleItem[k].Value);

              if(strcmp(ModifyRules->Rule[i].Parameters.SimpleItem[k].Name,"RuleType")==0)
                Rules.struRules[i].dwRuleType = (IVS_RULE_TYPE)atoi(
                                                  ModifyRules->Rule[i].Parameters.SimpleItem[k].Value);
            }

            memset(&(Rules.struRules[i].uEventParam),0x0,sizeof(VZ_IVS_EVENT_UNION));
            memset(&(Rules.struRuleTimes[i]),0x0,sizeof(VZ_IVS_RULE_TIME_INFO));

            for(k = 0; k<ModifyRules->Rule[i].Parameters.__sizeElementItem; k++) {
              int rlen = 0;
              //unsigned char* decode = NULL;
              unsigned int buf_max_len = 2*1024;
              unsigned char decode[2*1024] = {0};

              //for ElementItem(VZ_IVS_EVENT_UNION)
              if(strcmp(ModifyRules->Rule[i].Parameters.ElementItem[k].Name,
                        "EventParam")==0) {
                memset(decode, 0, buf_max_len);
                rlen = base64_decode_onvif(ModifyRules->Rule[i].Parameters.ElementItem[k].Value,
                                           decode, buf_max_len);

                //decode = base64_decode(ModifyRules->Rule[i].Parameters.ElementItem[k].Value, &rlen);
                if(rlen != sizeof(VZ_IVS_EVENT_UNION)) {
                  fprintf(stderr,"can't get VZ_IVS_EVENT_UNION\n");
                  gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
                  return VZ_LOCALAPI_FAILED;
                }

                memcpy(&(Rules.struRules[i].uEventParam),decode,sizeof(VZ_IVS_EVENT_UNION));
              }

              //for VZ_IVS_RULE_TIME_INFO
              if(strcmp(ModifyRules->Rule[i].Parameters.ElementItem[k].Name,"RuleTime")==0) {
                memset(decode, 0, buf_max_len);
                rlen = base64_decode_onvif(ModifyRules->Rule[i].Parameters.ElementItem[k].Value,
                                           decode, buf_max_len);

                //decode = base64_decode(ModifyRules->Rule[i].Parameters.ElementItem[k].Value, &rlen);
                if(rlen != sizeof(VZ_IVS_RULE_TIME_INFO)) {
                  fprintf(stderr,"decode RuleTime fail\n");
                  gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
                  return VZ_LOCALAPI_FAILED;

                } else {
                  memcpy(&(Rules.struRuleTimes[i]),decode,sizeof(VZ_IVS_RULE_TIME_INFO));
                }
              }
            }

            break;
          }
        }
      }

      if(!recognized) {
        gLocalErr = VZ_LOCALAPI_RULE_NONEXIST;
        return VZ_LOCALAPI_FAILED;
      }

    }

    if(ControlSystemDataALGChannel(AlgId,SYS_MSG_SET_IVS_EVENTRULE1, &Rules,
                                   sizeof(Rules)) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    return VZ_LOCALAPI_SUCCESS;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }
}

static int gen_new_uri(char* old_uri,int *port,int is_rtsp) {
  struct in_addr ip;
  const char*			nicname = ETH0_NAME;
#ifdef WIFI_LAN
  __u8 netlinkstate = 0;

  if((OnvifDisp_SendRequestSync(SYS_SRV_GET_NETLINK_STATE, NULL, 0,&netlinkstate,
                                sizeof(netlinkstate))>0)
     &&(netlinkstate == 0)) { //netlink UNCONNECT
    nicname = WIFI_NAME;
  }

#endif

  if (ControlSystemData(SYS_MSG_GET_IP, &ip, sizeof(struct in_addr)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  //ip.s_addr = net_get_ifaddr(nicname);
  char ip_new[20];
  char str_port[10] = {0};
  char port_new[10] = {0};
  strcpy(ip_new,inet_ntoa(ip));
  char new_uri[VZ_STR_SHORT_LEN]= {0};

  char* p = strstr(old_uri,"//");

  if(!p) {
    return VZ_LOCALAPI_FAILED;
  }

  p += 2;
  char* q = strchr(p,'/');

  if(!q) q = p + strlen(p);

  char* r = strchr(p,'@');

  if(r && r < q)p = r+1;

  char* s = strchr(p,':');

  if(s && s < q) {
    strncpy(str_port,s+1,q-(s+1));
    *port = atoi(str_port);
    int sys_rtsp_port;

    if(ControlSystemData(MSG_GET_RTSPPORT, &sys_rtsp_port,sizeof(int)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    sprintf(port_new,":%d",sys_rtsp_port);

  } else {
    if(is_rtsp) {
      *port = 554;

    } else {
      *port = 80;
    }
  }

  int len1 = p-old_uri;
  int len2 = strlen(ip_new);
  int lenport = strlen(port_new);
  int len3 = strlen(q);

  if((len1+len2+len3+lenport+1)>= VZ_STR_SHORT_LEN) {
    fprintf( stderr,"exceeding the max length\n" );
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(new_uri,old_uri,len1);
  memcpy(new_uri+len1,ip_new,len2);

  if(lenport) {
    memcpy(new_uri+len1+len2,port_new,lenport);
  }

  memcpy(new_uri+len1+len2+lenport,q,len3);
  new_uri[len1+len2+len3+lenport] = '\0';
  strcpy(old_uri,new_uri);
  return VZ_LOCALAPI_SUCCESS;
}

int  LocalSDK_gen_new_uri_without_port(char* old_uri) {
  if(old_uri==NULL)
    return VZ_LOCALAPI_FAILED;

  struct in_addr ip;
  const char*     nicname = ETH0_NAME;
#ifdef WIFI_LAN
  __u8 netlinkstate = 0;

  if((OnvifDisp_SendRequestSync(SYS_SRV_GET_NETLINK_STATE, NULL, 0,&netlinkstate,
                                sizeof(netlinkstate))>0)
     &&(netlinkstate == 0)) { //netlink UNCONNECT
    nicname = WIFI_NAME;
  }

#endif

  if (ControlSystemData(SYS_MSG_GET_IP, &ip, sizeof(struct in_addr)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  //ip.s_addr = net_get_ifaddr(nicname);
  char ip_new[20];
  strcpy(ip_new,inet_ntoa(ip));
  char new_uri[VZ_STR_SHORT_LEN]= {0};

  char* p = strstr(old_uri,"//");

  if(!p) {
    return VZ_LOCALAPI_FAILED;
  }

  p += 2;
  char* q = strchr(p,'/');

  if(!q) q = p + strlen(p);

  char* r = strchr(p,'@');

  if(r && r < q)p = r+1;

  int len1 = p-old_uri;
  int len2 = strlen(ip_new);
  int len3 = strlen(q);

  if((len1+len2+len3+1)>= VZ_STR_SHORT_LEN) {
    fprintf( stderr,"exceeding the max length\n" );
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(new_uri,old_uri,len1);
  memcpy(new_uri+len1,ip_new,len2);
  memcpy(new_uri+len1+len2,q,len3);
  new_uri[len1+len2+len3] = '\0';
  strcpy(old_uri,new_uri);
  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetMediaUriResponse(LocalSDK__MediaUri *MediaUri) {
  int SdkResult = VZ_LOCALAPI_SUCCESS;

  if(MediaUri && MediaUri->Uri) {
    int port;
    SdkResult = gen_new_uri(MediaUri->Uri,&port,1);

    if(SdkResult != VZ_LOCALAPI_SUCCESS) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    if(MediaUri->PortMapFlag)
      DoOnvifSetRemoteRtspPort(port,0);

  } else {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetMetadataConfiguration(char *ConfigToken,
                                       LocalSDK__MetadataConfig *GetMetadataConfig) {
  int i = 0;

  int recognized =0;

  for(i=0; i<ACOUNT_NUM; i++) {
    if(strcmp("exist Configuration tokens",ConfigToken)==0) {
      recognized = 1;
      break;
    }
  }

  if(recognized) {
    strcpy(GetMetadataConfig->Name,"");
    strcpy(GetMetadataConfig->token,"");
    strcpy(GetMetadataConfig->SessionTimeout,"");
    strcpy(GetMetadataConfig->Multicast.Address.IPv4Address,"");
    strcpy(GetMetadataConfig->Multicast.Address.IPv6Address,"");
    GetMetadataConfig->Event.Filtersize = 0;
    GetMetadataConfig->Event.SubscriptionPolicysize = 0;

    if(GetMetadataConfig->Event.Filtersize) {
      for(i =0; i<GetMetadataConfig->Event.Filtersize; i++) {
        strcpy(GetMetadataConfig->Event.Filterany[i],"");
      }
    }

    if(GetMetadataConfig->Event.SubscriptionPolicysize) {
      for(i =0; i<GetMetadataConfig->Event.SubscriptionPolicysize; i++) {
        strcpy(GetMetadataConfig->Event.SubscriptionPolicyany[i],"");
      }
    }

    GetMetadataConfig->Multicast.Address.Type =0;
    GetMetadataConfig->UseCount = 0;
    GetMetadataConfig->Multicast.Port = 0;
    GetMetadataConfig->Multicast.TTL = 0;
    GetMetadataConfig->Multicast.AutoStart = 0;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}
#ifdef DHCP_SUPPORT
//get the ntp server from dhcp,now only can get the local clock127.127.1.0
int get_ntp_fromdhcp(char* ntp ) {
  FILE *fp;
  char buffer[64]= {0};

  fp = fopen(NTP_CONF, "r");

  if ( !fp ) return -1;

  while(fgets(buffer, 64, fp)) {
    if (strncmp(buffer, "fudge",5) == 0) {
      fprintf(stderr,"No servers found, use local clock \n");

    } else if(strncmp(buffer, "server",6) == 0) {
      char *temp=strchr(buffer,' ');
      int length = strchr(buffer,'\n')-(temp+1);
      strncpy(ntp,temp+1,length);
      break;
    }
  }

  fclose(fp);
  return 0;
}
#endif
int LocalSDK__GetNTP(LocalSDK_NTPInfo *GetNTP) {
  int i =0;
  int SdkResult = VZ_LOCALAPI_SUCCESS;
  Ntp_Config_Data				sys_ntp;

  if(ControlSystemData(MSG_GET_SNTP, &sys_ntp,sizeof(Ntp_Config_Data)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

#ifdef DHCP_SUPPORT
  __u8 status_dhcp;

  if(ControlSystemData(SYS_MSG_GET_DHCP_STATUS,&status_dhcp,
                       sizeof(status_dhcp))<0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  GetNTP->FromDHCP =status_dhcp;

  if(status_dhcp) { //dhcp was open,get the ntp server from the dhcp
    char ntp[42]= {0};
    GetNTP->FromDHCP = vz_true;
    SdkResult = get_ntp_fromdhcp(ntp);

    if(SdkResult != 0) {
      gLocalErr = VZ_LOCALAPI_NTP_ERR;
      return VZ_LOCALAPI_FAILED;
    }

    GetNTP->sizeNTPFromDHCP =1;
    GetNTP->sizeNTPManual =0;

    for(i =0; i<GetNTP->sizeNTPFromDHCP; i++) {
      SdkResult = LocalSDK__CheckOut_Ip(ntp);

      if(SdkResult != VZ_LOCALAPI_SUCCESS) {
        GetNTP->NTPFromDHCP[i].Type=
          Vz__NetworkHostType__DNS;//tt__NetworkHostType__IPv4,
        strcpy(GetNTP->NTPFromDHCP[i].IPv4Address,"");
        strcpy(GetNTP->NTPFromDHCP[i].IPv6Address,"");
        strcpy(GetNTP->NTPFromDHCP[i].DNSname,ntp);

      } else {
        GetNTP->NTPFromDHCP[i].Type=
          Vz__NetworkHostType__IPv4;//tt__NetworkHostType__IPv4,
        strcpy(GetNTP->NTPFromDHCP[i].IPv4Address,ntp);
        strcpy(GetNTP->NTPFromDHCP[i].IPv6Address,"");
        strcpy(GetNTP->NTPFromDHCP[i].DNSname,"");
      }
    }

  } else {
    GetNTP->FromDHCP = vz_false;
    GetNTP->sizeNTPFromDHCP = 0;
    GetNTP->sizeNTPManual =1;
  }

#else
  GetNTP->FromDHCP = vz_false;
  GetNTP->sizeNTPFromDHCP = 0;
  GetNTP->sizeNTPManual =1;
#endif

  for(i =0; i<GetNTP->sizeNTPManual; i++) {
    //net struct need one variable to Type ?
    SdkResult = LocalSDK__CheckOut_Ip(sys_ntp.server_name);

    if(SdkResult != VZ_LOCALAPI_SUCCESS) {
      GetNTP->NTPManual[i].Type= Vz__NetworkHostType__DNS;//tt__NetworkHostType__IPv4,
      strcpy(GetNTP->NTPManual[i].IPv4Address,"");
      strcpy(GetNTP->NTPManual[i].IPv6Address,"");
      strcpy(GetNTP->NTPManual[i].DNSname,sys_ntp.server_name);

    } else {
      GetNTP->NTPManual[i].Type=
        Vz__NetworkHostType__IPv4;//tt__NetworkHostType__IPv4,
      strcpy(GetNTP->NTPManual[i].IPv4Address,sys_ntp.server_name);
      strcpy(GetNTP->NTPManual[i].IPv6Address,"");
      strcpy(GetNTP->NTPManual[i].DNSname,"");
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__SetNTP(LocalSDK_SetNTP *SetNTP) {
  int i=0;
  int result = 0;
  char* ServerAddr[VZ_LIST_MAX_NUM] = {0};

  //由dhcp指定ntp的功能
  if(SetNTP->FromDHCP) {
#ifdef DHCP_SUPPORT
    __u8 status_dhcp;

    if(ControlSystemData(SYS_MSG_GET_DHCP_STATUS,&status_dhcp,
                         sizeof(status_dhcp))<0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    if(!status_dhcp) { //if dhcp was off ,turn on it
      int value =1;

      if(ControlSystemData(SYS_MSG_SET_DHCPC_ENABLE,&value,sizeof(value))<0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }

#else
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;
#endif

  } else {

    for(i = 0; i<SetNTP->sizeNTPManual; i++) {
      if(SetNTP->NTPManual[i].Type == Vz__NetworkHostType__IPv6) {
        gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
        return VZ_LOCALAPI_FAILED;

      } else {
        if(SetNTP->NTPManual[i].Type == Vz__NetworkHostType__DNS) {
          /*result =LocalSDK__CheckOut_Ip(SetNTP->NTPManual[i].DNSname);
          if(result != VZ_LOCALAPI_SUCCESS)
          {
          	gLocalErr = VZ_LOCALAPI_DNS_INVALID;
          	return VZ_LOCALAPI_FAILED;
          }*/
          ServerAddr[i] = SetNTP->NTPManual[i].DNSname;

        } else {
          result =LocalSDK__CheckOut_Ip(SetNTP->NTPManual[i].IPv4Address);

          if(result != VZ_LOCALAPI_SUCCESS) {
            gLocalErr = VZ_LOCALAPI_IPV4_INVALID;
            return VZ_LOCALAPI_FAILED;
          }

          ServerAddr[i] = SetNTP->NTPManual[i].IPv4Address;
        }
      }
    }

    Ntp_Config_Data ntp;
    memset(&ntp,0x0,sizeof(Ntp_Config_Data));

    for(i=0; i<SetNTP->sizeNTPManual; i++) {
      strcpy(ntp.server_name,ServerAddr[i]);
      ntp.frequency = NTP_FREQUENCY_DEFAULT;
      ntp.enable =1;

      if (ControlSystemData(SYS_MSG_SET_SNTP, (void *)&ntp, sizeof(ntp)) < 0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetStreamUri(LocalSDK__MediaUri *MediaUri) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int result=VZ_LOCALAPI_SUCCESS;
  int relay=0;
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(MediaUri->profiletoken,pOnvifInfo,
                                        &ChannelId,&index);

  if(ret == VZ_LOCALAPI_SUCCESS) {
    //struct in_addr ip;
    //ip.s_addr = net_get_ifaddr(ETH0_NAME);
    VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

    if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                         sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
#ifdef DM3730
      strcpy(MediaUri->Uri,
             pOnvifInfo->Profiles[ChannelId].meta_profile[index].out_uri);
      result=LocalSDK__GetMediaUriResponse(MediaUri);
#elif defined(DM6446)
      strcpy(MediaUri->profiletoken,"");
      strcpy(MediaUri->profiletoken,
             pOnvifInfo->Profiles[ChannelId].meta_profile[index].oldtoken);
      MediaUri->channel= ChannelId;
      relay=1;
#endif

    } else {
      if(OnvifDisp_SendRequestSync(WS_MSG_GET_STREAM_URL, NULL, 0,MediaUri->Uri,
                                   sizeof(MediaUri->Uri))>0) {
        result = VZ_LOCALAPI_SUCCESS;

      } else {
        strcpy(MediaUri->Uri,
               pOnvifInfo->Profiles[ChannelId].meta_profile[index].out_uri);
        LocalSDK__GetMediaUriResponse(MediaUri);
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        result = VZ_LOCALAPI_FAILED;
      }
    }

    if(relay)
      return VZ_LOCALAPI_FAILED;

  } else {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    strcpy(MediaUri->profiletoken,"");
    return VZ_LOCALAPI_FAILED;
  }

  MediaUri->InvalidAfterConnect = vz_false;
  MediaUri->InvalidAfterReboot = vz_false;
  strcpy(MediaUri->profiletoken,"");
  return result;
}

//远程的先转发，在后处理中返回我们的地址
int LocalSDK__GetStreamUriRemote(LocalSDK__MediaUri *MediaUri) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int port;
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(MediaUri->profiletoken,pOnvifInfo,
                                        &ChannelId,&index);

  if(ret == VZ_LOCALAPI_SUCCESS) {
    strcpy(MediaUri->Uri,
           pOnvifInfo->Profiles[ChannelId].meta_profile[index].out_uri);
    ret = gen_new_uri(MediaUri->Uri,&port,1);

    if(ret != VZ_LOCALAPI_SUCCESS) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      strcpy(MediaUri->profiletoken,"");
      return VZ_LOCALAPI_FAILED;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetHostname(char *Name) {
  int i =0;
  __u8 flag =0;
  vz_tds_Hostname Hostname;
  memset(&Hostname,0,sizeof(Hostname));

  if((Name == NULL)||(strlen(Name) ==0)) {
    __u8 status_dhcp = 0;
    ControlSystemData(SYS_MSG_GET_DHCP_STATUS,&status_dhcp,sizeof(status_dhcp));

    if(status_dhcp) {
      //get the hostname from dhcp,now can not get the hostname from the dhcp
      gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
      return VZ_LOCALAPI_FAILED;

    } else {
      gLocalErr = VZ_LOCALAPI_HOSTNAME_INVALID;
      return VZ_LOCALAPI_FAILED;
    }
  }

  //check the hostname right or not start
  if(!((Name[0]>='a'&&Name[0]<='z')||(Name[0]>='A'&&Name[0]<='Z')||(Name[i]>='0'
                                                                    &&Name[i]<='9'))) {
    flag = 1;
  }

  if(!flag) {
    int loop = strlen(Name)-1;

    for(i =1; i<loop; i++) {
      if((Name[i]>='a'&&Name[i]<='z')||(Name[i]>='A'&&Name[i]<='Z')||(Name[i]>='0'
                                                                      &&Name[i]<='9')||(Name[i] == '-')||(Name[i] == '.')) {
      } else {
        flag = 1;
        break;
      }
    }

    if(!flag &&((Name[loop] == '-')||(Name[loop] == '.'))) {
      flag = 1;
    }
  }

  if(flag) {
    gLocalErr = VZ_LOCALAPI_HOSTNAME_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  //check the hostname right or not end
  strcpy(Hostname.name,Name);
  Hostname.FromDHCP = 0;

  if(DoOnvifSetHostName(&Hostname) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

void LocalSDK__GetEventAbility(char *EventAbility,char *type) {
  EventAbility[0] = 0;
  // 1:具体ability
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return;
}

int LocalSDK__GetAnalyticsModules(LocalSDK__Analytics *Analytics) {
  int AlgId=0;

  if(LocalSDK__CheckRuleToken(Analytics->token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }
}

int LocalSDK__SetPresetNum(LocalSDK__Analytics *Analytics) {
  int AlgId=0;

  if(LocalSDK__CheckRuleToken(Analytics->token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetRunMode(LocalSDK__Analytics *Analytics) {
  int AlgId=0;

  if(LocalSDK__CheckRuleToken(Analytics->token,&AlgId)==VZ_LOCALAPI_SUCCESS) {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetDrawMode(LocalSDK__Analytics *Analytics) {
  int channel=0;

  if(LocalSDK__CheckRuleToken(Analytics->token,&channel)==VZ_LOCALAPI_SUCCESS) {
    if(strcmp(Analytics->Analytics.Parameters.ElementItem[0].Name,
              "ivs_drawmode")==0) {
      gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
      return VZ_LOCALAPI_FAILED;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__ModifyAnalyticsModules(LocalSDK__Analytics *Analytics) {
  int channel=0;

  if(LocalSDK__CheckRuleToken(Analytics->token,&channel)==VZ_LOCALAPI_SUCCESS) {
    int i =0;

    if(Analytics->Analytics.Parameters.__sizeElementItem) {
      for(i =0; i<Analytics->Analytics.Parameters.__sizeElementItem; i++) {
        if(strcmp(Analytics->Analytics.Parameters.ElementItem[i].Name,
                  "ivs_drawmode")==0) {
          //set drawmode
          gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
          return VZ_LOCALAPI_FAILED;
        }
      }
    }

    if(Analytics->Analytics.Parameters.__sizeSimpleItem) {
      for(i =0; i<Analytics->Analytics.Parameters.__sizeSimpleItem; i++) {
        if(strcmp(Analytics->Analytics.Parameters.SimpleItem[i].Name,"preset_num")==0) {
          //set preset_num
          gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
          return VZ_LOCALAPI_FAILED;
        }

        if(strcmp(Analytics->Analytics.Parameters.SimpleItem[i].Name,"run_mode")==0) {
          //set run_mode
          gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
          return VZ_LOCALAPI_FAILED;
        }
      }
    }

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetChannelNum(int *ChannelNum);

int LocalSDK__CreateRecording(vz_onvif_Recording *Recording) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  if(pOnvifInfo->Recordings.Recording_num >= MAX_RECORD_NUM) {
    gLocalErr = VZ_LOCALAPI_RECORDING_EXCEED;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_Recordings Recordings;
  memcpy(&Recordings,&pOnvifInfo->Recordings,sizeof(Recordings));

  // create three tracks
  int i = 0;
  vz_onvif_Track *p_track = &Recording->Tracks[i];

  if (p_track) {
    strcpy(p_track->TrackToken, "VIDEO001");
    p_track->Configuration.TrackType = vz_TrackType_Video;
    i++;
  }

  p_track = &Recording->Tracks[i];

  if (p_track) {
    strcpy(p_track->TrackToken, "AUDIO001");
    p_track->Configuration.TrackType = vz_TrackType_Audio;
    i++;
  }

  p_track = &Recording->Tracks[i];

  if (p_track) {
    strcpy(p_track->TrackToken, "META001");
    p_track->Configuration.TrackType = vz_TrackType_Metadata;
    i++;
  }

  Recording->TrackNum = i;
  memcpy(&Recordings.Recording[Recordings.Recording_num],Recording,
         sizeof(vz_onvif_Recording));
  Recordings.Recording_num++;

  if(DoOnvifSetRecordings(&Recordings) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__DeleteRecording(char * p_RecordingToken) {
  //find RecordingToken
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i,j;
  int add_len = 0;
  vz_onvif_Recordings Recordings;
  memcpy(&Recordings,&pOnvifInfo->Recordings,sizeof(Recordings));

  for(i=0; i<Recordings.Recording_num; i++) {
    if(strcmp(p_RecordingToken,Recordings.Recording[i].RecordingToken) == 0) {
      strcpy(Recordings.Recording[i].RecordingToken,"");
      break;
    }
  }

  if(i==pOnvifInfo->Recordings.Recording_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  //delete recording
  int last_empty_str_index=Recordings.Recording_num;

  //排序
  for(j=0; j<Recordings.Recording_num; j++) {
    if(strlen(Recordings.Recording[j].RecordingToken)!=0) {
      if(last_empty_str_index <j) {
        memcpy(&(Recordings.Recording[last_empty_str_index]),&(Recordings.Recording[j]),
               sizeof(vz_onvif_Recording));
        memset(&(Recordings.Recording[j]),0,sizeof(vz_onvif_Recording));
        last_empty_str_index++;
      }

      add_len++;

    } else {
      if(last_empty_str_index >j)
        last_empty_str_index = j;

      memset(&(Recordings.Recording[j]),0,sizeof(vz_onvif_Recording));
    }
  }

  //长度修正
  Recordings.Recording_num = add_len;

  if(DoOnvifSetRecordings(&Recordings) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__CreateRecordingJob(vz_onvif_RecordingJob *RecordingJob) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}



int LocalSDK__DeleteRecordingJob(char *JobToken) {
  if(JobToken == NULL) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDINGJOB;
    return VZ_LOCALAPI_FAILED;
  }

  //find RecordingToken
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i,j;
  int add_len = 0;
  vz_onvif_RecordingJobs RecordingJobs;
  memcpy(&RecordingJobs,&pOnvifInfo->RecordingJobs,sizeof(RecordingJobs));

  for(i=0; i<RecordingJobs.RecordingJob_num; i++) {
    if(strcmp(JobToken,RecordingJobs.RecordingJob[i].RecordingJobToken)==0) {
      strcpy(RecordingJobs.RecordingJob[i].RecordingJobToken,"");
      break;
    }
  }

  if(i==pOnvifInfo->RecordingJobs.RecordingJob_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDINGJOB;
    return VZ_LOCALAPI_FAILED;
  }

  //delete recording
  int last_empty_str_index=RecordingJobs.RecordingJob_num;

  //排序
  for(j=0; j<RecordingJobs.RecordingJob_num; j++) {
    if(strlen(RecordingJobs.RecordingJob[j].RecordingJobToken)!=0) {
      if(last_empty_str_index <j) {
        memcpy(&(RecordingJobs.RecordingJob[last_empty_str_index]),
               &(RecordingJobs.RecordingJob[j]),sizeof(vz_onvif_RecordingJob));
        memset(&(RecordingJobs.RecordingJob[j]),0x0,sizeof(vz_onvif_RecordingJob));
        last_empty_str_index++;
      }

      add_len++;

    } else {
      if(last_empty_str_index >j)
        last_empty_str_index = j;

      memset(&(RecordingJobs.RecordingJob[j]),0x0,sizeof(vz_onvif_RecordingJob));
    }
  }

  //长度修正
  RecordingJobs.RecordingJob_num = add_len;

  if(DoOnvifSetRecordingJobs(&RecordingJobs) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__SetRecordingJobMode(LocalSDK__SetRecordingJob *RecordingJobMode) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}

int LocalSDK__GetRecordingJobState(vz_onvif_RecordingJob *JobState) {
  Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();

  if(pOnvifInfo->RecordingJobs.RecordingJob_num) {
    int i=0;
    int JobTokenFlag=0;

    for(i=0; i<pOnvifInfo->RecordingJobs.RecordingJob_num; i++) {
      if(strcmp(JobState->RecordingJobToken,
                pOnvifInfo->RecordingJobs.RecordingJob[i].RecordingJobToken)==0) {
        JobTokenFlag=1;
        break;
      }
    }

    if(JobTokenFlag) {
      memcpy(JobState,&pOnvifInfo->RecordingJobs.RecordingJob[i],
             sizeof(vz_onvif_RecordingJob));

    } else {
      gLocalErr = VZ_LOCALAPI_NO_RECORDINGJOB;
      return VZ_LOCALAPI_FAILED;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_NO_RECORDINGJOB;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__SetPreset(LocalSDK__PTZPreset *Preset) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}


int LocalSDK__RemovePreset(LocalSDK__PTZPreset *Preset) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}


int LocalSDK__GotoPreset(LocalSDK__PTZPreset *Preset) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}


int LocalSDK__SetImagingSettings(LocalSDK__ImagingSetting *ImagingSetting) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret= LocalSDK__LookUp_VideoSourceToken(ImagingSetting->VideoSourceToken,
                                             pOnvifInfo,&ChannelId,&index);

  if(ret==VZ_LOCALAPI_SUCCESS) {
    __u8 value;
    strcpy(ImagingSetting->VideoSourceToken,"");
    VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

    if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                         sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    int channel = sys_source_cfg[ChannelId].userID;

    if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
      strcpy(ImagingSetting->VideoSourceToken,
             pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken_S);
      ImagingSetting->channel= ChannelId;
      return VZ_LOCALAPI_FAILED;

    } else {
      if(ImagingSetting->Brightness>=0) {
        value=ImagingSetting->Brightness;

        if(value > VZ_IMAGING_OPTION_MAX) {
          gLocalErr = VZ_LOCALAPI_IMAGEVALUE_INVALID;
          return VZ_LOCALAPI_FAILED;
        }

        if(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_BRIGHTNESS, (void *)&value,
                                         sizeof(value))< 0) {
          gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
          return VZ_LOCALAPI_FAILED;
        }
      }

      if(ImagingSetting->ColorSaturation>=0) {
        value=ImagingSetting->ColorSaturation;

        if(value > VZ_IMAGING_OPTION_MAX) {
          gLocalErr = VZ_LOCALAPI_IMAGEVALUE_INVALID;
          return VZ_LOCALAPI_FAILED;
        }

        if(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_SATURATION, (void *)&value,
                                         sizeof(value))< 0) {
          gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
          return VZ_LOCALAPI_FAILED;
        }
      }

      if(ImagingSetting->Contrast>=0) {
        value=ImagingSetting->Contrast;

        if(value > VZ_IMAGING_OPTION_MAX) {
          gLocalErr = VZ_LOCALAPI_IMAGEVALUE_INVALID;
          return VZ_LOCALAPI_FAILED;
        }

        if(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_CONTRAST, (void *)&value,
                                         sizeof(value))< 0) {
          gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
          return VZ_LOCALAPI_FAILED;
        }
      }

      if(ImagingSetting->hue>=0) {
        value=ImagingSetting->hue;

        if(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_HUE, (void *)&value,
                                         sizeof(value))< 0) {
          gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
          return VZ_LOCALAPI_FAILED;
        }
      }

      /* useless
      if(ImagingSetting->Sharpness>=0){
      	value=ImagingSetting->Sharpness;
      	if(ControlSystemDataChannel(channel,SYS_MSG_SET_SHARPNESS, (void *)&value, sizeof(value))< 0){
      		gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      		return VZ_LOCALAPI_FAILED;
      	}
      }
      if(ImagingSetting->BLC.BLCLevel >=0){
      	value=ImagingSetting->BLC.Onoff;
      	if(ControlSystemDataChannel(channel,SYS_MSG_SET_BLC, (void *)&value, sizeof(value))< 0){
      		gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      		return VZ_LOCALAPI_FAILED;
      	}
      }
      */
      return VZ_LOCALAPI_SUCCESS;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    strcpy(ImagingSetting->VideoSourceToken,"");
    return VZ_LOCALAPI_FAILED;
  }
}
int LocalSDK__GetImagingSettings(LocalSDK__ImagingSetting *ImagingSetting) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret= LocalSDK__LookUp_VideoSourceToken(ImagingSetting->VideoSourceToken,
                                             pOnvifInfo,&ChannelId,&index);

  if(ret == VZ_LOCALAPI_SUCCESS) {
    strcpy(ImagingSetting->VideoSourceToken,"");
    VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

    if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                         sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    int channel = sys_source_cfg[ChannelId].userID;

    if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
      strcpy(ImagingSetting->VideoSourceToken,
             pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken_S);
      ImagingSetting->channel = ChannelId;
      return VZ_LOCALAPI_FAILED;

    } else {
      int	nBrightness,nSaturation,nContrast,nHue;

      if((ControlSystemData(MSG_GET_BRIGHTNESS, &nBrightness, sizeof(int)) < 0)
         ||(ControlSystemData(MSG_GET_SATURATION, &nSaturation, sizeof(int)) < 0)
         ||(ControlSystemData(MSG_GET_CONTRAST, &nContrast, sizeof(int)) < 0)
         ||(ControlSystemData(MSG_GET_HUE, &nHue, sizeof(int)) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

      ImagingSetting->Brightness=nBrightness;
      ImagingSetting->ColorSaturation=nSaturation;
      ImagingSetting->Contrast=nContrast;
      ImagingSetting->hue=nHue;
      return VZ_LOCALAPI_SUCCESS;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    strcpy(ImagingSetting->VideoSourceToken,"");
    return VZ_LOCALAPI_FAILED;
  }
}

int LocalSDK__GetImagingOptions(LocalSDK__ImagingOption *ImagingOption) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret= LocalSDK__LookUp_VideoSourceToken(ImagingOption->VideoSourceToken,
                                             pOnvifInfo,&ChannelId,&index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    strcpy(ImagingOption->VideoSourceToken,"");
    return VZ_LOCALAPI_FAILED;
  }

  strcpy(ImagingOption->VideoSourceToken,"");
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int channel = sys_source_cfg[ChannelId].userID;

  if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
    strcpy(ImagingOption->VideoSourceToken,
           pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken_S);
    ImagingOption->channel= ChannelId;
    return VZ_LOCALAPI_FAILED;
  }

  ImagingOption->Brightness.Min=VZ_IMAGING_OPTION_MIN;
  ImagingOption->Brightness.Max=VZ_IMAGING_OPTION_MAX;
  ImagingOption->ColorSaturation.Min=VZ_IMAGING_OPTION_MIN;
  ImagingOption->ColorSaturation.Max=VZ_IMAGING_OPTION_MAX;
  ImagingOption->Contrast.Min=VZ_IMAGING_OPTION_MIN;
  ImagingOption->Contrast.Max=VZ_IMAGING_OPTION_MAX;
  return VZ_LOCALAPI_SUCCESS;
}


void LocalSDK__GetEventTerminateTime(char *TimeoutString,int *Timeout) {
  *Timeout=60;//default 60s

  if(strlen(TimeoutString)) {
    char *temp=NULL;
    temp=strstr(TimeoutString,"PT");

    if(temp) {
      char time[8]= {0};
      int i=0;
      temp +=2;

      while(temp!='\0') {
        if((*temp >= '0' || *temp == '.') && (*temp <= '9')) {
          time[i]=*temp;
          i++;

        } else {
          float a = atof(time);
          int b=atoi(time);

          if(a-(float)b > 0.0001)
            b=b+1;

          *Timeout=b;

          if(*temp == 'M') {
            (*Timeout) *=60;
          }

          break;
        }

        temp++;
      }
    }

  } else {
    *Timeout=60;//default 60s
  }
}

int LocalSDK__CreatePullPointSubscription(LocalSDK__PullPointSubscription
                                          *Subscription ) {
  time_t tnow;
  int timeout=0;
  struct tm *gmtmnow;
  LocalSDK_DateTime datetime;
  char date[32]= {0};
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  /*
  __u8 value=1;
  if(ControlSystemData(SYS_MSG_SET_VZ_MOTIONENABLE, &value, sizeof(value)) < 0){
  	gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
  	return VZ_LOCALAPI_FAILED;
  }*/
  if(pOnvifInfo->Subscriptions.Subscriped_num >= MAX_SUBSCRIP_NUM) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  LocalSDK__GetEventTerminateTime(Subscription->Timeout,&timeout);
  int i=0;
  vz_onvif_Subscriptions Subscripvalue;
  memcpy(&Subscripvalue,&pOnvifInfo->Subscriptions,sizeof(Subscripvalue));

  if(strlen(Subscription->Filter)) {
    char *temp=strchr(Subscription->Filter,':');

    if(temp) {
      temp++;

      while(*temp != ' ' && *temp != '>' && i< STR_SHORT_LEN) {
        Subscripvalue.Subscrip[Subscripvalue.Subscriped_num].Filter[i++]=*temp++;
      }

      Subscripvalue.Subscrip[Subscripvalue.Subscriped_num].Filter[i++]=':';

      if(*temp == ' ') {
        temp=strchr(Subscription->Filter,'>');
      }

      if(temp) {
        temp++;

        while(*temp != '<' && i< STR_SHORT_LEN) {
          Subscripvalue.Subscrip[Subscripvalue.Subscriped_num].Filter[i++]=*temp++;
        }
      }
    }
  }

  time(&tnow);
  gmtmnow = gmtime(&tnow);
  tm2datetime(gmtmnow,&datetime);
  sprintf(date,"%4d-%02d-%02dT%02d:%02d:%02dZ_0",datetime.Date.Year,
          datetime.Date.Month,datetime.Date.Day,datetime.Time.Hour,datetime.Time.Minute,
          datetime.Time.Second);

  if(strlen(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_EVENTS].XAddr)) {
    sprintf(Subscription->Address,"%s/PullSubManager_%s",
            pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_EVENTS].XAddr,date);
  }

  Subscription->Time.CurrentTime=tnow;
  Subscription->Time.TerminationTime=tnow+timeout;
  strcpy(Subscripvalue.Subscrip[Subscripvalue.Subscriped_num].SubscripToken,
         Subscription->Address);
  Subscripvalue.Subscrip[Subscripvalue.Subscriped_num].TerminationTime =
    Subscription->Time.TerminationTime;
  Subscripvalue.Subscriped_num++;

  if(DoOnvifSetSubscription(&Subscripvalue) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}



#ifndef NEW_ONVIF

extern int ParseIVSRuleAlarm(VzIVSRuleAlarm *pRuleAlarm,
                             VzTargetsInfo *pTagsInfo);
int LocalSDK__GetVzTargetsInfo(VzTargetsInfo *pTI,struct timeval *pTV) {
  AV_DATA avData;
  unsigned int frameSN;
  int rt;
  int size = 0;

  pTI->num = 0;

  //获取最新帧的序列号
  GetAVData(AV_OP_GET_VZINFO_SERIAL, -1, &avData);

  if(avData.serial <= 0) {
    return(-1);
  }

  frameSN = avData.serial;

  rt = GetAVData(AV_OP_LOCK_VZINFO, frameSN, &avData);

  if(rt == RET_SUCCESS) {
    size = avData.size;

    do {
      if(size < 32) {
        size = -1;
        break;
      }

      VzIVSRuleAlarm *pRA = (VzIVSRuleAlarm *)(avData.ptr);
      pTV->tv_sec = avData.ts[0];
      pTV->tv_usec = avData.ts[1];


      size = ParseIVSRuleAlarm(pRA, pTI);

      if(size <= 0) {
        size = -1;
        break;
      }
    } while(0);

    GetAVData(AV_OP_UNLOCK_VZINFO, frameSN, NULL);

    return(size);

  } else if(rt == RET_NO_VALID_DATA) {
    //正常情况：av_server还未准备好所请求的数据
    return(0);

  } else {
    return(-1);
  }
}

#endif

void LocalSDK__GetStreamEncoderConfig(vz_onvif_VideoEncoderConfig
                                      *VideoEncoderConfig,VZ_StreamCodecConfig *StreamCode,
                                      vz_onvif_VideoEncoderConfig *InputConfig) {
  if(StreamCode->nEncodeType == VZ_ENC_H264)
    VideoEncoderConfig->Encoding=vz__VideoEncoding__H264;

  else if(StreamCode->nEncodeType == VZ_ENC_MPEG4)
    VideoEncoderConfig->Encoding=vz__VideoEncoding__MPEG4;

  else if(StreamCode->nEncodeType == VZ_ENC_JPEG)
    VideoEncoderConfig->Encoding=vz__VideoEncoding__JPEG;

  if(VideoEncoderConfig->Encoding == vz__VideoEncoding__H264) {
    memcpy(&VideoEncoderConfig->H264,&InputConfig->H264,
           sizeof(vz_onvif_VideoEncoder_H264));
  }

  VideoEncoderConfig->Resolution.Width=StreamCode->nVideoSizeXY>>16&0XFFFF;
  VideoEncoderConfig->Resolution.Height=StreamCode->nVideoSizeXY&0XFFFF;
  VideoEncoderConfig->Quality=1;
  VideoEncoderConfig->RateControl.BitrateLimit=StreamCode->nBitrate/1000;
  VideoEncoderConfig->RateControl.FrameRateLimit=StreamCode->nFrameRate;
  VideoEncoderConfig->RateControl.EncodingInterval=30;
}

void LocalSDK__GetJPEGStreamEncoderConfig(vz_onvif_VideoEncoderConfig
                                          *VideoEncoderConfig,VZ_JpegCodecConfig *StreamCode) {
  VideoEncoderConfig->Encoding=vz__VideoEncoding__JPEG;//jpeg
  VideoEncoderConfig->Resolution.Width=StreamCode->nVideoSizeXY>>16&0XFFFF;
  VideoEncoderConfig->Resolution.Height=StreamCode->nVideoSizeXY&0XFFFF;
  VideoEncoderConfig->Quality=StreamCode->nJpegQuality;
  VideoEncoderConfig->RateControl.FrameRateLimit=StreamCode->nFrameRate;
  VideoEncoderConfig->RateControl.BitrateLimit=0;
  VideoEncoderConfig->RateControl.EncodingInterval=30;
}

void LocalSDK__GetSourceBoundsConfig(vz_onvif_VideoSourceConfig
                                     *VideoSourceConfig) {
  VideoSourceConfig->Bounds.x=0;
  VideoSourceConfig->Bounds.y=0;
  VideoSourceConfig->Bounds.width=1280;
  VideoSourceConfig->Bounds.height=720;
}

void LocalSDK__GetStreamCodecConfig(vz_onvif_Profile *meta_profile,
                                    VZ_StreamCodecConfig *StreamCode,vz_onvif_VideoEncoderConfig *InputConfig) {
  LocalSDK__GetStreamEncoderConfig(&meta_profile->VideoEncoderConfig,StreamCode,
                                   InputConfig);
  LocalSDK__GetSourceBoundsConfig(&meta_profile->VideoSourceConfig);
}
/*
void LocalSDK__Get_IPC2_StreamEncoderConfig(vz_onvif_VideoEncoderConfig *VideoEncoderConfig)
{
	VZ_StreamCodecConfig StreamCode;
	StreamCode.nRateControl=0;
	StreamCode.nBitrate=512000;
	StreamCode.nVideoSizeXY=352<<16|288;
	StreamCode.nFrameRate=25;
	LocalSDK__GetStreamEncoderConfig(VideoEncoderConfig,&StreamCode);
}
void LocalSDK__Get_IPC2_StreamCodecConfig(vz_onvif_Profile *meta_profile)
{
	LocalSDK__Get_IPC2_StreamEncoderConfig(&meta_profile->VideoEncoderConfig);
	LocalSDK__GetSourceBoundsConfig(&meta_profile->VideoSourceConfig);
}
*/
int LocalSDK__GetRemoteProfiles(LocalSDK__Profiles *meta_profiles,int channel) {
  int j=0;
  int i=0;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  for(j=0; j<MAX_CHANNEL; j++) {
    if((sys_source_cfg[j].streamType == AVS_CAP_IPC)
       &&(sys_source_cfg[j].userID == channel)) {
      break;
    }
  }

  if(j != MAX_CHANNEL) {
    for(i=0; i<MAX_PROFILE_NUM; i++) {
      if(strlen(pOnvifInfo->Profiles[j].meta_profile[i].token)) {
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].token,
               pOnvifInfo->Profiles[j].meta_profile[i].token);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].Name,
               pOnvifInfo->Profiles[j].meta_profile[i].Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.Name,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.token,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.token);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig.Name,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig.token,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token);
        meta_profiles->Profile[meta_profiles->Profile_num].VA_ID=
          pOnvifInfo->Profiles[j].meta_profile[i].VA_ID;
        //strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoAnalyticsConfig.Name,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.Name);
        //strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoAnalyticsConfig.token,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.token);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].AudioEncoderConfiguration.Name,
               pOnvifInfo->Profiles[j].meta_profile[i].AudioEncoderConfiguration.Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].AudioEncoderConfiguration.token,
               pOnvifInfo->Profiles[j].meta_profile[i].AudioEncoderConfiguration.token);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].AudioSourceConfiguration.Name,
               pOnvifInfo->Profiles[j].meta_profile[i].AudioSourceConfiguration.Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].AudioSourceConfiguration.token,
               pOnvifInfo->Profiles[j].meta_profile[i].AudioSourceConfiguration.token);

        meta_profiles->Profile[meta_profiles->Profile_num].fixed=
          pOnvifInfo->Profiles[j].meta_profile[i].fixed;
        meta_profiles->Profile[meta_profiles->Profile_num].Type=
          pOnvifInfo->Profiles[j].meta_profile[i].Type;
        meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.UserCount=
          pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.UserCount;
        memcpy(&meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.Bounds,
               &pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds,
               sizeof(vz_onvif_VideoSourceRectangle));
        //if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Remote_Profile_Sub){
        //	LocalSDK__Get_IPC2_StreamCodecConfig(&meta_profiles->Profile[meta_profiles->Profile_num]);
        //}
        meta_profiles->Profile_num++;
      }
    }

    return VZ_LOCALAPI_SUCCESS;
  }

  return VZ_LOCALAPI_FAILED;
}

int LocalSDK__GetLocalProfiles_OneChannel(LocalSDK__Profiles *meta_profiles,
                                          int Channel) {
  int j=Channel;
  int i=0;
  int index=0;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  if(Channel<0 || Channel>MAX_CHANNEL)
    return VZ_LOCALAPI_FAILED;

  if(sys_source_cfg[Channel].streamType == AVS_CAP_V4L2) {

    VZ_StreamCodecConfig maincode;//,subcode;

    //VZ_JpegCodecConfig jpegcode;
    if(ControlSystemData(MSG_GET_MAIN_STREAMCODE, &maincode,
                         sizeof(VZ_StreamCodecConfig)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    index = sys_source_cfg[j].userID;

    for(i=0; i<MAX_PROFILE_NUM; i++) {
      if(strlen(
           pOnvifInfo->Profiles[j].meta_profile[i].token)) { //now only have main stream and alarm stream
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].token,
               pOnvifInfo->Profiles[j].meta_profile[i].token);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].Name,
               pOnvifInfo->Profiles[j].meta_profile[i].Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.Name,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.token,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.token);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig.Name,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.Name);
        strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig.token,
               pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token);
        meta_profiles->Profile[meta_profiles->Profile_num].VA_ID=
          pOnvifInfo->Profiles[j].meta_profile[i].VA_ID;
        //strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoAnalyticsConfig.Name,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.Name);
        //strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoAnalyticsConfig.token,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.token);
        meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.UserCount=
          pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.UserCount;
        meta_profiles->Profile[meta_profiles->Profile_num].fixed=
          pOnvifInfo->Profiles[j].meta_profile[i].fixed;
        meta_profiles->Profile[meta_profiles->Profile_num].Type=
          pOnvifInfo->Profiles[j].meta_profile[i].Type;

        if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Local_Profile_Main) {
          LocalSDK__GetStreamCodecConfig(
            &meta_profiles->Profile[meta_profiles->Profile_num],&maincode,
            &pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig);

        } else if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Local_Profile_Sub) {
          //LocalSDK__GetStreamCodecConfig(&meta_profiles->Profile[meta_profiles->Profile_num],&subcode,&pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig);
        } else if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Local_Profile_Jpeg) {
          //LocalSDK__GetJPEGStreamEncoderConfig(&meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig,&jpegcode);
          LocalSDK__GetSourceBoundsConfig(
            &meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig);

        } else {
          LocalSDK__GetSourceBoundsConfig(
            &meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig);
        }

        meta_profiles->Profile_num++;
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetLocalProfiles(LocalSDK__Profiles *meta_profiles) {
  int j=0;
  int i=0;
  int channel=0;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  for(j=0; j<MAX_CHANNEL; j++) {
    if(sys_source_cfg[j].streamType == AVS_CAP_V4L2) {
      VZ_StreamCodecConfig maincode;

      if(ControlSystemData(MSG_GET_MAIN_STREAMCODE, &maincode,
                           sizeof(VZ_StreamCodecConfig)) < 0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

      channel = sys_source_cfg[j].userID;

      for(i=0; i<MAX_PROFILE_NUM; i++) {
        if(strlen(pOnvifInfo->Profiles[j].meta_profile[i].token)
           /*&&(pOnvifInfo->Profiles[j].meta_profile[i].Type != Local_Profile_Sub)
           &&(pOnvifInfo->Profiles[j].meta_profile[i].Type < Profile_Type_Max)*/) { //now only have main stream and alarm stream
          strcpy(meta_profiles->Profile[meta_profiles->Profile_num].token,
                 pOnvifInfo->Profiles[j].meta_profile[i].token);
          strcpy(meta_profiles->Profile[meta_profiles->Profile_num].Name,
                 pOnvifInfo->Profiles[j].meta_profile[i].Name);
          strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.Name,
                 pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Name);
          strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.token,
                 pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.token);
          strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig.Name,
                 pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.Name);
          strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig.token,
                 pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token);
          meta_profiles->Profile[meta_profiles->Profile_num].VA_ID=
            pOnvifInfo->Profiles[j].meta_profile[i].VA_ID;
          //strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoAnalyticsConfig.Name,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.Name);
          //strcpy(meta_profiles->Profile[meta_profiles->Profile_num].VideoAnalyticsConfig.token,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.token);
          meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig.UserCount=
            pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.UserCount;
          meta_profiles->Profile[meta_profiles->Profile_num].fixed=
            pOnvifInfo->Profiles[j].meta_profile[i].fixed;
          meta_profiles->Profile[meta_profiles->Profile_num].Type=
            pOnvifInfo->Profiles[j].meta_profile[i].Type;

          if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Local_Profile_Main) {
            LocalSDK__GetStreamCodecConfig(
              &meta_profiles->Profile[meta_profiles->Profile_num],&maincode,
              &pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig);

          } else if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Local_Profile_Sub) {
            //LocalSDK__GetStreamCodecConfig(&meta_profiles->Profile[meta_profiles->Profile_num],&subcode,&pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig);
          } else if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Local_Profile_Jpeg) {
            //LocalSDK__GetJPEGStreamEncoderConfig(&meta_profiles->Profile[meta_profiles->Profile_num].VideoEncoderConfig,&jpegcode);
            LocalSDK__GetSourceBoundsConfig(
              &meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig);

          } else {
            LocalSDK__GetSourceBoundsConfig(
              &meta_profiles->Profile[meta_profiles->Profile_num].VideoSourceConfig);
          }

#ifdef NEW_ONVIF
          meta_profiles->Profile[meta_profiles->Profile_num].PTZCfg_id =
            pOnvifInfo->Profiles[j].meta_profile[i].PTZCfg_id;
#endif
          meta_profiles->Profile_num++;
        }
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__UpdateProfiles(vz_onvif_Profile *updateProfile) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(updateProfile->token,pOnvifInfo,
                                        &ChannelId,&index);

  if(ret==VZ_LOCALAPI_FAILED) { //no such a profile token
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_Profiles value;
  int k=0;
  memcpy(&value,&pOnvifInfo->Profiles[ChannelId],sizeof(vz_onvif_Profiles));
  //if(strcmp(value.meta_profile[i].oldtoken,updateProfile->oldtoken) != 0)
  {
    strcpy(value.meta_profile[index].oldtoken,updateProfile->oldtoken);
    strcpy(value.meta_profile[index].VideoEncoderConfig.oldtoken,
           updateProfile->VideoEncoderConfig.oldtoken);
    value.meta_profile[index].VideoEncoderConfig.Encoding=
      updateProfile->VideoEncoderConfig.Encoding;
    memcpy(&value.meta_profile[index].VideoEncoderConfig.Resolution,
           &updateProfile->VideoEncoderConfig.Resolution,sizeof(vz_onvif_VideoResolution));
    strcpy(value.meta_profile[index].VideoSourceConfig.oldtoken,
           updateProfile->VideoSourceConfig.oldtoken);

    if(strlen(updateProfile->AudioSourceConfiguration.oldtoken)) {
      strcpy(value.meta_profile[index].AudioSourceConfiguration.oldtoken,
             updateProfile->AudioSourceConfiguration.oldtoken);
      strcpy(value.meta_profile[index].AudioSourceConfiguration.oldSourceToken,
             updateProfile->AudioSourceConfiguration.oldSourceToken);
      //value.meta_profile[index].AudioSourceConfiguration.UserCount=updateProfile->AudioSourceConfiguration.UserCount;
    }

    if(strlen(updateProfile->AudioEncoderConfiguration.oldtoken)) {
      strcpy(value.meta_profile[index].AudioEncoderConfiguration.oldtoken,
             updateProfile->AudioEncoderConfiguration.oldtoken);
      //strcpy(value.meta_profile[index].AudioEncoderConfiguration.SessionTimeout,updateProfile->AudioEncoderConfiguration.SessionTimeout);
      //value.meta_profile[index].AudioEncoderConfiguration.Encoding=updateProfile->AudioEncoderConfiguration.Encoding;
      //value.meta_profile[index].AudioEncoderConfiguration.Bitrate=updateProfile->AudioEncoderConfiguration.Bitrate;
      //value.meta_profile[index].AudioEncoderConfiguration.SampleRate=updateProfile->AudioEncoderConfiguration.SampleRate;
    }

    for(k=0; k<MAX_PROFILE_NUM; k++) {
      memcpy(&value.meta_profile[k].VideoSourceConfig.Bounds,
             &updateProfile->VideoSourceConfig.Bounds,sizeof(vz_onvif_VideoSourceRectangle));
      //value.meta_profile[k].VideoSourceConfig.UserCount=updateProfile->VideoSourceConfig.UserCount;
    }

    if(DoOnvifSetProfiles(&value,ChannelId) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }
  //save remote video source token
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
    int curchannel= sys_source_cfg[ChannelId].userID;

    if(strcmp(pOnvifInfo->remote_server[curchannel].video.VideoSourceRelayToken,
              updateProfile->VideoSourceConfig.oldtoken)) {
      vz_onvif_RemoteVideoInfo videoInfo;
      memcpy(&videoInfo.RelayVSCfgOptions,
             &pOnvifInfo->remote_server[curchannel].video.RelayVSCfgOptions,
             sizeof(videoInfo.RelayVSCfgOptions));
      strcpy(videoInfo.VideoSourceToken,
             pOnvifInfo->remote_server[curchannel].video.VideoSourceToken);
      strcpy(videoInfo.VideoSourceRelayToken,
             updateProfile->VideoSourceConfig.oldtoken);
      strcpy(videoInfo.VideoSourceRelayToken_S,
             updateProfile->VideoSourceConfig.token);

      if(DoOnvifSetRVideoInfo(&videoInfo,curchannel) < 0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;

}


int LocalSDK__GetProfile(LocalSDK__Profiles *meta_profiles) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int channel=0;
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(meta_profiles->Profile[0].token,
                                        pOnvifInfo,&ChannelId,&index);

  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(ret==VZ_LOCALAPI_FAILED) { //no such a profile token
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    //meta_profiles->Profile_num=0;
    strcpy(meta_profiles->Profile[0].token,"");
    return VZ_LOCALAPI_FAILED;

  } else if((sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC)
            &&(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type !=
               Remote_Profile_Alarm)) { //the profile token is relay
    meta_profiles->Profile_num=0;
    meta_profiles->channel= ChannelId;
    strcpy(meta_profiles->Profile[0].token,"");
    strcpy(meta_profiles->Profile[0].token,
           pOnvifInfo->Profiles[ChannelId].meta_profile[index].oldtoken);
    return VZ_LOCALAPI_FAILED;
  }

  VZ_StreamCodecConfig maincode;

  if(ControlSystemData(MSG_GET_MAIN_STREAMCODE, &maincode,
                       sizeof(VZ_StreamCodecConfig)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }


  channel=sys_source_cfg[ChannelId].userID;
  strcpy(meta_profiles->Profile[0].Name,
         pOnvifInfo->Profiles[ChannelId].meta_profile[index].Name);
  strcpy(meta_profiles->Profile[0].VideoSourceConfig.Name,
         pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoSourceConfig.Name);
  strcpy(meta_profiles->Profile[0].VideoSourceConfig.token,
         pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoSourceConfig.token);
  strcpy(meta_profiles->Profile[0].VideoEncoderConfig.Name,
         pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig.Name);
  strcpy(meta_profiles->Profile[0].VideoEncoderConfig.token,
         pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig.token);
  meta_profiles->Profile[0].VA_ID=
    pOnvifInfo->Profiles[ChannelId].meta_profile[index].VA_ID;
  //strcpy(meta_profiles->Profile[0].VideoAnalyticsConfig.Name,pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoAnalyticsConfig.Name);
  //strcpy(meta_profiles->Profile[0].VideoAnalyticsConfig.token,pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoAnalyticsConfig.token);
  meta_profiles->Profile[0].VideoSourceConfig.UserCount =
    pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoSourceConfig.UserCount;
  meta_profiles->Profile[0].fixed=
    pOnvifInfo->Profiles[ChannelId].meta_profile[index].fixed;

  if(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
     Local_Profile_Sub) {
    //LocalSDK__GetStreamCodecConfig(&meta_profiles->Profile[0],&subcode,&pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig);
  } else if(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
            Local_Profile_Main) {
    LocalSDK__GetStreamCodecConfig(&meta_profiles->Profile[0],&maincode,
                                   &pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig);

  } else if(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
            Local_Profile_Jpeg) {
    //LocalSDK__GetJPEGStreamEncoderConfig(&meta_profiles->Profile[0].VideoEncoderConfig,&jpegcode);
    LocalSDK__GetSourceBoundsConfig(&meta_profiles->Profile[0].VideoSourceConfig);

  } else if(/*(pOnvifInfo->Profiles[j].meta_profile[i].Type == Remote_Profile_Sub) ||*/
    (pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
     Remote_Profile_Alarm)) {
    meta_profiles->Profile[0].VideoSourceConfig.UserCount
      =pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoSourceConfig.UserCount;
    memcpy(&meta_profiles->Profile[0].VideoSourceConfig.Bounds,
           &pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoSourceConfig.Bounds,
           sizeof(vz_onvif_VideoSourceRectangle));
    //if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Remote_Profile_Sub)
    //	LocalSDK__Get_IPC2_StreamEncoderConfig(&meta_profiles->Profile[0].VideoEncoderConfig);

  } else {
    LocalSDK__GetSourceBoundsConfig(&meta_profiles->Profile[0].VideoSourceConfig);
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetVideoEncoderConfiguration(LocalSDK__OneVideoEncoderConfig
                                           *VideoEncoderConfig) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int channel=0;
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_VideoEncoderToken(VideoEncoderConfig->BaseConfig.token,
                                             pOnvifInfo,&ChannelId,&index);

  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(ret==VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    VideoEncoderConfig->RelayFlag=1;
    strcpy(VideoEncoderConfig->BaseConfig.token,"");
    return VZ_LOCALAPI_FAILED;

  } else if((sys_source_cfg[ChannelId].streamType ==
             AVS_CAP_IPC)/*&&(pOnvifInfo->Profiles[j].meta_profile[i].Type != Remote_Profile_Sub)*/) { //the profile token is relay
    VideoEncoderConfig->RelayFlag=1;
    VideoEncoderConfig->channel = ChannelId;
    strcpy(VideoEncoderConfig->BaseConfig.token,"");
    strcpy(VideoEncoderConfig->BaseConfig.token,
           pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig.oldtoken);

    return VZ_LOCALAPI_FAILED;
  }

  VZ_StreamCodecConfig maincode;

  if(ControlSystemData(MSG_GET_MAIN_STREAMCODE, &maincode,
                       sizeof(VZ_StreamCodecConfig)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  channel = sys_source_cfg[ChannelId].userID;
  strcpy(VideoEncoderConfig->BaseConfig.Name,
         pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig.Name);
  strcpy(VideoEncoderConfig->BaseConfig.token,
         pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig.token);

  if(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
     Local_Profile_Sub) {
    //LocalSDK__GetStreamEncoderConfig(&VideoEncoderConfig->BaseConfig,&subcode,&pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig);
  } else if(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
            Local_Profile_Main) {
    LocalSDK__GetStreamEncoderConfig(&VideoEncoderConfig->BaseConfig,&maincode,
                                     &pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig);
  }

  /*else if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Remote_Profile_Sub)
  {
  	LocalSDK__Get_IPC2_StreamEncoderConfig(&VideoEncoderConfig->BaseConfig);
  }*/
  return VZ_LOCALAPI_SUCCESS;
}
#ifdef DM3730//6446 only alarm profile,not include the VideoEncoder info

int LocalSDK__GetVideoEncoderConfig_OneChannel(LocalSDK__VideoEncoderConfigs
                                               *EncoderConfigs,int Channel) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int j=Channel;

  if(Channel<0 || Channel>MAX_CHANNEL)
    return VZ_LOCALAPI_FAILED;

  if(sys_source_cfg[Channel].streamType == AVS_CAP_V4L2) {
    //int index = sys_source_cfg[j].userID;
    VZ_StreamCodecConfig maincode;

    if(ControlSystemData(MSG_GET_MAIN_STREAMCODE, &maincode,
                         sizeof(VZ_StreamCodecConfig)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    //get main stream
    if(EncoderConfigs->VideoEncoder_num < VZ_LIST_MAX_NUM
       && strlen(pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig.token)) {
      if(sys_source_cfg[j].enable)
        LocalSDK__GetStreamEncoderConfig(
          &EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num],&maincode,
          &pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig);

      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].Name,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig.Name);
      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].token,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig.token);
      EncoderConfigs->VideoEncoder_num++;
    }

    //get sub stream
    if(EncoderConfigs->VideoEncoder_num < VZ_LIST_MAX_NUM
       && strlen(pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig.token)) {
      //if(sys_source_cfg[j].enable)
      //	LocalSDK__GetStreamEncoderConfig(&EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num],&subcode,&pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig);
      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].Name,
             pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig.Name);
      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].token,
             pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig.token);
      EncoderConfigs->VideoEncoder_num++;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetVideoEncoderConfigurations(LocalSDK__VideoEncoderConfigs
                                            *EncoderConfigs) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VZ_StreamCodecConfig maincode;

  if(ControlSystemData(MSG_GET_MAIN_STREAMCODE, &maincode,
                       sizeof(VZ_StreamCodecConfig)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int j=0;

  for(j=0; j<MAX_CHANNEL; j++) {
    //int channel = sys_source_cfg[j].userID;
    //get main stream
    if(EncoderConfigs->VideoEncoder_num < VZ_LIST_MAX_NUM
       && strlen(pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig.token)) {
      if(sys_source_cfg[j].enable)
        LocalSDK__GetStreamEncoderConfig(
          &EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num],&maincode,
          &pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig);

      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].Name,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig.Name);
      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].token,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoEncoderConfig.token);
      EncoderConfigs->VideoEncoder_num++;
    }

    //get sub stream
    if(EncoderConfigs->VideoEncoder_num < VZ_LIST_MAX_NUM
       && strlen(pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig.token)) {
      //if(sys_source_cfg[j].enable)
      //	LocalSDK__GetStreamEncoderConfig(&EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num],&subcode,&pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig);
      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].Name,
             pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig.Name);
      strcpy(EncoderConfigs->VideoEncoderConfig[EncoderConfigs->VideoEncoder_num].token,
             pOnvifInfo->Profiles[j].meta_profile[1].VideoEncoderConfig.token);
      EncoderConfigs->VideoEncoder_num++;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

#endif
int LocalSDK__SetVideoEncoderConfiguration(LocalSDK__OneVideoEncoderConfig
                                           *VideoEncoderConfig) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int faild=0;
  int value=0;
  unsigned int VideoSizeXY=0;					///< 分辨率
  int channel=0;
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_VideoEncoderToken(VideoEncoderConfig->BaseConfig.token,
                                             pOnvifInfo,&ChannelId,&index);

  if(ret==VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    VideoEncoderConfig->RelayFlag=1;
    strcpy(VideoEncoderConfig->BaseConfig.token,"");
    return VZ_LOCALAPI_FAILED;

  } else if((sys_source_cfg[ChannelId].streamType ==
             AVS_CAP_IPC)) { //the profile token is relay
#if 0//def DM3730
    if(i==1)//the VideoEncoderConfigToken is belong to the second profile ,if it is true ,return unsupport
      gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;

    else
#endif
    {
      VideoEncoderConfig->RelayFlag=1;
      VideoEncoderConfig->channel = ChannelId;
      strcpy(VideoEncoderConfig->BaseConfig.token,"");
      strcpy(VideoEncoderConfig->BaseConfig.token,
             pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoEncoderConfig.oldtoken);
    }

    return VZ_LOCALAPI_FAILED;
  }

  channel = sys_source_cfg[ChannelId].userID;
  strcpy(VideoEncoderConfig->BaseConfig.token,"");

  if(VideoEncoderConfig->BaseConfig.Resolution.Width==176
     &&VideoEncoderConfig->BaseConfig.Resolution.Height==144) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_QCIF;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==320
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==240) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_QVGA;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==352
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==288) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_CIF;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==640
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==360) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_QHD;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==640
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==480) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_VGA;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==704
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==576) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_CIF4;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==720
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==576) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_D1;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==800
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==600) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_SVGA;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==1024
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==768) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_XGA;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==1280
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==720) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_HD;

  } else if(VideoEncoderConfig->BaseConfig.Resolution.Width==1920
            &&VideoEncoderConfig->BaseConfig.Resolution.Height==1080) {
    VideoSizeXY =
      VideoEncoderConfig->BaseConfig.Resolution.Width<<16|VideoEncoderConfig->BaseConfig.Resolution.Height;
    //VideoSizeXY = VZ_RESO_FHD;

  } else {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if(VideoEncoderConfig->H264flag
     &&(VideoEncoderConfig->BaseConfig.H264.GovLength > MAX_GOVLENGTH
        || VideoEncoderConfig->BaseConfig.H264.GovLength < MIN_GOVLENGTH)) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
     Local_Profile_Sub) { //sub stream
    value=VideoEncoderConfig->BaseConfig.RateControl.BitrateLimit;
    value *= 1000;

    if (ControlSystemDataLocalChannel(channel,SYS_MSG_SET_SUB_BITRATE,
                                      (void *)&value, sizeof(value)) < 0) {
      faild=1;
    }

    value=VideoEncoderConfig->BaseConfig.RateControl.FrameRateLimit;

    if ((faild == 0) &&
        ControlSystemDataLocalChannel(channel,SYS_MSG_SET_SUB_FRAMERATE, (void *)&value,
                                      sizeof(value)) < 0) {
      faild=1;
    }

    if ((faild == 0) &&
        ControlSystemDataLocalChannel(channel,SYS_MSG_SET_SUB_VIDEOSIZEXY,
                                      (void *)&VideoSizeXY, sizeof(VideoSizeXY)) < 0) {
      faild=1;
    }

  } else if(pOnvifInfo->Profiles[ChannelId].meta_profile[index].Type ==
            Local_Profile_Main) { //main stresm
    value=VideoEncoderConfig->BaseConfig.RateControl.BitrateLimit;
    value *= 1000;

    if (ControlSystemDataLocalChannel(channel,SYS_MSG_SET_BITRATE, (void *)&value,
                                      sizeof(value)) < 0) {
      faild=1;
    }

    value=VideoEncoderConfig->BaseConfig.RateControl.FrameRateLimit;

    if ((faild == 0) &&
        ControlSystemDataLocalChannel(channel,SYS_MSG_SET_FRAMERATE, (void *)&value,
                                      sizeof(value)) < 0) {
      faild=1;
    }

    if ((faild == 0) &&
        ControlSystemDataLocalChannel(channel,SYS_MSG_SET_VIDEOSIZEXY,
                                      (void *)&VideoSizeXY, sizeof(VideoSizeXY)) < 0) {
      faild=1;
    }

    if(VideoEncoderConfig->BaseConfig.Encoding==vz__VideoEncoding__H264)
      value = VZ_ENC_H264;

    else if(VideoEncoderConfig->BaseConfig.Encoding==vz__VideoEncoding__MPEG4)
      value = VZ_ENC_MPEG4;

    else if(VideoEncoderConfig->BaseConfig.Encoding==vz__VideoEncoding__JPEG)
      value = VZ_ENC_JPEG;

    else {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      return VZ_LOCALAPI_FAILED;
    }

    if ((faild == 0) &&
        ControlSystemDataLocalChannel(channel, SYS_MSG_SET_MAINENCODETYPE,
                                      (void *)&value, sizeof(value))<0) {
      faild=1;
    }
  }

  vz_onvif_Profiles Profilevalue;
  memcpy(&Profilevalue,&pOnvifInfo->Profiles[ChannelId],
         sizeof(vz_onvif_Profiles));
  memcpy(&Profilevalue.meta_profile[index].VideoEncoderConfig.H264,
         &VideoEncoderConfig->BaseConfig.H264,sizeof(vz_onvif_VideoEncoder_H264));
  memcpy(&Profilevalue.meta_profile[index].VideoEncoderConfig.RateControl,
         &VideoEncoderConfig->BaseConfig.RateControl,sizeof(vz_onvif_VideoRateControl));
  memcpy(&Profilevalue.meta_profile[index].VideoEncoderConfig.Resolution,
         &VideoEncoderConfig->BaseConfig.Resolution,sizeof(vz_onvif_VideoResolution));
  Profilevalue.meta_profile[index].VideoEncoderConfig.Quality =
    VideoEncoderConfig->BaseConfig.Quality;
  Profilevalue.meta_profile[index].VideoEncoderConfig.Encoding=
    VideoEncoderConfig->BaseConfig.Encoding;

  if(DoOnvifSetProfiles(&Profilevalue,ChannelId) < 0) {
    faild=1;
  }

  /*
  else if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Remote_Profile_Sub){
  	value=VideoEncoderConfig->BaseConfig.RateControl.BitrateLimit;
  	value *= 1000;
  	//need add in the furture

  }
  */
  if(faild) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

static void LocalSDK__GetStreamResolutions(vz_onvif_VideoResolution *Resolution,
                                           unsigned int value,int *Number) {
  int index =0;

  if((value & 1<<VZ_RESO_QCIF)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=176;
    Resolution[index].Height=144;
    index ++;
  }

  if((value & 1<<VZ_RESO_QVGA)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=320;
    Resolution[index].Height=240;
    index ++;
  }

  if((value & 1<<VZ_RESO_CIF)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=352;
    Resolution[index].Height=288;
    index ++;
  }

  if((value & 1<<VZ_RESO_QHD)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=640;
    Resolution[index].Height=360;
    index ++;
  }

  if((value & 1<<VZ_RESO_VGA)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=640;
    Resolution[index].Height=480;
    index ++;
  }

  if((value & 1<<VZ_RESO_CIF4)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=704;
    Resolution[index].Height=576;
    index ++;
  }

  if((value & 1<<VZ_RESO_D1)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=720;
    Resolution[index].Height=576;
    index ++;
  }

  if((value & 1<<VZ_RESO_SVGA)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=800;
    Resolution[index].Height=600;
    index ++;
  }

  if((value & 1<<VZ_RESO_XGA)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=1024;
    Resolution[index].Height=768;
    index ++;
  }

  if((value & 1<<VZ_RESO_HD)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=1280;
    Resolution[index].Height=720;
    index ++;
  }

  if((value & 1<<VZ_RESO_FHD)&&(index < VZ_RESOLUTION_MAX_NUM)) {
    Resolution[index].Width=1920;
    Resolution[index].Height=1080;
    index ++;
  }

  *Number = index;
}
int LocalSDK__GetVideoEncoderConfigurationOptions(
  LocalSDK__VideoEncoderConfigurationOption *Option) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  int j=0;
  int flag=0;

  for(j=0; j<MAX_CHANNEL; j++) {
    for(i=0; i<MAX_PROFILE_NUM; i++) {
      if(strlen(Option->ProfileToken)) {
        if(strlen(Option->ConfigurationToken)
           &&(strcmp(Option->ProfileToken,
                     pOnvifInfo->Profiles[j].meta_profile[i].token)==0)
           &&(strcmp(Option->ConfigurationToken,
                     pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token)==0)) {
          flag =1;
          break;

        } else if(strcmp(Option->ProfileToken,
                         pOnvifInfo->Profiles[j].meta_profile[i].token)==0) {
          flag =1;
          break;
        }

      } else {
        if(strlen(Option->ConfigurationToken)
           &&strcmp(Option->ConfigurationToken,
                    pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token)==0) {
          flag =1;
          break;
        }
      }
    }

    if(flag)
      break;
  }

  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }


  if((j<MAX_CHANNEL)
     && (sys_source_cfg[j].streamType == AVS_CAP_IPC)) { //the profile token is relay
    Option->RelayFlag=1;
    Option->channel = j;

    if(strlen(Option->ProfileToken)) {
      strcpy(Option->ProfileToken,"");
      strcpy(Option->ProfileToken,pOnvifInfo->Profiles[j].meta_profile[i].oldtoken);
    }

    if(strlen(Option->ConfigurationToken)) {
      strcpy(Option->ConfigurationToken,"");
      strcpy(Option->ConfigurationToken,
             pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.oldtoken);
    }

    return VZ_LOCALAPI_FAILED;
  }

  /*if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Remote_Profile_Sub)//the profile token is relay
  {
  	Option->H264.Resolution_num=1;
  	Option->H264.ResolutionsAvailable[0].Width=352;
  	Option->H264.ResolutionsAvailable[0].Height=288;
  	Option->H264.FrameRateRange.Min=1;
  	Option->H264.FrameRateRange.Max=25;
  	Option->H264.GovLengthRange.Min=1;
  	Option->H264.GovLengthRange.Max=400;
  	Option->H264.EncodingIntervalRange.Min=1;
  	Option->H264.EncodingIntervalRange.Max=400;
  	Option->H264.Profile=VZ__H264Profile__Extended;
  	Option->QualityRange.Min=0;
  	Option->QualityRange.Max=5;
  }else*/
  if(j==MAX_CHANNEL) {
    for(i=0; i<MAX_CHANNEL; i++) {
      if(sys_source_cfg[i].streamType==AVS_CAP_IPC)break;
    }

    if(i<MAX_CHANNEL) {
      Option->RelayFlag = 1;
      Option->channel = i;
      return VZ_LOCALAPI_FAILED;

    } else {
      j=0;
    }
  }

  if(j<MAX_CHANNEL) {
    unsigned int typevalue = 0;
    int localchannel=sys_source_cfg[j].userID;

    if(ControlSystemDataChannel(localchannel, MSG_GET_ENCODE_TYPE_SUPPORT,
                                &typevalue, sizeof(typevalue)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    unsigned int value = 0;

    if(ControlSystemDataChannel(localchannel, MSG_GET_VIDEO_SIZEXY_SUPPORT, &value,
                                sizeof(value)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    if(AVS_ENC_BIT_H264 & typevalue) {
      LocalSDK__GetStreamResolutions(Option->H264.ResolutionsAvailable,value,
                                     &Option->H264.Resolution_num);
      //if(Option->H264.Resolution_num)
      {
        Option->H264.FrameRateRange.Min=1;
        Option->H264.FrameRateRange.Max=25;
        Option->H264.GovLengthRange.Min=MIN_GOVLENGTH;
        Option->H264.GovLengthRange.Max=MAX_GOVLENGTH;
        Option->H264.EncodingIntervalRange.Min=1;
        Option->H264.EncodingIntervalRange.Max=400;
        Option->H264.BitRateRange.Min=128;
        Option->H264.BitRateRange.Max=2048;
        Option->H264.Profile=
          pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.H264.H264Profile;
      }
    }

    if(AVS_ENC_BIT_MPEG4 & typevalue) {
      LocalSDK__GetStreamResolutions(Option->MPEG4.ResolutionsAvailable,value,
                                     &Option->MPEG4.Resolution_num);

      if(Option->MPEG4.Resolution_num) {
        Option->MPEG4.FrameRateRange.Min=1;
        Option->MPEG4.FrameRateRange.Max=25;
        Option->MPEG4.GovLengthRange.Min=MIN_GOVLENGTH;
        Option->MPEG4.GovLengthRange.Max=MAX_GOVLENGTH;
        Option->MPEG4.EncodingIntervalRange.Min=1;
        Option->MPEG4.EncodingIntervalRange.Max=400;
        Option->MPEG4.Profile=vz__Mpeg4Profile__SP;
      }
    }

    if(AVS_ENC_BIT_JPEG & typevalue) {
      LocalSDK__GetStreamResolutions(Option->JPEG.ResolutionsAvailable,value,
                                     &Option->JPEG.Resolution_num);

      if(Option->JPEG.Resolution_num) {
        Option->JPEG.FrameRateRange.Min=0;
        Option->JPEG.FrameRateRange.Max=25;
        Option->JPEG.EncodingIntervalRange.Min=0;
        Option->JPEG.EncodingIntervalRange.Max=400;
      }
    }

    Option->QualityRange.Min =0;
    Option->QualityRange.Max =5;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetVideoEncoderConfigOptionsResponse(
  LocalSDK__SUbIPCVideoEncoderConfigOption *Option) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int index =0;
  int i=0;
  int j=0;
  int flag=0;

  for(j=0; j<MAX_CHANNEL; j++) {
    if(sys_source_cfg[j].streamType == AVS_CAP_IPC) {
      for(i=0; i<MAX_PROFILE_NUM; i++) {
        if(strlen(Option->ProfileToken)
           &&(strcmp(Option->ProfileToken,
                     pOnvifInfo->Profiles[j].meta_profile[i].token)==0)) {
          flag =1;
          index =i;
          break;

        } else if(strlen(Option->ConfigurationToken)
                  &&strcmp(Option->ConfigurationToken,
                           pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token)==0) {
          flag =1;
          index =i;
          break;
        }
      }
    }

    if(flag)
      break;
  }

  if(j!= MAX_CHANNEL && index == 1) {
    Option->Encoding =
      pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.Encoding;
    memcpy(&Option->Resolution,
           &pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.Resolution,
           sizeof(vz_onvif_VideoResolution));
    return VZ_LOCALAPI_SUCCESS;
  }

  return VZ_LOCALAPI_FAILED;
}
int LocalSDK__GetRemoteVideoSources(LocalSDK__VideoSources *Sources,
                                    int channel) {
  int j=0;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  for(j=0; j<MAX_CHANNEL; j++) {
    if((sys_source_cfg[j].streamType == AVS_CAP_IPC)
       &&(sys_source_cfg[j].userID == channel)) {
      break;
    }
  }

  if(j != MAX_CHANNEL) {
    Onvif_Info* pOnvifInfo = GetOnvifInfo();

    if(Sources->VideoSource_num < VZ_LIST_MAX_NUM
       && strlen(pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token)) {
      strcpy(Sources->VideoSource[Sources->VideoSource_num].VideoSourceToken,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token);
      strcpy(Sources->VideoSource[Sources->VideoSource_num].RelayVideoSourceToken,
             pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken_S);
      Sources->VideoSource_num++;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}
#ifdef DM3730 //6446 only alarm profile,not include the VideoSource info
int LocalSDK__GetVideoSources(LocalSDK__VideoSources *Sources) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VZ_StreamCodecConfig maincode;

  if(ControlSystemData(MSG_GET_MAIN_STREAMCODE, &maincode,
                       sizeof(VZ_StreamCodecConfig)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int nBrightness,nSaturation,nContrast,nHue;

  if((ControlSystemData(MSG_GET_BRIGHTNESS, &nBrightness, sizeof(int)) < 0)
     ||(ControlSystemData(MSG_GET_SATURATION, &nSaturation, sizeof(int)) < 0)
     ||(ControlSystemData(MSG_GET_CONTRAST, &nContrast, sizeof(int)) < 0)
     ||(ControlSystemData(MSG_GET_HUE, &nHue, sizeof(int)) < 0)) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int j=0;

  for(j=0; j<MAX_CHANNEL; j++) {
    //int index=sys_source_cfg[j].userID;
    if(sys_source_cfg[j].enable) {
      if(Sources->VideoSource_num < VZ_LIST_MAX_NUM
         &&pOnvifInfo->Profiles[j].meta_profile[0].Type == Local_Profile_Main) {
        strcpy(Sources->VideoSource[Sources->VideoSource_num].VideoSourceToken,
               pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token);
        //get main stream Imaging
        Sources->VideoSource[Sources->VideoSource_num].Imaging.Brightness=nBrightness;
        Sources->VideoSource[Sources->VideoSource_num].Imaging.ColorSaturation=
          nSaturation;
        Sources->VideoSource[Sources->VideoSource_num].Imaging.Contrast=nContrast;
        Sources->VideoSource[Sources->VideoSource_num].Imaging.hue=nHue;
        strcpy(Sources->VideoSource[Sources->VideoSource_num].Imaging.VideoSourceToken,
               pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token);
        //get main stream Framerate
        Sources->VideoSource[Sources->VideoSource_num].Framerate=maincode.nFrameRate;
        //get main stream Resolution
        Sources->VideoSource[Sources->VideoSource_num].Resolution.Width=
          maincode.nVideoSizeXY>>16&0XFFFF;
        Sources->VideoSource[Sources->VideoSource_num].Resolution.Height=
          maincode.nVideoSizeXY&0XFFFF;
        Sources->VideoSource_num++;
      }

      //get sub stream Imaging(now one channel only have one source,if main stream exist ,not check the sub stream)
      else if(Sources->VideoSource_num < VZ_LIST_MAX_NUM
              &&pOnvifInfo->Profiles[j].meta_profile[1].Type == Local_Profile_Sub) {
        Sources->VideoSource[Sources->VideoSource_num].Imaging.Brightness=nBrightness;
        Sources->VideoSource[Sources->VideoSource_num].Imaging.ColorSaturation=
          nSaturation;
        Sources->VideoSource[Sources->VideoSource_num].Imaging.Contrast=nContrast;
        Sources->VideoSource[Sources->VideoSource_num].Imaging.hue=nHue;
        strcpy(Sources->VideoSource[Sources->VideoSource_num].Imaging.VideoSourceToken,
               pOnvifInfo->Profiles[j].meta_profile[1].VideoSourceConfig.token);
        //get sub stream Framerate
        //Sources->VideoSource[Sources->VideoSource_num].Framerate=subcode.nFrameRate;
        //get sub stream Resolution
        //Sources->VideoSource[Sources->VideoSource_num].Resolution.Width=subcode.nVideoSizeXY>>16&0XFFFF;
        //Sources->VideoSource[Sources->VideoSource_num].Resolution.Height=subcode.nVideoSizeXY&0XFFFF;
        Sources->VideoSource_num++;
      }

    } else {
      if(strlen(pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token)) {
        strcpy(Sources->VideoSource[Sources->VideoSource_num].VideoSourceToken,
               pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token);
        Sources->VideoSource[Sources->VideoSource_num].Resolution.Width=
          pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.Bounds.width;
        Sources->VideoSource[Sources->VideoSource_num].Resolution.Height=
          pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.Bounds.height;
        Sources->VideoSource_num++;
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}
#endif

int LocalSDK__PTZMove(LocalSDK__ptz__ContinuousMove *move) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(move->ProfileToken,pOnvifInfo,&ChannelId,
                                        &index);
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    move->RelayFlag=1;
    strcpy(move->ProfileToken,"");
    return VZ_LOCALAPI_FAILED;

  } else if( sys_source_cfg[ChannelId].streamType ==
             AVS_CAP_IPC) { //the profile token is relay
    move->RelayFlag=1;
    move->channel=ChannelId;
    strcpy(move->ProfileToken,"");
    strcpy(move->ProfileToken,
           pOnvifInfo->Profiles[ChannelId].meta_profile[index].oldtoken);
    return VZ_LOCALAPI_FAILED;

  } else {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;
  }
}

int LocalSDK__PTZStopMove(LocalSDK__ptz__StopMove *move) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(move->ProfileToken,pOnvifInfo,&ChannelId,
                                        &index);

  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    move->RelayFlag=1;
    strcpy(move->ProfileToken,"");
    return VZ_LOCALAPI_FAILED;

  } else if( sys_source_cfg[ChannelId].streamType ==
             AVS_CAP_IPC) { //the profile token is relay
    move->RelayFlag=1;
    move->channel=ChannelId;
    strcpy(move->ProfileToken,"");
    strcpy(move->ProfileToken,
           pOnvifInfo->Profiles[ChannelId].meta_profile[index].oldtoken);
    return VZ_LOCALAPI_FAILED;

  } else {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_FAILED;
  }
}

int LocalSDK__GetRecordingList(LocalSDK__RecordingInfo *RecordingInfo) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}


void LocalSDK__SetRecordingTimeFilter(LocalSDK__RecordingTimeSearchFilter
                                      *TimeFilter) {
  strcpy(TimeFilter->TimeSearchToken,"TimeSearchFilter1");

  if(TimeFilter->RecordingsNum == 0) { //search all channel recordings
    Onvif_Info* pOnvifInfo = GetOnvifInfo();
    int i=0;

    for(i=0; i<MAX_CHANNEL; i++) {
      if(TimeFilter->RecordingsNum < VZ_LIST_MAX_NUM
         &&strlen(pOnvifInfo->Profiles[i].meta_profile[0].VideoRecordConfig.token)) {
        strcpy(TimeFilter->Recordings[TimeFilter->RecordingsNum],
               pOnvifInfo->Profiles[i].meta_profile[0].VideoRecordConfig.token);
        TimeFilter->RecordingsNum++;
      }
    }
  }

  memcpy(&gTimeSearchFilter,TimeFilter,
         sizeof(LocalSDK__RecordingTimeSearchFilter));
}

int LocalSDK__EndSearch(LocalSDK__EndSearch_Info *Info) {
  if(strcmp(Info->SearchToken,gTimeSearchFilter.TimeSearchToken)!=0) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  memset(&gTimeSearchFilter,0,sizeof(LocalSDK__RecordingTimeSearchFilter));
  time(&Info->Endpoint);

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetRecordingTimeList(LocalSDK__RecordingTimeSearchResults
                                   *Results) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}


static int Transform_DateStr_ToValue(char *dataformat, __u8 *value) {
  if(strcmp(dataformat,"YYYY/MM/DD")==0)
    *value = 0;

  else if (strcmp(dataformat,"MM/DD/YYYY")==0)
    *value = 1;

  else if (strcmp(dataformat,"DD/MM/YYYY")==0)
    *value = 2;

  else if (strcmp(dataformat,"YYYY/MM/DD;MM/DD/YYYY;DD/MM/YYYY")==0)
    *value = 'a';

  else {
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


static int SetOsd_VZLogo(int channel,int Position,__u8 enable,__u8 value) {
  if((ControlSystemDataLocalChannel(channel,SYS_MSG_SET_LOGO_POSITION, &Position,
                                    sizeof(Position)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_DETAIL_INFO,(void *)&value,
                                      sizeof(value)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_LOGO_ENABLE, &enable,
                                      sizeof(enable)) < 0)) {
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}
static int SetOSD_Date(int channel,int Position,__u8 enable,__u8 value) {
  if((ControlSystemDataLocalChannel(channel,SYS_MSG_SET_DATEFORMAT, &value,
                                    sizeof(value)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_DATEPPOS, &Position,
                                      sizeof(Position)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_DATE_STAMP_ENABLE, &enable,
                                      sizeof(enable)) < 0)) {
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}
static int SetOSD_Time(int channel,int Position,__u8 enable,__u8 value) {
  if((ControlSystemDataLocalChannel(channel,SYS_MSG_SET_TSTAMPFORMAT, &value,
                                    sizeof(value)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_TIMEPOS, &Position,
                                      sizeof(Position)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_TIME_STAMP_ENABLE, &enable,
                                      sizeof(enable)) < 0)) {
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}
static int SetOSD_Txt(int channel,int Position,__u8 enable,char *text) {
  if((ControlSystemDataLocalChannel(channel,SYS_MSG_SET_TEXT_POSITION, &Position,
                                    sizeof(Position)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_OVERLAY_TEXT,(void *)text,
                                      strlen(text)) < 0)
     ||(ControlSystemDataLocalChannel(channel,SYS_MSG_SET_TEXT_ENABLE, &enable,
                                      sizeof(enable)) < 0)) {
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}
int LocalSDK__SetOSD_Position(int channel,int Position,OSD_config *osd,
                              LocalSDK__OSDConfig *OSDConfig) {
  char *temp=NULL;
  temp=strrchr(OSDConfig->OSDToken,'_');

  if(temp) {
    if(strncmp(temp+1,"string",6)==0) {
      if((SetOSD_Txt(channel,Position,osd->nTextEnable,osd->overlaytext) < 0)
         ||(SetOSD_Time(channel,osd->timeposition,osd->tstampenable,
                        osd->tstampformat) < 0)
         ||(SetOSD_Date(channel,osd->dateposition,osd->dstampenable,osd->dateformat) < 0)
         ||(SetOsd_VZLogo(channel,osd->nLogoPosition,osd->nLogoEnable,
                          osd->nDetailInfo) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

    } else if(strncmp(temp+1,"date",4)==0) {

      if((SetOSD_Date(channel,Position,osd->dstampenable,osd->dateformat) < 0)
         ||(SetOSD_Time(channel,osd->timeposition,osd->tstampenable,
                        osd->tstampformat) < 0)
         ||(SetOSD_Txt(channel,osd->nTextPosition,osd->nTextEnable,osd->overlaytext) < 0)
         ||(SetOsd_VZLogo(channel,osd->nLogoPosition,osd->nLogoEnable,
                          osd->nDetailInfo) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

    } else if(strncmp(temp+1,"time",4)==0) {
      if((SetOSD_Time(channel,Position,osd->tstampenable,osd->tstampformat) < 0)
         ||(SetOSD_Date(channel,osd->dateposition,osd->dstampenable,osd->dateformat) < 0)
         ||(SetOSD_Txt(channel,osd->nTextPosition,osd->nTextEnable,osd->overlaytext) < 0)
         ||(SetOsd_VZLogo(channel,osd->nLogoPosition,osd->nLogoEnable,
                          osd->nDetailInfo) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

    } else if(strncmp(temp+1,"vzlogo",6)==0) {
      if((SetOsd_VZLogo(channel,Position,osd->nLogoEnable,osd->nDetailInfo) < 0)
         ||(SetOSD_Txt(channel,osd->nTextPosition,osd->nTextEnable,osd->overlaytext) < 0)
         ||(SetOSD_Time(channel,osd->timeposition,osd->tstampenable,
                        osd->tstampformat) < 0)
         ||(SetOSD_Date(channel,osd->dateposition,osd->dstampenable,
                        osd->dateformat) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

    } else {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      return VZ_LOCALAPI_FAILED;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__DeleteOSD(char * p_OSDToken,int *Channel) {
  //find Token
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i,j;
  int channel =0;
  int add_len = 0;
  VZ_ONVIF_OSDs value;
  memcpy(&value,&pOnvifInfo->OSDS,sizeof(VZ_ONVIF_OSDs));

  for(i=0; i<value.OSD_num; i++) {
    if(strcmp(p_OSDToken,value.OSD[i].OSDCfg.token) == 0) {
      strcpy(value.OSD[i].OSDCfg.token,"");
      strcpy(p_OSDToken,value.OSD[i].OSDCfg.relaytoken);
      channel = value.OSD[i].channel;
      break;
    }
  }

  if(i>=pOnvifInfo->OSDS.OSD_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  //delete OSD
  int last_empty_str_index=value.OSD_num;

  //排序
  for(j=0; j<value.OSD_num; j++) {
    if(strlen(value.OSD[j].OSDCfg.token)!=0) {
      if(last_empty_str_index <j) {
        memcpy(&(value.OSD[last_empty_str_index]),&(value.OSD[j]),sizeof(VZ_ONVIF_OSD));
        memset(&(value.OSD[j]),0,sizeof(VZ_ONVIF_OSD));
        last_empty_str_index++;
      }

      add_len++;

    } else {
      if(last_empty_str_index >j)
        last_empty_str_index = j;

      memset(&(value.OSD[j]),0,sizeof(VZ_ONVIF_OSD));
    }
  }

  //长度修正
  value.OSD_num = add_len;

  if(DoOnvifSetOSD(&value) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[channel].streamType == AVS_CAP_IPC) { //relay
    *Channel = channel;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__CreateOSD(VZ_ONVIF_OSD *OSD) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  if(pOnvifInfo->OSDS.OSD_num>= MAX_OSD_NUM) {
    gLocalErr = VZ_LOCALAPI_OSDS_EXCEED;
    return VZ_LOCALAPI_FAILED;
  }

  int i = 0;
  int exitflag = 0;
  int numbefore = pOnvifInfo->OSDS.OSD_num;

  for(i = 0; i<numbefore; i++) {
    if(pOnvifInfo->OSDS.OSD[i].used
       &&(pOnvifInfo->OSDS.OSD[i].channel == OSD->channel)
       &&(strcmp(pOnvifInfo->OSDS.OSD[i].OSDCfg.token,OSD->OSDCfg.token)==0)) {
      exitflag = 1;
      break;
    }
  }

  if(exitflag == 0) {
    VZ_ONVIF_OSDs Value;
    memcpy(&Value,&pOnvifInfo->OSDS,sizeof(Value));
    OSD->used = 1;
    memcpy(&Value.OSD[Value.OSD_num],OSD,sizeof(VZ_ONVIF_OSD));
    Value.OSD_num++;

    if(DoOnvifSetOSD(&Value) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  /*int ret = LocalSDK__SetOSD_display(OSD->channel,OSD->OSDCfg);
  if(ret == VZ_LOCALAPI_FAILED)
  	return ret;
  */
  return VZ_LOCALAPI_SUCCESS;
}



int LocalSDK__AddVideoAnalyticsConfiguration(LocalSDK__AddVideoAnalyticsConfig
                                             *Config) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_SetVideoAnalyticsConfig Value;
  memset(&Value,0x0,sizeof(vz_onvif_SetVideoAnalyticsConfig));
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(Config->ProfileToken,pOnvifInfo,
                                        &ChannelId,&index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return LocalSDK__AddVideoAnalytics2Profile(pOnvifInfo,
                                             Config->ConfigurationToken,ChannelId,index);
}

int LocalSDK__RemoveVideoAnalyticsConfiguration(char *ProfileToken) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_SetVideoAnalyticsConfig Value;
  memset(&Value,0x0,sizeof(vz_onvif_SetVideoAnalyticsConfig));
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(ProfileToken,pOnvifInfo,&ChannelId,
                                        &index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  ret=LocalSDK__DelVideoAnalyticsFromProfile(pOnvifInfo,ChannelId,index);
  return ret;
}

int LocalSDK__SetVideoAnalyticsConfiguration(LocalSDK__SetVideoAnalyticsConfig
                                             *Config) {
  int j=0;
  int index = 0;
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_VideoAnalytics Value;
  memset(&Value,0x0,sizeof(vz_onvif_VideoAnalytics));

  for(j=0; j<MAX_VIDEO_ANALYTICS_NUM; j++) {
    if(strcmp(Config->cfg.VACfg.token,
              pOnvifInfo->VideoAnalytics[j].VACfg.token)==0) {
      memcpy(&Value,&pOnvifInfo->VideoAnalytics[j],sizeof(vz_onvif_VideoAnalytics));
      index=j;
      break;
    }
  }

  if(j==MAX_VIDEO_ANALYTICS_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(&Value.VACfg.AnalyticsEngineCfg,&Config->cfg.VACfg.AnalyticsEngineCfg,
         sizeof(vz_onvif_AnalyticsEngineCfg));
  memcpy(&Value.VACfg.RuleEngineCfg,&Config->cfg.VACfg.RuleEngineCfg,
         sizeof(vz_onvif_RuleEngineCfg));

  if(DoOnvifSetVideoAnalytics(&Value,index) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}
int LocalSDK__GetVideoSourceConfigurationOptions(
  LocalSDK__VideoSourceConfigurationOption *Option) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  int j=0;
  int flag=0;

  for(j=0; j<MAX_CHANNEL; j++) {
    for(i=0; i<MAX_PROFILE_NUM; i++) {
      if(strlen(Option->ProfileToken)) {
        if(strlen(Option->ConfigurationToken)
           &&(strcmp(Option->ProfileToken,
                     pOnvifInfo->Profiles[j].meta_profile[i].token)==0)
           &&(strcmp(Option->ConfigurationToken,
                     pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.token)==0)) {
          flag =1;
          break;

        } else if(strcmp(Option->ProfileToken,
                         pOnvifInfo->Profiles[j].meta_profile[i].token)==0) {
          flag =1;
          break;
        }

      } else {
        if(strlen(Option->ConfigurationToken)
           &&strcmp(Option->ConfigurationToken,
                    pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.token)==0) {
          flag =1;
          break;
        }
      }
    }

    if(flag)
      break;
  }

  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(j==MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    Option->RelayFlag=1;
    strcpy(Option->ProfileToken,"");
    strcpy(Option->ConfigurationToken,"");
    return VZ_LOCALAPI_FAILED;

  } else if((sys_source_cfg[j].streamType == AVS_CAP_IPC)
            &&(i< MAX_PROFILE_NUM)) { //the profile token is relay
    Option->RelayFlag=1;
    Option->channel = j;

    if(Option->ProfileToken) {
      strcpy(Option->ProfileToken,"");

      if(i == MAX_PROFILE_NUM-1) { //alarm profile
        strcpy(Option->ProfileToken,pOnvifInfo->Profiles[j].meta_profile[0].oldtoken);

      } else
        strcpy(Option->ProfileToken,pOnvifInfo->Profiles[j].meta_profile[i].oldtoken);
    }

    if(Option->ConfigurationToken) {
      int channel = sys_source_cfg[j].userID;
      strcpy(Option->ConfigurationToken,"");
      strcpy(Option->ConfigurationToken,
             pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken);
    }

    return VZ_LOCALAPI_FAILED;
  }

#ifdef NEW_ONVIF
  Option->XRange.Max =pOnvifInfo->VSCfgOptions.BoundsRange.XRange.Max;
  Option->XRange.Min=pOnvifInfo->VSCfgOptions.BoundsRange.XRange.Min;
  Option->YRange.Max =pOnvifInfo->VSCfgOptions.BoundsRange.YRange.Max;
  Option->YRange.Min=pOnvifInfo->VSCfgOptions.BoundsRange.YRange.Min;
  Option->WidthRange.Min=pOnvifInfo->VSCfgOptions.BoundsRange.WidthRange.Min;
  Option->WidthRange.Max =pOnvifInfo->VSCfgOptions.BoundsRange.WidthRange.Max;
  Option->HeightRange.Min=pOnvifInfo->VSCfgOptions.BoundsRange.HeightRange.Min;
  Option->HeightRange.Max =pOnvifInfo->VSCfgOptions.BoundsRange.HeightRange.Max;
#else
  Option->XRange.Max
    =pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.x;
  Option->XRange.Min=
    pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.x;
  Option->YRange.Max
    =pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.y;
  Option->YRange.Min=
    pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.y;
  Option->WidthRange.Min=
    pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.width;
  Option->WidthRange.Max
    =pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.width;
  Option->HeightRange.Min=
    pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.height;
  Option->HeightRange.Max
    =pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Bounds.height;
#endif
  strcpy(Option->AvailableSourceToken[0],
         pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.token);

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetProfileResponse(LocalSDK__Profiles *meta_profiles,
                                 int remote_channel) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  int j=0;
  int flag=0;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  //Profile_Type Type = Profile_Type_Max;
  for(j=0; j<MAX_CHANNEL; j++) {
    if(sys_source_cfg[j].userID == remote_channel) {
      for(i=0; i<MAX_PROFILE_NUM; i++) {
        if(strlen(pOnvifInfo->Profiles[j].meta_profile[i].oldtoken)
           &&(strcmp(meta_profiles->Profile[0].token,
                     pOnvifInfo->Profiles[j].meta_profile[i].oldtoken)==0)) {
          flag=1;
          break;
        }
      }
    }

    if(flag)
      break;
  }

  if(flag) {
    strcpy(meta_profiles->Profile[0].token,
           pOnvifInfo->Profiles[j].meta_profile[i].token);
    strcpy(meta_profiles->Profile[0].Name,
           pOnvifInfo->Profiles[j].meta_profile[i].Name);
    strcpy(meta_profiles->Profile[0].VideoSourceConfig.Name,
           pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.Name);
    strcpy(meta_profiles->Profile[0].VideoSourceConfig.token,
           pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.token);
    strcpy(meta_profiles->Profile[0].VideoEncoderConfig.Name,
           pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.Name);
    strcpy(meta_profiles->Profile[0].VideoEncoderConfig.token,
           pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token);
    meta_profiles->Profile[0].VA_ID=pOnvifInfo->Profiles[j].meta_profile[i].VA_ID;
    //strcpy(meta_profiles->Profile[0].VideoAnalyticsConfig.Name,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.Name);
    //strcpy(meta_profiles->Profile[0].VideoAnalyticsConfig.token,pOnvifInfo->Profiles[j].meta_profile[i].VideoAnalyticsConfig.token);
    strcpy(meta_profiles->Profile[0].AudioSourceConfiguration.Name,
           pOnvifInfo->Profiles[j].meta_profile[i].AudioSourceConfiguration.Name);
    strcpy(meta_profiles->Profile[0].AudioSourceConfiguration.token,
           pOnvifInfo->Profiles[j].meta_profile[i].AudioSourceConfiguration.token);
    strcpy(meta_profiles->Profile[0].AudioEncoderConfiguration.Name,
           pOnvifInfo->Profiles[j].meta_profile[i].AudioEncoderConfiguration.Name);
    strcpy(meta_profiles->Profile[0].AudioEncoderConfiguration.token,
           pOnvifInfo->Profiles[j].meta_profile[i].AudioEncoderConfiguration.token);

    meta_profiles->Profile[0].VideoSourceConfig.UserCount=
      pOnvifInfo->Profiles[j].meta_profile[i].VideoSourceConfig.UserCount;
    return VZ_LOCALAPI_SUCCESS;
  }

  return VZ_LOCALAPI_FAILED;
}


int LocalSDK__GetRemoteChannel(char *ProfileToken,
                               LocalSDK__RemoteChannel_Info *Info) {
  int i=0;
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int l=0;
  int flag=0;

  for(l=0; l<MAX_CHANNEL; l++) {
    for(i=0; i<MAX_PROFILE_NUM; i++) {
      if(ProfileToken
         && (strcmp(ProfileToken,pOnvifInfo->Profiles[l].meta_profile[i].token)==0)
         && (sys_source_cfg[l].streamType == AVS_CAP_IPC)) {
        //Info->remote_channel = sys_source_cfg[l].userID;
        Info->channel= l;
        strcpy(Info->Token,pOnvifInfo->Profiles[l].meta_profile[i].oldtoken);
        flag =1;
        break;
      }
    }

    if(flag)
      break;
  }

  if(l == MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__RemoveVideoSourceConfiguration(char *ProfileToken) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(ProfileToken,pOnvifInfo,&ChannelId,
                                        &index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;

  } else {
    vz_onvif_Profiles value;
    memcpy(&value,&pOnvifInfo->Profiles[ChannelId],sizeof(vz_onvif_Profiles));
    strcpy(value.meta_profile[index].oldtoken,"");
    value.meta_profile[index].Type = Profile_Type_Max;
    memset(&value.meta_profile[index].VideoSourceConfig,0x0,
           sizeof(vz_onvif_VideoSourceConfig));

    if(DoOnvifSetProfiles(&value,ChannelId) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    VideoSourceCfg delsourcevalue;
    memset(&delsourcevalue,0x0,sizeof(delsourcevalue));

    if(ControlSystemDataALLChannel(ChannelId,SYS_MSG_SET_ONVIF_VIDEOSOURCECFG,
                                   &delsourcevalue, sizeof(delsourcevalue))< 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    return VZ_LOCALAPI_SUCCESS;
  }
}

int LocalSDK__GetVideoAnalyticsConfiguration(vz_onvif_VideoAnalytics
                                             *VideoAnalyticsCfg) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<MAX_VIDEO_ANALYTICS_NUM; i++) {
    if(strlen(VideoAnalyticsCfg->VACfg.token)
       && (strcmp(VideoAnalyticsCfg->VACfg.token,
                  pOnvifInfo->VideoAnalytics[i].VACfg.token)==0)) {
      break;
    }
  }

  if(i==MAX_VIDEO_ANALYTICS_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;

  } else {
    memcpy(VideoAnalyticsCfg,&pOnvifInfo->VideoAnalytics[i],
           sizeof(vz_onvif_VideoAnalytics));
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__CheckVideoSourcecfg(vz_onvif_VideoSourceRectangle *p_Bounds,
                                  vz_onvif_IntRectangleRange *BoundsRange) {
#ifdef NEW_ONVIF

  if(p_Bounds->x < BoundsRange->XRange.Min ||
     p_Bounds->x > BoundsRange->XRange.Max ||
     p_Bounds->y < BoundsRange->YRange.Min ||
     p_Bounds->y > BoundsRange->YRange.Max ||
     p_Bounds->width < BoundsRange->WidthRange.Min ||
     p_Bounds->width > BoundsRange->WidthRange.Max ||
     p_Bounds->height < BoundsRange->HeightRange.Min ||
     p_Bounds->height > BoundsRange->HeightRange.Max) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

#endif
  return VZ_LOCALAPI_SUCCESS;
}




int LocalSDK__SetVideoSource(LocalSDK__GetVideoSourceConfig
                             *VideoSourceConfig) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret= LocalSDK__LookUp_VideoSourceToken(VideoSourceConfig->Token,pOnvifInfo,
                                             &ChannelId,&index);
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(ret == VZ_LOCALAPI_FAILED) {
    strcpy(VideoSourceConfig->Token,"");
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;

  } else if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
    int channel =sys_source_cfg[ChannelId].userID;
    vz_onvif_IntRectangleRange *p_Range =
      &pOnvifInfo->remote_server[channel].video.RelayVSCfgOptions;

    if(LocalSDK__CheckVideoSourcecfg(&VideoSourceConfig->Config.Bounds,
                                     p_Range) == VZ_LOCALAPI_SUCCESS) {
      VideoSourceConfig->channel = ChannelId;
      strcpy(VideoSourceConfig->Token,
             pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken);
      strcpy(VideoSourceConfig->Config.token,
             pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken_S);

    } else {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      strcpy(VideoSourceConfig->Token,"");
    }

  } else {

#ifdef NEW_ONVIF

    if(LocalSDK__CheckVideoSourcecfg(&VideoSourceConfig->Config.Bounds,
                                     &pOnvifInfo->VSCfgOptions.BoundsRange) == VZ_LOCALAPI_FAILED)
#else
    if(VideoSourceConfig->Config.Bounds.width > MAX_VIDEOSOURCE_WITH
       ||VideoSourceConfig->Config.Bounds.width <= 0
       || VideoSourceConfig->Config.Bounds.height > MAX_VIDEOSOURCE_HEIGHT
       ||VideoSourceConfig->Config.Bounds.height <= 0)
#endif
    {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      strcpy(VideoSourceConfig->Token,"");
      return VZ_LOCALAPI_FAILED;
    }

    vz_onvif_Profiles value;
    memcpy(&value,&pOnvifInfo->Profiles[ChannelId],sizeof(vz_onvif_Profiles));
    memcpy(&value.meta_profile[index].VideoSourceConfig.Bounds,
           &VideoSourceConfig->Config.Bounds,sizeof(vz_onvif_VideoSourceRectangle));

    if(DoOnvifSetProfiles(&value,ChannelId) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    return VZ_LOCALAPI_SUCCESS;
  }

  return VZ_LOCALAPI_FAILED;
}


int LocalSDK__GetSpecifiedVideoSource(LocalSDK__GetVideoSourceConfig
                                      *VideoSourceConfig) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret= LocalSDK__LookUp_VideoSourceToken(VideoSourceConfig->Token,pOnvifInfo,
                                             &ChannelId,&index);
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int channel=0;

  if(ret == VZ_LOCALAPI_FAILED) {
    strcpy(VideoSourceConfig->Token,"");
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;

  } else if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
    VideoSourceConfig->channel = ChannelId;
    channel = sys_source_cfg[ChannelId].userID;
    strcpy(VideoSourceConfig->Token,
           pOnvifInfo->remote_server[channel].video.VideoSourceRelayToken);
    return VZ_LOCALAPI_FAILED;

  } else {
    LocalSDK__GetSourceBoundsConfig(&VideoSourceConfig->Config);
    strcpy(VideoSourceConfig->Config.Name,
           pOnvifInfo->Profiles[ChannelId].meta_profile[0].VideoSourceConfig.Name);
    strcpy(VideoSourceConfig->Config.token,
           pOnvifInfo->Profiles[ChannelId].meta_profile[0].VideoSourceConfig.token);
    channel = sys_source_cfg[ChannelId].userID;
    VideoSourceConfig->Config.UserCount=
      pOnvifInfo->Profiles[ChannelId].meta_profile[0].VideoSourceConfig.UserCount;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetLocalVideoSourceConfigs_OneChannel(LocalSDK__VideoSourceConfigs
                                                    *SourceConfigs,int Channel) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int j=Channel;

  if(Channel<0 || Channel>MAX_CHANNEL)
    return VZ_LOCALAPI_FAILED;

  if(sys_source_cfg[Channel].streamType == AVS_CAP_V4L2) {
    //int channel = sys_source_cfg[j].userID;
    if(SourceConfigs->VideoSource_num < VZ_LIST_MAX_NUM
       && strlen(pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token)) {
      LocalSDK__GetSourceBoundsConfig(
        &SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num]);
      strcpy(SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].Name,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.Name);
      strcpy(SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].token,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token);
      SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].UserCount=
        pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.UserCount;
      SourceConfigs->VideoSource_num++;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetLocalVideoSourceConfigs(LocalSDK__VideoSourceConfigs
                                         *SourceConfigs) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  int j=0;

  for(j=0; j<MAX_CHANNEL; j++) {
    if(sys_source_cfg[j].streamType == AVS_CAP_V4L2) {
      //int channel = sys_source_cfg[j].userID;
      if(SourceConfigs->VideoSource_num < VZ_LIST_MAX_NUM
         && strlen(pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token)) {
        LocalSDK__GetSourceBoundsConfig(
          &SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num]);
        strcpy(SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].Name,
               pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.Name);
        strcpy(SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].token,
               pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token);
        SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].UserCount=
          pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.UserCount;
        SourceConfigs->VideoSource_num++;
      }
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetRemoteVideoSourceConfigs(LocalSDK__VideoSourceConfigs
                                          *SourceConfigs,int channel) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(channel < 0 || channel > MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[channel].streamType == AVS_CAP_IPC) {
    Onvif_Info* pOnvifInfo = GetOnvifInfo();
    int index = sys_source_cfg[channel].userID;

    if(SourceConfigs->VideoSource_num < VZ_LIST_MAX_NUM
       && strlen(
         pOnvifInfo->Profiles[channel].meta_profile[0].VideoSourceConfig.token)) {
      strcpy(SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].oldtoken,
             pOnvifInfo->remote_server[index].video.VideoSourceRelayToken);
      strcpy(SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].Name,
             pOnvifInfo->Profiles[channel].meta_profile[0].VideoSourceConfig.Name);
      strcpy(SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].token,
             pOnvifInfo->Profiles[channel].meta_profile[0].VideoSourceConfig.token);
      SourceConfigs->VideoSourceConfig[SourceConfigs->VideoSource_num].UserCount=
        pOnvifInfo->Profiles[channel].meta_profile[0].VideoSourceConfig.UserCount;
      SourceConfigs->VideoSource_num++;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetRemoteVideoEncoderConfigs(LocalSDK__VideoEncoderConfigs
                                           *Configs,int channel) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(channel < 0 || channel > MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[channel].streamType == AVS_CAP_IPC) {
    Onvif_Info* pOnvifInfo = GetOnvifInfo();
    int i=0;

    for(i=0; i<MAX_PROFILE_NUM; i++) {
      if(Configs->VideoEncoder_num < VZ_LIST_MAX_NUM
         && strlen(
           pOnvifInfo->Profiles[channel].meta_profile[i].VideoEncoderConfig.token)) {
        strcpy(Configs->VideoEncoderConfig[Configs->VideoEncoder_num].oldtoken,
               pOnvifInfo->Profiles[channel].meta_profile[i].VideoEncoderConfig.oldtoken);
        strcpy(Configs->VideoEncoderConfig[Configs->VideoEncoder_num].Name,
               pOnvifInfo->Profiles[channel].meta_profile[i].VideoEncoderConfig.Name);
        strcpy(Configs->VideoEncoderConfig[Configs->VideoEncoder_num].token,
               pOnvifInfo->Profiles[channel].meta_profile[i].VideoEncoderConfig.token);
        //if(pOnvifInfo->Profiles[j].meta_profile[i].Type == Remote_Profile_Sub){
        //	LocalSDK__Get_IPC2_StreamEncoderConfig(&Configs->VideoEncoderConfig[Configs->VideoEncoder_num]);
        //}
        Configs->VideoEncoder_num++;
      }
    }

  } else {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetGuaranteedNumberOfVideoEncoderInstances(char *VideoSourceToken,
                                                         int *Number) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret= LocalSDK__LookUp_VideoSourceToken(VideoSourceToken,pOnvifInfo,
                                             &ChannelId,&index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  *Number=pOnvifInfo->Profiles[ChannelId].meta_profile[index].VideoSourceConfig.UserCount
          -1;
  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetSnapshotUri(LocalSDK__SnapshotUri *SnapshotUri) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int http_port;

  if(ControlSystemData(MSG_GET_HTTPPORT, &http_port,sizeof(int)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(SnapshotUri->ProfileToken,pOnvifInfo,
                                        &ChannelId,&index);

  if(ret == VZ_LOCALAPI_FAILED) {
    strcpy(SnapshotUri->ProfileToken,"");
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  /*else if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC){
  	strcpy(SnapshotUri->ProfileToken,pOnvifInfo->Profiles[ChannelId].meta_profile[index].oldtoken);
  	SnapshotUri->channel = ChannelId;
  	return VZ_LOCALAPI_FAILED;
  }*/
  else {
    SnapshotUri->channel = ChannelId;
    struct in_addr ip;
    const char*     nicname = ETH0_NAME;
#ifdef WIFI_LAN
    __u8 netlinkstate = 0;

    if((OnvifDisp_SendRequestSync(SYS_SRV_GET_NETLINK_STATE, NULL, 0,&netlinkstate,
                                  sizeof(netlinkstate))>0)
       &&(netlinkstate == 0)) { //netlink UNCONNECT
      nicname = WIFI_NAME;
    }

#endif

    //ip.s_addr = net_get_ifaddr(nicname);
    if (ControlSystemData(SYS_MSG_GET_IP, &ip, sizeof(struct in_addr)) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    sprintf(SnapshotUri->Uri,"http://%s:%d/onvif/snapshot/%s",inet_ntoa(ip),
            http_port,SnapshotUri->ProfileToken);
  }

  return VZ_LOCALAPI_SUCCESS;
}

static void iGetSnapshotCallBack(const char *path,int path_size,
                                 const char *data,int data_size,void* user_data) {
  fprintf(stderr,"ImageCallBack (%s:%d)\n", path,data_size);

  if((data_size <= 0)||(data == NULL)||(user_data == NULL)) {
    return ;
  }

  GetSnapShotInfo *puserdata = (GetSnapShotInfo *)user_data;

  if (data_size > puserdata->bufsize) {
    fprintf(stderr,"date is big than the buffer size (%d:%d)\n", data_size,
            puserdata->bufsize);
    return;
  }

  memcpy(puserdata->pdata, data, data_size);
  puserdata->datasize = data_size;
  return ;
}

int LocalSDK__GetSnapshotData(char * buff, int * rlen, char * ProfileToken) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int ChannelId=-1;
  int index=0;
  int result = VZ_LOCALAPI_FAILED;
  int ret=LocalSDK__LookUp_ProfileToken(ProfileToken,pOnvifInfo,&ChannelId,
                                        &index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;

  } else if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;

  } else {
    char path[512]= {0};

    if(OnvifDisp_SendRequestSync(AVS_GET_SNAP, NULL, 0,path, sizeof(path))>0) {
      GetSnapShotInfo info;
      info.bufsize = *rlen;
      info.pdata = buff;
      info.datasize = 0;

      if(CACHED_RET_FAILURE == Cached_GetFile(path, strlen(path),iGetSnapshotCallBack,
                                          (void *)&info)) {
        fprintf(stderr,"get Snappic error\n");
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;

      } else if(info.datasize == 0) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;

      } else {
        result = VZ_LOCALAPI_SUCCESS;
      }

      *rlen = info.datasize;

    } else {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    }
  }

  return result;
}



int LocalSDK__GetVideoSourceConfigResponse(LocalSDK__VideoSourceConfigs
                                           *SourceConfigs,int remote_channel) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int j=0;
  int k=0;
  int flag=0;
  int ret = VZ_LOCALAPI_FAILED;

  for(k=0; k<SourceConfigs->VideoSource_num; k++) {
    flag=0;

    for(j=0; j<MAX_CHANNEL; j++) {
      if(sys_source_cfg[j].streamType == AVS_CAP_IPC
         && sys_source_cfg[j].userID == remote_channel) {
        int index = sys_source_cfg[j].userID;

        if(strlen(SourceConfigs->VideoSourceConfig[k].oldtoken)
           &&((strcmp(SourceConfigs->VideoSourceConfig[k].oldtoken,
                      pOnvifInfo->remote_server[index].video.VideoSourceRelayToken)==0)
              || (strcmp(SourceConfigs->VideoSourceConfig[k].oldtoken,
                         pOnvifInfo->remote_server[index].video.VideoSourceRelayToken_S)==0))) {
          flag=1;
          break;
        }
      }
    }

    if(flag) {
      strcpy(SourceConfigs->VideoSourceConfig[k].Name,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.Name);
      strcpy(SourceConfigs->VideoSourceConfig[k].token,
             pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.token);
      SourceConfigs->VideoSourceConfig[k].UserCount=
        pOnvifInfo->Profiles[j].meta_profile[0].VideoSourceConfig.UserCount;
      ret = VZ_LOCALAPI_SUCCESS;
    }
  }

  return ret;
}

int LocalSDK__GetVideoEncoderConfigResponse(LocalSDK__VideoEncoderConfigs
                                            *Configs,int remote_channel) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  int j=0;
  int k=0;
  int flag=0;
  int ret = VZ_LOCALAPI_FAILED;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  for(k=0; k<Configs->VideoEncoder_num; k++) {
    flag=0;

    for(j=0; j<MAX_CHANNEL; j++) {
      if(sys_source_cfg[j].userID == remote_channel) {
        for(i=0; i<MAX_PROFILE_NUM; i++) {
          if(strlen(Configs->VideoEncoderConfig[k].oldtoken)
             &&(strcmp(Configs->VideoEncoderConfig[k].oldtoken,
                       pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.oldtoken)==0)) {
            flag=1;
            break;
          }
        }
      }

      if(flag)
        break;
    }

    if(flag) {
      strcpy(Configs->VideoEncoderConfig[k].Name,
             pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.Name);
      strcpy(Configs->VideoEncoderConfig[k].token,
             pOnvifInfo->Profiles[j].meta_profile[i].VideoEncoderConfig.token);
      ret= VZ_LOCALAPI_SUCCESS;
    }
  }

  return ret;
}

int LocalSDK__GetChannelNum(int *ChannelNum) {
  __u8 channelnum;

  if(ControlSystemData(SYS_MSG_GET_CHANNEL_NUM,&channelnum,
                       sizeof(channelnum))<0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  *ChannelNum = channelnum;
  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetReceiver(vz_onvif_Receiver *Response) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<pOnvifInfo->Receivers.Receiver_num; i++) {
    if(strcmp(Response->Token,pOnvifInfo->Receivers.Receiver[i].Token)==0) {
      break;
    }
  }

  if(i== pOnvifInfo->Receivers.Receiver_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(&Response->Config,&pOnvifInfo->Receivers.Receiver[i].Config,
         sizeof(vz_onvif__ReceiverConfig));

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__CreateReceiver(vz_onvif_Receiver *Receiver) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  char strName[VZ_TOKEN_STR_LEN]= {0};
  int i=0;
  int j=0;
  int exist =0;

  if(MAX_RECEIVER_NUM==pOnvifInfo->Receivers.Receiver_num) {
    gLocalErr = VZ_LOCALAPI_RECEIVER_EXCEED;
    return VZ_LOCALAPI_FAILED;
  }

  for(i=0; i<pOnvifInfo->Receivers.Receiver_num; i++) {
    if(Receiver->channel == pOnvifInfo->Receivers.Receiver[i].channel) {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      return VZ_LOCALAPI_FAILED;
    }
  }

  int index= pOnvifInfo->Receivers.Receiver_num;
  vz_onvif_Receivers value;
  memcpy(&value,&pOnvifInfo->Receivers,sizeof(value));

  //find a available Receiver token
  do {
    exist =0;
    sprintf(strName, "Receiver_%d",j);

    for(i=0; i<pOnvifInfo->Receivers.Receiver_num; i++) {
      if(strcmp(strName,pOnvifInfo->Receivers.Receiver[i].Token)==0) {
        exist =1;
        j++;
        break;
      }
    }
  } while(exist);

  strcpy(value.Receiver[index].Token,strName);
  value.Receiver[index].channel = Receiver->channel;

  value.Receiver[index].state = vz_ReceiverState__Connected;
  memcpy(&value.Receiver[index].Config,&Receiver->Config,
         sizeof(vz_onvif__ReceiverConfig));
  value.Receiver_num++;

  if(DoOnvifSetReceiver(&value) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  strcpy(Receiver->Token,strName);
  return VZ_LOCALAPI_SUCCESS;

}

int LocalSDK__DeleteReceiver(const char *Token) {
  if(Token ==NULL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  vz_onvif_Receivers Receivers;
  memcpy(&Receivers,&pOnvifInfo->Receivers,sizeof(vz_onvif_Receivers));

  for(i=0; i<Receivers.Receiver_num; i++) {
    if(strcmp(Token,Receivers.Receiver[i].Token)==0) {
      strcpy(Receivers.Receiver[i].Token,"");
      break;
    }
  }

  if(i== Receivers.Receiver_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  //排序
  int last_empty_str_index=Receivers.Receiver_num;
  int j=0;
  int add_len = 0;

  for(j=0; j<Receivers.Receiver_num; j++) {
    if(strlen(Receivers.Receiver[j].Token)!=0) {
      if(last_empty_str_index <j) {
        memcpy(&(Receivers.Receiver[last_empty_str_index]),&(Receivers.Receiver[j]),
               sizeof(vz_onvif_Receiver));
        memset(&(Receivers.Receiver[j]),0x0,sizeof(vz_onvif_Receiver));
        last_empty_str_index++;
      }

      add_len++;

    } else {
      if(last_empty_str_index >j)
        last_empty_str_index = j;

      memset(&(Receivers.Receiver[j]),0x0,sizeof(vz_onvif_Receiver));
    }
  }

  //长度修正
  Receivers.Receiver_num = add_len;

  if(DoOnvifSetReceiver(&Receivers) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__ModifyReceiver(vz_onvif_Receiver *Receiver) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  int index =-1;
  vz_onvif_Receivers value;
  memcpy(&value,&pOnvifInfo->Receivers,sizeof(vz_onvif_Receivers));

  for(i=0; i<value.Receiver_num; i++) {
    if(strcmp(Receiver->Token,value.Receiver[i].Token)==0) {
      if(Receiver->channel!= value.Receiver[i].channel) {
        index =i;
      }

      break;
    }
  }

  if(index >= 0) { //need to check the channel is used by other receivers
    for(i=0; i<value.Receiver_num; i++) {
      if(Receiver->channel==value.Receiver[i].channel && i != index) {
        gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
        return VZ_LOCALAPI_FAILED;
      }
    }
  }

  if(i== value.Receiver_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(&value.Receiver[i].Config,&Receiver->Config,
         sizeof(vz_onvif__ReceiverConfig));

  if(DoOnvifSetReceiver(&value) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;

}

int LocalSDK__SetReceiverMode(const char *Token,int mode) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  vz_onvif_Receivers value;
  memcpy(&value,&pOnvifInfo->Receivers,sizeof(vz_onvif_Receivers));

  for(i=0; i<value.Receiver_num; i++) {
    if(strcmp(Token,value.Receiver[i].Token)==0) {
      break;
    }
  }

  if(i== value.Receiver_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  value.Receiver[i].Config.Mode=mode;

  if(DoOnvifSetReceiver(&value) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;

}

int LocalSDK__GetReceiverState(const char *Token,int *State) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<pOnvifInfo->Receivers.Receiver_num; i++) {
    if(strcmp(Token,pOnvifInfo->Receivers.Receiver[i].Token)==0) {
      break;
    }
  }

  if(i== pOnvifInfo->Receivers.Receiver_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  *State = pOnvifInfo->Receivers.Receiver[i].state;

  return VZ_LOCALAPI_SUCCESS;

}

int LocalSDK__GetReplayUri(const char* RecordingToken, char *ReplayUri) {
  if(!RecordingToken || !ReplayUri) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  int i=0;
  int channel=0;
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  //find RecordingToken
  for(i=0; i<MAX_CHANNEL; i++) {
    if(strcmp(RecordingToken,
              pOnvifInfo->Profiles[i].meta_profile[0].VideoRecordConfig.token)==0) {
      channel=i;
      break;
    }
  }

  if(i==MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  struct in_addr ip;

  const char*     nicname = ETH0_NAME;

#ifdef WIFI_LAN
  __u8 netlinkstate = 0;

  if((OnvifDisp_SendRequestSync(SYS_SRV_GET_NETLINK_STATE, NULL, 0,&netlinkstate,
                                sizeof(netlinkstate))>0)
     &&(netlinkstate == 0)) { //netlink UNCONNECT
    nicname = WIFI_NAME;
  }

#endif

  //ip.s_addr = net_get_ifaddr(nicname);
  if (ControlSystemData(SYS_MSG_GET_IP, &ip, sizeof(struct in_addr)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  char ip_new[20];
  strcpy(ip_new,inet_ntoa(ip));
  sprintf(ReplayUri,"rtsp://VisionZenith:147258369@%s:8551/channel_%d/",ip_new,
          channel);

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetRemoteAudioSourcesConfig_OneChannel(
  LocalSDK__AudioSourceConfigs *SourceConfigs,int channel) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(channel < 0 || channel > MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[channel].streamType == AVS_CAP_IPC) {
    Onvif_Info* pOnvifInfo = GetOnvifInfo();

    if(SourceConfigs->AudioSource_num< VZ_LIST_MAX_NUM
       && strlen(
         pOnvifInfo->Profiles[channel].meta_profile[0].AudioSourceConfiguration.oldtoken)) {
      strcpy(SourceConfigs->AudioSourceConfig[SourceConfigs->AudioSource_num].oldtoken,
             pOnvifInfo->Profiles[channel].meta_profile[0].AudioSourceConfiguration.oldtoken);
      strcpy(SourceConfigs->AudioSourceConfig[SourceConfigs->AudioSource_num].oldSourceToken,
             pOnvifInfo->Profiles[channel].meta_profile[0].AudioSourceConfiguration.oldSourceToken);
      strcpy(SourceConfigs->AudioSourceConfig[SourceConfigs->AudioSource_num].Name,
             pOnvifInfo->Profiles[channel].meta_profile[0].AudioSourceConfiguration.Name);
      strcpy(SourceConfigs->AudioSourceConfig[SourceConfigs->AudioSource_num].token,
             pOnvifInfo->Profiles[channel].meta_profile[0].AudioSourceConfiguration.token);
      SourceConfigs->AudioSource_num++;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetRemoteAudioEncoderConfigs_OneChannel(
  LocalSDK__AudioEncoderConfigs *EncoderConfigs,int channel) {
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(channel < 0 || channel > MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[channel].streamType == AVS_CAP_IPC) {
    Onvif_Info* pOnvifInfo = GetOnvifInfo();
    int i=0;

    for(i=0; i<MAX_PROFILE_NUM; i++) {
      if(EncoderConfigs->AudioEncoder_num< VZ_LIST_MAX_NUM
         && strlen(
           pOnvifInfo->Profiles[channel].meta_profile[i].AudioEncoderConfiguration.oldtoken)) {
        strcpy(EncoderConfigs->AudioEncoderConfig[EncoderConfigs->AudioEncoder_num].oldtoken,
               pOnvifInfo->Profiles[channel].meta_profile[i].AudioEncoderConfiguration.oldtoken);
        strcpy(EncoderConfigs->AudioEncoderConfig[EncoderConfigs->AudioEncoder_num].Name,
               pOnvifInfo->Profiles[channel].meta_profile[i].AudioEncoderConfiguration.Name);
        strcpy(EncoderConfigs->AudioEncoderConfig[EncoderConfigs->AudioEncoder_num].token,
               pOnvifInfo->Profiles[channel].meta_profile[i].AudioEncoderConfiguration.token);
        EncoderConfigs->AudioEncoder_num++;
      }
    }

  } else {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

void LocalSDK__Get_RealProfileConfig(LocalSDK__RemoteChannel_ProfileConfig_Info
                                     *Info) {
  int i=0;
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return ;
  }

  int l=0;
  int Profileflag=0;
  int Configflag=0;
  Info->channel= -1;

  for(l=0; l<MAX_CHANNEL; l++) {
    if(sys_source_cfg[l].streamType == AVS_CAP_IPC) {
      for(i=0; i<MAX_PROFILE_NUM; i++) {
        if((Profileflag == 0) && strlen(Info->ProfileToken)
           && (strcmp(Info->ProfileToken,
                      pOnvifInfo->Profiles[l].meta_profile[i].token)==0)) {
          Info->channel= l;
          strcpy(Info->ProfileToken,pOnvifInfo->Profiles[l].meta_profile[i].oldtoken);
          Profileflag =1;
        }

        if(strlen(Info->ConfigToken)&&(Configflag == 0)) {
          if((Info->ConfigType == vz_Config_VideoEncoder)
             && (strcmp(Info->ConfigToken,
                        pOnvifInfo->Profiles[l].meta_profile[i].VideoEncoderConfig.token)==0)) {
            strcpy(Info->ConfigToken,
                   pOnvifInfo->Profiles[l].meta_profile[i].VideoEncoderConfig.oldtoken);
            Configflag =1;

          } else if((Info->ConfigType == vz_Config_VideoSource)
                    && (strcmp(Info->ConfigToken,
                               pOnvifInfo->Profiles[l].meta_profile[i].VideoSourceConfig.token)==0)) {
            int index = sys_source_cfg[l].userID;
            strcpy(Info->ConfigToken,
                   pOnvifInfo->remote_server[index].video.VideoSourceRelayToken);
            Configflag =1;

          } else if((Info->ConfigType == vz_Config_AudioEncoder)
                    && (strcmp(Info->ConfigToken,
                               pOnvifInfo->Profiles[l].meta_profile[i].AudioEncoderConfiguration.token)==0)) {
            strcpy(Info->ConfigToken,
                   pOnvifInfo->Profiles[l].meta_profile[i].AudioEncoderConfiguration.oldtoken);
            Configflag =1;

          } else if((Info->ConfigType == vz_Config_AudioSource)
                    && (strcmp(Info->ConfigToken,
                               pOnvifInfo->Profiles[l].meta_profile[i].AudioSourceConfiguration.token)==0)) {
            strcpy(Info->ConfigToken,
                   pOnvifInfo->Profiles[l].meta_profile[i].AudioSourceConfiguration.oldtoken);
            Configflag =1;
          }
        }

        if(Configflag && Profileflag)
          break;
      }
    }

    if(Configflag && Profileflag)
      break;
  }
}

int LocalSDK__Get_RemoteConfig(char *config ,int ConfigType,
                               LocalSDK__RemoteChannel_Info *Info) {
  int i=0;
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int l=0;
  Info->channel= -1;

  for(l=0; l<MAX_CHANNEL; l++) {
    if(sys_source_cfg[l].streamType == AVS_CAP_IPC) {
      for(i=0; i<MAX_PROFILE_NUM; i++) {
        if(config) {
          if((ConfigType == vz_Config_VideoEncoder)
             && (strcmp(config,
                        pOnvifInfo->Profiles[l].meta_profile[i].VideoEncoderConfig.token)==0)) {
            strcpy(Info->Token,
                   pOnvifInfo->Profiles[l].meta_profile[i].VideoEncoderConfig.oldtoken);
            Info->channel=l;
            break;

          } else if((ConfigType == vz_Config_VideoSource)
                    && (strcmp(config,
                               pOnvifInfo->Profiles[l].meta_profile[i].VideoSourceConfig.token)==0)) {
            int index = sys_source_cfg[l].userID;
            strcpy(Info->Token,
                   pOnvifInfo->remote_server[index].video.VideoSourceRelayToken);
            Info->channel=l;
            break;

          } else if((ConfigType == vz_Config_AudioEncoder)
                    && (strcmp(config,
                               pOnvifInfo->Profiles[l].meta_profile[i].AudioEncoderConfiguration.token)==0)) {
            strcpy(Info->Token,
                   pOnvifInfo->Profiles[l].meta_profile[i].AudioEncoderConfiguration.oldtoken);
            Info->channel=l;
            break;

          } else if((ConfigType == vz_Config_AudioSource)
                    && (strcmp(config,
                               pOnvifInfo->Profiles[l].meta_profile[i].AudioSourceConfiguration.token)==0)) {
            strcpy(Info->Token,
                   pOnvifInfo->Profiles[l].meta_profile[i].AudioSourceConfiguration.oldtoken);
            Info->channel=l;
            break;
          }
        }
      }
    }

    if(Info->channel >= 0)
      break;
  }

  if(l == MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetAudioSourceConfigResponse(LocalSDK__AudioSourceConfigs
                                           *SourceConfigs,int remote_channel) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  int j=0;
  int k=0;
  int flag=0;
  int ret = VZ_LOCALAPI_FAILED;

  for(k=0; k<SourceConfigs->AudioSource_num; k++) {
    flag=0;

    for(j=0; j<MAX_CHANNEL; j++) {
      if(sys_source_cfg[j].streamType == AVS_CAP_IPC
         && sys_source_cfg[j].userID == remote_channel) {
        if(strlen(SourceConfigs->AudioSourceConfig[k].oldtoken)
           &&((strcmp(SourceConfigs->AudioSourceConfig[k].oldtoken,
                      pOnvifInfo->Profiles[j].meta_profile[0].AudioSourceConfiguration.oldtoken)==0)
              || (strcmp(SourceConfigs->AudioSourceConfig[k].oldtoken,
                         pOnvifInfo->Profiles[j].meta_profile[0].AudioSourceConfiguration.oldSourceToken)
                  ==0))) {
          flag=1;
          break;
        }
      }
    }

    if(flag) {
      strcpy(SourceConfigs->AudioSourceConfig[k].Name,
             pOnvifInfo->Profiles[j].meta_profile[0].AudioSourceConfiguration.Name);
      strcpy(SourceConfigs->AudioSourceConfig[k].token,
             pOnvifInfo->Profiles[j].meta_profile[0].AudioSourceConfiguration.token);
      ret = VZ_LOCALAPI_SUCCESS;
    }
  }

  return ret;
}

int LocalSDK__GetAudioEncoderConfigResponse(LocalSDK__AudioEncoderConfigs
                                            *EncoderConfigs,int remote_channel) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  int j=0;
  int k=0;
  int flag=0;
  int ret = VZ_LOCALAPI_FAILED;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  for(k=0; k<EncoderConfigs->AudioEncoder_num; k++) {
    flag=0;

    for(j=0; j<MAX_CHANNEL; j++) {
      if(sys_source_cfg[j].userID == remote_channel) {
        for(i=0; i<MAX_PROFILE_NUM; i++) {
          if(strlen(EncoderConfigs->AudioEncoderConfig[k].oldtoken)
             &&(strcmp(EncoderConfigs->AudioEncoderConfig[k].oldtoken,
                       pOnvifInfo->Profiles[j].meta_profile[i].AudioEncoderConfiguration.oldtoken)==
                0)) {
            flag=1;
            break;
          }
        }
      }

      if(flag)
        break;
    }

    if(flag) {
      strcpy(EncoderConfigs->AudioEncoderConfig[k].Name,
             pOnvifInfo->Profiles[j].meta_profile[i].AudioEncoderConfiguration.Name);
      strcpy(EncoderConfigs->AudioEncoderConfig[k].token,
             pOnvifInfo->Profiles[j].meta_profile[i].AudioEncoderConfiguration.token);
      ret= VZ_LOCALAPI_SUCCESS;
    }
  }

  return ret;
}

int LocalSDK__UpdateRemoteAudioEncoder(vz_onvif_AudioEncoderConfig *Config,
                                       int Channel) {
  if(Channel < 0 || Channel >= MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_Profiles value;
  memcpy(&value,&pOnvifInfo->Profiles[Channel],sizeof(vz_onvif_Profiles));
  strcpy(value.meta_profile[0].AudioEncoderConfiguration.oldtoken,
         Config->oldtoken);

  if(DoOnvifSetProfiles(&value,Channel) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  strcpy(Config->token,
         pOnvifInfo->Profiles[Channel].meta_profile[0].AudioEncoderConfiguration.token);
  strcpy(Config->Name,
         pOnvifInfo->Profiles[Channel].meta_profile[0].AudioEncoderConfiguration.Name);
  return VZ_LOCALAPI_SUCCESS;

}

int LocalSDK__UpdateRemoteAudioSource(vz_onvif_AudioSourceConfig *Config,
                                      int Channel) {
  if(Channel < 0 || Channel >= MAX_CHANNEL) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_Profiles value;
  memcpy(&value,&pOnvifInfo->Profiles[Channel],sizeof(vz_onvif_Profiles));
  strcpy(value.meta_profile[0].AudioSourceConfiguration.oldtoken,
         Config->oldtoken);
  strcpy(value.meta_profile[0].AudioSourceConfiguration.oldSourceToken,
         Config->oldSourceToken);

  if(DoOnvifSetProfiles(&value,Channel) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  strcpy(Config->token,
         pOnvifInfo->Profiles[Channel].meta_profile[0].AudioSourceConfiguration.token);
  strcpy(Config->Name,
         pOnvifInfo->Profiles[Channel].meta_profile[0].AudioSourceConfiguration.Name);
  return VZ_LOCALAPI_SUCCESS;

}


int LocalSDK__SetRecordingConfiguration(vz_onvif_Recording *pConfig) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  if(pOnvifInfo->Recordings.Recording_num != 0) {
    int i=0;

    for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
      if(strlen(pConfig->RecordingToken)
         &&strcmp(pConfig->RecordingToken,
                  pOnvifInfo->Recordings.Recording[i].RecordingToken)==0)
        break;
    }

    if(i==pOnvifInfo->Recordings.Recording_num) {
      gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
      return VZ_LOCALAPI_FAILED;

    } else {
      vz_onvif_Recordings value;
      memcpy(&value,&pOnvifInfo->Recordings,sizeof(vz_onvif_Recordings));
      memcpy(&value.Recording[i].Config,&pConfig->Config,
             sizeof(vz_onvif_RecordingCfg));

      if(DoOnvifSetRecordings(&value) <0 ) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }

  } else {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__SetRecordingJobConfiguration(vz_onvif_RecordingJob *Config) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  if(pOnvifInfo->RecordingJobs.RecordingJob_num!= 0) {
    int i=0;
    int index =0;
    int channel=0;

    for(i=0; i<pOnvifInfo->RecordingJobs.RecordingJob_num; i++) {
      if(strlen(Config->RecordingJobToken)
         &&strcmp(Config->RecordingJobToken,
                  pOnvifInfo->RecordingJobs.RecordingJob[i].RecordingJobToken)==0) {
        index = i;
        break;
      }
    }

    if(i==pOnvifInfo->RecordingJobs.RecordingJob_num) {
      gLocalErr = VZ_LOCALAPI_NO_RECORDINGJOB;
      return VZ_LOCALAPI_FAILED;
    }

    //find RecordingToken
    for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
      if(strcmp(Config->RecordingToken,
                pOnvifInfo->Recordings.Recording[i].RecordingToken)==0) {
        channel = pOnvifInfo->Recordings.Recording[i].channel;
        break;
      }
    }

    if(i>=pOnvifInfo->Recordings.Recording_num) {
      gLocalErr = VZ_LOCALAPI_NO_RECORDING;
      return VZ_LOCALAPI_FAILED;
    }

    vz_onvif_RecordingJobs RecordingJobs;
    memcpy(&RecordingJobs,&pOnvifInfo->RecordingJobs,sizeof(RecordingJobs));

    // auto create recording source
    for (i = 0; i < Config->sizeSource; i++) {
      if (Config->JobSource[i].AutoCreateReceiverFlag
          && Config->JobSource[i].AutoCreateReceiver) {
        Config->JobSource[i].SourceTokenFlag = 1;
        Config->JobSource[i].SourceToken.TypeFlag = 1;
        strcpy(Config->JobSource[i].SourceToken.Type,
               "http://www.onvif.org/ver10/schema/Profile");

        strcpy(Config->JobSource[i].SourceToken.Token,
               pOnvifInfo->Profiles[channel].meta_profile[0].token);
      }
    }


    memcpy(&RecordingJobs.RecordingJob[index],Config,sizeof(vz_onvif_RecordingJob));

    if(DoOnvifSetRecordingJobs(&RecordingJobs) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

  } else {
    gLocalErr = VZ_LOCALAPI_NO_RECORDINGJOB;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetRecordingSummary(LocalSDK__RecordingSummary_Info *Info) {
  gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
  return VZ_LOCALAPI_FAILED;
}

int LocalSDK__GetNodes(LocalSDK__PTZNodes *Nodes) {
  Nodes->Node_num =1;
  int i=0;

  //暂时写死，范围只有0 ~ 63
  for(i=0; i<Nodes->Node_num; i++) {
    strcpy(Nodes->Node[i].Name,"LocalPTZ");
    Nodes->Node[i].SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.XRange.Min = 0;
    Nodes->Node[i].SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.XRange.Max= 63;
    Nodes->Node[i].SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.YRange.Min = 0;
    Nodes->Node[i].SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.YRange.Max= 63;
    Nodes->Node[i].SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.URI = NULL;
    Nodes->Node[i].SupportedPTZSpaces.ContinuousZoomVelocitySpace.XRange.Min = 0;
    Nodes->Node[i].SupportedPTZSpaces.ContinuousZoomVelocitySpace.XRange.Max= 63;
    Nodes->Node[i].SupportedPTZSpaces.ContinuousZoomVelocitySpace.URI = NULL;
  }

  return VZ_LOCALAPI_SUCCESS;
}



int LocalSDK__GetSerialPortConfiguration(LocalSDK__SerialPortConfig *Config) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<MAX_SERIAL_PORTS_NUM; i++) {
    if(strcmp(Config->PortToken,pOnvifInfo->SerialPorts[i].Token)==0) {
      break;
    }
  }

  if(i==MAX_SERIAL_PORTS_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  Config->BaudRate = pOnvifInfo->SerialPorts[i].Config.BaudRate;
  Config->CharacterLength = pOnvifInfo->SerialPorts[i].Config.DataBits;
  Config->StopBit = (float)pOnvifInfo->SerialPorts[i].Config.StopBit;
  Config->type = VZ__SerialPortType__RS485HalfDuplex;

  Config->ParityBit = VZ__ParityBit__Extended;

  switch(pOnvifInfo->SerialPorts[i].Config.Parity) {
  case 0:
    Config->ParityBit = VZ__ParityBit__None;
    break;

  case 1:
    Config->ParityBit = VZ__ParityBit__Odd;
    break;

  case 2:
    Config->ParityBit = VZ__ParityBit__Even;
    break;

  case 3:
    Config->ParityBit = VZ__ParityBit__Mark;
    break;

  case 4:
    Config->ParityBit = VZ__ParityBit__Space;
    break;

  default:
    break;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetSerialPortConfiguration(LocalSDK__SerialPortConfig *Config) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;
  int index = 0;

  for(i=0; i<MAX_SERIAL_PORTS_NUM; i++) {
    if(strcmp(Config->PortToken,pOnvifInfo->SerialPorts[i].Token)==0) {
      index = i;
      break;
    }
  }

  if(i==MAX_SERIAL_PORTS_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_SerialPortsCfg value;
  memset(&value,0,sizeof(value));

  strcpy(value.Type,pOnvifInfo->SerialPorts[i].Config.Type);
  value.Comm = pOnvifInfo->SerialPorts[i].Config.Comm;
  value.DataBits = Config->CharacterLength;
  value.BaudRate = Config->BaudRate;
  value.StopBit = (unsigned int)Config->StopBit;

  switch(Config->ParityBit) {
  case VZ__ParityBit__None:
    value.Parity = 0;
    break;

  case VZ__ParityBit__Odd:
    value.Parity = 1;
    break;

  case VZ__ParityBit__Even:
    value.Parity = 2;
    break;

  case VZ__ParityBit__Mark:
    value.Parity = 3;
    break;

  case VZ__ParityBit__Space:
    value.Parity = 4;
    break;

  default:
    value.Parity = 0;
    break;
  }

  if(DoOnvifSetSerialPortsCfg(&value,index) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetSerialPortCfgOptions(LocalSDK__GetSerialPortConfigOptions
                                      *pGetOptions) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<MAX_SERIAL_PORTS_NUM; i++) {
    if(strcmp(pGetOptions->PortToken,pOnvifInfo->SerialPorts[i].Token)==0) {
      break;
    }
  }

  if(i==MAX_SERIAL_PORTS_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  int index =0;
  pGetOptions->Options.ParityBit_Num = 5;

  for(i=0; i < pGetOptions->Options.ParityBit_Num; i++) {
    pGetOptions->Options.ParityBit[i] = VZ__ParityBit__None +i;
  }

  pGetOptions->Options.DataBits_Num = 5;

  for(i=0; i < pGetOptions->Options.DataBits_Num; i++) {
    pGetOptions->Options.DataBits[i] = 4 +i;
  }

  pGetOptions->Options.BaudRate[index++] = 300;
  pGetOptions->Options.BaudRate[index++] = 600;
  pGetOptions->Options.BaudRate[index++] = 1200;
  pGetOptions->Options.BaudRate[index++] = 2400;
  pGetOptions->Options.BaudRate[index++] = 4800;
  pGetOptions->Options.BaudRate[index++] = 9600;
  pGetOptions->Options.BaudRate[index++] = 14400;
  pGetOptions->Options.BaudRate[index++] = 19200;
  pGetOptions->Options.BaudRate[index++] = 38400;
  pGetOptions->Options.BaudRate[index++] = 43000;
  pGetOptions->Options.BaudRate[index++] = 56000;
  pGetOptions->Options.BaudRate[index++] = 57600;
  pGetOptions->Options.BaudRate[index++] = 115200;
  pGetOptions->Options.BaudRate_Num = index;

  index =0;
  pGetOptions->Options.StopBit[index++] = 0;
  pGetOptions->Options.StopBit[index++] = 1;
  pGetOptions->Options.StopBit[index++] = 2;
  pGetOptions->Options.StopBit_Num = index;

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetRelayOSDToken(int channel,
                               LocalSDK__OSDRelayTokens *OSDTokens) {
  /*
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_OSDCfg value;
  memcpy(&value,&pOnvifInfo->OSDCfgs[channel],sizeof(vz_onvif_OSDCfg));
  memcpy(value.RelayToken,OSDTokens,sizeof(LocalSDK__OSDRelayTokens));

  if(ControlSystemDataALLChannel(channel,SYS_MSG_SET_ONVIF_OSDTOKEN, &value, sizeof(vz_onvif_OSDCfg)) < 0){
  	gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
  	return VZ_LOCALAPI_FAILED;
  }*/
  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__Find_Profile(char *ProfileToken) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(ProfileToken,pOnvifInfo,&ChannelId,
                                        &index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__Find_Video_Analytics(char *AnalyticsToken,int *index) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i =0;

  for(i =0; i<MAX_VIDEO_ANALYTICS_NUM; i++) {
    if(pOnvifInfo->VideoAnalytics[i].used
       && strcmp(pOnvifInfo->VideoAnalytics[i].VACfg.token ,AnalyticsToken)==0) {
      *index = i;
      break;
    }
  }

  if(i>=MAX_VIDEO_ANALYTICS_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__Find_Video_Source(char *VideoSourceToken) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_VideoSourceToken(VideoSourceToken,pOnvifInfo,
                                            &ChannelId,&index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_NO_VSOURCE;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__CreateTrack(char *p_RecordingToken,vz_onvif_Track *p_Track) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
    if(strcmp(p_RecordingToken,
              pOnvifInfo->Recordings.Recording[i].RecordingToken) == 0) {
      break;
    }
  }

  if(i>=pOnvifInfo->Recordings.Recording_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  if(pOnvifInfo->Recordings.Recording[i].TrackNum >= MAX_TRACK_NUM) {
    gLocalErr = VZ_LOCALAPI_TRACK_EXCEED;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_Recordings Recordings;
  memcpy(&Recordings,&pOnvifInfo->Recordings,sizeof(Recordings));
  vz_onvif_Recording *P_Record = &Recordings.Recording[i];

  memcpy(&P_Record->Tracks[P_Record->TrackNum],p_Track,sizeof(vz_onvif_Track));
  P_Record->TrackNum++;

  if(DoOnvifSetRecordings(&Recordings) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__DeleteTrack(char *p_RecordingToken,char *p_TrackToken) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i,j;
  int add_len = 0;

  for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
    if(strcmp(p_RecordingToken,
              pOnvifInfo->Recordings.Recording[i].RecordingToken) == 0) {
      break;
    }
  }

  if(i>=pOnvifInfo->Recordings.Recording_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_Recordings Recordings;
  memcpy(&Recordings,&pOnvifInfo->Recordings,sizeof(Recordings));
  vz_onvif_Recording *P_Record = &Recordings.Recording[i];

  for(j=0; i<P_Record->TrackNum; j++) {
    if(strcmp(p_TrackToken,P_Record->Tracks[j].TrackToken) == 0) {
      strcpy(P_Record->Tracks[j].TrackToken,"");
      break;
    }
  }

  if(j>=P_Record->TrackNum) {
    gLocalErr = VZ_LOCALAPI_NO_TRACK;
    return VZ_LOCALAPI_FAILED;
  }

  //delete track
  int last_empty_str_index=P_Record->TrackNum;

  //排序
  for(j=0; j<P_Record->TrackNum; j++) {
    if(strlen(P_Record->Tracks[j].TrackToken)!=0) {
      if(last_empty_str_index <j) {
        memcpy(&(P_Record->Tracks[last_empty_str_index]),&(P_Record->Tracks[j]),
               sizeof(vz_onvif_Track));
        memset(&(P_Record->Tracks[j]),0,sizeof(vz_onvif_Track));
        last_empty_str_index++;
      }

      add_len++;

    } else {
      if(last_empty_str_index >j)
        last_empty_str_index = j;

      memset(&(P_Record->Tracks[j]),0,sizeof(vz_onvif_Track));
    }
  }

  //长度修正
  P_Record->TrackNum = add_len;

  if(DoOnvifSetRecordings(&Recordings) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetTrackConfiguration(char *p_RecordingToken,
                                    vz_onvif_Track *p_Track) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i,j;

  for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
    if(strcmp(p_RecordingToken,
              pOnvifInfo->Recordings.Recording[i].RecordingToken) == 0) {
      break;
    }
  }

  if(i>=pOnvifInfo->Recordings.Recording_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  vz_onvif_Recordings Recordings;
  memcpy(&Recordings,&pOnvifInfo->Recordings,sizeof(Recordings));
  vz_onvif_Recording *P_Record = &Recordings.Recording[i];

  for(j=0; i<P_Record->TrackNum; j++) {
    if(strcmp(p_Track->TrackToken,P_Record->Tracks[j].TrackToken) == 0) {
      break;
    }
  }

  if(j>=P_Record->TrackNum) {
    gLocalErr = VZ_LOCALAPI_NO_TRACK;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(&P_Record->Tracks[j].Configuration,&p_Track->Configuration,
         sizeof(vz_onvif_TrackConfiguration));

  if(DoOnvifSetRecordings(&Recordings) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

//save remote video source config options
int LocalSDK__UpdateRemoteVSrcCfgOptions(int curchannel,
                                         vz_onvif_IntRectangleRange * p_option) {
  if(curchannel < 0 || curchannel > MAX_REMOTE_CHANNEL || p_option == NULL) {
    return VZ_LOCALAPI_FAILED;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_RemoteVideoInfo videoInfo;
  memcpy(&videoInfo,&pOnvifInfo->remote_server[curchannel].video,
         sizeof(vz_onvif_RemoteVideoInfo));
  memcpy(&videoInfo.RelayVSCfgOptions,p_option,
         sizeof(vz_onvif_IntRectangleRange));

  if(DoOnvifSetRVideoInfo(&videoInfo,curchannel) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}



#ifdef NEW_ONVIF

int LocalSDK__GetOSD_ex(char	*OSDToken,VZ_ONVIF_OSD *OSDCfg) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i = 0;

  for(i = 0; i<pOnvifInfo->OSDS.OSD_num; i++) {
    if(pOnvifInfo->OSDS.OSD[i].used
       && (strcmp(pOnvifInfo->OSDS.OSD[i].OSDCfg.token,OSDToken)==0)) {
      break;
    }
  }

  if (i >= pOnvifInfo->OSDS.OSD_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(OSDCfg,&pOnvifInfo->OSDS.OSD[i],sizeof(VZ_ONVIF_OSD));

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetOSD_display(int ChannelId,vz_onvif_OSDCfg *OSDConfig) {
  int channel =0;
  int Result= 0;
  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  OSD_config osd;

  //default channel 0
  if(ControlSystemData(MSG_GET_OSD, &osd, sizeof(OSD_config)) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  channel=sys_source_cfg[ChannelId].userID;

  if(sys_source_cfg[ChannelId].streamType == AVS_CAP_IPC) {
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    return VZ_LOCALAPI_SUCCESS;
  }

  //set Position
  int setPosflag=0;
  int Position=0;
  int x=0;
  int y=0;

  if(OSDConfig->Position.Type == vz_OSDPosType_UpperLeft) {
    x=0;
    y=0;

  } else if(OSDConfig->Position.Type == vz_OSDPosType_UpperRight) {
    x=0;
    y=530;

  } else if(OSDConfig->Position.Type == vz_OSDPosType_LowerLeft) {
    x=530;
    y=0;

  } else if(OSDConfig->Position.Type == vz_OSDPosType_LowerRight) {
    x=530;
    y=530;

  } else if(OSDConfig->Position.Type == vz_OSDPosType_Custom) {
    x=OSDConfig->Position.Pos.x;
    y=OSDConfig->Position.Pos.y;

  } else {
    fprintf(stderr,"Position type error!!!\n");
  }

  Position = ((x<<16) & 0xFFFF0000) + (y & 0xFFFF);
  setPosflag=1;

  __u8 value=0;
  __u8 enable= 0;

  //	set TextString
  if(OSDConfig->Type == vz_OSDType_Text) {
    if(OSDConfig->TextString.Type == vz_OSDTextType_Plain) {
      char text[16]= {0};
      text[0] = 'O';//if overlaytext is null string, msg wont pass to system_server,so add 'O' in beginning

      if(OSDConfig->TextString.PlainTextFlag
         && strlen(OSDConfig->TextString.PlainText))
        strncpy(text+1,OSDConfig->TextString.PlainText,14);

      if(setPosflag==0)
        Position = osd.nTextPosition;

      enable = OSDConfig->TextString.PlainEnableFlag;

      if((SetOSD_Txt(channel,Position,enable,text) < 0)
         ||(SetOSD_Time(channel,osd.timeposition,osd.tstampenable,osd.tstampformat) < 0)
         ||(SetOSD_Date(channel,osd.dateposition,osd.dstampenable,osd.dateformat) < 0)
         ||(SetOsd_VZLogo(channel,osd.nLogoPosition,osd.nLogoEnable,
                          osd.nDetailInfo) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

    } else if(OSDConfig->TextString.Type == vz_OSDTextType_Date) {
      /*M/d/yyyy - e.g. 3/6/2013
      MM/dd/yyyy - e.g. 03/06/2013
      dd/MM/yyyy - e.g. 06/03/2013
      yyyy/MM/dd - e.g. 2013/03/06
      yyyy-MM-dd - e.g. 2013-06-03
      dddd, MMMM dd, yyyy - e.g. Wednesday, March 06, 2013
      MMMM dd, yyyy - e.g. March 06, 2013
      dd MMMM, yyyy - e.g. 06 March, 2013*/

      Result = Transform_DateStr_ToValue(OSDConfig->TextString.DateFormat,&value);

      if(Result == VZ_LOCALAPI_FAILED) {
        fprintf(stderr,"DateFormat:%s not support or error!!!\n",
                OSDConfig->TextString.DateFormat);
        gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
        return VZ_LOCALAPI_FAILED;
      }

      if(setPosflag==0)
        Position = osd.dateposition;

      enable = OSDConfig->TextString.DateEnableFlag;

      if((SetOSD_Date(channel,Position,enable,value) < 0)
         ||(SetOSD_Time(channel,osd.timeposition,osd.tstampenable,osd.tstampformat) < 0)
         ||(SetOSD_Txt(channel,osd.nTextPosition,osd.nTextEnable,osd.overlaytext) < 0)
         ||(SetOsd_VZLogo(channel,osd.nLogoPosition,osd.nLogoEnable,
                          osd.nDetailInfo) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

    } else if(OSDConfig->TextString.Type == vz_OSDTextType_Time) {
      //our time format is 12hr and 24hr .set the default time format to 24hr
      /*h:mm:ss tt - e.g. 2:14:21 PM
      hh:mm:ss tt - e.g. 02:14:21 PM
      H:mm:ss - e.g. 14:14:21
      HH:mm:ss - e.g. 14:14:21*/
      value=1;

      if(strstr(OSDConfig->TextString.TimeFormat,"tt")) {
        value=0;
      }

      if(setPosflag==0)
        Position = osd.timeposition;

      enable = OSDConfig->TextString.TimeEnableFlag;

      if((SetOSD_Time(channel,Position,enable,value) < 0)
         ||(SetOSD_Date(channel,osd.dateposition,osd.dstampenable,osd.dateformat) < 0)
         ||(SetOSD_Txt(channel,osd.nTextPosition,osd.nTextEnable,osd.overlaytext) < 0)
         ||(SetOsd_VZLogo(channel,osd.nLogoPosition,osd.nLogoEnable,
                          osd.nDetailInfo) < 0)) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }

    } else if(OSDConfig->TextString.Type == vz_OSDTextType_DateAndTime) {
      gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
      return VZ_LOCALAPI_FAILED;

    } else {

      fprintf(stderr,"TextString type error!!!\n");
    }

  } else if(OSDConfig->Type == vz__OSDType__Extended) {
    value = osd.nDetailInfo;//OSDConfig->vzLogoId;

    if(setPosflag==0)
      Position = osd.nLogoPosition;

    enable = OSDConfig->ImageEnableFlag;

    if((SetOsd_VZLogo(channel,Position,enable,value) < 0)
       ||(SetOSD_Txt(channel,osd.nTextPosition,osd.nTextEnable,osd.overlaytext) < 0)
       ||(SetOSD_Time(channel,osd.timeposition,osd.tstampenable,osd.tstampformat) < 0)
       ||(SetOSD_Date(channel,osd.dateposition,osd.dstampenable,osd.dateformat) < 0)) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  /*else{//only set position
  	if(setPosflag){
  		Result =LocalSDK__SetOSD_Position(channel,Position,&osd,OSDConfig);
  		return Result;
  	}
  	else{
  		fprintf(stderr,"only suppot set the position and string,please input the right parameter!!!\n");
  		gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
  		return VZ_LOCALAPI_FAILED;
  	}
  }*/
  //not support set the TextString font/background color and set the fontsize and the image
  return VZ_LOCALAPI_SUCCESS;

}

int LocalSDK_CheckOSD(VZ_ONVIF_OSD *OSDConfig) {
  int channel =0;
  int ret = VZ_LOCALAPI_FAILED;
  char strtype[32]= {0};

  if (sscanf(OSDConfig->OSDCfg.token, "OSD_%d_%s", &channel, strtype) != 2) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return ret;
  }

  switch(OSDConfig->OSDCfg.TextString.Type) {
  case vz_OSDTextType_Plain:
    if(strcmp(strtype,"Plain") == 0) {
      ret = VZ_LOCALAPI_SUCCESS;
    }

    break;

  case vz_OSDTextType_Date:
    if(strcmp(strtype,"Date") == 0) {
      ret = VZ_LOCALAPI_SUCCESS;
    }

    break;

  case vz_OSDTextType_Time:
    if(strcmp(strtype,"Time") == 0) {
      ret = VZ_LOCALAPI_SUCCESS;
    }

    break;

  default:
    break;
  }

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
  }

  return ret;
}


int LocalSDK__SetOSD_ex(VZ_ONVIF_OSD *OSDConfig) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i = 0;
  int result = VZ_LOCALAPI_SUCCESS;
  int channel = 0;

  for(i = 0; i<pOnvifInfo->OSDS.OSD_num; i++) {
    if(pOnvifInfo->OSDS.OSD[i].used
       && (strcmp(pOnvifInfo->OSDS.OSD[i].OSDCfg.token,OSDConfig->OSDCfg.token)==0)) {
      channel = pOnvifInfo->OSDS.OSD[i].channel;
      break;
    }
  }

  if (i >= pOnvifInfo->OSDS.OSD_num) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  VideoSourceCfg				sys_source_cfg[MAX_CHANNEL];

  if(ControlSystemData(MSG_GET_SOURCECFG, sys_source_cfg,
                       sizeof(VideoSourceCfg)*MAX_CHANNEL) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  if(sys_source_cfg[channel].streamType == AVS_CAP_IPC) { //relay
    strcpy(OSDConfig->OSDCfg.token,pOnvifInfo->OSDS.OSD[i].OSDCfg.relaytoken);
    strcpy(OSDConfig->OSDCfg.VideoSourceConfigurationToken,
           pOnvifInfo->OSDS.OSD[i].OSDCfg.relayVSrcCfgToken);
    OSDConfig->channel = channel;
    OSDConfig->used = 1;
    result = VZ_LOCALAPI_FAILED;

  } else {
    result = LocalSDK_CheckOSD(OSDConfig);

    if(result == VZ_LOCALAPI_SUCCESS) {
      int ret = LocalSDK__SetOSD_display(pOnvifInfo->OSDS.OSD[i].channel,
                                         &OSDConfig->OSDCfg);

      if(ret == VZ_LOCALAPI_FAILED)
        return ret;
    }
  }

  VZ_ONVIF_OSDs Value;
  memcpy(&Value,&pOnvifInfo->OSDS,sizeof(Value));
  memcpy(&Value.OSD[i].OSDCfg,&OSDConfig->OSDCfg,sizeof(vz_onvif_OSDCfg));

  if(DoOnvifSetOSD(&Value) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return result;
}



int LocalSDK__SetDiscoveryMode(vz_onvif_DiscoveryMode * DiscoveryMode) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  vz_onvif_DiscoveryMode value = *DiscoveryMode;

  if(*DiscoveryMode != pOnvifInfo->DiscoveryMode) {
    if(DoOnvifSetDiscoveryMode(&value) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetNode(char *PTZNodetoken,int * NodeID) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i = 0;
  int flag = 0;

  for( i = 0; i<MAX_PTZ_NODE_NUM; i++) {
    if(pOnvifInfo->ptz_node[i].used
       && (strcmp(pOnvifInfo->ptz_node[i].PTZNode.token,PTZNodetoken)==0)) {
      flag = 1;
      *NodeID = i;
      break;
    }
  }

  if(flag == 0) {
    gLocalErr = VZ_LOCALAPI_NO_PTZNODE;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__GetPTZConfigurationOptions(char *ptzCfgToken,int * NodeID) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i = 0;
  int flag = 0;
  char *PTZNodetoken = NULL;

  for( i = 0; i<MAX_PTZ_CFG_NUM; i++) {
    if(pOnvifInfo->ptz_cfg[i].used
       && (strcmp(pOnvifInfo->ptz_cfg[i].PTZConfiguration.token,ptzCfgToken)==0)) {
      PTZNodetoken=pOnvifInfo->ptz_cfg[i].PTZConfiguration.NodeToken;
      break;
    }
  }

  if(PTZNodetoken != NULL) {
    for( i = 0; i<MAX_PTZ_NODE_NUM; i++) {
      if(pOnvifInfo->ptz_node[i].used
         && (strcmp(pOnvifInfo->ptz_node[i].PTZNode.token,PTZNodetoken)==0)) {
        flag = 1;
        *NodeID = i;
        break;
      }
    }
  }

  if(flag == 0) {
    gLocalErr = VZ_LOCALAPI_NO_PTZNODE;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__SetPTZConfiguration(vz_onvif_PTZConfiguration *pCFG) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i = 0;
  int index = 0;

  for( i = 0; i<MAX_PTZ_CFG_NUM; i++) {
    if(pOnvifInfo->ptz_cfg[i].used
       && (strcmp(pOnvifInfo->ptz_cfg[i].PTZConfiguration.token,pCFG->token)==0)) {
      index = i;
      break;
    }
  }

  if (i >= MAX_PTZ_CFG_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  for( i = 0; i<MAX_PTZ_NODE_NUM; i++) {
    if(pOnvifInfo->ptz_node[i].used
       && (strcmp(pOnvifInfo->ptz_node[i].PTZNode.token,pCFG->NodeToken)==0)) {
      break;
    }
  }


  if (i >= MAX_PTZ_NODE_NUM) {
    gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
    return VZ_LOCALAPI_FAILED;
  }

  if (pCFG->DefaultPTZTimeoutFlag) {
    if (pCFG->DefaultPTZTimeout < pOnvifInfo->PTZCfgOptions.PTZTimeout.Min ||
        pCFG->DefaultPTZTimeout > pOnvifInfo->PTZCfgOptions.PTZTimeout.Max) {
      gLocalErr = VZ_LOCALAPI_VALUE_INVALID;
      return VZ_LOCALAPI_FAILED;
    }
  }

  // todo : add set ptz configuration code ...

  vz_onvif_PTZConfiguration value;
  memcpy(&value,pCFG,sizeof(vz_onvif_PTZConfiguration));

  if(DoOnvifSetPTZCfg(&value,index) <0 ) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK__GetPTZConfiguration(char *PTZCFGtoken,int * CFGID) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i = 0;
  int flag = 0;

  for( i = 0; i<MAX_PTZ_CFG_NUM; i++) {
    if(pOnvifInfo->ptz_cfg[i].used
       && (strcmp(pOnvifInfo->ptz_cfg[i].PTZConfiguration.token,PTZCFGtoken)==0)) {
      flag = 1;
      *CFGID = i;
      break;
    }
  }

  if(flag == 0) {
    gLocalErr = VZ_LOCALAPI_NO_PTZNODE;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}




static int LocalSDK__AddPTZCFG2Profile(Onvif_Info* pOnvifInfo,
                                       LocalSDK__AddCFG *cfginfo,int channel,int index) {
  int i = 0;
  int addindex = 0;

  for( i = 0; i<MAX_PTZ_CFG_NUM; i++) {
    if(pOnvifInfo->ptz_cfg[i].used
       && (strcmp(pOnvifInfo->ptz_cfg[i].PTZConfiguration.token,
                  cfginfo->ConfigurationToken)==0)) {
      addindex = i;
      break;
    }
  }

  if(i >= MAX_PTZ_CFG_NUM) {
    gLocalErr = VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  if (pOnvifInfo->Profiles[channel].meta_profile[index].PTZCfg_id != addindex) {
    int deindex = pOnvifInfo->Profiles[channel].meta_profile[index].PTZCfg_id;

    if ((deindex>0)
        && pOnvifInfo->ptz_cfg[deindex].PTZConfiguration.UseCount > 0) {
      vz_onvif_PTZConfiguration value;
      memcpy(&value,&pOnvifInfo->ptz_cfg[deindex].PTZConfiguration,
             sizeof(vz_onvif_PTZConfiguration));
      value.UseCount --;

      if(DoOnvifSetPTZCfg(&value,deindex) <0 ) {
        gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
        return VZ_LOCALAPI_FAILED;
      }
    }

    vz_onvif_PTZConfiguration add_value;
    memcpy(&add_value,&pOnvifInfo->ptz_cfg[addindex].PTZConfiguration,
           sizeof(vz_onvif_PTZConfiguration));
    add_value.UseCount ++;

    if(DoOnvifSetPTZCfg(&add_value,addindex) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }

    vz_onvif_Profiles pro_value;
    memcpy(&pro_value,&pOnvifInfo->Profiles[channel],sizeof(pro_value));
    pro_value.meta_profile[index].PTZCfg_id = -1;

    if(DoOnvifSetProfiles(&pro_value,channel) < 0) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__AddCFG2Profile(LocalSDK__AddCFG *cfginfo) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(cfginfo->ProfileToken,pOnvifInfo,
                                        &ChannelId,&index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  switch(cfginfo->type) {
  case vz_CFGType_VideoSource:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_VideoEncoder:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_AudioSource:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_AudioEncoder:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_PTZ:
    ret=LocalSDK__AddPTZCFG2Profile(pOnvifInfo,cfginfo,ChannelId,index);
    break;

  case vz_CFGType_VideoAnalytics:
    ret=LocalSDK__AddVideoAnalytics2Profile(pOnvifInfo,cfginfo->ConfigurationToken,
                                            ChannelId,index);
    break;

  default:
    break;

  }

  return ret;
}




static int LocalSDK__DelPTZCFGFromProfile(Onvif_Info* pOnvifInfo,int channel,
                                          int index) {
  int deindex = pOnvifInfo->Profiles[channel].meta_profile[index].PTZCfg_id;

  if ((deindex>0)
      && pOnvifInfo->ptz_cfg[deindex].PTZConfiguration.UseCount > 0) {
    vz_onvif_PTZConfiguration value;
    memcpy(&value,&pOnvifInfo->ptz_cfg[deindex].PTZConfiguration,sizeof(value));
    value.UseCount --;

    if(DoOnvifSetPTZCfg(&value,deindex) <0 ) {
      gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
      return VZ_LOCALAPI_FAILED;
    }
  }

  vz_onvif_Profiles pro_value;
  memcpy(&pro_value,&pOnvifInfo->Profiles[channel],sizeof(pro_value));
  pro_value.meta_profile[index].PTZCfg_id = -1;

  if(DoOnvifSetProfiles(&pro_value,channel) < 0) {
    gLocalErr = VZ_LOCALAPI_ACTION_FAIL;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}


int LocalSDK__DelCFGFromProfile(LocalSDK__DelCFG *cfginfo) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int ChannelId=-1;
  int index=0;
  int ret=LocalSDK__LookUp_ProfileToken(cfginfo->ProfileToken,pOnvifInfo,
                                        &ChannelId,&index);

  if(ret == VZ_LOCALAPI_FAILED) {
    gLocalErr = VZ_LOCALAPI_PROFILE_NONEXIST;
    return VZ_LOCALAPI_FAILED;
  }

  switch(cfginfo->type) {
  case vz_CFGType_VideoSource:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_VideoEncoder:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_AudioSource:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_AudioEncoder:
    gLocalErr = VZ_LOCALAPI_ACTION_UNSUPPORT;
    ret=VZ_LOCALAPI_FAILED;
    break;

  case vz_CFGType_PTZ:
    ret=LocalSDK__DelPTZCFGFromProfile(pOnvifInfo,ChannelId,index);
    break;

  case vz_CFGType_VideoAnalytics:
    ret=LocalSDK__DelVideoAnalyticsFromProfile(pOnvifInfo,ChannelId,index);
    break;

  default:
    break;

  }

  return ret;
}


int LocalSDK_GetRecordingOptions(LocalSDK__RecordingOptions * Option) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int channel = 0;

  if(pOnvifInfo->Recordings.Recording_num != 0) {
    int i=0;

    for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
      if(strlen(Option->RecordingToken)
         &&strcmp(Option->RecordingToken,
                  pOnvifInfo->Recordings.Recording[i].RecordingToken)==0) {
        channel = pOnvifInfo->Recordings.Recording[i].channel;
        break;
      }
    }

    if(i==pOnvifInfo->Recordings.Recording_num) {
      gLocalErr = VZ_LOCALAPI_NO_RECORDING;
      return VZ_LOCALAPI_FAILED;
    }

    int video = 0;
    int audio = 0;
    int meta = 0;
    vz_onvif_Recording *p_recording = &pOnvifInfo->Recordings.Recording[i];

    for(i=0; i<p_recording->TrackNum; i++) {
      if(p_recording->Tracks[i].Configuration.TrackType == vz_TrackType_Video) {
        video++;

      } else if(p_recording->Tracks[i].Configuration.TrackType ==
                vz_TrackType_Audio) {
        audio++;

      } else if(p_recording->Tracks[i].Configuration.TrackType ==
                vz_TrackType_Metadata) {
        meta++;
      }
    }

    Option->Track.SpareVideo = 2 - video;	// max support two video track
    Option->Track.SpareAudio = 2 - audio;	// max support two audio track
    Option->Track.SpareMetadata = 1 - meta;	// max support one metadata track
    Option->Track.SpareTotal = 2 - video + 2 - audio + 1 - meta;

    Option->Job.Spare = 2;
    snprintf(Option->Job.CompatibleSources, sizeof(Option->Job.CompatibleSources)-1,
             "%s",
             pOnvifInfo->Profiles[channel].meta_profile[0].token);

  } else {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK_find_recording(char *RecordingToken,
                            vz_onvif_Recording * RecordInfo) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
    if(strlen(RecordingToken)
       &&strcmp(RecordingToken,
                pOnvifInfo->Recordings.Recording[i].RecordingToken)==0) {
      break;
    }
  }

  if(i >= pOnvifInfo->Recordings.Recording_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(RecordInfo,&pOnvifInfo->Recordings.Recording[i],
         sizeof(vz_onvif_Recording));
  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK_find_track(char *RecordingToken,char *TrackToken,
                        vz_onvif_Track * TrackInfo) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int index = 0;
  int i=0;

  for(i=0; i<pOnvifInfo->Recordings.Recording_num; i++) {
    if(strlen(RecordingToken)
       &&strcmp(RecordingToken,
                pOnvifInfo->Recordings.Recording[i].RecordingToken)==0) {
      index = i;
      break;
    }
  }

  if(i >= pOnvifInfo->Recordings.Recording_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  for(i=0; i<pOnvifInfo->Recordings.Recording[index].TrackNum; i++) {
    if(strlen(TrackToken)
       &&strcmp(TrackToken,
                pOnvifInfo->Recordings.Recording[index].Tracks[i].TrackToken)==0) {
      break;
    }
  }

  if(i >= pOnvifInfo->Recordings.Recording[index].TrackNum) {
    gLocalErr = VZ_LOCALAPI_NO_TRACK;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(TrackInfo,&pOnvifInfo->Recordings.Recording[index].Tracks[i],
         sizeof(vz_onvif_Track));
  return VZ_LOCALAPI_SUCCESS;
}

int LocalSDK_find_recordingjob(char *RecordingjobToken,
                               vz_onvif_RecordingJob * RecordjobInfo) {
  Onvif_Info* pOnvifInfo = GetOnvifInfo();
  int i=0;

  for(i=0; i<pOnvifInfo->RecordingJobs.RecordingJob_num; i++) {
    if(strlen(RecordingjobToken)
       &&strcmp(RecordingjobToken,
                pOnvifInfo->RecordingJobs.RecordingJob[i].RecordingJobToken)==0) {
      break;
    }
  }

  if(i >= pOnvifInfo->RecordingJobs.RecordingJob_num) {
    gLocalErr = VZ_LOCALAPI_NO_RECORDING;
    return VZ_LOCALAPI_FAILED;
  }

  memcpy(RecordjobInfo,&pOnvifInfo->RecordingJobs.RecordingJob[i],
         sizeof(vz_onvif_RecordingJob));
  return VZ_LOCALAPI_SUCCESS;
}

#endif


