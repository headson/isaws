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
#include "onvif/onvif/onvif_device.h"
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif_event.h"
#include "onvif/onvif/onvif_util.h"
#include "onvif/onvif/onvif_cfg.h"


/***************************************************************************************/
ONVIF_CLS g_onvif_cls;


/***************************************************************************************/

typedef struct {
  ONVIF_ELEMENTITEM node;
  int flag;
}LocalElementItem;

typedef struct {
  ONVIF_CONFIG node;
  int flag;
}LocalConfig;

typedef struct {
  ONVIF_SIMPLEITEM node;
  int flag;
}LocalSimpleItem;


typedef struct {
  ONVIF_NOTIFYMESSAGE node;
  int flag;
}LocalNoteMsg;

typedef struct {
  ONVIF_TRACK node;
  int flag;
}LocalTrack;

typedef struct {
  ONVIF_PROFILE node;
  int flag;
}LocalProfile;

typedef struct {
  ONVIF_V_SRC_CFG node;
  int flag;
}LocalVideoSRC;

typedef struct {
  ONVIF_V_ENC_CFG node;
  int flag;
}LocalVideoENC;

typedef struct {
  ONVIF_A_SRC_CFG node;
  int flag;
}LocalAudioSRC;

typedef struct {
  ONVIF_A_ENC_CFG node;
  int flag;
}LocalAudioENC;

typedef struct {
  PTZ_CFG node;
  int flag;
}LocalPTZ;

typedef struct {
  ONVIF_METADATA_CFG node;
  int flag;
}LocalMetadata;

typedef struct {
  ONVIF_VACFG node;
  int flag;
}LocalVACFG;

typedef struct {
  ONVIF_NET_INF node;
  int flag;
}LocalNetINF;

typedef struct {
  ONVIF_OSD node;
  int flag;
}LocalOSD;

typedef struct {
  ONVIF_CFG_DESC node;
  int flag;
}LocalCfgDesc;

typedef struct {
  ONVIF_SIMPLE_ITEM_DESC node;
  int flag;
}LocalSItemDesc;

typedef struct {
  ONVIF_CFG_DESC_MSG node;
  int flag;
}LocalCfgDescMsg;

typedef struct {
  PTZ_NODE node;
  int flag;
}LocalPTZNode;

typedef struct {
  ONVIF_RECINF node;
  int flag;
}LocalRecInf;

typedef struct {
  ONVIF_RECORDING node;
  int flag;
}LocalRecord;

typedef struct {
  ONVIF_RECORDINGJOB node;
  int flag;
}LocalRecordJob;

typedef struct {
  LINKED_NODE node;
  int flag;
}LocalLinkNode;

typedef struct {
  LINKED_LIST node;
  int flag;
}LocalLinkList;



#define EITEM_MAX  8
#define SITEM_MAX  8
#define CONFIG_MAX  8
#define NOTEMSG_MAX  8
#define PROFILE_MAX  2
#define V_SRC_MAX  4
#define V_ENC_MAX  4
#define A_SRC_MAX  4
#define A_ENC_MAX  4
#define PTZCFG_MAX  2
#define METADATA_MAX  4
#define VACFG_MAX  4
#define NET_INF_MAX  2
#define OSD_MAX  8
#define CFG_DESC_MAX  1
#define SITEM_DESC_MAX  1
#define CFG_DESC_MSG_MAX  1
#define PTZ_NODE_MAX  2
#define RECINF_MAX  1
#define RECORD_MAX  1
#define TRACK_MAX  1
#define RECORDINGJOB_MAX  1
#define LINKED_NODE_MAX  8
#define LINKED_LIST_MAX  1



