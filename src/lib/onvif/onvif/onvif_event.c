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
#include "onvif/onvif/onvif_event.h"
#include "onvif/onvif/http_cln.h"
#include "onvif/onvif/onvif_pkt.h"
#include "onvif/onvif/onvif_util.h"
#include "onvif/bm/hqueue.h"

/***************************************************************************************/
extern ONVIF_CLS	g_onvif_cls;
static int SetSyncPoint = 0;
/***************************************************************************************/
void onvif_eua_init()
{
	g_onvif_cls.eua_fl = pps_ctx_fl_init(MAX_NUM_EUA, sizeof(EUA), TRUE);
	g_onvif_cls.eua_ul = pps_ctx_ul_init(g_onvif_cls.eua_fl, TRUE);

	g_onvif_cls.msg_list = h_list_create(FALSE);
	g_onvif_cls.mutex_list = sys_os_create_mutex();
}

void onvif_eua_deinit()
{
	LINKED_NODE * p_node;
	
	if (g_onvif_cls.eua_ul)
	{
		pps_ul_free(g_onvif_cls.eua_ul);
		g_onvif_cls.eua_ul = NULL;
	}
	
	if (g_onvif_cls.eua_fl)
	{
		pps_fl_free(g_onvif_cls.eua_fl);
		g_onvif_cls.eua_fl = NULL;
	}

	// free notify message list
	
	sys_os_mutex_enter(g_onvif_cls.mutex_list);

	p_node = h_list_lookup_start(g_onvif_cls.msg_list);
	while (p_node)
	{
		onvif_free_notify_message((ONVIF_NOTIFYMESSAGE *)p_node->p_data);
		
		p_node = h_list_lookup_next(g_onvif_cls.msg_list, p_node);
	}
	h_list_lookup_end(g_onvif_cls.msg_list);

	h_list_free_container(g_onvif_cls.msg_list);
	g_onvif_cls.msg_list = NULL;
	
	sys_os_mutex_leave(g_onvif_cls.mutex_list);

	sys_os_destroy_sig_mutx(g_onvif_cls.mutex_list);
	g_onvif_cls.mutex_list = NULL;
}

EUA * onvif_get_idle_eua()
{
	EUA * p_eua = (EUA *)ppstack_fl_pop(g_onvif_cls.eua_fl);
	if (p_eua)
	{
		memset(p_eua, 0, sizeof(EUA));
		
		pps_ctx_ul_add(g_onvif_cls.eua_ul, p_eua);
	}

	return p_eua;
}

void onvif_set_idle_eua(EUA * p_eua)
{
	pps_ctx_ul_del(g_onvif_cls.eua_ul, p_eua);

	memset(p_eua, 0, sizeof(EUA));
	
	ppstack_fl_push(g_onvif_cls.eua_fl, p_eua);
}

void onvif_free_used_eua(EUA * p_eua)
{
	if (pps_safe_node(g_onvif_cls.eua_fl, p_eua) == FALSE)
		return;
	
	onvif_set_idle_eua(p_eua);
}


uint32 onvif_get_eua_index(EUA * p_eua)
{
	return pps_get_index(g_onvif_cls.eua_fl, p_eua);
}

EUA * onvif_get_eua_by_index(unsigned int index)
{
	return (EUA *)pps_get_node_by_index(g_onvif_cls.eua_fl, index);
}

EUA * onvif_eua_lookup_start()
{
	return (EUA *)pps_lookup_start(g_onvif_cls.eua_ul);
}

EUA * onvif_eua_lookup_next(void * p_eua)
{
	return (EUA *)pps_lookup_next(g_onvif_cls.eua_ul, (EUA *)p_eua);
}

void onvif_eua_lookup_stop()
{
	pps_lookup_end(g_onvif_cls.eua_ul);
}

EUA * onvif_eua_lookup_by_addr(const char * addr)
{
	EUA * p_eua = onvif_eua_lookup_start();
	while (p_eua)
	{
		if (strcmp(p_eua->producter_addr, addr) == 0)
			break;
			
		p_eua = onvif_eua_lookup_next((void *)p_eua);
	}
	onvif_eua_lookup_stop();

	return p_eua;
}

