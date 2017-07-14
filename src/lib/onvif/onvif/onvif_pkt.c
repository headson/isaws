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

/***************************************************************************************/
#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/onvif_device.h"
#include "onvif/onvif/onvif_pkt.h"
#include "onvif/onvif/onvif_event.h"
#include "onvif/onvif/onvif_ptz.h"
#include "onvif/onvif/onvif_util.h"
#include "onvif/onvif/onvif_err.h"
#include "onvif/onvif/onvif_media.h"
#include "onvif/onvif/onvif_image.h"
#include "onvif/onvif/onvif_local_datastruct.h"
#ifdef VIDEO_ANALYTICS
#include "onvif/onvif/onvif_analytics.h"
#endif
#ifdef PROFILE_G_SUPPORT
#include "onvif/onvif/onvif_recording.h"
#endif

#if __WIN32_OS__
#pragma warning(disable:4996)
#endif

/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;
#define  default_renew_time     60

/***************************************************************************************/
char xml_hdr[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

char onvif_xmlns[] = 
	"<s:Envelope "
    "xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
    "xmlns:e=\"http://www.w3.org/2003/05/soap-encoding\" "
    "xmlns:wsa=\"http://www.w3.org/2005/08/addressing\" " 
    "xmlns:tns1=\"http://www.onvif.org/ver10/topics\" "
    "xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
    "xmlns:tt=\"http://www.onvif.org/ver10/schema\" " 
    "xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" " 
    "xmlns:wstop=\"http://docs.oasis-open.org/wsn/t-1\" " 
    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" " 
    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" "
    "xmlns:tev=\"http://www.onvif.org/ver10/events/wsdl\" "
    "xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" "
    "xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\" "
#ifdef VIDEO_ANALYTICS
	"xmlns:tan=\"http://www.onvif.org/ver20/analytics/wsdl\" "
#endif
#ifdef PROFILE_G_SUPPORT    
    "xmlns:trp=\"http://www.onvif.org/ver10/replay/wsdl\" "
    "xmlns:tse=\"http://www.onvif.org/ver10/search/wsdl\" "
    "xmlns:trc=\"http://www.onvif.org/ver10/recording/wsdl\" "    
#endif
    "xmlns:wsrf-rw=\"http://docs.oasis-open.org/wsrf/rw-2\" "
    "xmlns:wsntw=\"http://docs.oasis-open.org/wsn/bw-2\" "
	"xmlns:ter=\"http://www.onvif.org/ver10/error\" >";

char soap_head[] = 
	"<s:Header>"
    	"<wsa:Action>%s</wsa:Action>"
	"</s:Header>";		

char soap_body[] = 
    "<s:Body>";

char soap_tailer[] =
    "</s:Body></s:Envelope>";
    
/***************************************************************************************/
	
#ifdef VIDEO_ANALYTICS
	int build_VideoAnalyticsConfiguration_xml(char * p_buf, int mlen, onvif_VideoAnalyticsConfiguration * p_req);
#endif
/***************************************************************************************/

int build_err_rly_xml
(
char * p_buf, 
int mlen, 
const char * code, 
const char * subcode, 
const char * subcode_ex, 
const char * reason
)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Fault><s:Code><s:Value>%s</s:Value>", code);
    offset += snprintf(p_buf+offset, mlen-offset, "<s:Subcode><s:Value>%s</s:Value>", subcode);
    if (NULL != subcode_ex)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<s:Subcode><s:Value>%s</s:Value></s:Subcode>", subcode_ex);
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</s:Subcode></s:Code>");			
	offset += snprintf(p_buf+offset, mlen-offset, "<s:Reason><s:Text xml:lang=\"en\">%s</s:Text></s:Reason></s:Fault>", reason);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}


int build_GetDeviceInformation_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetDeviceInformation_RES * p_res = (GetDeviceInformation_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetDeviceInformationResponse>"
		    "<tds:Manufacturer>%s</tds:Manufacturer>"
		    "<tds:Model>%s</tds:Model>"
		    "<tds:FirmwareVersion>%s</tds:FirmwareVersion>"
		    "<tds:SerialNumber>%s</tds:SerialNumber>"
		    "<tds:HardwareId>%s</tds:HardwareId>"
	    "</tds:GetDeviceInformationResponse>", 
    	p_res->DeviceInfo.Manufacturer, 
    	p_res->DeviceInfo.Model, 
    	p_res->DeviceInfo.FirmwareVersion, 
    	p_res->DeviceInfo.SerialNumber, 
    	p_res->DeviceInfo.HardwareId);
		
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_MulticastConfiguration_xml(char * p_buf, int mlen, onvif_MulticastConfiguration * p_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Multicast>"
			"<tt:Address>"
				"<tt:Type>IPv4</tt:Type>"
				"<tt:IPv4Address>%s</tt:IPv4Address>"
			"</tt:Address>"
			"<tt:Port>%d</tt:Port>"
			"<tt:TTL>%d</tt:TTL>"
			"<tt:AutoStart>%s</tt:AutoStart>"
		"</tt:Multicast>", 
	    p_cfg->IPv4Address,
	    p_cfg->Port,
	    p_cfg->TTL,
	    p_cfg->AutoStart ? "true" : "false");

	return offset;	    
}

int build_VideoEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_V_ENC_CFG * p_v_enc_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
		"<tt:UseCount>%d</tt:UseCount>"
	    "<tt:Encoding>%s</tt:Encoding>"
	    "<tt:Resolution>"
	    	"<tt:Width>%d</tt:Width>"
	    	"<tt:Height>%d</tt:Height>"
	    "</tt:Resolution>"
	    "<tt:Quality>%d</tt:Quality>",
	    p_v_enc_cfg->VideoEncoderConfiguration.Name, 
	    p_v_enc_cfg->VideoEncoderConfiguration.UseCount, 
	    onvif_VideoEncodingToString(p_v_enc_cfg->VideoEncoderConfiguration.Encoding), 
	    p_v_enc_cfg->VideoEncoderConfiguration.Resolution.Width, 
	    p_v_enc_cfg->VideoEncoderConfiguration.Resolution.Height, 
	    p_v_enc_cfg->VideoEncoderConfiguration.Quality);

	if (p_v_enc_cfg->VideoEncoderConfiguration.RateControlFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,     
		    "<tt:RateControl>"
		    	"<tt:FrameRateLimit>%d</tt:FrameRateLimit>"
		    	"<tt:EncodingInterval>%d</tt:EncodingInterval>"
		    	"<tt:BitrateLimit>%d</tt:BitrateLimit>"
		    "</tt:RateControl>",		    
		    p_v_enc_cfg->VideoEncoderConfiguration.RateControl.FrameRateLimit,
		    p_v_enc_cfg->VideoEncoderConfiguration.RateControl.EncodingInterval, 
		    p_v_enc_cfg->VideoEncoderConfiguration.RateControl.BitrateLimit);
	}
	
	if (VideoEncoding_H264 == p_v_enc_cfg->VideoEncoderConfiguration.Encoding)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:H264>"
				"<tt:GovLength>%d</tt:GovLength>"
    			"<tt:H264Profile>%s</tt:H264Profile>"
    		"</tt:H264>", 
    		p_v_enc_cfg->VideoEncoderConfiguration.H264.GovLength,
	    	onvif_H264ProfileToString(p_v_enc_cfg->VideoEncoderConfiguration.H264.H264Profile));
	}
	else if (VideoEncoding_MPEG4 == p_v_enc_cfg->VideoEncoderConfiguration.Encoding)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:MPEG4>"
				"<tt:GovLength>%d</tt:GovLength>"
    			"<tt:Mpeg4Profile>%s</tt:Mpeg4Profile>"
    		"</tt:MPEG4>", 
    		p_v_enc_cfg->VideoEncoderConfiguration.MPEG4.GovLength,
	    	onvif_Mpeg4ProfileToString(p_v_enc_cfg->VideoEncoderConfiguration.MPEG4.Mpeg4Profile));
	}

	offset += build_MulticastConfiguration_xml(p_buf+offset, mlen-offset, &p_v_enc_cfg->VideoEncoderConfiguration.Multicast);
	
	offset += snprintf(p_buf+offset, mlen-offset, 		
	    "<tt:SessionTimeout>PT%dS</tt:SessionTimeout>",
	    p_v_enc_cfg->VideoEncoderConfiguration.SessionTimeout);

	return offset;    
}

int build_PTZConfiguration_xml(char * p_buf, int mlen, PTZ_CFG * p_ptz_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset,
    	"<tt:Name>%s</tt:Name>"
    	"<tt:UseCount>%d</tt:UseCount>"
    	"<tt:NodeToken>%s</tt:NodeToken>",     	
    	p_ptz_cfg->PTZConfiguration.Name, 
    	p_ptz_cfg->PTZConfiguration.UseCount,
    	p_ptz_cfg->PTZConfiguration.NodeToken);

   offset += snprintf(p_buf+offset, mlen-offset,  	
	    "<tt:DefaultAbsolutePantTiltPositionSpace>"
	    	"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace"
    	"</tt:DefaultAbsolutePantTiltPositionSpace>"
	    "<tt:DefaultAbsoluteZoomPositionSpace>"
	    	"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace"
    	"</tt:DefaultAbsoluteZoomPositionSpace>"
	    "<tt:DefaultRelativePanTiltTranslationSpace>"
	    	"http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace"
    	"</tt:DefaultRelativePanTiltTranslationSpace>"
	    "<tt:DefaultRelativeZoomTranslationSpace>"
	    	"http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace"
	    "</tt:DefaultRelativeZoomTranslationSpace>"
	    "<tt:DefaultContinuousPanTiltVelocitySpace>"
	   	 	"http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace"
	    "</tt:DefaultContinuousPanTiltVelocitySpace>"
	    "<tt:DefaultContinuousZoomVelocitySpace>"
	    	"http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace"
	    "</tt:DefaultContinuousZoomVelocitySpace>");

	if (p_ptz_cfg->PTZConfiguration.DefaultPTZSpeedFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:DefaultPTZSpeed>"); 	    
		if (p_ptz_cfg->PTZConfiguration.DefaultPTZSpeed.PanTiltFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\" space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace\" />",
				p_ptz_cfg->PTZConfiguration.DefaultPTZSpeed.PanTilt.x, 
				p_ptz_cfg->PTZConfiguration.DefaultPTZSpeed.PanTilt.y);
		}
		if (p_ptz_cfg->PTZConfiguration.DefaultPTZSpeed.ZoomFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:Zoom x=\"%0.1f\" space=\"http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace\" />",
				p_ptz_cfg->PTZConfiguration.DefaultPTZSpeed.Zoom.x);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:DefaultPTZSpeed>"); 
	}

	if (p_ptz_cfg->PTZConfiguration.DefaultPTZTimeoutFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,  	 	
	    	"<tt:DefaultPTZTimeout>PT%dS</tt:DefaultPTZTimeout>", 
	    	p_ptz_cfg->PTZConfiguration.DefaultPTZTimeout);
    }	

	if (p_ptz_cfg->PTZConfiguration.PanTiltLimitsFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset,  	 	
			"<tt:PanTiltLimits>"
				"<tt:Range>"
					"<tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace</tt:URI>"
					"<tt:XRange>"
						"<tt:Min>%0.1f</tt:Min>"
						"<tt:Max>%0.1f</tt:Max>"
					"</tt:XRange>"
					"<tt:YRange>"
						"<tt:Min>%0.1f</tt:Min>"
						"<tt:Max>%0.1f</tt:Max>"
					"</tt:YRange>"
				"</tt:Range>"
			"</tt:PanTiltLimits>",
			p_ptz_cfg->PTZConfiguration.PanTiltLimits.XRange.Min, 
			p_ptz_cfg->PTZConfiguration.PanTiltLimits.XRange.Max,
			p_ptz_cfg->PTZConfiguration.PanTiltLimits.YRange.Min, 
			p_ptz_cfg->PTZConfiguration.PanTiltLimits.YRange.Max);
	}

	if (p_ptz_cfg->PTZConfiguration.ZoomLimitsFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,  	 	
			"<tt:ZoomLimits>"
				"<tt:Range>"
					"<tt:URI>http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace</tt:URI>"
					"<tt:XRange>"
						"<tt:Min>%0.1f</tt:Min>"
						"<tt:Max>%0.1f</tt:Max>"
					"</tt:XRange>"
				"</tt:Range>"
			"</tt:ZoomLimits>",
			p_ptz_cfg->PTZConfiguration.ZoomLimits.XRange.Min,
			p_ptz_cfg->PTZConfiguration.ZoomLimits.XRange.Max);
	}

    if (p_ptz_cfg->PTZConfiguration.ExtensionFlag)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension>");
        if (p_ptz_cfg->PTZConfiguration.Extension.PTControlDirectionFlag)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTControlDirection>");
            if (p_ptz_cfg->PTZConfiguration.Extension.PTControlDirection.EFlipFlag)
            {
                offset += snprintf(p_buf+offset, mlen-offset, 
                    "<tt:EFlip>"
                        "<tt:Mode>%s</tt:Mode>"
                    "</tt:EFlip>",
                    onvif_EFlipModeToString(p_ptz_cfg->PTZConfiguration.Extension.PTControlDirection.EFlip));                
            }
            if (p_ptz_cfg->PTZConfiguration.Extension.PTControlDirection.ReverseFlag)
            {
                offset += snprintf(p_buf+offset, mlen-offset, 
                    "<tt:Reverse>"
                        "<tt:Mode>%s</tt:Mode>"
                    "</tt:Reverse>",
                    onvif_ReverseModeToString(p_ptz_cfg->PTZConfiguration.Extension.PTControlDirection.Reverse));
            }
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:PTControlDirection>");
        }    
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Extension>");
    }

	return offset;
}

int build_VideoSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_V_SRC_CFG * p_v_src_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
	    "<tt:UseCount>%d</tt:UseCount>"
	    "<tt:SourceToken>%s</tt:SourceToken>"
	    "<tt:Bounds height=\"%d\" width=\"%d\" y=\"%d\" x=\"%d\" />",
    	p_v_src_cfg->VideoSourceConfiguration.Name, 
        p_v_src_cfg->VideoSourceConfiguration.UseCount, 
        p_v_src_cfg->VideoSourceConfiguration.SourceToken, 
        p_v_src_cfg->VideoSourceConfiguration.Bounds.height, 
        p_v_src_cfg->VideoSourceConfiguration.Bounds.width, 
        p_v_src_cfg->VideoSourceConfiguration.Bounds.y, 
        p_v_src_cfg->VideoSourceConfiguration.Bounds.x);

	return offset;            
}

int build_AudioSourceConfiguration_xml(char * p_buf, int mlen, ONVIF_A_SRC_CFG * p_a_src_cfg)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:Name>%s</tt:Name>"
	    "<tt:UseCount>%d</tt:UseCount>"
	    "<tt:SourceToken>%s</tt:SourceToken>", 
		p_a_src_cfg->AudioSourceConfiguration.Name, 
        p_a_src_cfg->AudioSourceConfiguration.UseCount, 
        p_a_src_cfg->AudioSourceConfiguration.SourceToken);

	return offset;            
}

int build_AudioEncoderConfiguration_xml(char * p_buf, int mlen, ONVIF_A_ENC_CFG * p_a_enc_cfg)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
		"<tt:UseCount>%d</tt:UseCount>"
		"<tt:Encoding>%s</tt:Encoding>"
		"<tt:Bitrate>%d</tt:Bitrate>"
		"<tt:SampleRate>%d</tt:SampleRate>", 
		p_a_enc_cfg->AudioEncoderConfiguration.Name, 
    	p_a_enc_cfg->AudioEncoderConfiguration.UseCount, 
    	onvif_AudioEncodingToString(p_a_enc_cfg->AudioEncoderConfiguration.Encoding), 
	    p_a_enc_cfg->AudioEncoderConfiguration.Bitrate, 
	    p_a_enc_cfg->AudioEncoderConfiguration.SampleRate); 

	offset += build_MulticastConfiguration_xml(p_buf+offset, mlen-offset, &p_a_enc_cfg->AudioEncoderConfiguration.Multicast);

	offset += snprintf(p_buf+offset, mlen-offset, 		
	    "<tt:SessionTimeout>PT%dS</tt:SessionTimeout>", 
		p_a_enc_cfg->AudioEncoderConfiguration.SessionTimeout);
		
	return offset;    	    
}

int build_MetadataConfiguration_xml(char * p_buf, int mlen, ONVIF_METADATA_CFG * p_cfg)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:Name>%s</tt:Name>"
    	"<tt:UseCount>%d</tt:UseCount>",
    	p_cfg->MetadataConfiguration.Name,
    	p_cfg->MetadataConfiguration.UseCount);

	if (p_cfg->MetadataConfiguration.PTZStatusFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:PTZStatus>"
				"<tt:Status>%s</tt:Status>"
				"<tt:Position>%s</tt:Position>"
			"</tt:PTZStatus>",
	    	p_cfg->MetadataConfiguration.PTZStatus.Status ? "true" : "false",
	    	p_cfg->MetadataConfiguration.PTZStatus.Position ? "true" : "false");
	}

	if (p_cfg->MetadataConfiguration.AnalyticsFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:Analytics>%s</tt:Analytics>",
	    	p_cfg->MetadataConfiguration.Analytics ? "true" : "false");
	}

    offset += build_MulticastConfiguration_xml(p_buf+offset, mlen-offset, &p_cfg->MetadataConfiguration.Multicast);

	offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:SessionTimeout>PT%dS</tt:SessionTimeout>",
	    	p_cfg->MetadataConfiguration.SessionTimeout);

    return offset;
}

int build_Profile_xml(char * p_buf, int mlen, ONVIF_PROFILE * p_profile)
{
	int offset = 0;
	
	if (p_profile->v_src_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
        	"<tt:VideoSourceConfiguration token=\"%s\">", 
            p_profile->v_src_cfg->VideoSourceConfiguration.token);            
        offset += build_VideoSourceConfiguration_xml(p_buf+offset, mlen-offset, p_profile->v_src_cfg);    
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoSourceConfiguration>");	            
    }

    if (p_profile->a_src_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:AudioSourceConfiguration token=\"%s\">",
            p_profile->a_src_cfg->AudioSourceConfiguration.token);
        offset += build_AudioSourceConfiguration_xml(p_buf+offset, mlen-offset, p_profile->a_src_cfg);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioSourceConfiguration>");	            
    }

    if (p_profile->v_enc_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
        	"<tt:VideoEncoderConfiguration token=\"%s\">", 
        	p_profile->v_enc_cfg->VideoEncoderConfiguration.token);
		offset += build_VideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_profile->v_enc_cfg);        	    
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoEncoderConfiguration>");	            
    }

    if (p_profile->a_enc_cfg)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:AudioEncoderConfiguration token=\"%s\">", 
            p_profile->a_enc_cfg->AudioEncoderConfiguration.token);
        offset += build_AudioEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_profile->a_enc_cfg);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioEncoderConfiguration>");	            
    }

    if (p_profile->ptz_cfg)
    {
    	offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:PTZConfiguration token=\"%s\">", 
            p_profile->ptz_cfg->PTZConfiguration.token);
    	offset += build_PTZConfiguration_xml(p_buf+offset, mlen-offset, p_profile->ptz_cfg);
    	offset += snprintf(p_buf+offset, mlen-offset, "</tt:PTZConfiguration>");
    }

    if (p_profile->metadata_cfg)
    {
    	offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:MetadataConfiguration token=\"%s\">", 
            p_profile->metadata_cfg->MetadataConfiguration.token);
        offset += build_MetadataConfiguration_xml(p_buf+offset, mlen-offset, p_profile->metadata_cfg);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:MetadataConfiguration>");    
    }
#ifdef VIDEO_ANALYTICS
	if (p_profile->va_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoAnalyticsConfiguration token=\"%s\">", p_profile->va_cfg->VideoAnalyticsConfiguration.token);
		offset += build_VideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_profile->va_cfg->VideoAnalyticsConfiguration);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoAnalyticsConfiguration>");
	}
#endif
    return offset;
}
    