static LocalElementItem g_eitems[EITEM_MAX]={0};
static LocalSimpleItem g_sitems[SITEM_MAX]={0};
static LocalConfig g_configs[CONFIG_MAX]={0};
static LocalNoteMsg g_notemsgs[NOTEMSG_MAX]={0};
static LocalTrack g_tracks[TRACK_MAX]={0};
static LocalProfile g_profiles[PROFILE_MAX]={0};
static LocalVideoSRC g_v_srcs[V_SRC_MAX]={0};
static LocalVideoENC g_v_encs[V_ENC_MAX]={0};
static LocalAudioSRC g_a_srcs[A_SRC_MAX]={0};
static LocalAudioENC g_a_encs[A_ENC_MAX]={0};
static LocalPTZ g_ptzs[PTZCFG_MAX]={0};
static LocalMetadata g_metadatas[METADATA_MAX]={0};
static LocalVACFG g_vacfgs[VACFG_MAX]={0};
static LocalNetINF g_netinfs[NET_INF_MAX]={0};
static LocalOSD g_osds[OSD_MAX]={0};
static LocalCfgDesc g_cfg_descs[CFG_DESC_MAX]={0};
static LocalSItemDesc g_sitem_descs[SITEM_DESC_MAX]={0};
static LocalCfgDescMsg g_cfgdesc_msgs[CFG_DESC_MSG_MAX]={0};
static LocalPTZNode g_ptz_nodes[PTZ_NODE_MAX]={0};
static LocalRecInf g_recinfs[RECINF_MAX]={0};
static LocalRecord g_records[RECORD_MAX]={0};
static LocalRecordJob g_recordjobs[RECORDINGJOB_MAX]={0};
static LocalLinkNode g_linknodes[LINKED_NODE_MAX]={0};
static LocalLinkList g_linklists[LINKED_LIST_MAX]={0};
static char test_buff[1024*1024*10]={0};



#define get_node(item, max_item)\
do { \
  int i = -1;\
  for (i = 0; i < max_item; i++) {\
    if (item[i].flag == 0) {\
      item[i].flag = 1;\
      return &item[i].node;\
    }\
  }\
}while(0)


#define free_node(items, max_item, item, unit_size)\
do { \
  unsigned int index = (((unsigned int)item - (unsigned int)&items[0].node)/unit_size);\
  if(index >= max_item) {\
   printf("index[%u] error!!!!!!!!\n",index);\
  }\
  items[index].flag = 0;\
}while(0)

void * get_one_item(ONVIF_MEM_TYPE type) {
  //printf("get_one_item type[%d]!!!!!!!!\n",type);
  switch(type){
  case ONVIF_TYPE_ELEMENTITEM:
    get_node(g_eitems,EITEM_MAX);
    break;
  case ONVIF_TYPE_SIMPLEITEM:
    get_node(g_sitems,SITEM_MAX);
    break;
  case ONVIF_TYPE_CONFIG:
    get_node(g_configs,CONFIG_MAX);
    break;
  case ONVIF_TYPE_NOTIFYMESSAGE:
    get_node(g_notemsgs,NOTEMSG_MAX);
    break;
  case ONVIF_TYPE_TRACK:
    get_node(g_tracks,TRACK_MAX);
    break;
  case ONVIF_TYPE_PROFILE:
    get_node(g_profiles,PROFILE_MAX);
    break;
  case ONVIF_TYPE_V_SRC:
    get_node(g_v_srcs,V_SRC_MAX);
    break;
  case ONVIF_TYPE_V_ENC:
    get_node(g_v_encs,V_ENC_MAX);
    break;
  case ONVIF_TYPE_A_SRC:
    get_node(g_a_srcs,A_SRC_MAX);
    break;
  case ONVIF_TYPE_A_ENC:
    get_node(g_a_encs,A_ENC_MAX);
    break;
  case ONVIF_TYPE_PTZ:
    get_node(g_ptzs,PTZCFG_MAX);
    break;
  case ONVIF_TYPE_METADATA:
    get_node(g_metadatas,METADATA_MAX);
    break;
  case ONVIF_TYPE_VACFG:
    get_node(g_vacfgs,VACFG_MAX);
    break;
  case ONVIF_TYPE_NET_INF:
    get_node(g_netinfs,NET_INF_MAX);
    break;
  case ONVIF_TYPE_OSD:
    get_node(g_osds,OSD_MAX);
    break;
  case ONVIF_TYPE_CFG_DESC:
    get_node(g_cfg_descs,CFG_DESC_MAX);
    break;
  case ONVIF_TYPE_SITEM_DESC:
    get_node(g_sitem_descs,SITEM_DESC_MAX);
    break;
  case ONVIF_TYPE_CFGDESC_MSG:
    get_node(g_cfgdesc_msgs,CFG_DESC_MSG_MAX);
    break;
  case ONVIF_TYPE_PTZ_NODE:
    get_node(g_ptz_nodes,PTZ_NODE_MAX);
    break;
  case ONVIF_TYPE_RECINF:
    get_node(g_recinfs,RECINF_MAX);
    break;
  case ONVIF_TYPE_RECORDING:
    get_node(g_records,RECORD_MAX);
    break;
  case ONVIF_TYPE_RECORDINGJOB:
    get_node(g_recordjobs,RECORDINGJOB_MAX);
    break;
  case ONVIF_TYPE_LINK_NODE:
    get_node(g_linknodes,LINKED_NODE_MAX);
    break;
  case ONVIF_TYPE_LINK_LIST:
    get_node(g_linklists,LINKED_LIST_MAX);
    break;
  default:
    return NULL;
  }
  return NULL;
}

