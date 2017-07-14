
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
#include "onvif/onvif/onvif_cm.h"


const char * onvif_CapabilityCategoryToString(onvif_CapabilityCategory category)
{
	switch (category)
	{
	case CapabilityCategory_All:
		return "All";
		
	case CapabilityCategory_Analytics:
		return "Analytics";

	case CapabilityCategory_Device:
		return "Device";

	case CapabilityCategory_Events:
		return "Events";

	case CapabilityCategory_Imaging:
		return "Imaging";

	case CapabilityCategory_Media:
		return "Media";

	case CapabilityCategory_PTZ:
		return "PTZ";	
	}

	return "All";
}

onvif_CapabilityCategory onvif_StringToCapabilityCategory(const char * str)
{
	if (strcasecmp(str, "All") == 0)
	{
		return CapabilityCategory_All;
	}
	else if (strcasecmp(str, "Analytics") == 0)
	{
		return CapabilityCategory_Analytics;
	}
	else if (strcasecmp(str, "Device") == 0)
	{
		return CapabilityCategory_Device;
	}
	else if (strcasecmp(str, "Events") == 0)
	{
		return CapabilityCategory_Events;
	}
	else if (strcasecmp(str, "Imaging") == 0)
	{
		return CapabilityCategory_Imaging;
	}
	else if (strcasecmp(str, "Media") == 0)
	{
		return CapabilityCategory_Media;
	}
	else if (strcasecmp(str, "PTZ") == 0)
	{
		return CapabilityCategory_PTZ;
	}

	return CapabilityCategory_Invalid;
}

const char * onvif_VideoEncodingToString(onvif_VideoEncoding encoding)
{
	switch (encoding)
	{
	case VideoEncoding_JPEG:
		return "JPEG";
		
	case VideoEncoding_MPEG4:
		return "MPEG4";

	case VideoEncoding_H264:
		return "H264";
	}

	return "H264";
}

onvif_VideoEncoding onvif_StringToVideoEncoding(const char * str)
{
	if (strcasecmp(str, "JPEG") == 0)
	{
		return VideoEncoding_JPEG;
	}
	else if (strcasecmp(str, "MPEG4") == 0)
	{
		return VideoEncoding_MPEG4;
	}
	else if (strcasecmp(str, "H264") == 0)
	{
		return VideoEncoding_H264;
	}

	return VideoEncoding_H264;
}

const char * onvif_AudioEncodingToString(onvif_AudioEncoding encoding)
{
	switch (encoding)
	{
	case AudioEncoding_G711:
		return "G711";
		
	case AudioEncoding_G726:
		return "G726";

	case AudioEncoding_AAC:
		return "AAC";
	}

	return "G711";
}

onvif_AudioEncoding onvif_StringToAudioEncoding(const char * str)
{
	if (strcasecmp(str, "G711") == 0)
	{
		return AudioEncoding_G711;
	}
	else if (strcasecmp(str, "G726") == 0)
	{
		return AudioEncoding_G726;
	}
	else if (strcasecmp(str, "AAC") == 0)
	{
		return AudioEncoding_AAC;
	}

	return AudioEncoding_G711;
}

const char * onvif_H264ProfileToString(onvif_H264Profile profile)
{
	switch (profile)
	{
	case H264Profile_Baseline:
		return "Baseline";
		
	case H264Profile_Main:
		return "Main";

	case H264Profile_Extended:
		return "Extended";

	case H264Profile_High:
		return "High";
	}

	return "Baseline";
}

onvif_H264Profile onvif_StringToH264Profile(const char * str)
{
	if (strcasecmp(str, "Baseline") == 0)
	{
		return H264Profile_Baseline;
	}
	else if (strcasecmp(str, "Main") == 0)
	{
		return H264Profile_Main;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return H264Profile_Extended;
	}
	else if (strcasecmp(str, "High") == 0)
	{
		return H264Profile_High;
	}

	return H264Profile_Baseline;
}

const char * onvif_Mpeg4ProfileToString(onvif_Mpeg4Profile profile)
{
	switch (profile)
	{
	case Mpeg4Profile_SP:
		return "SP";
		
	case Mpeg4Profile_ASP:
		return "ASP";
	}

	return "SP";
}

onvif_Mpeg4Profile onvif_StringToMpeg4Profile(const char * str)
{
	if (strcasecmp(str, "SP") == 0)
	{
		return Mpeg4Profile_SP;
	}
	else if (strcasecmp(str, "ASP") == 0)
	{
		return Mpeg4Profile_ASP;
	}

	return Mpeg4Profile_SP;
}