int build_GetProfiles_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetProfiles_RES * p_res = (GetProfiles_RES *)argv;
    ONVIF_PROFILE * profile = p_res->profiles;
    
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetProfilesResponse>");
	
	while (profile)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	        "<trt:Profiles fixed=\"%s\" token=\"%s\"><tt:Name>%s</tt:Name>",
	        profile->fixed ? "true" : "false", profile->token, profile->name);

	    offset += build_Profile_xml(p_buf+offset, mlen-offset, profile);
	    
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profiles>");

	    profile = profile->next;
    }
    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetProfilesResponse>");            
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{	
	int offset;
	
	GetProfile_RES * p_res = (GetProfile_RES *)argv;
    ONVIF_PROFILE * profile = &p_res->profile;
    if (NULL == profile)
    {
    	return ONVIF_ERR_NO_PROFILE;
    }
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetProfileResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<trt:Profile fixed=\"%s\" token=\"%s\"><tt:Name>%s</tt:Name>",
        profile->fixed ? "true" : "false", profile->token, profile->name);

   	offset += build_Profile_xml(p_buf+offset, mlen-offset, profile);
	    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profile>");

    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetProfileResponse>");            
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreateProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	 
	CreateProfile_RES * p_res = (CreateProfile_RES *)argv;
	ONVIF_PROFILE * profile = &p_res->struprofile;
    if (NULL == profile)
    {
    	return -1;
    }
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:CreateProfileResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<trt:Profile fixed=\"%s\" token=\"%s\"><tt:Name>%s</tt:Name>",
        profile->fixed ? "true" : "false", profile->token, profile->name);

    offset += build_Profile_xml(p_buf+offset, mlen-offset, profile);
    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profile>");

    
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:CreateProfileResponse>");            
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_DeleteProfile_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:DeleteProfileResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_AddVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoSourceConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemoveVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoSourceConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_AddAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddAudioSourceConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemoveAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveAudioSourceConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_AddVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoEncoderConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemoveVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoEncoderConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_AddAudioEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddAudioEncoderConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemoveAudioEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveAudioEncoderConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetStreamUri_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	GetStreamUri_RES * p_res = (GetStreamUri_RES *)argv;
 
	offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<trt:GetStreamUriResponse>"
		    "<trt:MediaUri>"
			    "<tt:Uri>%s</tt:Uri>"
			    "<tt:InvalidAfterConnect>false</tt:InvalidAfterConnect>"
			    "<tt:InvalidAfterReboot>false</tt:InvalidAfterReboot>"
			    "<tt:Timeout>PT60S</tt:Timeout>"
		    "</trt:MediaUri>"
	    "</trt:GetStreamUriResponse>", p_res->stream_uri);

    printf("rtspuri : %s\n", p_res->stream_uri);
    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetSnapshotUri_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
		
	GetSnapshotUri_RES * p_res = (GetSnapshotUri_RES *)argv;
    
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
        "<trt:GetSnapshotUriResponse>"
            "<trt:MediaUri>"
                "<tt:Uri>%s</tt:Uri>"
                "<tt:InvalidAfterConnect>false</tt:InvalidAfterConnect>"
                "<tt:InvalidAfterReboot>false</tt:InvalidAfterReboot>"
                "<tt:Timeout>PT60S</tt:Timeout>"
            "</trt:MediaUri>"
        "</trt:GetSnapshotUriResponse>",
        p_res->stream_uri);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_MediaCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	onvif_MediaCapabilities * p_res = (onvif_MediaCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Media>"
		    "<tt:XAddr>%s</tt:XAddr>"
		    "<tt:StreamingCapabilities>"
			    "<tt:RTPMulticast>%s</tt:RTPMulticast>"
			    "<tt:RTP_TCP>%s</tt:RTP_TCP>"
			    "<tt:RTP_RTSP_TCP>%s</tt:RTP_RTSP_TCP>"
		    "</tt:StreamingCapabilities>"
		    "<tt:Extension>"
				"<tt:ProfileCapabilities>"
					"<tt:MaximumNumberOfProfiles>%d</tt:MaximumNumberOfProfiles>"
				"</tt:ProfileCapabilities>"
			"</tt:Extension>"
    	"</tt:Media>",		    
    	p_res->XAddr,
    	p_res->RTPMulticast ? "true" : "false",
    	p_res->RTP_TCP ? "true" : "false",
    	p_res->RTP_RTSP_TCP ? "true" : "false",
    	p_res->MaximumNumberOfProfiles);

	return offset;    	
}

int build_DeviceCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	onvif_DevicesCapabilities * p_res = (onvif_DevicesCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Device>"
    	"<tt:XAddr>%s</tt:XAddr>",
    	p_res->XAddr);    	
		    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Network>"
		    "<tt:IPFilter>%s</tt:IPFilter>"
		    "<tt:ZeroConfiguration>%s</tt:ZeroConfiguration>"
		    "<tt:IPVersion6>%s</tt:IPVersion6>"
		    "<tt:DynDNS>%s</tt:DynDNS>"
		    "<tt:Extension>"
		    	"<tt:Dot11Configuration>%s</tt:Dot11Configuration>"       
  			"</tt:Extension>"
	    "</tt:Network>",	    
    	p_res->IPFilter ? "true" : "false",
    	p_res->ZeroConfiguration ? "true" : "false",
    	p_res->IPVersion6 ? "true" : "false",
    	p_res->DynDNS ? "true" : "false",
    	p_res->Dot11Configuration ? "true" : "false");

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:System>"
		    "<tt:DiscoveryResolve>%s</tt:DiscoveryResolve>"
		    "<tt:DiscoveryBye>%s</tt:DiscoveryBye>"
		    "<tt:RemoteDiscovery>%s</tt:RemoteDiscovery>"
		    "<tt:SystemBackup>%s</tt:SystemBackup>"
		    "<tt:SystemLogging>%s</tt:SystemLogging>"
		    "<tt:FirmwareUpgrade>%s</tt:FirmwareUpgrade>"
		    "<tt:SupportedVersions>"
			    "<tt:Major>2</tt:Major>"
			    "<tt:Minor>42</tt:Minor>"
		    "</tt:SupportedVersions>"
		    "<tt:SupportedVersions>"
			    "<tt:Major>2</tt:Major>"
			    "<tt:Minor>20</tt:Minor>"
		    "</tt:SupportedVersions>"
		    "<tt:SupportedVersions>"
			    "<tt:Major>2</tt:Major>"
			    "<tt:Minor>10</tt:Minor>"
		    "</tt:SupportedVersions>"
		    "<tt:SupportedVersions>"
			    "<tt:Major>2</tt:Major>"
			    "<tt:Minor>0</tt:Minor>"
		    "</tt:SupportedVersions>"
		    "<tt:Extension>"
				"<tt:HttpFirmwareUpgrade>%s</tt:HttpFirmwareUpgrade>"
				"<tt:HttpSystemBackup>%s</tt:HttpSystemBackup>"
				"<tt:HttpSystemLogging>%s</tt:HttpSystemLogging>"
				"<tt:HttpSupportInformation>%s</tt:HttpSupportInformation>"
			"</tt:Extension>"
	    "</tt:System>",
	    p_res->DiscoveryResolve ? "true" : "false",
	    p_res->DiscoveryBye ? "true" : "false",
	    p_res->RemoteDiscovery ? "true" : "false",
	    p_res->SystemBackup ? "true" : "false",
	    p_res->SystemLogging ? "true" : "false",
	    p_res->FirmwareUpgrade ? "true" : "false",
	    p_res->HttpFirmwareUpgrade ? "true" : "false",
	    p_res->HttpSystemBackup ? "true" : "false",
	    p_res->HttpSystemLogging ? "true" : "false",
	    p_res->HttpSupportInformation ? "true" : "false");    

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:IO>"
			"<tt:InputConnectors>0</tt:InputConnectors>"
			"<tt:RelayOutputs>0</tt:RelayOutputs>"      
		"</tt:IO>");
		
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Security>"
			"<tt:TLS1.1>%s</tt:TLS1.1>"
			"<tt:TLS1.2>%s</tt:TLS1.2>"
			"<tt:OnboardKeyGeneration>%s</tt:OnboardKeyGeneration>"
			"<tt:AccessPolicyConfig>%s</tt:AccessPolicyConfig>"
			"<tt:X.509Token>%s</tt:X.509Token>"
			"<tt:SAMLToken>%s</tt:SAMLToken>"
			"<tt:KerberosToken>%s</tt:KerberosToken>"
			"<tt:RELToken>%s</tt:RELToken>"
			"<tt:Extension>"
				"<tt:TLS1.0>%s</tt:TLS1.0>"
				"<tt:Extension>"
					"<tt:Dot1X>%s</tt:Dot1X>"					
					"<tt:SupportedEAPMethod>%d</tt:SupportedEAPMethod>"
					"<tt:RemoteUserHandling>%s</tt:RemoteUserHandling>"
				"</tt:Extension>"
			"</tt:Extension>"	
		"</tt:Security>",
		p_res->TLS11 ? "true" : "false",
		p_res->TLS12 ? "true" : "false",
		p_res->OnboardKeyGeneration ? "true" : "false",
		p_res->AccessPolicyConfig ? "true" : "false",
		p_res->X509Token ? "true" : "false",
		p_res->SAMLToken ? "true" : "false",
		p_res->KerberosToken ? "true" : "false",
		p_res->RELToken ? "true" : "false",
		p_res->TLS10 ? "true" : "false",
		p_res->Dot1X ? "true" : "false",
		p_res->SupportedEAPMethods,
		p_res->RemoteUserHandling ? "true" : "false");
		
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Device>");
	
	return offset;    	
}

int build_EventsCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	onvif_EventCapabilities * p_res = (onvif_EventCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Events>"
		    "<tt:XAddr>%s</tt:XAddr>"
		    "<tt:WSSubscriptionPolicySupport>%s</tt:WSSubscriptionPolicySupport>"
		    "<tt:WSPullPointSupport>%s</tt:WSPullPointSupport>"
		    "<tt:WSPausableSubscriptionManagerInterfaceSupport>%s</tt:WSPausableSubscriptionManagerInterfaceSupport>"
	    "</tt:Events>",		    
    	p_res->XAddr,
    	p_res->WSSubscriptionPolicySupport ? "true" : "false",
    	p_res->WSPullPointSupport ? "true" : "false",
    	p_res->WSPausableSubscriptionManagerInterfaceSupport ? "true" : "false");

	return offset;    	
}

int build_ImagingCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	onvif_ImagingCapabilities * p_res = (onvif_ImagingCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Imaging>"
    		"<tt:XAddr>%s</tt:XAddr>"
    	"</tt:Imaging>",		    
    	p_res->XAddr);

	return offset;    	
}

int build_PTZCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	onvif_PTZCapabilities * p_res = (onvif_PTZCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:PTZ>"
    		"<tt:XAddr>%s</tt:XAddr>"
    	"</tt:PTZ>",		    
    	p_res->XAddr);

	return offset;    	
}

#ifdef VIDEO_ANALYTICS
int build_AnalyticsCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	onvif_AnalyticsCapabilities * p_res = (onvif_AnalyticsCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:Analytics>"
    		"<tt:XAddr>%s</tt:XAddr>"
    		"<tt:RuleSupport>%s</tt:RuleSupport>"
		 	"<tt:AnalyticsModuleSupport>%s</tt:AnalyticsModuleSupport>"
		 	"<tt:CellBasedSceneDescriptionSupported>%s</tt:CellBasedSceneDescriptionSupported>"
    	"</tt:Analytics>",		    
    	p_res->XAddr,
    	p_res->RuleSupport ? "true" : "false",
    	p_res->AnalyticsModuleSupport ? "true" : "false",
    	p_res->CellBasedSceneDescriptionSupported ? "true" : "false");

	return offset;
}
#endif

#ifdef PROFILE_G_SUPPORT

int build_RecordingCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	onvif_RecordingCapabilities * p_res = (onvif_RecordingCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Recording>"
			"<tt:XAddr>%s</tt:XAddr>"
			"<tt:ReceiverSource>%s</tt:ReceiverSource>"
			"<tt:MediaProfileSource>%s</tt:MediaProfileSource>"
			"<tt:DynamicRecordings>%s</tt:DynamicRecordings>"
			"<tt:DynamicTracks>%s</tt:DynamicTracks>"
			"<tt:MaxStringLength>%d</tt:MaxStringLength>"
		"</tt:Recording>",		    
    	p_res->XAddr,
    	p_res->ReceiverSource ? "true" : "false",
    	p_res->MediaProfileSource ? "true" : "false",
    	p_res->DynamicRecordings ? "true" : "false", 
    	p_res->DynamicTracks ? "true" : "false",
    	p_res->MaxStringLength);

	return offset;    	
}

int build_SearchCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	onvif_SearchCapabilities * p_res = (onvif_SearchCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Search>"
			"<tt:XAddr>%s</tt:XAddr>"
			"<tt:MetadataSearch>%s</tt:MetadataSearch>"
		"</tt:Search>",		    
    	p_res->XAddr,
    	p_res->MetadataSearch ? "true" : "false");

	return offset;    	
}

int build_ReplayCapabilities_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	onvif_ReplayCapabilities * p_res = (onvif_ReplayCapabilities *)argv;
    offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Replay>"
			"<tt:XAddr>%s</tt:XAddr>"
		"</tt:Replay>",		    
    	p_res->XAddr);

	return offset;
}

#endif

int build_GetCapabilities_rly_xml(char * p_buf, int mlen, const char * argv)
{
	 
	GetCapabilities_RES * p_res = (GetCapabilities_RES *)argv;
	onvif_CapabilityCategory category = p_res->Category;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetCapabilitiesResponse><tds:Capabilities>");

	if (CapabilityCategory_Media == category)
	{
	    offset += build_MediaCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.media);
	}
	else if (CapabilityCategory_Device == category)
	{
	    offset += build_DeviceCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.device);
	}
    else if (CapabilityCategory_Events == category)
    {
        offset += build_EventsCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.events);
    }
    else if (CapabilityCategory_Imaging == category)
    {
        offset += build_ImagingCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.image);
    }
    else if (CapabilityCategory_PTZ == category)
    {
        offset += build_PTZCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.ptz);
    }
#ifdef VIDEO_ANALYTICS
	else if (CapabilityCategory_Analytics == category)
    {
        offset += build_AnalyticsCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.analytics);
    }
#endif    
	else if (CapabilityCategory_All == category)
	{
#ifdef VIDEO_ANALYTICS
		offset += build_AnalyticsCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.analytics);
#endif	
	    offset += build_DeviceCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.device);
	    offset += build_EventsCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.events);
	    offset += build_ImagingCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.image);
	    offset += build_MediaCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.media);	
	    offset += build_PTZCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.ptz);

	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension>");
#ifdef PROFILE_G_SUPPORT	    
	    if (p_res->Capabilities.recording.support)
	    {
	    	offset += build_RecordingCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.recording);
	    }
	    if (p_res->Capabilities.search.support)
	    {
	    	offset += build_SearchCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.search);
	    }
	    if (p_res->Capabilities.replay.support)
	    {
	    	offset += build_ReplayCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.replay);
	    }
#endif	    
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Extension>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities></tds:GetCapabilitiesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetNetworkInterfaces_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetNetworkInterfaces_RES *p_res = (GetNetworkInterfaces_RES *)argv; 
	ONVIF_NET_INF * p_net_inf = p_res->interfaces;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkInterfacesResponse>");
	
	while (p_net_inf)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkInterfaces token=\"%s\">", p_net_inf->NetworkInterface.token);			
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Enabled>%s</tt:Enabled>", p_net_inf->NetworkInterface.Enabled ? "true" : "false");
		
		if (p_net_inf->NetworkInterface.InfoFlag)
		{
		    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Info>");
			if (p_net_inf->NetworkInterface.Info.NameFlag)
			{
			    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Name>%s</tt:Name>", p_net_inf->NetworkInterface.Info.Name);
			}    
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:HwAddress>%s</tt:HwAddress>", p_net_inf->NetworkInterface.Info.HwAddress);
			if (p_net_inf->NetworkInterface.Info.MTUFlag)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:MTU>%d</tt:MTU>", p_net_inf->NetworkInterface.Info.MTU);
			}	
		    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Info>");
		}

		if (p_net_inf->NetworkInterface.IPv4Flag)
		{
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4>");
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Enabled>%s</tt:Enabled>", p_net_inf->NetworkInterface.IPv4.Enabled ? "true" : "false");
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Config>");

			if (p_net_inf->NetworkInterface.IPv4.Config.DHCP == FALSE)
			{
			    offset += snprintf(p_buf+offset, mlen-offset, 
			        "<tt:Manual>"
				        "<tt:Address>%s</tt:Address>"
				        "<tt:PrefixLength>%d</tt:PrefixLength>"
			        "</tt:Manual>",
			        p_net_inf->NetworkInterface.IPv4.Config.Address, 
			        p_net_inf->NetworkInterface.IPv4.Config.PrefixLength);				
			}
			else
			{
			    offset += snprintf(p_buf+offset, mlen-offset, 
			        "<tt:FromDHCP>"
				        "<tt:Address>%s</tt:Address>"
				        "<tt:PrefixLength>%d</tt:PrefixLength>"
			        "</tt:FromDHCP>",
			        p_net_inf->NetworkInterface.IPv4.Config.Address, 
			        p_net_inf->NetworkInterface.IPv4.Config.PrefixLength);
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DHCP>%s</tt:DHCP>", p_net_inf->NetworkInterface.IPv4.Config.DHCP ? "true" : "false");

			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Config>");
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:IPv4>");
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkInterfaces>");
		
		p_net_inf = p_net_inf->next;
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetNetworkInterfacesResponse>");
		
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetNetworkInterfaces_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SetNetworkInterfacesResponse>"
			"<tds:RebootNeeded>false</tds:RebootNeeded>"
		"</tds:SetNetworkInterfacesResponse>");		
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_ImageSettings_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = 0;
	onvif_ImagingSettings * p_res = (onvif_ImagingSettings *)argv;
	
    if (p_res->BacklightCompensationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:BacklightCompensation>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_BacklightCompensationModeToString(p_res->BacklightCompensation.Mode));
		if (p_res->BacklightCompensation.LevelFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Level>%0.1f</tt:Level>", p_res->BacklightCompensation.Level);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:BacklightCompensation>");
	}

	if (p_res->BrightnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Brightness>%0.1f</tt:Brightness>", p_res->Brightness);
	}
	if (p_res->ColorSaturationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ColorSaturation>%0.1f</tt:ColorSaturation>", p_res->ColorSaturation);
	}
	if (p_res->ContrastFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Contrast>%0.1f</tt:Contrast>", p_res->Contrast);
	}

	if (p_res->ExposureFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Exposure>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_ExposureModeToString(p_res->Exposure.Mode));
		if (p_res->Exposure.PriorityFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Priority>%s</tt:Priority>", onvif_ExposurePriorityToString(p_res->Exposure.Priority));
		}
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Window bottom=\"%0.1d\" top=\"%0.1d\" right=\"%0.1d\" left=\"%0.1d\"></tt:Window>",
            p_res->Exposure.Window.bottom, p_res->Exposure.Window.top,
            p_res->Exposure.Window.right, p_res->Exposure.Window.left);
		
		if (p_res->Exposure.MinExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MinExposureTime>%0.1f</tt:MinExposureTime>", p_res->Exposure.MinExposureTime);
		}
		if (p_res->Exposure.MaxExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaxExposureTime>%0.1f</tt:MaxExposureTime>", p_res->Exposure.MaxExposureTime);
		}
		if (p_res->Exposure.MinGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MinGain>%0.1f</tt:MinGain>", p_res->Exposure.MinGain);
		}
		if (p_res->Exposure.MaxGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaxGain>%0.1f</tt:MaxGain>", p_res->Exposure.MaxGain);
		}
		if (p_res->Exposure.MinIrisFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MinIris>%0.1f</tt:MinIris>", p_res->Exposure.MinIris);
		}
		if (p_res->Exposure.MaxIrisFlag)
		{
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaxIris>%0.1f</tt:MaxIris>", p_res->Exposure.MaxIris);
	    }	
	    if (p_res->Exposure.ExposureTimeFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:ExposureTime>%0.1f</tt:ExposureTime>", p_res->Exposure.ExposureTime);
	    }	
	    if (p_res->Exposure.GainFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Gain>%0.1f</tt:Gain>", p_res->Exposure.Gain);
	    }	
	    if (p_res->Exposure.IrisFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Iris>%0.1f</tt:Iris>", p_res->Exposure.Iris);
	    }
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Exposure>");			
	}

	if (p_res->FocusFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Focus>");
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:AutoFocusMode>%s</tt:AutoFocusMode>", onvif_AutoFocusModeToString(p_res->Focus.AutoFocusMode));
	    if (p_res->Focus.DefaultSpeedFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:DefaultSpeed>%0.1f</tt:DefaultSpeed>", p_res->Focus.DefaultSpeed);
	    }
	    if (p_res->Focus.NearLimitFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:NearLimit>%0.1f</tt:NearLimit>", p_res->Focus.NearLimit);
	    }
	    if (p_res->Focus.FarLimitFlag)
	    {
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:FarLimit>%0.1f</tt:FarLimit>", p_res->Focus.FarLimit);
	    }	
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Focus>");
    }

    if (p_res->IrCutFilterFlag)
    {
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilter>%s</tt:IrCutFilter>", onvif_IrCutFilterModeToString(p_res->IrCutFilter));
	}

	if (p_res->SharpnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Sharpness>%0.1f</tt:Sharpness>", p_res->Sharpness);
	}

	if (p_res->WideDynamicRangeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:WideDynamicRange>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_WideDynamicModeToString(p_res->WideDynamicRange.Mode));
		if (p_res->WideDynamicRange.LevelFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Level>%0.1f</tt:Level>", p_res->WideDynamicRange.Level);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:WideDynamicRange>");	
	}

	if (p_res->WhiteBalanceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:WhiteBalance>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>%s</tt:Mode>", onvif_WhiteBalanceModeToString(p_res->WhiteBalance.Mode));
		if (p_res->WhiteBalance.CrGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:CrGain>%0.1f</tt:CrGain>", p_res->WhiteBalance.CrGain);
		}
		if (p_res->WhiteBalance.CbGainFlag)
		{
	    	offset += snprintf(p_buf+offset, mlen-offset, "<tt:CbGain>%0.1f</tt:CbGain>", p_res->WhiteBalance.CbGain);
	    }	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:WhiteBalance>");	
	}

	return offset;
}

