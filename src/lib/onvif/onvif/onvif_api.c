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
#include "onvif/onvif/hxml.h"
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif_probe.h"
#include "onvif/onvif/http.h"
#include "onvif/onvif/onvif_device.h"
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/onvif_timer.h"
#include "onvif/onvif/onvif_api.h"
#include "onvif/cfgpro/onvif_cfgfile_mng.h"

/***************************************************************************************/
static HTTPSRV hsrv;
extern ONVIF_CLS g_onvif_cls;

/***************************************************************************************/
void onvif_start()
{
	onvif_init();
	Onvif_Info* pOnvifInfo = GetOnvifInfo();
	
    if (pOnvifInfo->log_enable)
	{
	    //log_init("ipsee.txt");
	}
	printf("Happytime onvif server version 3.6\r\n");
    printf("Onvif server running at %s:%d\r\n", g_onvif_cls.local_ipstr, g_onvif_cls.local_port);

	sys_buf_init();
	http_msg_buf_fl_init(16);
	
	http_srv_init(&hsrv, inet_addr(g_onvif_cls.local_ipstr), g_onvif_cls.local_port, 16);

	//onvif_timer_init();

	onvif_start_discovery();
}

void onvif_stop()
{
	onvif_stop_discovery();

	//onvif_timer_deinit();

	http_srv_deinit(&hsrv);
	
	http_msg_buf_fl_deinit();
	sys_buf_deinit();

  log_close();
  onvif_eua_deinit();
}

void onvif_modify_localip(unsigned int *pdata)
{
	struct in_addr addr;
  addr.s_addr = pdata[0];

	strcpy(g_onvif_cls.local_ipstr, inet_ntoa(addr));
  printf("%s,%d,IP=%s!!!!!!!!!\n",__FUNCTION__,__LINE__,g_onvif_cls.local_ipstr);
}



