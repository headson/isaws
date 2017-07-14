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
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/soap_parser.h"
#include "onvif/onvif/onvif_util.h"


/***************************************************************************************/
BOOL parse_Bool(const char * pdata)
{    
    if (strcasecmp(pdata, "true") == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL parse_XSDDatetime(const char * s, time_t * p)
{
	if (s)
	{ 
		char zone[32];
		struct tm T;
		const char *t;
		
		*zone = '\0';
		memset(&T, 0, sizeof(T));
		
		if (strchr(s, '-'))
		{
			t = "%d-%d-%dT%d:%d:%d%31s";
		}	
		else if (strchr(s, ':'))
		{
			t = "%4d%2d%2dT%d:%d:%d%31s";
		}	
		else /* parse non-XSD-standard alternative ISO 8601 format */
		{
			t = "%4d%2d%2dT%2d%2d%2d%31s";
		}
		
		if (sscanf(s, t, &T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec, zone) < 6)
		{
			return FALSE;
		}
		
		if (T.tm_year == 1)
		{
			T.tm_year = 70;
		}	
		else
		{
			T.tm_year -= 1900;
		}
		
		T.tm_mon--;
		
		if (*zone == '.')
		{ 
			for (s = zone + 1; *s; s++)
			{
				if (*s < '0' || *s > '9')
				{
					break;
				}	
			}	
		}
    	else
    	{
      		s = zone;
      	}
      	
		if (*s)
		{
			if (*s == '+' || *s == '-')
			{ 
				int h = 0, m = 0;
				if (s[3] == ':')
				{ 
					/* +hh:mm */
					sscanf(s, "%d:%d", &h, &m);
					if (h < 0)
						m = -m;
				}
				else /* +hhmm */
				{
					m = (int)strtol(s, NULL, 10);
					h = m / 100;
					m = m % 100;
				}
				
				T.tm_min -= m;
				T.tm_hour -= h;
				/* put hour and min in range */
				T.tm_hour += T.tm_min / 60;
				T.tm_min %= 60;
				
				if (T.tm_min < 0)
				{ 
					T.tm_min += 60;
					T.tm_hour--;
				}
				
				T.tm_mday += T.tm_hour / 24;
				T.tm_hour %= 24;
				
				if (T.tm_hour < 0)
				{
					T.tm_hour += 24;
					T.tm_mday--;
				}
				/* note: day of the month may be out of range, timegm() handles it */
			}

			*p = onvif_timegm(&T);
		}
		else /* no UTC or timezone, so assume we got a localtime */
		{ 
			T.tm_isdst = -1;
			*p = mktime(&T);
		}
	}
	
	return TRUE;
}

BOOL parse_XSDDuration(const char *s, int *a)
{ 
	int sign = 1, Y = 0, M = 0, D = 0, H = 0, N = 0, S = 0;
	float f = 0;
	*a = 0;
	if (s)
	{ 
		if (*s == '-')
		{ 
			sign = -1;
			s++;
		}
		if (*s++ != 'P')
			return FALSE;
			
		/* date part */
		while (s && *s)
		{ 
			int n;
			char k;
			if (*s == 'T')
			{ 
				s++;
				break;
			}
			
			if (sscanf(s, "%d%c", &n, &k) != 2)
				return FALSE;
				
			s = strchr(s, k);
			if (!s)
				return FALSE;
				
			switch (k)
			{ 
			case 'Y':
				Y = n;
				break;
				
			case 'M':
				M = n;
				break;
				
			case 'D':
				D = n;
				break;
				
			default:
				return FALSE;
			}
			
			s++;
		}
		
	    /* time part */
	    while (s && *s)
		{ 
			int n;
			char k;
			if (sscanf(s, "%d%c", &n, &k) != 2)
				return FALSE;
				
			s = strchr(s, k);
			if (!s)
				return FALSE;
				
			switch (k)
			{ 
			case 'H':
				H = n;
				break;
				
			case 'M':
				N = n;
				break;
				
			case '.':
				S = n;
				if (sscanf(s, "%g", &f) != 1)
					return FALSE;
				s = NULL;
				continue;
				
			case 'S':
				S = n;
				break;
				
			default:
				return FALSE;
			}
			
			s++;
		}
	    /* convert Y-M-D H:N:S.f to signed int */
	    *a = sign * ((((((((((Y * 12) + M) * 30) + D) * 24) + H) * 60) + N) * 60) + S);
	}

	return TRUE;
}


/***************************************************************************************/

ONVIF_RET parse_MulticastConfiguration(XMLN * p_node, onvif_MulticastConfiguration * p_req)
{
	XMLN * p_Multicast;
	XMLN * p_Address;
	XMLN * p_Port;
	XMLN * p_TTL;
	XMLN * p_AutoStart;
	
	p_Multicast = xml_node_soap_get(p_node, "Multicast");
    if (NULL == p_Multicast)
    {
        return ONVIF_ERR_MISSINGATTR;
    }

    p_Address = xml_node_soap_get(p_Multicast, "Address");
    if (p_Address)
    {
    	XMLN * p_IPv4Address;
    	
        p_IPv4Address = xml_node_soap_get(p_Address, "IPv4Address");
	    if (p_IPv4Address && p_IPv4Address->data)
	    {
	        strncpy(p_req->IPv4Address, p_IPv4Address->data, sizeof(p_req->IPv4Address)-1);
	    }
    }

    p_Port = xml_node_soap_get(p_Multicast, "Port");
    if (p_Port && p_Port->data)
    {
    	p_req->Port = atoi(p_Port->data);
    }

    p_TTL = xml_node_soap_get(p_Multicast, "TTL");
    if (p_TTL && p_TTL->data)
    {
    	p_req->TTL = atoi(p_TTL->data);
    }

    p_AutoStart = xml_node_soap_get(p_Multicast, "AutoStart");
    if (p_AutoStart && p_AutoStart->data)
    {
    	p_req->AutoStart = parse_Bool(p_AutoStart->data);
    }

    return ONVIF_OK;
}

ONVIF_RET parse_SetVideoEncoderConfiguration(XMLN * p_node, SetVideoEncoderConfiguration_REQ * p_req)
{
    XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_Encoding;
	XMLN * p_Resolution;
	XMLN * p_Quality;
	XMLN * p_RateControl;
	XMLN * p_SessionTimeout;
	XMLN * p_ForcePersistence;
	const char * token;

	p_Configuration = xml_node_soap_get(p_node, "Configuration");
    if (NULL == p_Configuration)
    {
        return ONVIF_ERR_MISSINGATTR;
    }
    
    token = xml_attr_get(p_Configuration, "token");
    if (token)
    {
        strncpy(p_req->Configuration.token, token, sizeof(p_req->Configuration.token)-1);
    }    

    p_Name = xml_node_soap_get(p_Configuration, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
    }

    p_UseCount = xml_node_soap_get(p_Configuration, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_req->Configuration.UseCount = atoi(p_UseCount->data);
    }

    p_Encoding = xml_node_soap_get(p_Configuration, "Encoding");
    if (p_Encoding && p_Encoding->data)
    {
		p_req->Configuration.Encoding = onvif_StringToVideoEncoding(p_Encoding->data);
    }

    p_Resolution = xml_node_soap_get(p_Configuration, "Resolution");
    if (p_Resolution)
    {
        XMLN * p_Width;
		XMLN * p_Height;

		p_Width = xml_node_soap_get(p_Resolution, "Width");
	    if (p_Width && p_Width->data)
	    {
	        p_req->Configuration.Resolution.Width = atoi(p_Width->data);
	    }

	    p_Height = xml_node_soap_get(p_Resolution, "Height");
	    if (p_Height && p_Height->data)
	    {
	        p_req->Configuration.Resolution.Height = atoi(p_Height->data);
	    }
    }

    p_Quality = xml_node_soap_get(p_Configuration, "Quality");
    if (p_Quality && p_Quality->data)
    {
        p_req->Configuration.Quality = atoi(p_Quality->data);
    }

    p_RateControl = xml_node_soap_get(p_Configuration, "RateControl");
    if (p_RateControl)
    {
    	XMLN * p_FrameRateLimit;
		XMLN * p_EncodingInterval;
		XMLN * p_BitrateLimit;
		
		p_req->Configuration.RateControlFlag = 1;
    	
        p_FrameRateLimit = xml_node_soap_get(p_RateControl, "FrameRateLimit");
	    if (p_FrameRateLimit && p_FrameRateLimit->data)
	    {
	        p_req->Configuration.RateControl.FrameRateLimit = atoi(p_FrameRateLimit->data);
	    }

	    p_EncodingInterval = xml_node_soap_get(p_RateControl, "EncodingInterval");
	    if (p_EncodingInterval && p_EncodingInterval->data)
	    {
	        p_req->Configuration.RateControl.EncodingInterval = atoi(p_EncodingInterval->data);
	    }

	    p_BitrateLimit = xml_node_soap_get(p_RateControl, "BitrateLimit");
	    if (p_BitrateLimit && p_BitrateLimit->data)
	    {
	        p_req->Configuration.RateControl.BitrateLimit = atoi(p_BitrateLimit->data);
	    }
    }
    
    if (p_req->Configuration.Encoding == VideoEncoding_H264)
    {
    	XMLN * p_H264 = xml_node_soap_get(p_Configuration, "H264");
    	if (p_H264)
    	{
    		XMLN * p_GovLength;
			XMLN * p_H264Profile;
			
    		p_req->Configuration.H264Flag = 1;
    		
    		p_GovLength = xml_node_soap_get(p_H264, "GovLength");
		    if (p_GovLength && p_GovLength->data)
		    {
		        p_req->Configuration.H264.GovLength = atoi(p_GovLength->data);
		    }

		    p_H264Profile = xml_node_soap_get(p_H264, "H264Profile");
		    if (p_H264Profile && p_H264Profile->data)
		    {
				p_req->Configuration.H264.H264Profile = onvif_StringToH264Profile(p_H264Profile->data);
		    }
    	}
    }
    else if (p_req->Configuration.Encoding == VideoEncoding_MPEG4)
    {
    	XMLN * p_MPEG4 = xml_node_soap_get(p_Configuration, "MPEG4");
    	if (p_MPEG4)
    	{
    		XMLN * p_GovLength;
			XMLN * p_Mpeg4Profile;
			
    		p_req->Configuration.MPEG4Flag = 1;
    		
    		p_GovLength = xml_node_soap_get(p_MPEG4, "GovLength");
		    if (p_GovLength && p_GovLength->data)
		    {
		        p_req->Configuration.MPEG4.GovLength = atoi(p_GovLength->data);
		    }

		    p_Mpeg4Profile = xml_node_soap_get(p_MPEG4, "Mpeg4Profile");
		    if (p_Mpeg4Profile && p_Mpeg4Profile->data)
		    {
				p_req->Configuration.MPEG4.Mpeg4Profile = onvif_StringToMpeg4Profile(p_Mpeg4Profile->data);
		    }
    	}
    }	

	parse_MulticastConfiguration(p_Configuration, &p_req->Configuration.Multicast);
	
	p_SessionTimeout = xml_node_soap_get(p_Configuration, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
		parse_XSDDuration(p_SessionTimeout->data, &p_req->Configuration.SessionTimeout);
	}

	p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}
    	
    return ONVIF_OK;
}


ONVIF_RET parse_SetSystemDateAndTime(XMLN * p_node, SetSystemDateAndTime_REQ * p_req)
{
    XMLN * p_DateTimeType;
	XMLN * p_DaylightSavings;
	XMLN * p_TimeZone;
	XMLN * p_UTCDateTime;

	p_DateTimeType = xml_node_soap_get(p_node, "DateTimeType");
    if (p_DateTimeType && p_DateTimeType->data)
    {
        p_req->SystemDateTime.DateTimeType = onvif_StringToSetDateTimeType(p_DateTimeType->data);
    }    

    p_DaylightSavings = xml_node_soap_get(p_node, "DaylightSavings");
    if (p_DaylightSavings && p_DaylightSavings->data)
    {
        p_req->SystemDateTime.DaylightSavings = parse_Bool(p_DaylightSavings->data);
    }

    p_TimeZone = xml_node_soap_get(p_node, "TimeZone");
    if (p_TimeZone)
    {
    	XMLN * p_TZ;
		
    	p_req->SystemDateTime.TimeZoneFlag = 1;
    	
        p_TZ = xml_node_soap_get(p_TimeZone, "TZ");
		if (p_TZ && p_TZ->data)
		{
			strncpy(p_req->SystemDateTime.TimeZone.TZ, p_TZ->data, sizeof(p_req->SystemDateTime.TimeZone.TZ)-1);
		}		
    }    

    p_UTCDateTime = xml_node_soap_get(p_node, "UTCDateTime");
    if (p_UTCDateTime)
    {
    	XMLN * p_Time;
		XMLN * p_Hour;
		XMLN * p_Minute;
		XMLN * p_Second;
		XMLN * p_Date;
		XMLN * p_Year;
		XMLN * p_Month;
		XMLN * p_Day;
		
    	p_req->UTCDateTimeFlag = 1;
    	
        p_Time = xml_node_soap_get(p_UTCDateTime, "Time");
	    if (!p_Time)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }

	    p_Hour = xml_node_soap_get(p_Time, "Hour");
	    if (!p_Hour || !p_Hour->data)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }
	    p_req->UTCDateTime.Time.Hour = atoi(p_Hour->data);

	    p_Minute = xml_node_soap_get(p_Time, "Minute");
	    if (!p_Minute || !p_Minute->data)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }
	    p_req->UTCDateTime.Time.Minute = atoi(p_Minute->data);

	    p_Second = xml_node_soap_get(p_Time, "Second");
	    if (!p_Second || !p_Second->data)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }
	    p_req->UTCDateTime.Time.Second = atoi(p_Second->data);

	    p_Date = xml_node_soap_get(p_UTCDateTime, "Date");
	    if (!p_Date)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }

	    p_Year = xml_node_soap_get(p_Date, "Year");
	    if (!p_Year || !p_Year->data)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }
	    p_req->UTCDateTime.Date.Year = atoi(p_Year->data);

	    p_Month = xml_node_soap_get(p_Date, "Month");
	    if (!p_Month || !p_Month->data)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }
	    p_req->UTCDateTime.Date.Month = atoi(p_Month->data);

	    p_Day = xml_node_soap_get(p_Date, "Day");
	    if (!p_Day || !p_Day->data)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }
	    p_req->UTCDateTime.Date.Day = atoi(p_Day->data);
    }    

    return ONVIF_OK;
}