void free_one_item(void *node, ONVIF_MEM_TYPE type) {
  //printf("free_one_item type[%d]!!!!!!!!\n",type);
  switch(type){
  case ONVIF_TYPE_ELEMENTITEM:
    free_node(g_eitems,EITEM_MAX,node,sizeof(LocalElementItem));
    break;
  case ONVIF_TYPE_SIMPLEITEM:
    free_node(g_sitems,SITEM_MAX,node,sizeof(LocalSimpleItem));
    break;
  case ONVIF_TYPE_CONFIG:
    free_node(g_configs,CONFIG_MAX,node,sizeof(LocalConfig));
    break;
  case ONVIF_TYPE_NOTIFYMESSAGE:
    free_node(g_notemsgs,NOTEMSG_MAX,node,sizeof(LocalNoteMsg));
    break;
  case ONVIF_TYPE_TRACK:
    free_node(g_tracks,TRACK_MAX,node,sizeof(LocalTrack));
    break;
  case ONVIF_TYPE_PROFILE:
    free_node(g_profiles,PROFILE_MAX,node,sizeof(ONVIF_PROFILE));
    break;
  case ONVIF_TYPE_V_SRC:
    free_node(g_v_srcs,V_SRC_MAX,node,sizeof(ONVIF_V_SRC_CFG));
    break;
  case ONVIF_TYPE_V_ENC:
    free_node(g_v_encs,V_ENC_MAX,node,sizeof(ONVIF_V_ENC_CFG));
    break;
  case ONVIF_TYPE_A_SRC:
    free_node(g_a_srcs,A_SRC_MAX,node,sizeof(ONVIF_A_SRC_CFG));
    break;
  case ONVIF_TYPE_A_ENC:
    free_node(g_a_encs,A_ENC_MAX,node,sizeof(ONVIF_A_ENC_CFG));
    break;
  case ONVIF_TYPE_PTZ:
    free_node(g_ptzs,PTZCFG_MAX,node,sizeof(PTZ_CFG));
    break;
  case ONVIF_TYPE_METADATA:
    free_node(g_metadatas,METADATA_MAX,node,sizeof(ONVIF_METADATA_CFG));
    break;
  case ONVIF_TYPE_VACFG:
    free_node(g_vacfgs,VACFG_MAX,node,sizeof(ONVIF_VACFG));
    break;
  case ONVIF_TYPE_NET_INF:
    free_node(g_netinfs,NET_INF_MAX,node,sizeof(ONVIF_NET_INF));
    break;
  case ONVIF_TYPE_OSD:
    free_node(g_osds,OSD_MAX,node,sizeof(ONVIF_OSD));
    break;
  case ONVIF_TYPE_CFG_DESC:
    free_node(g_cfg_descs,CFG_DESC_MAX,node,sizeof(ONVIF_CFG_DESC));
    break;
  case ONVIF_TYPE_SITEM_DESC:
    free_node(g_sitem_descs,SITEM_DESC_MAX,node,sizeof(ONVIF_SIMPLE_ITEM_DESC));
    break;
  case ONVIF_TYPE_CFGDESC_MSG:
    free_node(g_cfgdesc_msgs,CFG_DESC_MSG_MAX,node,sizeof(ONVIF_CFG_DESC_MSG));
    break;
  case ONVIF_TYPE_PTZ_NODE:
    free_node(g_ptz_nodes,PTZ_NODE_MAX,node,sizeof(PTZ_NODE));
    break;
  case ONVIF_TYPE_RECINF:
    free_node(g_recinfs,RECINF_MAX,node,sizeof(ONVIF_RECINF));
    break;
  case ONVIF_TYPE_RECORDING:
    free_node(g_records,RECORD_MAX,node,sizeof(ONVIF_RECORDING));
    break;
  case ONVIF_TYPE_RECORDINGJOB:
    free_node(g_recordjobs,RECORDINGJOB_MAX,node,sizeof(ONVIF_RECORDINGJOB));
    break;
  case ONVIF_TYPE_LINK_NODE:
    free_node(g_linknodes,LINKED_NODE_MAX,node,sizeof(LINKED_NODE));
    break;
  case ONVIF_TYPE_LINK_LIST:
    free_node(g_linklists,LINKED_LIST_MAX,node,sizeof(LINKED_LIST));
    break;
  default:
    return NULL;
  }
}

