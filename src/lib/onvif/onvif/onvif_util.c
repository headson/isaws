/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/onvif_util.h"
#include <iconv.h>

void onvif_get_time_str(char * buff, int len, int sec_off)
{
	time_t nowtime;
	struct tm *gtime;	

	time(&nowtime);
	nowtime += sec_off;
	gtime = gmtime(&nowtime);

	snprintf(buff, len, "%04d-%02d-%02dT%02d:%02d:%02dZ", 		 
		gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday,
		gtime->tm_hour, gtime->tm_min, gtime->tm_sec);		
}

void onvif_get_time_str_s(char * buff, int len, time_t nowtime, int sec_off)
{
	struct tm *gtime;	

	nowtime += sec_off;
	gtime = gmtime(&nowtime);

	snprintf(buff, len, "%04d-%02d-%02dT%02d:%02d:%02dZ", 		 
		gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday,
		gtime->tm_hour, gtime->tm_min, gtime->tm_sec);		
}

BOOL onvif_is_valid_hostname(const char * name)
{
	// 0-9, a-z, A-Z, '-'

	const char * p = name;
	while (*p != '\0')
	{
		if ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p == '-') || (*p == '.'))
		{
			++p;
			continue;
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

void onvif_get_timezone(char * tz, int len)
{
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
    if (time_zone < -12) 
    {  
        time_zone += 24;   
    } 
    else if (time_zone > 12) 
    {  
        time_zone -= 24;  
    }  

	snprintf(tz, len, "PST%dPDT", time_zone);    
}

// stdoffset[dst[offset],[start[/time],end[/time]]]
BOOL onvif_is_valid_timezone(const char * tz)
{
	// not imcomplete

	const char * p = tz;
	while (*p != '\0')
	{
		if (*p >= '0' && *p <= '9')
		{
			return TRUE;
		}

		++p;
	}

	return FALSE;
}

const char * onvif_uuid_create()
{
	static char uuid[100];

	srand((unsigned int)time(NULL));
	sprintf(uuid, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x", 
		rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, 
		rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF);

	return uuid;
}

const char * onvif_get_local_ip()
{
	struct in_addr addr;
    addr.s_addr = get_default_if_ip();

	return inet_ntoa(addr);
}

time_t onvif_timegm(struct tm *T)
{
	time_t t, g, z;
	struct tm tm;
	
	t = mktime(T);	
	if (t == (time_t)-1)
	{
		return (time_t)-1;
	}
	
	tm = *gmtime(&t);

	tm.tm_isdst = 0;
	g = mktime(&tm);
	if (g == (time_t)-1)
	{
		return (time_t)-1;
	}
	
	z = g - t;
	return t - z;
}

/**
 * http://192.168.5.235/onvif/device_service
 */
int onvif_parse_xaddr_(const char * pdata, char * host, char * url, int * port)
{
    const char *p1, *p2;
    int len = strlen(pdata);

    *port = 80;
    
    if (len > 7) // skip "http://"
    {
        p1 = strchr(pdata+7, ':');
        if (p1)
        {
			char buff[100];

            strncpy(host, pdata+7, p1-pdata-7);            
            memset(buff, 0, 100);
            
            p2 = strchr(p1, '/');
            if (p2)
            {
                strncpy(url, p2, len - (p2 - pdata));
                
                len = p2 - p1 - 1;
                strncpy(buff, p1+1, len);                
            }
            else
            {
                len = len - (p1 - pdata);
                strncpy(buff, p1+1, len);
            }  

            *port = atoi(buff);
        }
        else
        {
            p2 = strchr(pdata+7, '/');
            if (p2)
            {
                strncpy(url, p2, len - (p2 - pdata));
                
                len = p2 - pdata - 7;
                strncpy(host, pdata+7, len);
            }
            else
            {
                len = len - 7;
                strncpy(host, pdata+7, len);
            }
        }
    }

    return 1;
}

/**
 * http://192.168.5.235/onvif/device_service http://[fe80::c256:e3ff:fea2:e019]/onvif/device_service 
 */
int onvif_parse_xaddr(const char * pdata, char * host, char * url, int * port)
{
    const char *p1, *p2;
    char xaddr[256];

    p2 = pdata;
    p1 = strchr(pdata, ' ');
    while (p1)
    {
        strncpy(xaddr, p2, p1-p2);

        onvif_parse_xaddr_(xaddr, host, url, port);
        if (is_ip_address(host)) // if ipv4 address
        {
            return 1;
        }

        p2 = p1+1;
        p1 = strchr(p2, ' ');
    }

	if (p2)
	{
		onvif_parse_xaddr_(p2, host, url, port);	
	}

    return 1;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc = 0;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd == (iconv_t)-1) return -1;
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,&outlen) == -1)
	{
		rc = -1;
	}
	iconv_close(cd);
	
	return rc;
}
	
int UTF8_to_GB2312(char *inbuf,int inlen,char *outbuf,int outlen)
{
	if(inbuf == NULL || outbuf == NULL)
		return -1;
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}

int GB2312_to_UTF8(char *inbuf,int inlen,char *outbuf,int outlen)
{
	if(inbuf == NULL || outbuf == NULL)
		return -1;
	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}