ONVIF_RET parse_AddScopes(XMLN * p_AddScopes, ONVIF_SCOPE * p_scope, int scope_max)
{
	int i = 0;
	
	XMLN * p_ScopeItem = xml_node_soap_get(p_AddScopes, "ScopeItem");
	while (p_ScopeItem)
	{
		if (i < scope_max)
		{
			p_scope[i].fixed = FALSE;
			strncpy(p_scope[i].scope, p_ScopeItem->data, sizeof(p_scope[i].scope)-1);

			++i;
		}
		else
		{
			return ONVIF_ERR_TOO_MANY_SCOPES;
		}
		
		p_ScopeItem = p_ScopeItem->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SetScopes(XMLN * p_AddScopes, ONVIF_SCOPE * p_scope, int scope_max)
{
	int i = 0;
	
	XMLN * p_Scopes = xml_node_soap_get(p_AddScopes, "Scopes");
	while (p_Scopes)
	{
		if (i < scope_max)
		{
			p_scope[i].fixed = FALSE;
			strncpy(p_scope[i].scope, p_Scopes->data, sizeof(p_scope[i].scope)-1);

			++i;
		}
		else
		{
			return ONVIF_ERR_TOO_MANY_SCOPES;
		}
		
		p_Scopes = p_Scopes->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SetDiscoveryMode(XMLN * p_node, SetDiscoveryMode_REQ * p_req)
{
	XMLN * p_DiscoveryMode = xml_node_soap_get(p_node, "DiscoveryMode");
	if (p_DiscoveryMode && p_DiscoveryMode->data)
	{
		p_req->DiscoveryMode = onvif_StringToDiscoveryMode(p_DiscoveryMode->data);	
	}

	return ONVIF_OK;
}

ONVIF_RET parse_Filter(XMLN * p_node, ONVIF_FILTER * p_req)
{
    int i = 0;
    XMLN * p_TopicExpression;
    XMLN * p_MessageContent;

    p_TopicExpression = xml_node_soap_get(p_node, "TopicExpression");
    while (p_TopicExpression && soap_strcmp(p_TopicExpression->name, "TopicExpression") == 0)
    {
        if (p_TopicExpression->data && i < 10)
        {
            strncpy(p_req->TopicExpression[i], p_TopicExpression->data, sizeof(p_req->TopicExpression[i])-1);
            i++;
        }

        p_TopicExpression = p_TopicExpression->next;
    }

    i = 0;

    p_MessageContent = xml_node_soap_get(p_node, "MessageContent");
    while (p_MessageContent && soap_strcmp(p_MessageContent->name, "MessageContent") == 0)
    {
        if (p_MessageContent->data && i < 10)
        {
            strncpy(p_req->MessageContent[i], p_MessageContent->data, sizeof(p_req->MessageContent[i])-1);
            i++;
        }

        p_MessageContent = p_MessageContent->next;
    }

    return ONVIF_OK;
}

ONVIF_RET parse_Subscribe(XMLN * p_node, Subscribe_REQ * p_req)
{
	XMLN * p_ConsumerReference;
	XMLN * p_Address;
	XMLN * p_InitialTerminationTime;
	XMLN * p_Filter;
	
	p_ConsumerReference = xml_node_soap_get(p_node, "ConsumerReference");
	if (NULL == p_ConsumerReference)
	{
		return ONVIF_ERR_MISSINGATTR;		
	}

	p_Address = xml_node_soap_get(p_ConsumerReference, "Address");
	if (p_Address && p_Address->data)
	{
		strncpy(p_req->ConsumerReference, p_Address->data, sizeof(p_req->ConsumerReference)-1);
	}	

	p_InitialTerminationTime = xml_node_soap_get(p_node, "InitialTerminationTime");
	if (p_InitialTerminationTime && p_InitialTerminationTime->data)
	{
		p_req->InitialTerminationTimeFlag = 1;
		parse_XSDDuration(p_InitialTerminationTime->data, &p_req->InitialTerminationTime);
	}

    p_Filter = xml_node_soap_get(p_node, "Filter");
	if (p_Filter)
	{
	    p_req->FiltersFlag = 1;
		parse_Filter(p_Filter, &p_req->Filters);
	}
    
	return ONVIF_OK;
}

ONVIF_RET parse_Renew(XMLN * p_node, Renew_REQ * p_req)
{
	XMLN * p_TerminationTime;
	
	p_TerminationTime = xml_node_soap_get(p_node, "TerminationTime");
	if (p_TerminationTime && p_TerminationTime->data)
	{
		parse_XSDDuration(p_TerminationTime->data, &p_req->TerminationTime);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_CreatePullPointSubscription(XMLN * p_node, CreatePullPointSubscription_REQ * p_req)
{
	XMLN * p_InitialTerminationTime;
	XMLN * p_Filter;

	p_InitialTerminationTime = xml_node_soap_get(p_node, "InitialTerminationTime");
	if (p_InitialTerminationTime && p_InitialTerminationTime->data)
	{
		p_req->InitialTerminationTimeFlag = 1;
		parse_XSDDuration(p_InitialTerminationTime->data, &p_req->InitialTerminationTime);
	}

    p_Filter = xml_node_soap_get(p_node, "Filter");
	if (p_Filter)
	{
	    p_req->FiltersFlag = 1;
		parse_Filter(p_Filter, &p_req->Filters);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_PullMessages(XMLN * p_node, PullMessages_REQ * p_req)
{
	XMLN * p_Timeout;
	XMLN * p_MessageLimit;

	p_Timeout = xml_node_soap_get(p_node, "Timeout");
	if (p_Timeout && p_Timeout->data)
	{
		parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
	}

	p_MessageLimit = xml_node_soap_get(p_node, "MessageLimit");
	if (p_MessageLimit && p_MessageLimit->data)
	{
		p_req->MessageLimit = atoi(p_MessageLimit->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_AddPTZConfiguration(XMLN * p_node, AddPTZConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;
	
	assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_Vector(XMLN * p_node, onvif_Vector * p_req)
{
	const char * p_x;
	const char * p_y;
	
	p_x = xml_attr_get(p_node, "x");
	if (p_x)
	{			
		p_req->x = (float)atof(p_x);
	}

	p_y = xml_attr_get(p_node, "y");
	if (p_y)
	{
		p_req->y = (float)atof(p_y);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_Vector1D(XMLN * p_node, onvif_Vector1D * p_req)
{
	const char * p_x;
	
	p_x = xml_attr_get(p_node, "x");
	if (p_x)
	{			
		p_req->x = (float)atof(p_x);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_PTZSpeed(XMLN * p_node, onvif_PTZSpeed * p_req)
{
	XMLN * p_PanTilt;
	XMLN * p_Zoom;

	p_PanTilt = xml_node_soap_get(p_node, "PanTilt");
	if (p_PanTilt)
	{
		p_req->PanTiltFlag = 1;		
		parse_Vector(p_PanTilt, &p_req->PanTilt);
	}

	p_Zoom = xml_node_soap_get(p_node, "Zoom");
	if (p_Zoom)
	{
		p_req->ZoomFlag = 1;		
		parse_Vector1D(p_Zoom, &p_req->Zoom);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_PTZVector(XMLN * p_node, onvif_PTZVector * p_req)
{
	XMLN * p_PanTilt;
	XMLN * p_Zoom;

	p_PanTilt = xml_node_soap_get(p_node, "PanTilt");
	if (p_PanTilt)
	{	
		p_req->PanTiltFlag = 1;		
		parse_Vector(p_PanTilt, &p_req->PanTilt);
	}

	p_Zoom = xml_node_soap_get(p_node, "Zoom");
	if (p_Zoom)
	{	
		p_req->ZoomFlag = 1;		
		parse_Vector1D(p_Zoom, &p_req->Zoom);
	}

	return ONVIF_OK;
}


ONVIF_RET parse_ContinuousMove(XMLN * p_node, ContinuousMove_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_Velocity;
	XMLN * p_Timeout;
	
	assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	p_Velocity = xml_node_soap_get(p_node, "Velocity");
	if (p_Velocity)
	{	
		parse_PTZSpeed(p_Velocity, &p_req->Velocity);
	}

	p_Timeout = xml_node_soap_get(p_node, "Timeout");
	if (p_Timeout && p_Timeout->data)
	{
		p_req->TimeoutFlag = parse_XSDDuration(p_Timeout->data, &p_req->Timeout);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_PTZ_Stop(XMLN * p_node, PTZ_Stop_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_PanTilt;
	XMLN * p_Zoom;
	
	assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	p_PanTilt = xml_node_soap_get(p_node, "PanTilt");
	if (p_PanTilt && p_PanTilt->data)
	{
		p_req->PanTiltFlag = 1;
		p_req->PanTilt = parse_Bool(p_PanTilt->data);
	}

	p_Zoom = xml_node_soap_get(p_node, "Zoom");
	if (p_Zoom && p_Zoom->data)
	{
		p_req->ZoomFlag = 1;
		p_req->Zoom = parse_Bool(p_Zoom->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_AbsoluteMove(XMLN * p_node, AbsoluteMove_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_Position;
	XMLN * p_Speed;
	
    assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

    p_Position = xml_node_soap_get(p_node, "Position");
	if (p_Position)
	{	
	    parse_PTZVector(p_Position, &p_req->Position);
	}

	p_Speed = xml_node_soap_get(p_node, "Speed");
	if (p_Speed)
	{	
		p_req->SpeedFlag = 1;		
	    parse_PTZSpeed(p_Speed, &p_req->Speed);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_RelativeMove(XMLN * p_node, RelativeMove_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_Translation;
	XMLN * p_Speed;
	
    assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

    p_Translation = xml_node_soap_get(p_node, "Translation");
	if (p_Translation)
	{	
	    parse_PTZVector(p_Translation, &p_req->Translation);
	}

	p_Speed = xml_node_soap_get(p_node, "Speed");
	if (p_Speed)
	{	
		p_req->SpeedFlag = 1;		
	    parse_PTZSpeed(p_Speed, &p_req->Speed);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SetPreset(XMLN * p_node, SetPreset_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_PresetName;
	XMLN * p_PresetToken;
	
    assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_PresetName = xml_node_soap_get(p_node, "PresetName");
	if (p_PresetName && p_PresetName->data)
	{
		p_req->PresetNameFlag = 1;
		strncpy(p_req->PresetName, p_PresetName->data, sizeof(p_req->PresetName)-1);
	}

	p_PresetToken = xml_node_soap_get(p_node, "PresetToken");
	if (p_PresetToken && p_PresetToken->data)
	{
		p_req->PresetTokenFlag = 1;
		strncpy(p_req->PresetToken, p_PresetToken->data, sizeof(p_req->PresetToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_RemovePreset(XMLN * p_node, RemovePreset_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_PresetToken;
	
	assert(p_node);

    p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_PresetToken = xml_node_soap_get(p_node, "PresetToken");
	if (p_PresetToken && p_PresetToken->data)
	{
		strncpy(p_req->PresetToken, p_PresetToken->data, sizeof(p_req->PresetToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GotoPreset(XMLN * p_node, GotoPreset_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_PresetToken;
	XMLN * p_Speed;
	
	assert(p_node);

    p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_PresetToken = xml_node_soap_get(p_node, "PresetToken");
	if (p_PresetToken && p_PresetToken->data)
	{
		strncpy(p_req->PresetToken, p_PresetToken->data, sizeof(p_req->PresetToken)-1);
	}

	p_Speed = xml_node_soap_get(p_node, "Speed");
	if (p_Speed)
	{	
		p_req->SpeedFlag = 1;		
	    parse_PTZSpeed(p_Speed, &p_req->Speed);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GotoHomePosition(XMLN * p_node, GotoHomePosition_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_Speed;
	
    assert(p_node);

    p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_Speed = xml_node_soap_get(p_node, "Speed");
	if (p_Speed)
	{	
		p_req->SpeedFlag = 1;
	    parse_PTZSpeed(p_Speed, &p_req->Speed);
	}

	return ONVIF_OK;
}


ONVIF_RET parse_SetDNS(XMLN * p_node, SetDNS_REQ * p_req)
{
	int i = 0;
	XMLN * p_FromDHCP;
	XMLN * p_SearchDomain;
	XMLN * p_DNSManual;
	
	assert(p_node);

	p_FromDHCP = xml_node_soap_get(p_node, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		p_req->DNSInformation.FromDHCP = parse_Bool(p_FromDHCP->data);
	}	
	
	p_SearchDomain = xml_node_soap_get(p_node, "SearchDomain");
	while (p_SearchDomain && soap_strcmp(p_SearchDomain->name, "SearchDomain") == 0)
	{
		p_req->DNSInformation.SearchDomainFlag = 1;
		
		if (p_SearchDomain->data && i < MAX_SEARCHDOMAIN)
		{
			strncpy(p_req->DNSInformation.SearchDomain[i], p_SearchDomain->data, sizeof(p_req->DNSInformation.SearchDomain[i])-1);
			++i;
		}

		p_SearchDomain = p_SearchDomain->next;
	}

	i = 0;
	
	p_DNSManual = xml_node_soap_get(p_node, "DNSManual");
	while (p_DNSManual && soap_strcmp(p_DNSManual->name, "DNSManual") == 0)
	{
		XMLN * p_Type;
		XMLN * p_IPv4Address;
		
		p_Type = xml_node_soap_get(p_DNSManual, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") != 0) // todo : now only support ipv4
			{
				p_DNSManual = p_DNSManual->next;
				continue;
			}
		}

		p_IPv4Address = xml_node_soap_get(p_DNSManual, "IPv4Address");
		if (p_IPv4Address && p_IPv4Address->data)
		{
			if (is_ip_address(p_IPv4Address->data) == FALSE)
			{
				return ONVIF_ERR_INVALID_IPV4_ADDR;
			}
			else if (i < MAX_DNS_SERVER)
			{
				strncpy(p_req->DNSInformation.DNSServer[i], p_IPv4Address->data, sizeof(p_req->DNSInformation.DNSServer[i])-1);
				++i;
			}
		}
		
		p_DNSManual = p_DNSManual->next;
	}

	return ONVIF_OK;	
}

ONVIF_RET parse_SetNTP(XMLN * p_node, SetNTP_REQ * p_req)
{
	int i = 0;
	XMLN * p_FromDHCP;
	XMLN * p_NTPManual;
	
	assert(p_node);

	p_FromDHCP = xml_node_soap_get(p_node, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		p_req->NTPInformation.FromDHCP = parse_Bool(p_FromDHCP->data);
	}	
	
	p_NTPManual = xml_node_soap_get(p_node, "NTPManual");
	while (p_NTPManual && soap_strcmp(p_NTPManual->name, "NTPManual") == 0)
	{
		XMLN * p_Type;
		XMLN * p_IPv4Address;
		XMLN * p_DNSname;
		
		p_Type = xml_node_soap_get(p_NTPManual, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") != 0 && strcasecmp(p_Type->data, "DNS") != 0) // todo : now only support ipv4
			{
				p_NTPManual = p_NTPManual->next;
				continue;
			}
		}

		p_IPv4Address = xml_node_soap_get(p_NTPManual, "IPv4Address");
		if (p_IPv4Address && p_IPv4Address->data)
		{
			if (is_ip_address(p_IPv4Address->data) == FALSE)
			{
				return ONVIF_ERR_INVALID_IPV4_ADDR;
			}
			else if (i < MAX_NTP_SERVER)
			{
				strncpy(p_req->NTPInformation.NTPServer[i], p_IPv4Address->data, sizeof(p_req->NTPInformation.NTPServer[i])-1);
				++i;
			}
		}

		p_DNSname = xml_node_soap_get(p_NTPManual, "DNSname");
		if (p_DNSname && p_DNSname->data)
		{
			if (i < MAX_NTP_SERVER)
			{
				strncpy(p_req->NTPInformation.NTPServer[i], p_DNSname->data, sizeof(p_req->NTPInformation.NTPServer[i])-1);
				++i;
			}
		}
		
		p_NTPManual = p_NTPManual->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SetNetworkProtocols(XMLN * p_node, SetNetworkProtocols_REQ * p_req)
{
	char name[32];
	BOOL enable;
	int  port[MAX_SERVER_PORT];
	XMLN * p_NetworkProtocols;

	assert(p_node);
	
	p_NetworkProtocols = xml_node_soap_get(p_node, "NetworkProtocols");
	while (p_NetworkProtocols && soap_strcmp(p_NetworkProtocols->name, "NetworkProtocols") == 0)
	{
		int i = 0;
		XMLN * p_Name;
		XMLN * p_Enabled;
		XMLN * p_Port;
		
		enable = FALSE;
		memset(name, 0, sizeof(name));
		memset(port, 0, sizeof(int)*MAX_SERVER_PORT);
		
		p_Name = xml_node_soap_get(p_NetworkProtocols, "Name");
		if (p_Name && p_Name->data)
		{
			strncpy(name, p_Name->data, sizeof(name)-1);
		}

		p_Enabled = xml_node_soap_get(p_NetworkProtocols, "Enabled");
		if (p_Enabled && p_Enabled->data)
		{
			if (strcasecmp(p_Enabled->data, "true") == 0)
			{
				enable = TRUE;
			}
		}		
		
		p_Port = xml_node_soap_get(p_NetworkProtocols, "Port");
		while (p_Port && p_Port->data && soap_strcmp(p_Port->name, "Port") == 0)
		{
			if (i < MAX_SERVER_PORT)
			{
				port[i++] = atoi(p_Port->data);
			}
			
			p_Port = p_Port->next;
		}

		if (strcasecmp(name, "HTTP") == 0)
		{
			p_req->NetworkProtocol.HTTPFlag = 1;
			p_req->NetworkProtocol.HTTPEnabled = enable;
			memcpy(p_req->NetworkProtocol.HTTPPort, port, sizeof(int)*MAX_SERVER_PORT);
		}
		else if (strcasecmp(name, "HTTPS") == 0)
		{
			p_req->NetworkProtocol.HTTPSFlag = 1;
			p_req->NetworkProtocol.HTTPSEnabled = enable;
			memcpy(p_req->NetworkProtocol.HTTPSPort, port, sizeof(int)*MAX_SERVER_PORT);
		}
		else if (strcasecmp(name, "RTSP") == 0)
		{
			p_req->NetworkProtocol.RTSPFlag = 1;
			p_req->NetworkProtocol.RTSPEnabled = enable;
			memcpy(p_req->NetworkProtocol.RTSPPort, port, sizeof(int)*MAX_SERVER_PORT);
		}
		else
		{
			return ONVIF_ERR_SERVICE_NOT_SUPPORT;
		}

		p_NetworkProtocols = p_NetworkProtocols->next;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_SetNetworkDefaultGateway(XMLN * p_node, SetNetworkDefaultGateway_REQ * p_req)
{
	int i = 0;
	XMLN * p_IPv4Address;
	
	assert(p_node);	
	
	p_IPv4Address = xml_node_soap_get(p_node, "IPv4Address");
	while (p_IPv4Address && p_IPv4Address->data && soap_strcmp(p_IPv4Address->name, "IPv4Address") == 0)
	{
		if (is_ip_address(p_IPv4Address->data) == FALSE)
		{
			return ONVIF_ERR_INVALID_IPV4_ADDR;
		}

		if (i < MAX_GATEWAY)
		{
			strncpy(p_req->IPv4Address[i++], p_IPv4Address->data, sizeof(p_req->IPv4Address[0])-1);
		}

		p_IPv4Address = p_IPv4Address->next;
	}

	return ONVIF_OK;
}


ONVIF_RET parse_CreateProfile(XMLN * p_node, CreateProfile_REQ * p_req)
{
	XMLN * p_Name;
	XMLN * p_Token;
	
	assert(p_node);

	p_Name = xml_node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	p_Token = xml_node_soap_get(p_node, "Token");
	if (p_Token && p_Token->data)
	{
		p_req->TokenFlag = 1;
		strncpy(p_req->Token, p_Token->data, sizeof(p_req->Token)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_AddVideoSourceConfiguration(XMLN * p_node, AddVideoSourceConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;
	
	assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_AddVideoEncoderConfiguration(XMLN * p_node, AddVideoEncoderConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;
	
	assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	return ONVIF_OK;
}


ONVIF_RET parse_AddAudioSourceConfiguration(XMLN * p_node, AddAudioSourceConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;
	
	assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_AddAudioEncoderConfiguration(XMLN * p_node, AddAudioEncoderConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;
	
	assert(p_node);

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_StreamSetup(XMLN * p_node, onvif_StreamSetup * p_req)
{
	XMLN * p_Stream;
	XMLN * p_Transport;

	p_Stream = xml_node_soap_get(p_node, "Stream");
	if (p_Stream && p_Stream->data)
	{
		p_req->Stream = onvif_StringToStreamType(p_Stream->data);
		if (StreamType_Invalid == p_req->Stream)
		{
			return ONVIF_ERR_INVALID_STREAMSETUP;
		}
	}

	p_Transport = xml_node_soap_get(p_node, "Transport");
	if (p_Transport)
	{
		XMLN * p_Protocol = xml_node_soap_get(p_Transport, "Protocol");
		if (p_Protocol && p_Protocol->data)
		{
			p_req->Transport.Protocol = onvif_StringToTransportProtocol(p_Protocol->data);
			if (TransportProtocol_Invalid == p_req->Transport.Protocol)
			{
				return ONVIF_ERR_INVALID_STREAMSETUP;
			}
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetStreamUri(XMLN * p_node, GetStreamUri_REQ * p_req)
{
	ONVIF_RET ret = ONVIF_OK;
	XMLN * p_StreamSetup;
	XMLN * p_ProfileToken;

	p_StreamSetup = xml_node_soap_get(p_node, "StreamSetup");
	if (p_StreamSetup)
	{
		ret = parse_StreamSetup(p_StreamSetup, &p_req->StreamSetup);		
	}
	
	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	return ret;
}


ONVIF_RET parse_SetNetworkInterfaces(XMLN * p_node, SetNetworkInterfaces_REQ * p_req)
{
    XMLN * p_InterfaceToken;
	XMLN * p_NetworkInterface;

	p_InterfaceToken = xml_node_soap_get(p_node, "InterfaceToken");
	if (p_InterfaceToken && p_InterfaceToken->data)
	{
	    strncpy(p_req->NetworkInterface.token, p_InterfaceToken->data, sizeof(p_req->NetworkInterface.token)-1);
	}
	else
	{
	    return ONVIF_ERR_MISSINGATTR;
	}

	p_NetworkInterface = xml_node_soap_get(p_node, "NetworkInterface");
	if (p_NetworkInterface)
	{
		XMLN * p_Enabled;
		XMLN * p_MTU;
		XMLN * p_IPv4;
		
	    p_req->NetworkInterface.Enabled = TRUE;
	    
	    p_Enabled = xml_node_soap_get(p_NetworkInterface, "Enabled");
	    if (p_Enabled && p_Enabled->data)
	    {
	        p_req->NetworkInterface.Enabled = parse_Bool(p_Enabled->data);
	    }

	    p_MTU = xml_node_soap_get(p_NetworkInterface, "MTU");
	    if (p_MTU && p_MTU->data)
	    {
	    	p_req->NetworkInterface.InfoFlag = 1;
	    	p_req->NetworkInterface.Info.MTUFlag = 1;
	        p_req->NetworkInterface.Info.MTU = atoi(p_MTU->data);
	    }

	    p_IPv4 = xml_node_soap_get(p_NetworkInterface, "IPv4");
	    if (p_IPv4)
	    {
	    	XMLN * p_Enabled;
			XMLN * p_DHCP;
			
	        p_req->NetworkInterface.IPv4Flag = 1;
	        
	        p_Enabled = xml_node_soap_get(p_IPv4, "Enabled");
	        if (p_Enabled && p_Enabled->data)
    	    {
    	        p_req->NetworkInterface.IPv4.Enabled = parse_Bool(p_Enabled->data);
    	    }

    	    p_DHCP = xml_node_soap_get(p_IPv4, "DHCP");
	        if (p_DHCP && p_DHCP->data)
	        {
	            p_req->NetworkInterface.IPv4.Config.DHCP = parse_Bool(p_DHCP->data);
	        }

	        if (p_req->NetworkInterface.IPv4.Config.DHCP == FALSE)
	        {
	            XMLN * p_Manual = xml_node_soap_get(p_IPv4, "Manual");
	            if (p_Manual)
	            {
	                XMLN * p_Address;
					XMLN * p_PrefixLength;

					p_Address = xml_node_soap_get(p_Manual, "Address");
	                if (p_Address && p_Address->data)
	                {
	                    strncpy(p_req->NetworkInterface.IPv4.Config.Address, p_Address->data, sizeof(p_req->NetworkInterface.IPv4.Config.Address)-1);
	                }

	                p_PrefixLength = xml_node_soap_get(p_Manual, "PrefixLength");
	                if (p_PrefixLength && p_PrefixLength->data)
	                {
	                    p_req->NetworkInterface.IPv4.Config.PrefixLength = atoi(p_PrefixLength->data);
	                }
	            }
	        }
	    }
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetVideoSourceConfigurationOptions(XMLN * p_node, GetVideoSourceConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;
	
	assert(p_node);

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	
	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_SetVideoSourceConfiguration(XMLN * p_node, SetVideoSourceConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_SourceToken;
	XMLN * p_Bounds;
	XMLN * p_ForcePersistence;
	
	assert(p_node);

	p_Configuration = xml_node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
		const char * token = xml_attr_get(p_Configuration, "token");
		if (token)
		{
			strncpy(p_req->Configuration.token, token, sizeof(p_req->Configuration.token)-1);
		}
		else
		{
			return ONVIF_ERR_MISSINGATTR;
		}
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}

	p_Name = xml_node_soap_get(p_Configuration, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
	}

	p_UseCount = xml_node_soap_get(p_Configuration, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		p_req->Configuration.UseCount = atoi(p_UseCount->data);
	}

	p_SourceToken = xml_node_soap_get(p_Configuration, "SourceToken");
	if (p_SourceToken && p_SourceToken->data)
	{
		strncpy(p_req->Configuration.SourceToken, p_SourceToken->data, sizeof(p_req->Configuration.SourceToken)-1);
	}

	p_Bounds = xml_node_soap_get(p_Configuration, "Bounds");
	if (p_Bounds)
	{
		const char * p_x;
		const char * p_y;
		const char * p_width;
		const char * p_height;

		p_x = xml_attr_get(p_Bounds, "x");
		if (p_x)
		{
			p_req->Configuration.Bounds.x = atoi(p_x);
		}

		p_y = xml_attr_get(p_Bounds, "y");
		if (p_y)
		{
			p_req->Configuration.Bounds.y = atoi(p_y);
		}

		p_width = xml_attr_get(p_Bounds, "width");
		if (p_width)
		{
			p_req->Configuration.Bounds.width = atoi(p_width);
		}

		p_height = xml_attr_get(p_Bounds, "height");
		if (p_height)
		{
			p_req->Configuration.Bounds.height = atoi(p_height);
		}
	}

	p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}	
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetVideoEncoderConfigurationOptions(XMLN * p_node, GetVideoEncoderConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;
	
	assert(p_node);

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	
	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetAudioSourceConfigurationOptions(XMLN * p_node, GetAudioSourceConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;
	
	assert(p_node);

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	
	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_SetAudioSourceConfiguration(XMLN * p_node, SetAudioSourceConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_SourceToken;
	XMLN * p_ForcePersistence;
	
	assert(p_node);

	p_Configuration = xml_node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
		const char * token = xml_attr_get(p_Configuration, "token");
		if (token)
		{
			strncpy(p_req->Configuration.token, token, sizeof(p_req->Configuration.token)-1);
		}
		else
		{
			return ONVIF_ERR_MISSINGATTR;
		}
	}
	else
	{
		return ONVIF_ERR_MISSINGATTR;
	}

	p_Name = xml_node_soap_get(p_Configuration, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
	}

	p_UseCount = xml_node_soap_get(p_Configuration, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		p_req->Configuration.UseCount = atoi(p_UseCount->data);
	}

	p_SourceToken = xml_node_soap_get(p_Configuration, "SourceToken");
	if (p_SourceToken && p_SourceToken->data)
	{
		strncpy(p_req->Configuration.SourceToken, p_SourceToken->data, sizeof(p_req->Configuration.SourceToken)-1);
	}

	p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}	
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetAudioEncoderConfigurationOptions(XMLN * p_node, GetAudioEncoderConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;
	
	assert(p_node);

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	
	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_SetAudioEncoderConfiguration(XMLN * p_node, SetAudioEncoderConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_Encoding;
	XMLN * p_Bitrate;
	XMLN * p_SampleRate;
	XMLN * p_SessionTimeout;
	XMLN * p_ForcePersistence;
	const char * token;

	p_Configuration = xml_node_soap_get(p_node, "Configuration");
    if (!p_Configuration)
    {
        return ONVIF_ERR_MISSINGATTR;
    }
    
    token = xml_attr_get(p_Configuration, "token");
    if (token)
    {
        strncpy(p_req->Configuration.token, token, sizeof(p_req->Configuration.token)-1);
    }    

    p_Name = xml_node_soap_get(p_Configuration, "Name");
    if (p_Name && p_Name->data)
    {
        strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
    }

    p_UseCount = xml_node_soap_get(p_Configuration, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        p_req->Configuration.UseCount = atoi(p_UseCount->data);
    }

    p_Encoding = xml_node_soap_get(p_Configuration, "Encoding");
    if (p_Encoding && p_Encoding->data)
    {
		p_req->Configuration.Encoding = onvif_StringToAudioEncoding(p_Encoding->data);
    }    

    p_Bitrate = xml_node_soap_get(p_Configuration, "Bitrate");
    if (p_Bitrate && p_Bitrate->data)
    {
        p_req->Configuration.Bitrate = atoi(p_Bitrate->data);
    }

    p_SampleRate = xml_node_soap_get(p_Configuration, "SampleRate");
    if (p_SampleRate && p_SampleRate->data)
    {
        p_req->Configuration.SampleRate = atoi(p_SampleRate->data);
    }    

	parse_MulticastConfiguration(p_Configuration, &p_req->Configuration.Multicast);
	
	p_SessionTimeout = xml_node_soap_get(p_Configuration, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
		parse_XSDDuration(p_SessionTimeout->data, &p_req->Configuration.SessionTimeout);
	}

	p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}
    	
    return ONVIF_OK;
}

ONVIF_RET parse_SetImagingSettings(XMLN * p_node, SetImagingSettings_REQ * p_req)
{
	XMLN * p_VideoSourceToken;
	XMLN * p_ImagingSettings;
	XMLN * p_BacklightCompensation;
	XMLN * p_Brightness;
	XMLN * p_ColorSaturation;
	XMLN * p_Contrast;
	XMLN * p_Exposure;
	XMLN * p_Focus;
	XMLN * p_IrCutFilter;
	XMLN * p_Sharpness;
	XMLN * p_WideDynamicRange;
	XMLN * p_WhiteBalance;
	XMLN * p_ForcePersistence;
	
	p_VideoSourceToken = xml_node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MISSINGATTR;
    }

    p_ImagingSettings = xml_node_soap_get(p_node, "ImagingSettings");
    if (NULL == p_ImagingSettings)
    {
    	return ONVIF_ERR_MISSINGATTR;
    }

    p_BacklightCompensation = xml_node_soap_get(p_ImagingSettings, "BacklightCompensation");
    if (p_BacklightCompensation)
    {
    	XMLN * p_Mode;
		XMLN * p_Level;

		p_req->ImagingSettings.BacklightCompensationFlag = 1;
		
		p_Mode = xml_node_soap_get(p_BacklightCompensation, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		p_req->ImagingSettings.BacklightCompensation.Mode = onvif_StringToBacklightCompensationMode(p_Mode->data);
    	}

    	p_Level = xml_node_soap_get(p_BacklightCompensation, "Level");
    	if (p_Level && p_Level->data)
    	{
    		p_req->ImagingSettings.BacklightCompensation.LevelFlag = 1;
    	    p_req->ImagingSettings.BacklightCompensation.Level = (float)atof(p_Level->data);
    	}
    }

    p_Brightness = xml_node_soap_get(p_ImagingSettings, "Brightness");
    if (p_Brightness && p_Brightness->data)
    {
    	p_req->ImagingSettings.BrightnessFlag = 1;
    	p_req->ImagingSettings.Brightness = (float)atof(p_Brightness->data);
    }

    p_ColorSaturation = xml_node_soap_get(p_ImagingSettings, "ColorSaturation");
    if (p_ColorSaturation && p_ColorSaturation->data)
    {
    	p_req->ImagingSettings.ColorSaturationFlag = 1;
    	p_req->ImagingSettings.ColorSaturation = (float)atof(p_ColorSaturation->data);
    }

    p_Contrast = xml_node_soap_get(p_ImagingSettings, "Contrast");
    if (p_Contrast && p_Contrast->data)
    {
    	p_req->ImagingSettings.ContrastFlag = 1;
    	p_req->ImagingSettings.Contrast = (float)atof(p_Contrast->data);
    }

    p_Exposure = xml_node_soap_get(p_ImagingSettings, "Exposure");
    if (p_Exposure)
    {
    	XMLN * p_Mode;
		XMLN * p_Priority;
		XMLN * p_MinExposureTime;
		XMLN * p_MaxExposureTime;
		XMLN * p_MinGain;
		XMLN * p_MaxGain;
		XMLN * p_MinIris;
		XMLN * p_MaxIris;
		XMLN * p_ExposureTime;
		XMLN * p_Gain;
		XMLN * p_Iris;

		p_req->ImagingSettings.ExposureFlag = 1;
		
		p_Mode = xml_node_soap_get(p_Exposure, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		p_req->ImagingSettings.Exposure.Mode = onvif_StringToExposureMode(p_Mode->data);
    	}

        p_Priority = xml_node_soap_get(p_Exposure, "Priority");
    	if (p_Priority && p_Priority->data)
    	{
    		p_req->ImagingSettings.Exposure.PriorityFlag = 1;
    		p_req->ImagingSettings.Exposure.Priority = onvif_StringToExposurePriority(p_Priority->data);
    	}
    	
    	p_MinExposureTime = xml_node_soap_get(p_Exposure, "MinExposureTime");
    	if (p_MinExposureTime && p_MinExposureTime->data)
    	{
    		p_req->ImagingSettings.Exposure.MinExposureTimeFlag = 1;
    		p_req->ImagingSettings.Exposure.MinExposureTime = (float)atof(p_MinExposureTime->data);
    	}

    	p_MaxExposureTime = xml_node_soap_get(p_Exposure, "MaxExposureTime");
    	if (p_MaxExposureTime && p_MaxExposureTime->data)
    	{
    		p_req->ImagingSettings.Exposure.MaxExposureTimeFlag = 1;
    		p_req->ImagingSettings.Exposure.MaxExposureTime = (float)atof(p_MaxExposureTime->data);
    	}

    	p_MinGain = xml_node_soap_get(p_Exposure, "MinGain");
    	if (p_MinGain && p_MinGain->data)
    	{
    		p_req->ImagingSettings.Exposure.MinGainFlag = 1;
    		p_req->ImagingSettings.Exposure.MinGain = (float)atof(p_MinGain->data);
    	}

    	p_MaxGain = xml_node_soap_get(p_Exposure, "MaxGain");
    	if (p_MaxGain && p_MaxGain->data)
    	{
    		p_req->ImagingSettings.Exposure.MaxGainFlag = 1;
    		p_req->ImagingSettings.Exposure.MaxGain = (float)atof(p_MaxGain->data);
    	}

    	p_MinIris = xml_node_soap_get(p_Exposure, "MinIris");
    	if (p_MinIris && p_MinIris->data)
    	{
    		p_req->ImagingSettings.Exposure.MinIrisFlag = 1;
    		p_req->ImagingSettings.Exposure.MinIris = (float)atof(p_MinIris->data);
    	}

    	p_MaxIris = xml_node_soap_get(p_Exposure, "MaxIris");
    	if (p_MaxIris && p_MaxIris->data)
    	{
    		p_req->ImagingSettings.Exposure.MaxIrisFlag = 1;
    		p_req->ImagingSettings.Exposure.MaxIris = (float)atof(p_MaxIris->data);
    	}

    	p_ExposureTime = xml_node_soap_get(p_Exposure, "ExposureTime");
    	if (p_ExposureTime && p_ExposureTime->data)
    	{
    		p_req->ImagingSettings.Exposure.ExposureTimeFlag = 1;
    		p_req->ImagingSettings.Exposure.ExposureTime = (float)atof(p_ExposureTime->data);
    	}

    	p_Gain = xml_node_soap_get(p_Exposure, "Gain");
    	if (p_Gain && p_Gain->data)
    	{
    		p_req->ImagingSettings.Exposure.GainFlag = 1;
    		p_req->ImagingSettings.Exposure.Gain = (float)atof(p_Gain->data);
    	}

    	p_Iris = xml_node_soap_get(p_Exposure, "Iris");
    	if (p_Iris && p_Iris->data)
    	{
    		p_req->ImagingSettings.Exposure.IrisFlag = 1;
    		p_req->ImagingSettings.Exposure.Iris = (float)atof(p_Iris->data);
    	}
    }

    p_Focus = xml_node_soap_get(p_ImagingSettings, "Focus");
    if (p_Focus)
    {
        XMLN * p_AutoFocusMode;
		XMLN * p_DefaultSpeed;
		XMLN * p_NearLimit;
		XMLN * p_FarLimit;

		p_req->ImagingSettings.FocusFlag = 1;
		
		p_AutoFocusMode = xml_node_soap_get(p_Focus, "AutoFocusMode");
    	if (p_AutoFocusMode && p_AutoFocusMode->data)
    	{
    		p_req->ImagingSettings.Focus.AutoFocusMode = onvif_StringToAutoFocusMode(p_AutoFocusMode->data);
    	}

    	p_DefaultSpeed = xml_node_soap_get(p_Focus, "DefaultSpeed");
    	if (p_DefaultSpeed && p_DefaultSpeed->data)
    	{
    		p_req->ImagingSettings.Focus.DefaultSpeedFlag = 1;
    	    p_req->ImagingSettings.Focus.DefaultSpeed = (float)atof(p_DefaultSpeed->data);
    	}

    	p_NearLimit = xml_node_soap_get(p_Focus, "NearLimit");
    	if (p_NearLimit && p_NearLimit->data)
    	{
    		p_req->ImagingSettings.Focus.NearLimitFlag = 1;
    	    p_req->ImagingSettings.Focus.NearLimit = (float)atof(p_NearLimit->data);
    	}

    	p_FarLimit = xml_node_soap_get(p_Focus, "FarLimit");
    	if (p_FarLimit && p_FarLimit->data)
    	{
    		p_req->ImagingSettings.Focus.FarLimitFlag = 1;
    	    p_req->ImagingSettings.Focus.FarLimit = (float)atof(p_FarLimit->data);
    	}
    }
    
    p_IrCutFilter = xml_node_soap_get(p_ImagingSettings, "IrCutFilter");
    if (p_IrCutFilter && p_IrCutFilter->data)
    {
    	p_req->ImagingSettings.IrCutFilterFlag = 1;
    	p_req->ImagingSettings.IrCutFilter = onvif_StringToIrCutFilterMode(p_IrCutFilter->data);
    }

    p_Sharpness = xml_node_soap_get(p_ImagingSettings, "Sharpness");
    if (p_Sharpness && p_Sharpness->data)
    {
    	p_req->ImagingSettings.SharpnessFlag = 1;
    	p_req->ImagingSettings.Sharpness = (float)atof(p_Sharpness->data);
    }

    p_WideDynamicRange = xml_node_soap_get(p_ImagingSettings, "WideDynamicRange");
    if (p_WideDynamicRange)
    {
    	XMLN * p_Mode;
		XMLN * p_Level;

		p_req->ImagingSettings.WideDynamicRangeFlag = 1;
		
		p_Mode = xml_node_soap_get(p_WideDynamicRange, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		p_req->ImagingSettings.WideDynamicRange.Mode = onvif_StringToWideDynamicMode(p_Mode->data);
    	}

    	p_Level = xml_node_soap_get(p_WideDynamicRange, "Level");
    	if (p_Level && p_Level->data)
    	{
    		p_req->ImagingSettings.WideDynamicRange.LevelFlag = 1;
    		p_req->ImagingSettings.WideDynamicRange.Level = (float)atof(p_Level->data);
    	}
    }

    p_WhiteBalance = xml_node_soap_get(p_ImagingSettings, "WhiteBalance");
    if (p_WhiteBalance)
    {
    	XMLN * p_Mode;
		XMLN * p_CrGain;
		XMLN * p_CbGain;

		p_req->ImagingSettings.WhiteBalanceFlag = 1;
		
		p_Mode = xml_node_soap_get(p_WhiteBalance, "Mode");
    	if (p_Mode && p_Mode->data)
    	{
    		p_req->ImagingSettings.WhiteBalance.Mode = onvif_StringToWhiteBalanceMode(p_Mode->data);
    	}

    	p_CrGain = xml_node_soap_get(p_WhiteBalance, "CrGain");
    	if (p_CrGain && p_CrGain->data)
    	{
    		p_req->ImagingSettings.WhiteBalance.CrGainFlag = 1;
    	    p_req->ImagingSettings.WhiteBalance.CrGain = (float)atof(p_CrGain->data);
    	}

    	p_CbGain = xml_node_soap_get(p_WhiteBalance, "CbGain");
    	if (p_CbGain && p_CbGain->data)
    	{
    		p_req->ImagingSettings.WhiteBalance.CbGainFlag = 1;
    	    p_req->ImagingSettings.WhiteBalance.CbGain = (float)atof(p_CbGain->data);
    	}
    }

    p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
    if (p_ForcePersistence && p_ForcePersistence->data)
    {
    	p_req->ForcePersistenceFlag = 1;
    	p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
    }
    
	return ONVIF_OK;
}

ONVIF_RET parse_Move(XMLN * p_node, Move_REQ * p_req)
{
	XMLN * p_VideoSourceToken;
	XMLN * p_Focus;
	XMLN * p_Absolute;
	XMLN * p_Relative;
	XMLN * p_Continuous;
	
	p_VideoSourceToken = xml_node_soap_get(p_node, "VideoSourceToken");
    if (p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strncpy(p_req->VideoSourceToken, p_VideoSourceToken->data, sizeof(p_req->VideoSourceToken)-1);
    }
    else
    {
    	return ONVIF_ERR_MISSINGATTR;
    }

    p_Focus = xml_node_soap_get(p_node, "Focus");
    if (NULL == p_Focus)
    {
    	return ONVIF_ERR_MISSINGATTR;
    }

    p_Absolute = xml_node_soap_get(p_node, "Absolute");
    if (p_Absolute)
    {
    	XMLN * p_Position;
		XMLN * p_Speed;
		
    	p_req->Focus.AbsoluteFlag = 1;
    	
    	p_Position = xml_node_soap_get(p_Absolute, "Position");
    	if (p_Position && p_Position->data)
    	{
    		p_req->Focus.Absolute.Position = (float)atof(p_Position->data);
    	}

    	p_Speed = xml_node_soap_get(p_Absolute, "Speed");
    	if (p_Speed && p_Speed->data)
    	{
    		p_req->Focus.Absolute.SpeedFlag = 1;
    		p_req->Focus.Absolute.Speed = (float)atof(p_Speed->data);
    	}
    }
    
	p_Relative = xml_node_soap_get(p_node, "Relative");
	if (p_Relative)
	{
		XMLN * p_Distance;
		XMLN * p_Speed;
		
		p_req->Focus.RelativeFlag = 1;
		
		p_Distance = xml_node_soap_get(p_Relative, "Distance");
		if (p_Distance && p_Distance->data)
		{
			p_req->Focus.Relative.Distance = (float)atof(p_Distance->data);
		}

		p_Speed = xml_node_soap_get(p_Relative, "Speed");
		if (p_Speed && p_Speed->data)
		{
			p_req->Focus.Relative.SpeedFlag = 1;			
			p_req->Focus.Relative.Speed = (float)atof(p_Speed->data);
		}
	}

	p_Continuous = xml_node_soap_get(p_node, "Continuous");
	if (p_Continuous)
	{
		XMLN * p_Speed;
		
		p_req->Focus.ContinuousFlag = 1;
		
		p_Speed = xml_node_soap_get(p_Continuous, "Speed");
		if (p_Speed && p_Speed->data)
		{
			p_req->Focus.Continuous.Speed = (float)atof(p_Speed->data);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_CreateUsers(XMLN * p_node, ONVIF_USER * p_user, int user_max)
{
	int i = 0;
	
	XMLN * p_User = xml_node_soap_get(p_node, "User");
	while (p_User)
	{
		if (i < user_max)
		{
			XMLN * p_Username;
			XMLN * p_Password;
			XMLN * p_UserLevel;

			p_Username = xml_node_soap_get(p_User, "Username");
			if (p_Username && p_Username->data)
			{
				strncpy(p_user[i].Username, p_Username->data, sizeof(p_user[i].Username)-1);
			}

			p_Password = xml_node_soap_get(p_User, "Password");
			if (p_Password && p_Password->data)
			{
				if (strlen(p_Password->data) >= sizeof(p_user[i].Password))
				{
					return ONVIF_ERR_PASSWORD_TOO_LONG;
				}
				
				strncpy(p_user[i].Password, p_Password->data, sizeof(p_user[i].Password)-1);
			}

			p_UserLevel = xml_node_soap_get(p_User, "UserLevel");
			if (p_UserLevel && p_UserLevel->data)
			{
				p_user[i].UserLevel = onvif_StringToUserLevel(p_UserLevel->data);
			}

			++i;
		}
		else
		{
			return ONVIF_ERR_TOO_MANY_USERS;
		}
		
		p_User = p_User->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_DeleteUsers(XMLN * p_node, ONVIF_USER * p_user, int user_max)
{
	int i = 0;
	
	XMLN * p_Username = xml_node_soap_get(p_node, "Username");
	while (p_Username)
	{
		if (i < user_max)
		{
			strncpy(p_user[i].Username, p_Username->data, sizeof(p_user[i].Username)-1);

			++i;
		}
		else
		{
			break;
		}
		
		p_Username = p_Username->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetOSDs(XMLN * p_node, GetOSDs_REQ * p_req)
{
	XMLN * p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetOSD(XMLN * p_node, GetOSD_REQ * p_req)
{
	XMLN * p_OSDToken = xml_node_soap_get(p_node, "OSDToken");
	if (p_OSDToken && p_OSDToken->data)
	{
		strncpy(p_req->OSDToken, p_OSDToken->data, sizeof(p_req->OSDToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_OSDColor(XMLN * p_node, onvif_OSDColor * p_req)
{
	XMLN * p_Color;
	const char * p_Transparent;

	p_Transparent = xml_attr_get(p_node, "Transparent");
	if (p_Transparent)
	{
		p_req->TransparentFlag = 1;
		p_req->Transparent = atoi(p_Transparent);
	}

	p_Color = xml_node_soap_get(p_node, "Color");
	if (p_Color)
	{
		const char * p_X;
		const char * p_Y;
		const char * p_Z;

		p_X = xml_attr_get(p_Color, "X");
		if (p_X)
		{
			p_req->X = (float) atof(p_X);
		}

		p_Y = xml_attr_get(p_Color, "Y");
		if (p_X)
		{
			p_req->Y = (float) atof(p_Y);
		}

		p_Z = xml_attr_get(p_Color, "Z");
		if (p_Z)
		{
			p_req->Z = (float) atof(p_Z);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_OSDConfiguration(XMLN * p_node, onvif_OSDConfiguration * p_req)
{
	XMLN * p_OSD;
	XMLN * p_VideoSourceConfigurationToken;
	XMLN * p_Type;
	XMLN * p_Position;
	XMLN * p_TextString;
	XMLN * p_Image;
	const char * p_token;

	p_OSD = xml_node_soap_get(p_node, "OSD");
	if (NULL == p_OSD)
	{
		return ONVIF_ERR_MISSINGATTR;
	}

	p_token = xml_attr_get(p_OSD, "token");
	if (p_token)
	{
		strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
	}

	p_VideoSourceConfigurationToken = xml_node_soap_get(p_OSD, "VideoSourceConfigurationToken");
	if (p_VideoSourceConfigurationToken && p_VideoSourceConfigurationToken->data)
	{
		strncpy(p_req->VideoSourceConfigurationToken, p_VideoSourceConfigurationToken->data, sizeof(p_req->VideoSourceConfigurationToken)-1);
	}

	p_Type = xml_node_soap_get(p_OSD, "Type");
	if (p_Type && p_Type->data)
	{
		p_req->Type = onvif_StringToOSDType(p_Type->data);
	}

	p_Position = xml_node_soap_get(p_OSD, "Position");
	if (p_Position)
	{
		XMLN * p_Type;
		XMLN * p_Pos;

		p_Type = xml_node_soap_get(p_Position, "Type");
		if (p_Type && p_Type->data)
		{
			p_req->Position.Type = onvif_StringToOSDPosType(p_Type->data);
		}

		p_Pos = xml_node_soap_get(p_Position, "Pos");
		if (p_Pos)
		{
			p_req->Position.PosFlag = 1;
			parse_Vector(p_Pos, &p_req->Position.Pos);
		}
	}

	p_TextString = xml_node_soap_get(p_OSD, "TextString");
	if (p_TextString)
	{
		XMLN * p_Type;
		XMLN * p_DateFormat;
		XMLN * p_TimeFormat;
		XMLN * p_FontSize;
		XMLN * p_FontColor;
		XMLN * p_BackgroundColor;
		XMLN * p_PlainText;
		
		p_req->TextStringFlag = 1;
		
		p_Type = xml_node_soap_get(p_TextString, "Type");
		if (p_Type && p_Type->data)
		{
			p_req->TextString.Type = onvif_StringToOSDTextType(p_Type->data);
		}

		p_DateFormat = xml_node_soap_get(p_TextString, "DateFormat");
		if (p_DateFormat && p_DateFormat->data)
		{
			p_req->TextString.DateFormatFlag = 1;
			strncpy(p_req->TextString.DateFormat, p_DateFormat->data, sizeof(p_req->TextString.DateFormat)-1);
		}

		p_TimeFormat = xml_node_soap_get(p_TextString, "TimeFormat");
		if (p_TimeFormat && p_TimeFormat->data)
		{
			p_req->TextString.TimeFormatFlag = 1;
			strncpy(p_req->TextString.TimeFormat, p_TimeFormat->data, sizeof(p_req->TextString.TimeFormat)-1);
		}

		p_FontSize = xml_node_soap_get(p_TextString, "FontSize");
		if (p_FontSize && p_FontSize->data)
		{
			p_req->TextString.FontSizeFlag = 1;
			p_req->TextString.FontSize = atoi(p_FontSize->data);
		}

		p_FontColor = xml_node_soap_get(p_TextString, "FontColor");
		if (p_FontColor)
		{
			p_req->TextString.FontColorFlag = 1;
			
			parse_OSDColor(p_FontColor, &p_req->TextString.FontColor);
		}

		p_BackgroundColor = xml_node_soap_get(p_TextString, "BackgroundColor");
		if (p_BackgroundColor)
		{
			p_req->TextString.BackgroundColorFlag = 1;
			
			parse_OSDColor(p_BackgroundColor, &p_req->TextString.BackgroundColor);
		}

		p_PlainText = xml_node_soap_get(p_TextString, "PlainText");
		if (p_PlainText && p_PlainText->data)
		{
			p_req->TextString.PlainTextFlag = 1;
			strncpy(p_req->TextString.PlainText, p_PlainText->data, sizeof(p_req->TextString.PlainText)-1);
		}
	}

	p_Image = xml_node_soap_get(p_OSD, "Image");
	if (p_Image)
	{
		XMLN * p_ImgPath;
		
		p_req->ImageFlag = 1;
		
		p_ImgPath = xml_node_soap_get(p_Image, "ImgPath");
		if (p_ImgPath && p_ImgPath->data)
		{
			strncpy(p_req->Image.ImgPath, p_ImgPath->data, sizeof(p_req->Image.ImgPath)-1);
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SetOSD(XMLN * p_node, SetOSD_REQ * p_req)
{
	return parse_OSDConfiguration(p_node, &p_req->OSD);
}

ONVIF_RET parse_CreateOSD(XMLN * p_node, CreateOSD_REQ * p_req)
{
	return parse_OSDConfiguration(p_node, &p_req->OSD);
}

ONVIF_RET parse_DeleteOSD(XMLN * p_node, DeleteOSD_REQ * p_req)
{
	XMLN * p_OSDToken = xml_node_soap_get(p_node, "OSDToken");
	if (p_OSDToken && p_OSDToken->data)
	{
		strncpy(p_req->OSDToken, p_OSDToken->data, sizeof(p_req->OSDToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_FloatRange(XMLN * p_node, onvif_FloatRange * p_req)
{
	XMLN * p_Min;
	XMLN * p_Max;

	p_Min = xml_node_soap_get(p_node, "Min");
	if (p_Min && p_Min->data)
	{
		p_req->Min = (float)atof(p_Min->data);
	}
	
	p_Max = xml_node_soap_get(p_node, "Max");
	if (p_Max && p_Max->data)
	{
		p_req->Max = (float)atof(p_Max->data);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_PTZConfiguration(XMLN * p_node, onvif_PTZConfiguration * p_req)
{
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_NodeToken;
	XMLN * p_DefaultPTZSpeed;
	XMLN * p_DefaultPTZTimeout;
	XMLN * p_PanTiltLimits;
	XMLN * p_ZoomLimits;
	XMLN * p_Extension;
	
	p_Name = xml_node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	p_UseCount = xml_node_soap_get(p_node, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		p_req->UseCount = atoi(p_UseCount->data);
	}

	p_NodeToken = xml_node_soap_get(p_node, "NodeToken");
	if (p_NodeToken && p_NodeToken->data)
	{
		strncpy(p_req->NodeToken, p_NodeToken->data, sizeof(p_req->NodeToken)-1);
	}

	p_DefaultPTZSpeed = xml_node_soap_get(p_node, "DefaultPTZSpeed");
	if (p_DefaultPTZSpeed)
	{
		XMLN * p_PanTilt;
		XMLN * p_Zoom;

		p_req->DefaultPTZSpeedFlag = 1;
		
		p_PanTilt = xml_node_soap_get(p_DefaultPTZSpeed, "PanTilt");
		if (p_PanTilt)
		{
			p_req->DefaultPTZSpeed.PanTiltFlag = 1;
			parse_Vector(p_PanTilt, &p_req->DefaultPTZSpeed.PanTilt);
		}

		p_Zoom = xml_node_soap_get(p_DefaultPTZSpeed, "Zoom");
		if (p_Zoom)
		{
			p_req->DefaultPTZSpeed.ZoomFlag = 1;
			parse_Vector1D(p_Zoom, &p_req->DefaultPTZSpeed.Zoom);
		}
	}

	p_DefaultPTZTimeout = xml_node_soap_get(p_node, "DefaultPTZTimeout");
	if (p_DefaultPTZTimeout && p_DefaultPTZTimeout->data)
	{
		p_req->DefaultPTZTimeoutFlag = parse_XSDDuration(p_DefaultPTZTimeout->data, &p_req->DefaultPTZTimeout);
	}

	p_PanTiltLimits = xml_node_soap_get(p_node, "PanTiltLimits");
	if (p_PanTiltLimits)
	{
		XMLN * p_Range;
		
		p_req->PanTiltLimitsFlag = 1;
		
		p_Range = xml_node_soap_get(p_PanTiltLimits, "Range");
		if (p_Range)
		{
			XMLN * p_XRange;
			XMLN * p_YRange;

			p_XRange = xml_node_soap_get(p_Range, "XRange");
			if (p_XRange)
			{
				parse_FloatRange(p_XRange, &p_req->PanTiltLimits.XRange);
			}	

			p_YRange = xml_node_soap_get(p_Range, "YRange");
			if (p_YRange)
			{
				parse_FloatRange(p_YRange, &p_req->PanTiltLimits.YRange);
			}
		}
	}

	p_ZoomLimits = xml_node_soap_get(p_node, "ZoomLimits");
	if (p_ZoomLimits)
	{
		XMLN * p_Range;
		
		p_req->ZoomLimitsFlag = 1;
		
		p_Range = xml_node_soap_get(p_ZoomLimits, "Range");
		if (p_Range)
		{
			XMLN * p_XRange;

			p_XRange = xml_node_soap_get(p_Range, "XRange");
			if (p_XRange)
			{
				parse_FloatRange(p_XRange, &p_req->ZoomLimits.XRange);
			}
		}
	}

	p_Extension = xml_node_soap_get(p_node, "Extension");
	if (p_Extension)
	{
		XMLN * p_PTControlDirection;
		XMLN * p_EFlip;
		XMLN * p_Reverse;
		XMLN * p_Mode;
		
		p_req->ExtensionFlag = 1;
		
		p_PTControlDirection = xml_node_soap_get(p_Extension, "PTControlDirection");
		if (p_PTControlDirection)
		{
			p_req->Extension.PTControlDirectionFlag = 1;
			
			p_EFlip = xml_node_soap_get(p_PTControlDirection, "EFlip");
			if (p_EFlip)
			{
				p_req->Extension.PTControlDirection.EFlipFlag = 1;

				p_Mode = xml_node_soap_get(p_EFlip, "Mode");
				if (p_Mode && p_Mode->data)
				{
					p_req->Extension.PTControlDirection.EFlip = onvif_StringToEFlipMode(p_Mode->data);
				}
			}

			p_Reverse = xml_node_soap_get(p_PTControlDirection, "Reverse");
			if (p_Reverse)
			{
				p_req->Extension.PTControlDirection.ReverseFlag = 1;

				p_Mode = xml_node_soap_get(p_Reverse, "Mode");
				if (p_Mode && p_Mode->data)
				{
					p_req->Extension.PTControlDirection.Reverse = onvif_StringToReverseMode(p_Mode->data);
				}
			}
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_SetConfiguration(XMLN * p_node, SetConfiguration_REQ * p_req)
{
	ONVIF_RET ret;
	const char * p_token;
	XMLN * p_PTZConfiguration;
	XMLN * p_ForcePersistence;

	p_PTZConfiguration = xml_node_soap_get(p_node, "PTZConfiguration");
	if (NULL == p_PTZConfiguration)
	{
		return ONVIF_ERR_MISSINGATTR;
	}

	p_token = xml_attr_get(p_PTZConfiguration, "token");
	if (p_token)
	{
		strncpy(p_req->PTZConfiguration.token, p_token, sizeof(p_req->PTZConfiguration.token)-1);
	}

	ret = parse_PTZConfiguration(p_PTZConfiguration, &p_req->PTZConfiguration);
	if (ONVIF_OK != ret)
	{
		return ret;
	}

	p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);		
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetMetadataConfigurationOptions(XMLN * p_node, GetMetadataConfigurationOptions_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_ProfileToken;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		p_req->ConfigurationTokenFlag = 1;
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		p_req->ProfileTokenFlag = 1;
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SetMetadataConfiguration(XMLN * p_node, SetMetadataConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;
	XMLN * p_Name;
	XMLN * p_PTZStatus;
	XMLN * p_Analytics;
	XMLN * p_SessionTimeout;
	XMLN * p_ForcePersistence;
	const char * token;
	
	p_Configuration = xml_node_soap_get(p_node, "Configuration");
	if (NULL == p_Configuration)
	{
		return ONVIF_ERR_MISSINGATTR;
	}
	
	token = xml_attr_get(p_Configuration, "token");
	if (token)
	{
		strncpy(p_req->Configuration.token, token, sizeof(p_req->Configuration.token)-1);
	}

	p_Name = xml_node_soap_get(p_Configuration, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Configuration.Name, p_Name->data, sizeof(p_req->Configuration.Name)-1);
	}

	p_PTZStatus = xml_node_soap_get(p_Configuration, "PTZStatus");
	if (p_PTZStatus)
	{
		XMLN * p_Status;
		XMLN * p_Position;

		p_req->Configuration.PTZStatusFlag = 1;
		
		p_Status = xml_node_soap_get(p_PTZStatus, "Status");
		if (p_Status && p_Status->data)
		{
			p_req->Configuration.PTZStatus.Status = parse_Bool(p_Status->data);
		}

		p_Position = xml_node_soap_get(p_PTZStatus, "Position");
		if (p_Position && p_Position->data)
		{
			p_req->Configuration.PTZStatus.Position = parse_Bool(p_Position->data);
		}
	}

	p_Analytics = xml_node_soap_get(p_Configuration, "Analytics");
	if (p_Analytics && p_Analytics->data)
	{
		p_req->Configuration.AnalyticsFlag = 1;
		p_req->Configuration.Analytics = parse_Bool(p_Analytics->data);
	}

	parse_MulticastConfiguration(p_Configuration, &p_req->Configuration.Multicast);

	p_SessionTimeout = xml_node_soap_get(p_Configuration, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
		parse_XSDDuration(p_SessionTimeout->data, &p_req->Configuration.SessionTimeout);
	}

	p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_AddMetadataConfiguration(XMLN * p_node, AddMetadataConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}
	
	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}
	
	return ONVIF_OK;
}

#ifdef PROFILE_G_SUPPORT

ONVIF_RET parse_RecordingConfiguration(XMLN * p_node, onvif_RecordingConfiguration * p_req)
{
	XMLN * p_Source;
	XMLN * p_Content;
	XMLN * p_MaximumRetentionTime;

	p_Source = xml_node_soap_get(p_node, "Source");
	if (p_Source)
	{
		XMLN * p_SourceId;
		XMLN * p_Name;
		XMLN * p_Location;
		XMLN * p_Description;
		XMLN * p_Address;

		p_SourceId = xml_node_soap_get(p_Source, "SourceId");
		if (p_SourceId && p_SourceId->data)
		{
			strncpy(p_req->Source.SourceId, p_SourceId->data, sizeof(p_req->Source.SourceId)-1);
		}

		p_Name = xml_node_soap_get(p_Source, "Name");
		if (p_Name && p_Name->data)
		{
			strncpy(p_req->Source.Name, p_Name->data, sizeof(p_req->Source.Name)-1);
		}

		p_Location = xml_node_soap_get(p_Source, "Location");
		if (p_Location && p_Location->data)
		{
			strncpy(p_req->Source.Location, p_Location->data, sizeof(p_req->Source.Location)-1);
		}

		p_Description = xml_node_soap_get(p_Source, "Description");
		if (p_Description && p_Description->data)
		{
			strncpy(p_req->Source.Description, p_Description->data, sizeof(p_req->Source.Description)-1);
		}

		p_Address = xml_node_soap_get(p_Source, "Address");
		if (p_Address && p_Address->data)
		{
			strncpy(p_req->Source.Address, p_Address->data, sizeof(p_req->Source.Address)-1);
		}
	}

	p_Content = xml_node_soap_get(p_node, "Content");
	if (p_Content && p_Content->data)
	{
		strncpy(p_req->Content, p_Content->data, sizeof(p_req->Content)-1);
	}

	p_MaximumRetentionTime = xml_node_soap_get(p_node, "MaximumRetentionTime");
	if (p_MaximumRetentionTime && p_MaximumRetentionTime->data)
	{
		p_req->MaximumRetentionTimeFlag = parse_XSDDuration(p_MaximumRetentionTime->data, &p_req->MaximumRetentionTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_CreateRecording(XMLN * p_node, CreateRecording_REQ * p_req)
{
	XMLN * p_RecordingConfiguration;

	p_RecordingConfiguration = xml_node_soap_get(p_node, "RecordingConfiguration");
	if (NULL == p_RecordingConfiguration)
	{
		return ONVIF_ERR_BAD_CONFIGURATION;
	}
	else
	{
		return parse_RecordingConfiguration(p_RecordingConfiguration, &p_req->RecordingConfiguration);
	}
}

ONVIF_RET parse_SetRecordingConfiguration(XMLN * p_node, SetRecordingConfiguration_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_RecordingConfiguration;

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}
	
	p_RecordingConfiguration = xml_node_soap_get(p_node, "RecordingConfiguration");
	if (p_RecordingConfiguration)
	{
		return parse_RecordingConfiguration(p_RecordingConfiguration, &p_req->RecordingConfiguration);
	}
	else
	{
		return ONVIF_ERR_BAD_CONFIGURATION;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_TrackConfiguration(XMLN * p_node, onvif_TrackConfiguration * p_req)
{
	XMLN * p_TrackType;
	XMLN * p_Description;

	p_TrackType = xml_node_soap_get(p_node, "TrackType");
	if (p_TrackType && p_TrackType->data)
	{
		p_req->TrackType = onvif_StringToTrackType(p_TrackType->data);
		if (TrackType_Invalid == p_req->TrackType)
		{
			return ONVIF_ERR_BAD_CONFIGURATION;
		}
	}

	p_Description = xml_node_soap_get(p_node, "Description");
	if (p_Description && p_Description->data)
	{
		strncpy(p_req->Description, p_Description->data, sizeof(p_req->Description)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_CreateTrack(XMLN * p_node, CreateTrack_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackConfiguration;

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackConfiguration = xml_node_soap_get(p_node, "TrackConfiguration");
	if (p_TrackConfiguration)
	{
		return parse_TrackConfiguration(p_TrackConfiguration, &p_req->TrackConfiguration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_DeleteTrack(XMLN * p_node, DeleteTrack_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackToken;

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackToken = xml_node_soap_get(p_node, "TrackToken");
	if (p_TrackToken && p_TrackToken->data)
	{
		strncpy(p_req->TrackToken, p_TrackToken->data, sizeof(p_req->TrackToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetTrackConfiguration(XMLN * p_node, GetTrackConfiguration_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackToken;

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackToken = xml_node_soap_get(p_node, "TrackToken");
	if (p_TrackToken && p_TrackToken->data)
	{
		strncpy(p_req->TrackToken, p_TrackToken->data, sizeof(p_req->TrackToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SetTrackConfiguration(XMLN * p_node, SetTrackConfiguration_REQ * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_TrackToken;
	XMLN * p_TrackConfiguration;

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_TrackToken = xml_node_soap_get(p_node, "TrackToken");
	if (p_TrackToken && p_TrackToken->data)
	{
		strncpy(p_req->TrackToken, p_TrackToken->data, sizeof(p_req->TrackToken)-1);
	}

	p_TrackConfiguration = xml_node_soap_get(p_node, "TrackConfiguration");
	if (p_TrackConfiguration)
	{
		return parse_TrackConfiguration(p_TrackConfiguration, &p_req->TrackConfiguration);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_JobConfiguration(XMLN * p_node, onvif_RecordingJobConfiguration * p_req)
{
	XMLN * p_RecordingToken;
	XMLN * p_Mode;
	XMLN * p_Priority;
	XMLN * p_Source;

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	p_Mode = xml_node_soap_get(p_node, "Mode");
	if (p_Mode && p_Mode->data)
	{
		strncpy(p_req->Mode, p_Mode->data, sizeof(p_req->Mode)-1);
	}

	p_Priority = xml_node_soap_get(p_node, "Priority");
	if (p_Priority && p_Priority->data)
	{
		p_req->Priority = atoi(p_Priority->data);
	}

	p_Source = xml_node_soap_get(p_node, "Source");
	while (p_Source && soap_strcmp(p_Source->name, "Source") == 0)
	{
		int i = p_req->sizeSource;
		XMLN * p_SourceToken;
		XMLN * p_AutoCreateReceiver;
		XMLN * p_Tracks;

		p_SourceToken = xml_node_soap_get(p_Source, "SourceToken");
		if (p_SourceToken)
		{
			const char * p_Type;
			XMLN * p_Token;

			p_req->Source[i].SourceTokenFlag = 1;
			
			p_Type = xml_attr_get(p_SourceToken, "Type");
			if (p_Type)
			{
				p_req->Source[i].SourceToken.TypeFlag = 1;
				strncpy(p_req->Source[i].SourceToken.Type, p_Type, sizeof(p_req->Source[i].SourceToken.Type)-1);
			}

			p_Token = xml_node_soap_get(p_SourceToken, "Token");
			if (p_Token && p_Token->data)
			{
				strncpy(p_req->Source[i].SourceToken.Token, p_Token->data, sizeof(p_req->Source[i].SourceToken.Token)-1);
			}
		}

		p_AutoCreateReceiver = xml_node_soap_get(p_Source, "AutoCreateReceiver");
		if (p_AutoCreateReceiver && p_AutoCreateReceiver->data)
		{
			p_req->Source[i].AutoCreateReceiverFlag = 1;
			p_req->Source[i].AutoCreateReceiver = parse_Bool(p_AutoCreateReceiver->data);
		}

		p_Tracks = xml_node_soap_get(p_Source, "Tracks");
		while (p_Tracks && soap_strcmp(p_Tracks->name, "Tracks") == 0)
		{
			int j = p_req->Source[i].sizeTracks;
			XMLN * p_SourceTag;
			XMLN * p_Destination;

			p_SourceTag = xml_node_soap_get(p_Tracks, "SourceTag");
			if (p_SourceTag && p_SourceTag->data)
			{
				strncpy(p_req->Source[i].Tracks[j].SourceTag, p_SourceTag->data, sizeof(p_req->Source[i].Tracks[j].SourceTag)-1);
			}

			p_Destination = xml_node_soap_get(p_Tracks, "Destination");
			if (p_Destination && p_Destination->data)
			{
				strncpy(p_req->Source[i].Tracks[j].Destination, p_Destination->data, sizeof(p_req->Source[i].Tracks[j].Destination)-1);
			}
			
			p_Tracks = p_Tracks->next;

			p_req->Source[i].sizeTracks++;
			if (p_req->Source[i].sizeTracks >= 5)
			{
				break;
			}
		}
		
		p_Source = p_Source->next;

		p_req->sizeSource++;
		if (p_req->sizeSource >= 5)
		{
			break;
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_CreateRecordingJob(XMLN * p_node, CreateRecordingJob_REQ * p_req)
{
	XMLN * p_JobConfiguration;
	
	p_JobConfiguration = xml_node_soap_get(p_node, "JobConfiguration");
	if (p_JobConfiguration)
	{
		return parse_JobConfiguration(p_JobConfiguration, &p_req->JobConfiguration);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_SetRecordingJobConfiguration(XMLN * p_node, SetRecordingJobConfiguration_REQ * p_req)
{
	XMLN * p_JobToken;
	XMLN * p_JobConfiguration;

	p_JobToken = xml_node_soap_get(p_node, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		strncpy(p_req->JobToken, p_JobToken->data, sizeof(p_req->JobToken)-1);
	}
	
	p_JobConfiguration = xml_node_soap_get(p_node, "JobConfiguration");
	if (p_JobConfiguration)
	{
		return parse_JobConfiguration(p_JobConfiguration, &p_req->JobConfiguration);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_SetRecordingJobMode(XMLN * p_node, SetRecordingJobMode_REQ * p_req)
{
	XMLN * p_JobToken;
	XMLN * p_Mode;

	p_JobToken = xml_node_soap_get(p_node, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		strncpy(p_req->JobToken, p_JobToken->data, sizeof(p_req->JobToken)-1);
	}
	
	p_Mode = xml_node_soap_get(p_node, "Mode");
	if (p_Mode && p_Mode->data)
	{
		strncpy(p_req->Mode, p_Mode->data, sizeof(p_req->Mode));
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetMediaAttributes(XMLN * p_node, GetMediaAttributes_REQ * p_req)
{
	int idx;
	XMLN * p_RecordingTokens;
	XMLN * p_Time;

	p_RecordingTokens = xml_node_soap_get(p_node, "RecordingTokens");
	while (p_RecordingTokens && p_RecordingTokens->data && soap_strcmp(p_RecordingTokens->name, "RecordingTokens") == 0)
	{
		idx = p_req->sizeRecordingTokens;
		strncpy(p_req->RecordingTokens[idx], p_RecordingTokens->data, sizeof(p_req->RecordingTokens[idx])-1);
		
		p_RecordingTokens = p_RecordingTokens->next;
		p_req->sizeRecordingTokens++;

		if (p_req->sizeRecordingTokens >= 10)
		{
			break;
		}
	}

	p_Time = xml_node_soap_get(p_node, "Time");
	if (p_Time && p_Time->data)
	{
		parse_XSDDatetime(p_Time->data, &p_req->Time);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SearchScope(XMLN * p_node, onvif_SearchScope * p_req)
{
	XMLN * p_IncludedSources;
	XMLN * p_IncludedRecordings;
	XMLN * p_RecordingInformationFilter;

	p_IncludedSources = xml_node_soap_get(p_node, "IncludedSources");
	while (p_IncludedSources && soap_strcmp(p_IncludedSources->name, "IncludedSources") == 0)
	{
		int idx = p_req->sizeIncludedSources;
		const char * p_Type;
		XMLN * p_Token;

		p_Type = xml_attr_get(p_IncludedSources, "Type");
		if (p_Type)
		{
			p_req->IncludedSources[idx].TypeFlag = 1;
			strncpy(p_req->IncludedSources[idx].Type, p_Type, sizeof(p_req->IncludedSources[idx].Type));			
		}

		p_Token = xml_node_soap_get(p_IncludedSources, "Token");
		if (p_Token && p_Token->data)
		{
			strncpy(p_req->IncludedSources[idx].Token, p_Token->data, sizeof(p_req->IncludedSources[idx].Token));
		}
		
		p_req->sizeIncludedSources++;
		if (p_req->sizeIncludedSources >= sizeof(p_req->IncludedSources)/sizeof(p_req->IncludedSources[0]))
		{
			break;
		}		
	}

	p_IncludedRecordings = xml_node_soap_get(p_node, "IncludedRecordings");
	while (p_IncludedRecordings && p_IncludedRecordings->data && soap_strcmp(p_IncludedRecordings->name, "IncludedRecordings") == 0)
	{
		int idx = p_req->sizeIncludedRecordings;
		
		strncpy(p_req->IncludedRecordings[idx], p_IncludedRecordings->data, sizeof(p_req->IncludedRecordings[idx])-1);

		p_req->sizeIncludedRecordings++;
		if (p_req->sizeIncludedRecordings >= sizeof(p_req->IncludedRecordings)/sizeof(p_req->IncludedRecordings[0]))
		{
			break;
		}
	}

	p_RecordingInformationFilter = xml_node_soap_get(p_node, "RecordingInformationFilter");
	if (p_RecordingInformationFilter && p_RecordingInformationFilter->data)
	{
		strncpy(p_req->RecordingInformationFilter, p_RecordingInformationFilter->data, sizeof(p_req->RecordingInformationFilter));
	}

	return ONVIF_OK;
}

ONVIF_RET parse_FindRecordings(XMLN * p_node, FindRecordings_REQ * p_req)
{
	XMLN * p_Scope;
	XMLN * p_MaxMatches;
	XMLN * p_KeepAliveTime;

	p_Scope = xml_node_soap_get(p_node, "Scope");
	if (p_Scope)
	{
		parse_SearchScope(p_Scope, &p_req->Scope);
	}

	p_MaxMatches = xml_node_soap_get(p_node, "MaxMatches");
	if (p_MaxMatches && p_MaxMatches->data)
	{
		p_req->MaxMatchesFlag = 1;
		p_req->MaxMatches = atoi(p_MaxMatches->data);
	}

	p_KeepAliveTime = xml_node_soap_get(p_node, "KeepAliveTime");
	if (p_KeepAliveTime && p_KeepAliveTime->data)
	{
		parse_XSDDuration(p_KeepAliveTime->data, &p_req->KeepAliveTime);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetRecordingSearchResults(XMLN * p_node, GetRecordingSearchResults_REQ * p_req)
{
	XMLN * p_SearchToken;
	XMLN * p_MinResults;
	XMLN * p_MaxResults;
	XMLN * p_WaitTime;

	p_SearchToken = xml_node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	p_MinResults = xml_node_soap_get(p_node, "MinResults");
	if (p_MinResults && p_MinResults->data)
	{
		p_req->MinResultsFlag = 1;
		p_req->MinResults = atoi(p_MinResults->data);
	}

	p_MaxResults = xml_node_soap_get(p_node, "MaxResults");
	if (p_MaxResults && p_MaxResults->data)
	{
		p_req->MaxResultsFlag = 1;
		p_req->MaxResults = atoi(p_MaxResults->data);
	}

	p_WaitTime = xml_node_soap_get(p_node, "WaitTime");
	if (p_WaitTime && p_WaitTime->data)
	{
		p_req->WaitTimeFlag = 1;
		parse_XSDDuration(p_WaitTime->data, &p_req->WaitTime);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_FindEvents(XMLN * p_node, FindEvents_REQ * p_req)
{
	XMLN * p_StartPoint;
	XMLN * p_EndPoint;
	XMLN * p_Scope;
	XMLN * p_IncludeStartState;
	XMLN * p_MaxMatches;
	XMLN * p_KeepAliveTime;

	p_StartPoint = xml_node_soap_get(p_node, "StartPoint");
	if (p_StartPoint && p_StartPoint->data)
	{
		parse_XSDDatetime(p_StartPoint->data, &p_req->StartPoint);
	}

	p_EndPoint = xml_node_soap_get(p_node, "EndPoint");
	if (p_EndPoint && p_EndPoint->data)
	{
		p_req->EndPointFlag = 1;
		parse_XSDDatetime(p_EndPoint->data, &p_req->EndPoint);
	}

	p_Scope = xml_node_soap_get(p_node, "Scope");
	if (p_Scope)
	{
		parse_SearchScope(p_Scope, &p_req->Scope);
	}

	p_IncludeStartState = xml_node_soap_get(p_node, "IncludeStartState");
	if (p_IncludeStartState && p_IncludeStartState->data)
	{
		p_req->IncludeStartState = parse_Bool(p_IncludeStartState->data);
	}

	p_MaxMatches = xml_node_soap_get(p_node, "MaxMatches");
	if (p_MaxMatches && p_MaxMatches->data)
	{
		p_req->MaxMatchesFlag = 1;
		p_req->MaxMatches = atoi(p_MaxMatches->data);
	}

	p_KeepAliveTime = xml_node_soap_get(p_node, "KeepAliveTime");
	if (p_KeepAliveTime && p_KeepAliveTime->data)
	{
		p_req->KeepAliveTimeFlag = 1;
		parse_XSDDuration(p_KeepAliveTime->data, &p_req->KeepAliveTime);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetEventSearchResults(XMLN * p_node, GetEventSearchResults_REQ * p_req)
{
	XMLN * p_SearchToken;
	XMLN * p_MinResults;
	XMLN * p_MaxResults;
	XMLN * p_WaitTime;

	p_SearchToken = xml_node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	p_MinResults = xml_node_soap_get(p_node, "MinResults");
	if (p_MinResults && p_MinResults->data)
	{
		p_req->MinResultsFlag = 1;
		p_req->MinResults = atoi(p_MinResults->data);
	}

	p_MaxResults = xml_node_soap_get(p_node, "MaxResults");
	if (p_MaxResults && p_MaxResults->data)
	{
		p_req->MaxResultsFlag = 1;
		p_req->MaxResults = atoi(p_MaxResults->data);
	}

	p_WaitTime = xml_node_soap_get(p_node, "WaitTime");
	if (p_WaitTime && p_WaitTime->data)
	{
		p_req->WaitTimeFlag = 1;
		parse_XSDDuration(p_WaitTime->data, &p_req->WaitTime);
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_EndSearch(XMLN * p_node, EndSearch_REQ * p_req)
{
	XMLN * p_SearchToken;

	p_SearchToken = xml_node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetSearchState(XMLN * p_node, GetSearchState_REQ * p_req)
{
	XMLN * p_SearchToken;

	p_SearchToken = xml_node_soap_get(p_node, "SearchToken");
	if (p_SearchToken && p_SearchToken->data)
	{
		strncpy(p_req->SearchToken, p_SearchToken->data, sizeof(p_req->SearchToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetReplayUri(XMLN * p_node, GetReplayUri_REQ * p_req)
{
	ONVIF_RET ret = ONVIF_OK;
	XMLN * p_StreamSetup;
	XMLN * p_RecordingToken;

	p_StreamSetup = xml_node_soap_get(p_node, "StreamSetup");
	if (p_StreamSetup)
	{
		ret = parse_StreamSetup(p_StreamSetup, &p_req->StreamSetup);
	}

	p_RecordingToken = xml_node_soap_get(p_node, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		strncpy(p_req->RecordingToken, p_RecordingToken->data, sizeof(p_req->RecordingToken)-1);
	}

	return ret;
}

ONVIF_RET parse_SetReplayConfiguration(XMLN * p_node, SetReplayConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;

	p_Configuration = xml_node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
		XMLN * p_SessionTimeout;
		
		p_SessionTimeout = xml_node_soap_get(p_node, "SessionTimeout");
		if (p_SessionTimeout && p_SessionTimeout->data)
		{
			parse_XSDDuration(p_SessionTimeout->data, &p_req->SessionTimeout);
		}
	}

	return ONVIF_OK;
}

#endif	// end of PROFILE_G_SUPPORT

#ifdef VIDEO_ANALYTICS

ONVIF_RET parse_GetSupportedRules(XMLN * p_node, GetSupportedRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_SimpleItem(XMLN * p_node, onvif_SimpleItem * p_req)
{
	const char * p_Name;
	const char * p_Value;

	p_Name = xml_attr_get(p_node, "Name");
	if (p_Name)
	{
		strncpy(p_req->Name, p_Name, sizeof(p_req->Name));
	}

	p_Value = xml_attr_get(p_node, "Value");
	if (p_Value)
	{
		strncpy(p_req->Value, p_Value, sizeof(p_req->Value));
	}

	return ONVIF_OK;
}

int parse_ElementItem_any(XMLN * p_node, char * p_buf,unsigned int mlen)
{
	struct XMLN * p_temp = p_node;
	int offset = 0;
	while (p_temp)
	{
		struct XMLN * p_attr = p_temp->f_attrib;
		char attr[256]={0};
		int offset_attr=0;
		while (p_attr)
		{
			offset_attr += snprintf(attr+offset_attr, 256-offset_attr, "%s=\"%s\" ",p_attr->name,p_attr->data);
			p_attr = p_attr->next;
		}

		if(p_temp->f_child == NULL)
		{
			if(strlen(attr))
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<%s %s />",p_temp->name,attr);
			}
			else
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<%s />",p_temp->name);
			}
		}
		else
		{
			if(strlen(attr))
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<%s %s>",p_temp->name,attr);
			}
			else
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<%s>",p_temp->name);
			}
			offset += parse_ElementItem_any(p_temp->f_child,p_buf+offset,mlen-offset);
			offset += snprintf(p_buf+offset, mlen-offset, "</%s>",p_temp->name);
		}
		
		p_temp = p_temp->next;
	}
	return offset;
}

ONVIF_RET parse_ElementItem(XMLN * p_node, onvif_ElementItem * p_req)
{
	const char * p_Name;
	const char * p_Any;
	p_Name = xml_attr_get(p_node, "Name");
	if (p_Name)
	{
		strncpy(p_req->Name, p_Name, sizeof(p_req->Name));
	}

	if (p_node->dlen >0)
	{
		p_req->Any =get_idle_net_buf();//(char *)malloc(p_node->dlen+1);
		memset(p_req->Any,0,p_node->dlen+1);
		if(p_req->Any)
			strcpy(p_req->Any, (char *)p_node->data);
		else
			p_req->Any = NULL;
	}
	else if(p_node->f_child != NULL)
	{
		char p_buf[1024]={0};
		int mlen = parse_ElementItem_any(p_node->f_child,p_buf,1024);
		p_req->Any =get_idle_net_buf();//(char *)malloc(mlen+1);
		memset(p_req->Any,0,mlen+1);
		if(p_req->Any)
			memcpy(p_req->Any, p_buf,mlen);
		else
			p_req->Any = NULL;
	}
	else
		p_req->Any = NULL;

	return ONVIF_OK;
}

ONVIF_RET parse_Config(XMLN * p_node, onvif_Config * p_req)
{
	XMLN * p_Parameters;	
	const char * p_Name;
	const char * p_Type;
	ONVIF_RET ret;

	p_Name = xml_attr_get(p_node, "Name");
	if (p_Name)
	{
		strncpy(p_req->Name, p_Name, sizeof(p_req->Name));
	}

	p_Type = xml_attr_get(p_node, "Type");
	if (p_Type)
	{
		strncpy(p_req->Type, p_Type, sizeof(p_req->Type));
	}

	p_Parameters = xml_node_soap_get(p_node, "Parameters");
	if (p_Parameters)
	{	
		XMLN * p_SimpleItem;
		XMLN * p_ElementItem;

		p_SimpleItem = xml_node_soap_get(p_Parameters, "SimpleItem");
		while (p_SimpleItem && soap_strcmp(p_SimpleItem->name, "SimpleItem") == 0)
		{
			ONVIF_SIMPLEITEM * p_simple_item = onvif_add_simple_item(&p_req->Parameters.SimpleItem);
			if (p_simple_item)
			{
				ret = parse_SimpleItem(p_SimpleItem, &p_simple_item->SimpleItem);
				if (ONVIF_OK != ret)
				{
					onvif_free_simple_items(&p_req->Parameters.SimpleItem);
					break;
				}
			}
			
			p_SimpleItem = p_SimpleItem->next;
		}

		p_ElementItem = xml_node_soap_get(p_Parameters, "ElementItem");
		while (p_ElementItem && soap_strcmp(p_ElementItem->name, "ElementItem") == 0)
		{
			ONVIF_ELEMENTITEM * p_element_item = onvif_add_element_item(&p_req->Parameters.ElementItem);
			if (p_element_item)
			{
				ret = parse_ElementItem(p_ElementItem, &p_element_item->ElementItem);
				if (ONVIF_OK != ret)
				{
					onvif_free_element_items(&p_req->Parameters.ElementItem);
					break;
				}
			}
			
			p_ElementItem = p_ElementItem->next;
		}
	}

	return ONVIF_OK;
}

ONVIF_RET parse_CreateRules(XMLN * p_node, CreateRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_Rule;
	ONVIF_RET ret;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_Rule = xml_node_soap_get(p_node, "Rule");
	while (p_Rule && soap_strcmp(p_Rule->name, "Rule") == 0)
	{
		ONVIF_CONFIG * p_config = onvif_add_config(&p_req->Rule);
		if (p_config)
		{
			ret = parse_Config(p_Rule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_configs(&p_req->Rule);
				return ret;
			}
		}
		
		p_Rule = p_Rule->next;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_DeleteRules(XMLN * p_node, DeleteRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_RuleName;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_RuleName = xml_node_soap_get(p_node, "RuleName");
	while (p_RuleName && p_RuleName->data && soap_strcmp(p_RuleName->name, "RuleName") == 0)
	{	
		int idx = p_req->sizeRuleName;
		
		strncpy(p_req->RuleName[idx], p_RuleName->data, sizeof(p_req->RuleName[idx])-1);
		
		p_RuleName = p_RuleName->next;

		p_req->sizeRuleName++;
		if (p_req->sizeRuleName >= 10)
		{
			break;
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetRules(XMLN * p_node, GetRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_ModifyRules(XMLN * p_node, ModifyRules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_Rule;
	ONVIF_RET ret;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_Rule = xml_node_soap_get(p_node, "Rule");
	while (p_Rule && soap_strcmp(p_Rule->name, "Rule") == 0)
	{
		ONVIF_CONFIG * p_config = onvif_add_config(&p_req->Rule);
		if (p_config)
		{
			ret = parse_Config(p_Rule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_configs(&p_req->Rule);
				return ret;
			}
		}
		
		p_Rule = p_Rule->next;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_CreateAnalyticsModules(XMLN * p_node, CreateAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_AnalyticsModule;
	ONVIF_RET ret;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_AnalyticsModule = xml_node_soap_get(p_node, "AnalyticsModule");
	while (p_AnalyticsModule && soap_strcmp(p_AnalyticsModule->name, "AnalyticsModule") == 0)
	{
		ONVIF_CONFIG * p_config = onvif_add_config(&p_req->AnalyticsModule);
		if (p_config)
		{
			ret = parse_Config(p_AnalyticsModule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_configs(&p_req->AnalyticsModule);
				return ret;
			}
		}
		
		p_AnalyticsModule = p_AnalyticsModule->next;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_DeleteAnalyticsModules(XMLN * p_node, DeleteAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_AnalyticsModuleName;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_AnalyticsModuleName = xml_node_soap_get(p_node, "AnalyticsModuleName");
	while (p_AnalyticsModuleName && p_AnalyticsModuleName->data && soap_strcmp(p_AnalyticsModuleName->name, "AnalyticsModuleName") == 0)
	{	
		int idx = p_req->sizeAnalyticsModuleName;
		
		strncpy(p_req->AnalyticsModuleName[idx], p_AnalyticsModuleName->data, sizeof(p_req->AnalyticsModuleName[idx])-1);
		
		p_AnalyticsModuleName = p_AnalyticsModuleName->next;

		p_req->sizeAnalyticsModuleName++;
		if (p_req->sizeAnalyticsModuleName >= 10)
		{
			break;
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_GetAnalyticsModules(XMLN * p_node, GetAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_ModifyAnalyticsModules(XMLN * p_node, ModifyAnalyticsModules_REQ * p_req)
{
	XMLN * p_ConfigurationToken;
	XMLN * p_AnalyticsModule;
	ONVIF_RET ret;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	p_AnalyticsModule = xml_node_soap_get(p_node, "AnalyticsModule");
	while (p_AnalyticsModule && soap_strcmp(p_AnalyticsModule->name, "AnalyticsModule") == 0)
	{
		ONVIF_CONFIG * p_config = onvif_add_config(&p_req->AnalyticsModule);
		if (p_config)
		{
			ret = parse_Config(p_AnalyticsModule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_configs(&p_req->AnalyticsModule);
				return ret;
			}
		}
		
		p_AnalyticsModule = p_AnalyticsModule->next;
	}
	
	return ONVIF_OK;
}

ONVIF_RET parse_AddVideoAnalyticsConfiguration(XMLN * p_node, AddVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;
	XMLN * p_ConfigurationToken;

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_GetVideoAnalyticsConfiguration(XMLN * p_node, GetVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_ConfigurationToken;

	p_ConfigurationToken = xml_node_soap_get(p_node, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(p_req->ConfigurationToken, p_ConfigurationToken->data, sizeof(p_req->ConfigurationToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_RemoveVideoAnalyticsConfiguration(XMLN * p_node, RemoveVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_ProfileToken;

	p_ProfileToken = xml_node_soap_get(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(p_req->ProfileToken, p_ProfileToken->data, sizeof(p_req->ProfileToken)-1);
	}

	return ONVIF_OK;
}

ONVIF_RET parse_AnalyticsEngineConfiguration(XMLN * p_node, onvif_AnalyticsEngineConfiguration * p_req)
{
	XMLN * p_AnalyticsModule;
	ONVIF_CONFIG * p_config;
	ONVIF_RET ret;
	p_AnalyticsModule = xml_node_soap_get(p_node, "AnalyticsModule");
	while (p_AnalyticsModule && soap_strcmp(p_AnalyticsModule->name, "AnalyticsModule") == 0)
	{
		p_config = onvif_add_config(&p_req->AnalyticsModule);
		if (p_config)
		{
			ret = parse_Config(p_AnalyticsModule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_configs(&p_req->AnalyticsModule);
				return ret;
			}
		}
		
		p_AnalyticsModule = p_AnalyticsModule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_RuleEngineConfiguration(XMLN * p_node, onvif_RuleEngineConfiguration * p_req)
{
	XMLN * p_Rule;
	ONVIF_CONFIG * p_config;
	ONVIF_RET ret;
	p_Rule = xml_node_soap_get(p_node, "Rule");
	while (p_Rule && soap_strcmp(p_Rule->name, "Rule") == 0)
	{
		p_config = onvif_add_config(&p_req->Rule);
		if (p_config)
		{
			ret = parse_Config(p_Rule, &p_config->Config);
			if (ONVIF_OK != ret)
			{
				onvif_free_configs(&p_req->Rule);
				return ret;
			}
		}
		
		p_Rule = p_Rule->next;
	}

	return ONVIF_OK;
}

ONVIF_RET parse_VideoAnalyticsConfiguration(XMLN * p_node, onvif_VideoAnalyticsConfiguration * p_req)
{
	XMLN * p_Name;
	XMLN * p_UseCount;
	XMLN * p_AnalyticsEngineConfiguration;
	XMLN * p_RuleEngineConfiguration;	
	const char * p_token;
	ONVIF_RET ret = ONVIF_OK;

	p_token = xml_attr_get(p_node, "token");
	if (p_token)
	{
		strncpy(p_req->token, p_token, sizeof(p_req->token)-1);
	}
	
	p_Name = xml_node_soap_get(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(p_req->Name, p_Name->data, sizeof(p_req->Name)-1);
	}

	p_UseCount = xml_node_soap_get(p_node, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		p_req->UseCount = atoi(p_UseCount->data);
	}

	p_AnalyticsEngineConfiguration = xml_node_soap_get(p_node, "AnalyticsEngineConfiguration");
	if (p_AnalyticsEngineConfiguration)
	{
		ret = parse_AnalyticsEngineConfiguration(p_AnalyticsEngineConfiguration, &p_req->AnalyticsEngineConfiguration);
		if (ONVIF_OK != ret)
		{
			return ret;
		}
	}

	p_RuleEngineConfiguration = xml_node_soap_get(p_node, "RuleEngineConfiguration");
	if (p_RuleEngineConfiguration)
	{
		ret = parse_RuleEngineConfiguration(p_RuleEngineConfiguration, &p_req->RuleEngineConfiguration);
	}
	
	return ret;
}

ONVIF_RET parse_SetVideoAnalyticsConfiguration(XMLN * p_node, SetVideoAnalyticsConfiguration_REQ * p_req)
{
	XMLN * p_Configuration;
	XMLN * p_ForcePersistence;
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;

	p_Configuration = xml_node_soap_get(p_node, "Configuration");
	if (p_Configuration)
	{
		ret = parse_VideoAnalyticsConfiguration(p_Configuration, &p_req->Configuration);
	}

	p_ForcePersistence = xml_node_soap_get(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		p_req->ForcePersistence = parse_Bool(p_ForcePersistence->data);
	}
	
	return ret;
}

#endif	// end of VIDEO_ANALYTICS



