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
#include "onvif/onvif/onvif_cm.h"
#include "onvif/onvif/onvif_cfg.h"
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif_util.h"


/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;


/***************************************************************************************/

BOOL onvif_parse_user(XMLN * p_node, ONVIF_USER * p_user)
{
	XMLN * p_username;
    XMLN * p_password;
    XMLN * p_userlevel;
    
	p_username = xml_node_get(p_node, "username");
	if (p_username && p_username->data)
	{
		strncpy(p_user->Username, p_username->data, sizeof(p_user->Username)-1);
	}
	else
	{
		return FALSE;
	}

	p_password = xml_node_get(p_node, "password");
	if (p_password && p_password->data)
	{
		strncpy(p_user->Password, p_password->data, sizeof(p_user->Password)-1);
	}

	p_userlevel = xml_node_get(p_node, "userlevel");
	if (p_userlevel && p_userlevel->data)
	{
		p_user->UserLevel = onvif_StringToUserLevel(p_userlevel->data);
	}

	return TRUE;
}

void onvif_parse_h264_options(XMLN * p_video_encoder, ONVIF_V_ENC_CFG * p_v_enc_cfg)
{
	XMLN * p_h264;
	XMLN * p_gov_length;
	XMLN * p_h264_profile;

	p_h264 = xml_node_get(p_video_encoder, "h264");
	if (NULL == p_h264)
	{
		return;
	}

	p_v_enc_cfg->VideoEncoderConfiguration.H264Flag = 1;
	
	p_gov_length = xml_node_get(p_h264, "gov_length");
	if (p_gov_length && p_gov_length->data)
	{
		p_v_enc_cfg->VideoEncoderConfiguration.H264.GovLength = atoi(p_gov_length->data);
	}

	p_h264_profile = xml_node_get(p_h264, "h264_profile");
	if (p_h264_profile && p_h264_profile->data)
	{
		p_v_enc_cfg->VideoEncoderConfiguration.H264.H264Profile = onvif_StringToH264Profile(p_h264_profile->data);
	}
}

void onvif_parse_mpeg4_options(XMLN * p_video_encoder, ONVIF_V_ENC_CFG * p_v_enc_cfg)
{
	XMLN * p_mpeg4;
	XMLN * p_gov_length;
	XMLN * p_mpeg4_profile;

	p_mpeg4 = xml_node_get(p_video_encoder, "mpeg4");
	if (NULL == p_mpeg4)
	{
		return;
	}

	p_v_enc_cfg->VideoEncoderConfiguration.MPEG4Flag = 1;
	
	p_gov_length = xml_node_get(p_mpeg4, "gov_length");
	if (p_gov_length && p_gov_length->data)
	{
		p_v_enc_cfg->VideoEncoderConfiguration.MPEG4.GovLength = atoi(p_gov_length->data);
	}

	p_mpeg4_profile = xml_node_get(p_mpeg4, "mpeg4_profile");
	if (p_mpeg4_profile && p_mpeg4_profile->data)
	{
		p_v_enc_cfg->VideoEncoderConfiguration.MPEG4.Mpeg4Profile = onvif_StringToMpeg4Profile(p_mpeg4_profile->data);
	}
}



void onvif_parse_cfg(char * xml_buff, int rlen)
{
	XMLN * p_node;
	XMLN * p_serv_ip;
	XMLN * p_serv_port;
	XMLN * p_need_auth;
	XMLN * p_log_enable;
	XMLN * p_information;
	XMLN * p_user;
	XMLN * p_profile;
	XMLN * p_scope;
	XMLN * p_event;

	p_node = xxx_hxml_parse(xml_buff, rlen);
	if (NULL == p_node)
	{
		return;
	}

	xml_node_del(p_node);
}

void onvif_load_cfg()
{
	int len;
    int rlen;
    FILE * fp;
    char xml_buff[2048];

    // read config file
	
	fp = fopen("config.xml", "r");
	if (NULL == fp)
	{
		return;
	}
	
	fseek(fp, 0, SEEK_END);
	
	len = ftell(fp);
	if (len <= 0 || len >= 2048)
	{
		fclose(fp);
		return;
	}
	fseek(fp, 0, SEEK_SET);

	rlen = fread(xml_buff, 1, len, fp);
	
	fclose(fp);

	onvif_parse_cfg(xml_buff, rlen);
}