void onvif_init_multicast_cfg(onvif_MulticastConfiguration * p_cfg)
{
	p_cfg->Port = 32002;
	p_cfg->TTL = 2;
	p_cfg->AutoStart = FALSE;
	strcpy(p_cfg->IPv4Address, "239.0.1.0");
}


ONVIF_RET onvif_add_scopes(ONVIF_SCOPE * p_scope, int scope_max)
{
    int i;
	int SdkResult = ONVIF_OK;

	SdkResult = my_onvif_add_scopes(p_scope,i);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_set_scopes(ONVIF_SCOPE * p_scope, int scope_max)
{
    int i;
	int SdkResult = ONVIF_OK;
	for (i = 0; i < scope_max; i++)
	{
		if (p_scope[i].scope[0] == '\0')
		{
			break;
		}
	}

	SdkResult = my_onvif_set_scopes(p_scope,i);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_remove_scopes(ONVIF_SCOPE * p_scope, int scope_max)
{
    int i;
	int SdkResult = ONVIF_OK;
	for (i = 0; i < scope_max; i++)
	{
		if (p_scope[i].scope[0] == '\0')
		{
			break;
		}
	}

	SdkResult = my_onvif_remove_scopes(p_scope,i);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

const char * onvif_get_user_pass(const char * username)
{
	return my_onvif_get_user_pass(username);
}


ONVIF_RET onvif_add_users(ONVIF_USER * p_user, int user_max)
{
    int i;
	int SdkResult = ONVIF_OK;
	for (i = 0; i < user_max; i++)
	{
	    int len;
		if (p_user[i].Username[0] == '\0')
		{
			break;
		}

		len = strlen(p_user[i].Username);
		if (len <= 3)
		{
			return ONVIF_ERR_USERNAME_TOO_SHORT;
		}
	}

	SdkResult = my_onvif_add_users(p_user,i);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_delete_users(ONVIF_USER * p_user, int user_max)
{
    int i;
	int SdkResult = ONVIF_OK;
	for (i = 0; i < user_max; i++)
	{
		if (p_user[i].Username[0] == '\0')
		{
			break;
		}
	}	
	
	SdkResult = my_onvif_delete_users(p_user,i);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

ONVIF_RET onvif_set_users(ONVIF_USER * p_user, int user_max)
{
    int i;
	int SdkResult = ONVIF_OK;
	for (i = 0; i < user_max; i++)
	{
		if (p_user[i].Username[0] == '\0')
		{
			break;
		}
	}
	
	SdkResult = my_onvif_set_users(p_user,i);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}


ONVIF_NOTIFYMESSAGE * onvif_add_notify_message(ONVIF_NOTIFYMESSAGE ** p_message)
{
	ONVIF_NOTIFYMESSAGE * p_tmp;
	ONVIF_NOTIFYMESSAGE * p_new_message = (ONVIF_NOTIFYMESSAGE *)get_one_item(ONVIF_TYPE_NOTIFYMESSAGE);//(ONVIF_NOTIFYMESSAGE *) malloc(sizeof(ONVIF_NOTIFYMESSAGE));
	if (NULL == p_new_message)
	{
		return NULL;
	}

	memset(p_new_message, 0, sizeof(ONVIF_NOTIFYMESSAGE));

	if (p_message)
	{
		p_tmp = *p_message;
		if (NULL == p_tmp)
		{
			*p_message = p_new_message;
		}
		else
		{
			while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

			p_tmp->next = p_new_message;
		}	
	}

	return p_new_message;
}

void onvif_free_notify_message(ONVIF_NOTIFYMESSAGE * p_message)
{
	if (p_message)
	{
		onvif_free_simple_items(&p_message->NotificationMessage.Message.Source.SimpleItem);
		onvif_free_simple_items(&p_message->NotificationMessage.Message.Key.SimpleItem);
		onvif_free_simple_items(&p_message->NotificationMessage.Message.Data.SimpleItem);

		onvif_free_element_items(&p_message->NotificationMessage.Message.Source.ElementItem);
		onvif_free_element_items(&p_message->NotificationMessage.Message.Key.ElementItem);
		onvif_free_element_items(&p_message->NotificationMessage.Message.Data.ElementItem);
	}
}

void onvif_free_notify_messages(ONVIF_NOTIFYMESSAGE ** p_message)
{
	ONVIF_NOTIFYMESSAGE * p_next;
	ONVIF_NOTIFYMESSAGE * p_tmp = *p_message;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		onvif_free_notify_message(p_tmp);
    free_one_item(p_tmp, ONVIF_TYPE_NOTIFYMESSAGE);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_message = NULL;
}

ONVIF_SIMPLEITEM * onvif_add_simple_item(ONVIF_SIMPLEITEM ** p_simpleitem)
{
	ONVIF_SIMPLEITEM * p_tmp;
	ONVIF_SIMPLEITEM * p_new_item = (ONVIF_SIMPLEITEM *)get_one_item(ONVIF_TYPE_SIMPLEITEM);//(ONVIF_SIMPLEITEM *) malloc(sizeof(ONVIF_SIMPLEITEM));
	if (NULL == p_new_item)
	{
		return NULL;
	}

	memset(p_new_item, 0, sizeof(ONVIF_SIMPLEITEM));

	p_tmp = *p_simpleitem;
	if (NULL == p_tmp)
	{
		*p_simpleitem = p_new_item;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_item;
	}	

	return p_new_item;
}

void onvif_free_simple_items(ONVIF_SIMPLEITEM ** p_simpleitem)
{
    ONVIF_SIMPLEITEM * p_next;
	ONVIF_SIMPLEITEM * p_tmp = *p_simpleitem;

	while (p_tmp)
	{
		p_next = p_tmp->next;
		free_one_item(p_tmp, ONVIF_TYPE_SIMPLEITEM);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_simpleitem = NULL;
}


ONVIF_ELEMENTITEM * onvif_add_element_item(ONVIF_ELEMENTITEM ** p_elementitem)
{
	ONVIF_ELEMENTITEM * p_tmp;
	ONVIF_ELEMENTITEM * p_new_item = (ONVIF_ELEMENTITEM *)get_one_item(ONVIF_TYPE_ELEMENTITEM);//(ONVIF_ELEMENTITEM *) malloc(sizeof(ONVIF_ELEMENTITEM));
	if (NULL == p_new_item)
	{
		return NULL;
	}

	memset(p_new_item, 0, sizeof(ONVIF_ELEMENTITEM));

	p_tmp = *p_elementitem;
	if (NULL == p_tmp)
	{
		*p_elementitem = p_new_item;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_item;
	}	

	return p_new_item;
}

void onvif_free_element_items(ONVIF_ELEMENTITEM ** p_elementitem)
{
	ONVIF_ELEMENTITEM * p_next;
	ONVIF_ELEMENTITEM * p_tmp = *p_elementitem;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		if(p_tmp->ElementItem.Any)
      free_net_buf(p_tmp->ElementItem.Any);
			//free(p_tmp->ElementItem.Any);
		free_one_item(p_tmp, ONVIF_TYPE_ELEMENTITEM);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_elementitem = NULL;
}



#ifdef PROFILE_G_SUPPORT

ONVIF_TRACK * onvif_add_track(ONVIF_TRACK ** p_tracks)
{
	ONVIF_TRACK * p_tmp;
	ONVIF_TRACK * p_track = (ONVIF_TRACK *)get_one_item(ONVIF_TYPE_TRACK);//(ONVIF_TRACK *) malloc(sizeof(ONVIF_TRACK));
	if (NULL == p_track)
	{
		return NULL;
	}

	memset(p_track, 0, sizeof(ONVIF_TRACK));

	p_tmp = *p_tracks;
	if (NULL == p_tmp)
	{
		*p_tracks = p_track;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_track;
	}	

	return p_track;
}

void onvif_free_tracks(ONVIF_TRACK ** p_tracks)
{
	ONVIF_TRACK * p_next;
	ONVIF_TRACK * p_tmp = *p_tracks;

	while (p_tmp)
	{
		p_next = p_tmp->next;
    free_one_item(p_tmp, ONVIF_TYPE_TRACK);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_tracks = NULL;
}

ONVIF_TRACK * onvif_find_track(ONVIF_TRACK * p_tracks, const char * token)
{
	ONVIF_TRACK * p_track = p_tracks;
	while (p_track)
	{
		if (strcmp(p_track->Track.TrackToken, token) == 0)
		{
			break;
		}

		p_track = p_track->next;
	}

	return p_track;
}

int	onvif_get_track_nums_by_type(ONVIF_TRACK * p_tracks, onvif_TrackType type)
{
	int nums = 0;
	
	ONVIF_TRACK * p_track = p_tracks;
	while (p_track)
	{
		if (p_track->Track.Configuration.TrackType == type)
		{
			nums++;
		}

		p_track = p_track->next;
	}

	return nums;
}

#endif	// end of PROFILE_G_SUPPORT

#ifdef VIDEO_ANALYTICS

ONVIF_CONFIG * onvif_add_config(ONVIF_CONFIG ** p_config)
{
	ONVIF_CONFIG * p_tmp;
	ONVIF_CONFIG * p_new_config = (ONVIF_CONFIG *)get_one_item(ONVIF_TYPE_CONFIG);//(ONVIF_CONFIG *) malloc(sizeof(ONVIF_CONFIG));
	if (NULL == p_new_config)
	{
		return NULL;
	}

	memset(p_new_config, 0, sizeof(ONVIF_CONFIG));

	p_tmp = *p_config;
	if (NULL == p_tmp)
	{
		*p_config = p_new_config;
	}
	else
	{
		while (p_tmp && p_tmp->next) p_tmp = p_tmp->next;

		p_tmp->next = p_new_config;
	}	

	return p_new_config;
}

void onvif_free_config(ONVIF_CONFIG * p_config)
{
	onvif_free_simple_items(&p_config->Config.Parameters.SimpleItem);
	onvif_free_element_items(&p_config->Config.Parameters.ElementItem);
}

void onvif_free_configs(ONVIF_CONFIG ** p_config)
{
	ONVIF_CONFIG * p_next;
	ONVIF_CONFIG * p_tmp = *p_config;

	while (p_tmp)
	{
		p_next = p_tmp->next;

		onvif_free_config(p_tmp);
		free_one_item(p_tmp, ONVIF_TYPE_CONFIG);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_config = NULL;
}

ONVIF_CONFIG * onvif_find_config(ONVIF_CONFIG ** p_config, const char * name)
{
	ONVIF_CONFIG * p_cfg = *p_config;
	while (p_cfg)
	{
		if (strcmp(p_cfg->Config.Name, name) == 0)
		{
			break;
		}

		p_cfg = p_cfg->next;
	}

	return p_cfg;
}

void onvif_remove_config(ONVIF_CONFIG ** p_config, ONVIF_CONFIG * p_remove)
{
	BOOL found = FALSE;
	ONVIF_CONFIG * p_prev = NULL;
	ONVIF_CONFIG * p_cfg = *p_config;	
	
	while (p_cfg)
	{
		if (p_cfg == p_remove)
		{
			found = TRUE;
			break;
		}

		p_prev = p_cfg;
		p_cfg = p_cfg->next;
	}

	if (found)
	{
		if (NULL == p_prev)
		{
			*p_config = p_cfg->next;
		}
		else
		{
			p_prev->next = p_cfg->next;
		}

		onvif_free_config(p_cfg);
    free_one_item(p_cfg,ONVIF_TYPE_CONFIG);
		//free(p_cfg);
	}
}

ONVIF_CONFIG * onvif_get_config_prev(ONVIF_CONFIG ** p_config, ONVIF_CONFIG * p_found)
{
	ONVIF_CONFIG * p_prev = *p_config;
	
	if (p_found == *p_config)
	{
		return NULL;
	}

	while (p_prev)
	{
		if (p_prev->next == p_found)
		{
			break;
		}
		
		p_prev = p_prev->next;
	}

	return p_prev;
}


#endif	// end of VIDEO_ANALYTICS

void onvif_free_simple_item_descs(ONVIF_SIMPLE_ITEM_DESC ** p_simpleitem)
{
    ONVIF_SIMPLE_ITEM_DESC * p_next;
	ONVIF_SIMPLE_ITEM_DESC * p_tmp = *p_simpleitem;

	while (p_tmp)
	{
		p_next = p_tmp->next;
		free_one_item(p_tmp,ONVIF_TYPE_SITEM_DESC);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_simpleitem = NULL;
}

void onvif_free_cfg_desc_msg(ONVIF_CFG_DESC_MSG * p_cfg)
{
	onvif_free_simple_item_descs(&p_cfg->Messages.Source.SimpleItemDescription);
	onvif_free_simple_item_descs(&p_cfg->Messages.Source.ElementItemDescription);
	onvif_free_simple_item_descs(&p_cfg->Messages.Key.SimpleItemDescription);
	onvif_free_simple_item_descs(&p_cfg->Messages.Key.ElementItemDescription);
	onvif_free_simple_item_descs(&p_cfg->Messages.Data.SimpleItemDescription);
	onvif_free_simple_item_descs(&p_cfg->Messages.Data.ElementItemDescription);
}

void onvif_free_cfg_desc_msgs(ONVIF_CFG_DESC_MSG ** p_cfg)
{
	ONVIF_CFG_DESC_MSG * p_next;
	ONVIF_CFG_DESC_MSG * p_tmp = *p_cfg;

	while (p_tmp)
	{
		p_next = p_tmp->next;
		onvif_free_cfg_desc_msg(p_tmp);
		free_one_item(p_tmp,ONVIF_TYPE_CFGDESC_MSG);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_cfg = NULL;
}

void onvif_free_cfg_desc(ONVIF_CFG_DESC * p_cfgdesc)
{
	onvif_free_simple_item_descs(&p_cfgdesc->ConfigDescription.Parameters.SimpleItemDescription);
	onvif_free_simple_item_descs(&p_cfgdesc->ConfigDescription.Parameters.ElementItemDescription);
	onvif_free_cfg_desc_msgs(&p_cfgdesc->ConfigDescription.Messages);
}

void onvif_free_cfg_descs(ONVIF_CFG_DESC ** p_cfgdesc)
{
	ONVIF_CFG_DESC * p_next;
	ONVIF_CFG_DESC * p_tmp = *p_cfgdesc;

	while (p_tmp)
	{
		p_next = p_tmp->next;
		onvif_free_cfg_desc(p_tmp);
		free_one_item(p_tmp,ONVIF_TYPE_CFG_DESC);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_cfgdesc = NULL;
}

void onvif_free_profile_sev(ONVIF_PROFILE * p_profile)
{
	if (NULL == p_profile)
	{
		return;
	}
	
	if (p_profile->v_src_cfg)
    {
      free_one_item(p_profile->v_src_cfg,ONVIF_TYPE_V_SRC);
        //free(p_profile->v_src_cfg);
    }

    if (p_profile->v_enc_cfg)
    {
      free_one_item(p_profile->v_enc_cfg,ONVIF_TYPE_V_ENC);
      //  free(p_profile->v_enc_cfg);
    }

    if (p_profile->a_src_cfg)
    {
      free_one_item(p_profile->a_src_cfg,ONVIF_TYPE_A_SRC);
      //  free(p_profile->a_src_cfg);
    }

    if (p_profile->a_enc_cfg)
    {
      free_one_item(p_profile->a_enc_cfg,ONVIF_TYPE_A_ENC);
      //  free(p_profile->a_enc_cfg);
    }

    if (p_profile->metadata_cfg)
    {
      free_one_item(p_profile->metadata_cfg,ONVIF_TYPE_METADATA);
      //  free(p_profile->metadata_cfg);
    }

    if (p_profile->ptz_cfg)
    {
      free_one_item(p_profile->ptz_cfg,ONVIF_TYPE_PTZ);
      //  free(p_profile->ptz_cfg);
    }

    if (p_profile->va_cfg)
    {
		onvif_free_configs(&p_profile->va_cfg->rules);
		onvif_free_configs(&p_profile->va_cfg->modules);
		onvif_free_configs(&p_profile->va_cfg->VideoAnalyticsConfiguration.AnalyticsEngineConfiguration.AnalyticsModule);
		onvif_free_configs(&p_profile->va_cfg->VideoAnalyticsConfiguration.RuleEngineConfiguration.Rule);
		onvif_free_cfg_descs(&p_profile->va_cfg->SupportedRules.RuleDescription);
      free_one_item(p_profile->va_cfg,ONVIF_TYPE_VACFG);
      //  free(p_profile->va_cfg);
    }
}

void onvif_free_profiles_sev(ONVIF_PROFILE ** p_profile)
{
	ONVIF_PROFILE * p_next;
	ONVIF_PROFILE * p_tmp = *p_profile;

	while (p_tmp)
	{
		p_next = p_tmp->next;

        onvif_free_profile_sev(p_tmp);
    free_one_item(p_tmp,ONVIF_TYPE_PROFILE);
		//free(p_tmp);
		p_tmp = p_next;
	}

	*p_profile = NULL;
}



void onvif_init()
{
	memset(&g_onvif_cls, 0, sizeof(ONVIF_CLS));
	
	onvif_eua_init();
	//modify 8000 ¶Ë¿Ú³åÍ»
	g_onvif_cls.local_port = 8000;
	const char * ip = onvif_get_local_ip();
	if (ip)
	{
		strcpy(g_onvif_cls.local_ipstr, ip);
	}
}






