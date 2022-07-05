/*
 *   Copyright (c) 2014 - 2019 Oleh Kulykov <info@resident.name>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include "nwy_osi_api.h"
#include "nwy_usb_serial.h"
#include "osi_log.h"
#include "nwy_rws_proc.h"
#include "nwy_data.h"
#include "ssl.h"

static nwy_osiThread_t* nwy_test_rws_thread = NULL;
void *_socket = NULL;
void nwy_test_rws_dbg( char* fmt, ... );
#define NWY_RWS_LOG(...) nwy_test_rws_dbg(__VA_ARGS__)
int dataCallFlag = 0;
int handle = 0;
static char recv_buff[512] = {0};

void nwy_test_rws_dbg( char* fmt, ... )
{
  static char buf[1024];
  va_list args;
  int len =0;

  memset(buf, 0,sizeof(buf));
  
  sprintf(buf, "NWY_RWS:");
  len = strlen(buf);
  va_start(args, fmt);
  
  vsnprintf(&buf[len], sizeof(buf)-len-1,fmt, args);
  va_end(args);

  OSI_LOGXI(OSI_LOGPAR_S, 0, "%s\n", buf);
  nwy_usb_serial_send((char *)buf, strlen(buf));
}

static void on_socket_received_bin(void * socket, const void *data, const unsigned int length,void *arg) 
{
  int echo_len;
  memset(recv_buff, 0, 512);
  echo_len = (length < 512)?length:511;
  memcpy(recv_buff, data, length);
	NWY_RWS_LOG("nwy_rws:on_socket_received_bin data =%d %s\n",length, recv_buff);
}

static void on_socket_disconnected(void *socket,int reason, void *arg) 
{
	NWY_RWS_LOG("nwy_rws:on_socket_disconnected reason =%d\n",reason);
}

int nwy_rws_ssl_open_test() 
{
	nwy_rws_opt_t opt;
  nwy_rws_ssl_conf_type ssl_conf;
	char *url="wss://0cz3bk0pt4.execute-api.us-east-2.amazonaws.com:443/production";
	
	memset(&opt,0,sizeof(nwy_rws_opt_t));
  memset(&ssl_conf, 0, sizeof(nwy_rws_ssl_conf_type));

  NWY_RWS_LOG("nwy_rws_ssl_open_test start\n");
	opt.ping_interval = 10;
	opt.ping_timout = 10;
	opt.should_handshake = 1;
	ssl_conf.authmode = MBEDTLS_SSL_VERIFY_NONE;
  ssl_conf.ssl_version = MBEDTLS_SSL_MINOR_VERSION_3;
  opt.ssl_conf = &ssl_conf;

	nwy_sleep(2*1000);

	_socket = nwy_ws_open(url,&opt,on_socket_received_bin,on_socket_disconnected,NULL);
	if(_socket == NULL)
	{
		NWY_RWS_LOG("nwy_rws_ssl_open_test failed\n");
		return 0;
	}
  NWY_RWS_LOG("nwy_rws_ssl_open_test OK\n");

	return 0;
}

int nwy_rws_open_test() 
{
	nwy_rws_opt_t opt;
	char *url="ws://123.207.136.134:9010/ajaxchattest";
	
	memset(&opt,0,sizeof(nwy_rws_opt_t));
	
	opt.ping_interval = 10;
	opt.ping_timout = 10;
	opt.should_handshake = 1;
	opt.origin = "coolaf.com";
	opt.key = "u0T2ZP/HrojFljdf1FXmpA==";

	nwy_sleep(2*1000);

	_socket = nwy_ws_open(url,&opt,on_socket_received_bin,on_socket_disconnected,NULL);
	if(_socket == NULL)
	{
		NWY_RWS_LOG("nwy_ws_open failed\n");
		return 0;
	}

	return 0;
}

void data_call_cb(   int hndl,nwy_data_call_state_t ind_state)
{
	if(ind_state == NWY_DATA_CALL_CONNECTED)
	{
		NWY_RWS_LOG(" data call success\n ");
		dataCallFlag  = 1;
	}
}
 static void start_data_call()
{
	nwy_data_start_call_v02_t dataCallPara;
	
	memset(&dataCallPara,0,sizeof(nwy_data_start_call_v02_t));
	nwy_sleep(10*1000);
	handle = nwy_data_get_srv_handle(data_call_cb);
	dataCallPara.is_auto_recon = 0;
	dataCallPara.profile_idx = 1;
	nwy_data_start_call(handle,&dataCallPara);
	while(dataCallFlag == 0)
	{
	  nwy_sleep(2*1000);
	}

	return ;
}
static void nwy_test_rws_main_func(void *param)
{
  start_data_call();
  nwy_rws_ssl_open_test();

  while(1)
  {
    nwy_sleep(10*1000);
	//nwy_ws_send_binary(_socket,"1234",4);
	nwy_rws_send_text(_socket,"{\"action\":\"message\"}");
  }

  nwy_rws_close(_socket);
  nwy_data_stop_call(handle);
  nwy_data_relealse_srv_handle(handle);
  
}

int appimg_enter(void *param)
{
  nwy_sleep(10*1000);
  nwy_test_rws_thread = nwy_create_thread("test-rws", nwy_test_rws_main_func, NULL, NWY_OSI_PRIORITY_NORMAL, 1024*10, 0);
  return 0;
}

void appimg_exit(void)
{
}
