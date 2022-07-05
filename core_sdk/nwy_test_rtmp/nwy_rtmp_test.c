/*  RTMPDump
 *  Copyright (C) 2009 Andrej Stepanchuk
 *  Copyright (C) 2009 Howard Chu
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RTMPDump; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

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
#include "nwy_data.h"
#include "librtmp/log.h"
#include "librtmp/rtmp.h"
#include "osi_log.h"

static nwy_osiThread_t* nwy_test_rtmp_thread = NULL;
int dataCallFlag = 0;
int handle = 0;
 RTMP g_rtmp;

 void data_call_cb(   int hndl,nwy_data_call_state_t ind_state)
 {
	 if(ind_state == NWY_DATA_CALL_CONNECTED)
	 {
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

static void   nwy_rtmp_open_test()
{
	char *url = "http://123.139.59.166:12018/";
	RTMPPacket rtmpPacket;
	
	memset(&g_rtmp,0,sizeof(RTMP));
	memset(&rtmpPacket,0,sizeof(RTMPPacket));
	
	RTMP_LogSetLevel(RTMP_LOGINFO);
	 
	RTMP_Init(&g_rtmp);

	RTMP_SetupURL(&g_rtmp,url);

	RTMP_EnableWrite(&g_rtmp);

	RTMP_Connect(&g_rtmp,&rtmpPacket);
	
	memset(&rtmpPacket,0,sizeof(RTMPPacket));

	RTMP_SendPacket(&g_rtmp,&rtmpPacket,1);

}
static void nwy_test_rtmp_main_func(void *param)
{
	RTMP_LogPrintf("RTMP:task start_data_call\n");
	start_data_call();
	RTMP_LogPrintf("RTMP:nwy data call success\n");
	nwy_rtmp_open_test();
 	RTMP_Log(RTMP_LOGDEBUG,"RTMP:test\n");
 	RTMP_LogPrintf("RTMP:nwy test\n");

  while(1)
  {
    nwy_sleep(10*1000);

  }

  nwy_data_stop_call(handle);
  nwy_data_relealse_srv_handle(handle);
  
}

int appimg_enter(void *param)
{
  nwy_sleep(10*1000);
  nwy_test_rtmp_thread = nwy_create_thread("test-rtmp", nwy_test_rtmp_main_func, NULL, NWY_OSI_PRIORITY_NORMAL, 1024*10, 0);
  return 0;
}

void appimg_exit(void)
{
}