int build_GetVideoSources_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetVideoSources_RES * p_res = (GetVideoSources_RES *)argv;
	ONVIF_V_SRC * p_v_src = p_res->p_v_src;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourcesResponse>");

	while (p_v_src)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	    	"<trt:VideoSources token=\"%s\">"
		    	"<tt:Framerate>%0.1f</tt:Framerate>"
			    "<tt:Resolution>"
				    "<tt:Width>%d</tt:Width>"
					"<tt:Height>%d</tt:Height>"
				"</tt:Resolution>", 
			p_v_src->VideoSource.token, 
			p_v_src->VideoSource.Framerate, 
			p_v_src->VideoSource.Resolution.Width, 
			p_v_src->VideoSource.Resolution.Height); 
			
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Imaging>");
		offset += build_ImageSettings_xml(p_buf+offset, mlen-offset, (char *)&p_v_src->VideoSource.Imaging);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Imaging>");
		
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:VideoSources>");
	    
	    p_v_src = p_v_src->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourcesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetAudioSources_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetAudioSources_RES * p_res = (GetAudioSources_RES *)argv;
	ONVIF_A_SRC * p_a_src = p_res->a_src;
    
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourcesResponse>");
	
	while (p_a_src)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	    	"<trt:AudioSources token=\"%s\">"
	    		"<tt:Channels>%d</tt:Channels>"
	    	"</trt:AudioSources>", 
	    	p_a_src->AudioSource.token,
	    	p_a_src->AudioSource.Channels);
	    
	    p_a_src = p_a_src->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourcesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}


int build_GetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	Get_v_enc_cfg_RES * p_res = (Get_v_enc_cfg_RES *)argv;
	ONVIF_V_ENC_CFG * p_v_enc_cfg = &p_res->v_enc_cfg;
    if (NULL == p_v_enc_cfg)
    {
    	return ONVIF_ERR_NO_CONFIG;
    }
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_v_enc_cfg->VideoEncoderConfiguration.token);
    offset += build_VideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_v_enc_cfg);	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	
	GetAudioEncoderCfg_RES * p_res = (GetAudioEncoderCfg_RES *)argv;
	ONVIF_A_ENC_CFG * p_a_enc_cfg = &p_res->strua_enc_cfg;
    if (NULL == p_a_enc_cfg)
    {
    	return ONVIF_ERR_NO_CONFIG;
    }
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_a_enc_cfg->AudioEncoderConfiguration.token);
	offset += build_AudioEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_a_enc_cfg);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_SetAudioEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetAudioDecoderConfigurationResponse />");		    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	Get_v_enc_cfgs_RES * p_res = (Get_v_enc_cfgs_RES *)argv;
    ONVIF_V_ENC_CFG * p_v_enc_cfg = p_res->p_v_enc_cfg;
    
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationsResponse>");

	while (p_v_enc_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_v_enc_cfg->VideoEncoderConfiguration.token);
    	offset += build_VideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_v_enc_cfg);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
    	
    	p_v_enc_cfg = p_v_enc_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetCompatibleVideoEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	ONVIF_V_ENC_CFG * p_v_enc_cfg;
	
	GetCompatiblev_enc_cfgs_RES * p_res = (GetCompatiblev_enc_cfgs_RES *)argv;
	p_v_enc_cfg = p_res->p_v_enc_cfg;
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleVideoEncoderConfigurationsResponse>");

	while (p_v_enc_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_v_enc_cfg->VideoEncoderConfiguration.token);
    	offset += build_VideoEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_v_enc_cfg);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
    	
    	p_v_enc_cfg = p_v_enc_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleVideoEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetAudioEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	
	GetAudioEncoderCfgs_RES * p_res = (GetAudioEncoderCfgs_RES *)argv;
    ONVIF_A_ENC_CFG * p_a_enc_cfg = p_res->a_enc_cfg;
    
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationsResponse>");

	while (p_a_enc_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_a_enc_cfg->AudioEncoderConfiguration.token);
    	offset += build_AudioEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_a_enc_cfg);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
    	
    	p_a_enc_cfg = p_a_enc_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetCompatibleAudioEncoderConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	
	GetCompatibleA_Enc_Cfgs_RES * p_res = (GetCompatibleA_Enc_Cfgs_RES *)argv;
	
	ONVIF_A_ENC_CFG * p_a_enc_cfg;

	p_a_enc_cfg = p_res->a_enc_cfg;
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleAudioEncoderConfigurationsResponse>");

	while (p_a_enc_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_a_enc_cfg->AudioEncoderConfiguration.token);
    	offset += build_AudioEncoderConfiguration_xml(p_buf+offset, mlen-offset, p_a_enc_cfg);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
    	
    	p_a_enc_cfg = p_a_enc_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleAudioEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

int build_GetVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	Get_v_src_cfgs_RES * p_res = (Get_v_src_cfgs_RES *)argv;
    ONVIF_V_SRC_CFG * p_v_src_cfg = p_res->p_v_src_cfgs;
    
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationsResponse>");

	while (p_v_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, 
	    	"<trt:Configurations token=\"%s\">", 
	    	p_v_src_cfg->VideoSourceConfiguration.token);
	    offset += build_VideoSourceConfiguration_xml(p_buf+offset, mlen-offset, p_v_src_cfg);	
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_v_src_cfg = p_v_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	Get_v_src_cfg_RES * p_res = (Get_v_src_cfg_RES *)argv;
    ONVIF_V_SRC_CFG * p_v_src_cfg = &p_res->v_src_cfg;
    if (NULL == p_v_src_cfg)
    {
    	return ONVIF_ERR_NO_CONFIG;
    }
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_v_src_cfg->VideoSourceConfiguration.token);
    offset += build_VideoSourceConfiguration_xml(p_buf+offset, mlen-offset, p_v_src_cfg);	
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");	    
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetVideoSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoSourceConfigurationResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetVideoSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	int i =0;
	
	Get_v_src_options_RES * p_res = (Get_v_src_options_RES *)argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationOptionsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");
    
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tt:BoundsRange>"
			"<tt:XRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:XRange>"
			"<tt:YRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:YRange>"
			"<tt:WidthRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:WidthRange>"
			"<tt:HeightRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:HeightRange>"
		"</tt:BoundsRange>", 
		p_res->v_src_option.BoundsRange.XRange.Min, 
		p_res->v_src_option.BoundsRange.XRange.Max,
		p_res->v_src_option.BoundsRange.YRange.Min, 
		p_res->v_src_option.BoundsRange.YRange.Max,
		p_res->v_src_option.BoundsRange.WidthRange.Min, 
		p_res->v_src_option.BoundsRange.WidthRange.Max,
		p_res->v_src_option.BoundsRange.HeightRange.Min, 
		p_res->v_src_option.BoundsRange.HeightRange.Max);	

	for (i = 0; i < MAX_LIST_NUM; i++)
	{
		if (p_res->SourceToken[i][0] != '\0')
		{
    		offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoSourceTokensAvailable>%s</tt:VideoSourceTokensAvailable>",p_res->SourceToken[i]);
		}
	}
    
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationOptionsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetCompatibleVideoSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	GetCompatiblev_src_cfgs_RES * p_res = (GetCompatiblev_src_cfgs_RES *)argv;
	ONVIF_V_SRC_CFG * p_v_src_cfg;

	p_v_src_cfg = p_res->p_v_src_cfgs;
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleVideoSourceConfigurationsResponse>");

	while (p_v_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_v_src_cfg->VideoSourceConfiguration.token);
	    offset += build_VideoSourceConfiguration_xml(p_buf+offset, mlen-offset, p_v_src_cfg);	
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_v_src_cfg = p_v_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleVideoSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	
	GetAudioSourceCfgs_RES * p_res = (GetAudioSourceCfgs_RES *)argv;
    ONVIF_A_SRC_CFG * p_a_src_cfg = p_res->a_src_cfg;
    
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationsResponse>");

	while (p_a_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_a_src_cfg->AudioSourceConfiguration.token);
	    offset += build_AudioSourceConfiguration_xml(p_buf+offset, mlen-offset, p_a_src_cfg);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_a_src_cfg = p_a_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetCompatibleAudioSourceConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	ONVIF_A_SRC_CFG * p_a_src_cfg;
	GetCompatibleA_Src_Cfgs_RES * p_res = (GetCompatibleA_Src_Cfgs_RES *)argv;
	
	p_a_src_cfg = p_res->a_src_cfg;
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleAudioSourceConfigurationsResponse>");

	while (p_a_src_cfg)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_a_src_cfg->AudioSourceConfiguration.token);
	    offset += build_AudioSourceConfiguration_xml(p_buf+offset, mlen-offset, p_a_src_cfg);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	    
	    p_a_src_cfg = p_a_src_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleAudioSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetAudioSourceConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	int i =0;
	ONVIF_A_SRC_CFG * p_a_src_cfg = NULL;
	GetA_Src_Options_RES * p_res = (GetA_Src_Options_RES *)argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationOptionsResponse><trt:Options>");

	for (i = 0; i < MAX_LIST_NUM; i++)
	{
		if (p_res->SourceToken[i][0] != '\0')
		{
    		offset += snprintf(p_buf+offset, mlen-offset, "<tt:InputTokensAvailable>%s</tt:InputTokensAvailable>",p_res->SourceToken[i]);
		}
	}
	
    
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options></trt:GetAudioSourceConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	 
	GetAudioSourceCfg_RES * p_res = (GetAudioSourceCfg_RES *)argv;
    ONVIF_A_SRC_CFG * p_a_src_cfg = &p_res->strua_src_cfg;
    if (NULL == p_a_src_cfg)
    {
    	return ONVIF_ERR_NO_CONFIG;
    }
    
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_a_src_cfg->AudioSourceConfiguration.token);
    offset += build_AudioSourceConfiguration_xml(p_buf+offset, mlen-offset, p_a_src_cfg);
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");	    
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetAudioSourceConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetAudioSourceConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetVideoEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{   
	int i = 0;
	int offset;
		
	Get_v_enc_options_RES * p_res = (Get_v_enc_options_RES *)argv;

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationOptionsResponse><trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:QualityRange>"
			"<tt:Min>%d</tt:Min>"
			"<tt:Max>%d</tt:Max>"
		"</tt:QualityRange>",
		p_res->v_enc_option.QualityRange.Min, 
		p_res->v_enc_option.QualityRange.Max);

	if (p_res->v_enc_option.JPEGFlag)
	{
		// JPEG options	
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:JPEG>");
		
		for (i = 0; i < MAX_RES_NUMS; i++)
		{
			if (p_res->v_enc_option.JPEG.ResolutionsAvailable[i].Width == 0 || 
				p_res->v_enc_option.JPEG.ResolutionsAvailable[i].Height == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:ResolutionsAvailable>"
					"<tt:Width>%d</tt:Width>"
					"<tt:Height>%d</tt:Height>"
				"</tt:ResolutionsAvailable>",
				p_res->v_enc_option.JPEG.ResolutionsAvailable[i].Width, 
				p_res->v_enc_option.JPEG.ResolutionsAvailable[i].Height);
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:FrameRateRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:FrameRateRange>"
			"<tt:EncodingIntervalRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:EncodingIntervalRange>",
			p_res->v_enc_option.JPEG.FrameRateRange.Min, 
			p_res->v_enc_option.JPEG.FrameRateRange.Max,
			p_res->v_enc_option.JPEG.EncodingIntervalRange.Min, 
			p_res->v_enc_option.JPEG.EncodingIntervalRange.Max);

		offset += snprintf(p_buf+offset, mlen-offset, "</tt:JPEG>");			
	}

	if (p_res->v_enc_option.MPEG4Flag)
	{
		// MPEG4 options
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:MPEG4>");
		
		for (i = 0; i < MAX_RES_NUMS; i++)
		{
			if (p_res->v_enc_option.MPEG4.ResolutionsAvailable[i].Width == 0 || 
				p_res->v_enc_option.MPEG4.ResolutionsAvailable[i].Height == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:ResolutionsAvailable>"
					"<tt:Width>%d</tt:Width>"
					"<tt:Height>%d</tt:Height>"
				"</tt:ResolutionsAvailable>",
				p_res->v_enc_option.MPEG4.ResolutionsAvailable[i].Width, 
				p_res->v_enc_option.MPEG4.ResolutionsAvailable[i].Height);
		}	
		
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:GovLengthRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:GovLengthRange>"
			"<tt:FrameRateRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:FrameRateRange>"
			"<tt:EncodingIntervalRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:EncodingIntervalRange>",
			p_res->v_enc_option.MPEG4.GovLengthRange.Min, 
			p_res->v_enc_option.MPEG4.GovLengthRange.Max, 
			p_res->v_enc_option.MPEG4.FrameRateRange.Min, 
			p_res->v_enc_option.MPEG4.FrameRateRange.Max,
			p_res->v_enc_option.MPEG4.EncodingIntervalRange.Min, 
			p_res->v_enc_option.MPEG4.EncodingIntervalRange.Max);

		if (p_res->v_enc_option.MPEG4.Mpeg4Profile_SP)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mpeg4ProfilesSupported>SP</tt:Mpeg4ProfilesSupported>");
		}
		
		if (p_res->v_enc_option.MPEG4.Mpeg4Profile_ASP)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mpeg4ProfilesSupported>ASP</tt:Mpeg4ProfilesSupported>");
		}
			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:MPEG4>");	
	}

	if (p_res->v_enc_option.H264Flag)
	{
		// H264 options
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264>");
		
		for (i = 0; i < MAX_RES_NUMS; i++)
		{
			if (p_res->v_enc_option.H264.ResolutionsAvailable[i].Width == 0 || 
				p_res->v_enc_option.H264.ResolutionsAvailable[i].Height == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:ResolutionsAvailable>"
					"<tt:Width>%d</tt:Width>"
					"<tt:Height>%d</tt:Height>"
				"</tt:ResolutionsAvailable>",
				p_res->v_enc_option.H264.ResolutionsAvailable[i].Width, 
				p_res->v_enc_option.H264.ResolutionsAvailable[i].Height);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:GovLengthRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:GovLengthRange>"
			"<tt:FrameRateRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:FrameRateRange>"
			"<tt:EncodingIntervalRange>"
				"<tt:Min>%d</tt:Min>"
				"<tt:Max>%d</tt:Max>"
			"</tt:EncodingIntervalRange>",
			p_res->v_enc_option.H264.GovLengthRange.Min, 
			p_res->v_enc_option.H264.GovLengthRange.Max, 
			p_res->v_enc_option.H264.FrameRateRange.Min, 
			p_res->v_enc_option.H264.FrameRateRange.Max,
			p_res->v_enc_option.H264.EncodingIntervalRange.Min, 
			p_res->v_enc_option.H264.EncodingIntervalRange.Max);

		if (p_res->v_enc_option.H264.H264Profile_Baseline)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported>");
		}
		
		if (p_res->v_enc_option.H264.H264Profile_Main)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Main</tt:H264ProfilesSupported>");
		}

		if (p_res->v_enc_option.H264.H264Profile_Extended)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Extended</tt:H264ProfilesSupported>");
		}

		if (p_res->v_enc_option.H264.H264Profile_High)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>High</tt:H264ProfilesSupported>");
		}
			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:H264>");	
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options></trt:GetVideoEncoderConfigurationOptionsResponse>");	    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetAudioEncoderConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{    
	int i, j;
	int offset;
	GetA_Enc_Options_RES * p_res = (GetA_Enc_Options_RES *) argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationOptionsResponse><trt:Options>");

	for (i = 0; i < p_res->a_enc_option.sizeOptions; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Options>");		
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Encoding>%s</tt:Encoding>", 
			onvif_AudioEncodingToString(p_res->a_enc_option.Options[i].Encoding));
			
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:BitrateList>");
		for (j = 0; j < 10; j++)
		{
			if (p_res->a_enc_option.Options[i].BitrateList[i] > 0)
			{
				offset += snprintf(p_buf+offset, mlen-offset, 
					"<tt:Items>%d</tt:Items>",
					p_res->a_enc_option.Options[i].BitrateList[i]);
			}
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:BitrateList>");
		
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:SampleRateList>");
		for (j = 0; j < 10; j++)
		{
			if (p_res->a_enc_option.Options[i].SampleRateList[i] > 0)
			{
				offset += snprintf(p_buf+offset, mlen-offset, 
					"<tt:Items>%d</tt:Items>",
					p_res->a_enc_option.Options[i].SampleRateList[i]);
			}
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:SampleRateList>");
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Options>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options></trt:GetAudioEncoderConfigurationOptionsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SystemReboot_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:SystemRebootResponse>"
			"<tds:Message>Rebooting</tds:Message>"
		"</tds:SystemRebootResponse>");	    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetSystemFactoryDefault_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetSystemFactoryDefaultResponse />");	    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetSystemLog_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int type = (long)argv; /* 0:System Log, 1:Access Log */

	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetSystemLogResponse><tds:SystemLog>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:String>Test %s Log</tt:String>", type == 1 ? "Access" : "System");
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:SystemLog></tds:GetSystemLogResponse>");	    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetVideoEncoderConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoEncoderConfigurationResponse />");		    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetSystemDateAndTime_rly_xml(char * p_buf, int mlen, const char * argv)
{
	time_t nowtime;
	struct tm *gtime;
	GetSystemDateAndTime_RES * p_res = (GetSystemDateAndTime_RES *)argv;

	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	time(&nowtime);
	gtime = gmtime(&nowtime);
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetSystemDateAndTimeResponse>"
			"<tds:SystemDateAndTime>"
			"<tt:DateTimeType>%s</tt:DateTimeType>"
			"<tt:DaylightSavings>%s</tt:DaylightSavings>",
			onvif_SetDateTimeTypeToString(p_res->SystemDateTime.DateTimeType), 
			p_res->SystemDateTime.DaylightSavings ? "true" : "false");

	if (p_res->SystemDateTime.TimeZoneFlag && 
		p_res->SystemDateTime.TimeZone.TZ[0] != '\0')
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:TimeZone><tt:TZ>%s</tt:TZ></tt:TimeZone>", 
			p_res->SystemDateTime.TimeZone.TZ);			
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 			
			"<tt:UTCDateTime>"
				"<tt:Time>"
					"<tt:Hour>%d</tt:Hour>"
					"<tt:Minute>%d</tt:Minute>"
					"<tt:Second>%d</tt:Second>"
				"</tt:Time>"
				"<tt:Date>"
					"<tt:Year>%d</tt:Year>"
					"<tt:Month>%d</tt:Month>"
					"<tt:Day>%d</tt:Day>"
				"</tt:Date>"
			"</tt:UTCDateTime>",	
		p_res->UTCDateTime.Time.Hour, p_res->UTCDateTime.Time.Minute,p_res->UTCDateTime.Time.Second, 
		p_res->UTCDateTime.Date.Year, p_res->UTCDateTime.Date.Month, p_res->UTCDateTime.Date.Day);		

	offset += snprintf(p_buf+offset, mlen-offset, 			
			"<tt:LocalDateTime>"
				"<tt:Time>"
					"<tt:Hour>%d</tt:Hour>"
					"<tt:Minute>%d</tt:Minute>"
					"<tt:Second>%d</tt:Second>"
				"</tt:Time>"
				"<tt:Date>"
					"<tt:Year>%d</tt:Year>"
					"<tt:Month>%d</tt:Month>"
					"<tt:Day>%d</tt:Day>"
				"</tt:Date>"
			"</tt:LocalDateTime>"	
			"</tds:SystemDateAndTime>"
		"</tds:GetSystemDateAndTimeResponse>",
		p_res->LocalDateTime.Time.Hour, p_res->LocalDateTime.Time.Minute,p_res->LocalDateTime.Time.Second, 
		p_res->LocalDateTime.Date.Year, p_res->LocalDateTime.Date.Month, p_res->LocalDateTime.Date.Day);	
		
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}