BOOL onvif_put_notify_message(ONVIF_NOTIFYMESSAGE * p_message)
{
	BOOL ret = FALSE;
	
	sys_os_mutex_enter(g_onvif_cls.mutex_list);

	if (NULL != g_onvif_cls.msg_list)
	{
		if (h_list_get_number_of_nodes(g_onvif_cls.msg_list) >= 20) // max 20 notify message
		{
			LINKED_NODE * p_node = h_list_get_from_front(g_onvif_cls.msg_list);
			onvif_free_notify_message((ONVIF_NOTIFYMESSAGE *) p_node->p_data);
			
			h_list_remove_from_front(g_onvif_cls.msg_list);
		}

		ret = h_list_add_at_back(g_onvif_cls.msg_list, (void *)p_message);
	}

	sys_os_mutex_leave(g_onvif_cls.mutex_list);

	return ret;
}

HQUEUE * onvif_xpath_parse(char * xpath)
{
    BOOL ret = TRUE;
    HQUEUE * queue = hqCreate(100, 128, 0);	
    
    int i = 0, fpush = 0;
    int quote_nums = 0;
	char fbuf[100];
	char buff[128];
    char * p = xpath; 

    while (*p != '\0')
    {
        switch (*p)
        {
        case ' ':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;
            
        case '(':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
			
			fbuf[fpush++] = '(';
            break;

        case '/':
            if (*(p+1) == '/') p++;
            break;

        case ':':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;

        case '[':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}

			fbuf[fpush++] = '[';
            break;

        case '=':
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
			else
			{
				ret = FALSE;
				break;
			}
            break;

        case ']':
			if (fpush == 0)
			{
				return FALSE;
			}
			if (fbuf[--fpush] != '[')
			{
				ret = FALSE;
				break;
			}
			
			if (i > 0)
			{
				buff[i] = '\0';
				hqBufPut(queue, buff);
				i = 0;
			}
            break;
           
		case '"':
			if (quote_nums == 0)
			{
				quote_nums++;	
				i = 0;
			}
			else if (quote_nums == 1)
			{
				quote_nums = 0;

				if (i > 0)
				{
					buff[i] = '\0';
					hqBufPut(queue, buff);
					i = 0;
				}
				else
				{
					ret = FALSE;
				    break;
				}
			}
			break;

        case ')':
			if (fpush == 0)
			{
				ret = FALSE;
				break;
			}
			if (fbuf[--fpush] != '(')
			{
				ret = FALSE;
				break;
			}

			hqBufPut(queue, ")");
            break;

        default:
            buff[i++] = *p;
            break;
        }
        
        p++;
    }

    if (ret == FALSE || fpush != 0)
    {
        hqDelete(queue);
        queue = NULL;
    }
    
    return queue;
}

ONVIF_RET onvif_check_filters(ONVIF_FILTER * p_filter)
{
    int i;

    for (i = 0; i < 10; i++)
    {
        if (p_filter->TopicExpression[i][0] != '\0')
        {
            // todo : check if support the topic ...
            char * topic1 = strstr(p_filter->TopicExpression[i],"tns1:RuleEngine");
			char * topic2 = strstr(p_filter->TopicExpression[i],"tns1:RecordingConfig");
			if((topic1 == NULL) && (topic2 == NULL))
            {
                return ONVIF_ERR_INVALID_TOPIC_EXPRESSION_FAULT;
            }
        }

        if (p_filter->MessageContent[i][0] != '\0')
        {
            // todo : check the message content is valid ...
            
            HQUEUE * queue = onvif_xpath_parse(p_filter->MessageContent[i]);
            if (NULL == queue)
            {
                return ONVIF_ERR_INVALID_MESSAGE_CONTENT_EXPRESSION_FAULT;
            }
            hqDelete(queue);
        }
    }
    
    return ONVIF_OK;
}

/***************************************************************************************/