const char * onvif_UserLevelToString(onvif_UserLevel level)
{
	switch (level)
	{
	case UserLevel_Administrator:
		return "Administrator";
		
	case UserLevel_Operator:
		return "Operator";

	case UserLevel_User:
		return "User";

	case UserLevel_Anonymous:
		return "Anonymous";

	case UserLevel_Extended:
		return "Extended";	
	}

	return "User";
}

onvif_UserLevel onvif_StringToUserLevel(const char * str)
{
	if (strcasecmp(str, "Administrator") == 0)
	{
		return UserLevel_Administrator;
	}
	else if (strcasecmp(str, "Operator") == 0)
	{
		return UserLevel_Operator;
	}
	else if (strcasecmp(str, "User") == 0)
	{
		return UserLevel_User;
	}
	else if (strcasecmp(str, "Anonymous") == 0)
	{
		return UserLevel_Anonymous;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return UserLevel_Extended;
	}

	return UserLevel_User;
}

const char * onvif_MoveStatusToString(onvif_MoveStatus status)
{
	switch (status)
	{
	case MoveStatus_IDLE:
		return "IDLE";
		
	case MoveStatus_MOVING:
		return "MOVING";

	case MoveStatus_UNKNOWN:
		return "UNKNOWN";
	}

	return "IDLE";
}

onvif_MoveStatus onvif_StringToMoveStatus(const char * str)
{
	if (strcasecmp(str, "IDLE") == 0)
	{
		return MoveStatus_IDLE;
	}
	else if (strcasecmp(str, "MOVING") == 0)
	{
		return MoveStatus_MOVING;
	}
	else if (strcasecmp(str, "UNKNOWN") == 0)
	{
		return MoveStatus_UNKNOWN;
	}

	return MoveStatus_IDLE;
}

const char * onvif_OSDTypeToString(onvif_OSDType type)
{
	switch (type)
	{
	case OSDType_Text:
		return "Text";
		
	case OSDType_Image:
		return "Image";

	case OSDType_Extended:
		return "Extended";
	}

	return "Text";
}

onvif_OSDType onvif_StringToOSDType(const char * type)
{
	if (strcasecmp(type, "Text") == 0)
	{
		return OSDType_Text;
	}
	else if (strcasecmp(type, "Image") == 0)
	{
		return OSDType_Image;
	}
	else if (strcasecmp(type, "Extended") == 0)
	{
		return OSDType_Extended;
	}

	return OSDType_Text;
}

const char * onvif_OSDPosTypeToString(onvif_OSDPosType type)
{
	switch (type)
	{
	case OSDPosType_UpperLeft:
		return "UpperLeft";
		
	case OSDPosType_UpperRight:
		return "UpperRight";
		
	case OSDPosType_LowerLeft:
		return "LowerLeft";
		
	case OSDPosType_LowerRight:
		return "LowerRight";
		
	case OSDPosType_Custom:	
		return "Custom";
	}

	return "UpperLeft";
}

onvif_OSDPosType onvif_StringToOSDPosType(const char * type)
{	
	if (strcasecmp(type, "UpperLeft") == 0)
	{
		return OSDPosType_UpperLeft;
	}
	else if (strcasecmp(type, "UpperRight") == 0)
	{
		return OSDPosType_UpperRight;
	}
	else if (strcasecmp(type, "LowerLeft") == 0)
	{
		return OSDPosType_LowerLeft;
	}
	else if (strcasecmp(type, "LowerRight") == 0)
	{
		return OSDPosType_LowerRight;
	}
	else if (strcasecmp(type, "Custom") == 0)
	{
		return OSDPosType_Custom;
	}

	return OSDPosType_UpperLeft;
}

const char * onvif_OSDTextTypeToString(onvif_OSDTextType type)
{
	switch (type)
	{
	case OSDTextType_Plain:
		return "Plain";
		
	case OSDTextType_Date:
		return "Date";
		
	case OSDTextType_Time:
		return "Time";
		
	case OSDTextType_DateAndTime:
		return "DateAndTime";
	}

	return "Plain";
}