int build_SetSystemDateAndTime_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetSystemDateAndTimeResponse></tds:SetSystemDateAndTimeResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_DeviceServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_DevicesCapabilities * p_res = (onvif_DevicesCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:Network IPFilter=\"%s\" ZeroConfiguration=\"%s\" "
			"IPVersion6=\"%s\" DynDNS=\"%s\" Dot11Configuration=\"%s\" "
			"Dot1XConfigurations=\"%d\" HostnameFromDHCP=\"%s\" NTP=\"%d\" DHCPv6=\"%s\">"
		"</tds:Network>",
		p_res->IPFilter ? "true" : "false",
		p_res->ZeroConfiguration ? "true" : "false",
		p_res->IPVersion6 ? "true" : "false",
		p_res->DynDNS ? "true" : "false",
		p_res->Dot11Configuration ? "true" : "false",
		p_res->Dot1XConfigurations,
		p_res->HostnameFromDHCP ? "true" : "false",
		p_res->NTP,
		p_res->DHCPv6 ? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:Security TLS1.0=\"%s\" TLS1.1=\"%s\" TLS1.2=\"%s\" "
			"OnboardKeyGeneration=\"%s\" AccessPolicyConfig=\"%s\" DefaultAccessPolicy=\"%s\" "
			"Dot1X=\"%s\" RemoteUserHandling=\"%s\" X.509Token=\"%s\" SAMLToken=\"%s\" "
			"KerberosToken=\"%s\" UsernameToken=\"%s\" HttpDigest=\"%s\" RELToken=\"%s\" SupportedEAPMethods=\"%d\" MaxUsers=\"%d\">"
		"</tds:Security>",
		p_res->TLS10 ? "true" : "false",
		p_res->TLS11 ? "true" : "false",
		p_res->TLS12 ? "true" : "false",
		p_res->OnboardKeyGeneration ? "true" : "false",
		p_res->AccessPolicyConfig ? "true" : "false",
		p_res->DefaultAccessPolicy ? "true" : "false",
		p_res->Dot1X ? "true" : "false",
		p_res->RemoteUserHandling ? "true" : "false",
		p_res->X509Token ? "true" : "false",
		p_res->SAMLToken ? "true" : "false",
		p_res->KerberosToken ? "true" : "false",
		p_res->UsernameToken ? "true" : "false",
		p_res->HttpDigest ? "true" : "false",
		p_res->RELToken ? "true" : "false",
		p_res->SupportedEAPMethods,
		p_res->MaxUsers);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:System DiscoveryResolve=\"%s\" DiscoveryBye=\"%s\" "
			"RemoteDiscovery=\"%s\" SystemBackup=\"%s\" SystemLogging=\"%s\" "
			"FirmwareUpgrade=\"%s\" HttpFirmwareUpgrade=\"%s\" HttpSystemBackup=\"%s\" "
			"HttpSystemLogging=\"%s\" HttpSupportInformation=\"%s\">"
		"</tds:System>",
		p_res->DiscoveryResolve ? "true" : "false",
		p_res->DiscoveryBye ? "true" : "false",
		p_res->RemoteDiscovery ? "true" : "false",
		p_res->SystemBackup ? "true" : "false",
		p_res->SystemLogging ? "true" : "false",
		p_res->FirmwareUpgrade ? "true" : "false",
		p_res->HttpFirmwareUpgrade ? "true" : "false",
		p_res->HttpSystemBackup ? "true" : "false",
		p_res->HttpSystemLogging ? "true" : "false",
		p_res->HttpSupportInformation ? "true" : "false");
			
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");

	return offset;
}

int build_MediaServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_MediaCapabilities * p_res = (onvif_MediaCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:Capabilities SnapshotUri=\"%s\" Rotation=\"%s\" VideoSourceMode=\"%s\" OSD=\"%s\">",
		p_res->SnapshotUri ? "true" : "false",
		p_res->Rotation ? "true" : "false",
		p_res->VideoSourceMode ? "true" : "false",
		p_res->OSD ? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trt:ProfileCapabilities MaximumNumberOfProfiles=\"%d\" />"
		"<trt:StreamingCapabilities RTPMulticast=\"%s\" RTP_TCP=\"%s\" RTP_RTSP_TCP=\"%s\" "
			"NonAggregateControl=\"%s\" NoRTSPStreaming=\"%s\" />",
		p_res->MaximumNumberOfProfiles,
		p_res->RTPMulticast ? "true" : "false",
		p_res->RTP_TCP ? "true" : "false",
		p_res->RTP_RTSP_TCP ? "true" : "false",
		p_res->NonAggregateControl ? "true" : "false",
		p_res->NoRTSPStreaming ? "true" : "false");
			
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Capabilities>");

	return offset;
}

int build_EventsServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_EventCapabilities * p_res = (onvif_EventCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tev:Capabilities WSSubscriptionPolicySupport=\"%s\" WSPullPointSupport=\"%s\" "
			"WSPausableSubscriptionManagerInterfaceSupport=\"%s\" MaxNotificationProducers=\"%d\" "
			"MaxPullPoints=\"%d\" PersistentNotificationStorage=\"%s\">"
		"</tev:Capabilities>",
		p_res->WSSubscriptionPolicySupport ? "true" : "false",
		p_res->WSPullPointSupport ? "true" : "false",
		p_res->WSPausableSubscriptionManagerInterfaceSupport ? "true" : "false",
		p_res->MaxNotificationProducers,
		p_res->MaxPullPoints,
		p_res->PersistentNotificationStorage ? "true" : "false");

	return offset;
}

int build_PTZServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_PTZCapabilities * p_res = (onvif_PTZCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tptz:Capabilities EFlip=\"%s\" Reverse=\"%s\" GetCompatibleConfigurations=\"%s\" />",
		p_res->EFlip ? "true" : "false",
		p_res->Reverse ? "true" : "false",
		p_res->GetCompatibleConfigurations ? "true" : "false");

	return offset;
}

int build_ImagingServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_ImagingCapabilities * p_res = (onvif_ImagingCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<timg:Capabilities ImageStabilization=\"%s\" />",
		p_res->ImageStabilization ? "true" : "false");

	return offset;
}

#ifdef VIDEO_ANALYTICS
int build_AnalyticsServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_AnalyticsCapabilities * p_res = (onvif_AnalyticsCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tan:Capabilities RuleSupport=\"%s\" AnalyticsModuleSupport=\"%s\" CellBasedSceneDescriptionSupported=\"%s\" />",
		p_res->RuleSupport ? "true" : "false",
		p_res->AnalyticsModuleSupport ? "true" : "false",
		p_res->CellBasedSceneDescriptionSupported ? "true" : "false");

	return offset;
}
#endif	// end of VIDEO_ANALYTICS

#ifdef PROFILE_G_SUPPORT

int build_RecordingServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	char Encoding[100];
	onvif_RecordingCapabilities * p_res = (onvif_RecordingCapabilities *)argv;
	memset(Encoding, 0, sizeof(Encoding));
	
	if (p_res->JPEG)
	{
		strcat(Encoding, "JPEG ");
	}
	if (p_res->MPEG4)
	{
		strcat(Encoding, "MPEG4 ");
	}
	if (p_res->H264)
	{
		strcat(Encoding, "H264 ");
	}
	if (p_res->G711)
	{
		strcat(Encoding, "G711 ");
	}
	if (p_res->G726)
	{
		strcat(Encoding, "G726 ");
	}
	if (p_res->AAC)
	{
		strcat(Encoding, "AAC ");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trc:Capabilities DynamicRecordings=\"%s\" DynamicTracks=\"%s\" Encoding=\"%s\" MaxRate=\"%0.1f\" "
		"MaxTotalRate=\"%0.1f\" MaxRecordings=\"%d\" MaxRecordingJobs=\"%d\" Options=\"%s\" MetadataRecording=\"%s\" />",
		p_res->DynamicRecordings ? "true" : "false",
		p_res->DynamicTracks ? "true" : "false",
		Encoding,
		p_res->MaxRate,
		p_res->MaxTotalRate,
		p_res->MaxRecordings,
		p_res->MaxRecordingJobs,
		p_res->Options ? "true" : "false",
		p_res->MetadataRecording ? "true" : "false");

	return offset;
}

int build_SearchServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_SearchCapabilities * p_res = (onvif_SearchCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:Capabilities MetadataSearch=\"%s\" GeneralStartEvents=\"%s\" />",
		p_res->MetadataSearch ? "true" : "false",
		p_res->GeneralStartEvents ? "true" : "false");

	return offset;
}

int build_ReplayServicesCapabilities_xml(char * p_buf, int mlen,const char * argv)
{
	int offset = 0;
	onvif_ReplayCapabilities * p_res = (onvif_ReplayCapabilities *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trp:Capabilities ReversePlayback=\"%s\" SessionTimeoutRange=\"%0.1f %0.1f\" RTP_RTSP_TCP=\"%s\" />",
		p_res->ReversePlayback ? "true" : "false",
		p_res->SessionTimeoutRange.Min,
		p_res->SessionTimeoutRange.Max,
		p_res->RTP_RTSP_TCP ? "true" : "false");

	return offset;
}

#endif


int build_Version_xml(char * p_buf, int mlen, int major, int minor)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:Version>"
		    "<tt:Major>%d</tt:Major>"
		    "<tt:Minor>%d</tt:Minor>"
	    "</tds:Version>",
	    major, minor);

	return offset;
}

int build_GetServices_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetServices_RES * p_res = (GetServices_RES *)argv;
	BOOL bIncludeCapability = p_res->bIncludeCapability;
	
    int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetServicesResponse>");

	// device manager
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tds:Service>"
	    "<tds:Namespace>http://www.onvif.org/ver10/device/wsdl</tds:Namespace>"
	    "<tds:XAddr>%s</tds:XAddr>", 
	    p_res->Capabilities.device.XAddr);	
	if (bIncludeCapability)
	{
        offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");        
        offset += build_DeviceServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.device);				
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
	}    
	offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");

	// media 
	if (p_res->Capabilities.media.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/media/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.media.XAddr);	
		if (bIncludeCapability)
		{
	        offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_MediaServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.media);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
	
	// event 
	if (p_res->Capabilities.events.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/events/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.events.XAddr);	
		if (bIncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_EventsServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.events);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}

	// ptz
	if (p_res->Capabilities.ptz.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver20/ptz/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.ptz.XAddr);	
		if (bIncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_PTZServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.ptz);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}

	// image
	if (p_res->Capabilities.image.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver20/imaging/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.image.XAddr);	
		if (bIncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_ImagingServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.image);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}

#ifdef VIDEO_ANALYTICS
	// analytics
	if (p_res->Capabilities.analytics.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver20/analytics/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.analytics.XAddr);	
		if (bIncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_AnalyticsServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.analytics);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
#endif

#ifdef PROFILE_G_SUPPORT

	// recording
	if (p_res->Capabilities.recording.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/recording/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.recording.XAddr);	
		if (bIncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_RecordingServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.recording);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}

	// search
	if (p_res->Capabilities.search.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/search/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.search.XAddr);	
		if (bIncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_SearchServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.search);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}

	// replay
	if (p_res->Capabilities.replay.support)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tds:Service>"
		    "<tds:Namespace>http://www.onvif.org/ver10/replay/wsdl</tds:Namespace>"
		    "<tds:XAddr>%s</tds:XAddr>", 
		    p_res->Capabilities.replay.XAddr);	
		if (bIncludeCapability)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
	        offset += build_ReplayServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.replay);				
			offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
		}    
		offset += build_Version_xml(p_buf+offset, mlen-offset, 2, 42);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:Service>");
	}
	
#endif

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetServicesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{	
	int i;
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetScopes_RES * p_res = (GetScopes_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetScopesResponse>");

	for (i = 0; i < MAX_SCOPE_NUMS; i++)
	{
		if (p_res->scopes[i].scope[0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tds:Scopes>"
					"<tt:ScopeDef>%s</tt:ScopeDef>"
					"<tt:ScopeItem>%s</tt:ScopeItem>"
				"</tds:Scopes>", 
				p_res->scopes[i].fixed ? "Fixed" : "Configurable", 
				p_res->scopes[i].scope);
		}
	}
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetScopesResponse>");    	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_AddScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:AddScopesResponse />");    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetScopesResponse />");    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemoveScopes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	ONVIF_SCOPE * p_scope = (ONVIF_SCOPE *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:RemoveScopesResponse>");

    for (i = 0; i < MAX_SCOPE_NUMS; i++)
	{
		if (p_scope[i].scope[0] == '\0')
		{
			break;
		}

		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ScopeItem>%s</tt:ScopeItem>", p_scope[i].scope);  
	}
	
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:RemoveScopesResponse>");    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetHostname_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetHostname_RES * p_res = (GetHostname_RES *)argv;

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tds:GetHostnameResponse>"
        	"<tds:HostnameInformation>"
	            "<tt:FromDHCP>%s</tt:FromDHCP>"
	            "<tt:Name>%s</tt:Name>"    
       		"</tds:HostnameInformation>"
       	"</tds:GetHostnameResponse>",
      	p_res->HostnameInfo.FromDHCP ? "true" : "false",
      	p_res->HostnameInfo.Name);
    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetHostname_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetHostnameResponse />");
    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetHostnameFromDHCP_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	BOOL RebootNeeded = *((BOOL *)argv);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<tds:SetHostnameFromDHCPResponse>"
    		"<tds:RebootNeeded>%s</tds:RebootNeeded>"
    	"</tds:SetHostnameFromDHCPResponse>",
    	RebootNeeded ? "true" : "false");
    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetNetworkProtocols_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i = 0;
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetNetworkProtocols_RES * p_res = (GetNetworkProtocols_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkProtocolsResponse>");
	

	if (p_res->NetworkProtocol.HTTPFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tds:NetworkProtocols>"
			"<tt:Name>HTTP</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", 
			p_res->NetworkProtocol.HTTPEnabled ? "true" : "false");

		for (i = 0; i < MAX_SERVER_PORT; i++)
		{
			if (p_res->NetworkProtocol.HTTPPort[i] == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", p_res->NetworkProtocol.HTTPPort[i]);
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}

	if (p_res->NetworkProtocol.HTTPSFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tds:NetworkProtocols>"
			"<tt:Name>HTTPS</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", 
			p_res->NetworkProtocol.HTTPSEnabled ? "true" : "false");

		for (i = 0; i < MAX_SERVER_PORT; i++)
		{
			if (p_res->NetworkProtocol.HTTPSPort[i] == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", p_res->NetworkProtocol.HTTPSPort[i]);
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}

	if (p_res->NetworkProtocol.RTSPFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tds:NetworkProtocols>"
			"<tt:Name>RTSP</tt:Name>"
			"<tt:Enabled>%s</tt:Enabled>", 
			p_res->NetworkProtocol.RTSPEnabled ? "true" : "false");

		for (i = 0; i < MAX_SERVER_PORT; i++)
		{
			if (p_res->NetworkProtocol.RTSPPort[i] == 0)
			{
				continue;
			}
			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Port>%d</tt:Port>", p_res->NetworkProtocol.RTSPPort[i]);
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetNetworkProtocolsResponse>");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetNetworkProtocols_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkProtocolsResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetNetworkDefaultGateway_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;	
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetNetworkDefaultGateway_RES * p_res = (GetNetworkDefaultGateway_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkDefaultGatewayResponse><tds:NetworkGateway>");
	

	for (i = 0; i < MAX_GATEWAY; i++)
	{
		if (p_res->NetworkGateway.IPv4Address[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4Address>%s</tt:IPv4Address>", p_res->NetworkGateway.IPv4Address[i]);
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkGateway></tds:GetNetworkDefaultGatewayResponse>");		
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetNetworkDefaultGateway_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkDefaultGatewayResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetDiscoveryMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetDiscoveryMode_RES * p_res = (GetDiscoveryMode_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetDiscoveryModeResponse>"
   			"<tds:DiscoveryMode>%s</tds:DiscoveryMode>"
	  	"</tds:GetDiscoveryModeResponse>", 
	  	onvif_DiscoveryModeToString(p_res->DiscoveryMode));	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetDiscoveryMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetDiscoveryModeResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetDNS_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetDNS_RES * p_res = (GetDNS_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDNSResponse><tds:DNSInformation>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:FromDHCP>%s</tt:FromDHCP>", p_res->DNSInfo.FromDHCP ? "true" : "false");

	if (p_res->DNSInfo.SearchDomainFlag)
	{
		for (i = 0; i < MAX_SEARCHDOMAIN; i++)
		{
			if (p_res->DNSInfo.SearchDomain[i][0] == '\0')
			{
				continue;
			}

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SearchDomain>%s</tt:SearchDomain>", p_res->DNSInfo.SearchDomain[i]);
		}
	}
	
	for (i = 0; i < MAX_DNS_SERVER; i++)
	{
		if (p_res->DNSInfo.DNSServer[i][0] == '\0')
		{
			continue;
		}
		
		if (p_res->DNSInfo.FromDHCP)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DNSFromDHCP>");
		}
		else
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DNSManual>");
		}

		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Type>IPv4</tt:Type><tt:IPv4Address>%s</tt:IPv4Address>",
			p_res->DNSInfo.DNSServer[i]);

		if (p_res->DNSInfo.FromDHCP)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:DNSFromDHCP>");
		}
		else
		{
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:DNSManual>");
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:DNSInformation></tds:GetDNSResponse>");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetDNS_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetDNSResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetNTP_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetNTP_RES * p_res = (GetNTP_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNTPResponse><tds:NTPInformation>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:FromDHCP>%s</tt:FromDHCP>", p_res->NTPInfo.FromDHCP ? "true" : "false");
	
	for (i = 0; i < MAX_NTP_SERVER; i++)
	{
		if (p_res->NTPInfo.NTPServer[i][0] == '\0')
		{
			continue;
		}
		
		if (p_res->NTPInfo.FromDHCP)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:NTPFromDHCP>");
		}
		else
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:NTPManual>");
		}
		
		if (is_ip_address(p_res->NTPInfo.NTPServer[i]))
		{
		    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>IPv4</tt:Type>");
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4Address>%s</tt:IPv4Address>", p_res->NTPInfo.NTPServer[i]);
		}
		else
		{
		    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>DNS</tt:Type>");
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DNSname>%s</tt:DNSname>", p_res->NTPInfo.NTPServer[i]);
		}

		if (p_res->NTPInfo.FromDHCP)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:NTPFromDHCP>");
		}
		else
		{
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:NTPManual>");
		}
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:NTPInformation></tds:GetNTPResponse>");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetNTP_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNTPResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_GetServiceCapabilities_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetCapabilities_RES * p_res = (GetCapabilities_RES *)argv;
	onvif_CapabilityCategory category = p_res->Category;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);

	if (CapabilityCategory_Events == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetServiceCapabilitiesResponse");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	

	if (CapabilityCategory_Device == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetServiceCapabilitiesResponse>");
		offset += build_DeviceServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.device);
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_Media == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetServiceCapabilitiesResponse>");
		offset += build_MediaServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.media);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_Events == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tev:GetServiceCapabilitiesResponse>");
		offset += build_EventsServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.events);
		offset += snprintf(p_buf+offset, mlen-offset, "</tev:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_PTZ == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetServiceCapabilitiesResponse>");
		offset += build_PTZServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.ptz);
		offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_Imaging == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetServiceCapabilitiesResponse>");
		offset += build_ImagingServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.image);
		offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetServiceCapabilitiesResponse>");
	}