ONVIF_RET onvif_Subscribe(Subscribe_REQ * p_req)
{
    EUA * p_eua;
    
    if (p_req->FiltersFlag)
    {
        ONVIF_RET ret = onvif_check_filters(&p_req->Filters);
        if (ONVIF_OK != ret)
        {
            return ret;
        }
    }
    
	p_eua = onvif_get_idle_eua();
	if (p_eua)
	{
		if (p_req->InitialTerminationTimeFlag)
		{
			p_eua->init_term_time = p_req->InitialTerminationTime;
		}
		
		strcpy(p_eua->consumer_addr, p_req->ConsumerReference);

		onvif_parse_xaddr(p_eua->consumer_addr, p_eua->host, p_eua->url, &p_eua->port);

		sprintf(p_eua->producter_addr, "http://%s:%d/event_service/%u", g_onvif_cls.local_ipstr, g_onvif_cls.local_port, onvif_get_eua_index(p_eua));
		p_eua->subscibe_time = time(NULL);
		p_eua->last_renew_time = time(NULL);
		
		p_eua->used = 1;

		p_eua->FiltersFlag = p_req->FiltersFlag;
        memcpy(&p_eua->Filters, &p_req->Filters, sizeof(ONVIF_FILTER));

		p_req->p_eua = p_eua;
	}
	else
	{
	    return ONVIF_ERR_RESOURCE_UNKNOWN_FAULT;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_Renew(Renew_REQ * p_req)
{
	EUA * p_eua = onvif_eua_lookup_by_addr(p_req->ProducterReference);
	if (p_eua)
	{
		time_t cur_time = time(NULL);
		if ((cur_time - p_eua->last_renew_time) >= p_eua->init_term_time)
		{
			onvif_free_used_eua(p_eua);
		}
		else
		{
			p_eua->last_renew_time = cur_time;
			return ONVIF_OK;
		}
	}	

	return ONVIF_ERR_RESOURCE_UNKNOWN_FAULT;
}

ONVIF_RET onvif_Unsubscribe(const char * addr)
{
	EUA * p_eua = onvif_eua_lookup_by_addr(addr);
	if (p_eua)
	{
		onvif_free_used_eua(p_eua);

		return ONVIF_OK;
	}	

	return ONVIF_ERR_RESOURCE_UNKNOWN_FAULT;
}

ONVIF_RET onvif_CreatePullPointSubscription(CreatePullPointSubscription_REQ * p_req)
{    
    EUA * p_eua;
     
    if (p_req->FiltersFlag)
    {
        ONVIF_RET ret = onvif_check_filters(&p_req->Filters);
        if (ONVIF_OK != ret)
        {
            return ret;
        }
    }

	p_eua = onvif_get_idle_eua();
	if (p_eua)
	{
		if (p_req->InitialTerminationTimeFlag)
		{
			p_eua->init_term_time = p_req->InitialTerminationTime;
		}

		sprintf(p_eua->producter_addr, "http://%s:%d/event_service/%u", g_onvif_cls.local_ipstr, g_onvif_cls.local_port, onvif_get_eua_index(p_eua));
		p_eua->subscibe_time = time(NULL);
		p_eua->last_renew_time = time(NULL);

		p_eua->pollMode = TRUE;
		p_eua->last_msg = time(NULL);
		
		p_eua->used = 1;

		p_eua->FiltersFlag = p_req->FiltersFlag;
        memcpy(&p_eua->Filters, &p_req->Filters, sizeof(ONVIF_FILTER));
        
		p_req->p_eua = p_eua;
	}
	else
	{
	    return ONVIF_ERR_RESOURCE_UNKNOWN_FAULT;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_PullMessages(PullMessages_REQ * p_req)
{
	EUA * p_eua = onvif_get_eua_by_index(p_req->eua_idx);
	if (p_eua)
	{
		p_eua->last_renew_time = time(NULL);
		
		return ONVIF_OK;
	}	

	return ONVIF_ERR_RESOURCE_UNKNOWN_FAULT;
}

void onvif_notify(EUA * p_eua, ONVIF_NOTIFYMESSAGE * p_message)
{
    int len;
	char bufs[4*1024];	
	HTTPREQ	req;
	
	memset(&req, 0, sizeof(HTTPREQ));
	
	strcpy(req.host, p_eua->host);
	strcpy(req.url, p_eua->url);
	strcpy(req.action, "http://docs.oasis-open.org/wsn/bw-2/NotificationConsumer/Notify");
	req.port = p_eua->port;
	len = build_Notify_xml(bufs, sizeof(bufs), (char *)p_message);

	http_onvif_trans(&req, 5000, bufs, len);
}

BOOL onvif_simpleitem_filter(ONVIF_SIMPLEITEM * p_item, char * name, char * value, BOOL flag)
{
    BOOL nameflag = FALSE;
    BOOL valueflag = FALSE;
    
    while (p_item)
    {
        if (strcmp(p_item->SimpleItem.Name, name) == 0)
        {
            nameflag = TRUE;
        }

        
        if (value[0] != '\0')
        {
            if (strcmp(p_item->SimpleItem.Value, value) == 0)
            {
                valueflag = TRUE;
            }

            if (flag == 1)
            {
                if (nameflag && valueflag)
                {

                    return TRUE;
                }
            }
            else if (flag == 2)
            {
                if (nameflag || valueflag)
                {

                    return TRUE;
                }
            }
        }
        else if (nameflag)
        {
            return TRUE;
        }
        
        p_item = p_item->next;
    }

    return FALSE;
}

BOOL onvif_elementitem_filter(ONVIF_ELEMENTITEM * p_item, char * name)
{
    while (p_item)
    {
        if (strcmp(p_item->ElementItem.Name, name) == 0)
        {
            return TRUE;
        }

        
        p_item = p_item->next;
    }

    return FALSE;
}


BOOL onvif_message_content_filter_ex(HQUEUE  * queue, ONVIF_NOTIFYMESSAGE * p_message)
{
	int  flag = 0;
	int  itemflag = 0;
	char buff[128];	
	char name[100];
	char value[100] = {'\0'};

	if (hqBufGet(queue, buff) == FALSE) // prefix
	{
		return FALSE;
	}

	if (hqBufGet(queue, buff) == FALSE) // SimpleItem or ElementItem
	{
		return FALSE;
	}

	if (strcasecmp(buff, "simpleitem") == 0)
	{
		itemflag = 0;
	}
	else if (strcasecmp(buff, "ElementItem") == 0)
	{
		itemflag = 1;
	}
	else
	{
		return FALSE;
	}

	if (hqBufGet(queue, buff) == FALSE) // @Name
	{
		return FALSE;
	}
	if (strcasecmp(buff, "@name") != 0)
	{
		return FALSE;
	}

	if (hqBufGet(queue, buff) == FALSE) 
	{
		return FALSE;
	}
	strcpy(name, buff);
    
	if (hqBufPeek(queue, buff))
	{
		if (strcasecmp(buff, "and") == 0)
		{
			flag = 1;
		}
		else if (strcasecmp(buff, "or") == 0)
		{
			flag = 2;
		}
		else
		{
			goto finish;
		}

		hqBufGet(queue, buff);

		if (hqBufGet(queue, buff) == FALSE) 
		{
			return FALSE;
		}
		if (strcasecmp(buff, "@value") != 0)
		{
			return FALSE;
		}

		if (hqBufGet(queue, buff) == FALSE) 
		{
			return FALSE;
		}
		strcpy(value, buff);
	}

finish:

    if (itemflag == 0)
    {
        if (p_message->NotificationMessage.Message.SourceFlag && onvif_simpleitem_filter(p_message->NotificationMessage.Message.Source.SimpleItem, name, value, flag))
        {
            return TRUE;
        }
        
        if (p_message->NotificationMessage.Message.KeyFlag && onvif_simpleitem_filter(p_message->NotificationMessage.Message.Key.SimpleItem, name, value, flag))
        {
            return TRUE;
        }

        if (p_message->NotificationMessage.Message.DataFlag && onvif_simpleitem_filter(p_message->NotificationMessage.Message.Data.SimpleItem, name, value, flag))
        {
            return TRUE;
        }
    }
    else if (itemflag == 1)
    {
        if (p_message->NotificationMessage.Message.SourceFlag && onvif_elementitem_filter(p_message->NotificationMessage.Message.Source.ElementItem, name))
        {
            return TRUE;
        }

        if (p_message->NotificationMessage.Message.KeyFlag && onvif_elementitem_filter(p_message->NotificationMessage.Message.Key.ElementItem, name))
        {
            return TRUE;
        }

        if (p_message->NotificationMessage.Message.DataFlag && onvif_elementitem_filter(p_message->NotificationMessage.Message.Data.ElementItem, name))
        {
            return TRUE;
        }           
    }    
    
	return FALSE;
}

BOOL onvif_message_content_filter(char * filter, ONVIF_NOTIFYMESSAGE * p_message)
{
    BOOL notflag = FALSE;
    char buff[128];
    BOOL ret = FALSE;
    int  flag = 0;
        
    HQUEUE * queue = onvif_xpath_parse(filter);
    if (NULL == queue)
    {
        return FALSE;
    }

    while (!hqBufIsEmpty(queue))
    {
        hqBufGet(queue, buff);

        if (strcmp(buff, "not") == 0)
        {
            notflag = TRUE;            
        }
        else if (strcmp(buff, "boolean") == 0)
        {
            BOOL ret1 = onvif_message_content_filter_ex(queue, p_message);
			if (notflag)
			{
				ret1 = !ret1;
				notflag = FALSE;
			}

			if (flag == 1)
			{
			    ret = ret && ret1;
			}
			else if (flag == 2)
			{
			    ret = ret || ret1;
			}
			else
			{
			    ret = ret1;
			}
        }
		else if (strcmp(buff, "and") == 0)
		{
			flag = 1;
		}
		else if (strcmp(buff, "or") == 0)
		{
			flag = 2;
		}
    }
    
    hqDelete(queue);
    
    return ret;
}

BOOL onvif_event_filter(ONVIF_NOTIFYMESSAGE * p_message, EUA * p_eua)
{
    int i;
    
    if (p_eua->FiltersFlag == 0)
    {
        return TRUE;
    }

    for (i = 0; i < 10; i++)
    {
        if (p_eua->Filters.TopicExpression[i][0] != '\0')
        {            
            // todo : check the message is required ... 
            
            int len = strlen(p_eua->Filters.TopicExpression[i]);
            char topic[256];
            
            strcpy(topic, p_eua->Filters.TopicExpression[i]);
            
            if (topic[len-1] == '*')
            {
                topic[len-1] = '\0';
            }
            
            if (strstr(topic, p_eua->Filters.TopicExpression[i]))
            {
                return TRUE;
            }
        }

        if (p_eua->Filters.MessageContent[i][0] != '\0')
        {
            // todo : check the message is required ... 

            if (onvif_message_content_filter(p_eua->Filters.MessageContent[i], p_message))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

/**
 Generate notify message, just for test
*/
ONVIF_NOTIFYMESSAGE * onvif_init_notify_message(EUA * p_eua,void *pEventCfg)
{
	if(pEventCfg)//get alarm event
	{
		return my_onvif_init_notify_message(p_eua,pEventCfg);
	}
	else// just for test
	{
		ONVIF_NOTIFYMESSAGE * p_message = onvif_add_notify_message(NULL);
		if (p_message)
		{
			ONVIF_SIMPLEITEM * p_simpleitem;
			
			strcpy(p_message->NotificationMessage.ConsumerAddress, p_eua->consumer_addr);
			strcpy(p_message->NotificationMessage.ProducterAddress, p_eua->producter_addr);
			strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
			if(p_eua->FiltersFlag)
				strcpy(p_message->NotificationMessage.Topic, p_eua->Filters.TopicExpression[0]);
			else
				strcpy(p_message->NotificationMessage.Topic, "tns1:RuleEngine/CellMotionDetector/Motion");
			p_message->NotificationMessage.Message.PropertyOperationFlag = 1;
			p_message->NotificationMessage.Message.PropertyOperation = PropertyOperation_Changed;
			p_message->NotificationMessage.Message.UtcTime = time(NULL);
			p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Source.SimpleItem);
			if (p_simpleitem)
			{
				p_message->NotificationMessage.Message.SourceFlag = 1;
				strcpy(p_simpleitem->SimpleItem.Name, "VideoSourceConfigurationToken");
				strcpy(p_simpleitem->SimpleItem.Value, "11");
			}

			p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
			if (p_simpleitem)
			{
				p_message->NotificationMessage.Message.DataFlag= 1;
				strcpy(p_simpleitem->SimpleItem.Name, "IsMotion");
				strcpy(p_simpleitem->SimpleItem.Value, "true");
			}
		}
		return p_message;
	}
}

void onvif_SetSynchronizationPoint(int value)
{
	SetSyncPoint = value;
}

int onvif_GetSynchronizationPoint()
{
	int ret;
	ret = SetSyncPoint;
	return ret;
}