onvif_OSDTextType	onvif_StringToOSDTextType(const char * type)
{
	if (strcasecmp(type, "Plain") == 0)
	{
		return OSDTextType_Plain;
	}
	else if (strcasecmp(type, "Date") == 0)
	{
		return OSDTextType_Date;
	}
	else if (strcasecmp(type, "Time") == 0)
	{
		return OSDTextType_Time;
	}
	else if (strcasecmp(type, "DateAndTime") == 0)
	{
		return OSDTextType_DateAndTime;
	}

	return OSDTextType_Plain;
}

const char * onvif_BacklightCompensationModeToString(onvif_BacklightCompensationMode mode)
{
	switch (mode)
	{
	case BacklightCompensationMode_OFF:
		return "OFF";
		
	case BacklightCompensationMode_ON:
		return "ON";
	}

	return "OFF";
}

onvif_BacklightCompensationMode onvif_StringToBacklightCompensationMode(const char * str)
{
	if (strcasecmp(str, "OFF") == 0)
	{
		return BacklightCompensationMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{

		return BacklightCompensationMode_ON;
	}

	return BacklightCompensationMode_OFF;
}

const char * onvif_ExposureModeToString(onvif_ExposureMode mode)
{
	switch (mode)
	{
	case ExposureMode_AUTO:
		return "AUTO";

	case ExposureMode_MANUAL:
		return "MANUAL";	
	}

	return "AUTO";
}

onvif_ExposureMode onvif_StringToExposureMode(const char * str)
{
	if (strcasecmp(str, "AUTO") == 0)
	{

		return ExposureMode_AUTO;
	}
	else if (strcasecmp(str, "MANUAL") == 0)
	{
		return ExposureMode_MANUAL;
	}

	return ExposureMode_AUTO;
}

const char * onvif_ExposurePriorityToString(onvif_ExposurePriority mode)
{
	switch (mode)
	{
	case ExposurePriority_LowNoise:
		return "LowNoise";

	case ExposurePriority_FrameRate:
		return "FrameRate";	
	}

	return "LowNoise";
}

onvif_ExposurePriority onvif_StringToExposurePriority(const char * str)
{
	if (strcasecmp(str, "LowNoise") == 0)
	{

		return ExposurePriority_LowNoise;
	}
	else if (strcasecmp(str, "FrameRate") == 0)
	{
		return ExposurePriority_FrameRate;
	}

	return ExposurePriority_LowNoise;
}

const char * onvif_AutoFocusModeToString(onvif_AutoFocusMode mode)
{
	switch (mode)
	{
	case AutoFocusMode_AUTO:
		return "AUTO";

	case AutoFocusMode_MANUAL:
		return "MANUAL";	
	}

	return "AUTO";
}

onvif_AutoFocusMode onvif_StringToAutoFocusMode(const char * str)
{
	if (strcasecmp(str, "AUTO") == 0)
	{

		return AutoFocusMode_AUTO;
	}
	else if (strcasecmp(str, "MANUAL") == 0)
	{
		return AutoFocusMode_MANUAL;
	}

	return AutoFocusMode_AUTO;
}

const char * onvif_WideDynamicModeToString(onvif_WideDynamicMode mode)
{
	switch (mode)
	{
	case WideDynamicMode_OFF:
		return "OFF";

	case WideDynamicMode_ON:
		return "ON";	
	}

	return "OFF";
}

onvif_WideDynamicMode onvif_StringToWideDynamicMode(const char * str)
{
	if (strcasecmp(str, "OFF") == 0)
	{

		return WideDynamicMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{
		return WideDynamicMode_ON;
	}

	return WideDynamicMode_OFF;
}

const char * onvif_IrCutFilterModeToString(onvif_IrCutFilterMode mode)
{
	switch (mode)
	{
	case IrCutFilterMode_ON:
		return "ON";

	case IrCutFilterMode_OFF:
		return "OFF";	

	case IrCutFilterMode_AUTO:
		return "AUTO";
	}

	return "ON";
}

onvif_IrCutFilterMode onvif_StringToIrCutFilterMode(const char * str)
{
	if (strcasecmp(str, "ON") == 0)
	{
		return IrCutFilterMode_ON;
	}
	else if (strcasecmp(str, "OFF") == 0)
	{
		return IrCutFilterMode_OFF;
	}
	else if (strcasecmp(str, "AUTO") == 0)
	{
		return IrCutFilterMode_AUTO;
	}

	return IrCutFilterMode_ON;
}

const char * onvif_WhiteBalanceModeToString(onvif_WhiteBalanceMode mode)
{
	switch (mode)
	{
	case WhiteBalanceMode_AUTO:
		return "AUTO";

	case WhiteBalanceMode_MANUAL:
		return "MANUAL";	
	}

	return "AUTO";
}

onvif_WhiteBalanceMode onvif_StringToWhiteBalanceMode(const char * str)
{
	if (strcasecmp(str, "AUTO") == 0)
	{
		return WhiteBalanceMode_AUTO;
	}
	else if (strcasecmp(str, "MANUAL") == 0)
	{
		return WhiteBalanceMode_MANUAL;
	}

	return WhiteBalanceMode_AUTO;
}

const char * onvif_EFlipModeToString(onvif_EFlipMode mode)
{
    switch (mode)
	{
	case EFlipMode_OFF:
		return "OFF";

	case EFlipMode_ON:
		return "ON";

	case EFlipMode_Extended:
		return "Extended";	
	}

	return "OFF";
}

onvif_EFlipMode onvif_StringToEFlipMode(const char * str)
{
    if (strcasecmp(str, "OFF") == 0)
	{
		return EFlipMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{
		return EFlipMode_ON;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return EFlipMode_Extended;
	}

	return EFlipMode_OFF;
}

const char * onvif_ReverseModeToString(onvif_ReverseMode mode)
{
    switch (mode)
	{
	case ReverseMode_OFF:
		return "OFF";

	case ReverseMode_ON:
		return "ON";

    case ReverseMode_AUTO:
        return "AUTO";
        
	case ReverseMode_Extended:
		return "Extended";	
	}

	return "OFF";
}

onvif_ReverseMode onvif_StringToReverseMode(const char * str)
{
    if (strcasecmp(str, "OFF") == 0)
	{
		return ReverseMode_OFF;
	}
	else if (strcasecmp(str, "ON") == 0)
	{
		return ReverseMode_ON;
	}
	else if (strcasecmp(str, "AUTO") == 0)
	{
		return ReverseMode_AUTO;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return ReverseMode_Extended;
	}

	return ReverseMode_OFF;
}

const char * onvif_DiscoveryModeToString(onvif_DiscoveryMode mode)
{
	switch (mode)
	{
	case DiscoveryMode_Discoverable:
		return "Discoverable";

	case DiscoveryMode_NonDiscoverable:
		return "NonDiscoverable";
	}

	return "Discoverable";
}

onvif_DiscoveryMode	onvif_StringToDiscoveryMode(const char * str)
{
	if (strcasecmp(str, "Discoverable") == 0)
	{
		return DiscoveryMode_Discoverable;
	}
	else if (strcasecmp(str, "NonDiscoverable") == 0)
	{
		return DiscoveryMode_NonDiscoverable;
	}

	return DiscoveryMode_Discoverable;
}

const char * onvif_SetDateTimeTypeToString(onvif_SetDateTimeType type)
{
	switch (type)
	{
	case SetDateTimeType_Manual:
		return "Manual";

	case SetDateTimeType_NTP:
		return "NTP";
	}

	return "Manual";
}

onvif_SetDateTimeType onvif_StringToSetDateTimeType(const char * str)
{
	if (strcasecmp(str, "Manual") == 0)
	{
		return SetDateTimeType_Manual;
	}
	else if (strcasecmp(str, "NTP") == 0)
	{
		return SetDateTimeType_NTP;
	}

	return SetDateTimeType_Manual;
}

const char * onvif_StreamTypeToString(onvif_StreamType type)
{
	switch (type)
	{
	case StreamType_RTP_Unicast:
		return "RTP-Unicast";

	case StreamType_RTP_Multicast:
		return "RTP-Multicast";
	}

	return "RTP_Unicast";
}

onvif_StreamType onvif_StringToStreamType(const char * str)
{
	if (strcasecmp(str, "RTP-Unicast") == 0)
	{
		return StreamType_RTP_Unicast;
	}
	else if (strcasecmp(str, "RTP-Multicast") == 0)
	{
		return StreamType_RTP_Multicast;
	}

	return StreamType_Invalid;
}

const char * onvif_TransportProtocolToString(onvif_TransportProtocol type)
{
	switch (type)
	{
	case TransportProtocol_UDP:
		return "UDP";

	case TransportProtocol_TCP:
		return "TCP";

	case TransportProtocol_RTSP:
		return "RTSP";

	case TransportProtocol_HTTP:
		return "HTTP";	
	}

	return "UDP";
}

onvif_TransportProtocol onvif_StringToTransportProtocol(const char * str)
{
	if (strcasecmp(str, "UDP") == 0)
	{
		return TransportProtocol_UDP;
	}
	else if (strcasecmp(str, "TCP") == 0)
	{
		return TransportProtocol_TCP;
	}
	else if (strcasecmp(str, "RTSP") == 0)
	{
		return TransportProtocol_RTSP;
	}
	else if (strcasecmp(str, "HTTP") == 0)
	{
		return TransportProtocol_HTTP;
	}

	return TransportProtocol_Invalid;
}

const char * onvif_TrackTypeToString(onvif_TrackType type)
{
	switch (type)
	{
	case TrackType_Video:
		return "Video";

	case TrackType_Audio:
		return "Audio";

	case TrackType_Metadata:
		return "Metadata";

	case TrackType_Extended:
		return "Extended";	
	}

	return "Video";
}

onvif_TrackType	onvif_StringToTrackType(const char * str)
{
	if (strcasecmp(str, "Video") == 0)
	{
		return TrackType_Video;
	}
	else if (strcasecmp(str, "Audio") == 0)
	{
		return TrackType_Audio;
	}
	else if (strcasecmp(str, "Metadata") == 0)
	{
		return TrackType_Metadata;
	}
	else if (strcasecmp(str, "Extended") == 0)
	{
		return TrackType_Extended;
	}

	return TrackType_Invalid;
}

const char * onvif_PropertyOperationToString(onvif_PropertyOperation type)
{
	switch (type)
	{
	case PropertyOperation_Initialized:
		return "Initialized";

	case PropertyOperation_Deleted:
		return "Deleted";

	case PropertyOperation_Changed:
		return "Changed";	
	}

	return "Initialized";
}

onvif_PropertyOperation	onvif_StringToPropertyOperation(const char * str)
{
	if (strcasecmp(str, "Initialized") == 0)
	{
		return PropertyOperation_Initialized;
	}
	else if (strcasecmp(str, "Deleted") == 0)
	{
		return PropertyOperation_Deleted;
	}
	else if (strcasecmp(str, "Changed") == 0)
	{
		return PropertyOperation_Changed;
	}

	return PropertyOperation_Invalid;
}

const char * onvif_RecordingStatusToString(onvif_RecordingStatus status)
{
	switch (status)
	{
	case RecordingStatus_Initiated:
		return "Initialized";

	case RecordingStatus_Recording:
		return "Recording";

	case RecordingStatus_Stopped:
		return "Stopped";	

	case RecordingStatus_Removing:
		return "Removing";

	case RecordingStatus_Removed:
		return "Removed";

	case RecordingStatus_Unknown:
		return "Unknown";	
	}

	return "Initialized";
}

onvif_RecordingStatus onvif_StringToRecordingStatus(const char * str)
{
	if (strcasecmp(str, "Initialized") == 0)
	{
		return RecordingStatus_Initiated;
	}
	else if (strcasecmp(str, "Recording") == 0)
	{
		return RecordingStatus_Recording;
	}
	else if (strcasecmp(str, "Stopped") == 0)
	{
		return RecordingStatus_Stopped;
	}
	else if (strcasecmp(str, "Removing") == 0)
	{
		return RecordingStatus_Removing;
	}
	else if (strcasecmp(str, "Removed") == 0)
	{
		return RecordingStatus_Removed;
	}
	else if (strcasecmp(str, "Unknown") == 0)
	{
		return RecordingStatus_Unknown;
	}

	return RecordingStatus_Unknown;
}

const char * onvif_SearchStateToString(onvif_SearchState state)
{
	switch (state)
	{
	case SearchState_Queued:
		return "Queued";

	case SearchState_Searching:
		return "Searching";

	case SearchState_Completed:
		return "Completed";	

	case SearchState_Unknown:
		return "Unknown";
	}

	return "Queued";
}

onvif_SearchState onvif_StringToSearchState(const char * str)
{
	if (strcasecmp(str, "Queued") == 0)
	{
		return SearchState_Queued;
	}
	else if (strcasecmp(str, "Searching") == 0)
	{
		return SearchState_Searching;
	}
	else if (strcasecmp(str, "Completed") == 0)
	{
		return SearchState_Completed;
	}
	else if (strcasecmp(str, "Unknown") == 0)
	{
		return SearchState_Unknown;
	}

	return SearchState_Unknown;
}