#ifdef VIDEO_ANALYTICS	
	else if (CapabilityCategory_Analytics == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetServiceCapabilitiesResponse>");
		offset += build_AnalyticsServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.analytics);
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetServiceCapabilitiesResponse>");
	}	
#endif	
#ifdef PROFILE_G_SUPPORT
	else if (CapabilityCategory_Recording == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetServiceCapabilitiesResponse>");
		offset += build_RecordingServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.recording);
		offset += snprintf(p_buf+offset, mlen-offset, "</trc:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_Search == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetServiceCapabilitiesResponse>");
		offset += build_SearchServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.search);
		offset += snprintf(p_buf+offset, mlen-offset, "</tse:GetServiceCapabilitiesResponse>");
	}
	else if (CapabilityCategory_Replay == category)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trp:GetServiceCapabilitiesResponse>");
		offset += build_ReplayServicesCapabilities_xml(p_buf+offset, mlen-offset,(char *)&p_res->Capabilities.replay);
		offset += snprintf(p_buf+offset, mlen-offset, "</trp:GetServiceCapabilitiesResponse>");
	}
#endif

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_GetEventProperties_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset,soap_head, "http://www.onvif.org/ver10/events/wsdl/EventPortType/GetEventPropertiesResponse");
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tev:GetEventPropertiesResponse>"
			"<tev:TopicNamespaceLocation>"
				"http://www.onvif.org/onvif/ver10/topics/topicns.xml"
			"</tev:TopicNamespaceLocation>"
			"<wsnt:FixedTopicSet>true</wsnt:FixedTopicSet>"

			"<wstop:TopicSet xmlns=\"\">"
				"<tns1:RuleEngine>"
					"<CellMotionDetector>"
					"<Motion wstop:topic=\"true\">"
					"<tt:MessageDescription IsProperty=\"true\">"
					"<tt:Source>"
					"<tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
					"<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
					"<tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\"/>"
					"</tt:Source>"
					"<tt:Data>"
					"<tt:SimpleItemDescription Name=\"IsMotion\" Type=\"xs:boolean\"/>"
					"</tt:Data>"					
					"</tt:MessageDescription>"					
					"</Motion>"
					"</CellMotionDetector>"
					"<LineDetector wstop:topic=\"true\">"
					"<Crossed wstop:topic=\"true\">"
					"<tt:MessageDescription IsProperty=\"true\">"
					"<tt:Source>"
					"<tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
					"<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
					"<tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\"/>"
					"</tt:Source>"
					"<tt:Data>"
					"<tt:SimpleItemDescription Name=\"ObjectId\" Type=\"xs:integer\"/>"
					"</tt:Data>"
					"</tt:MessageDescription>"
					"</Crossed>"
					"</LineDetector>"
					"<FieldDetector wstop:topic=\"true\">"
					"<ObjectsInside wstop:topic=\"true\">"
					"<tt:MessageDescription IsProperty=\"true\">"
					"<tt:Source>"
					"<tt:SimpleItemDescription Name=\"VideoSourceConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
					"<tt:SimpleItemDescription Name=\"VideoAnalyticsConfigurationToken\" Type=\"tt:ReferenceToken\"/>"
					"<tt:SimpleItemDescription Name=\"Rule\" Type=\"xs:string\"/>"
					"</tt:Source>"
					"<tt:Key>"
					"<tt:SimpleItemDescription Name=\"ObjectId\" Type=\"xs:integer\"/>"
					"</tt:Key>"
					"<tt:Data>"
					"<tt:SimpleItemDescription Name=\"IsInside\" Type=\"xs:boolean\"/>"
					"</tt:Data>"
					"</tt:MessageDescription>"
					"</ObjectsInside>"
					"</FieldDetector>"
	  			"</tns1:RuleEngine>"
			"</wstop:TopicSet>"	
			"<wsnt:TopicExpressionDialect>"
				"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet"										
			"</wsnt:TopicExpressionDialect>"
			"<wsnt:TopicExpressionDialect>"
				"http://docs.oasis-open.org/wsnt/t-1/TopicExpression/ConcreteSet"
			"</wsnt:TopicExpressionDialect>"
			"<wsnt:TopicExpressionDialect>"
				"http://docs.oasis-open.org/wsn/t-1/TopicExpression/Concrete"
			"</wsnt:TopicExpressionDialect>"	
			"<tev:MessageContentFilterDialect>"
				"http://www.onvif.org/ver10/tev/messageContentFilter/ItemFilter"
			"</tev:MessageContentFilterDialect>"
			"<tev:MessageContentSchemaLocation>"
				"http://www.onvif.org/onvif/ver10/schema/onvif.xsd"
			"</tev:MessageContentSchemaLocation>"
		"</tev:GetEventPropertiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_Subscribe_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	EUA * p_eua = (EUA *) argv;
	char cur_time[100], term_time[100];
	
	if (NULL == p_eua)
	{
		return -1;
	}
	
    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeResponse");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);	
	
	if (default_renew_time > p_eua->init_term_time)
	{
		onvif_get_time_str(term_time, sizeof(term_time), default_renew_time);
	}
	else
	{
		onvif_get_time_str(term_time, sizeof(term_time), p_eua->init_term_time);
	}
	
    offset += snprintf(p_buf+offset, mlen-offset, 
    	"<wsnt:SubscribeResponse>"
	        "<wsnt:SubscriptionReference>"
	            "<wsa:Address>%s</wsa:Address>"
	        "</wsnt:SubscriptionReference>"
	        "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
	        "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
	    "</wsnt:SubscribeResponse>",
	    p_eua->producter_addr, cur_time, term_time);
    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_Unsubscribe_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/UnsubscribeResponse");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:UnsubscribeResponse></wsnt:UnsubscribeResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_Renew_rly_xml(char * p_buf, int mlen, const char * argv)
{
	char cur_time[100], term_time[100];
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://docs.oasis-open.org/wsn/bw-2/SubscriptionManager/RenewResponse");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);
	onvif_get_time_str(term_time, sizeof(term_time), 60);
		
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<wsnt:RenewResponse>"			
	        "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
	        "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
		"</wsnt:RenewResponse>", term_time, cur_time);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreatePullPointSubscription_rly_xml(char * p_buf, int mlen, const char * argv)
{
	char cur_time[100], term_time[100];
	EUA * p_eua = (EUA *) argv;

	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://www.onvif.org/ver10/events/wsdl/EventPortType/CreatePullPointSubscriptionResponse");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);
	onvif_get_time_str(term_time, sizeof(term_time), 60);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tev:CreatePullPointSubscriptionResponse>"	
			"<tev:SubscriptionReference>"
	            "<wsa:Address>%s</wsa:Address>"
	        "</tev:SubscriptionReference>"	        
	        "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
	        "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
		"</tev:CreatePullPointSubscriptionResponse>", 
		p_eua->producter_addr,
		cur_time, term_time);
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SimpleItem_xml(char * p_buf, int mlen, onvif_SimpleItem * p_req)
{
	int offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:SimpleItem Name=\"%s\" Value=\"%s\" />",
		p_req->Name, p_req->Value);

	return offset;
}

int build_ElementItem_xml(char * p_buf, int mlen, onvif_ElementItem * p_req)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\">", p_req->Name);
	if (p_req->Any)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "%s", p_req->Any);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:ElementItem>");

	return offset;
}


int build_NotificationMessage_xml(char * p_buf, int mlen, onvif_NotificationMessage * p_req)
{
	int offset = 0;
	char utctime[64];
	ONVIF_SIMPLEITEM * p_simpleitem;
	ONVIF_ELEMENTITEM * p_elementitem;
	struct tm *gtime;
	
	gtime = gmtime(&p_req->Message.UtcTime);	
	snprintf(utctime, sizeof(utctime), "%04d-%02d-%02dT%02d:%02d:%02dZ", 		 
		gtime->tm_year+1900, gtime->tm_mon+1, gtime->tm_mday, gtime->tm_hour, gtime->tm_min, gtime->tm_sec);
		
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<wsnt:SubscriptionReference>"
			"<wsa:Address>%s</wsa:Address>"
		"</wsnt:SubscriptionReference>"
		"<wsnt:Topic Dialect=\"%s\">%s</wsnt:Topic>"
		"<wsnt:ProducerReference>"
			"<wsa:Address>%s</wsa:Address>"
		"</wsnt:ProducerReference>",
		p_req->ConsumerAddress,
		p_req->Dialect,
		p_req->Topic, 
		p_req->ProducterAddress);

	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:Message>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Message UtcTime=\"%s\"", utctime);
	if (p_req->Message.PropertyOperationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " PropertyOperation=\"%s\"", 
			onvif_PropertyOperationToString(p_req->Message.PropertyOperation));
	}
	offset += snprintf(p_buf+offset, mlen-offset, ">");

	if (p_req->Message.SourceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
		p_simpleitem = p_req->Message.Source.SimpleItem;
		while (p_simpleitem)
		{
			offset += build_SimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
			p_simpleitem = p_simpleitem->next;
		}
		
		p_elementitem = p_req->Message.Source.ElementItem;
		while (p_elementitem)
		{
			offset += build_ElementItem_xml(p_buf+offset, mlen-offset, &p_elementitem->ElementItem);
			p_elementitem = p_elementitem->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");
	}

	if (p_req->Message.KeyFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Key>");
		p_simpleitem = p_req->Message.Key.SimpleItem;
		while (p_simpleitem)
		{
			offset += build_SimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
			p_simpleitem = p_simpleitem->next;
		}

		p_elementitem = p_req->Message.Key.ElementItem;
		while (p_elementitem)
		{
			offset += build_ElementItem_xml(p_buf+offset, mlen-offset, &p_elementitem->ElementItem);
			p_elementitem = p_elementitem->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Key>");
	}

	if (p_req->Message.DataFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Data>");
		p_simpleitem = p_req->Message.Data.SimpleItem;
		while (p_simpleitem)
		{
			offset += build_SimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
			p_simpleitem = p_simpleitem->next;
		}

		p_elementitem = p_req->Message.Data.ElementItem;
		while (p_elementitem)
		{
			offset += build_ElementItem_xml(p_buf+offset, mlen-offset, &p_elementitem->ElementItem);
			p_elementitem = p_elementitem->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Data>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Message>");		
	offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:Message>");

	return offset;
}

int build_PullMessages_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0, msg_nums = 0;
	char cur_time[100], term_time[100];
	LINKED_NODE * p_node;
	ONVIF_NOTIFYMESSAGE * p_tmp;
	ONVIF_NOTIFYMESSAGE * p_message;
	
	PullMessages_REQ * p_req = (PullMessages_REQ *)argv;
	EUA * p_eua = onvif_get_eua_by_index(p_req->eua_idx);

	assert(p_eua);

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/PullMessagesResponse");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	onvif_get_time_str(cur_time, sizeof(cur_time), 0);
	onvif_get_time_str(term_time, sizeof(term_time), 60);

	offset += snprintf(p_buf+offset, mlen-offset, "<tev:PullMessagesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, 
        "<tev:CurrentTime>%s</tev:CurrentTime>"
        "<tev:TerminationTime>%s</tev:TerminationTime>",
        cur_time, term_time);

	// get notify message from message list
	sys_os_mutex_enter(g_onvif_cls.mutex_list);
	
	
	int interval =0;
	time_t star_time = time(NULL);
	while((msg_nums == 0)&&(interval < p_req->Timeout))
	{
		p_node = h_list_lookup_start(g_onvif_cls.msg_list);
		while (p_node && msg_nums < p_req->MessageLimit)
		{
			p_message = (ONVIF_NOTIFYMESSAGE *) p_node->p_data;
			
			if (p_message->NotificationMessage.Message.UtcTime > p_eua->last_msg)
			{
				p_tmp = p_message;
				while (p_tmp)
				{	
					if (onvif_event_filter(p_tmp, p_eua))
					{
						strcpy(p_tmp->NotificationMessage.ConsumerAddress, p_eua->consumer_addr);
						strcpy(p_tmp->NotificationMessage.ProducterAddress, p_eua->producter_addr);
						
						offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:NotificationMessage>");
						offset += build_NotificationMessage_xml(p_buf+offset, mlen-offset, &p_message->NotificationMessage);
						offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:NotificationMessage>");
					}
	
					p_tmp = p_tmp->next;
				}
				
				msg_nums++;
				p_eua->last_msg = p_message->NotificationMessage.Message.UtcTime;
			}
	
			p_node = h_list_lookup_next(g_onvif_cls.msg_list, p_node);
		}
		h_list_lookup_end(g_onvif_cls.msg_list);
		if(msg_nums > 0)
		{
			break;
		}
		sleep(1);
		time_t next_time = time(NULL);
		interval = next_time - star_time;
	}
	
	sys_os_mutex_leave(g_onvif_cls.mutex_list);
        
	offset += snprintf(p_buf+offset, mlen-offset, "</tev:PullMessagesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetSynchronizationPoint_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://www.onvif.org/ver10/events/wsdl/PullPointSubscription/SetSynchronizationPointResponse");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);	
		
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tev:SetSynchronizationPointResponse>"
		"</tev:SetSynchronizationPointResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_Notify_xml(char * p_buf, int mlen, const char * argv)
{
	ONVIF_NOTIFYMESSAGE * p_message = (ONVIF_NOTIFYMESSAGE *)argv;
	ONVIF_NOTIFYMESSAGE * p_tmp = p_message;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify");
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
		
	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:Notify>");
	while (p_tmp)
	{
    	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:NotificationMessage>");   
    	offset += build_NotificationMessage_xml(p_buf+offset, mlen-offset, &p_tmp->NotificationMessage);
		offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:NotificationMessage>");
		
		p_tmp = p_tmp->next;
	}			
	offset += snprintf(p_buf+offset, mlen-offset, "</wsnt:Notify>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetWsdlUrl_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:GetWsdlUrlResponse>"
			"<tds:WsdlUrl>http://www.onvif.org/</tds:WsdlUrl>"
		"</tds:GetWsdlUrlResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int builid_PTZSpaces_xml(char * p_buf, int mlen, PTZ_NODE * p_node)
{
    int offset = 0;
    
    if (p_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpaceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:AbsolutePanTiltPositionSpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
				"<tt:YRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:YRange>"
			"</tt:AbsolutePanTiltPositionSpace>", 
			p_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.XRange.Max,
			p_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.YRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.AbsolutePanTiltPositionSpace.YRange.Max);
	}

	if (p_node->PTZNode.SupportedPTZSpaces.AbsoluteZoomPositionSpaceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:AbsoluteZoomPositionSpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
			"</tt:AbsoluteZoomPositionSpace>", 
			p_node->PTZNode.SupportedPTZSpaces.AbsoluteZoomPositionSpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.AbsoluteZoomPositionSpace.XRange.Max);
    }

    if (p_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpaceFlag)
    {
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:RelativePanTiltTranslationSpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/TranslationGenericSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
				"<tt:YRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:YRange>"
			"</tt:RelativePanTiltTranslationSpace>",
			p_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.XRange.Max,
			p_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.YRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.RelativePanTiltTranslationSpace.YRange.Max);
    }

    if (p_node->PTZNode.SupportedPTZSpaces.RelativeZoomTranslationSpaceFlag)
    {
		offset += snprintf(p_buf+offset, mlen-offset, 			
			"<tt:RelativeZoomTranslationSpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/ZoomSpaces/TranslationGenericSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
			"</tt:RelativeZoomTranslationSpace>", 
			p_node->PTZNode.SupportedPTZSpaces.RelativeZoomTranslationSpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.RelativeZoomTranslationSpace.XRange.Max);	
	}

	if (p_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpaceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 	
			"<tt:ContinuousPanTiltVelocitySpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/VelocityGenericSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
				"<tt:YRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:YRange>"
			"</tt:ContinuousPanTiltVelocitySpace>",
			p_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.XRange.Max,
			p_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.YRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.ContinuousPanTiltVelocitySpace.YRange.Max);	
	}

	if (p_node->PTZNode.SupportedPTZSpaces.ContinuousZoomVelocitySpaceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 	
			"<tt:ContinuousZoomVelocitySpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/ZoomSpaces/VelocityGenericSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
			"</tt:ContinuousZoomVelocitySpace>",
			p_node->PTZNode.SupportedPTZSpaces.ContinuousZoomVelocitySpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.ContinuousZoomVelocitySpace.XRange.Max);	
	}

	if (p_node->PTZNode.SupportedPTZSpaces.PanTiltSpeedSpaceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 	
			"<tt:PanTiltSpeedSpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/PanTiltSpaces/GenericSpeedSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
			"</tt:PanTiltSpeedSpace>",  
			p_node->PTZNode.SupportedPTZSpaces.PanTiltSpeedSpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.PanTiltSpeedSpace.XRange.Max);	
	}

	if (p_node->PTZNode.SupportedPTZSpaces.ZoomSpeedSpaceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 			
			"<tt:ZoomSpeedSpace>"
				"<tt:URI>http://www.onvif.org/ver10/tptz/ZoomSpaces/ZoomGenericSpeedSpace</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
			"</tt:ZoomSpeedSpace>",
			p_node->PTZNode.SupportedPTZSpaces.ZoomSpeedSpace.XRange.Min,
			p_node->PTZNode.SupportedPTZSpaces.ZoomSpeedSpace.XRange.Max);
	}

	return offset;
}

int build_PTZNode_xml(char * p_buf, int mlen, PTZ_NODE * p_node)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tptz:PTZNode token=\"%s\" FixedHomePosition=\"%s\">",
		p_node->PTZNode.token, 
		p_node->PTZNode.FixedHomePosition ? "true" : "false");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Name>%s</tt:Name>", p_node->PTZNode.Name);
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:SupportedPTZSpaces>");

	offset += builid_PTZSpaces_xml(p_buf+offset, mlen-offset, p_node);
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:SupportedPTZSpaces>");
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tt:MaximumNumberOfPresets>%d</tt:MaximumNumberOfPresets>", 
	    p_node->PTZNode.MaximumNumberOfPresets);
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:HomeSupported>%s</tt:HomeSupported>",
        p_node->PTZNode.HomeSupported ? "true" : "false");
    offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZNode>"); 

    return offset;
}

