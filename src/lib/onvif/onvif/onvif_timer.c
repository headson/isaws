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
#include "onvif/onvif/onvif_timer.h"
//#include "file_msg_drv.h"
#include "sys_msg_drv.h"

/*******************************************************************/
extern ONVIF_CLS g_onvif_cls;

//static int g_timer_cnt;
#define  default_renew_time     60
/*******************************************************************/
void onvif_timer(void *arg)
{
	unsigned int i;
	time_t cur_time = time(NULL);
	//EUA * p_eua = NULL;

	int max_renew_time;

	//g_timer_cnt++;
	///*
	EUA * p_eua = onvif_eua_lookup_start();
	while (p_eua)
	{
		if(p_eua->init_term_time > 0)
			max_renew_time = p_eua->init_term_time;
		else
			max_renew_time = default_renew_time;
		
		if ((cur_time - p_eua->last_renew_time) > max_renew_time)
		{
			EUA * p_eua_next =  onvif_eua_lookup_next((void *)p_eua);
			onvif_eua_lookup_stop();
			onvif_free_used_eua(p_eua);
			p_eua = p_eua_next;
			onvif_eua_lookup_start();
			continue;
		}
		//Onvif_Info* pOnvifInfo = GetOnvifInfo();
		//if (pOnvifInfo->evt_sim_flag && (g_timer_cnt % pOnvifInfo->evt_sim_interval) == 0)
		{
			if (!p_eua->pollMode)	// notify mode
			{
				ONVIF_NOTIFYMESSAGE * p_message = onvif_init_notify_message(p_eua,arg);
				if (p_message)
				{
					onvif_notify(p_eua, p_message);
					onvif_free_notify_messages(&p_message);
				}
			}
		}
		p_eua = onvif_eua_lookup_next((void *)p_eua);
	}
	onvif_eua_lookup_stop();
	//*/
}


#if	__WIN32_OS__

#pragma comment(lib, "winmm.lib")

void CALLBACK onvif_win_timer(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
	onvif_timer(NULL);
}

void onvif_timer_init()
{
	g_onvif_cls.timer_id = timeSetEvent(1000,0,onvif_win_timer,0,TIME_PERIODIC);
}

void onvif_timer_deinit()
{
	timeKillEvent(g_onvif_cls.timer_id);
}

#else

void * onvif_timer_task(void * arg)
{
	struct timeval tv;
	MSG_BUF msgbuf;
	void * argv = NULL;
	int qID = Msg_Init(MSG_KEY);
	if(qID < 0)
	{
		printf("Failed to Msg_Init MSG_KEY\n");
		return(-1);
	}
	
	while (g_onvif_cls.sys_timer_run == 1)
	{	
		int msgSize = msgrcv(qID, &msgbuf,sizeof(MSG_BUF)-sizeof(long), SYS_ONVIF_SERVER_MSG, IPC_NOWAIT);
		if(msgSize < 0)
		{
			//tv.tv_sec = 1;
			//tv.tv_usec = 0;
			//select(1,NULL,NULL,NULL,&tv);
			
			if(onvif_GetSynchronizationPoint())
			{
				onvif_SetSynchronizationPoint(0);
			}
			else
			{
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				select(1,NULL,NULL,NULL,&tv);
			}
			argv = NULL;
		}
		else
		{
			switch(msgbuf.cmd)
			{
			case(MSG_CMD_SET_ONVIF_SEND_EVENT):
				{
					EventConfig *pEventCfg = (EventConfig *)msgbuf.buffer;
					argv = msgbuf.buffer;
				}
				break;
			default:
				argv = NULL;
				break;
			}
		}
		onvif_timer(argv);
	}

	g_onvif_cls.timer_id = 0;
	Msg_Kill(qID);
	log_print("onvif timer task exit\r\n");
}

void onvif_timer_init()
{
    pthread_t tid;
    
	g_onvif_cls.sys_timer_run = 1;

	tid = sys_os_create_thread((void *)onvif_timer_task, NULL);
	if (tid == 0)
	{
		log_print("onvif_timer_init::pthread_create pp_timer_task\r\n");
		return;
	}

    g_onvif_cls.timer_id = (unsigned int)tid;

	log_print("create onvif timer thread sucessful\r\n");
}

void onvif_timer_deinit()
{
	g_onvif_cls.sys_timer_run = 0;
	while (g_onvif_cls.timer_id != 0)
	{
		usleep(1000);
	}
}

#endif



