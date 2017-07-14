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
#include "onvif/onvif/onvif_device.h"
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/onvif_util.h"
#include "onvif/cfgpro/onvif_cfgfile_mng.h"

/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;
extern char *g_probe_buf;


/***************************************************************************************/
int onvif_probe_init()
{
    int opt = 1;
    int addr_len;
	int len = 65535;
	SOCKET fd;
    struct sockaddr_in addr;
    struct ip_mreq mcast;
    
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		printf("socket SOCK_DGRAM error!\n");
		return -1;
	}	

	addr_len = sizeof(addr);

	addr.sin_family = AF_INET;
#if __WIN32_OS__
	addr.sin_addr.s_addr = inet_addr(g_onvif_cls.local_ipstr);
#else
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif
	addr.sin_port = htons(3702);
	
	if (bind(fd,(struct sockaddr *)&addr,sizeof(addr)) == -1)
	{
		printf("Bind udp socket fail,error = %s\r\n", sys_os_get_socket_error());
		closesocket(fd);
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&len, sizeof(int)))
	{
		printf("setsockopt SO_SNDBUF error!\n");
	}

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(int)))
	{
		printf("setsockopt SO_RCVBUF error!\n");
	}
    
	/* reuse socket addr */  
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))) 
    {  
        printf("setsockopt SO_REUSEADDR error!\n");
    }  
    
	mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
#if __WIN32_OS__	
	mcast.imr_interface.s_addr = inet_addr(g_onvif_cls.local_ipstr);
#else
	mcast.imr_interface.s_addr = htonl(INADDR_ANY);
#endif

	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0)
	{
		printf("setsockopt IP_ADD_MEMBERSHIP error!%s\n", sys_os_get_socket_error());
		return -1;
	}

	return fd;
}



int onvif_probe_rly(char * p_msg_id,int vfd,unsigned int rip,unsigned int rport)
{
    int i;
    int rlen;
    int offset;
    int buff_len;
    char send_buffer[1024 * 10];    
    struct sockaddr_in addr;
	
	Onvif_Info* pOnvifInfo = GetOnvifInfo();
	NetworkPort *pNetPort = GetNetPort();
	if (p_msg_id == NULL || DiscoveryMode_NonDiscoverable == pOnvifInfo->DiscoveryMode)
	{
		return -1;
	}
	
	buff_len = sizeof(send_buffer);
	
	offset = snprintf(send_buffer, buff_len, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
			"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
			"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
			"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
			"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
			"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
			"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
			"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "
			"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
			"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");
	
	offset += snprintf(send_buffer+offset, buff_len-offset, "<s:Header>"
			"<wsa:MessageID>uuid:%s</wsa:MessageID>"
			"<wsa:RelatesTo>%s</wsa:RelatesTo>"
			"<wsa:To s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To>"
			"<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action>"
			"</s:Header>", onvif_uuid_create(), p_msg_id);

	offset += snprintf(send_buffer+offset, buff_len-offset, "<s:Body>"
			"<d:ProbeMatches><d:ProbeMatch>"
			"<wsa:EndpointReference>"
			"<wsa:Address>urn:uuid:%s</wsa:Address>"
			"</wsa:EndpointReference>"
			"<d:Types>dn:NetworkVideoTransmitter tds:Device</d:Types>", 
			pOnvifInfo->EndpointReference);

	offset += snprintf(send_buffer+offset, buff_len-offset, "<d:Scopes>");

	for (i = 0; i < pOnvifInfo->scopes.fixed_len; i++)
	{
		if (pOnvifInfo->scopes.fixed_scopes[i][0] != '\0')
		{
			offset += snprintf(send_buffer+offset, buff_len-offset, "%s ", pOnvifInfo->scopes.fixed_scopes[i]);
		}
	}
	for (i = 0; i < pOnvifInfo->scopes.add_len; i++)
	{
		if (pOnvifInfo->scopes.addtional_scopes[i][0] != '\0')
		{
			offset += snprintf(send_buffer+offset, buff_len-offset, "%s ", pOnvifInfo->scopes.addtional_scopes[i]);
		}
	}

	
	offset += snprintf(send_buffer+offset, buff_len-offset, "</d:Scopes>");
	
	char end_point[30];
	
	if(pNetPort->http_port==80){
		sprintf(end_point,"%s",g_onvif_cls.local_ipstr);
	}
	else{
		sprintf(end_point,"%s:%d",g_onvif_cls.local_ipstr,pNetPort->http_port);
	}
	
	offset += snprintf(send_buffer+offset, buff_len-offset, "<d:XAddrs>http://%s/onvif/device_service</d:XAddrs>"
			"<d:MetadataVersion>1</d:MetadataVersion>"
			"</d:ProbeMatch></d:ProbeMatches></s:Body></s:Envelope>",end_point);

	// send to received addr
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = rip;
	addr.sin_port = rport;
	
	rlen = sendto(vfd, send_buffer, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rlen != offset)
	{
		printf("onvif_probe_rly::rlen = %d,slen = %d,ip=0x%08x\r\n", rlen, offset, ntohl(rip));
	}

	// send to multicast addr
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("239.255.255.250");
	addr.sin_port = htons(3702);
	
    rlen = sendto(vfd, send_buffer, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rlen != offset)
	{
		printf("onvif_hello::rlen = %d, slen = %d\r\n", rlen, offset);
	}

	return rlen;

}