int build_GetNodes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetNodes_RES * p_res = (GetNodes_RES *)argv;
	PTZ_NODE * p_node = p_res->p_node;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, "http://www.onvif.org/ver20/ptz/wsdl/GetNodesResponse");
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetNodesResponse>");

	while (p_node)
	{
		offset += build_PTZNode_xml(p_buf+offset, mlen-offset, p_node);

    	p_node = p_node->next;
    }
    
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetNodesResponse>");	
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_GetNode_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	GetNode_RES * p_res = (GetNode_RES *)argv;
	PTZ_NODE * p_node = &p_res->struNode;
	if (NULL == p_node)
	{
		return ONVIF_ERR_NO_ENTITY;
	}
	
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetNodeResponse>");

	offset += build_PTZNode_xml(p_buf+offset, mlen-offset, p_node);
	   
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetNodeResponse>");	
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetPTZCfgs_RES * p_res = (GetPTZCfgs_RES *)argv;
	PTZ_CFG * p_ptz_cfg = p_res->p_PTZCfg;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfigurationsResponse>");

	while (p_ptz_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
            "<tptz:PTZConfiguration token=\"%s\">", 
            p_ptz_cfg->PTZConfiguration.token);
    	offset += build_PTZConfiguration_xml(p_buf+offset, mlen-offset, p_ptz_cfg);
    	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZConfiguration>");

		p_ptz_cfg = p_ptz_cfg->next;
	}	
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetConfigurationsResponse>");	

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	GetPTZCfg_RES * p_res = (GetPTZCfg_RES *)argv;
	PTZ_CFG * p_ptz_cfg = &p_res->struPTZCfg;
	if (NULL == p_ptz_cfg)
	{
		return ONVIF_ERR_NO_CONFIG;
	}
	
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfigurationResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, 
            "<tptz:PTZConfiguration token=\"%s\">", 
            p_ptz_cfg->PTZConfiguration.token);
	offset += build_PTZConfiguration_xml(p_buf+offset, mlen-offset, p_ptz_cfg);
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZConfiguration>");
   	
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetConfigurationResponse>");	

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_AddPTZConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddPTZConfigurationResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemovePTZConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemovePTZConfigurationResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	PTZ_NODE * p_node;
	
		
	GetPTZCfgOptions_RES * p_res = (GetPTZCfgOptions_RES *)argv;
	onvif_PTZConfigurationOptions *p_PTZOptions = &p_res->PTZConfigurationOptions;
	p_node = &p_res->struNode;
	assert(p_node);
	
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:PTZConfigurationOptions>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Spaces>");

	offset += builid_PTZSpaces_xml(p_buf+offset, mlen-offset, p_node);
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Spaces>");

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:PTZTimeout>"
			"<tt:Min>PT%dS</tt:Min>"
			"<tt:Max>PT%dS</tt:Max>"
		"</tt:PTZTimeout>", 
		p_PTZOptions->PTZTimeout.Min, 
		p_PTZOptions->PTZTimeout.Max);

	if (p_PTZOptions->PTControlDirectionFlag)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTControlDirection>");
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:EFlip>");
	    if (p_PTZOptions->PTControlDirection.EFlipMode_OFF)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>OFF</tt:Mode>");
	    }
	    if (p_PTZOptions->PTControlDirection.EFlipMode_ON)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>ON</tt:Mode>");
	    }
	    if (p_PTZOptions->PTControlDirection.EFlipMode_Extended)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>Extended</tt:Mode>");
	    }
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:EFlip>");
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Reverse>");
	    if (p_PTZOptions->PTControlDirection.ReverseMode_OFF)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>OFF</tt:Mode>");
	    }
	    if (p_PTZOptions->PTControlDirection.ReverseMode_ON)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>ON</tt:Mode>");
	    }
	    if (p_PTZOptions->PTControlDirection.ReverseMode_AUTO)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>AUTO</tt:Mode>");
	    }
	    if (p_PTZOptions->PTControlDirection.ReverseMode_Extended)
	    {
	        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>Extended</tt:Mode>");
	    }
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Reverse>");
	    offset += snprintf(p_buf+offset, mlen-offset, "</tt:PTControlDirection>");
	}
    
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZConfigurationOptions>");	
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetConfigurationOptionsResponse>");	

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_PTZ_GetStatus_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	onvif_PTZStatus * p_ptz_status = (onvif_PTZStatus *)argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetStatusResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:PTZStatus>");

	if (p_ptz_status->PositionFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Position>");
		if (p_ptz_status->Position.PanTiltFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\" space=\"http://www.onvif.org/ver10/tptz/PanTiltSpaces/PositionGenericSpace\" />",
				p_ptz_status->Position.PanTilt.x,
				p_ptz_status->Position.PanTilt.y);
		}	
		if (p_ptz_status->Position.ZoomFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:Zoom x=\"%0.1f\" space=\"http://www.onvif.org/ver10/tptz/ZoomSpaces/PositionGenericSpace\" />",
				p_ptz_status->Position.Zoom.x);
		}		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Position>");
	}

	if (p_ptz_status->MoveStatusFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:MoveStatus>");
		if (p_ptz_status->MoveStatus.PanTiltFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:PanTilt>%s</tt:PanTilt>",
				onvif_MoveStatusToString(p_ptz_status->MoveStatus.PanTilt));
		}
		if (p_ptz_status->MoveStatus.ZoomFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:Zoom>%s</tt:Zoom>",
				onvif_MoveStatusToString(p_ptz_status->MoveStatus.Zoom));
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:MoveStatus>");
	}
	
	if (p_ptz_status->ErrorFlag && strlen(p_ptz_status->Error) > 0)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Error>%s</tt:Error>", p_ptz_status->Error);
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:UtcTime>%s</tt:UtcTime>", p_ptz_status->UtcTime);
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZStatus>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetStatusResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


int build_ContinuousMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:ContinuousMoveResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_PTZ_Stop_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:StopResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_AbsoluteMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:AbsoluteMoveResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RelativeMove_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:RelativeMoveResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetPreset_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tptz:SetPresetResponse>"
	    	"<tptz:PresetToken>%s</tptz:PresetToken>"
	    "</tptz:SetPresetResponse>", argv);	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetPresets_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset;
	GetPresets_RES * p_res = (GetPresets_RES *)argv;
    
    offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetPresetsResponse>");

	for (i = 0; i < p_res->num; i++)
	{
	    if (p_res->presets[i].UsedFlag == 0)
	    {
	        continue;
	    }
	    
    	offset += snprintf(p_buf+offset, mlen-offset, 
    		"<tptz:Preset token=\"%s\">",
    		p_res->presets[i].PTZPreset.token);
    	offset += snprintf(p_buf+offset, mlen-offset, 
    		"<tt:Name>%s</tt:Name>", 
    		p_res->presets[i].PTZPreset.Name);
    		
		if (p_res->presets[i].PTZPreset.PTZPositionFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZPosition>");
			if (p_res->presets[i].PTZPreset.PTZPosition.PanTiltFlag)
			{
		    	offset += snprintf(p_buf+offset, mlen-offset, 
		    		"<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\" />",
		    		p_res->presets[i].PTZPreset.PTZPosition.PanTilt.x,
		    		p_res->presets[i].PTZPreset.PTZPosition.PanTilt.y);
		    }
		    if (p_res->presets[i].PTZPreset.PTZPosition.ZoomFlag)
		    {
		    	offset += snprintf(p_buf+offset, mlen-offset, 
		    		"<tt:Zoom x=\"%0.1f\" />",
		    		p_res->presets[i].PTZPreset.PTZPosition.Zoom.x);
		    }        
	        offset += snprintf(p_buf+offset, mlen-offset, "</tt:PTZPosition>");
	    }
	    
        offset += snprintf(p_buf+offset, mlen-offset, "</tptz:Preset>");
    }
    
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetPresetsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemovePreset_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:RemovePresetResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GotoPreset_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GotoPresetResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GotoHomePosition_rly_xml(char * p_buf, int mlen, const char * argv)
{
    int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GotoHomePositionResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetHomePosition_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:SetHomePositionResponse />");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetGuaranteedNumberOfVideoEncoderInstances_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tptz:GetGuaranteedNumberOfVideoEncoderInstancesResponse>"
		    "<trt:TotalNumber>%d</trt:TotalNumber>"
		    "<trt:JPEG>%d</trt:JPEG>"
		    "<trt:H264>%d</trt:H264>"
		    "<trt:MPEG4>%d</trt:MPEG4>"
		"</tptz:GetGuaranteedNumberOfVideoEncoderInstancesResponse>", 
		6, 1, 5, 2);	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetImagingSettings_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	
	GetImagSettings_RES * p_res = (GetImagSettings_RES *)argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetImagingSettingsResponse><timg:ImagingSettings>");

	offset += build_ImageSettings_xml(p_buf+offset, mlen-offset,(char *)&p_res->Imaging);
	
	offset += snprintf(p_buf+offset, mlen-offset, "</timg:ImagingSettings></timg:GetImagingSettingsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	GetImagingOptions_RES * p_res = (GetImagingOptions_RES *)argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetOptionsResponse><timg:ImagingOptions>");

	if (p_res->ImagingOptions.BacklightCompensationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:BacklightCompensation>");
		if (p_res->ImagingOptions.BacklightCompensation.Mode_OFF)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>OFF</tt:Mode>");
		}
		if (p_res->ImagingOptions.BacklightCompensation.Mode_ON)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>ON</tt:Mode>");
		}
		if (p_res->ImagingOptions.BacklightCompensation.LevelFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:Level>"
		            "<tt:Min>%0.1f</tt:Min>"
		            "<tt:Max>%0.1f</tt:Max>"
		        "</tt:Level>",
		        p_res->ImagingOptions.BacklightCompensation.Level.Min,
		        p_res->ImagingOptions.BacklightCompensation.Level.Max);
	    }    
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:BacklightCompensation>");
	}

	if (p_res->ImagingOptions.BrightnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:Brightness>"
	    	    "<tt:Min>%0.1f</tt:Min>"
	    	    "<tt:Max>%0.1f</tt:Max>"
		    "</tt:Brightness>",
			p_res->ImagingOptions.Brightness.Min, 
			p_res->ImagingOptions.Brightness.Max);
	}

	if (p_res->ImagingOptions.ColorSaturationFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:ColorSaturation>"
		        "<tt:Min>%0.1f</tt:Min>"
		        "<tt:Max>%0.1f</tt:Max>"
		    "</tt:ColorSaturation>",
			p_res->ImagingOptions.ColorSaturation.Min, 
			p_res->ImagingOptions.ColorSaturation.Max);
	}

	if (p_res->ImagingOptions.ContrastFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:Contrast>"
			    "<tt:Min>%0.1f</tt:Min>"
			    "<tt:Max>%0.1f</tt:Max>"
		    "</tt:Contrast>",
			p_res->ImagingOptions.Contrast.Min, 
			p_res->ImagingOptions.Contrast.Max);
	}

	if (p_res->ImagingOptions.ExposureFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Exposure>");
		if (p_res->ImagingOptions.Exposure.Mode_AUTO)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>AUTO</tt:Mode>");
		}
		if (p_res->ImagingOptions.Exposure.Mode_MANUAL)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>MANUAL</tt:Mode>");
		}
		if (p_res->ImagingOptions.Exposure.Priority_LowNoise)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Priority>LowNoise</tt:Priority>");
		}
		if (p_res->ImagingOptions.Exposure.Priority_FrameRate)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Priority>FrameRate</tt:Priority>");
		}
		if (p_res->ImagingOptions.Exposure.MinExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:MinExposureTime>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:MinExposureTime>",
				p_res->ImagingOptions.Exposure.MinExposureTime.Min, 
				p_res->ImagingOptions.Exposure.MinExposureTime.Max);
		}
		if (p_res->ImagingOptions.Exposure.MaxExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:MaxExposureTime>"
			        "<tt:Min>%0.1f</tt:Min>"
			        "<tt:Max>%0.1f</tt:Max>"
			    "</tt:MaxExposureTime>",
				p_res->ImagingOptions.Exposure.MaxExposureTime.Min, 
				p_res->ImagingOptions.Exposure.MaxExposureTime.Max);	
		}	
		if (p_res->ImagingOptions.Exposure.MinGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:MinGain>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:MinGain>",
				p_res->ImagingOptions.Exposure.MinGain.Min, 
				p_res->ImagingOptions.Exposure.MinGain.Max);
		}	
		if (p_res->ImagingOptions.Exposure.MaxGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:MaxGain>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:MaxGain>",
				p_res->ImagingOptions.Exposure.MaxGain.Min, 
				p_res->ImagingOptions.Exposure.MaxGain.Max);
		}
		if (p_res->ImagingOptions.Exposure.MinIrisFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:MinIris>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:MinIris>",
				p_res->ImagingOptions.Exposure.MinIris.Min, 
				p_res->ImagingOptions.Exposure.MinIris.Max);
		}
		if (p_res->ImagingOptions.Exposure.MaxIrisFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:MaxIris>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:MaxIris>",
				p_res->ImagingOptions.Exposure.MaxIris.Min, 
				p_res->ImagingOptions.Exposure.MaxIris.Max);	
		}	
		if (p_res->ImagingOptions.Exposure.ExposureTimeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:ExposureTime>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:ExposureTime>",
				p_res->ImagingOptions.Exposure.ExposureTime.Min, 
				p_res->ImagingOptions.Exposure.ExposureTime.Max);	
		}
		if (p_res->ImagingOptions.Exposure.GainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:Gain>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:Gain>",
				p_res->ImagingOptions.Exposure.Gain.Min, 
				p_res->ImagingOptions.Exposure.Gain.Max);	
		}	
		if (p_res->ImagingOptions.Exposure.IrisFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:Iris>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:Iris>",
				p_res->ImagingOptions.Exposure.Iris.Min, 
				p_res->ImagingOptions.Exposure.Iris.Max);	
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Exposure>");
	}	

	if (p_res->ImagingOptions.FocusFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Focus>");
		if (p_res->ImagingOptions.Focus.AutoFocusModes_AUTO)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:AutoFocusModes>AUTO</tt:AutoFocusModes>");
		}
		if (p_res->ImagingOptions.Focus.AutoFocusModes_MANUAL)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:AutoFocusModes>MANUAL</tt:AutoFocusModes>");
		}
		if (p_res->ImagingOptions.Focus.DefaultSpeedFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:DefaultSpeed>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:DefaultSpeed>",
				p_res->ImagingOptions.Focus.DefaultSpeed.Min, 
				p_res->ImagingOptions.Focus.DefaultSpeed.Max);
		}	
		if (p_res->ImagingOptions.Focus.NearLimitFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:NearLimit>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:NearLimit>",
				p_res->ImagingOptions.Focus.NearLimit.Min, 
				p_res->ImagingOptions.Focus.NearLimit.Max);
		}	
		if (p_res->ImagingOptions.Focus.FarLimitFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:FarLimit>"
		    	    "<tt:Min>%0.1f</tt:Min>"
		    	    "<tt:Max>%0.1f</tt:Max>"
			    "</tt:FarLimit>",
				p_res->ImagingOptions.Focus.FarLimit.Min, 
				p_res->ImagingOptions.Focus.FarLimit.Max);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Focus>");
	}	
	
	if (p_res->ImagingOptions.IrCutFilterMode_ON)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>ON</tt:IrCutFilterModes>");
	}
	if (p_res->ImagingOptions.IrCutFilterMode_OFF)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>OFF</tt:IrCutFilterModes>");
	}
	if (p_res->ImagingOptions.IrCutFilterMode_AUTO)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>AUTO</tt:IrCutFilterModes>");
	}

	if (p_res->ImagingOptions.SharpnessFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:Sharpness>"
		        "<tt:Min>%0.1f</tt:Min>"
		        "<tt:Max>%0.1f</tt:Max>"
		    "</tt:Sharpness>",
			p_res->ImagingOptions.Sharpness.Min, 
			p_res->ImagingOptions.Sharpness.Max);
	}	

	if (p_res->ImagingOptions.WideDynamicRangeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:WideDynamicRange>");
		if (p_res->ImagingOptions.WideDynamicRange.Mode_ON)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>ON</tt:Mode>");
		}
		if (p_res->ImagingOptions.WideDynamicRange.Mode_OFF)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>OFF</tt:Mode>");
		}
		if (p_res->ImagingOptions.WideDynamicRange.LevelFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:Level>"
			        "<tt:Min>%0.1f</tt:Min>"
			        "<tt:Max>%0.1f</tt:Max>"
			    "</tt:Level>",
				p_res->ImagingOptions.WideDynamicRange.Level.Min, 
				p_res->ImagingOptions.WideDynamicRange.Level.Max);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:WideDynamicRange>");
	}

	if (p_res->ImagingOptions.WhiteBalanceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:WhiteBalance>");
		if (p_res->ImagingOptions.WhiteBalance.Mode_AUTO)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>AUTO</tt:Mode>");
		}
		if (p_res->ImagingOptions.WhiteBalance.Mode_MANUAL)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Mode>MANUAL</tt:Mode>");
		}
		if (p_res->ImagingOptions.WhiteBalance.YrGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:YrGain>"
			        "<tt:Min>%0.1f</tt:Min>"
			        "<tt:Max>%0.1f</tt:Max>"
			    "</tt:YrGain>",
				p_res->ImagingOptions.WhiteBalance.YrGain.Min, 
				p_res->ImagingOptions.WhiteBalance.YrGain.Max);
		}	
		if (p_res->ImagingOptions.WhiteBalance.YbGainFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:YbGain>"
			        "<tt:Min>%0.1f</tt:Min>"
			        "<tt:Max>%0.1f</tt:Max>"
			    "</tt:YbGain>",
				p_res->ImagingOptions.WhiteBalance.YbGain.Min, 
				p_res->ImagingOptions.WhiteBalance.YbGain.Max);
		}	
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:WhiteBalance>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</timg:ImagingOptions></timg:GetOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetImagingSettings_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:SetImagingSettingsResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetMoveOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{	
	int offset;
	
	offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<timg:GetMoveOptionsResponse>"
			"<timg:MoveOptions>"
			"</timg:MoveOptions>"
		"</timg:GetMoveOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_Move_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<timg:MoveResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_IMG_GetStatus_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	onvif_ImagingStatus * p_res = (onvif_ImagingStatus *)argv;

	offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetStatusResponse>");
	
	if (p_res->FocusStatusFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<timg:Status>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:FocusStatus20>");
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Position>%0.1f</tt:Position>"
			"<tt:MoveStatus>%s</tt:MoveStatus>",
			p_res->FocusStatus.Position,
			onvif_MoveStatusToString(p_res->FocusStatus.MoveStatus));
		if (p_res->FocusStatus.ErrorFlag && p_res->FocusStatus.Error[0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Error>%s</tt:Error>", p_res->FocusStatus.Error);
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:FocusStatus20>");
		offset += snprintf(p_buf+offset, mlen-offset, "</timg:Status>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetStatusResponse>");	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_IMG_Stop_rly_xml(char * p_buf, int mlen, const char * argv)
{
	return ONVIF_ERR_NO_IMAGEING_FOR_SOURCE;
}

int build_GetUsers_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	int offset = snprintf(p_buf, mlen, xml_hdr);
	GetUsers_RES * p_res = (GetUsers_RES *)argv;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetUsersResponse>");

	for (i = 0; i < MAX_USERS; i++)
	{
		if (p_res->users[i].Username[0] != '\0')
		{		
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tds:User>"
					"<tt:Username>%s</tt:Username>"
					"<tt:UserLevel>%s</tt:UserLevel>"
				"</tds:User>", 
				p_res->users[i].Username, 
				onvif_UserLevelToString(p_res->users[i].UserLevel));
		}
	}
    
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetUsersResponse>");    	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreateUsers_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:CreateUsersResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_DeleteUsers_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:DeleteUsersResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetUser_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetUserResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_StartFirmwareUpgrade_rly_xml(char * p_buf, int mlen, const char * argv)
{
	StartFirmwareUpgrade_RES * p_res = (StartFirmwareUpgrade_RES *) argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:StartFirmwareUpgradeResponse>"
	   		"<tds:UploadUri>%s</tds:UploadUri>"
	   		"<tds:UploadDelay>PT%dS</tds:UploadDelay>"
	   		"<tds:ExpectedDownTime>PT%dS</tds:ExpectedDownTime>"
  		"</tds:StartFirmwareUpgradeResponse>", 
  		p_res->UploadUri, p_res->UploadDelay, p_res->UploadDelay);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_OSD_xml(char * p_buf, int mlen, ONVIF_OSD * p_osd)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoSourceConfigurationToken>%s</tt:VideoSourceConfigurationToken>",
		p_osd->OSD.VideoSourceConfigurationToken);
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>",
		onvif_OSDTypeToString(p_osd->OSD.Type));

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Position>"
			"<tt:Type>%s</tt:Type>"
			"<tt:Pos x=\"%0.2f\" y=\"%0.2f\">"
			"</tt:Pos>"
		"</tt:Position>", 
		onvif_OSDPosTypeToString(p_osd->OSD.Position.Type), 
		p_osd->OSD.Position.Pos.x,
		p_osd->OSD.Position.Pos.y);

	if (p_osd->OSD.Type == OSDType_Text)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:TextString>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", onvif_OSDTextTypeToString(p_osd->OSD.TextString.Type));

		if (p_osd->OSD.TextString.Type == OSDTextType_Date)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>%s</tt:DateFormat>", p_osd->OSD.TextString.DateFormat);
		}
		else if (p_osd->OSD.TextString.Type == OSDTextType_Time)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:TimeFormat>%s</tt:TimeFormat>", p_osd->OSD.TextString.TimeFormat);
		}
		else if (p_osd->OSD.TextString.Type == OSDTextType_DateAndTime)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>%s</tt:DateFormat>", p_osd->OSD.TextString.DateFormat);
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:TimeFormat>%s</tt:TimeFormat>", p_osd->OSD.TextString.TimeFormat);
		}
		else if (p_osd->OSD.TextString.Type == OSDTextType_Plain)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:PlainText>%s</tt:PlainText>", p_osd->OSD.TextString.PlainText);
		}

		if (p_osd->OSD.TextString.FontSizeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:FontSize>%d</tt:FontSize>", p_osd->OSD.TextString.FontSize);
		}

		if (p_osd->OSD.TextString.FontColorFlag)
		{
			if (p_osd->OSD.TextString.FontColor.TransparentFlag)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:FontColor Transparent=\"%d\">", p_osd->OSD.TextString.FontColor.Transparent);
			}	
			else
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:FontColor>");
			}

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Color X=\"%0.1f\" Y=\"%0.1f\" Z=\"%0.1f\"></tt:Color>", 
				p_osd->OSD.TextString.FontColor.X, p_osd->OSD.TextString.FontColor.Y, p_osd->OSD.TextString.FontColor.Z);

			offset += snprintf(p_buf+offset, mlen-offset, "</tt:FontColor>");	
		}

		if (p_osd->OSD.TextString.BackgroundColorFlag)
		{
			if (p_osd->OSD.TextString.BackgroundColor.TransparentFlag)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:BackgroundColor Transparent=\"%d\">", p_osd->OSD.TextString.BackgroundColor.Transparent);
			}	
			else
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:BackgroundColor>");
			}

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Color X=\"%0.1f\" Y=\"%0.1f\" Z=\"%0.1f\"></tt:Color>", 
				p_osd->OSD.TextString.BackgroundColor.X, p_osd->OSD.TextString.BackgroundColor.Y, p_osd->OSD.TextString.BackgroundColor.Z);

			offset += snprintf(p_buf+offset, mlen-offset, "</tt:BackgroundColor>");	
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tt:TextString>");
	}
	else if (p_osd->OSD.Type == OSDType_Image)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Image><tt:ImgPath>%s</tt:ImgPath></tt:Image>", p_osd->OSD.Image.ImgPath);
	}	
	
	return offset;
}

int build_GetOSDs_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	ONVIF_OSD * p_osd;
	GetOSDs_RES * p_res = (GetOSDs_RES *) argv;
	
	p_osd = p_res->OSDs;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetOSDsResponse>");

	while (p_osd)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:OSDs token=\"%s\">", p_osd->OSD.token);
		offset += build_OSD_xml(p_buf+offset, mlen-offset, p_osd);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:OSDs>");
		p_osd = p_osd->next;
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetOSDsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_GetOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	GetOSD_RES * p_res = (GetOSD_RES *) argv;	
	ONVIF_OSD * p_osd = &p_res->struOSD;
	if (NULL == p_osd)
	{
		return ONVIF_ERR_NO_CONFIG;
	}
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetOSDResponse>");

	if (p_osd)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:OSD token=\"%s\">", p_osd->OSD.token);
		offset += build_OSD_xml(p_buf+offset, mlen-offset, p_osd);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:OSD>");
		p_osd = p_osd->next;
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetOSDResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetOSDResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreateOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:CreateOSDResponse><trt:OSDToken>%s</trt:OSDToken></trt:CreateOSDResponse>", argv);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_DeleteOSD_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:DeleteOSDResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetOSDOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	GetOSDOptions_RES * p_res = (GetOSDOptions_RES *)argv;
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetOSDOptionsResponse><trt:OSDOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaximumNumberOfOSDs Total=\"%d\"",
		p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.Total);
	if (p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.ImageFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Image=\"%d\"", 
			p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.Image);
	}
	if (p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.PlainTextFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " PlainText=\"%d\"", 
			p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.PlainText);
	}
	if (p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.DateFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Date=\"%d\"", 
			p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.Date);
	}
	if (p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.TimeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Time=\"%d\"", 
			p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.Time);
	}
	if (p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.DateAndTimeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " DateAndTime=\"%d\"", 
			p_res->OSDConfigurationOptions.MaximumNumberOfOSDs.DateAndTime);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "></tt:MaximumNumberOfOSDs>");

	if (p_res->OSDConfigurationOptions.OSDType_Text)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTypeToString(OSDType_Text));
	}
	if (p_res->OSDConfigurationOptions.OSDType_Image)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTypeToString(OSDType_Image));
	}
	if (p_res->OSDConfigurationOptions.OSDType_Extended)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
			onvif_OSDTypeToString(OSDType_Extended));
	}

	if (p_res->OSDConfigurationOptions.OSDPosType_LowerLeft)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_LowerLeft));
	}
	if (p_res->OSDConfigurationOptions.OSDPosType_LowerRight)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_LowerRight));
	}
	if (p_res->OSDConfigurationOptions.OSDPosType_UpperLeft)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_UpperLeft));
	}
	if (p_res->OSDConfigurationOptions.OSDPosType_UpperRight)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_UpperRight));
	}
	if (p_res->OSDConfigurationOptions.OSDPosType_Custom)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>%s</tt:PositionOption>", 
			onvif_OSDPosTypeToString(OSDPosType_Custom));
	}

	if (p_res->OSDConfigurationOptions.TextOptionFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:TextOption>");
		
		if (p_res->OSDConfigurationOptions.TextOption.OSDTextType_Plain)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
				onvif_OSDTextTypeToString(OSDTextType_Plain));
		}
		if (p_res->OSDConfigurationOptions.TextOption.OSDTextType_Date)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
				onvif_OSDTextTypeToString(OSDTextType_Date));
		}
		if (p_res->OSDConfigurationOptions.TextOption.OSDTextType_Time)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
				onvif_OSDTextTypeToString(OSDTextType_Time));
		}
		if (p_res->OSDConfigurationOptions.TextOption.OSDTextType_DateAndTime)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", 
				onvif_OSDTextTypeToString(OSDTextType_DateAndTime));
		}

		if (p_res->OSDConfigurationOptions.TextOption.FontSizeRangeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:FontSizeRange>"
					"<tt:Min>%d</tt:Min>"				
					"<tt:Max>%d</tt:Max>"				
				"</tt:FontSizeRange>", 
				p_res->OSDConfigurationOptions.TextOption.FontSizeRange.Min,
				p_res->OSDConfigurationOptions.TextOption.FontSizeRange.Max);
		}

		for (i = 0; i < p_res->OSDConfigurationOptions.TextOption.DateFormatSize; i++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>%s</tt:DateFormat>",
				p_res->OSDConfigurationOptions.TextOption.DateFormat[i]);
		}
		
		for (i = 0; i < p_res->OSDConfigurationOptions.TextOption.TimeFormatSize; i++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:TimeFormat>%s</tt:TimeFormat>",
				p_res->OSDConfigurationOptions.TextOption.TimeFormat[i]);
		}

		// build onvif color options ...

		offset += snprintf(p_buf+offset, mlen-offset, "</tt:TextOption>");
	}

	if (p_res->OSDConfigurationOptions.ImageOptionFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ImageOption>");

		for (i = 0; i < p_res->OSDConfigurationOptions.ImageOption.ImagePathSize; i++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:ImagePath>%s</tt:ImagePath>",
				p_res->OSDConfigurationOptions.ImageOption.ImagePath[i]);
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:ImageOption>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:OSDOptions></trt:GetOSDOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:SetConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_StartMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:StartMulticastStreamingResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_StopMulticastStreaming_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:StopMulticastStreamingResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	
	GetMetadataConfigurations_RES * p_res = (GetMetadataConfigurations_RES *)argv;
	ONVIF_METADATA_CFG * p_cfg = p_res->metadata_cfg;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationsResponse>");
	while (p_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_cfg->MetadataConfiguration.token);
		offset += build_MetadataConfiguration_xml(p_buf+offset, mlen-offset, p_cfg);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
		
		p_cfg = p_cfg->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetMetadataConfigurationsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset;
	
	GetMetadataConfiguration_RES * p_res = (GetMetadataConfiguration_RES *)argv;
    ONVIF_METADATA_CFG * p_cfg = &p_res->strumetadata;
    if (NULL == p_cfg)
    {
    	return ONVIF_ERR_NO_CONFIG;
    }

    offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_cfg->MetadataConfiguration.token);
	offset += build_MetadataConfiguration_xml(p_buf+offset, mlen-offset, p_cfg);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetMetadataConfigurationResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetCompatibleMetadataConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;	
	
	GetCompatibleM_Cfgs_RES * p_res = (GetCompatibleM_Cfgs_RES *)argv;
	ONVIF_METADATA_CFG * p_cfg = p_res->metadata_cfg;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleMetadataConfigurationsResponse>");
	while (p_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_cfg->MetadataConfiguration.token);
		offset += build_MetadataConfiguration_xml(p_buf+offset, mlen-offset, p_cfg);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
		
		p_cfg = p_cfg->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleMetadataConfigurationsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetMetadataConfigurationOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	ONVIF_PROFILE * p_profile = NULL;
	GetMetadataCfgOptions_RES * p_res = (GetMetadataCfgOptions_RES *) argv;
	

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZStatusFilterOptions>");

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:PanTiltStatusSupported>%s</tt:PanTiltStatusSupported>"
		"<tt:ZoomStatusSupported>%s</tt:ZoomStatusSupported>",
		p_res->M_Options.PTZStatusFilterOptions.PanTiltStatusSupported ? "true" : "false",
		p_res->M_Options.PTZStatusFilterOptions.ZoomStatusSupported ? "true" : "false");

	if (p_res->M_Options.PTZStatusFilterOptions.PanTiltPositionSupportedFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:PanTiltPositionSupported>%s</tt:PanTiltPositionSupported>",
			p_res->M_Options.PTZStatusFilterOptions.PanTiltPositionSupported ? "true" : "false");
	}

	if (p_res->M_Options.PTZStatusFilterOptions.ZoomPositionSupportedFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:ZoomPositionSupported>%s</tt:ZoomPositionSupported>",
			p_res->M_Options.PTZStatusFilterOptions.ZoomPositionSupported ? "true" : "false");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:PTZStatusFilterOptions>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetMetadataConfigurationOptionsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_SetMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetMetadataConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_AddMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddMetadataConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemoveMetadataConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveMetadataConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

#ifdef PROFILE_G_SUPPORT

int build_GetRecordingSummary_rly_xml(char * p_buf, int mlen, const char * argv)
{
	char DataFrom[64];
	char DataUntil[64];
	
	GetRecordingSummary_RES * p_res = (GetRecordingSummary_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	onvif_get_time_str_s(DataFrom, sizeof(DataFrom), p_res->Summary.DataFrom, 0);
	onvif_get_time_str_s(DataUntil, sizeof(DataUntil), p_res->Summary.DataUntil, 0);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:GetRecordingSummaryResponse>"
			"<tse:Summary>"
				"<tt:DataFrom>%s</tt:DataFrom>"
				"<tt:DataUntil>%s</tt:DataUntil>"
				"<tt:NumberRecordings>%d</tt:NumberRecordings>"
			"</tse:Summary>"
		"</tse:GetRecordingSummaryResponse>",
		DataFrom,
		DataUntil,
		p_res->Summary.NumberRecordings);
		
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RecordingSourceInformation_xml(char * p_buf, int mlen, onvif_RecordingSourceInformation * p_req)
{
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Source>"
			"<tt:SourceId>%s</tt:SourceId>"
			"<tt:Name>%s</tt:Name>"
			"<tt:Location>%s</tt:Location>"
			"<tt:Description>%s</tt:Description>"
			"<tt:Address>%s</tt:Address>"
		"</tt:Source>",
		p_req->SourceId,
		p_req->Name,
		p_req->Location,
		p_req->Description,
		p_req->Address);
		
	return offset;		
}

int build_TrackInformation_xml(char * p_buf, int mlen, onvif_TrackInformation * p_req)
{
	int offset = 0;
	char DataFrom[64], DataTo[64];

	onvif_get_time_str_s(DataFrom, sizeof(DataFrom), p_req->DataFrom, 0);
	onvif_get_time_str_s(DataTo, sizeof(DataTo), p_req->DataTo, 0);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:TrackToken>%s</tt:TrackToken>"
		"<tt:TrackType>%s</tt:TrackType>"
		"<tt:Description>%s</tt:Description>"
		"<tt:DataFrom>%s</tt:DataFrom>"
		"<tt:DataTo>%s</tt:DataTo>",
		p_req->TrackToken,
		onvif_TrackTypeToString(p_req->TrackType),
		p_req->Description,
		DataFrom,
		DataTo);

	return offset;			
}

int build_RecordingInformation_xml(char * p_buf, int mlen, onvif_RecordingInformation * p_req)
{
	int i;
	int offset = 0;
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingToken>%s</tt:RecordingToken>", p_req->RecordingToken);
	offset += build_RecordingSourceInformation_xml(p_buf+offset, mlen-offset, &p_req->Source);

	if (p_req->EarliestRecordingFlag)
	{
		char EarliestRecording[64];
		onvif_get_time_str_s(EarliestRecording, sizeof(EarliestRecording), p_req->EarliestRecording, 0);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:EarliestRecording>%s</tt:EarliestRecording>", EarliestRecording);
	}

	if (p_req->LatestRecordingFlag)
	{
		char LatestRecording[64];
		onvif_get_time_str_s(LatestRecording, sizeof(LatestRecording), p_req->LatestRecording, 0);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:LatestRecording>%s</tt:LatestRecording>", LatestRecording);
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Content>%s</tt:Content>", p_req->Content);

	for (i = 0; i < p_req->sizeTrack; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Track>");
		offset += build_TrackInformation_xml(p_buf+offset, mlen-offset, &p_req->Track[i]);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Track>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingStatus>%s</tt:RecordingStatus>", 
		onvif_RecordingStatusToString(p_req->RecordingStatus));

	return offset;		
}

int build_GetRecordingInformation_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetRecordingInformation_RES * p_res = (GetRecordingInformation_RES *)argv;

	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetRecordingInformationResponse><tse:RecordingInformation>");	
	offset += build_RecordingInformation_xml(p_buf+offset, mlen-offset, &p_res->RecordingInformation);
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:RecordingInformation></tse:GetRecordingInformationResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_TrackAttributes_xml(char * p_buf, int mlen, onvif_TrackAttributes * p_req)
{
	int offset = 0;

	offset += build_TrackInformation_xml(p_buf+offset, mlen-offset, &p_req->TrackInformation);

	if (p_req->VideoAttributesFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoAttributes>");
		if (p_req->VideoAttributes.BitrateFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Bitrate>%d</tt:Bitrate>", p_req->VideoAttributes.Bitrate);
		}
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:Width>%d</tt:Width>"
			"<tt:Height>%d</tt:Height>"
			"<tt:Encoding>%s</tt:Encoding>"
			"<tt:Framerate>%0.1f</tt:Framerate>",
			p_req->VideoAttributes.Width,
			p_req->VideoAttributes.Height,
			onvif_VideoEncodingToString(p_req->VideoAttributes.Encoding),
			p_req->VideoAttributes.Framerate);			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoAttributes>");		
	}

	if (p_req->AudioAttributesFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:AudioAttributes>");
		if (p_req->AudioAttributes.BitrateFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Bitrate>%d</tt:Bitrate>", p_req->AudioAttributes.Bitrate);
		}
		offset += snprintf(p_buf+offset, mlen-offset,
			"<tt:Encoding>%s</tt:Encoding>"
			"<tt:Samplerate>%d</tt:Samplerate>",
			onvif_AudioEncodingToString(p_req->AudioAttributes.Encoding),
			p_req->AudioAttributes.Samplerate);			
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioAttributes>");
	}

	if (p_req->MetadataAttributesFlag)
	{		
		if (p_req->MetadataAttributes.PtzSpacesFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MetadataAttributes PtzSpaces=\"%s\">", p_req->MetadataAttributes.PtzSpaces);
		}
		else
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:MetadataAttributes>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:CanContainPTZ>%s</tt:CanContainPTZ>"
			"<tt:CanContainAnalytics>%s</tt:CanContainAnalytics>"
			"<tt:CanContainNotifications>%s</tt:CanContainNotifications>",
			p_req->MetadataAttributes.CanContainPTZ ? "true" : "false",
			p_req->MetadataAttributes.CanContainAnalytics ? "true" : "false",
			p_req->MetadataAttributes.CanContainNotifications ? "true" : "false");
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:MetadataAttributes>");
	}

	return offset;
}

int build_MediaAttributes_xml(char * p_buf, int mlen, onvif_MediaAttributes * p_req)
{
	int i, offset = 0;
	char From[64], Until[64];

	onvif_get_time_str_s(From, sizeof(From), p_req->From, 0);
	onvif_get_time_str_s(Until, sizeof(Until), p_req->Until, 0);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingToken>%s</tt:RecordingToken>", p_req->RecordingToken);

	for (i = 0; i < p_req->sizeTrackAttributes; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:TrackAttributes>");
		offset += build_TrackAttributes_xml(p_buf+offset, mlen-offset, &p_req->TrackAttributes[i]);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:TrackAttributes>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:From>%s</tt:From>"
    	"<tt:Until>%s</tt:Until>", 
    	From,
    	Until);

	return offset;    	
}