int onvif_probe_net_rx(int fd)
{
    int sret;
    int rlen;
    int addr_len;
	char *rbuf = g_probe_buf;
  int buf_size = 1024*10;
	char message_id[128];
	unsigned int src_ip;
	unsigned int src_port;
	fd_set fdr;
	struct timeval tv;
	struct sockaddr_in addr;
	XMLN * p_node;
	
	memset(message_id,0,sizeof(message_id));
	
	FD_ZERO(&fdr);
	FD_SET(fd, &fdr);
	
	tv.tv_sec = 0;
	tv.tv_usec = 100 * 1000;
	
	sret = select(fd+1, &fdr,NULL,NULL,&tv);
	if (sret == 0)
	{
		return 0;
	}
	else if (sret < 0)
	{
		printf("onvif_probe_net_rx::select err[%s]\r\n", sys_os_get_socket_error());
		return -1;
	}		
	
	addr_len = sizeof(struct sockaddr_in);
	rlen = recvfrom(fd, rbuf, buf_size, 0, (struct sockaddr *)&addr, (socklen_t*)&addr_len);
	if (rlen < 0)
	{
		printf("onvif_probe_net_rx::recvfrom err[%s]\r\n", sys_os_get_socket_error());
		return -1;
	}
	
	src_ip = addr.sin_addr.s_addr;
	src_port = addr.sin_port;

	p_node = xxx_hxml_parse(rbuf, rlen);
	if (p_node == NULL)
	{
		printf("hxml parse err!!!\r\n");
	}	
	else
	{
		XMLN * p_envelope = p_node;
		if (p_envelope)
		{
			XMLN * p_header;
            XMLN * p_body;
            
			p_header = xml_node_soap_get(p_envelope, "Header"); 
			if (p_header)
			{
				XMLN * p_message_id = xml_node_soap_get(p_header, "MessageID"); 
				if (p_message_id && p_message_id->data)
				{
					strcpy(message_id, p_message_id->data);
				}
			}

			p_body = xml_node_soap_get(p_envelope, "Body");
			if (p_body)
			{
				XMLN * p_probe = xml_node_soap_get(p_body, "Probe");
				if (p_probe)
				{
					XMLN * p_probe_type = xml_node_soap_get(p_probe, "Types");
					if (p_probe_type && p_probe_type->data)
					{
						if (soap_strcmp(p_probe_type->data, "NetworkVideoTransmitter") == 0)
						{
							printf("discovery NetworkVideoTransmitter[0x%x]\r\n",src_ip);
							
							onvif_probe_rly(message_id, fd, src_ip, src_port);
						}
					}
					else
					{
						printf("discovery NetworkVideoTransmitter[0x%x]\r\n",src_ip);
						
						onvif_probe_rly(message_id, fd, src_ip, src_port);
					}
				}
			}
		}
	}

	xml_node_del(p_node);

	return 0;
}