int build_GetMediaAttributes_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	GetMediaAttributes_RES * p_res = (GetMediaAttributes_RES *)argv;

	int offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetMediaAttributesResponse>");

	for (i = 0; i < p_res->sizeMediaAttributes; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tse:MediaAttributes>");
		offset += build_MediaAttributes_xml(p_buf+offset, mlen-offset, &p_res->MediaAttributes[i]);
		offset += snprintf(p_buf+offset, mlen-offset, "</tse:MediaAttributes>");
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:GetMediaAttributesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

int build_FindRecordings_rly_xml(char * p_buf, int mlen, const char * argv)
{
	FindRecordings_RES * p_res = (FindRecordings_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:FindRecordingsResponse>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:FindRecordingsResponse>",
		p_res->SearchToken);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetRecordingSearchResults_rly_xml(char * p_buf, int mlen, const char * argv)
{
	ONVIF_RECINF * p_RecInf;
	GetRecordingSearchResults_RES * p_res = (GetRecordingSearchResults_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetRecordingSearchResultsResponse><tse:ResultList>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:SearchState>%s</tt:SearchState>", onvif_SearchStateToString(p_res->ResultList.SearchState));

	p_RecInf = p_res->ResultList.RecordInformation;
	while (p_RecInf)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingInformation>");
		offset += build_RecordingInformation_xml(p_buf+offset, mlen-offset, &p_RecInf->RecordingInformation);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:RecordingInformation>");

		p_RecInf = p_RecInf->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:ResultList></tse:GetRecordingSearchResultsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_FindEvents_rly_xml(char * p_buf, int mlen, const char * argv)
{
	FindEvents_RES * p_res = (FindEvents_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:FindEventsResponse>"
			"<tse:SearchToken>%s</tse:SearchToken>"
		"</tse:FindEventsResponse>",
		p_res->SearchToken);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_FindEventResult_xml(char * p_buf, int mlen, onvif_FindEventResult * p_req)
{
	int offset = 0;
	char TimeBuff[64];

	onvif_get_time_str_s(TimeBuff, sizeof(TimeBuff), p_req->Time, 0);

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RecordingToken>%s</tt:RecordingToken>"
     	"<tt:TrackToken>%s</tt:TrackToken>"
     	"<tt:Time>%s</tt:Time>",
     	p_req->RecordingToken, 
     	p_req->TrackToken, 
     	TimeBuff);

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Event>");
	offset += build_NotificationMessage_xml(p_buf+offset, mlen-offset, &p_req->Event);
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Event>");
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:StartStateEvent>%s</tt:StartStateEvent>",
		p_req->StartStateEvent ? "true" : "false");
	
	return offset;
}

int build_GetEventSearchResults_rly_xml(char * p_buf, int mlen, const char * argv)
{
	ONVIF_EVENTRESULT * p_EventResult;
	GetEventSearchResults_RES * p_res = (GetEventSearchResults_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tse:GetEventSearchResultsResponse><tse:ResultList>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:SearchState>%s</tt:SearchState>", onvif_SearchStateToString(p_res->ResultList.SearchState));

	p_EventResult = p_res->ResultList.Result;
	while (p_EventResult)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Result>");
		offset += build_FindEventResult_xml(p_buf+offset, mlen-offset, &p_EventResult->FindEventResult);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Result>");

		p_EventResult = p_EventResult->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tse:ResultList></tse:GetEventSearchResultsResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_EndSearch_rly_xml(char * p_buf, int mlen, const char * argv)
{
	char EndPoint[64];
	EndSearch_RES * p_res = (EndSearch_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	onvif_get_time_str_s(EndPoint, sizeof(EndPoint), p_res->Endpoint, 0);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:EndSearchResponse>"
			"<tse:Endpoint>%s</tse:Endpoint>"
		"</tse:EndSearchResponse>",
		EndPoint);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetSearchState_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetSearchState_RES * p_res = (GetSearchState_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tse:GetSearchStateResponse>"
			"<tse:State>%s</tse:State>"
		"</tse:GetSearchStateResponse>",
		onvif_SearchStateToString(p_res->State));
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreateRecording_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trc:CreateRecordingResponse>"
			"<trc:RecordingToken>%s</trc:RecordingToken>"
		"</trc:CreateRecordingResponse>",
		argv);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_DeleteRecording_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:DeleteRecordingResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RecordingConfiguration_xml(char * p_buf, int mlen, onvif_RecordingConfiguration * p_req)
{
	int offset = 0;
	
	offset += build_RecordingSourceInformation_xml(p_buf+offset, mlen-offset, &p_req->Source);
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Content>%s</tt:Content>", p_req->Content);
	
	if (p_req->MaximumRetentionTimeFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:MaximumRetentionTime>PT%dS</tt:MaximumRetentionTime>", p_req->MaximumRetentionTime);
	}

	return offset;
}

int build_TrackConfiguration(char * p_buf, int mlen, onvif_TrackConfiguration * p_req)
{
	int offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:TrackType>%s</tt:TrackType>"
		"<tt:Description>%s</tt:Description>",
		onvif_TrackTypeToString(p_req->TrackType),
		p_req->Description);

	return offset;		
}

int build_GetRecordings_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetRecordings_RES * p_res = (GetRecordings_RES *)argv;
	ONVIF_RECORDING * p_recording = p_res->p_recording;
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingsResponse>");
	while (p_recording)
	{
		ONVIF_TRACK * p_track = p_recording->Recording.Tracks;
		
		offset += snprintf(p_buf+offset, mlen-offset, "<trc:RecordingItem>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RecordingToken>%s</tt:RecordingToken>", p_recording->Recording.RecordingToken);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Configuration>");
		offset += build_RecordingConfiguration_xml(p_buf+offset, mlen-offset, &p_recording->Recording.Configuration);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Configuration>");
		
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Tracks>");
		while (p_track)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Track><tt:TrackToken>%s</tt:TrackToken><tt:Configuration>", p_track->Track.TrackToken);
			offset += build_TrackConfiguration(p_buf+offset, mlen-offset, &p_track->Track.Configuration);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Configuration></tt:Track>");
			
			p_track = p_track->next;
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Tracks>");
		
		offset += snprintf(p_buf+offset, mlen-offset, "</trc:RecordingItem>");
		
		p_recording = p_recording->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:GetRecordingsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetRecordingConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetRecordingConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetRecordingConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	
	GetRecordingConfiguration_RES * p_res = (GetRecordingConfiguration_RES *)argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingConfigurationResponse><trc:RecordingConfiguration>");
	offset += build_RecordingConfiguration_xml(p_buf+offset, mlen-offset, &p_res->Configuration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:RecordingConfiguration></trc:GetRecordingConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreateTrack_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trc:CreateTrackResponse>"
			"<trc:TrackToken>%s</trc:TrackToken>"
		"</trc:CreateTrackResponse>",
		argv);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_DeleteTrack_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:DeleteTrackResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetTrackConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;

	GetTrackConfiguration_RES * p_res = (GetTrackConfiguration_RES *)argv;
	
	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetTrackConfigurationResponse><trc:TrackConfiguration>");
	offset += build_TrackConfiguration(p_buf+offset, mlen-offset, &p_res->Configuration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:TrackConfiguration></trc:GetTrackConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetTrackConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetTrackConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RecordingJobConfiguration_xml(char * p_buf, int mlen, onvif_RecordingJobConfiguration * p_req)
{
	int i, j, offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RecordingToken>%s</tt:RecordingToken>"
		"<tt:Mode>%s</tt:Mode>"
		"<tt:Priority>%d</tt:Priority>", 
		p_req->RecordingToken,
		p_req->Mode, 
		p_req->Priority);

	for (i = 0; i < p_req->sizeSource; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
		
		if (p_req->Source[i].SourceTokenFlag)
		{
			if (p_req->Source[i].SourceToken.TypeFlag)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken Type=\"%s\">", p_req->Source[i].SourceToken.Type);
			}
			else
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken>");
			}

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Token>%s</tt:Token>", p_req->Source[i].SourceToken.Token);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:SourceToken>");
		}

		for (j = 0; j < p_req->Source[i].sizeTracks; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
				"<tt:Tracks>"
					"<tt:SourceTag>%s</tt:SourceTag>"
	      			"<tt:Destination>%s</tt:Destination>"
      			"</tt:Tracks>",
      			p_req->Source[i].Tracks[j].SourceTag,
      			p_req->Source[i].Tracks[j].Destination);
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");
	}
	
	return offset;
}

int build_CreateRecordingJob_rly_xml(char * p_buf, int mlen, const char * argv)
{
	CreateRecordingJob_REQ * p_req = (CreateRecordingJob_REQ *) argv;

	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:CreateRecordingJobResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:JobToken>%s</trc:JobToken>", p_req->JobToken);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:JobConfiguration>");
	offset += build_RecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_req->JobConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobConfiguration>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:CreateRecordingJobResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_DeleteRecordingJob_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:DeleteRecordingJobResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetRecordingJobs_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetRecordingJobs_RES * p_res = (GetRecordingJobs_RES *)argv;
	
	ONVIF_RECORDINGJOB * p_recordingjob = p_res->recording_jobs;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingJobsResponse>");
	while (p_recordingjob)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trc:JobItem>");
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:JobToken>%s</tt:JobToken>", p_recordingjob->RecordingJob.JobToken);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:JobConfiguration>");
		offset += build_RecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_recordingjob->RecordingJob.JobConfiguration);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:JobConfiguration>");
		offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobItem>");

		p_recordingjob = p_recordingjob->next;
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:GetRecordingJobsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetRecordingJobConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	SetRecordingJobConfiguration_REQ * p_req = (SetRecordingJobConfiguration_REQ *)argv;

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetRecordingJobConfigurationResponse><trc:JobConfiguration>");
	offset += build_RecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_req->JobConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobConfiguration></trc:SetRecordingJobConfigurationResponse>");

	return offset;
}

int build_GetRecordingJobConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	
	GetRecordingJobConfiguration_RES * p_res = (GetRecordingJobConfiguration_RES *)argv;

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingJobConfigurationResponse><trc:JobConfiguration>");
	offset += build_RecordingJobConfiguration_xml(p_buf+offset, mlen-offset, &p_res->JobConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:JobConfiguration></trc:GetRecordingJobConfigurationResponse>");
	
	return offset;
}

int build_SetRecordingJobMode_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:SetRecordingJobModeResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetRecordingJobState_rly_xml(char * p_buf, int mlen, const char * argv)
{
	onvif_RecordingJobStateInformation * p_res = (onvif_RecordingJobStateInformation *) argv;

	int i, j;
	int offset = snprintf(p_buf, mlen, xml_hdr);
	
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingJobStateResponse><trc:State>");
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:RecordingToken>%s</tt:RecordingToken>" 
		"<tt:State>%s</tt:State>", 
		p_res->RecordingToken,
		p_res->State);

	for (i = 0; i < p_res->sizeSources; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Sources>");
		
		if (p_res->Sources[i].SourceToken.TypeFlag)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken Type=\"%s\">", 
				p_res->Sources[i].SourceToken.Type);
		}
		else
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SourceToken>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Token>%s</tt:Token>",
			p_res->Sources[i].SourceToken.Token);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:SourceToken>");

		offset += snprintf(p_buf+offset, mlen-offset, "<tt:State>%s</tt:State>",
			p_res->Sources[i].State);

		// tracks
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Tracks>");

		for (j = 0; j < p_res->Sources[i].sizeTrack; j++)
		{
	      	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Track>");
	      	offset += snprintf(p_buf+offset, mlen-offset, 
	      		"<tt:SourceTag>%s</tt:SourceTag>"
	      		"<tt:Destination>%s</tt:Destination>", 
	      		p_res->Sources[i].Track[j].SourceTag,
	      		p_res->Sources[i].Track[j].Destination);
	      	if (p_res->Sources[i].Track[j].ErrorFlag)
	      	{
	      		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Error>%s</tt:Error>", 
		      		p_res->Sources[i].Track[j].Error);
	      	}
      		offset += snprintf(p_buf+offset, mlen-offset, "<tt:State>%s</tt:State>", 
	      		p_res->Sources[i].Track[j].State);
	      	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Track>");
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Tracks>");
		// end of tracks
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Sources>");	
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:State></trc:GetRecordingJobStateResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetRecordingOptions_rly_xml(char * p_buf, int mlen, const char * argv)
{
	onvif_RecordingOptions * p_res = (onvif_RecordingOptions *) argv;

	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:GetRecordingOptionsResponse><trc:Options>");

	// job options
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:Job");
	if (p_res->Job.SpareFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " Spare=\"%d\" ", p_res->Job.Spare);
	}
	if (p_res->Job.CompatibleSourcesFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " CompatibleSources=\"%s\"", p_res->Job.CompatibleSources);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "></trc:Job>");

	// track options
	offset += snprintf(p_buf+offset, mlen-offset, "<trc:Track");
	if (p_res->Track.SpareTotalFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareTotal=\"%d\"", p_res->Track.SpareTotal);
	}
	if (p_res->Track.SpareVideoFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareVideo=\"%d\"", p_res->Track.SpareVideo);
	}
	if (p_res->Track.SpareAudioFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareAudio=\"%d\"", p_res->Track.SpareAudio);
	}
	if (p_res->Track.SpareMetadataFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, " SpareMetadata=\"%d\"", p_res->Track.SpareMetadata);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "></trc:Track>");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trc:Options></trc:GetRecordingOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetReplayUri_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetReplayUri_RES * p_res = (GetReplayUri_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trp:GetReplayUriResponse>"
			"<trp:Uri>%s</trp:Uri>"
		"</trp:GetReplayUriResponse>",
		p_res->Uri);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetReplayConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetReplayConfiguration_RES * p_res = (GetReplayConfiguration_RES *)argv;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<trp:GetReplayConfigurationResponse>"
			"<trp:Configuration>"
				"<tt:SessionTimeout>PT%dS</tt:SessionTimeout>"
			"</trp:Configuration>"
		"</trp:GetReplayConfigurationResponse>",
		p_res->SessionTimeout);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetReplayConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trp:SetReplayConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

#endif // end of PROFILE_G_SUPPORT

#ifdef VIDEO_ANALYTICS

int build_Config_xml(char * p_buf, int mlen, onvif_Config * p_req)
{
	int offset = 0;
	ONVIF_SIMPLEITEM * p_simpleitem;
	ONVIF_ELEMENTITEM * p_elementitem;

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");

	p_simpleitem = p_req->Parameters.SimpleItem;
	while (p_simpleitem)
	{
		offset += build_SimpleItem_xml(p_buf+offset, mlen-offset, &p_simpleitem->SimpleItem);
		
		p_simpleitem = p_simpleitem->next;
	}

	p_elementitem = p_req->Parameters.ElementItem;
	while (p_elementitem)
	{
		offset += build_ElementItem_xml(p_buf+offset, mlen-offset,&p_elementitem->ElementItem);
		p_elementitem = p_elementitem->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");
	
	return offset;
}

int build_VideoAnalyticsConfiguration_xml(char * p_buf, int mlen, onvif_VideoAnalyticsConfiguration * p_req)
{
	int offset = 0;
	ONVIF_CONFIG * p_config;
	
	offset += snprintf(p_buf+offset, mlen-offset, 
		"<tt:Name>%s</tt:Name>"
    	"<tt:UseCount>%d</tt:UseCount>",
    	p_req->Name, p_req->UseCount);

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:AnalyticsEngineConfiguration>");

	p_config = p_req->AnalyticsEngineConfiguration.AnalyticsModule;
	while (p_config)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:AnalyticsModule Name=\"%s\" Type=\"%s\">", 
			p_config->Config.Name, p_config->Config.Type);
		offset += build_Config_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsModule>");
		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsEngineConfiguration>");

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:RuleEngineConfiguration>");

	p_config = p_req->RuleEngineConfiguration.Rule;
	while (p_config)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Rule Name=\"%s\" Type=\"%s\">", 
			p_config->Config.Name, p_config->Config.Type);
		offset += build_Config_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Rule>");
		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tt:RuleEngineConfiguration>");
	
	return offset;
}

int build_GetVideoAnalyticsConfigurations_rly_xml(char * p_buf, int mlen, const char * argv)
{
	Get_VA_Configurations_RES * p_res = (Get_VA_Configurations_RES *)argv;
	ONVIF_VACFG * p_va_cfg = p_res->p_va_cfg;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoAnalyticsConfigurationsResponse>");

	while (p_va_cfg)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", p_va_cfg->VideoAnalyticsConfiguration.token);
		offset += build_VideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_va_cfg->VideoAnalyticsConfiguration);
		offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");

		p_va_cfg = p_va_cfg->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoAnalyticsConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_AddVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoAnalyticsConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = 0;
	Get_VA_Configuration_RES * p_res = (Get_VA_Configuration_RES *)argv;

	offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoAnalyticsConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", p_res->struva_cfg.VideoAnalyticsConfiguration.token);
	offset += build_VideoAnalyticsConfiguration_xml(p_buf+offset, mlen-offset, &p_res->struva_cfg.VideoAnalyticsConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoAnalyticsConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_RemoveVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoAnalyticsConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_SetVideoAnalyticsConfiguration_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoAnalyticsConfigurationResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_ItemListDescription_xml(char * p_buf, int mlen, onvif_ItemListDescription * p_req)
{
	int offset = 0;
	ONVIF_SIMPLE_ITEM_DESC * p_simpleitem_desc;
	
	p_simpleitem_desc = p_req->SimpleItemDescription;
	while (p_simpleitem_desc)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Name=\"%s\" Type=\"%s\" />",
			p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
		p_simpleitem_desc = p_simpleitem_desc->next;
	}

	p_simpleitem_desc = p_req->ElementItemDescription;
	while (p_simpleitem_desc)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItemDescription Name=\"%s\" Type=\"%s\" />",
			p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
		p_simpleitem_desc = p_simpleitem_desc->next;
	}

	return offset;
}

int build_ConfigDescription_Messages_xml(char * p_buf, int mlen, onvif_ConfigDescription_Messages * p_req)
{
	int offset = 0;

	if (p_req->SourceFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
		build_ItemListDescription_xml(p_buf+offset, mlen-offset, &p_req->Source);		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");
	}

	if (p_req->KeyFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Key>");
		build_ItemListDescription_xml(p_buf+offset, mlen-offset, &p_req->Source);		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Key>");
	}

	if (p_req->DataFlag)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Data>");
		build_ItemListDescription_xml(p_buf+offset, mlen-offset, &p_req->Source);		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Data>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:ParentTopic>%s</tt:ParentTopic>", p_req->ParentTopic);
	
	return offset;
}

int build_GetSupportedRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int i;
	GetSupportedRules_RES * p_res = (GetSupportedRules_RES *)argv;
	ONVIF_CFG_DESC * p_cfg_desc;
	ONVIF_SIMPLE_ITEM_DESC * p_simpleitem_desc;
	ONVIF_CFG_DESC_MSG * p_cfg_desc_msg;
	
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetSupportedRulesResponse><tan:SupportedRules>");

	for (i = 0; i < p_res->SupportedRules.sizeRuleContentSchemaLocation; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
			"<tt:RuleContentSchemaLocation>%s</tt:RuleContentSchemaLocation>",
			p_res->SupportedRules.RuleContentSchemaLocation[i]);
	}

	p_cfg_desc = p_res->SupportedRules.RuleDescription;
	while (p_cfg_desc)
	{		
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RuleDescription Name=\"%s\">", p_cfg_desc->ConfigDescription.Name);

		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");

		p_simpleitem_desc = p_cfg_desc->ConfigDescription.Parameters.SimpleItemDescription;
		while (p_simpleitem_desc)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Name=\"%s\" Type=\"%s\" />", 
				p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
			p_simpleitem_desc = p_simpleitem_desc->next;
		}

		p_simpleitem_desc = p_cfg_desc->ConfigDescription.Parameters.ElementItemDescription;
		while (p_simpleitem_desc)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItemDescription Name=\"%s\" Type=\"%s\" />", 
				p_simpleitem_desc->SimpleItemDescription.Name, p_simpleitem_desc->SimpleItemDescription.Type);
			p_simpleitem_desc = p_simpleitem_desc->next;
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");

		p_cfg_desc_msg = p_cfg_desc->ConfigDescription.Messages;
		while (p_cfg_desc_msg)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Messages IsProperty=\"%s\">", p_cfg_desc_msg->Messages.IsProperty ? "true" : "false");
			offset += build_ConfigDescription_Messages_xml(p_buf+offset, mlen-offset, &p_cfg_desc_msg->Messages);			
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Messages>");
			
			p_cfg_desc_msg = p_cfg_desc_msg->next;
		}
		
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:RuleDescription>");
		
		p_cfg_desc = p_cfg_desc->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:SupportedRules></tan:GetSupportedRulesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreateRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:CreateRulesResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_DeleteRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:DeleteRulesResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetRules_RES * p_res = (GetRules_RES *)argv;
	ONVIF_CONFIG * p_config;
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetRulesResponse>");

	p_config = p_res->Rule;
	while (p_config)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tan:Rule Name=\"%s\" Type=\"%s\">", p_config->Config.Name, p_config->Config.Type);
		offset += build_Config_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:Rule>");
		
		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetRulesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_ModifyRules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:ModifyRulesResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_CreateAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:CreateAnalyticsModulesResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_DeleteAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:DeleteAnalyticsModulesResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_GetAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	GetAnalyticsModules_RES * p_res = (GetAnalyticsModules_RES *)argv;
	ONVIF_CONFIG * p_config;
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetAnalyticsModulesResponse>");

	p_config = p_res->AnalyticsModule;
	while (p_config)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tan:AnalyticsModule Name=\"%s\" Type=\"%s\">", p_config->Config.Name, p_config->Config.Type);
		offset += build_Config_xml(p_buf+offset, mlen-offset, &p_config->Config);
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:AnalyticsModule>");
		
		p_config = p_config->next;
	}
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetAnalyticsModulesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

int build_ModifyAnalyticsModules_rly_xml(char * p_buf, int mlen, const char * argv)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:ModifyAnalyticsModulesResponse />");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

#endif