void onvif_hello(int fd)
{
    int i;    
    int rlen;
    int offset = 0;
    int mlen;
    char p_buf[1024*10];
    struct sockaddr_in addr;

    mlen = sizeof(p_buf);
	Onvif_Info* pOnvifInfo = GetOnvifInfo();
	NetworkPort *pNetPort = GetNetPort();
    offset += snprintf(p_buf+offset, mlen-offset,  
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "		
		"xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" "		
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Header>"
	    "<wsa:MessageID>uuid:%s</wsa:MessageID>"
	    "<wsa:RelatesTo>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:RelatesTo>"
	    "<wsa:To s:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
	    "<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello</wsa:Action>"
		"</s:Header>", onvif_uuid_create());

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Body><d:Hello>"
            "<wsa:EndpointReference>"
                "<wsa:Address>urn:uuid:%s</wsa:Address>"
            "</wsa:EndpointReference>"
            "<d:Types>dn:NetworkVideoTransmitter tds:Device</d:Types>",
            pOnvifInfo->EndpointReference);

   	offset += snprintf(p_buf+offset, mlen-offset, "<d:Scopes>");

	for (i = 0; i < pOnvifInfo->scopes.fixed_len; i++)
	{
		if (pOnvifInfo->scopes.fixed_scopes[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "%s ", pOnvifInfo->scopes.fixed_scopes[i]);
		}
	}
	for (i = 0; i < pOnvifInfo->scopes.add_len; i++)
	{
		if (pOnvifInfo->scopes.addtional_scopes[i][0] != '\0')
		{
			offset += snprintf(p_buf+offset, mlen-offset, "%s ", pOnvifInfo->scopes.addtional_scopes[i]);
		}
	}
	
   	offset += snprintf(p_buf+offset, mlen-offset, "</d:Scopes>");

		char end_point[30];
		if(pNetPort->http_port==80){
			sprintf(end_point,"%s",g_onvif_cls.local_ipstr);
		}
		else{
			sprintf(end_point,"%s:%d",g_onvif_cls.local_ipstr,pNetPort->http_port);
		}
		
   	offset += snprintf(p_buf+offset, mlen-offset, "<d:XAddrs>http://%s/onvif/device_service</d:XAddrs>"
        "<d:MetadataVersion>1</d:MetadataVersion></d:Hello></s:Body></s:Envelope>",
        end_point);
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("239.255.255.250");
	addr.sin_port = htons(3702);
	
    rlen = sendto(fd, p_buf, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if (rlen != offset)
	{
		printf("onvif_hello::rlen = %d, slen = %d\r\n", rlen, offset);
	}
	
}


void * onvif_discovery_thread(void * argv)
{
	onvif_hello(g_onvif_cls.discovery_fd);
	
	while (g_onvif_cls.discovery_flag)
	{
		onvif_probe_net_rx(g_onvif_cls.discovery_fd);

		usleep(1000);
	}

	g_onvif_cls.discovery_tid = 0;

	return NULL;
}

void onvif_start_discovery()
{
	g_onvif_cls.discovery_fd = onvif_probe_init();
	if (g_onvif_cls.discovery_fd < 0)
	{
		printf("onvif_probe_init fd failed\r\n");
		return;
	}

	g_onvif_cls.discovery_flag = 1;
	g_onvif_cls.discovery_tid = sys_os_create_thread((void *)onvif_discovery_thread, NULL);
}

void onvif_stop_discovery()
{
	g_onvif_cls.discovery_flag = 0;
	while (g_onvif_cls.discovery_tid != 0)
	{
		usleep(1000);
	}

	closesocket(g_onvif_cls.discovery_fd);
	g_onvif_cls.discovery_fd = -1;
}



