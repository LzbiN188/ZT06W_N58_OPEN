/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */
/*add by neoway start*/
#include "nwy_osi_api.h"
#include "nwy_config.h"
#ifdef FEATURE_NWY_AT_HX_GNSS
#include "nwy_loc_hx.h"
#else
#include "nwy_loc.h"
#endif
#include "nwy_voice.h"
#include "nwy_sim.h"
#include "nwy_wifi.h"
#include "nwy_vir_at.h"
#include "osi_log.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "nwy_sms.h"
#include "nwy_data.h"
#include "nwy_usb_serial.h"
#include "nwy_adc.h"
#include "nwy_led.h"
#include "nwy_gpio.h"
#include "nwy_gpio_open.h"
#include "nwy_fota.h"
#include "nwy_fota_api.h"
#include "nwy_uart.h"
#include "nwy_spi.h"
#include "nwy_i2c.h"
#include "nwy_pm.h"
#include "nwy_keypad.h"
#include "stdio.h"
#include "nwy_network.h"
#include "nwy_file.h"
#include "nwy_audio_api.h"
#include "nwy_socket.h"
//#include "nwy_alimqtt.h"
#include "mqtt_api.h"
#include "nwy_osi_api.h"
#include "nwy_ftp.h"
#include "nwy_http.h"
#include "MQTTClient.h"
#include "nwy_mqtt.h"
/* added by wangchen for N58 ussd api to test 20200826 begin */
#include "nwy_ussd.h"
/* added by wangchen for N58 ussd api to test 20200826 end */
#include "nwy_pwm.h"
#include "nwy_dm.h"
#include "nwy_ble.h"
#ifdef FEATURE_NWY_OPEN_ZZD_SDK
#include "nwy_oem_zzd_api.h"
#endif
#include "nwy_ip_packet.h"
#include "nwy_poc_dsds.h"
/* added by wangchen for stk openapi 2021.08.26 begin */
#ifdef FEATURE_NWY_STK_COMMAND
#include "nwy_sat.h"
#endif
/* added by wangchen for stk openapi 2021.08.26 end */
#include "nwy_camera.h"
#ifdef FEATURE_NEOWAY_PIPECLOUD
#include "nwy_pipecloud.h"
#endif

/*Add Macro for Network Test*/
#define FEATURE_NETWORK_TEST
extern int vsnprintf (char *, size_t, const char *, va_list);

#define NWY_EXT_SIO_RX_MAX          (2 * 1024)
#define NWY_EXT_INPUT_RECV_MSG       (NWY_APP_EVENT_ID_BASE + 1)
#define NWY_EXT_FOTA_DATA_REC_END      (NWY_APP_EVENT_ID_BASE + 2)
#define NWY_EXT_APPIMG_FOTA_DATA_REC_END	(NWY_APP_EVENT_ID_BASE + 3)


#define NWY_NTP_BASE_TIME 0xBC191380

#define NWY_APPIMG_FOTA_BLOCK_SIZE  (2*1024)
#define CERT_EFS_CHECKSUM_STR_SIZE 4
#define RS485_GPIO_PORT   	(2)
#define RS485_DIR_TX        (1)   //hight level for send
#define RS485_DIR_RX        (0)   //low level for recv
#define NWY_UART_RECV_SINGLE_MAX   512
#define NWY_FILE_NAME_MAX          32

typedef struct dataRecvContext
{
    unsigned size;
    unsigned pos;
    char data[0];
} dataRecvContext_t;

typedef struct nwy_file_ftp_info_s
{
  int is_vaild;
  char filename[256];
  int pos;
  //int length;
  //int file_size;
}nwy_file_ftp_info_s;
typedef enum
{
  NWY_CUSTOM_IP_TYPE_OR_DNS_NONE = -1,
  NWY_CUSTOM_IP_TYPE_OR_DNS_IPV4 = 0,
  NWY_CUSTOM_IP_TYPE_OR_DNS_IPV6 = 1,
  NWY_CUSTOM_IP_TYPE_OR_DNS_DNS = 2
}nwy_ip_type_or_dns_enum;

char nwy_ext_sio_recv_buff[NWY_EXT_SIO_RX_MAX + 1] = {0};
static nwy_osiMessageQueue_t *nwy_usb_con_rd_mq = NULL;
static int nwy_ext_sio_len = 0;

nwy_osiThread_t *g_app_thread = NULL;
nwy_osiTimer_t *g_timer = NULL;
static uint8 nwy_update_flag = 0;
dataRecvContext_t *g_recv = NULL;
static nwy_file_ftp_info_s g_fileftp;
nwy_osiThread_t *tcp_recv_thread = NULL;
nwy_osiThread_t *udp_recv_thread = NULL;
static int tcp_connect_flag = 0;
static int udp_connect_flag = 0;

/*Begin by zhaoyating to control tripple sim 20220615*/
#define MAX_INPUT_OPTION 4 /* contain '\r' '\n' */
#define STD_SUCCESS 1
#define STD_ERROR 0
#define NWY_IS_DIGIT( x )  ( ( ((x -'0') >= 0) && ((x-'0') <= 9) ) ? 1 : 0)
#define NWY_IS_HEXSTR( x )  ( ( (((x -'0') >= 0) && ((x-'0') <= 9)) || \
                                  ((x >= 'a') && (x <= 'f')) || \
                                  ((x >= 'A') && (x <= 'F'))) ? 1 : 0)

/*End by zhaoyating to control tripple sim 20220615*/

#if 0
typedef struct
{
  size_t size;
  void  *data;
} nwy_usb_con_recv_msg_t;

static int nwy_usb_con_recv_cb(void *data, size_t size)
{
  nwy_usb_con_recv_msg_t msg;
  if(!nwy_usb_con_rd_mq)
    return 0;
  msg.data = malloc(size);
  if(!msg.data)
    return 0;
  memcpy(msg.data, data, size);
  msg.size = size;
  if(!nwy_put_msg_que(nwy_usb_con_rd_mq, &msg, 8))
  {
    free(msg.data);
    return 0;
  }
  nwy_ext_send_sig(g_app_thread,NWY_EXT_INPUT_RECV_MSG);
  return size;
}

void nwy_usb_con_init(void)
{
  nwy_usb_con_rd_mq = nwy_create_msg_Que(32, sizeof(nwy_usb_con_recv_msg_t));
  nwy_usb_serial_reg_recv_cb(nwy_usb_con_recv_cb);
}

void nwy_usb_con_printf(const char *format, ...)
{
    char send_buf[512];
    va_list ap;
    size_t size;

    va_start(ap, format);
    size = vsnprintf(send_buf, sizeof(send_buf), format, ap);
    va_end(ap);

    nwy_usb_serial_send(send_buf, size);
}

size_t nwy_usb_con_recv(void *buf, size_t size, uint32 timeout)
{
  nwy_usb_con_recv_msg_t msg;
  if(!nwy_usb_con_rd_mq || !buf || !size)
    return 0;

  if(!nwy_get_msg_que(nwy_usb_con_rd_mq, &msg, timeout))
    return 0;

  if(size > msg.size)
    size = msg.size;
  memcpy(buf, msg.data, size);
  free(msg.data);

  return size;
}
#endif
#if 0

//add by yinjianjun in 2019.12.10
static int ppp_state[10] = {0};
static void nwy_data_cb_fun(
    int hndl,
    nwy_data_call_state_t ind_state)
{
  OSI_LOGI(0, "=DATA= hndl=%d,ind_state=%d", hndl,ind_state);
  if (hndl > 0 && hndl <= 8)
    ppp_state[hndl-1] = ind_state;
}

static void nwy_data_test()
{
   for (int n = 0; n < 10; n++)
    {
        OSI_LOGI(0, "hello world %d", n);
        nwy_sleep(2000);
    }
    int hndl = nwy_data_get_srv_handle(nwy_data_cb_fun);
    OSI_LOGI(0, "=DATA=  hndl= %d", hndl);

    nwy_data_profile_info_t profile_info;
    memset(&profile_info,0,sizeof(nwy_data_profile_info_t));
    profile_info.auth_proto = NWY_DATA_AUTH_PROTO_NONE;
    profile_info.pdp_type = NWY_DATA_PDP_TYPE_IPV4;
    memcpy(profile_info.apn,"3gnet",sizeof(profile_info.apn));
    memcpy(profile_info.user_name,"card",sizeof(profile_info.user_name));
    memcpy(profile_info.pwd,"card",sizeof(profile_info.pwd));
    int ret = nwy_data_set_profile(1,NWY_DATA_PROFILE_3GPP,&profile_info);
    OSI_LOGI(0, "=DATA=  nwy_data_set_profile ret= %d", ret);

    memset(&profile_info,0,sizeof(nwy_data_profile_info_t));
    ret = nwy_data_get_profile(1,NWY_DATA_PROFILE_3GPP,&profile_info);
    OSI_LOGI(0, "=DATA=  nwy_data_get_profile ret= %d", ret);
    OSI_LOGI(0, "=DATA=  profile= %d|%d", profile_info.pdp_type,profile_info.auth_proto);
    OSI_PRINTFI("=DATA=   profile= %s|%s|%s", profile_info.apn,profile_info.user_name,profile_info.pwd);

    nwy_data_start_call_v02_t param_t;
    param_t.profile_idx = 1;
    ret = nwy_data_start_call(hndl,&param_t);
    OSI_LOGI(0, "=DATA=  nwy_data_start_call ret= %d", ret);

    int query_cnt = 0;
    while(query_cnt < 200)
    {
      if (ppp_state[hndl-1] == NWY_DATA_CALL_CONNECTED)
        break;
      query_cnt++;
      nwy_sleep(100);
      OSI_LOGI(0, "=DATA=  query_cnt= %d", query_cnt);
    }


    int len = 0;
    nwy_data_addr_t_info addr_info;
    memset(&addr_info,0,sizeof(nwy_data_addr_t_info));
    OSI_LOGI(0, "=DATA=  addr_info size= %d",sizeof(nwy_data_addr_t_info));
    ret = nwy_data_get_ip_addr(hndl, &addr_info, &len);
    OSI_LOGI(0, "=DATA=  nwy_data_get_ip_addr = %d|len%d", ret,len);

    ret = nwy_data_stop_call(hndl);
    OSI_LOGI(0, "=DATA=  nwy_data_stop_call ret= %d", ret);
    query_cnt = 0;
    while(query_cnt < 200)
    {
      if (ppp_state[hndl-1] == NWY_DATA_CALL_DISCONNECTED)
        break;
      query_cnt++;
      nwy_sleep(100);
      OSI_LOGI(0, "=DATA=  query_cnt= %d", query_cnt);
    }

    nwy_data_relealse_srv_handle(hndl);
    OSI_LOGI(0, "=DATA=  nwy_data_relealse_srv_handle hndl= %d", hndl);
}


void nwy_at_rsp_process(char *buf, int len)
{
    if (buf == NULL) {
        OSI_LOGI(0,"nwy buf is null");
    }
    OSI_LOGI(0, "nwy nwy_at_rsp_process len = %d",len);

    OSI_LOGXI(OSI_LOGPAR_S, 0,"hj nwy_at_rsp_process resp = %s ", buf);
}

static void nwy_at_test()
{
    int ret = -1;
    nwy_at_info at_cmd;
    nwy_sdk_at_parameter_init();
    nwy_sdk_at_rsp_reg(nwy_at_rsp_process);
    nwy_sleep(500);

    memset(&at_cmd, 0, sizeof(nwy_at_info));
    at_cmd.length = strlen("AT+XIIC?\r\n");
    memcpy(at_cmd.at_command, "AT+XIIC?\r\n",at_cmd.length);
    ret = nwy_sdk_at_cmd_send(&at_cmd);
    OSI_LOGI(0, "hj ret = %d",ret);
    nwy_sleep(500);

    memset(&at_cmd, 0, sizeof(nwy_at_info));
    at_cmd.length = strlen("AT+CPIN?\r\n");
    memcpy(at_cmd.at_command, "AT+CPIN?\r\n",at_cmd.length);
    ret = nwy_sdk_at_cmd_send(&at_cmd);
    OSI_LOGI(0, "hj ret = %d",ret);
    nwy_sleep(500);

}

void nwy_at_sim_test(void)
{
  OSI_LOGI(0,"=UIM= nwy_at_sim_test coming in");
  nwy_sim_result_type sim = {"","","1234","1234",NWY_SIM_AUTH_NULL};
  nwy_result_type result;
  nwy_sim_status sim_status;

  //memset(sim.lock_sim_password,0,sizeof(sim.lock_sim_password));
  //memcpy(sim.lock_sim_password,"1234",strlen("1234"));
  nwy_sleep(5*1000);
  sim_status = nwy_sim_get_card_status();
  OSI_LOGI(0,"=UIM= sim status : %d", sim_status);
  result = nwy_sim_get_iccid(&sim);
  OSI_LOGI(0,"=UIM= iccid result : %d", result);
  result = nwy_sim_get_imsi(&sim);
  OSI_LOGI(0,"=UIM= imsi result : %d", result);
  nwy_sleep(10*1000);
  OSI_LOGXI(OSI_LOGPAR_S, 0, "=UIM= iccid 3 : %s\n", sim.iccid);
  OSI_LOGXI(OSI_LOGPAR_S, 0, "=UIM= imsi 3 : %s\n", sim.imsi);
  OSI_LOGXI(OSI_LOGPAR_S, 0, "=UIM= lock pass : %s\n", sim.lock_sim_password);
  OSI_LOGXI(OSI_LOGPAR_S, 0, "=UIM= unlock pass : %s\n", sim.unlock_sim_password);
  result = nwy_sim_enable_pin(&sim);
  result = nwy_sim_get_pin_mode(&sim);
  nwy_sleep(30*1000);
  result = nwy_sim_disable_pin(&sim);
  result = nwy_sim_get_pin_mode(&sim);
  OSI_LOGI(0,"=UIM= nwy_at_sim_test coming out");
}

//add by lipengchao in 2019.12.10
void nwy_at_sms_test(void)
{
  nwy_sleep(30*1000);
  nwy_sms_result_type sms = {""};
  nwy_sms_get_sca(&sms);
  OSI_LOGXI(OSI_LOGPAR_S, 0, "=SMS= sca 3 : %s\n", sms.sca);
}
#endif

void nwy_ext_send_sig(nwy_osiThread_t *thread,uint16 sig)
{
    nwy_osiEvent_t event;

    memset(&event, 0, sizeof(event));
    event.id = sig;
    nwy_send_thead_event(thread, &event, 0);
}

void nwy_ext_echo(char* fmt, ...)
{
    static char    echo_str[NWY_EXT_SIO_RX_MAX];
	static nwy_osiMutex_t *echo_mutex = NULL;
    va_list     a;
	int i, size;

	if(NULL == echo_mutex)
		echo_mutex = nwy_create_mutex();
	if(NULL == echo_mutex)
		return;
	nwy_lock_mutex(echo_mutex, 0);
    va_start(a, fmt);
    vsnprintf(echo_str, NWY_EXT_SIO_RX_MAX, fmt, a);
    va_end(a);
	size = strlen((char *)echo_str);
	i = 0;
	while(1)
    {
    	int tx_size;

    	tx_size = nwy_usb_serial_send((char *)echo_str + i, size - i);
		if(tx_size <= 0)
			break;
		i += tx_size;
		if((i < size))
			nwy_sleep(10);
		else
			break;
	}
	nwy_unlock_mutex(echo_mutex);
}
#if 0
static void nwy_ext_input_gets(char* msg, ...)
{
    int size = 0;
    static char echo_str[NWY_EXT_SIO_RX_MAX] = {0};
    nwy_osiEvent_t event;

    va_list     a;
    va_start(a, msg);
    vsprintf(echo_str, msg, a);
    va_end(a);
    nwy_usb_serial_send(echo_str, strlen((char *)echo_str));
    memset(nwy_ext_sio_recv_buff, 0, NWY_EXT_SIO_RX_MAX);
    while(1)
    {
        memset(&event, 0, sizeof(event));
        nwy_wait_thead_event(g_app_thread, &event, 0);
        nwy_ext_echo("\r\n hj event = %d", event.id);
        if (event.id == NWY_EXT_INPUT_RECV_MSG) {
            size = nwy_usb_con_recv(nwy_ext_sio_recv_buff, NWY_EXT_SIO_RX_MAX, 0);
            if (size) {
                nwy_ext_sio_len = size;
                nwy_usb_con_printf("\r\n%s", nwy_ext_sio_recv_buff);
            }
            return;
        }
    }
}
#endif
void nwy_ext_input_gets(char* msg, ...)
{
    static char echo_str[NWY_EXT_SIO_RX_MAX] = {0};
    nwy_osiEvent_t event;

    memset(&event, 0, sizeof(event));
    va_list     a;
    va_start(a, msg);
    vsprintf(echo_str, msg, a);
    va_end(a);
    nwy_usb_serial_send(echo_str, strlen((char *)echo_str));
    while(1)
    {
        memset(&event, 0, sizeof(event));
        nwy_wait_thead_event(g_app_thread, &event, 0);
        if (event.id == NWY_EXT_INPUT_RECV_MSG){
            return;
        }
    }
}
void nwy_ext_sio_data_cb(const char *data,uint32 length)
{
    static int len = 0;
    OSI_LOGI(0,"\r\nnwy length = %d len = %d", length, len);
    if (nwy_update_flag == 0) {
        if (length == 2 && data[length-1] == '\n' && data[length-2] == '\r') {
            OSI_LOGI(0, "\r\ndrop \\r\\n and input end nwy_ext_sio_len = %d len = %d", nwy_ext_sio_len,len);
            if (len == 0) {
                memset(nwy_ext_sio_recv_buff, 0, NWY_EXT_SIO_RX_MAX);
                nwy_ext_sio_len = 0;
            }
            len = 0;
            nwy_ext_echo("\r\n%s", nwy_ext_sio_recv_buff);
            nwy_ext_send_sig(g_app_thread, NWY_EXT_INPUT_RECV_MSG);
            return;
        }
        // clear the nwy_ext_sio_recv_buff,in next time
        if (len == 0) {
            OSI_LOGI(0, "\r\nnwy reset buff");
            nwy_ext_sio_len = 0;
            memset(nwy_ext_sio_recv_buff, 0, NWY_EXT_SIO_RX_MAX);
        }
        if (len + length > NWY_EXT_SIO_RX_MAX) {
            memcpy(nwy_ext_sio_recv_buff + len, data, (NWY_EXT_SIO_RX_MAX- len));
            nwy_ext_sio_len = nwy_ext_sio_len + (NWY_EXT_SIO_RX_MAX- len);
            len = 0;
            if(nwy_ext_sio_recv_buff[nwy_ext_sio_len - 1] == '\r')
                nwy_ext_sio_len --;
            nwy_ext_sio_recv_buff[nwy_ext_sio_len] = '\0';
            nwy_ext_echo("\r\n%s", nwy_ext_sio_recv_buff);
            nwy_ext_send_sig(g_app_thread, NWY_EXT_INPUT_RECV_MSG);
            return;
        } else {
            memcpy(nwy_ext_sio_recv_buff + len, data, length);

            OSI_LOGXI(OSI_LOGPAR_S, 0, "nwy:%s", nwy_ext_sio_recv_buff);
            if (length > 1) {
                OSI_LOGI(0, "nwy: data[length-1] = %d data[length-2] = %d", data[length-1], data[length-2]);
                if(data[length-1] == '\n' && data[length-2] == '\r') {
                    nwy_ext_sio_recv_buff[len+length-2] = '\0';
                    nwy_ext_sio_len = nwy_ext_sio_len +length -2;
                    OSI_LOGI(0, "nwy: recv over len = %d nwy_ext_sio_len = %d", len, nwy_ext_sio_len);
                    len = 0;

                    nwy_ext_echo("\r\n%s", nwy_ext_sio_recv_buff);
                    nwy_ext_send_sig(g_app_thread, NWY_EXT_INPUT_RECV_MSG);
                    return;
                } else{
                    nwy_ext_sio_len = nwy_ext_sio_len + length;
                }
            } else if (length == 1) {
                if(data[length-1] == '\r' || data[length-1] == '\n') {
                    nwy_ext_sio_recv_buff[nwy_ext_sio_len-1] = '\0';
                    nwy_ext_sio_len = nwy_ext_sio_len -1;
                    len = 0;
                    nwy_ext_echo("\r\n%s", nwy_ext_sio_recv_buff);
                    nwy_ext_send_sig(g_app_thread, NWY_EXT_INPUT_RECV_MSG);
                    return;
                } else {
                    memcpy(nwy_ext_sio_recv_buff + len, data, length);
                    nwy_ext_sio_len = nwy_ext_sio_len + length;
                }
            }
        }
        len = len + length;

    }
    else if (nwy_update_flag == 1)
    {
        nwy_ext_app_info_proc(data, length);
    }
    else if (nwy_update_flag == 2)
    {
		nwy_ext_appimg_info_proc(data, length);
    }
}

void nwy_ext_main_menu()
{

    nwy_ext_echo("\r\nPlease select an option to test from the items listed below. \r\n");
    nwy_ext_echo("1. Wifi test \r\n");
    nwy_ext_echo("2. Virtual AT test \r\n");
    nwy_ext_echo("3. SIM test \r\n");
    nwy_ext_echo("4. Data test \r\n");
    nwy_ext_echo("5. Network test \r\n");
    nwy_ext_echo("6. Voice test \r\n");
    nwy_ext_echo("7. Sms test \r\n");
    nwy_ext_echo("8. Location test \r\n");
    nwy_ext_echo("9. Fota test \r\n");
    nwy_ext_echo("10. Uart test \r\n");
    nwy_ext_echo("11. I2C test \r\n");
    nwy_ext_echo("12. SPI test \r\n");
    nwy_ext_echo("13. GPIO test \r\n");
    nwy_ext_echo("14. ADC test \r\n");
    nwy_ext_echo("15. PM test \r\n");
    nwy_ext_echo("16. Audio test \r\n");
    nwy_ext_echo("17. LED backlight test \r\n");
    nwy_ext_echo("18. keypad test \r\n");
    nwy_ext_echo("19. device info test \r\n");
    nwy_ext_echo("20. file write and read test \r\n");
    nwy_ext_echo("21. tcp test \r\n");
    nwy_ext_echo("22. udp test \r\n");
    nwy_ext_echo("23. tts test \r\n");
    nwy_ext_echo("24. dtmf test \r\n");
    nwy_ext_echo("25. close powerkey shutdown test \r\n");
    nwy_ext_echo("26. set at callback and used by app\r\n");
    nwy_ext_echo("27. set date and time\r\n");
    nwy_ext_echo("28. get date and time\r\n");
    nwy_ext_echo("29. LCD test\r\n");
    nwy_ext_echo("30. Timer test\r\n");
    nwy_ext_echo("31. DM test\r\n");
#ifndef FEATURE_NWY_OPEN_ALIOS
    nwy_ext_echo("32. alimqtt test\r\n");
    nwy_ext_echo("33. alimqtt pub\r\n");
    nwy_ext_echo("34. alimqtt sub\r\n");
    nwy_ext_echo("35. alimqtt connect states\r\n");
    nwy_ext_echo("36. alimqtt disconnect\r\n");
    nwy_ext_echo("37. alimqtt unsub\r\n");
#endif
    nwy_ext_echo("38. appimg fota test\r\n");
    nwy_ext_echo("39. semaphore test\r\n");
    nwy_ext_echo("40. send hex string test\r\n");
    nwy_ext_echo("41. ftp test\r\n");
    nwy_ext_echo("42. http/https test\r\n");
    nwy_ext_echo("43. pm charger test\r\n");
    nwy_ext_echo("44. sdcard test\r\n");
    nwy_ext_echo("45. read sdcard status test\r\n");
    nwy_ext_echo("46. ble test\r\n");
    nwy_ext_echo("47. get chip id test\r\n");
    nwy_ext_echo("48. read&write flash test\r\n");
    nwy_ext_echo("49. multiplex http test\r\n");
    nwy_ext_echo("50. multiplex ftp test\r\n");
    nwy_ext_echo("51. get device ver test\r\n");
    nwy_ext_echo("52. get boot causes test\r\n");
    nwy_ext_echo("53. paho mqtt test\r\n");
    nwy_ext_echo("54. rs485 test\r\n");
    nwy_ext_echo("55. ussd test\r\n");
    nwy_ext_echo("56. pwm test\r\n");
    nwy_ext_echo("57. Time Zone test\r\n");
    nwy_ext_echo("58. nwy_audio_file_play_test\r\n");
#ifdef FEATURE_NWY_OPEN_ZZD_SDK
    nwy_ext_echo("59. zzd oem poc test\r\n");
#endif
    nwy_ext_echo("60. nwy_audio_change_channel_test \r\n");
    nwy_ext_echo("61. spi flash test\r\n");
#ifdef FEATURE_NWY_OPEN_LITE_AZURE
    nwy_ext_echo("62. nwy azure test\r\n");
#endif
    nwy_ext_echo("63. nwy cert test\r\n");
nwy_ext_echo("65. nwy ip packet test\r\n");
    nwy_ext_echo("66. nwy multi socket test\r\n");
    nwy_ext_echo("67. lwm2m teest\r\n");
    nwy_ext_echo("68. nwy_hp_type_test\r\n");
    nwy_ext_echo("69. nwy audio file record test\r\n");
    nwy_ext_echo("71. send hex\r\n");
    nwy_ext_echo("72. nwy_set_poc_sampleRate_test\r\n");
    nwy_ext_echo("73. aws mqtt test\r\n");
    nwy_ext_echo("74. nwy poweroff charge test\r\n");
    nwy_ext_echo("75. rhino ids test\r\n");
    nwy_ext_echo("76. poc dsds test\r\n");
    nwy_ext_echo("77. nwy file play async test\r\n");
#ifdef FEATURE_NWY_STK_COMMAND
    nwy_ext_echo("78. nwy stk test\r\n");
#endif
    nwy_ext_echo("79. nwy camera test\r\n");
    nwy_ext_echo("80. nwy pipecloud test\r\n");
    nwy_ext_echo("81. nwy auto power on test\r\n");
}

void nwy_wifi_test()
{
    int i;
	int ret;
    nwy_wifi_scan_list_t scan_list;

    memset(&scan_list, 0, sizeof(scan_list));

	nwy_ext_echo("enter wifi test.\r\n ");

#ifdef FEATURE_NWY_OPEN_ALIOS
	ret = nwy_wifi_scan_aliba(&scan_list, UART_PORT_ID);
#else
	ret = nwy_wifi_scan(&scan_list);
#endif

	if( 0 == ret )
	{
		if ( 0 == scan_list.num )
		{
			nwy_ext_echo("\r\n Wifi scan nothing");
		}
		else
		{
			for (i = 0; i< scan_list.num; i++)
			{
		        OSI_LOGI(0, "nwy wifi ap mac is %02x:%02x:%02x:%02x:%02x:%02x",
		        scan_list.ap_list[i].mac[5],scan_list.ap_list[i].mac[4],scan_list.ap_list[i].mac[3],scan_list.ap_list[i].mac[2],
		        scan_list.ap_list[i].mac[1],scan_list.ap_list[i].mac[0]);
		        OSI_LOGI(0, "nwy channel = %d", scan_list.ap_list[i].channel);
		        OSI_LOGI(0, "nwy rssi = %d", scan_list.ap_list[i].rssival);

#ifdef FEATURE_NWY_OPEN_ALIOS
				nwy_ext_echo("\r\nwifi ap ssid is %s", scan_list.ap_list[i].ssid);
#endif
		        nwy_ext_echo("\r\nwifi ap mac is %02x:%02x:%02x:%02x:%02x:%02x",
		        scan_list.ap_list[i].mac[5],scan_list.ap_list[i].mac[4],scan_list.ap_list[i].mac[3],scan_list.ap_list[i].mac[2],
		        scan_list.ap_list[i].mac[1],scan_list.ap_list[i].mac[0]);
		        nwy_ext_echo("\r\nchannel = %d", scan_list.ap_list[i].channel);
		        nwy_ext_echo("\r\nrssi = %d", scan_list.ap_list[i].rssival);
			}
		}
	}
}

static void nwy_pull_out_sim()
{
    nwy_ext_echo("\r\n SIM pull out");
}

/* added by wangchen for N58 sms api to test 20200215 begin */
static void nwy_recv_sms()
{
    nwy_sms_recv_info_type_t sms_data = {0};

    nwy_sms_recv_message(&sms_data);

    if(1 == sms_data.cnmi_mt)
    {
        nwy_ext_echo("\r\n recv one sms index:%d",sms_data.nIndex);
    }
	else if(2 == sms_data.cnmi_mt)
	{
	    nwy_ext_echo("\r\n recv one sms from:%s msg_context:%s",sms_data.oa,sms_data.pData);
	}
}
/* added by wangchen for N58 sms api to test 20200215 end */
static void nwy_incom_voice()
{
	char string_clip[128] ={0};
	nwy_get_voice_callerid(string_clip);
    nwy_ext_echo("\r\n %s",string_clip);
}

static void nwy_hex_to_string(int len , char *data)
{
    int i = 0;
    unsigned char highByte;
    unsigned char lowByte;
    char *tmp_buf =(char *)malloc(len*2+1);
    if(NULL== tmp_buf)
    {
        OSI_LOGI(0,"tcprecv malloc fail");
        return;
    }
    memset(tmp_buf, 0, (len*2+1));
   //16����ת�ַ���
    for (i = 0; i < len; i++)
    {
      highByte = data[i] >> 4;
      lowByte =  data[i] & 0x0f ;

      highByte += 0x30;

      if (highByte > 0x39)
        tmp_buf[i * 2] = highByte + 0x07;
      else
        tmp_buf[i * 2] = highByte;

      lowByte += 0x30;
      if (lowByte > 0x39)
        tmp_buf[i * 2 + 1] = lowByte + 0x07;
      else
        tmp_buf[i * 2 + 1] = lowByte;
    }
   nwy_ext_echo(tmp_buf);
}
static nwy_recv_data_print(char *data)
{
    int len = 0;
    int socktid = 0;
    char buf[64] = {0};
    int i = 0;
    int count = 0;
    char *data_info = NULL;
    while (1) {
        if (data[i] == ',') {
            count++;
            if (count == 2) {
                break;
            }
        }
        i++;
    }
    memcpy(buf, data, i);
    OSI_LOGI(0, "nwy buf = %s",buf);
    socktid = atoi(strtok(buf,","));
    len = atoi(strtok(NULL, ","));


    data_info = data +i +1;
    OSI_LOGI(0, "nwy recv socktid = %d len = %d data_len = %d",socktid, len, strlen(data_info));
    if (data_info  == NULL) {
        OSI_LOGI(0, "nwy recv NULL");
        return;
    }

    for (int i = 0; i< len; i++) {
        OSI_LOGI(0, "%02x", data_info[i]);
    }
    nwy_ext_echo("%d,%d,", socktid, len);
    OSI_LOGI(0, "data_info[strlen(data_info) -2] = %d data_info[strlen(data_info)-1] = %d",data_info[strlen(data_info) -2],data_info[strlen(data_info)-1]);
    if (data_info[strlen(data_info) -2] == '\r' && data_info[strlen(data_info)-1] == '\n') {
        nwy_ext_echo(data_info);
    }else {
        nwy_hex_to_string(len,data_info);
    }
}
static void nwy_voice_ind()
{
	char state[64] ={0};
	nwy_get_voice_state(state);
    nwy_ext_echo("\r\n %s",state);
}
void nwy_ext_tcpsetup_cb(char* data)
{
    nwy_ext_echo("\r\n+TCPSETUP: ");
    nwy_ext_echo("%s",data);
}
void nwy_ext_tcprecvs_cb(char* data)
{
    nwy_ext_echo("\r\n+TCPRECV(S): ");
    nwy_recv_data_print(data);
}
void nwy_ext_tcp_close_cb(char* data)
{
    nwy_ext_echo("\r\n+TCPCLOSE:");
    nwy_ext_echo(data);
}

void nwy_ext_tcprecv_cb(char* data)
{
    nwy_ext_echo("\r\n+TCPRECV: ");
    nwy_recv_data_print(data);
}
void nwy_ext_acpt_close_cb(char* data)
{
    nwy_ext_echo("\r\n+CLOSECLIENT: ");
    nwy_ext_echo(data);
}

void nwy_ext_udprecv_cb(char* data)
{
    nwy_ext_echo("\r\n+UDPRECV: ");
    nwy_recv_data_print(data);
}

void nwy_ext_udpsetup_cb(char* data)
{
    nwy_ext_echo("\r\n+UDPSETUP:");
    nwy_ext_echo(data);
}

void nwy_ext_client_acpt_cb(char* data)
{
    nwy_ext_echo("\r\nConnect AcceptSocket=");
    nwy_ext_echo(data);
}

void nwy_ext_gprs_disconnect_cb(char *data)
{
    nwy_ext_echo("\r\nGPRS DISCONNECTION");
    nwy_ext_echo(data);

}

void nwy_ext_sms_list_resp_cb(char *data)
{
    nwy_ext_echo("\r\n+CMGL: ");
    nwy_ext_echo(data);

}

void nwy_ext_ring_resp_cb(char *data)
{
    nwy_ext_echo("\r\nRING");
}

/* added by wangchen for stk openapi 2021.08.26 begin */
#ifdef FEATURE_NWY_STK_COMMAND
static void nwy_recv_sat_info_cb()
{
    nwy_sat_notify_info_type_t sat_data = {0};

    nwy_sat_recv_data(&sat_data);
    /* then can process sat data by customer */
    nwy_ext_echo("\r\nprocess sat data");
}
#endif
/* added by wangchen for stk openapi 2021.08.26 end */

void nwy_ext_init_unsol_reg()
{
    nwy_sdk_at_unsolicited_cb_reg("+EUSIM", nwy_pull_out_sim);
	/* added by wangchen for N58 sms api to test 20200215 begin */
	nwy_sdk_at_unsolicited_cb_reg("+CMT", nwy_recv_sms);
	/* added by wangchen for N58 sms api to test 20200215 end */
	nwy_sdk_at_unsolicited_cb_reg("+CLIP", nwy_incom_voice);
	nwy_sdk_at_unsolicited_cb_reg("+ATD", nwy_voice_ind);
    nwy_sdk_at_unsolicited_cb_reg("Connect AcceptSocket=", nwy_ext_client_acpt_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPRECV(S): ", nwy_ext_tcprecvs_cb);
    nwy_sdk_at_unsolicited_cb_reg("+CLOSECLIENT: ", nwy_ext_acpt_close_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPSETUP: ", nwy_ext_tcpsetup_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPRECV: ", nwy_ext_tcprecv_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPCLOSE: ", nwy_ext_tcp_close_cb);
    nwy_sdk_at_unsolicited_cb_reg("+UDPRECV: ", nwy_ext_udprecv_cb);
    nwy_sdk_at_unsolicited_cb_reg("+UDPSETUP: ", nwy_ext_udpsetup_cb);
    nwy_sdk_at_unsolicited_cb_reg("GPRS DISCONNECTION", nwy_ext_gprs_disconnect_cb);
    nwy_sdk_at_unsolicited_cb_reg("+CMGL: ", nwy_ext_sms_list_resp_cb);
    nwy_sdk_at_unsolicited_cb_reg("RING", nwy_ext_ring_resp_cb);
/* added by wangchen for stk openapi 2021.08.26 begin */
#ifdef FEATURE_NWY_STK_COMMAND
    nwy_sdk_at_unsolicited_cb_reg("+STKPCI", nwy_recv_sat_info_cb);
#endif
/* added by wangchen for stk openapi 2021.08.26 end */
}

static void nwy_ext_virtual_at_test()
{
    int ret = -1;
    char resp[2048] = {0};
    nwy_at_info at_cmd;
    nwy_sleep(500);

    nwy_ext_input_gets("\r\nSend AT cmd :");
    if (strlen(nwy_ext_sio_recv_buff) > 128) {
        return ;
    }

    memset(&at_cmd, 0, sizeof(nwy_at_info));
    memcpy(at_cmd.at_command,nwy_ext_sio_recv_buff,nwy_ext_sio_len);
    at_cmd.at_command[nwy_ext_sio_len] = '\r';
    at_cmd.at_command[nwy_ext_sio_len+1] = '\n';
    at_cmd.length = nwy_ext_sio_len +2;
    ret =  nwy_sdk_at_cmd_send(&at_cmd, resp, NWY_AT_TIMEOUT_DEFAULT);
    if (ret == NWY_SUCESS) {
        nwy_ext_echo("\r\n Resp:%s", resp);
    } else if (ret == NWY_AT_GET_RESP_TIMEOUT) {
        nwy_ext_echo("\r\n AT timeout");
    } else {
        nwy_ext_echo("\r\n AT ERROR");
    }
}
static void nwy_ext_virtual_send_hex()
{
    int ret = -1;
    char resp[2048] = {0};
    nwy_at_info at_cmd;
    char *str = "AT+UDPSEND=1,5";
    nwy_sleep(500);

    memset(&at_cmd, 0, sizeof(nwy_at_info));
    memcpy(at_cmd.at_command, str, strlen(str));
    at_cmd.at_command[strlen(str)] = '\r';
    at_cmd.at_command[strlen(str)+1] = '\n';
    at_cmd.length = strlen(str) +2;
    ret =  nwy_sdk_at_cmd_send(&at_cmd, resp, NWY_AT_TIMEOUT_DEFAULT);
    if (ret == NWY_SUCESS) {
        if (strstr(resp, ">") != NULL) {
            nwy_ext_echo("\r\n Resp:%s", resp);
            uint8 data[5] = {0};
            data[0] = 0x00;
            data[1] = 0x00;
            data[2] = 0x02;
            data[3] = 0x03;
            data[4] = 0x04;
            memcpy(at_cmd.at_command, data, 5);
            at_cmd.length = 5;
            ret =  nwy_sdk_at_cmd_send(&at_cmd, resp, NWY_AT_TIMEOUT_DEFAULT);
        }
    } else if (ret == NWY_AT_GET_RESP_TIMEOUT) {
        nwy_ext_echo("\r\n AT timeout");
    } else {
        nwy_ext_echo("\r\n AT ERROR");
    }
}

static void  nwy_csim_test()
{
    int ret = -1;
    char *open = "0070000001";
    char *choice = "01A4040010A0000005591010FFFFFFFF8900000100";
    char *cmd = "81E2910003BF2000";
    char *close = "0070800100";
    char resp[1024] = {0};
    int resp_len = 1024;
    char data[1024] = {0};
    int sim_id = 0;

    nwy_ext_input_gets("\r\n input data:");
    strcpy(data, nwy_ext_sio_recv_buff);
    ret = nwy_sim_csim(sim_id, data, strlen(data), resp, resp_len);
    if(ret < 0)
    {
        nwy_ext_echo("\n nwy_sim_csim open error");
        return ;
    }

    nwy_ext_echo("\n nwy_sim_csim success [%s]", resp);
}

/* beigin : add by lipengchao in 2020.2.16 for sim api test */
void nwy_ext_sim_menu()
{
  nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
  nwy_ext_echo("1. pin test\r\n");
  nwy_ext_echo("2. get sim status \r\n");
  nwy_ext_echo("3. ccid & imsi\r\n");
  nwy_ext_echo("4. get pin mode\r\n");
/* Begin by zhaoyating to control tripple sim 20220615*/
  nwy_ext_echo("5. get current sim\r\n");
  nwy_ext_echo("6. set current sim\r\n");
  nwy_ext_echo("7. exit\r\n");
/* End by zhaoyating to control tripple sim 20220615*/
}

static void nwy_sim_ready_handle()
{

  OSI_LOGI(0,"=UIM= nwy_sim_ready_handle coming in");
  nwy_sim_result_type sim = {"","","","",NWY_SIM_AUTH_NULL};
  nwy_result_type result;
  int pin_mode = 0;
  nwy_sim_status sim_status = NWY_SIM_STATUS_NOT_INSERT;
/* Begin by zhaoyating to control tripple sim 20220615*/
  int current_sim ;
/* End by zhaoyating to control tripple sim 20220615*/
  char* sptr = nwy_ext_sio_recv_buff;
  /*result = nwy_sim_get_iccid(&sim);
  nwy_sleep(1000);
  OSI_LOGI(0,"=UIM= iccid result : %d", result);
  result = nwy_sim_get_imsi(&sim);
  nwy_sleep(1000);
  OSI_LOGI(0,"=UIM= imsi result : %d", result);
*/
    while (1)
    {
        nwy_ext_sim_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        switch(atoi(sptr))
        {
            case 1:
            {
                nwy_ext_input_gets("\r\nPlease set pin mode:0-disable,1-enable: ");
                pin_mode = atoi(sptr);
                nwy_ext_input_gets("\r\nPlease input lock password: ");
                strncpy(sim.unlock_sim_password, sptr, strlen(sptr));
                strncpy(sim.lock_sim_password, sptr, strlen(sptr));
                if(pin_mode == 0)
                    result = nwy_sim_disable_pin(&sim);
                else if(pin_mode == 1)
                    result = nwy_sim_enable_pin(&sim);
                nwy_sleep(1000);
                if(result == NWY_RES_OK)
                    nwy_ext_echo("\r\n pin operate success");
                else
                    nwy_ext_echo("\r\n pin operate fail");
                break;
            }
            case 2:
            {
                sim_status = nwy_sim_get_card_status();
                nwy_ext_echo("\r\n status is %d(0-ready,1-not insert,2-pin;3-puk,4-busy)", sim_status);
                break;
            }
            case 3:
            {
                memset(&sim , 0, sizeof(nwy_sim_result_type));
                result = nwy_sim_get_iccid(&sim);
                nwy_sleep(1000);
                result = nwy_sim_get_imsi(&sim);
                nwy_ext_echo("\r\n iccid:%s, imsi:%s)", sim.iccid, sim.imsi);
                break;
            }
            case 4:
                memset(&sim , 0, sizeof(nwy_sim_result_type));
                if (nwy_sim_get_pin_mode(&sim))
                    nwy_ext_echo("\r\nget pin error");
                else
                {
                    nwy_sleep(2000);
                    nwy_ext_echo("\r\npin mode :%d", sim.pin_mode);
                }
                break;
/* Begin by zhaoyating to control tripple sim 20220615*/
			#ifdef FEATURE_NWY_OPER_SCAN_AND_SEL
            case 5:  //get current sim for triple sim device
                current_sim = nwy_get_oper_curr_sim_id();
                nwy_ext_echo("\r\n========================================\r\n");
                nwy_ext_echo("\r\nCurrent sim is %d (1 - CU sim,2 - CT SIM,3 - CMCC SIM) \r\n",current_sim);
                nwy_ext_echo("\r\n========================================\r\n");
                break;
            case 6: // set current sim for triple sim device
                nwy_ext_echo("\r\n========================================\r\n");
                nwy_ext_input_gets("\r\nPlease select sim card (1 - CU sim,2 - CT SIM,3 - CMCC SIM):");
                char sub_opt[10] = "";
                if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                    break;
                memset(sub_opt, 0, sizeof(sub_opt));
                memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
                if (STD_SUCCESS != NWY_IS_DIGIT (sub_opt[0]))
                    break;
                if ((sub_opt[0] == '3') || (sub_opt[0] == '1') || (sub_opt[0] == '2'))
                {
                    nwy_set_switch_simcross(sub_opt[0] - '0');
                    nwy_ext_echo("\r\nSet SIM CARD %d over\r\n",(sub_opt[0] - '0'));
                }
                else
                {
                    nwy_ext_echo("\r\nInput Wrong SIM CARD mode!!!\r\n");
                }
                nwy_ext_echo("\r\n========================================\r\n");
                break;
			#endif
            case 7:
                return;
/*End by zhaoyating to control tripple sim 20220615*/
            default:
                break;
        }
    }
}

static void nwy_sim_test(void)
{
   //csim test sim1
  nwy_ext_input_gets("\n input 0 == test csim / 1 == SIM test / 2 == set sim detect trigger mode / 3 == set sim detect mode");
  if(0 == atoi(nwy_ext_sio_recv_buff))
  {
      nwy_csim_test();
      return;
  }
  if(2 == atoi(nwy_ext_sio_recv_buff))
  {
      nwy_ext_input_gets("\n select sim detect trigger mode(0-low level,1-high level)");
      int mode = atoi(nwy_ext_sio_recv_buff);
      nwy_set_sim_detect_trigger_mode(mode);
      return;
  }
  if(3 == atoi(nwy_ext_sio_recv_buff))
  {
      nwy_ext_input_gets("\n select sim detect mode(0-disable,1-enable)");
      int enable = atoi(nwy_ext_sio_recv_buff);
      nwy_set_sim_detect_mode(enable);
      return;
  }
  else
  {
      nwy_ext_echo("\r\n SIM test");
  }

  nwy_sim_status sim_status = NWY_SIM_STATUS_NOT_INSERT;
  sim_status = nwy_sim_get_card_status();
  switch(sim_status)
  {
    case NWY_SIM_STATUS_READY:
	    nwy_ext_echo("\r\n SIM card ready, Proactive verification");
	    nwy_sim_ready_handle();
    break;

    case NWY_SIM_STATUS_NOT_INSERT:
    	nwy_ext_echo("\r\n SIM card not insert");
    	break;
    case NWY_SIM_STATUS_PIN1:
    nwy_ext_echo("\r\n SIM card lock pin");

    break;

    case NWY_SIM_STATUS_PUK1:
    	nwy_ext_echo("\r\n SIM card lock puk");
    break;

    case NWY_SIM_STATUS_BUSY:
    	nwy_ext_echo("\r\n SIM card busy");
    break;
    default:
    	break;
  }
}
/* end : add by lipengchao in 2020.2.16 for sim api test */
/* added by wangchen for N58 ussd api to test 20200826 begin */
int test_send_ussd()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_ussd_info_t ussd_data = {0};
    uint8 dcs;
    uint8 simid;
    char ussd_string[160] = {0};
    nwy_ussd_info_t ussd_info = {0};
    nwy_ussd_result_t ret = NWY_USSD_SUCCESS;

    memset(ussd_string, 0, strlen(ussd_string));

    nwy_ext_input_gets("\r\nPlease input dcs: ");
    dcs = atoi(sptr);

    nwy_ext_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);

    nwy_ext_input_gets("\r\nPlease input ussd string: ");
    memcpy(ussd_string,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));


    ret = nwy_ussd_send_msg(simid, ussd_string, dcs, &ussd_info);
    if(NWY_SMS_SUCCESS != ret)
    {
        OSI_LOGI(0, "nwy send ussd fail");
        return -1;
    }

    nwy_ext_echo("\r\n ussd info Option:%d",ussd_info.nOption);
    nwy_ext_echo("\r\n ussd info Dcs:%d",ussd_info.nDcs);
    nwy_ext_echo("\r\n ussd info usd:%s",ussd_info.aucDestUsd);

    return 0;
}

static void nwy_ussd_test(void)
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_sms_info_type_t sms_data = {0};
    int ret = 0;

    nwy_ext_echo("\r\nPlease select an option to test ussd below. \r\n");
    nwy_ext_echo("1. send ussd test \r\n");

    nwy_ext_input_gets("\r\nPlease input option: ");

    switch(atoi(sptr))
    {
        case 1:
        {
            ret = test_send_ussd();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n send ussd fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n send ussd success \r\n");
            }
            break;
        }

        default:
            nwy_ext_echo("\r\n invalid input \r\n");
            break;
    }
}
/* added by wangchen for N58 ussd api to test 20200826 end */

void nwy_ext_app_info_proc(const uint8 *data, int length)
{
    if(length >= g_recv->size - g_recv->pos)
    {
      length = g_recv->size - g_recv->pos;
      OSI_LOGI(0,"\r\ninput data len > mem len so cut off %d", length);
    }
    memcpy((g_recv->data + g_recv->pos), data, length);
    g_recv->pos = g_recv->pos + length;

    nwy_ext_echo("\r\n nwy_app_info.rev_len = %d length = %d\r\n", g_recv->pos, length);
    if (g_recv->pos  == g_recv->size) {
        nwy_ext_send_sig(g_app_thread, NWY_EXT_FOTA_DATA_REC_END);
        nwy_update_flag = 0;
    }
    if (g_recv->pos > g_recv->size) {
        nwy_ext_send_sig(g_app_thread, NWY_EXT_FOTA_DATA_REC_END);
        nwy_update_flag = 0;
        //nwy_ext_echo("\r\nAPP info is error,upgrade fail");
    }
}


static void nwy_fota_test(void)
{
    nwy_osiEvent_t event;

    nwy_sleep(500);

    if(0 == nwy_get_update_result())
        nwy_ext_echo("\r\n last update success");

    nwy_ext_input_gets("\r\n Please input firmware packet size:");
    int rtn;
    rtn  = atoi(nwy_ext_sio_recv_buff);
    if (rtn <= 0)
    {
        nwy_ext_echo("\r\n Fota Error : invalid packet size:%s", nwy_ext_sio_recv_buff);
        return ;
    }
    nwy_ext_echo("\r\n update packet size:%d", rtn);
    uint32_t size = (uint32_t)rtn;
    g_recv = malloc(sizeof(dataRecvContext_t) + size);
    if (g_recv == NULL)
    {
        nwy_ext_echo("\r\n Fota Error : malloc packet buffer fail");
        return ;
    }

    g_recv->size = size;
    g_recv->pos = 0;
    nwy_update_flag = 1;
    while(1)
    {
        memset(&event, 0, sizeof(event));
        nwy_wait_thead_event(g_app_thread, &event, 0);
        if (event.id == NWY_EXT_FOTA_DATA_REC_END){
            nwy_ext_echo("\r\n recv %d bytes firmware packet success", g_recv->size);

            unsigned wsize = nwy_fota_update(g_recv->data, g_recv->size);
            if (wsize != g_recv->size)
            {
                nwy_ext_echo("\r\n Fota Error : write firmware fail");
            }
            else
            {
                nwy_version_update(true);
                nwy_ext_echo("\r\n Fota Error : verify firmware fail");
            }
            free(g_recv);
            return;
        }
    }
}

int uart_hd = 0;
static void nwy_uart_printf(const char *format, ...)
{
    char send_buf[512] = "\r\n";
    va_list ap;
    size_t size = 2;
    int hd = uart_hd;

    va_start(ap, format);
    size += vsnprintf(send_buf + size, sizeof(send_buf) - size, format, ap);
    va_end(ap);

    nwy_uart_send_data(hd, (uint8_t *)send_buf, size);
}

static void nwy_uart_recv_handle (const char *str,uint32_t length)
{
    nwy_uart_parity_t parity;
    nwy_uart_data_bits_t data_size;
    nwy_uart_stop_bits_t stop_size;
    bool flow_ctrl;
    int baud;
    const char *cmd = str;
    int hd = uart_hd;

    switch(*cmd++)
    {
        case 'b':
            baud = atoi(cmd);
            if(1200 <= baud && 8000000 >= baud)
            {
                nwy_uart_set_baud(hd, baud);
                nwy_uart_printf("switch baud rate to:%d", baud);
            }
            else
                nwy_uart_printf("invalid baud rate:%d", baud);
            break;

        case 'p':
            nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
            parity = atoi(cmd);
            if(0 <= parity && 2 >= parity)
            {
                nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
                nwy_uart_printf("switch parity to:%d", parity);
            }
            else
                nwy_uart_printf("invalid parity:%d", parity);
            break;

        case 'd':
            nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
            data_size = atoi(cmd);
            if(7 <= data_size && 8 >= data_size)
            {
                nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
                nwy_uart_printf("switch data_size to:%d", data_size);
            }
            else
                nwy_uart_printf("invalid data_size:%d", data_size);
            break;

        case 's':
            nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
            stop_size = atoi(cmd);
            if(1 <= stop_size && 2 >= stop_size)
            {
                nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
                nwy_uart_printf("switch stop_size to:%d", stop_size);
            }
            else
                nwy_uart_printf("invalid stop_size:%d", stop_size);
            break;

        case 'f':
            nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
            flow_ctrl = atoi(cmd);
            if(0 <= parity && 1 >= parity)
            {
                nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
                nwy_uart_printf("switch flow_ctrl to:%d", flow_ctrl);
            }
            else
                nwy_uart_printf("invalid flow_ctrl:%d", flow_ctrl);
            break;

        case 't':
            nwy_set_rx_frame_timeout(hd,70);
            nwy_uart_printf("nwy_set_rx_frame_timeout:%d", 70);
            break;

         default:
         	{
                nwy_uart_send_data(hd, (uint8_t *)str, length);
				nwy_uart_printf("nwy_uart_send data length = %d\r\n",length);
         	}
            break;
    }
}

/**
 * @brief: set rs485 control gpio direction
 *
 * @param port: gpio id
 * @param value : 0-low; 1-high

 * @return 0: set failed
 *         1: set success
 */
static void nwy_rs485_direction_switch(int port, int value)
{
   nwy_gpio_set_direction(port,nwy_output);
   nwy_gpio_set_value(port,value);
}

/*if send completly, the callback func will set RS485 as rx state*/
static void nwy_uart_send_complet_handle(int param)
{
    nwy_rs485_direction_switch(RS485_GPIO_PORT, RS485_DIR_TX);
    nwy_ext_echo("\r\n nwy_uart_send_complet_handle success!");
}

static void nwy_uart_test(void)
{
    uint32_t name;
    nwy_sleep(500);

    nwy_ext_input_gets("\r\please input the uart id(1-URT1,2-URT2 or 3-URT3):");
    int por = atoi(nwy_ext_sio_recv_buff);
    if(por == 1)
        name = NWY_NAME_UART1;
    else if(por == 2)
        name = NWY_NAME_UART2;
    else if(por == 3)
        name = NWY_NAME_UART3;

    nwy_ext_input_gets("\r\please input the uart mode(0-at,1-data):");
    int mode = atoi(nwy_ext_sio_recv_buff);

    int hd = nwy_uart_init(name,mode);
    uart_hd = hd;
    nwy_uart_reg_recv_cb(hd,nwy_uart_recv_handle);

    nwy_ext_input_gets("\r\Please sure to close this uart(0-no,1-yes):");
    int cst = atoi(nwy_ext_sio_recv_buff);
    if(cst)
        nwy_uart_deinit(hd);
    else
        return;
}

static void nwy_rs485_test(void)
{
    uint32_t name;
    nwy_sleep(500);
    int port;
    int mode;
    int hd;
    char *pstsnd = "hellors485";

    port = 1; /*uart port id*/
    name = NWY_NAME_UART1;
    mode = 1; /*data*/

    /*init: set RS485 as rx state*/
    hd = nwy_uart_init(name,mode);
    nwy_rs485_direction_switch(RS485_GPIO_PORT, RS485_DIR_RX);

    /*register cb func to uart drv */
    nwy_uart_reg_tx_cb(hd, nwy_uart_send_complet_handle );

    /* for send, set RS485 as tx state */
    nwy_rs485_direction_switch(RS485_GPIO_PORT, RS485_DIR_TX);
    nwy_uart_send_data(hd, (uint8_t *)pstsnd, strlen(pstsnd));
    nwy_ext_input_gets("\r\Please sure to close this uart(0-no,1-yes):");
    int cst = atoi(nwy_ext_sio_recv_buff);
    if(cst)
        nwy_uart_deinit(hd);
    else
        return;
}

#define SPI_NOR_TEST_DATA_SIZE    4096
#define SPI_NOR_SIZE    (8 * 1024 * 1024)

#define Xm25_JedecDeviceID 0x9f
#define Xm25_SectorEarse 0x20
#define Xm25_ReadData 0x03
#define Xm25_WriteEnable 0x06
#define Xm25_PageProgram 0x02
#define Xm25_ReadStatusReg 0x05

uint8_t OSI_ALIGNED(16) recvdata[SPI_NOR_TEST_DATA_SIZE] = {0};
uint8_t OSI_ALIGNED(16) sendata[SPI_NOR_TEST_DATA_SIZE] = {0};

static void nwy_spi_test(void)
{
    uint16_t i;
    uint16_t j = 0;
    uint32_t addr;
    nwy_sleep(20000);
    nwy_ext_echo("\r\n nwy_spi_nor_test start");
    int hd = nwy_spi_init(NAME_SPI_BUS_1, SPI_MODE_0, 1000000, 8);
    uint32_t id = nwy_read_flash_id(hd, Xm25_JedecDeviceID);
    nwy_ext_echo("\r\n test read id %x", id);
    addr = 0;
    for (i = 0; i < 10; i++)
    {
        int64_t startTime;
        int64_t endtime;
        memset(sendata, j, SPI_NOR_TEST_DATA_SIZE);
        j = j + 1;
        if (j == 256)
            j = 0;
        startTime = nwy_get_up_time_us_int64();
        nwy_spi_flash_sector_erase(hd,Xm25_WriteEnable,Xm25_ReadStatusReg,Xm25_SectorEarse,addr);
        endtime = nwy_get_up_time_us_int64() - startTime;
        nwy_ext_echo("\r\n spi erase flash %d bytes data at %d take  %d ms", SPI_NOR_TEST_DATA_SIZE, addr, (int)(endtime / 1000));
        startTime = nwy_get_up_time_us_int64();
        nwy_spi_flash_write(hd, Xm25_WriteEnable,Xm25_ReadStatusReg,Xm25_PageProgram, (uint8_t *)sendata, addr, SPI_NOR_TEST_DATA_SIZE);
        endtime = nwy_get_up_time_us_int64() - startTime;
        nwy_ext_echo("\r\n spi write flash %d bytes data to %d take  %d ms", SPI_NOR_TEST_DATA_SIZE, addr, (int)(endtime / 1000));
        startTime = nwy_get_up_time_us_int64();
        nwy_spi_flash_read(hd,Xm25_ReadData, (uint8_t *)recvdata, addr, SPI_NOR_TEST_DATA_SIZE);
        endtime = nwy_get_up_time_us_int64() - startTime;
        nwy_ext_echo("\r\n spi read flash %d bytes data from %d take  %d ms", SPI_NOR_TEST_DATA_SIZE, addr, (int)(endtime / 1000));
        int cmp = memcmp(recvdata, sendata, SPI_NOR_TEST_DATA_SIZE);
        nwy_ext_echo("\r\n spi read write flash %d bytes data from %d compare %d", SPI_NOR_TEST_DATA_SIZE, addr, cmp);
        memset(recvdata, 0, SPI_NOR_TEST_DATA_SIZE);
        nwy_sleep(2000);
        addr += SPI_NOR_TEST_DATA_SIZE;
        addr %= SPI_NOR_SIZE;
    }
    nwy_spi_deinit(hd);
}


#define SE_CHIP_WAKEUP_GPIO 18
static void nwy_se_wakeup_init(void)
{
    nwy_gpio_set_direction(SE_CHIP_WAKEUP_GPIO, nwy_output);
    nwy_gpio_set_value(SE_CHIP_WAKEUP_GPIO, 1);
}

static void nwy_se_wakeup(void)
{
    nwy_gpio_set_value(SE_CHIP_WAKEUP_GPIO, 0);
    osiDelayUS(500);
    nwy_gpio_set_value(SE_CHIP_WAKEUP_GPIO, 1);
}

static void nwy_gpio_spi_test(void)
{
    struct spi_gpio_platform_data gpio =
    {
        .sck = 23,
        .mosi= 21,
        .miso= 17,
        .chipselect=22
    };
    nwy_se_wakeup_init();
    osiThreadSleep(1000);
    nwy_ext_echo("\r\n nwy gpio spi test start");
    osiThreadSleep(1000);
    int n = 3;
    while(n--)
    {
        int hd = nwy_drv_gpio_spi_init(&gpio, SPI_MODE_0, 1000000, 8);
        nwy_ext_echo("\r\n nwy gpio spi test, spi handle:%x", hd);
        char se_rst_cmd[] = {0x3b, 0x17, 0x01, 0x81, 0x00, 0x02, 0x95, 0x12, 0x40, 0x0d};
        char se_rsp[3];
        nwy_se_wakeup();
        int rtn = nwy_drv_gpio_spi_transfer(hd, se_rst_cmd, NULL, sizeof(se_rst_cmd));
        nwy_ext_echo("\r\n nwy gpio spi test, se rst cmd send:%d", rtn);
        osiThreadSleep(200);
        int i = 2;
        while(i--)
        {
            memset(se_rsp, 0, sizeof(se_rsp));
            nwy_se_wakeup();
            rtn = nwy_drv_gpio_spi_transfer(hd, NULL, se_rsp, sizeof(se_rsp));
            if(!rtn &&(se_rsp[0] || se_rsp[1] || se_rsp[2]))
            {
                nwy_ext_echo("\r\n nwy gpio spi test, se resp:%x %x %x", se_rsp[0], se_rsp[1], se_rsp[2]);
            }
            osiThreadSleep(2);
        }
        rtn = nwy_drv_gpio_spi_deinit(hd);
        nwy_ext_echo("\r\n nwy gpio spi test, spi deinit:%d", rtn);
        osiThreadSleep(1000);
    }
}

#define NWY_MAKE_TAG(a, b, c, d) ((unsigned)(a) | ((unsigned)(b) << 8) | ((unsigned)(c) << 16) | ((unsigned)(d) << 24))
typedef struct
{
    int hd;
    nwy_osiMutex_t *lock;
} nwy_flash_spi_t;

static int nwy_spi_flash_block_read(nwy_spi_flash_t *dev, uint32_t offset, uint32_t size, void *buf)
{
    OSI_LOGI(0, "nwy sf block read (0x%x) offset/%u size/%d", buf, (unsigned)offset, size);
    nwy_flash_spi_t *flash_spi= (nwy_flash_spi_t *)(dev->priv);
    nwy_lock_mutex(flash_spi->lock, 0);
    nwy_spi_flash_read(flash_spi->hd,Xm25_ReadData, (uint8_t *)buf, offset, size);
    nwy_unlock_mutex(flash_spi->lock);
    return size;
}

static int nwy_spi_flash_block_write(nwy_spi_flash_t *dev, uint32_t offset, uint32_t size, const void *data)
{
    OSI_LOGI(0, "nwy sf block write (0x%x) offset/%u size/%d", data, (unsigned)offset, size);
    nwy_flash_spi_t *flash_spi= (nwy_flash_spi_t *)(dev->priv);
    nwy_lock_mutex(flash_spi->lock, 0);
    nwy_spi_flash_write(flash_spi->hd, Xm25_WriteEnable,Xm25_ReadStatusReg,Xm25_PageProgram, (uint8_t *)data, offset, size);
    nwy_unlock_mutex(flash_spi->lock);
    return size;
}

static int nwy_spi_flash_block_erase(nwy_spi_flash_t *dev, uint32_t offset, uint32_t size)
{
    OSI_LOGI(0, "nwy sf block erase offset/%u size/%d", (unsigned)offset, size);
    nwy_flash_spi_t *flash_spi= (nwy_flash_spi_t *)(dev->priv);
    nwy_lock_mutex(flash_spi->lock, 0);
    nwy_spi_flash_sector_erase(flash_spi->hd,Xm25_WriteEnable,Xm25_ReadStatusReg,Xm25_SectorEarse,offset);
    nwy_unlock_mutex(flash_spi->lock);

    return size;
}

static nwy_flash_spi_t nwy_flash_spi_bus;
static nwy_spi_flash_t nwy_spi1_xm25 =
{
    .name = NWY_MAKE_TAG('X', 'M', '2', '5'),
    .block_size = 4096,
    .block_count = 2048,/* 2048:8M bytes 4096:16M bytes */
    .read = nwy_spi_flash_block_read,
    .write = nwy_spi_flash_block_write,
    .erase = nwy_spi_flash_block_erase,
};

#define SPI_FLASH_MOUNT_POINT "/dataflash"
static void nwy_spi_flash_mount_test(void)
{
    int opt, ret;
    nwy_block_device_t * block_dev;
    nwy_ext_echo("\r\n nwy_spi_nor_test start");
    int hd = nwy_spi_init(NAME_SPI_BUS_1, SPI_MODE_0, 30000000, 8);
    uint32_t id = nwy_read_flash_id(hd, Xm25_JedecDeviceID);
    nwy_ext_echo("\r\n test read id %x", id);
    nwy_flash_spi_bus.hd = hd;
    nwy_flash_spi_bus.lock = osiMutexCreate();
    nwy_spi1_xm25.priv = &nwy_flash_spi_bus;
    nwy_ext_input_gets("\r\input option:0-flash capacity <= 16M bytes,1-flash capacity > 16M bytes:");
    opt = atoi(nwy_ext_sio_recv_buff);
    if(opt == 1)
        block_dev = nwy_vfs_logical_block_device_create(NAME_SPI_BUS_1, 0, 8192 * 4096);
    else
        block_dev = nwy_vfs_block_device_create(&nwy_spi1_xm25);
    if(block_dev == NULL)
    {
        OSI_LOGE(0, "flash test block_dev create fail");
        return;
    }
    while(1)
    {
        nwy_ext_input_gets("\r\input option:0-fs mount,1-fs format,2-fs write test,3-fs read test,4-fs free size,5-exit:");
        opt = atoi(nwy_ext_sio_recv_buff);
        if(opt == 0)
        {
            nwy_ext_echo("\r\n flash test nwy_vfs_mount start");
            ret = nwy_vfs_mount(SPI_FLASH_MOUNT_POINT, block_dev);
            if(0 > ret)
            {
                nwy_ext_echo("\r\n flash test mount fail");
                ret = nwy_vfs_mkfs(block_dev);
                if(0 > ret)
                {
                    nwy_ext_echo("\r\n flash test format fail");
                    continue;
                }
                ret = nwy_vfs_mount(SPI_FLASH_MOUNT_POINT, block_dev);
                if(0 > ret)
                {
                    nwy_ext_echo("\r\n flash test remount fail");
                    continue;
                }
            }
            nwy_ext_echo("\r\n flash test nwy_vfs_mount success");
        }
        else if(opt == 1)
        {
            ret = nwy_vfs_mkfs(block_dev);
            nwy_ext_echo("\r\n flash test format :%d", ret);
            continue;
        }
        else if(opt == 2)
        {
            int i = 1;
            int len, size;
            char file_name[64] = {0};
            while(i)
            {
                memset(file_name, 0, sizeof(file_name));
                sprintf(file_name, "%s/%d", SPI_FLASH_MOUNT_POINT, i);
                int fd = nwy_sdk_fopen(file_name, NWY_CREAT | NWY_RDWR | NWY_TRUNC);
                if(fd < 0)
                {
                    nwy_ext_echo("\r\nfile open %s fail\r\n", file_name);
                    break;
                }
                else
                {
                    len = size = 0;
                    while(len < (4096 * i))
                    {
                        size = nwy_sdk_fwrite(fd, file_name, strlen(file_name));
                        if(size <= 0)
                        {
                            nwy_ext_echo("\r\nfile write %s fail\r\n", file_name);
                            nwy_sdk_fclose(fd);
                            break;
                        }
                        len += size;
                    }
                    nwy_sdk_fclose(fd);
                    if(len > 0)
                        nwy_ext_echo("\r\nfile write %s size %d success\r\n", file_name, len);
                    else
                        break;
                }
                i++;
                nwy_sleep(100);
            }
        }
        else if(opt == 3)
        {
            int i = 1;
            int len, size;
            char file_name[64] = {0};
            char read_data[64];
            while(i)
            {
                memset(file_name, 0, sizeof(file_name));
                sprintf(file_name, "%s/%d", SPI_FLASH_MOUNT_POINT, i);
                int fd = nwy_sdk_fopen(file_name, NWY_RDONLY);
                if(fd < 0)
                {
                    nwy_ext_echo("\r\nfile open %s fail\r\n", file_name);
                    break;
                }
                else
                {
                    len = size = 0;
                    while(len < (4096 * i))
                    {
                        memset(read_data, 0, sizeof(read_data));
                        size = nwy_sdk_fread(fd, read_data, strlen(file_name));
                        if(size <= 0)
                        {
                            nwy_ext_echo("\r\nfile read %s fail\r\n", file_name);
                            nwy_sdk_fclose(fd);
                            break;
                        }
                        if(memcmp(read_data, file_name, size))
                        {
                            nwy_ext_echo("\r\nfile read %s data compare fail, read %s dest %s\r\n", file_name, read_data, file_name);
                            nwy_sdk_fclose(fd);
                            break;
                        }
                        len += size;
                    }
                    nwy_sdk_fclose(fd);
                    if(len > 0)
                        nwy_ext_echo("\r\nfile read %s size %d success\r\n", file_name, len);
                    else
                        break;
                }
                i++;
                nwy_sleep(100);
            }
        }
        else if(opt == 4)
        {
            int free_size = nwy_sdk_vfs_free_size(SPI_FLASH_MOUNT_POINT);
            nwy_ext_echo("\r\%s free size:%d\r\n", SPI_FLASH_MOUNT_POINT, free_size);
        }
        else if(opt == 5)
            return;
    }
    nwy_ext_echo("\r\n flash test end");
}


#define ARRAY_NR(a)  (sizeof(a) / sizeof(a[0]))
static void nwy_i2c_test(void)
#if 1
{
    int bus;
    char *i2c_bus = NAME_I2C_BUS_1;
    uint8_t i2c_DEV_ADDR = 0xE2;
    uint8_t read_flag[32] = {0};
    int i = 0;
    uint8_t bbc;
    int ret;
    uint8_t cmd_flag[5] = {0x03, 0x00, 0x00, 0x30};
	int cmd_size = sizeof(cmd_flag);

	nwy_ext_input_gets("\nPlease sel i2c(1-i2c1,2-i2c2,3-i2c3):");
    int i2c = atoi(nwy_ext_sio_recv_buff);
    if(i2c == 1)
    {
        i2c_bus = NAME_I2C_BUS_1;
    }
    else if(i2c == 2)
    {
        i2c_bus = NAME_I2C_BUS_2;
    }
    else if(i2c == 3)
    {
        i2c_bus = NAME_I2C_BUS_3;
    }
    bus = nwy_i2c_init(i2c_bus, NWY_I2C_BPS_100K);
    if(NWY_SUCESS > bus)
    {
        nwy_ext_echo("\r\n I2c Error : bus:%s init fail", i2c_bus);
        return;
    }


    bbc = cmd_flag[0];
    for(i = 1; i < cmd_size - 1; i++)
        bbc = bbc ^ cmd_flag[i];

    cmd_flag[cmd_size - 1] = bbc;

    for(i=0; i<cmd_size; i++)
    {
        // send slave dev addr and start flag and write flag
        if(0 == i)
        {
            ret = nwy_i2c_raw_put_byte(bus, (i2c_DEV_ADDR) & 0xfe, 1, 0);
            if(ret != NWY_SUCESS)
            {
                nwy_ext_echo("\r\n I2c Error : bus:%s read fail", i2c_bus);
                goto error;
            }
            nwy_ext_echo("write addr:0x%02x ", (i2c_DEV_ADDR) & 0xfe);
        }
        if(cmd_size == (i + 1))
            ret = nwy_i2c_raw_put_byte(bus, cmd_flag[i], 0, 1); // send last data and stop flag
        else
            ret = nwy_i2c_raw_put_byte(bus, cmd_flag[i], 0, 0); // send cmd and data
        if(ret != NWY_SUCESS)
        {
            nwy_ext_echo("\r\n I2c Error : bus:%s read fail", i2c_bus);
            goto error;
        }
        nwy_ext_echo("0x%02x ", cmd_flag[i]);
    }
    nwy_ext_echo("end\n\n");

int j = 20;
while(j--)
{
    nwy_sleep(16);

    // send slave dev addr and start flag and read flag
    ret = nwy_i2c_raw_put_byte(bus, (i2c_DEV_ADDR) | 0x01, 1, 0);
    if(ret != NWY_SUCESS)
    {
        nwy_ext_echo("\r\n I2c Error0 : bus:%s read fail", i2c_bus);
        goto error;
    }
    nwy_ext_echo("read addr:0x%02x ", (i2c_DEV_ADDR) | 0x01);
    // read frame length
    ret = nwy_i2c_raw_get_byte(bus, &read_flag[0], 0, 0);
    if(ret != NWY_SUCESS)
    {
        nwy_ext_echo("\r\n I2c Error1 : bus:%s read fail", i2c_bus);
        goto error;
    }
    ret = nwy_i2c_raw_get_byte(bus, &read_flag[1], 0, 0);
    if(ret != NWY_SUCESS)
    {
        nwy_ext_echo("\r\n I2c Error2 : bus:%s read fail", i2c_bus);
        goto error;
    }
    int len = read_flag[0] + read_flag[1] * 256;
    nwy_ext_echo("len:[%d]0x%02x 0x%02x ", len, read_flag[0], read_flag[1]);
    // read frame data
    for(i=0; i<len; i++)
    {
        if(i == (len - 1))
            ret = nwy_i2c_raw_get_byte(bus, &read_flag[i], 0, 1); // send cmd and data
        else
            ret = nwy_i2c_raw_get_byte(bus, &read_flag[i], 0, 0); // send cmd and data
        if(ret != NWY_SUCESS)
        {
            nwy_ext_echo("\r\n I2c Error3 : bus:%s read fail", i2c_bus);
            goto error;
        }
        nwy_ext_echo("0x%02x ", read_flag[i]);
    }
    if(i)
        break;
}

    nwy_ext_echo("\r\n, find i2c on bus:%s", i2c_bus);
error:
    nwy_i2c_deinit(bus);
    return;
}

#else
{
#define AT24C02_DEV_ADDR  0xA0
#define AT24C02_REG_CHECK_ADDR  255
    int bus = nwy_i2c_init(NAME_I2C_BUS_1, NWY_I2C_BPS_400K);
    if(NWY_SUCESS > bus)
    {
        nwy_ext_echo("\r\n I2c Error : bus:%s init fail", NAME_I2C_BUS_1);
        return;
    }
    uint8_t valid_flag;
    int rtn = nwy_i2c_read(bus, AT24C02_DEV_ADDR, AT24C02_REG_CHECK_ADDR, &valid_flag, 1);
    if(rtn != NWY_SUCESS)
    {
        nwy_ext_echo("\r\n I2c Error : bus:%s read fail", NAME_I2C_BUS_1);
        return;
    }
#define AT24C02_CHECK_MN  0x55
    if(valid_flag != AT24C02_CHECK_MN)
    {
        nwy_ext_echo("\r\n I2c Error : check AT24C02 fail");
        return;
    }
    nwy_ext_echo("\r\n, find AT24C02 on bus:%s", NAME_I2C_BUS_1);
    return;
}
#endif
/* beigin : add by yangmengsha in 2020.2.17 for some api test */
void nwy_adc_test()
{
    int value = -1;

    nwy_ext_input_gets("\r\nChoose the ADC channel(2-CHANNEL2,3-CHANNEL3,4-VBAT):");
    int channel = atoi(nwy_ext_sio_recv_buff);
    nwy_ext_input_gets("\r\nChoose the ADC scale(0-1V250,1-2V444,2-3V233,3-5V000):");
    int scale = atoi(nwy_ext_sio_recv_buff);

    value = nwy_adc_get(channel,scale);
    nwy_ext_echo("\r\n nwy adc get value = %d", value);
}

void nwy_led_backlight_test()
{
    int ret = -1;
    int port;
    int opt;
    char *cmd = nwy_ext_sio_recv_buff + 0;
    char *id  = nwy_ext_sio_recv_buff + 2;
    char *value  = nwy_ext_sio_recv_buff + 4;

    while(1)
    {
        nwy_ext_echo("\r\nbacklight test cmd:\r\n");
        nwy_ext_echo("o:n   [Open backlight,     n(0-LEDk0,1-LEDk1,2-LEDk2)]\r\n");
        nwy_ext_echo("c:n   [Close backlight,    n(0-LEDk0,1-LEDk1,2-LEDk2)]\r\n");
        nwy_ext_echo("s:n,v [Set backlight level,n(0-LEDk0,1-LEDk1,2-LEDk2),v(0~63)]\r\n");
        nwy_ext_echo("e     [Exit backlight test]\r\n");
        nwy_ext_input_gets("Please input backlight test cmd:\r\n");

        switch(*cmd)
        {
        case 'o':
            port = atoi(id);
            ret = nwy_led_back_light_open(port);
            if(ret)
                nwy_ext_echo("\r\n LED backlight open success");
            else
                nwy_ext_echo("\r\n LED backlight open failed");
            break;

        case 's':
            port = atoi(id);
            int level = atoi(value);
            ret = nwy_set_back_light_level(port + NWY_POWER_RGB_IB0, level);
            if(ret)
                nwy_ext_echo("\r\n set LED backlight level to:%d success", level);
            else
                nwy_ext_echo("\r\n set LED backlight level to:%d failed", level);
            break;

        case 'c':
            port = atoi(id);
            ret = nwy_led_back_light_close(port);
            if(ret)
                nwy_ext_echo("\r\n LED backlight close success");
            else
                nwy_ext_echo("\r\n LED backlight close failed");
            break;

        case 'e':
            return;
        }
    }
}

int gpio_pin = 0;
int isr_mode;

static void _gpioisropen(int param)
{
    if(isr_mode > 2)
        OSI_LOGI(0, "yms test gpio in isr");
    else
        nwy_ext_echo("\r\ngpio isr set success");
}

void nwy_gpio_test()
{
    int test = 0;
    int port, mode = 0;
    int opt;
    int dire;
    int valu;
    int isrmode;

    while (1 )
    {
    nwy_ext_input_gets("\r\nset the gpio id: q exit");
    if(nwy_ext_sio_recv_buff[0] == 'q')
    {
        nwy_ext_echo("exit now \r\n");
        return ;
    }
    port = atoi(nwy_ext_sio_recv_buff);
    nwy_ext_echo("set GPIO%d \r\n", port);
    nwy_ext_input_gets("\r\nset the gpio mode(0-in,1-out,2-irq,3-pullup or down):");
    mode = atoi(nwy_ext_sio_recv_buff);
    nwy_ext_echo("set MODE: %d \r\n", mode);

    switch(mode)
    {
        case 0:
            nwy_gpio_set_direction(port,nwy_input);
            int value = nwy_gpio_get_value(port);
            nwy_ext_echo("nwy_gpio_get_value(%d) = %d \r\n", port,value);
            break;

        case 1:
            nwy_ext_input_gets("set the gpio value(0-low,1-high):");
            valu = atoi(nwy_ext_sio_recv_buff);
            nwy_gpio_set_direction(port,nwy_output);
            nwy_gpio_set_value(port,valu);
            nwy_ext_echo("nwy_gpio_set_value(%d,%d) \r\n", port,valu);
            break;

        case 2:
            nwy_ext_input_gets("\r\nset the interrupt isr mode(0-rising,2-rising&falling,3-high,4-low):");
            isrmode = atoi(nwy_ext_sio_recv_buff);
            gpio_pin = port;
            nwy_close_gpio(port);
            int data = nwy_open_gpio_irq_config(port,isrmode,_gpioisropen);
            isr_mode = isrmode;

            if(data)
            {
                nwy_ext_echo("gpio isr config success\r\n");
            }
            else
            {
                nwy_ext_echo("gpio isr config failure \r\n");
            }
            break;

        case 3:
            nwy_ext_input_gets("\r\nset the gpio up or down:");
            int pull = atoi(nwy_ext_sio_recv_buff);
            nwy_gpio_pullup_or_pulldown(port, pull);
            nwy_ext_echo("nwy_gpio_pullup_or_pulldown(%d,%d) \r\n", port,pull);

        default:
        break;
    }
        nwy_ext_input_gets("\r\n test end (1: exit ):");
        int close = atoi(nwy_ext_sio_recv_buff);
        if(close == 1)
        {
            if(mode == 2)
            {
                nwy_gpio_open_irq_disable(port);
            }
            nwy_close_gpio(port);
            nwy_ext_echo("close_gpio%d \r\n", port);
        }
    }
}

void nwy_pm_test()
{
    int opt;
    int hour, min, sec, ret;
    int state = nwy_power_state();
    nwy_ext_echo("\r\nnwy_power_state in %d",state);
    nwy_subpower_switch(NWY_POWER_SD , true, false);
    nwy_set_pmu_power_level(NWY_POWER_SD,1800);

    while(1)
    {
        nwy_ext_input_gets("\r\nPlease select the pm test(0-DTR SLEEP,1-ALARM,2-POWER OFF,3-PSM,8-EXIT):");
        opt = atoi(nwy_ext_sio_recv_buff);
        switch(opt)
        {
            case 0:
                {
                    nwy_ext_input_gets("\r\nPlease select the DTR SLEEP MODE(0-normal,1-low_in_sleep,2-high_in_sleep):");
                    opt = atoi(nwy_ext_sio_recv_buff);
                    switch(opt)
                    {
                        case 0:
                           nwy_dtr_set_state(NWY_NORMAL);
                           nwy_ext_echo("\r\nwy_dtr_set_state in %d",opt);
                           break;
                        case 1:
                           nwy_dtr_set_state(NWY_LOW_INSLEEP);
                           nwy_ext_echo("\r\nwy_dtr_set_state in %d",opt);
                           break;
                        case 2:
                           nwy_dtr_set_state(NWY_HIGH_INSLEEP);
                           nwy_ext_echo("\r\nwy_dtr_set_state in %d",opt);
                           break;
                        default:
                           nwy_ext_echo("\r\nwy_dtr_set_state in error");
                           break;
                      }
                }
                break;
            case 1:
                {
                    nwy_ext_input_gets("\r\nPlease input bootup alarm option:0-set alarm,1-delete alarm,2-reset");
                    opt = atoi(nwy_ext_sio_recv_buff);
                    if(0 == opt)
                    {
                        nwy_ext_input_gets("\r\nPlease input bootup alarm time(hh:mm:ss):");
                        ret = sscanf(nwy_ext_sio_recv_buff, "%d:%d:%d", &hour, &min, &sec);
                        if(ret == 3)
                        {
                            uint32_t sec_in_day = hour * 3600 + min * 60 + sec;
                            nwy_bootup_alarm_set(sec_in_day);
                            nwy_ext_echo("set bootup alarm to:%d", sec_in_day);
                        }
                    }
                    else if(1 == opt)
                    {
                        ret = nwy_bootup_alarm_del();
                        nwy_ext_echo("delete bootup alarm ret:%d", ret);
                    }
                }
                break;
            case 2:
                {
                    nwy_ext_input_gets("\r\nPlease select the power off mode(0-quickly,1-normal,2-reset,3-alarm):");
                    int mode = atoi(nwy_ext_sio_recv_buff);
                    if(mode == 3)
                    {
                        nwy_ext_echo("system will shutdown with bootup alarm");
                        ret = nwy_shutdown_with_alarm();
                        if(ret < 0)
                            nwy_ext_echo("system shutdown with bootup alarm execute fail");
                        return;
                    }
                    nwy_power_off(mode);
                }
                break;
            case 3:
               {
                    nwy_ext_input_gets("\r\nPlease input psm sleep time(unit second):");
                    int secs = atoi(nwy_ext_sio_recv_buff);
                    nwy_pm_psm_time_set(secs);
                    nwy_pm_state_set(NWY_LP_ENABLE_PSM);
               }
               break;
            case 8:
               return;
            default:
               nwy_ext_echo("\r\nwy_dtr_set_state in error");
               break;
        }
    }
}

static void _openkeypad(nwy_key_t key, nwy_keyState_t evt)
{
    uint8_t status;
    if (evt & key_state_press)
        status = 1;
    if (evt & key_state_release)
        status = 0;


    if (evt == key_state_press)
    {
         nwy_ext_echo("\r\nthe key  is PRESS");
    }
    else
    {
        nwy_ext_echo("\r\nkey id %d,status %d", key, status);
    }
}

void nwy_keypad_test()
{
    nwy_ext_input_gets("\r\nSet the debounce:");
    int debunc = atoi(nwy_ext_sio_recv_buff);
    nwy_ext_input_gets("\r\nSet the ITV:");
    int itv = atoi(nwy_ext_sio_recv_buff);

//    nwy_keypad_debounce_time(debunc,itv);
    reg_nwy_key_cb(_openkeypad);
}
/* end : add by yangmengsha in 2020.2.17 for some api test */
void nwy_ext_file_menu()
{
  nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
  nwy_ext_echo("1. write to file\r\n");
  nwy_ext_echo("2. read file \r\n");
  nwy_ext_echo("3. read file size \r\n");
  nwy_ext_echo("4. Exit\r\n");
}

void nwy_file_test(void)
{
    char* sptr = nwy_ext_sio_recv_buff;
    char file_name[NWY_FILE_NAME_MAX + 1] = {0};
    char context[NWY_UART_RECV_SINGLE_MAX + 1] = {0};
	int ls = nwy_sdk_vfs_ls();
    int len = 0,fd =-1, size =0;
    nwy_ext_echo("\r\nfilesystem statistics:%d\r\n", ls);
    while (1)
    {
        nwy_ext_file_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        switch(atoi(sptr))
        {
            case 1:
            {
                memset(file_name, 0, sizeof(file_name));
                nwy_ext_input_gets("\r\nPlease input filename(len <= 32): ");
                if (strlen(sptr) > NWY_FILE_NAME_MAX) {
                    nwy_ext_echo("\r\nfile name can't beyond 32");
                    break;
                }
                strcpy(file_name, sptr);
                int fd = nwy_sdk_fopen(file_name, NWY_CREAT | NWY_RDWR | NWY_TRUNC);
                if(fd < 0)
                    nwy_ext_echo("\r\nfile open fail\r\n");
                else
                {
                    memset(context, 0, NWY_UART_RECV_SINGLE_MAX);
                    nwy_ext_input_gets("\r\nPlease input context(len <= 512): ");
                    OSI_LOGXI(OSI_LOGPAR_SI, 0,"\r\nnwy sptr = %s  nwy_ext_sio_len = %d", sptr, nwy_ext_sio_len);
                    if (nwy_ext_sio_len > NWY_UART_RECV_SINGLE_MAX) {
                        nwy_ext_echo("\r\nNo more than 512 bytes at a time to write in file ");
                        break;
                    }
                    strncpy(context, sptr, strlen(sptr));
                    len = nwy_sdk_fwrite(fd, context, strlen(context));
                    nwy_sdk_fclose(fd);
                    if(len != strlen(context))
                        nwy_ext_echo("\r\nfile write: len=%d, return len=%d\r\n",strlen(context), len);
                    else
                        nwy_ext_echo("\r\nfile write success and close\r\n");
                }
                break;
            }
            case 2:
            {
                memset(file_name, 0, sizeof(file_name));
                nwy_ext_input_gets("\r\nPlease input filename(len<=32): ");
                if (strlen(sptr) > NWY_FILE_NAME_MAX) {
                    nwy_ext_echo("\r\nfile name can't beyond 32");
                    break;
                }
                strcpy(file_name, sptr);
                fd = nwy_sdk_fopen(file_name, NWY_RDONLY);
                if(fd < 0)
                 nwy_ext_echo("\r\nfile open fail");
                else
                {
                    memset(context, 0, NWY_UART_RECV_SINGLE_MAX);
                    len = nwy_sdk_fread(fd, context, sizeof(context));
                    nwy_sdk_fclose(fd);
                    nwy_ext_echo("\r\nfile read:len=%d\r\n", len);
                    if(len!=0)
                        nwy_ext_echo("\r\nfile data:%s\r\n", context);
                    else
                        nwy_ext_echo("\r\nfile read success\r\n");
                }
                break;
            }
            case 3:
            {
                memset(file_name, 0, sizeof(file_name));
                nwy_ext_input_gets("\r\nPlease input filename(len<=32): ");
                if (strlen(sptr) > NWY_FILE_NAME_MAX) {
                    nwy_ext_echo("\r\nfile name can't beyond 32");
                    break;
                }
                strcpy(file_name, sptr);
                size = nwy_sdk_fsize(file_name);
                nwy_ext_echo("\r\nfile size = %d\r\n", size);
                fd = nwy_sdk_fopen(file_name, NWY_RDONLY);
                if(fd < 0)
                    nwy_ext_echo("\r\nfile open fail\r\n");
                else
                {
                    size = nwy_sdk_fsize_fd(fd);
                    nwy_ext_echo("\r\nfile size2 = %d\r\n", size);
                    nwy_sdk_fclose(fd);
                }
                break;
            }
            case 4:
                return;
            default:
                break;
        }
    }
}

void nwy_ext_tcp_menu()
{
  nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
  nwy_ext_echo("1. tcp setup\r\n");
  nwy_ext_echo("2. tcp send \r\n");
  nwy_ext_echo("3. tcp close\r\n");
  nwy_ext_echo("4. Exit tcpfunction\r\n");
}

static void nwy_tcp_recv_func(void *param)
{
    char recv_buff[NWY_UART_RECV_SINGLE_MAX +1] = {0};
    int recv_len =0, result = 0;
    int s = *(int *)param;
    fd_set rd_fd;
    fd_set ex_fd;
    FD_ZERO(&rd_fd);
    FD_ZERO(&ex_fd);
    FD_SET(s,&rd_fd);
    FD_SET(s,&ex_fd);
    struct timeval tv = {0};
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    while(1)
    {

        FD_ZERO(&rd_fd);
        FD_ZERO(&ex_fd);
        FD_SET(s,&rd_fd);
        FD_SET(s,&ex_fd);
        result = nwy_socket_select(s+1, &rd_fd, NULL,&ex_fd, &tv);
        if (result < 0)
        {
          nwy_ext_echo("\r\ntcp select error:\r\n");
          nwy_socket_close(s);
          tcp_connect_flag =0;
          nwy_exit_thread();
        }
        else if(result > 0)
        {
            if (FD_ISSET(s, &rd_fd))
            {
                memset(recv_buff, 0, NWY_UART_RECV_SINGLE_MAX +1);
                recv_len = nwy_socket_recv(s, recv_buff, NWY_UART_RECV_SINGLE_MAX, 0);
                if(recv_len >0)
                    nwy_ext_echo("\r\ntcp read:%d:%s\r\n", recv_len, recv_buff);
                else if(recv_len == 0) {
                    nwy_ext_echo("\r\ntcp close by server\r\n");
                    nwy_socket_close(s);
                    *(int *)param = 0;
                    tcp_connect_flag =0;
                    nwy_exit_thread();
                }
                else
                {
                    nwy_ext_echo("\r\ntcp connection error\r\n");
                    nwy_socket_close(s);
                    *(int *)param = 0;
                    tcp_connect_flag =0;
                    nwy_exit_thread();
                }
            }
            if (FD_ISSET(s, &ex_fd))
            {
                nwy_ext_echo("\r\ntcp select ex_fd:\r\n");
                nwy_socket_close(s);
                *(int *)param = 0;
                tcp_connect_flag =0;
                nwy_exit_thread();
            }
        }
        else
            OSI_LOGI(0,"\r\ntcp select timeout:\r\n");
        nwy_sleep(1000);
    }
}



nwy_ip_type_or_dns_enum nwy_judge_ip_or_dns(char *str)
{
	int len= 0;
	int strLen = 0;
	nwy_ip_type_or_dns_enum retValue = NWY_CUSTOM_IP_TYPE_OR_DNS_DNS;
	if(str == NULL )
	{
		return NWY_CUSTOM_IP_TYPE_OR_DNS_NONE;
	}
	else
	{
		if(strlen(str) <= 0 )
		{
			return NWY_CUSTOM_IP_TYPE_OR_DNS_NONE;
		}
	}

	strLen = strlen(str);

	for(len = 0;len < strLen; len++)
	{
		if( ((*(str+len) >= '0') && (*(str+len) <= '9')) || (*(str+len) == '.') )
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if(len == strLen)
	{
		retValue = NWY_CUSTOM_IP_TYPE_OR_DNS_IPV4;
		return retValue;
	}
	len = 0;
	for(len = 0;len < strLen; len++)
	{
		if( ((*(str+len) >= '0') && (*(str+len) <= '9')) ||
			((*(str+len) >= 'a') && (*(str+len) <= 'f')) ||
			((*(str+len) >= 'A') && (*(str+len) <= 'F')) ||
			(*(str+len) == ':')
			)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	if(len == strLen)
	{
		retValue = NWY_CUSTOM_IP_TYPE_OR_DNS_IPV6;
		return retValue;
	}
	return retValue;
}

static int nwy_hostname_check(char *hostname)
{
    int a,b,c,d;
    char temp[32] = {0};
    if(strlen(hostname) > 15)
    return NWY_GEN_E_UNKNOWN;

    if((sscanf(hostname,"%d.%d.%d.%d",&a,&b,&c,&d))!=4)
        return NWY_GEN_E_UNKNOWN;

    if(!((a <= 255 && a >= 0)&&(b <= 255 && b >= 0)&&(c <= 255 && c >= 0)))
        return NWY_GEN_E_UNKNOWN;

    sprintf(temp,"%d.%d.%d.%d",a,b,c,d);
    memset(hostname, 0, sizeof(hostname));
    strcpy(hostname, temp);
    return NWY_SUCESS;
}

static int nwy_port_get(char *port_str, int *port)
{
    int i = 0;
    if (port == NULL) {
        return NWY_GEN_E_INVALID_PARA;
    }
    for (int i = 0; i < strlen(port_str); i++) {
        if ((port_str + i) == '\r' || (port_str + i) == '\n') {
            continue;
        }
        if ((port_str[i]) < '0' || (port_str[i]) > '9') {
            return NWY_GEN_E_INVALID_PARA;
        }
    }

    *port = atoi(port_str);
    if (*port < 1 || *port > 65535) {
        *port = 0;
        return NWY_GEN_E_INVALID_PARA;
    } else {
        return NWY_SUCESS;
    }
}

static int nwy_get_ip_str(char *url_or_ip, char *ip_str, int *isipv6)
{
    char *str = NULL;
    nwy_ip_type_or_dns_enum ip_dns_type = NWY_CUSTOM_IP_TYPE_OR_DNS_NONE;

    ip_dns_type =nwy_judge_ip_or_dns(url_or_ip);
    if(ip_dns_type == NWY_CUSTOM_IP_TYPE_OR_DNS_DNS)
    {
      str = nwy_gethostbyname1(url_or_ip, isipv6);
      if(str == NULL || 0==strlen(str))
      {
          nwy_ext_echo("\r\ninput ip or url %s invalid\r\n", url_or_ip);
          return NWY_GEN_E_UNKNOWN;
      }
      memcpy(ip_str, str, strlen(str));

      nwy_ext_echo("\r\n%s get ip:%s\r\n", url_or_ip, ip_str);
    }
    else {
        memcpy(ip_str, url_or_ip, strlen(url_or_ip));
    }
    if (strchr(ip_str,':') != NULL) {
        *isipv6 = 1;
    } else {
        *isipv6 = 0;
    }
    return NWY_SUCESS;
}
void nwy_test_send_api()
{
  //int port=12177;
  //char *ip_str = "183.239.240.40";
  char ip_str[128] = {0};
  char ip_buf[16] = {0};
  int isipv6 = 0;
  int ret = NWY_GEN_E_UNKNOWN;
  int port = 0;
  nwy_ext_input_gets("\r\n input ipv4:");
  strcpy(ip_str, nwy_ext_sio_recv_buff);
  ret = nwy_get_ip_str(ip_str, ip_buf, &isipv6);
  if (ret != NWY_SUCESS) {
      nwy_ext_echo("\r\ninput ipv4 is error\r\n");
      return;
  }
  ret = nwy_hostname_check(ip_buf);
  if (ret != NWY_SUCESS) {
    nwy_ext_echo("\r\ninput ip or url is invalid");
    return;
  }
  nwy_ext_input_gets("\r\n input port:");
  port = atoi(nwy_ext_sio_recv_buff);
  nwy_ext_echo("\r\ntcp nwy_test_send_api start:\r\n");
  int socket= nwy_socket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  //int s= nwy_socket_open(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in sa;
  struct sockaddr_in *to4 = (struct sockaddr_in *)&sa;
  to4->sin_len = sizeof(struct sockaddr_in);
  to4->sin_family = AF_INET;
  to4->sin_port = htons(port);

  ip_addr_t addr;
  if (ipaddr_aton(ip_buf, &addr) == 0)
  {
    nwy_ext_echo("\r\ntcp test fail1:\r\n");
	nwy_socket_close(socket);
    return;
  }

  inet_addr_from_ip4addr(&to4->sin_addr, ip_2_ip4(&addr));

  //int on = 1;
  //nwy_socket_setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

  int opt = 1;
  nwy_socket_setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));

  if (0 != nwy_socket_set_nonblock(socket))
  {
    nwy_ext_echo("\r\nsocket set err\r\n");
    return;
  }

  uint64_t start = nwy_get_ms();
  do
  {
    int ret = nwy_socket_connect(socket, (struct sockaddr *)&sa, sizeof(sa));
    if((nwy_get_ms()-start)>= 10000 )
    {
      nwy_ext_echo("\r\nsocket connect timeout\r\n");
      nwy_socket_shutdown(socket, SHUT_RD);
      nwy_socket_close(socket);
      return;
    }

    if(ret == -1)
    {
      //nwy_ext_echo("errno=%d", nwy_socket_errno());
      if( EISCONN == nwy_socket_errno() )
      {
        nwy_ext_echo( "nwy_net_connect_tcp connect ok..");
        break;
      }

      if (EINPROGRESS != nwy_socket_errno() && EALREADY != nwy_socket_errno() )
      {
        nwy_ext_echo("connect errno = %d", nwy_socket_errno());
        nwy_socket_close(socket);
        return;
      }
    }
    nwy_sleep(100);
  }while(1);

  char send_buf[128] = {0};
  nwy_ext_input_gets("\r\n input send data (0 -128):");
  memcpy(send_buf, nwy_ext_sio_recv_buff, nwy_ext_sio_len);

  int send_len = nwy_socket_send(socket, send_buf, nwy_ext_sio_len, 0);
  if(send_len != nwy_ext_sio_len)
  {
    nwy_ext_echo("send len=%d, return len=%d", nwy_ext_sio_len,send_len);
	nwy_socket_close(socket);
    return;
  }
  nwy_ext_echo("send len=%d success\n", strlen(send_buf));
  nwy_socket_close(socket);
}


void nwy_tcp_test(void)
{
    static int s = 0;
    char url_or_ip[256] = {0};
    char ip_buf[256] = {0};
    int port=0;
    nwy_ip_type_or_dns_enum ip_dns_type =-1;
    char* sptr = nwy_ext_sio_recv_buff;
    int af_inet_flag = AF_INET;
    char *ip_str =NULL;
    int isipv6 =0;
    char buff[NWY_UART_RECV_SINGLE_MAX + 1] = {0};
    int on = 1;
    int opt = 1,ret = 0, send_len;
    uint64_t start ;
    ip_addr_t addr = {0};

    struct sockaddr_in sa_v4;
    struct sockaddr_in6 sa_v6;

    while (1)
    {
        nwy_ext_tcp_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        switch(atoi(sptr))
        {
            case 1:
            {
                if (0 == nwy_ext_check_data_connect()) {
                    nwy_ext_echo("\r\ndata call not setup");
                    break;
                }
                if(tcp_connect_flag)
                {
                    nwy_ext_echo("\r\ntcp alreay connect");
                    break;
                }
                memset(url_or_ip, 0, sizeof(url_or_ip));
                memset(ip_buf, 0, sizeof(ip_buf));
                nwy_ext_input_gets("\r\nPlease input url: ");
                strncpy(url_or_ip, sptr, strlen(sptr));
                nwy_ext_input_gets("\r\nPlease input port: ");
                if (nwy_port_get(sptr, &port)!= NWY_SUCESS) {
                    nwy_ext_echo("\r\nInput port error");
                    break;
                }

                nwy_ext_echo("\r\nurl test start:%s:%d\r\n", url_or_ip, port);
                ret = nwy_get_ip_str(url_or_ip, ip_buf, &isipv6);
                if (ret != NWY_SUCESS) {
                    break;
                }
                OSI_LOGXI(OSI_LOGPAR_SI,0, "nwy ip_buf= %s isipv6 = %d", ip_buf,isipv6);
                if (isipv6) {
                    if (ip6addr_aton( ip_buf,&addr.u_addr.ip6) == 0) {
                        nwy_ext_echo("\r\ninput ip or url is invalid");
                        break;
                    } else {

                        inet6_addr_from_ip6addr(&sa_v6.sin6_addr, ip_2_ip6(&addr));
                        sa_v6.sin6_len = sizeof(struct sockaddr_in);
                        sa_v6.sin6_family = AF_INET6;
                        sa_v6.sin6_port = htons(port);
                        af_inet_flag = AF_INET6;
                    }
                } else {
                    ret = nwy_hostname_check(ip_buf);
                    if (ret != NWY_SUCESS) {
                        nwy_ext_echo("\r\ninput ip or url is invalid");
                        break;
                    }

                    if (ip4addr_aton(ip_buf, &addr.u_addr.ip4) == 0)
                    {
                        nwy_ext_echo("\r\ninput ip error:\r\n");
                        break;
                    }
                    inet_addr_from_ip4addr(&sa_v4.sin_addr, ip_2_ip4(&addr));
                    sa_v4.sin_len = sizeof(struct sockaddr_in);
                    sa_v4.sin_family = AF_INET;
                    sa_v4.sin_port = htons(port);
                    af_inet_flag = AF_INET;
                }

                if (s == 0) {
                    s= nwy_socket_open(af_inet_flag, SOCK_STREAM, IPPROTO_TCP);
                }
                nwy_socket_setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));

                nwy_socket_setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
                if (0 != nwy_socket_set_nonblock(s))
                {
                    nwy_ext_echo("\r\nsocket set err\r\n");
                    break;
                }
                start = nwy_get_ms();
                do
                {

                    if (af_inet_flag == AF_INET6) {
                        ret = nwy_socket_connect(s, (struct sockaddr *)&sa_v6, sizeof(sa_v6));

                    } else {
                        ret = nwy_socket_connect(s, (struct sockaddr *)&sa_v4, sizeof(sa_v4));

                    }
                    if(ret)
                    {
                    }
                    if((nwy_get_ms()-start)>= 10000 )
                    {
                        nwy_ext_echo("\r\nsocket connect timeout\r\n");
                        nwy_socket_shutdown(s, SHUT_RD);
                        nwy_socket_close(s);
                        s = 0;
                        break;
                    }
                    if(ret == -1)
                    {
                        //nwy_ext_echo("errno=%d\r\n", nwy_socket_errno());
                        if( EISCONN == nwy_socket_errno() )
                        {
                            nwy_ext_echo( "\r\nnwy_net_connect_tcp connect ok..");
                            tcp_connect_flag = 1;
                            break;
                        }
                        if (EINPROGRESS != nwy_socket_errno() && EALREADY != nwy_socket_errno() )
                        {
                            nwy_ext_echo("\r\nconnect errno = %d", nwy_socket_errno());
                            nwy_socket_close(s);
                            s = 0;
                            tcp_connect_flag = 0;
                            break;
                        }
                        }
                    nwy_sleep(100);
                }while(1);
                if(tcp_connect_flag)
                {
                    tcp_recv_thread = nwy_create_thread("tcp_recv_thread",nwy_tcp_recv_func, (void*)&s, NWY_OSI_PRIORITY_NORMAL, 2048, 4);
                    if(tcp_recv_thread ==NULL)
                    {
                        nwy_ext_echo("\r\ncreate tcp recv thread failed, close connect");
                        nwy_socket_close(s);
                        s = 0;
                        tcp_connect_flag = 0;
                    }
                }
                break;
            }
            case 2:
            {
                if(!tcp_connect_flag)
                {
                    nwy_ext_echo("\r\ntcp not setup");
                    break;
                }
                memset(buff, 0, sizeof(buff));
                nwy_ext_input_gets("\r\nPlease input send data(<=512): ");
                if (nwy_ext_sio_len > NWY_UART_RECV_SINGLE_MAX) {
                    nwy_ext_echo("\r\nNo more than 512 bytes at a time to send ");
                    break;
                }
                memcpy(buff, sptr, nwy_ext_sio_len);
                send_len = nwy_socket_send(s, buff, nwy_ext_sio_len, 0);
                if(send_len != nwy_ext_sio_len)
                    nwy_ext_echo("\r\nsend len=%d, return len=%d", strlen(buff),send_len);
                else
                    nwy_ext_echo("\r\nsend ok");
                break;
            }
            case 3:
            {
                if(tcp_connect_flag)
                {
                    nwy_sleep(100);
                    nwy_socket_close(s);
                    s = 0;
                    tcp_connect_flag = 0;
                }
                nwy_ext_echo("\r\nclose tcp success");
                // kill phread
                break;
            }
            case 4:
                return;
            default:
                break;
        }
    }
}
static void nwy_udp_recv_func(void *param)
{
    char recv_buff[NWY_UART_RECV_SINGLE_MAX + 1] = {0};
    int recv_len =0, result = 0;
    int s = *(int *)param;
    nwy_osiEvent_t event = {0};
    fd_set rd_fd;
    fd_set ex_fd;
    FD_ZERO(&rd_fd);
    FD_ZERO(&ex_fd);
    FD_SET(s,&rd_fd);
    FD_SET(s,&ex_fd);
    struct timeval tv = {0};
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    while(1)
    {

        FD_ZERO(&rd_fd);
        FD_ZERO(&ex_fd);
        FD_SET(s,&rd_fd);
        FD_SET(s,&ex_fd);
        result = nwy_socket_select(s+1, &rd_fd, NULL,&ex_fd, &tv);
        if (result < 0)
        {
          nwy_ext_echo("\r\nudp select error:\r\n");
          nwy_socket_close(s);
          *(int *)param = 0;
          udp_connect_flag =0;
          nwy_exit_thread();
        }
        else if(result > 0)
        {
            if (FD_ISSET(s, &rd_fd))
            {
                memset(recv_buff, 0, NWY_UART_RECV_SINGLE_MAX + 1);
                recv_len = nwy_socket_recv(s, recv_buff, NWY_UART_RECV_SINGLE_MAX, 0);
                if(recv_len >0)
                    nwy_ext_echo("\r\nudp read:%d:%s\r\n", recv_len, recv_buff);
                else if(recv_len == 0)
                    nwy_ext_echo("\r\nudp close by server\r\n");
                else
                {
                    nwy_ext_echo("\r\nudp connection error\r\n");
                    nwy_socket_close(s);
                    *(int *)param = 0;
                    udp_connect_flag =0;
                    nwy_exit_thread();
                }
            }
            if (FD_ISSET(s, &ex_fd))
            {
                nwy_ext_echo("\r\nudp select ex_fd:\r\n");
                nwy_socket_close(s);
                *(int *)param = 0;
                udp_connect_flag =0;
                nwy_exit_thread();
            }
        }
        else
        {
            OSI_LOGI(0, "\r\nudp select timeout:\r\n");
        }//nwy_ext_echo("\r\nudp select timeout:\r\n");
    }
}


void nwy_ext_udp_menu()
{
  nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
  nwy_ext_echo("1. udp setup\r\n");
  nwy_ext_echo("2. udp send \r\n");
  nwy_ext_echo("3. udp close\r\n");
  nwy_ext_echo("4. Exit \r\n");
}

void nwy_udp_test(void)
{
  static int s = 0;
  char buff[NWY_UART_RECV_SINGLE_MAX + 1] = {0};
  char* sptr = nwy_ext_sio_recv_buff;
  nwy_ip_type_or_dns_enum ip_dns_type = -1;
  char url_or_ip[256] = {0};
  char ip_buf[256] = {0};
  int port=0;
  int af_inet_flag = AF_INET;
  int ret = -1;
  int isipv6 =0, send_len =0;
  struct sockaddr_in sa_v4;
  struct sockaddr_in6 sa_v6;
  ip_addr_t addr;
  uint64_t start = 0;
  while (1)
  {
    nwy_ext_udp_menu();
    nwy_ext_input_gets("\r\nPlease input option: ");
    switch(atoi(sptr))
    {
      case 1:
      {
        if (0 == nwy_ext_check_data_connect()) {
            nwy_ext_echo("\r\ndata call not setup");
            break;
        }
        if(udp_connect_flag)
        {
          nwy_ext_echo("\r\nudp alreay connect");
          break;
        }
        memset(url_or_ip, 0, sizeof(url_or_ip));
        memset(ip_buf, 0, sizeof(ip_buf));
        nwy_ext_input_gets("\r\nPlease input url/ipv4: ");
        strncpy(url_or_ip, sptr, strlen(sptr));
        nwy_ext_input_gets("\r\nPlease input port: ");
        if (nwy_port_get(sptr, &port)!= NWY_SUCESS) {
            nwy_ext_echo("\r\nInput port error");
            break;
        }

        port = atoi(sptr);

        nwy_ext_echo("\r\nurl test start:%s:%d\r\n", url_or_ip, port);
        ret = nwy_get_ip_str(url_or_ip, ip_buf, &isipv6);
        if (ret != NWY_SUCESS) {
            break;
        }

        if (isipv6) {
            if (ip6addr_aton( ip_buf,&addr.u_addr.ip6) != 1) {
                nwy_ext_echo("\r\ninput ip or url is invalid");
            } else {

                inet6_addr_from_ip6addr(&sa_v6.sin6_addr, ip_2_ip6(&addr));
                sa_v6.sin6_len = sizeof(struct sockaddr_in);
                sa_v6.sin6_family = AF_INET6;
                sa_v6.sin6_port = htons(port);
                af_inet_flag = AF_INET6;
            }
        } else {
            ret = nwy_hostname_check(ip_buf);
            if (ret != NWY_SUCESS) {
                nwy_ext_echo("\r\ninput ip or url is invalid");
                break;
            }

            if (ip4addr_aton(ip_buf, &addr.u_addr.ip4) == 0)
            {
                nwy_ext_echo("\r\ninput ip error:\r\n");
                break;
            }
            inet_addr_from_ip4addr(&sa_v4.sin_addr, ip_2_ip4(&addr));
            sa_v4.sin_len = sizeof(struct sockaddr_in);
            sa_v4.sin_family = AF_INET;
            sa_v4.sin_port = htons(port);
            af_inet_flag = AF_INET;
        }


        if (s == 0) {
            s= nwy_socket_open(af_inet_flag, SOCK_DGRAM, IPPROTO_UDP);
        }
        if (s < 0)
          nwy_ext_echo("\r\nUDP setup failed");
        else
        {
          udp_recv_thread = nwy_create_thread("udp_recv_thread",nwy_udp_recv_func, (void*)&s, NWY_OSI_PRIORITY_NORMAL, 2048, 4);
          if(udp_recv_thread ==NULL)
          {
              nwy_ext_echo("\r\ncreate udp recv thread failed");
              nwy_socket_close(s);
              s = 0;
          }
          else
          {
            udp_connect_flag =1;
            nwy_ext_echo("\r\nUDP setup successful");
          }
        }
        break;
      }
      case 2:
      {
        memset(buff, 0, sizeof(buff));
        nwy_ext_input_gets("\r\nPlease input data(<=512): ");
        if (nwy_ext_sio_len > NWY_UART_RECV_SINGLE_MAX) {
            nwy_ext_echo("\r\nNo more than 512 bytes at a time to send ");
            break;
        }
        memcpy(buff, sptr, nwy_ext_sio_len);
        if (af_inet_flag == AF_INET) {
            send_len= nwy_socket_sendto(s, buff, nwy_ext_sio_len, 0, (struct sockaddr *)&sa_v4, sizeof(sa_v4));
        } else if (af_inet_flag == AF_INET6) {
            send_len= nwy_socket_sendto(s, buff, nwy_ext_sio_len, 0, (struct sockaddr *)&sa_v6, sizeof(sa_v6));
        }
        if(send_len != nwy_ext_sio_len)
          nwy_ext_echo("\r\nsend len=%d, return len=%d", strlen(buff),send_len);
        else
          nwy_ext_echo("\r\nudp send success:\r\n");
        break;
      }
      case 3:
      {
        if(udp_connect_flag)
        {
          nwy_socket_close(s);
          s = 0;
          udp_connect_flag = 0;
        }
        nwy_ext_echo("\r\nclose UDP success");
        break;
      }
      case 4:
        return ;
      default:
        break;
    }
  }
}

/* Start Network test demo */

/*Begin: Add by novalis.wong for support network api test  in 2020.02.17*/
#ifdef FEATURE_NETWORK_TEST
#define MAX_INPUT_OPTION 4 /* contain '\r' '\n' */
#define STD_SUCCESS 1
#define STD_ERROR 0
#define NWY_IS_DIGIT( x )  ( ( ((x -'0') >= 0) && ((x-'0') <= 9) ) ? 1 : 0)
#define NWY_IS_HEXSTR( x )  ( ( (((x -'0') >= 0) && ((x-'0') <= 9)) || \
                                  ((x >= 'a') && (x <= 'f')) || \
                                  ((x >= 'A') && (x <= 'F'))) ? 1 : 0)

#define TBL_SIZE(tbl) (sizeof(tbl)/sizeof(tbl[0]))
// flag for scan finish.
static int scan_finish = 0;

static nwy_nw_net_scan_list_t nw_scan_list;
static const byte cmcc_plmn[][6] =
{
  "46000",
  "46002",
  "46004",
  "46007",
  "46008",
  "46013",
  "46020",
  "45412",
  "45413",
};
static const byte unicom_plmn[][6] =
{
  "46001",
  "46006",
  "46009",
  "46010",
};
static const byte ct_plmn[][6] =
{
  "46003",
  "46005",
  "46011",
  "46012",
  "46059",
  "45502",
  "45507",
};

/*
@func
    nwy_network_util_check_digit_string
@desc
    util function for check digit stirng
@param
    str: input characters string
@return
    0 : invalid digit string
    1 : valid digit string
@other
*/
static int nwy_network_util_check_digit_string(char *str)
{
    int i = 0;
    if(str == NULL)
        return 0;
    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] == '\r' || str[i] == '\n' || str[i] == '\0')
        {
            str[i] = '\0';
            break;
        }
        if (1 != NWY_IS_DIGIT (str[i]))
            return 0;
    }
    return 1;
}

/*
@func
    nwy_network_util_check_hex_string
@desc
    util function for check hex stirng
@param
    str: input characters string
@return
    0 : invalid hex string
    1 : valid hex string
@other
*/
static int nwy_network_util_check_hex_string(char *str)
{
    int i = 0;
    if(str == NULL)
        return 0;
    for (i = 0; i < strlen(str); i++)
    {
        if (str[i] == '\r' || str[i] == '\n')
        {
            str[i] = '\0';
            break;
        }
        if (1 != NWY_IS_HEXSTR(str[i]))
            return 0;
    }
    return 1;
}

/*
@func
    nwy_network_util_get_oper_type
@desc
    get current operator type from plmn string
@param
    plmn_buf: input plmn string
@return
    0 : CMCC
    1 : CU
    2 : CT
    3 : OTHER
@other
*/
int nwy_network_util_get_oper_type(char *plmn_buf)
{
  int i;
  if (plmn_buf == NULL)
    return 3;
  for(i = 0; i < TBL_SIZE(cmcc_plmn); i++)
  {
    if(strncmp(plmn_buf, (char*)cmcc_plmn[i], strlen((char*)cmcc_plmn[i])) == 0)
    {
      return 0;
    }
  }

  for(i = 0; i < TBL_SIZE(unicom_plmn); i++)
  {
    if(strncmp(plmn_buf, (char*)unicom_plmn[i], strlen((char*)unicom_plmn[i])) == 0)
    {
      return 1;
    }
  }

  for(i = 0; i < TBL_SIZE(ct_plmn); i++)
  {
    if(strncmp(plmn_buf, (char*)ct_plmn[i], strlen((char*)ct_plmn[i])) == 0)
    {
      return 2;
    }
  }
  return 3;
}

static void nwy_network_test_help()
{
    /*
        0 - nwy_nw_get_register_info //
        1 - nwy_nw_get_network_mode //
        2 - nwy_nw_set_network_mode //
        3 - nwy_nw_get_forbidden_plmn //
        4 - nwy_nw_manual_network_scan & nwy_nw_manual_network_select
        5 - nwy_nw_band_lock
        6 - nwy_nw_freq_lock
        7 - nwy_nw_get_IMS_state
        8 - nwy_nw_set_IMS_state
        9 - nwy_nw_get_default_pdn_info
        10 - nwy_nw_get_signal_rssi
        11 - nwy_nw_get_netmsg
        12 - nwy_nw_get_cfgdftpdn_info
        13 - nwy_nw_set_cfgdftpdn_info
        14 - nwy_nw_get_netsharemode
        15 - nwy_nw_set_netsharemode
        16 - nwy_nw_get_netshareact
        17 - nwy_nw_set_netshareact
        18 - nwy_nw_get_netshareact quick test
        19 - nwy_nw_set_netshareact quick test
        20 - nwy_nw_set_radio_st
        21 - nwy_nw_get_radio_st
        q - exit network test.
        other - help information
    */
    nwy_ext_echo("\r\n========================================");
    nwy_ext_echo("\r\nNetwork Test Help Information:\n");
    nwy_ext_echo("0: Get netwrok information\r\n");
    nwy_ext_echo("1: Get network mode \r\n");
    nwy_ext_echo("2: Set network mode \r\n");
    nwy_ext_echo("3: Get Forbidden netwrok list \r\n");
    nwy_ext_echo("4: Manual network scan \r\n");
    nwy_ext_echo("5: Set network band lock\r\n");
    nwy_ext_echo("6: Set network freq lock\r\n");
    nwy_ext_echo("7: Get network IMS state\r\n");
    nwy_ext_echo("8: Set network IMS state\r\n");
    nwy_ext_echo("9: Get default pdn info\r\n");
    nwy_ext_echo("10: Get network signal rssi\r\n");
    nwy_ext_echo("11: Get network info\r\n");
    nwy_ext_echo("12: Get CFGDFTPDN info\r\n");
    nwy_ext_echo("13: Set CFGDFTPDN info\r\n");
    nwy_ext_echo("14: Get netsharemode info\r\n");
    nwy_ext_echo("15: Set netsharemode info\r\n");
    nwy_ext_echo("16: Get netshareact info\r\n");
    nwy_ext_echo("17: Set netshareact info\r\n");
    nwy_ext_echo("18: enable netshareact info quick test\r\n");
    nwy_ext_echo("19: disable netshareact info quick test\r\n");
    nwy_ext_echo("20: set radio state\r\n");
    nwy_ext_echo("21: get radio state\r\n");
    nwy_ext_echo("22: scan cell info\r\n");
    //nwy_ext_echo("h: Help information message\r\n");
    nwy_ext_echo("q: Quit network test\r\n");
    nwy_ext_echo("\r\n========================================");
}

/*
@func
    nwy_network_test_scan_cb
@desc
    network manual scan callback func
    call by nwy_nw_manual_network_scan()
@param
    net_list: output network scan result point
@return
    NONE
@other
*/
static void nwy_network_test_scan_cb (
    nwy_nw_net_scan_list_t *net_list
)
{
    int i;
    if (net_list == NULL)
        return;
    if (net_list->result != 1)
        nwy_ext_echo("\r\nManual network Scan failed\r\n");
    else
    {
        nwy_ext_echo("\r\nManual Scan Result: %d",net_list->num);
        for (i = 0; i < net_list->num; i++)
        {
            nwy_ext_echo("\r\nIndex: %d\r\n", i);
            nwy_ext_echo("MCC-MNC:%s-%s\r\n", net_list->net_list[i].net_name.mcc, net_list->net_list[i].net_name.mnc);
            nwy_ext_echo("Long EONS:%s\r\n", net_list->net_list[i].net_name.long_eons);
            nwy_ext_echo("Short EONS:%s\r\n", net_list->net_list[i].net_name.short_eons);
            nwy_ext_echo("Net Status:%s\r\n", scan_net_status[net_list->net_list[i].net_status]);
            nwy_ext_echo("Net Rat:%s\r\n", scan_net_rat[net_list->net_list[i].net_rat]);
        }
    }
    nwy_ext_echo("\r\nStart Test Select Network...\r\n");
    memset(&nw_scan_list,0x00,sizeof(nw_scan_list));
    memcpy(&nw_scan_list,net_list, sizeof(nwy_nw_net_scan_list_t));
    scan_finish = 1;
}

/*
@func
    nwy_network_test_nw_select
@desc
    network manual select function.
@param
    NONE
@return
    NONE
@other
    User can only choose insert SIM/USIM operator's network,
    and if choose not support network will cause UE crash.
*/
void nwy_network_test_nw_select()
{
    nwy_sim_result_type imsi = {0};
    char plmn_str[10] = "";
    int i = 0;
    int opertor_type = 3;
    char sub_opt[10] = "";
    int id = 0;
    nwy_nw_net_select_param_t param = {0};
    // 1. Get current sim imsi for operator type
    if (nwy_sim_get_imsi(&imsi) == NWY_RES_OK )
    {
        nwy_sleep(1000); //wait for imsi result
        //1.1 Phase imsi to Operator Type.
        opertor_type = nwy_network_util_get_oper_type((char *)imsi.imsi);
        nwy_ext_echo("\r\nOperator type: %d(0-CMCC,1-CU,2-CT,3-OTHER),Current netwrok list: ", opertor_type);
        //2. Get scan result's operator type and compare with current operator type.
        for (i = 0; i < nw_scan_list.num; i++)
        {
            memset(plmn_str,0x00,sizeof(plmn_str));
            sprintf(plmn_str, "%s%s", nw_scan_list.net_list[i].net_name.mcc,
                                     nw_scan_list.net_list[i].net_name.mnc);
            //2.1 only show user available select netwrok list.
            if (nwy_network_util_get_oper_type(plmn_str) == opertor_type)
            {
                nwy_ext_echo("\r\nIndex:%d Rat:%d ", i,nw_scan_list.net_list[i].net_rat);
                nwy_ext_echo("MCC-MNC:%s-%s", nw_scan_list.net_list[i].net_name.mcc, nw_scan_list.net_list[i].net_name.mnc);
            }
        }
        nwy_ext_input_gets("\r\nPlease select index at before table:");
        if (strlen((char *)nwy_ext_sio_recv_buff) > 5)
            return;
        memset(sub_opt, 0x00, sizeof(sub_opt));
        memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
        if (STD_SUCCESS != nwy_network_util_check_digit_string(sub_opt))
            return;
        id = atoi(sub_opt);
        memcpy(param.mcc, nw_scan_list.net_list[id].net_name.mcc, sizeof(param.mcc));
        memcpy(param.mnc,nw_scan_list.net_list[id].net_name.mnc, sizeof(param.mnc));
        param.net_rat = nw_scan_list.net_list[id].net_rat;
        //3. start manual select network.
        nwy_nw_manual_network_select(&param);
        nwy_ext_input_gets("\r\nSelect index %d network over!!!",id);
    }
    else
    {
        nwy_ext_input_gets("\r\nGet Current operator type FAILED!!!");
    }
    return;
}

/*
@func
    nwy_cellinfo_scan_cb
@desc
    network manual scan callback func
    call by nwy_nw_manual_network_scan()
@param
    net_list: output network scan result point
@return
    NONE
@other
*/
static void nwy_cellinfo_scan_cb (void *infor, int num)
{
    nwy_scanned_locator_info_t *cellinfo = (nwy_scanned_locator_info_t *)infor;

    int i;
    if (cellinfo->num == 0)
    {
        nwy_ext_echo("\r\nCell info Scan failed\r\n");
        return;
    }
    if (cellinfo->curr_rat == 4)
    {
        nwy_ext_echo("\r\nLte Serving cell: mcc:%d,mnc:%d,band:%d,arfcn:%d,tac:%x,cellid:%x,pci:%d,rsrp:%d,rsrq:%d,sinr:%d\r\n",
                         cellinfo->scell_info.lte_scell_info.mcc, cellinfo->scell_info.lte_scell_info.mnc,
                         cellinfo->scell_info.lte_scell_info.bandInfo, cellinfo->scell_info.lte_scell_info.dlEarfcn,
                         cellinfo->scell_info.lte_scell_info.tac, cellinfo->scell_info.lte_scell_info.cellId,
                         cellinfo->scell_info.lte_scell_info.pcid, cellinfo->scell_info.lte_scell_info.rsrp,
                         cellinfo->scell_info.lte_scell_info.rsrq, cellinfo->scell_info.lte_scell_info.SINR);
        for (i = 0; i < cellinfo->ncell_info.ncell_num; i++)
        {
            nwy_ext_echo("\r\nLte Neighbour cell %d: mcc:%d,mnc:%d,arfcn:%d,tac:%x,cellid:%x,pci:%d,rsrp:%d,rsrq:%d\r\n", i,
                         cellinfo->ncell_info.lte_ncell_info[i].mcc, cellinfo->ncell_info.lte_ncell_info[i].mnc,
                         cellinfo->ncell_info.lte_ncell_info[i].frequency, cellinfo->ncell_info.lte_ncell_info[i].tac,
                         cellinfo->ncell_info.lte_ncell_info[i].cellId, cellinfo->ncell_info.lte_ncell_info[i].pcid,
                         cellinfo->ncell_info.lte_ncell_info[i].rsrp, cellinfo->ncell_info.lte_ncell_info[i].rsrq);
        }
    }
    else if (cellinfo->curr_rat == 2)
    {
        nwy_ext_echo("\r\nGsm Serving cell: mcc:%d,mnc:%d,band:%d,arfcn:%d,lac:%x,cellid:%x,bsic:%d,rssi:%d,RxQualFull:%d\r\n",
                         cellinfo->scell_info.gsm_scell_info.mcc, cellinfo->scell_info.gsm_scell_info.mnc,
                         cellinfo->scell_info.gsm_scell_info.CurrBand, cellinfo->scell_info.gsm_scell_info.Arfcn,
                         cellinfo->scell_info.gsm_scell_info.Lac, cellinfo->scell_info.gsm_scell_info.Cellid,
                         cellinfo->scell_info.gsm_scell_info.Bsic,cellinfo->scell_info.gsm_scell_info.rssi,
                         cellinfo->scell_info.gsm_scell_info.RxQualFull);
        for (i = 0; i < cellinfo->ncell_info.ncell_num; i++)
        {
            nwy_ext_echo("\r\nGsm Neighbour cell %d: mcc:%d,mnc:%d,arfcn:%d,lac:%x,cellid:%x,bsic:%d,rssi:%d\r\n", i,
                         cellinfo->ncell_info.gsm_ncell_info[i].mcc, cellinfo->ncell_info.gsm_ncell_info[i].mnc,
                         cellinfo->ncell_info.gsm_ncell_info[i].Arfcn, cellinfo->ncell_info.gsm_ncell_info[i].lac,
                         cellinfo->ncell_info.gsm_ncell_info[i].Cellid, cellinfo->ncell_info.gsm_ncell_info[i].Bsic,
                         cellinfo->ncell_info.gsm_ncell_info[i].rssi);
        }
    }
    else
        nwy_ext_echo("\r\nCell info Scan failed\r\n");
}

void nwy_network_test_main()
{
    char option[10] = "";
    char sub_opt[10] = "";
    int i = 0;
    nwy_nw_regs_info_type_t reg_info = {0};
    nwy_nw_operator_name_t opt_name = {0};
    uint8_t csq_val = 99;
    nwy_nw_mode_type_t network_mode = NWY_NW_MODE_NONE;
    nwy_nw_fplmn_list_t fplmn_list = {0};
    uint32_t act = 0;
    char set_band[64] = "";
    int freq_num = 0;
    uint32_t nfreq[9] = {0};
    uint8_t ims_enable = 0;
    int lac = 0;
    int cid = 0;;

    while(1)
    {
        //show help information first
        nwy_network_test_help();
        nwy_ext_input_gets("\r\nPlease select options :");

        if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION) {
            //nwy_network_test_help();
            nwy_ext_echo("ERROR not this option!!");
            continue ;
        }
        memset(option, 0, sizeof(option));
        memcpy(option, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
        /*1. special for exit*/
        if ( (strncmp(option, "q" ,strlen(option) ) == 0) || \
           ( option[0] == 'q') )
        { // exit
            return;
        }
        /*2. normal test */
        switch(atoi(option))
        {
        case 0: // register info test
            memset(&reg_info,0x00,sizeof(reg_info));
            if (NWY_RES_OK == nwy_nw_get_register_info(&reg_info))
            {
                nwy_ext_echo("\r\n========================================\r\n");
                if (reg_info.data_regs_valid == 1)
                {
                    nwy_ext_echo("Network Data Reg state: %d\r\n"
                                 "Network Data Roam state: %d\r\n"
                                 "Network Data Radio Tech: %d\r\n",
                                    reg_info.data_regs.regs_state,
                                    reg_info.data_regs.roam_state,
                                    reg_info.data_regs.radio_tech);
                }
                if (reg_info.voice_regs_valid == 1)
                {
                    nwy_ext_echo("Network Voice Reg state: %d\r\n"
                                 "Network Voice Roam state: %d\r\n"
                                 "Network Voice Radio Tech: %d\r\n",
                                    reg_info.voice_regs.regs_state,
                                    reg_info.voice_regs.roam_state,
                                    reg_info.voice_regs.radio_tech);
                }

                // Get operator name
                if (reg_info.voice_regs.regs_state != NWY_NW_SERVICE_NONE)
                {
                    memset(&opt_name,0x00,sizeof(opt_name));
                    if (NWY_SUCCESS == nwy_nw_get_operator_name(&opt_name))
                    {
                        nwy_ext_echo("Long EONS: %s\r\n"
                                     "Short EONS: %s\r\n"
                                     "MCC and MNC: %s %s\r\n"
                                     "SPN: %s\r\n",
                                     (char *)opt_name.long_eons,
                                     (char *)opt_name.short_eons,
                                     opt_name.mcc,
                                     opt_name.mnc,
                                     opt_name.spn);
                    }
                }

                //Get CSQ
                nwy_nw_get_signal_csq(&csq_val);
                nwy_ext_echo("\r\nCSQ is %d \r\n",csq_val);
                //Get Lac and CELL ID
                nwy_sim_get_lacid(&lac, &cid);
                nwy_ext_echo("\r\nLAC: %X, CELL_ID: %X \r\n", lac, cid);
                nwy_ext_echo("\r\n========================================\r\n");
            }
            else
            {
                nwy_ext_echo("\r\nGet Register Information Failed!!!\r\n");
            }
        break;

        case 1: /*Get network mode*/
            if (NWY_SUCCESS == nwy_nw_get_network_mode(&network_mode))
            {
                nwy_ext_echo("\r\n========================================\r\n");
                if (network_mode == NWY_NW_MODE_GSM)
                    nwy_ext_echo("\r\nCurrent Network mode: GSM (%d)\r\n",network_mode);
                if (network_mode == NWY_NW_MODE_LTE)
                    nwy_ext_echo("\r\nCurrent Network mode: LTE (%d)\r\n",network_mode);
                nwy_ext_echo("\r\n========================================\r\n");
            }
            else
            {
                nwy_ext_echo("\r\nGet Network mode Failed!!!\r\n");
            }
        break;
        case 2:/*Set network mode*/
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_ext_input_gets("\r\nPlease select network mode (0 - AUTO,2 - GSM,4 - LTE):");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            if (STD_SUCCESS != NWY_IS_DIGIT (sub_opt[0]))
                break;
            if ((sub_opt[0] == '0') || (sub_opt[0] == '2') || (sub_opt[0] == '4'))
            {
                nwy_nw_set_network_mode(sub_opt[0] - '0');
                nwy_ext_echo("\r\nSet network mode over\r\n");
            }
            else
            {
                nwy_ext_echo("\r\nInput Wrong network mode!!!\r\n");
            }
            nwy_ext_echo("\r\n========================================\r\n");
        break;
        case 3: /*Get FPLMN list*/
            memset(&fplmn_list,0x00,sizeof(fplmn_list));
            nwy_ext_echo("\r\n========================================\r\n");
            if (NWY_SUCCESS == nwy_nw_get_forbidden_plmn(&fplmn_list))
            {
                nwy_ext_echo("\r\nGet FPLMN list %d",fplmn_list.num);
                for (i= 0;i < fplmn_list.num; i++)
                {
                    nwy_ext_echo("\r\n%s - %s",fplmn_list.fplmn[i].mcc,fplmn_list.fplmn[i].mnc);
                }
                nwy_ext_echo("\r\n");
            }
            else
            {
                nwy_ext_echo("\r\nGet FPLMN List Failed!!!\r\n");
            }
            nwy_ext_echo("\r\n========================================\r\n");
        break;
        case 4: /*Test Manual scan and select network*/
            nwy_ext_echo("\r\n========================================");
            if (NWY_SUCCESS == nwy_nw_manual_network_scan( nwy_network_test_scan_cb ))
            {
                nwy_ext_echo("\r\nWaiting Scan result...");
                scan_finish = 0;
                while(1)
                {
                    if (scan_finish  == 1)
                        break;
                    nwy_sleep(1000);
                }
                if (nw_scan_list.result == STD_SUCCESS)
                {
                    nwy_network_test_nw_select();
                }
            }
            else
            {
                nwy_ext_echo("\r\nScan network Failed!!!\r\n");
            }
            nwy_ext_echo("\r\n========================================\r\n");
        break;
        case 5: /*BANDLOCK TEST*/
            nwy_ext_echo("\r\n========================================\r\n");
            //1. Get rat mode
            nwy_ext_input_gets("\r\nPlease set rat mode(2-GSM,4-LTE): ");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            if (STD_SUCCESS != NWY_IS_DIGIT (sub_opt[0]))
                break;
            act = sub_opt[0] - '0';
            //2. Get bandlock mask
            nwy_ext_input_gets("\r\nPlease set bandlock mask(limit 256bits): ");
            if (strlen((char *)nwy_ext_sio_recv_buff) > 64)
                break;
            memset(set_band, 0, sizeof(set_band));
            memcpy(set_band, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            if (STD_SUCCESS != nwy_network_util_check_hex_string(set_band))
                break;
            //3. Set BADNLOCK
            nwy_nw_band_lock(act, set_band);
            nwy_ext_echo("\r\n========================================\r\n");
        break;
        case 6:/*NFREQLOCK TEST*/
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_ext_input_gets("\r\nPlease enter freq count(max,9):");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            if (STD_SUCCESS != NWY_IS_DIGIT (sub_opt[0]))
                break;
            freq_num = sub_opt[0]  -'0';
            if (freq_num > 9 || freq_num < 0)
                break;
            for (i = 0; i < freq_num; i++)
            {
                nwy_ext_input_gets("\r\nPlease enter freq number(max,65535): ");
                if (strlen((char *)nwy_ext_sio_recv_buff) > 6 + 2)
                    break;
                memset(sub_opt, 0, sizeof(sub_opt));
                memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
                if (STD_SUCCESS != nwy_network_util_check_digit_string(sub_opt))
                    break;
                if (65535 > atoi(sub_opt))
                    nfreq[i] = atoi(sub_opt);
                else
                    nfreq[i] = 65535;
                OSI_LOGI(0, "=network= freq[%d]:%d", i, nfreq[i]);
            }
            if (NWY_SUCCESS == nwy_nw_freq_lock(nfreq,freq_num))
                nwy_ext_echo("\r\nlock freq success\r\n");
            else
                nwy_ext_echo("\r\nlock freq fail\r\n");
             nwy_ext_echo("\r\n========================================\r\n");
        break;
        case 7: /*Get IMS state*/
            nwy_nw_get_IMS_state(&ims_enable);
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_ext_echo("\r\nIMS state %d",ims_enable);
            nwy_ext_echo("\r\n========================================\r\n");
        break;
        case 8:/*Set IMS state*/
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_ext_input_gets("\r\nPlease set IMS state(0-off,1-on):");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            if (STD_SUCCESS != NWY_IS_DIGIT (sub_opt[0]))
                break;
            //1. before set IMS open  need set LTE only mode first.
            if (sub_opt[0] == '1')
            {
                nwy_nw_set_network_mode(4);
                nwy_sleep(2000);
            }
            //2. set IMS state
            if (NWY_SUCCESS == nwy_nw_set_IMS_state(sub_opt[0] - '0'))
            {
                nwy_ext_echo("\r\nIMS SET Success!!!\r\n");
            }
            else
            {
                nwy_ext_echo("\r\nIMS SET Failed!!!\r\n");
            }
            nwy_ext_echo("\r\n========================================\r\n");
        break;
		/*Begin: Add by yjj for OPEN_SDK_API get default pdn apn in 2020.09.04*/
        case 9:/*Get Default pdn info*/
        {
          char* apn = nwy_nw_get_default_pdn_apn();
          if (apn == NULL)
            nwy_ext_echo("\r\nGet Default Pdn Info Failed!!\r\n");
          else
            nwy_ext_echo("\r\nDefault Pdn Apn:%s\r\n",apn);
          break;
        }
		/*End: Add by yjj for OPEN_SDK_API get default pdn apn in 2020.09.04*/
        /*Begin:Add by huangweigang for get rssi in 2020.11.14*/
        case 10:
        {
            nwy_ext_echo("\r\n========================================\r\n");
            uint8_t rssi = 0;
            if (NWY_SUCCESS ==  nwy_nw_get_signal_rssi(&rssi))
                nwy_ext_echo("RSSI: %d", rssi);
            else
                nwy_ext_echo("Get RSSI Failed!");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        /*End:Add by huangweigang for get rssi in 2020.11.14*/
        /*Begin:Add by huangweigang for get NET info in 2020.11.14*/
        case 11:
        {
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_serving_cell_info *pNetmsg = (nwy_serving_cell_info *)malloc(sizeof(nwy_serving_cell_info));
            memset(pNetmsg, 0x00, sizeof(nwy_serving_cell_info));
            if (NWY_SUCCESS == nwy_nw_get_netmsg(pNetmsg))
            {
                if (pNetmsg->curr_rat == 4)
                    nwy_ext_echo("LTE NETMSG: CELL_ID=%X, PCI=%d, RSRQ=%d, RSRP=%d, SINR=%d, dlBler=%d, ulBler=%d\r\n", pNetmsg->cellId,
                        pNetmsg->pcid, pNetmsg->rsrq, pNetmsg->rsrp, pNetmsg->SINR, pNetmsg->dlBler, pNetmsg->ulBler);
                else if (pNetmsg->curr_rat == 2)
                    nwy_ext_echo("GSM NETMSG: CELL_ID=%X, PCI=%d, TxPwr=%d, RxPwr=%d, Bler=%d\r\n", pNetmsg->cellId, pNetmsg->pcid,
                        pNetmsg->txPower, pNetmsg->rxPower, pNetmsg->ulBler);
                else
                    nwy_ext_echo("\r\nGet NETMSG Failed!!!\r\n");
            }
            else
            {
                nwy_ext_echo("\r\nGet NETMSG Failed!!!\r\n");
            }
            free(pNetmsg);
            pNetmsg = NULL;
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        /*Begin:Add by zhanghao for get and set CFGDFTPDN info in 2020.11.19*/
        case 12: /* Get CFGDFTPDN */
        {
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_nw_cfgdftpdn_t cfgdftpdn_get_info;
            memset(&cfgdftpdn_get_info, 0, sizeof(nwy_nw_cfgdftpdn_t));
            int ret = nwy_nw_get_cfgdftpdn_info(&cfgdftpdn_get_info);
            if (ret == NWY_ERROR)
            {
                nwy_ext_echo("nwy_nw_get_cfgdftpdn_info error!");
                break;
            }
            nwy_ext_echo("+CFGDFTPDN: pdpType: %d, authPort: %d, apn: %s, username: %s, password: %s", cfgdftpdn_get_info.pdpType,
                         cfgdftpdn_get_info.authProt, cfgdftpdn_get_info.apn, cfgdftpdn_get_info.userName, cfgdftpdn_get_info.password);
            nwy_ext_echo("\r\n get cfgdftpdn ok!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        case 13: /* Set CFGDFTPDN */
        {
            int pdpType,authType;
            nwy_nw_cfgdftpdn_t cfgdftpdn_info;
            memset(&cfgdftpdn_info, 0, sizeof(nwy_nw_cfgdftpdn_t));

            nwy_ext_echo("\r\n========================================\r\n");
            /* get parameter */
            nwy_ext_input_gets("\r\nPlease select pdpType mode (1 - IP, 2 - IPV6, 3 - IPV4V6):");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            pdpType = sub_opt[0] - '0';
            if ((pdpType != 3) && (pdpType !=1) && (pdpType !=2)){
              nwy_ext_echo("\r\nInvaild pdp_type value: %d\r\n",pdpType );
              break;
            }
            cfgdftpdn_info.pdpType = pdpType;

            nwy_ext_input_gets("\r\nPlease select auth mode (0 - NONE, 1 - PAP , 2 - CHAP):");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            authType = sub_opt[0] - '0';
            if ((authType < 0) || (authType > 2)){
              nwy_ext_echo("\r\nInvaild auth_type value: %d\r\n",authType);
              break;
            }
            cfgdftpdn_info.authProt = authType;

            nwy_ext_input_gets("\r\nPlease select apn maxlen is 99:");
            if (strlen((char *)nwy_ext_sio_recv_buff) > 99)
                break;
            memcpy(cfgdftpdn_info.apn, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));


            nwy_ext_input_gets("\r\nPlease select username maxlen is 64:");
            if (strlen((char *)nwy_ext_sio_recv_buff) > 64)
                break;
            memcpy(cfgdftpdn_info.userName, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));

            nwy_ext_input_gets("\r\nPlease select password maxlen is 64:");
            if (strlen((char *)nwy_ext_sio_recv_buff) > 64)
                break;
            memcpy(cfgdftpdn_info.password, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));

            nwy_ext_echo("\r\n+CFGDFTPDN: pdpType: %d, authPort: %d, apn: %s, username: %s, password: %s", cfgdftpdn_info.pdpType,
                         cfgdftpdn_info.authProt, cfgdftpdn_info.apn,cfgdftpdn_info.userName, cfgdftpdn_info.password);

            int ret = nwy_nw_set_cfgdftpdn_info(cfgdftpdn_info);
            nwy_ext_echo("\r\n set cfgdftpdn ok!\r\n");
            nwy_ext_echo("\r\n================================\r\n");
            break;
        }
        /*end:Add by zhanghao for get and set CFGDFTPDN info in 2020.11.19*/
        /*Begin: Add by zhangtengwei for OPEN_SDK_API open ECM  in 2021.1.15*/
        case 14: /* Get netsharemode */
        {
            nwy_ext_echo("\r\n========================================\r\n");
            int ret = nwy_nw_get_netsharemode();

            if (ret == NWY_ERROR)
            {
                nwy_ext_echo("nwy_nw_get_netsharemode() error!");
                break;
            }
            nwy_ext_echo("get netsharemode=%d (0=RNDIS,1=ECM)",ret);
            nwy_ext_echo("\r\n get netsharemode ok!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        case 15: /* Set netsharemode */
        {
            int input_netshare_mode;
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_ext_input_gets("\r\nPlease select netsharemode (0 = RNDIS,1 = ECM):");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            input_netshare_mode = sub_opt[0] - '0';
            if ((input_netshare_mode < 0) || (input_netshare_mode > 1)){
              nwy_ext_echo("\r\nInvaild input_netshare_mode : %d\r\n",input_netshare_mode);
              break;
            }
            int ret = nwy_nw_set_netsharemode(input_netshare_mode);

            if (ret == NWY_ERROR)
            {
                nwy_ext_echo("nwy_nw_set_netsharemode() error!");
                break;
            }
            nwy_ext_echo("\r\n set netsharemode=%d (0=RNDIS,1=ECM)",input_netshare_mode);
            nwy_ext_echo("\r\n set netsharemode ok!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        case 16: /* Get netshareact */
        {
            nwy_ext_echo("\r\n========================================\r\n");
            int action,auto_connect,ip_type,share_mode;
            char apn[100]={0};
            char user_name[65]={0};
            char passwd[65]={0};
            int ret = nwy_nw_get_netshareact(&action,&auto_connect,apn,user_name,passwd,&ip_type,&share_mode);

            if (ret == NWY_ERROR)
            {
                nwy_ext_echo("nwy_nw_get_netsharemode() error!");
                break;
            }
            nwy_ext_echo("get netshareact action=%d, auto_connect=%d, apn=%s, user_name=%s,passwd=%s,ip_type=%d,share_mode=%d (0=RNDIS,1=ECM)",action,auto_connect,apn,user_name,passwd,ip_type,share_mode);
            nwy_ext_echo("\r\n get netshareact ok!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
         case 17: /* Set netshareact */
        {
            int ret=-1;
            int ucid,action,auto_connect,auth_type,ip_type;
            char apn[100]={0};
            char user_name[65]={0};
            char passwd[65]={0};

            nwy_ext_echo("\r\n========================================\r\n");
            nwy_ext_input_gets("\r\nPlease input profile info: profile_id <2-7>");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            ucid = sub_opt[0] - '0';
            if ((ucid < 2) || (ucid > 7)){
              nwy_ext_echo("\r\nInvaild ucid value: %d\r\n",ucid);
              break;
            }

            nwy_ext_input_gets("\r\nPlease input netshare action :  <0-close,1-open>");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            action = sub_opt[0] - '0';
            if ((action < 0) || (action > 1)){
              nwy_ext_echo("\r\nInvaild action value: %d\r\n",action);
              break;
            }

            nwy_ext_input_gets("\r\nPlease input netshare auto_connect info: <0-disable,1-enable>");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            auto_connect = sub_opt[0] - '0';
            if ((action < 0) || (action > 1)){
              nwy_ext_echo("\r\nInvaild auto_connect value: %d\r\n",auto_connect);
              break;
            }

            nwy_ext_input_gets("\r\nPlease input  apn (length 0-%d)\r\n",NWY_APN_MAX_LEN);
            if (strlen((char *)nwy_ext_sio_recv_buff) > (NWY_APN_MAX_LEN))
                break;
            memset(apn, 0, sizeof(apn));
            memcpy(apn, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));

            nwy_ext_input_gets("\r\nPlease input  profile info: user name (length 0-%d)\r\n",NWY_APN_USER_MAX_LEN);
            if (strlen((char *)nwy_ext_sio_recv_buff) > (NWY_APN_USER_MAX_LEN))
                break;
            memset(user_name, 0, sizeof(user_name));
            memcpy(user_name, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));

            nwy_ext_input_gets("\r\nPlease input  profile info:  password (length 0-%d)\r\n",NWY_APN_PWD_MAX_LEN);
            if (strlen((char *)nwy_ext_sio_recv_buff) > (NWY_APN_PWD_MAX_LEN))
                break;
            memset(passwd, 0, sizeof(passwd));
            memcpy(passwd, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));

            nwy_ext_input_gets("\r\nPlease input profile info: auth_type <0-3>\r\n 0:NONE, 1:PAP, 2:CHAP 3.PAP or CHAP\r\n");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            auth_type = sub_opt[0] - '0';
            if ((auth_type < 0) || (auth_type > 3)){
              nwy_ext_echo("\r\nInvaild auth_type value: %d\r\n",auth_type);
              break;
            }

            nwy_ext_input_gets("\r\nPlease input profile info: pdp_type <1-3>\r\n1:IPV4, 2:IPV6, 3:IPV4V6\r\n");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            ip_type = sub_opt[0] - '0';
            if ((ip_type != 3) && (ip_type !=1) && (ip_type !=2)){
              nwy_ext_echo("\r\nInvaild pdp_type value: %d\r\n",ip_type );
              break;
            }

            ret=nwy_nw_set_netshareact(ucid,action,auto_connect,apn,user_name ,passwd,auth_type,ip_type);

            if (ret == NWY_ERROR)
            {
                nwy_ext_echo("nwy_nw_set_netsharemode() error!");
                break;
            }
            nwy_ext_echo("set netshareact action=%d, auto_connect=%d, apn=%s, user_name=%s,passwd=%s ,auth_type=%d,ip_type=%d",action,auto_connect,apn,user_name,passwd,auth_type,ip_type);
            nwy_ext_echo("\r\n set netshareact ok!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        case 18: /* enable netshareact quick test */
        {
            nwy_ext_echo("\r\n========================================\r\n");
            int ret=nwy_nw_set_netshareact(2,1,1,"3gnet","card","card",1,1);

            if (ret == NWY_ERROR)
            {
                nwy_ext_echo("enable netshareact quick test error!");
                break;
            }
            nwy_ext_echo("nwy_nw_set_netshareact(2,1,1,\"3gnet\",\"card\",\"card\",1,1)(ucid,action,auto_connect, apn, user_name,passwd,auth_type,ip_type)");
            nwy_ext_echo("\r\n enable netshareact quick test ok!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        case 19: /* disable netshareact quick test */
        {
            nwy_ext_echo("\r\n========================================\r\n");

            int ret=nwy_nw_set_netshareact(2,0,0,NULL,NULL,NULL,0,1);

            if (ret == NWY_ERROR)
            {
                nwy_ext_echo("disable netshareact quick test error!");
                break;
            }
            nwy_ext_echo("nwy_nw_set_netshareact(2,0,0,NULL,NULL,NULL,0,1)(ucid,action,auto_connect, apn, user_name,passwd,auth_type,ip_type)");
            nwy_ext_echo("\r\n disable netshareact quick test ok!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        /*End:Add by zhangtengwei for OPEN_SDK_API open ECM  in 2021.1.15*/
        /*Begin: Add by huangweigang for support flight mode in 2021.3.30*/
        case 20:
        {
            nwy_ext_echo("\r\n========================================\r\n");
            nwy_ext_input_gets("\r\nPlease set RADIO state(0-off,1-on):");
            if (strlen((char *)nwy_ext_sio_recv_buff) > MAX_INPUT_OPTION)
                break;
            memset(sub_opt, 0, sizeof(sub_opt));
            memcpy(sub_opt, nwy_ext_sio_recv_buff, strlen((char *)nwy_ext_sio_recv_buff));
            if(nwy_nw_set_radio_st(atoi(sub_opt)) == 0)
              nwy_ext_echo("\r\nSet radio state OK!\r\n");
            else
              nwy_ext_echo("\r\nSet radio state error!\r\n");
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        case 21:
        {
            nwy_ext_echo("\r\n========================================\r\n");
            int nFM = 0;
            if (nwy_nw_get_radio_st(&nFM) != 0)
            {
              nwy_ext_echo("\r\nget radio state error!\r\n");
              break;
            }

            nwy_ext_echo("\r\nradio state:%d\r\n", nFM);
            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        case 22:
        {
            nwy_ext_echo("\r\n========================================\r\n");
            if (NWY_SUCCESS != nwy_nw_get_neighborLocatorInfo( nwy_cellinfo_scan_cb ))
            {
              nwy_ext_echo("\r\ncell scan failed!\r\n");
              break;
            }

            nwy_ext_echo("\r\n========================================\r\n");
            break;
        }
        /*End: Add by huangweigang for support flight mode in 2021.3.30*/
        default:
            nwy_ext_echo("\r\nError option!!");
        break;
        }
    }
}
#else
void nwy_network_test_main()
{
    nwy_ext_echo("\r\nNot Support Network Test!!!\r\n");
}
#endif
/*End:  Add by novalis.wong for support network api test in 2020.02.17*/

/* added by wangchen for N58 sms api to test 20200215 begin */
int test_send_sms()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_sms_info_type_t sms_data = {0};
    nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input dest phone number: ");
    memcpy(sms_data.phone_num,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));//

    nwy_ext_input_gets("\r\nPlease input msg len: ");
    if(strlen(sptr) > 3)
    {
        nwy_ext_echo("\r\n invalid msg len");
        return -1;
    }
    sms_data.msg_context_len = atoi(sptr);

    nwy_ext_input_gets("\r\nPlease input msg data: ");
    memcpy(sms_data.msg_contxet,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

    nwy_ext_input_gets("\r\nPlease input msg format(0:GSM7 2:UNICODE): ");
    sms_data.msg_format = atoi(sptr);

    ret = nwy_sms_send_message(&sms_data);
    if(NWY_SMS_SUCCESS != ret)
    {
        OSI_LOGI(0, "nwy send sms fail");
        return -1;
    }

    return 0;
}

int test_del_sms()
{
    uint16_t  nindex;
    nwy_sms_storage_type_e nStorage;
	char* sptr = nwy_ext_sio_recv_buff;
	nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input sms index: ");
	nindex = atoi(sptr);

	nwy_ext_input_gets("\r\nPlease input sms storage: ");
	nStorage = atoi(sptr);

	ret = nwy_sms_delete_message(nindex, nStorage);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy del sms fail");
		return -1;
	}

	return 0;
}

int test_del_sms_by_type()
{
    nwy_sms_storage_type_e nStorage;
    nwy_sms_msg_dflag_e dflag = NWY_SMS_MSG_DFLAG_NONE;
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input sms delete type: ");
    dflag = atoi(sptr);

    nwy_ext_input_gets("\r\nPlease input sms storage: ");
    nStorage = atoi(sptr);

    ret = nwy_sms_delete_message_by_type(dflag, nStorage);
    if(NWY_SMS_SUCCESS != ret)
    {
        OSI_LOGI(0, "nwy del sms fail");
        return -1;
    }

    return 0;
}

int test_get_sca_sms()
{
	char* sptr = nwy_ext_sio_recv_buff;
	nwy_sms_result_type sca = {0};
	nwy_result_t ret = NWY_SMS_SUCCESS;

	ret = nwy_sms_get_sca(&sca);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy del sms fail");
		return -1;
	}

    nwy_ext_echo("\r\n sca:%s \r\n", sca.sca);
	return 0;
}

int test_set_sca_sms()
{
    char* sptr = nwy_ext_sio_recv_buff;
	char sca[21] = {0};
	unsigned tosca;
	nwy_result_t ret = NWY_SMS_SUCCESS;

	nwy_ext_input_gets("\r\nPlease input sca number: ");
	memcpy(sca,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

	nwy_ext_input_gets("\r\nPlease input sca type: ");
	tosca = atoi(sptr);

	ret = nwy_sms_set_sca(sca, tosca);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy set sca fail");
		return -1;
	}

	return 0;
}

int test_set_storage_sms()
{
    char* sptr = nwy_ext_sio_recv_buff;
	nwy_sms_storage_type_e sms_storage;
	nwy_result_t ret = NWY_SMS_SUCCESS;

	nwy_ext_input_gets("\r\nPlease input sms storage: ");
	sms_storage = atoi(sptr);

	ret = nwy_sms_set_storage(sms_storage);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy set sms storage fail");
		return -1;
	}

	return 0;
}

void test_get_storage_sms()
{
    nwy_sms_storage_type_e sms_storage;

	sms_storage = nwy_sms_get_storage();

	nwy_ext_echo("\r\n sms_storage:%d", sms_storage);
}

int test_set_report_mode_sms()
{
    char* sptr = nwy_ext_sio_recv_buff;
    uint8_t mode = 0;
	uint8_t mt = 0;
	uint8_t bm = 0;
	uint8_t ds = 0;
	uint8_t bfr = 0;
	nwy_result_t ret = NWY_SMS_SUCCESS;

	nwy_ext_input_gets("\r\nPlease input sms mode: ");
	mode = atoi(sptr);

	nwy_ext_input_gets("\r\nPlease input sms mt: ");
	mt = atoi(sptr);

	ret = nwy_set_report_option(mode, mt, bm, ds, bfr);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy set sms report mode fail");
		return -1;
	}

	return 0;
}

int test_read_sms_by_index()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_sms_recv_info_type_t sms_data = {0};
	unsigned index = 0;
	nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input sms index: ");
	index = atoi(sptr);

	ret = nwy_sms_read_message(index, &sms_data);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy read sms fail");
		return -1;
	}

	OSI_LOGI(0,  "=SMS= finish to read sms\n");

	nwy_sleep(2000);//wait for sms_data

    nwy_ext_echo("\r\n read one sms oa:%s msg_context:%s",sms_data.oa,sms_data.pData);

	OSI_LOGI(0,  "=SMS= end to test read sms\n");

	return 0;
}

int test_read_sms_by_list()
{
    nwy_sms_msg_list_t sms_list = {0};
    nwy_sms_recv_info_type_t sms_data = {0};
    memset(&sms_list, 0, sizeof(sms_list));

    if (nwy_sms_read_message_list(&sms_list) == NWY_SMS_ERROR)
    {
        nwy_ext_echo("\r\n nwy_sms_read_message_list fail!");
        return -1;
    }
    else
    {
        nwy_sleep(6000);
        OSI_LOGI(0,  "=SMS= nwy_sms_read_message_list sms_list.count=%d", sms_list.count);
        nwy_ext_echo("\r\n sms number=%d", sms_list.count);
        char unread_index[500] = {0};
        char read_index[500] = {0};
        char unsent_index[500] = {0};
        char sent_index[500] = {0};
        for (int temp = 0; (temp < sms_list.count); temp++)
        {
            OSI_LOGI(0, "\r\n =SMS= index=%d, tpye =%d", sms_list.sms_info[temp].index, sms_list.sms_info[temp].type);
            if (sms_list.sms_info[temp].type == NWY_SMS_STORED_CMGL_STATUS_UNREAD)
            {
                sprintf(unread_index + strlen(unread_index), "%d,", sms_list.sms_info[temp].index);
            }
            if (sms_list.sms_info[temp].type == NWY_SMS_STORED_CMGL_STATUS_READ)
            {
                sprintf(read_index + strlen(read_index), "%d,", sms_list.sms_info[temp].index);

            }
            if (sms_list.sms_info[temp].type == NWY_SMS_STORED_CMGL_STATUS_UNSENT)
            {
                sprintf(unsent_index + strlen(unsent_index), "%d,", sms_list.sms_info[temp].index);
            }
            if (sms_list.sms_info[temp].type == NWY_SMS_STORED_CMGL_STATUS_SENT)
            {
                sprintf(sent_index + strlen(sent_index), "%d,", sms_list.sms_info[temp].index);
            }
        }
        unread_index[strlen(unread_index) - 1] = '\0';
        read_index[strlen(read_index) - 1] = '\0';
        unsent_index[strlen(unsent_index) - 1] = '\0';
        sent_index[strlen(sent_index) - 1] = '\0';
        nwy_ext_echo("\r\n unread message index: %s", unread_index);
        nwy_ext_echo("\r\n read message index: %s", read_index);
        nwy_ext_echo("\r\n unsent message index: %s", unsent_index);
        nwy_ext_echo("\r\n sent message index: %s", sent_index);
    }
    return 0;
}


void nwy_HexStrToByte(const char *source, int sourceLen, unsigned char *dest)
{
    short i;
    unsigned char highByte = 0, lowByte = 0;

    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper((unsigned char)source[i]);
        lowByte = toupper((unsigned char)source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return;
}


int test_send_pdu_sms()
{
    int ret = 0;
    char* sptr = nwy_ext_sio_recv_buff;
    char pbuf[1024] = {0};
    nwy_ext_input_gets("\r\nPlease input sms pdu data: ");
    nwy_HexStrToByte(nwy_ext_sio_recv_buff, strlen(nwy_ext_sio_recv_buff), pbuf);
    ret = nwy_sms_send_pdu_message(pbuf, strlen(nwy_ext_sio_recv_buff)/2);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy send pdu sms fail");
		return -1;
	}

	OSI_LOGI(0,  "=SMS= finish to send pdu sms\n");
	return 0;
}

void nwy_sms_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
	nwy_sms_info_type_t sms_data = {0};
	int ret = 0;

	nwy_init_sms_option();

    nwy_ext_echo("\r\nPlease select an option to test sms below. \r\n");
	nwy_ext_echo("1. send sms test \r\n");
    nwy_ext_echo("2. delete sms test \r\n");
    nwy_ext_echo("3. get sca test \r\n");
    nwy_ext_echo("4. set sca test \r\n");
    nwy_ext_echo("5. set storage test \r\n");
    nwy_ext_echo("6. get storage test \r\n");
	nwy_ext_echo("7. set report mode test \r\n");
	nwy_ext_echo("8. read sms by index test \r\n");
	nwy_ext_echo("9. send pdu sms \r\n");
    nwy_ext_echo("10. del sms by type \r\n");
    nwy_ext_echo("11. read sms list \r\n");
	nwy_ext_echo("0. do nothing waiting \r\n");

	nwy_ext_input_gets("\r\nPlease input option: ");

	switch(atoi(sptr))
	{
	    case 0:
	    {
	        break;
	    }
	    case 1:
	    {
	        ret = test_send_sms();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n send sms fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n send sms success \r\n");
			}
			break;
	    }
		case 2:
		{
	        ret = test_del_sms();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n del sms fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n del sms success \r\n");
			}
			break;
	    }
		case 3:
		{
	        ret = test_get_sca_sms();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n get sms sca fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n get sms sca success \r\n");
			}
			break;
	    }
		case 4:
		{
	        ret = test_set_sca_sms();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n set sms sca fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n set sms sca success \r\n");
			}
			break;
	    }
		case 5:
		{
	        ret = test_set_storage_sms();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n set sms storage fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n set sms storage success \r\n");
			}
			break;
	    }
		case 6:
		{
		    test_get_storage_sms();
		    break;
		}
		case 7:
		{
		    ret = test_set_report_mode_sms();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n set sms report mode fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n set sms report mode success \r\n");
			}
			break;
		}
		case 8:
	    {
	        ret = test_read_sms_by_index();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n read sms fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n read sms success \r\n");
			}
			break;
	    }
        case 9:
	    {
	        ret = test_send_pdu_sms();
			if(0 != ret)
			{
			    nwy_ext_echo("\r\n send sms fail \r\n");
			}
			else
			{
			    nwy_ext_echo("\r\n send sms success \r\n");
			}
			break;
	    }
        case 10:
        {
            ret = test_del_sms_by_type();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n del sms by type fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n del sms by type success \r\n");
            }
            break;
        }
        case 11:
        {
            ret = test_read_sms_by_list();
            if(0 != ret)
            {
                nwy_ext_echo("\r\ntest_read_sms_by_list fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n test_read_sms_by_list success \r\n");
            }
            break;
        }

		default:
			nwy_ext_echo("\r\n invalid input \r\n");
			break;

	}

}
/* added by wangchen for N58 sms api to test 20200215 end */

#ifdef FEATURE_NWY_STK_COMMAND
int test_set_switch_stk()
{
    nwy_sat_onoff_type_e switch_type;
    char* sptr = nwy_ext_sio_recv_buff;
    int ret = 0;

    nwy_ext_input_gets("\r\nPlease input stk switch: ");
    switch_type = atoi(sptr);

    ret = nwy_sat_set_on_off(switch_type);
    if(0 != ret)
    {
        OSI_LOGI(0, "nwy switch stk fail");
        return -1;
    }

    return 0;
}

int test_get_switch_stk()
{
    nwy_sat_onoff_type_e  switch_type = NWY_SAT_TYPE_MAX;
    char* sptr = nwy_ext_sio_recv_buff;
    int ret = 0;

    switch_type = nwy_sat_get_on_off();
    if(NWY_SAT_TYPE_MAX == switch_type)
    {
        OSI_LOGI(0, "nwy get stk switch fail");
        return -1;
    }

    nwy_ext_echo("\r\n stk switch on/off:%d", switch_type);

    return 0;
}

int test_terminal_response_stk()
{
    char* sptr = nwy_ext_sio_recv_buff;
    char data[500] = {0};
    int ret = 0;

    nwy_ext_input_gets("\r\nPlease input terminal response data: ");
    memcpy(data,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

    ret = nwy_sat_terminal_response(data);
    if(0 != ret)
    {
        OSI_LOGI(0, "nwy send terminal response fail");
        return -1;
    }

    return 0;
}

int test_envelope_command_stk()
{
    char* sptr = nwy_ext_sio_recv_buff;
    char data[500] = {0};
    int ret = 0;

    nwy_ext_input_gets("\r\nPlease input envelope command data: ");
    memcpy(data,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

    ret = nwy_sat_envelope_command(data);
    if(0 != ret)
    {
        OSI_LOGI(0, "nwy send envelope command fail");
        return -1;
    }

    return 0;
}

int test_set_profile_stk()
{
    char* sptr = nwy_ext_sio_recv_buff;
    char data[100] = {0};
    int ret = 0;

    nwy_ext_input_gets("\r\nPlease input set profile data: ");
    memcpy(data,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

    ret = nwy_sat_set_profile(data);
    if(0 != ret)
    {
        OSI_LOGI(0, "nwy set profile fail");
        return -1;
    }

    return 0;
}

int test_get_profile_stk()
{
    char* sptr = nwy_ext_sio_recv_buff;
    char data[100] = {0};
    int ret = 0;

    ret = nwy_sat_get_profile(data);
    if(0 != ret)
    {
        OSI_LOGI(0, "nwy get profile fail");
        return -1;
    }

    nwy_ext_echo("\r\n profile:%s \r\n", data);
    return 0;
}

void nwy_stk_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_sms_info_type_t sms_data = {0};
    int ret = 0;


    nwy_ext_echo("\r\nPlease select an option to test stk below. \r\n");
    nwy_ext_echo("1. set stk on/off test \r\n");
    nwy_ext_echo("2. get stk on/off test \r\n");
    nwy_ext_echo("3. send stk terminal response test \r\n");
    nwy_ext_echo("4. send stk envelope command test \r\n");
    nwy_ext_echo("5. set profile test \r\n");
    nwy_ext_echo("6. get profile test \r\n");
    nwy_ext_input_gets("\r\nPlease input option: ");

    switch(atoi(sptr))
    {
        case 0:
        {
            break;
        }
        case 1:
        {
            ret = test_set_switch_stk();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n set stk switch fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n set stk switch success \r\n");
            }
            break;
        }
        case 2:
        {
            ret = test_get_switch_stk();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get stk switch fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get stk switch success \r\n");
            }
            break;
        }
        case 3:
        {
            ret = test_terminal_response_stk();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n stk send terminal response fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n stk send terminal response success \r\n");
            }
            break;
        }
        case 4:
        {
            ret = test_envelope_command_stk();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n stk send envelope command fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n stk send envelope command success \r\n");
            }
            break;
        }
        case 5:
        {
            ret = test_set_profile_stk();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n stk set profile fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n stk set profile success \r\n");
            }
            break;
        }
        case 6:
        {
            ret = test_get_profile_stk();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n stk get profile fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n stk get profile success \r\n");
            }
            break;
        }

        default:
            nwy_ext_echo("\r\n invalid input \r\n");
            break;
    }
}
#endif

void nwy_camera_test()
{
    char* str = nwy_ext_sio_recv_buff;
    int ret = 0;
	int save_mode = 0;
	int image_format = 0;
	char image_path[128] = {0};
	crop_image_info_t crop_img_info;
	memset(&crop_img_info, 0, sizeof(crop_img_info));
    nwy_ext_echo("\r\nPlease select an option to test camera below.\r\n");
    nwy_ext_echo("1. open camera test \r\n");
    nwy_ext_echo("2. close camera test \r\n");
    nwy_ext_echo("3. nwy_camera_capture_image jpg test\r\n");
    nwy_ext_echo("4. nwy_camera_capture_image yuv test\r\n");
    nwy_ext_echo("5. nwy_camera_crop_capture_image jpg test\r\n");
    nwy_ext_echo("6. nwy_camera_crop_capture_image yuv test\r\n");
    nwy_ext_input_gets("\r\nPlease input option: ");
    switch(atoi(str))
    {
        case 0:
        {
            break;
        }
        case 1: //camera open
        {
            ret = nwy_camera_open();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n test open camera failed \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n test open camera success \r\n");
            }
            break;
        }
        case 2: //camera close
        {
            nwy_camera_close();
            nwy_ext_echo("\r\n test close camera success \r\n");
            break;
        }
        case 3: //capture jpg save in fs
        {
			image_format = JPG_FORMAT;
			save_mode = FS_MODE;
			strcpy(image_path, "/nwy/cam.jpg");
            ret = nwy_camera_capture_image(image_format, save_mode, image_path);
            if(0 != ret)
            {
                nwy_ext_echo("\r\n test camera capture jpg failed\r\n");
            }
            else
            {
                nwy_ext_echo("\r\n test camera capture jpg success\r\n");
            }
            break;
        }
        case 4: //capture yuv save in fs
        {
			image_format = YUV_FORMAT;
			save_mode = FS_MODE;
			strcpy(image_path, "/nwy/cam.yuv");
            ret = nwy_camera_capture_image(image_format, save_mode, image_path);
            if(0 != ret)
            {
                nwy_ext_echo("\r\n test camera capture yuv failed\r\n");
            }
            else
            {
                nwy_ext_echo("\r\n test camera capture yuv success\r\n");
            }
            break;
        }
        case 5: //crop capture jpg save in fs
        {
			crop_img_info.start_x = 0;
			crop_img_info.start_y = 0;
			crop_img_info.des_width = 320;
			crop_img_info.des_height = 240;
			image_format = JPG_FORMAT;
			save_mode = FS_MODE;
			strcpy(image_path, "/nwy/crop_cam.jpg");
			ret = nwy_camera_crop_capture_image(image_format, save_mode, image_path, &crop_img_info);
            if(0 != ret)
            {
                nwy_ext_echo("\r\n test camera capture jpg failed\r\n");
            }
            else
            {
                nwy_ext_echo("\r\n test camera capture jpg success\r\n");
            }
            break;
        }
        case 6: //crop capture yuv save in fs
        {
			crop_img_info.start_x = 0;
			crop_img_info.start_y = 0;
			crop_img_info.des_width = 320;
			crop_img_info.des_height = 240;
			image_format = YUV_FORMAT;
			save_mode = FS_MODE;
			strcpy(image_path, "/nwy/crop_cam.yuv");
			ret = nwy_camera_crop_capture_image(image_format, save_mode, image_path, &crop_img_info);
            if(0 != ret)
            {
                nwy_ext_echo("\r\n test camera crop capture yuv failed\r\n");
            }
            else
            {
                nwy_ext_echo("\r\n test camera crop capture yuv success\r\n");
            }
            break;
        }
        default:
            nwy_ext_echo("\r\n invalid input \r\n");
            break;
    }
}
int test_get_switch_poc_dsds()
{
    nwy_poc_dsds_onoff_type_e  on_off = NWY_POC_DSDS_TYPE_MAX;
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    on_off = nwy_poc_dsds_get_on_off();
    if(NWY_POC_DSDS_TYPE_MAX == on_off)
    {
        OSI_LOGI(0, "nwy switch poc dsds fail");
        return -1;
    }

    nwy_ext_echo("\r\n poc dsds switch on/off:%d", on_off);

    return 0;
}

int test_set_switch_poc_dsds()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_poc_dsds_onoff_type_e switch_type;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input switch type: ");
    switch_type = atoi(sptr);

    ret = nwy_poc_dsds_set_on_off(switch_type);
    if(NWY_SMS_SUCCESS != ret)
    {
        OSI_LOGI(0, "nwy set poc dsds switch fail");
        return -1;
    }

    return 0;
}

int test_get_default_card()
{
    nwy_poc_dsds_defaultcard_type_e  default_card = NWY_POC_DSDS_DEFAULT_MAX;
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    default_card = nwy_poc_dsds_get_default_card();
    if(NWY_POC_DSDS_DEFAULT_MAX == default_card)
    {
        OSI_LOGI(0, "nwy get deault card fail");
        return -1;
    }

    nwy_ext_echo("\r\n get default card:%d", default_card);

    return 0;
}

int test_set_default_card()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_poc_dsds_defaultcard_type_e default_card;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input default card: ");
    default_card = atoi(sptr);

    ret = nwy_poc_dsds_set_default_card(default_card);
    if(NWY_SMS_SUCCESS != ret)
    {
        OSI_LOGI(0, "nwy set default card fail");
        return -1;
    }

    return 0;
}

int test_get_auto_switch_card()
{
    nwy_poc_dsds_auto_switch_type_e  auto_switch = NWY_POC_DSDS_AUTO_SWITCH_MAX;
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    auto_switch = nwy_poc_dsds_get_auto_switch_card();
    if(NWY_POC_DSDS_AUTO_SWITCH_MAX == auto_switch)
    {
        OSI_LOGI(0, "nwy get auto switch card fail");
        return -1;
    }

    nwy_ext_echo("\r\n get auto switch card:%d", auto_switch);

    return 0;
}

int test_set_auto_switch_card()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_poc_dsds_auto_switch_type_e auto_switch;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input auto switch: ");
    auto_switch = atoi(sptr);

    ret = nwy_poc_dsds_set_auto_switch_card(auto_switch);
    if(NWY_SMS_SUCCESS != ret)
    {
        OSI_LOGI(0, "nwy set auto switch card fail");
        return -1;
    }

    return 0;
}

int test_get_master_card()
{
    nwy_poc_dsds_mastercard_type_e  master_card = NWY_POC_DSDS_MASTER_NONE;
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    master_card = nwy_poc_dsds_get_mastercard_card();
    if(NWY_POC_DSDS_MASTER_NONE == master_card)
    {
        OSI_LOGI(0, "nwy get master card fail");
        return -1;
    }

    nwy_ext_echo("\r\n get master card:%d", master_card);

    return 0;
}

int test_set_master_card()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_poc_dsds_mastercard_type_e master_card;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    nwy_ext_input_gets("\r\nPlease input master card: ");
    master_card = atoi(sptr);

    ret = nwy_poc_dsds_set_mastercard_card(master_card);
    if(NWY_SMS_SUCCESS != ret)
    {
        OSI_LOGI(0, "nwy set master card fail");
        return -1;
    }

    return 0;
}


void nwy_poc_dsds_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    int ret = 0;

    nwy_ext_echo("\r\nPlease select an option to test poc dsds below. \r\n");
    nwy_ext_echo("1. get poc dsds func on/off \r\n");
    nwy_ext_echo("2. set poc dsds func on/off \r\n");
    nwy_ext_echo("3. get poc dsds default card \r\n");
    nwy_ext_echo("4. set poc dsds default card \r\n");
    nwy_ext_echo("5. get poc dsds auto switch card \r\n");
    nwy_ext_echo("6. set poc dsds auto switch card \r\n");
    nwy_ext_echo("7. get poc dsds master card \r\n");
    nwy_ext_echo("8. set poc dsds master card \r\n");

    nwy_ext_input_gets("\r\nPlease input option: ");

    switch(atoi(sptr))
    {
        case 0:
        {
            break;
        }
        case 1:
        {
            ret = test_get_switch_poc_dsds();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get switch poc dsds fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get switch poc dsds success \r\n");
            }
            break;
        }
        case 2:
        {
            ret = test_set_switch_poc_dsds();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n set switch poc dsds fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n set switch poc dsds success \r\n");
            }
            break;
        }
        case 3:
        {
            ret = test_get_default_card();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get default card fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get default card success \r\n");
            }
            break;
        }
        case 4:
        {
            ret = test_set_default_card();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n set default card fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n set default card success \r\n");
            }
            break;
        }
        case 5:
        {
            ret = test_get_auto_switch_card();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get auto switch card fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get auto switch card success \r\n");
            }
            break;
        }
        case 6:
        {
            ret = test_set_auto_switch_card();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n set auto switch card fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n set auto switch card success \r\n");
            }
            break;
        }
        case 7:
        {
            ret = test_get_master_card();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get master fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get master success \r\n");
            }
            break;
        }
        case 8:
       {
            ret = test_set_master_card();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n set master fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n set master success \r\n");
            }
            break;
        }

        default:
            nwy_ext_echo("\r\n invalid input \r\n");
            break;
    }

}

/* Begin: Add by yjj for support data api test demo in 2020.02.17*/
static int ppp_state[10] = {0};
void nwy_data_cb_fun(
    int hndl,
    nwy_data_call_state_t ind_state)
{
  OSI_LOGI(0, "=DATA= hndl=%d,ind_state=%d", hndl,ind_state);
  if (hndl > 0 && hndl <= 8)
  {
    ppp_state[hndl-1] = ind_state;
    nwy_ext_echo("\r\nData call status update, handle_id:%d,state:%d\r\n",hndl,ind_state);
  }
}

int nwy_ext_check_data_connect()
{
    int i = 0;
    for (i = 0; i< NWY_DATA_CALL_MAX_NUM; i++) {
        if (ppp_state[i] == NWY_DATA_CALL_CONNECTED) {
            return 1;
        }
    }
    return 0;
}

void nwy_ext_data_menu()
{
  nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
  nwy_ext_echo("1. Create resource handle\r\n");
  nwy_ext_echo("2. Get profile info\r\n");
  nwy_ext_echo("3. Set profile info\r\n");
  nwy_ext_echo("4. Start data call \r\n");
  nwy_ext_echo("5. Get data info \r\n");
  nwy_ext_echo("6. Stop data call\r\n");
  nwy_ext_echo("7. Release resource handle\r\n");
  nwy_ext_echo("8. Get data traffic statistics info\r\n");
  nwy_ext_echo("9. Get data auto reconnect status\r\n");
  nwy_ext_echo("10. Exit data function\r\n");
}

void nwy_data_test()
{
  int ret = -1;
  char* sptr = nwy_ext_sio_recv_buff;
  nwy_data_profile_info_t profile_info;
  nwy_sleep(500);
  while (1)
  {
      nwy_ext_data_menu();
      nwy_ext_input_gets("\r\nPlease input option: ");
      switch(atoi(sptr))
      {
          case 1:
          {
              int hndl = nwy_data_get_srv_handle(nwy_data_cb_fun);
              OSI_LOGI(0, "=DATA=  hndl= %d", hndl);
/* Begin: Add by yjj for TD83000 in 2021.05.12*/
              if ((hndl > 0) &&(hndl <= NWY_DATA_CALL_MAX_NUM))
                nwy_ext_echo("\r\nCreate a Resource Handle id: %d success\r\n",hndl);
              else
                nwy_ext_echo("\r\nCreate a Resource Handle failed, result<%d>\r\n",hndl);
/* End: Add by yjj for TD83000 in 2021.05.12*/
              break;
          }
          case 2:
          {
            nwy_ext_input_gets("\r\nPlease input profile index: (1-7)");
            int profile_id = atoi(sptr);
            if ((profile_id <= 0) || (profile_id > 7)){
              nwy_ext_echo("\r\nInvaild profile id: %d\r\n",profile_id);
              break;
            }
            memset(&profile_info,0,sizeof(nwy_data_profile_info_t));
            ret = nwy_data_get_profile(profile_id,NWY_DATA_PROFILE_3GPP,&profile_info);
            OSI_LOGI(0, "=DATA=  nwy_data_get_profile ret= %d", ret);
            OSI_LOGI(0, "=DATA=  profile= %d|%d", profile_info.pdp_type,profile_info.auth_proto);
            OSI_PRINTFI("=DATA=   profile= %s|%s|%s", profile_info.apn,profile_info.user_name,profile_info.pwd);
            if (ret != NWY_RES_OK){
              nwy_ext_echo("\r\nRead profile %d info fail, result%d\r\n",profile_id,ret);
            }
            else {
                nwy_ext_echo("\r\nProfile %d info: <pdp_type>,<auth_proto>,<apn>,<user_name>,<password>\r\n%d,%d,%s,%s,%s\r\n",profile_id,profile_info.pdp_type,
                profile_info.auth_proto,profile_info.apn,profile_info.user_name,profile_info.pwd);
            }
            break;
          }
          case 3:
          {
            memset(&profile_info,0,sizeof(nwy_data_profile_info_t));
            nwy_ext_input_gets("\r\nPlease input profile info: profile_id <1-7>");
            int profile_id = atoi(sptr);
            if ((profile_id <= 0) || (profile_id > 7)){
              nwy_ext_echo("\r\nInvaild profile id: %d\r\n",profile_id);
              break;
            }

            nwy_ext_input_gets("\r\nPlease input profile info: auth_proto <0-2>\r\n0:NONE, 1:PAP, 2:CHAP\r\n");
            int auth_proto = atoi(sptr);
            if ((auth_proto < 0) || (auth_proto > 2)){
              nwy_ext_echo("\r\nInvaild auth_proto value: %d\r\n",auth_proto);
              break;
            }
            profile_info.auth_proto = auth_proto;

            nwy_ext_input_gets("\r\nPlease input profile info: pdp_type <1-3,6>\r\n1:IPV4, 2:IPV6, 3:IPV4V6, 6:PPP\r\n");
            int pdp_type = atoi(sptr);
            if ((pdp_type != 1) && (pdp_type !=2) && (pdp_type !=3) && (pdp_type !=6)){
              nwy_ext_echo("\r\nInvaild pdp_type value: %d\r\n",pdp_type );
              break;
            }
            profile_info.pdp_type = pdp_type;

            nwy_ext_input_gets("\r\nPlease input profile info: apn (length 0-%d)\r\n",NWY_APN_MAX_LEN);
            if (strlen(nwy_ext_sio_recv_buff) > NWY_APN_MAX_LEN) {
              nwy_ext_echo("\r\nInvaild apn len\r\n");
              break;
            }
            if (strlen(nwy_ext_sio_recv_buff) > 0)
              memcpy(profile_info.apn,nwy_ext_sio_recv_buff,sizeof(profile_info.apn));

            nwy_ext_input_gets("\r\nPlease input profile info: user name (length 0-%d)\r\n",NWY_APN_USER_MAX_LEN);
            if (strlen(nwy_ext_sio_recv_buff) > NWY_APN_USER_MAX_LEN) {
              nwy_ext_echo("\r\nInvaild user name len\r\n");
              break;
            }
            if (strlen(nwy_ext_sio_recv_buff) > 0)
              memcpy(profile_info.user_name,nwy_ext_sio_recv_buff,sizeof(profile_info.user_name));

            nwy_ext_input_gets("\r\nPlease input profile info: password(length 0-%d)\r\n",NWY_APN_PWD_MAX_LEN);
            if (strlen(nwy_ext_sio_recv_buff) > NWY_APN_PWD_MAX_LEN) {
              nwy_ext_echo("\r\nInvaild password len\r\n");
              break;
            }
            if (strlen(nwy_ext_sio_recv_buff) > 0)
              memcpy(profile_info.pwd,nwy_ext_sio_recv_buff,sizeof(profile_info.pwd));

            ret = nwy_data_set_profile(profile_id,NWY_DATA_PROFILE_3GPP,&profile_info);
            OSI_LOGI(0, "=DATA=  nwy_data_set_profile ret= %d", ret);
            if (ret != NWY_RES_OK)
              nwy_ext_echo("\r\nSet profile %d info fail, result<%d>\r\n",profile_id,ret);
            else
              nwy_ext_echo("\r\nSet profile %d info success\r\n",profile_id);
            break;
          }
          case 4:
          {
            nwy_data_start_call_v02_t param_t;
            memset(&param_t,0,sizeof(nwy_data_start_call_v02_t));
            nwy_ext_input_gets("\r\nPlease select resource handle id <1-%d>",NWY_DATA_CALL_MAX_NUM);
            int hndl = atoi(sptr);
            if ((hndl <= 0) || (hndl > NWY_DATA_CALL_MAX_NUM)){
              nwy_ext_echo("\r\nInvaild resource handle id: %d\r\n",hndl);
              break;
            }

            nwy_ext_input_gets("\r\nPlease select profile: profile_id <1-7>");
            int profile_id = atoi(sptr);
            if ((profile_id <= 0) || (profile_id > 7)){
              nwy_ext_echo("\r\nInvaild profile id: %d\r\n",profile_id);
              break;
            }
            param_t.profile_idx = profile_id;

            /* Begin: Add by YJJ for support auto re_connect in 2020.05.20*/
            nwy_ext_input_gets("\r\nPlease set auto_connect: 0 Disable, 1 Enable");
            int is_auto_recon = atoi(sptr);
            if ((is_auto_recon != 0) && (is_auto_recon != 1)){
              nwy_ext_echo("\r\nInvaild auto_connect: %d\r\n",is_auto_recon);
              break;
            }
            param_t.is_auto_recon = is_auto_recon;

            if(is_auto_recon ==1)
            {
              nwy_ext_input_gets("\r\nPlease set auto_connect maximum times: 0 Always Re_connect, [1-65535] maximum_times");
              int auto_re_max = atoi(sptr);
              if ((auto_re_max < 0) || (auto_re_max > 65535)){
                nwy_ext_echo("\r\nInvaild auto_connect maximum times : %d\r\n",auto_re_max);
                break;
              }
              param_t.auto_re_max = auto_re_max;

              nwy_ext_input_gets("\r\nPlease set auto_connect interval (ms): [100,86400000]");
              int auto_interval_ms = atoi(sptr);
              if ((auto_interval_ms < 100) || (auto_interval_ms > 86400000)){
                nwy_ext_echo("\r\nInvaild auto_connect interval: %d\r\n",auto_interval_ms);
                break;
              }
              param_t.auto_interval_ms = auto_interval_ms;
            }

            /* End: Add by YJJ for auto re_connect in 2020.05.20*/

            ret = nwy_data_start_call(hndl,&param_t);
            OSI_LOGI(0, "=DATA=  nwy_data_start_call ret= %d", ret);
            if (ret != NWY_RES_OK)
              nwy_ext_echo("\r\nStart data call fail, result<%d>\r\n",ret);
            else
              nwy_ext_echo("\r\nStart data call ...\r\n");
            break;
          }
          case 5:
          {
            int len = 0;
            nwy_data_addr_t_info addr_info;
            nwy_ext_input_gets("\r\nPlease select resource handle id <1-%d>",NWY_DATA_CALL_MAX_NUM);
            int hndl = atoi(sptr);
            if ((hndl <= 0) || (hndl > NWY_DATA_CALL_MAX_NUM)){
              nwy_ext_echo("\r\nInvaild resource handle id: %d\r\n",hndl);
              break;
            }

            memset(&addr_info,0,sizeof(nwy_data_addr_t_info));
            OSI_LOGI(0, "=DATA=  addr_info size= %d",sizeof(nwy_data_addr_t_info));
            ret = nwy_data_get_ip_addr(hndl, &addr_info, &len);
            OSI_LOGI(0, "=DATA=  nwy_data_get_ip_addr = %d|len%d", ret,len);
            if (ret != NWY_RES_OK)
            {
              nwy_ext_echo("\r\nGet data info fail, result<%d>\r\n",ret);
              break;
            }
            nwy_ext_echo("\r\nGet data info success\r\nIface address: %s,%s\r\n",
                nwy_ip4addr_ntoa(&addr_info.iface_addr_s.ip_addr.addr),
                nwy_ip6addr_ntoa(&addr_info.iface_addr_s.ip6_addr.addr));
            nwy_ext_echo("Dnsp address: %s,%s\r\n",
                nwy_ip4addr_ntoa(&addr_info.dnsp_addr_s.ip_addr.addr),
                nwy_ip6addr_ntoa(&addr_info.dnsp_addr_s.ip6_addr.addr));
            nwy_ext_echo("Dnss address: %s,%s\r\n",
                nwy_ip4addr_ntoa(&addr_info.dnss_addr_s.ip_addr.addr),
                nwy_ip6addr_ntoa(&addr_info.dnss_addr_s.ip6_addr.addr));
            break;
          }
          case 6:
          {
            nwy_ext_input_gets("\r\nPlease select resource handle id <1-%d>",NWY_DATA_CALL_MAX_NUM);
            int hndl = atoi(sptr);
            if ((hndl <= 0) || (hndl > NWY_DATA_CALL_MAX_NUM)){
              nwy_ext_echo("\r\nInvaild resource handle id: %d\r\n",hndl);
              break;
            }
            ret = nwy_data_stop_call(hndl);
            OSI_LOGI(0, "=DATA=  nwy_data_stop_call ret= %d", ret);
            if (ret != NWY_RES_OK)
              nwy_ext_echo("\r\nStop data call fail, result<%d>\r\n",ret);
            else
              nwy_ext_echo("\r\nStop data call ...\r\n");
            break;
          }
          case 7:
          {
            nwy_ext_input_gets("\r\nPlease select resource handle id <1-%d>",NWY_DATA_CALL_MAX_NUM);
            int hndl = atoi(sptr);
            if ((hndl < 0) || (hndl > NWY_DATA_CALL_MAX_NUM)){
              nwy_ext_echo("\r\nInvaild resource handle id: %d\r\n",hndl);
              break;
            }

            nwy_data_relealse_srv_handle(hndl);
            OSI_LOGI(0, "=DATA=  nwy_data_relealse_srv_handle hndl= %d", hndl);
            nwy_ext_echo("\r\nRelease resource handle id %d\r\n",hndl);
            break;
          }
          case 8:
          {
            char rsp[128] = {0};
            nwy_data_flowcalc_info_t flowcalc_info = {0};
            nwy_data_get_flowcalc_info(&flowcalc_info);
            snprintf (rsp, sizeof(rsp), "tx_bytes:%llu,rx_bytes:%llu\r\ntx_packets:%lu,rx_packets:%lu",
                      flowcalc_info.tx_bytes,flowcalc_info.rx_bytes,
                      flowcalc_info.tx_packets,flowcalc_info.rx_packets);
            nwy_ext_echo("\r\nGet data traffic statistics success\r\n%s\r\n",rsp);
            break;
          }
          case 9:
          {
            nwy_ext_input_gets("\r\nPlease select resource handle id <1-%d>",NWY_DATA_CALL_MAX_NUM);
            int hndl = atoi(sptr);
            if ((hndl < 0) || (hndl > NWY_DATA_CALL_MAX_NUM)){
              nwy_ext_echo("\r\nInvaild resource handle id: %d\r\n",hndl);
              break;
            }
            nwy_data_auto_reconn_status_t auto_reconn_status = {0};
            ret = nwy_data_get_auto_reconnect_status(hndl,&auto_reconn_status);
            OSI_LOGI(0, "=DATA=  nwy_data_get_auto_reconnect_status hndl= %d, ret=%d", hndl,ret);
            if (ret == NWY_RES_OK)
              nwy_ext_echo("\r\nhandle id %d auto reconnect status query success, state=%d,cnt=%d\r\n",hndl,
                    auto_reconn_status.auto_reconn_state,auto_reconn_status.auto_re_cnt);
            else
              nwy_ext_echo("\r\nhandle id %d auto reconnect status query failed, ret=%d\r\n",hndl,ret);
            break;
          }
          case 10:
          {
            return;
          }
          default:
              nwy_ext_echo("\r\nPlease input vaild value");
              break;
      }
  }
}
/* End: Add by yjj for support data api test demo in 2020.02.17*/
void nwy_ext_voice_menu()
{
	nwy_ext_echo("\r\nPlease select an option to test voice listed below. \r\n");
	nwy_ext_echo("1. call start \r\n");
	nwy_ext_echo("2. call end test\r\n");
	nwy_ext_echo("3. answver call \r\n");
	nwy_ext_echo("4. set volte voice \r\n");
	nwy_ext_echo("5. caller id test \r\n");
	nwy_ext_echo("99. Exit voice function \r\n");
}

static int test_phone_num(char *phone_num)
{
    int i = 0;
    int len = strlen(phone_num);

    if (len < 2) {
        nwy_ext_echo("number should large than 2\n");
        return -1;
    }

    for (i = 0; i < len; i++) {
        switch(phone_num[i])
        {
            case '+':
            case '*':
                if (i != 0)
                    return -1;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '0':
                continue;
            default:
                return -1;
        }

    }

    return 0;
}

void nwy_voice_test()
{
   int ret = -1;
   char* sptr = nwy_ext_sio_recv_buff;
   int sim_id = 0x00;
   int setvolte = 0;
   int query_cnt = 0;
   char phone_num[32] = {0};
   nwy_sleep(50);

   while(1)
   {
     nwy_ext_voice_menu();
     nwy_ext_input_gets("\r\nPlease input option: ");
     switch(atoi(sptr))
     {
      case 1:
	  case 4:
      {
	     if(atoi(sptr) == 4)
         {
	   	    nwy_ext_input_gets("\r\n open volte(1-on 0-off):");
		    setvolte = atoi(sptr);
		    nwy_ext_echo("\r\n volte value :%d", setvolte);
            nwy_voice_setvolte(sim_id,setvolte);
	      }

	      nwy_ext_input_gets("\r\n input phone num:");
	      if (strlen(nwy_ext_sio_recv_buff) > 128) {
	       return ;
	      }
          memset(phone_num,0,strlen(phone_num));
	      memcpy(phone_num,(char *)nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

          nwy_ext_echo("\r\n phone num :%s", phone_num);

          if (test_phone_num(phone_num) == -1) {

	  	   nwy_ext_echo("\r\n invaild phone num");
           return ;
          }

	      ret = nwy_voice_call_start(sim_id,phone_num);
	      if (ret == 0) {
	        nwy_ext_echo("\r\n call success");
	      }else
	      {
	       nwy_ext_echo("\r\n call fail");
	      }

          break;
	  }
      case 2:
      {
         nwy_voice_call_end(sim_id);
         break;
      }
      case 3:
      {
         nwy_voice_call_autoanswver();
         break;
      }

     case 5:
	 {
       	nwy_ext_echo("\r\n need open clip at+clip = 1");
	    nwy_ext_virtual_at_test();
		break;
     }
     case 99:
     {
        return;
     }
     default:
        break;
   }
  }
}
void nwy_app_timer_cb(void)
{
    OSI_LOGI(0, "nwy_app_timer_cb");
    nwy_ext_echo("\r\n Timer test ok");
}
void nwy_timer_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    if (g_timer == NULL) {
        nwy_ext_echo("\r\n Timer init fail");
        return ;
    }
    nwy_ext_input_gets("\r\n Start timer(y/n): ");
    if (strstr(sptr, "y") !=NULL) {
        nwy_start_timer_periodic(g_timer, 1000);
    } else if (strstr(sptr, "n") !=NULL){
        nwy_stop_timer(g_timer);
    } else {
        nwy_ext_echo("\r\nInput error");
    }
}
static int semaphore_count = 0;
nwy_osiSemaphore_t *nwy_semaphore = NULL;
void nwy_semaphore_taskA_proc()
{
    int i = 0;
    nwy_semaphore_acquire(nwy_semaphore, 0);
    for (i  = 0;i < 100; i++) {
        nwy_ext_echo("\r\nTaskA semaphore_count = %d", ++semaphore_count);
        nwy_sleep(10);
    }
    nwy_semahpore_release(nwy_semaphore);
    nwy_exit_thread();
}
void nwy_semaphore_taskB_proc()
{
    int i = 0;
    nwy_semaphore_acquire(nwy_semaphore, 0);
    for (i  = 0;i < 50; i++) {
        nwy_ext_echo("\r\nTaskB semaphore_count = %d", --semaphore_count);
        nwy_sleep(10);
    }
    nwy_semahpore_release(nwy_semaphore);
    nwy_exit_thread();
}
void nwy_semaphore_test()
{
    semaphore_count = 0;
    nwy_semaphore = nwy_semaphore_create(1, 1);
    nwy_create_thread("semaphore_testA", nwy_semaphore_taskA_proc, NULL, NWY_OSI_PRIORITY_NORMAL+1, 1024, 4);
    nwy_create_thread("semaphore_testB", nwy_semaphore_taskB_proc, NULL, NWY_OSI_PRIORITY_NORMAL, 1024, 4);
}
#ifdef FEATURE_NEOWAY_PIPECLOUD
void nwy_pipecloud_test()
{
    nwy_pipe_server_para_st server;
    nwy_pipe_switch_st info;
    char* sptr = nwy_ext_sio_recv_buff;

    memset(&server, 0, sizeof(server));
    memset(&info, 0, sizeof(info));

    nwy_ext_input_gets("\r\n input server url(len <= 64)");
    memcpy(server.server_url, sptr, strlen(sptr));

    nwy_ext_input_gets("\r\n input server port(1-65535)");
    server.server_port = atoi(sptr);
    nwy_pipecloude_svr_cfg_set(&server);

    info.type = NWY_PIPE_SERVICE_TYPE_NEOWAY;
    info.status = NWY_PIPE_SWITCH_ENABLE;
    nwy_pipecloude_switch_set(&info);
}
#endif
void nwy_dm_test()
{
    char model_buf[100] = {0};
    char version_buf[100] = {0};
    char open_sdk_version_buf[100] = {0};
    char heapinfo[100] = {0};
    char cpu_info[1024] = {0};
    char thread_list[1024] = {0};
    char sn[NWY_DM_SN_LENGTH_MAX] = {0};
    float value = 0;
    int ret = 0;

    nwy_dm_get_device_heapinfo(heapinfo);
    nwy_dm_get_dev_model(model_buf, 100);
    nwy_dm_get_inner_version(version_buf, 100);
    nwy_dm_get_open_sdk_version(open_sdk_version_buf, 100);
    nwy_dm_get_rftemperature(&value);
    nwy_get_thread_list(thread_list);
    ret = nwy_get_thread_runtime_stats(cpu_info);
    OSI_LOGI(0,"nwy_get_thread_runtime_stats ret = %d", ret);

    nwy_ext_echo("\r\n dev heapinfo:%s",heapinfo);
    nwy_ext_echo("\r\n dev model:%s",model_buf);
    nwy_ext_echo("\r\n dev version :%s",version_buf);
    nwy_ext_echo("\r\n open sdk version :%s",open_sdk_version_buf);
    nwy_ext_echo("\r\n temperature is %.2f", value);
    nwy_ext_echo("\r\nname\t\tcount\t\tusage");
    nwy_ext_echo("\r\n%s", cpu_info);
    nwy_ext_echo("name\t\tstate\tpri\tvalid_stack\tsequence");
    nwy_ext_echo("\r\n%s", thread_list);
    ret = nwy_dm_get_sn(0, sn);
    if(ret <= 0)
        OSI_LOGE(0,"nwy_dm_test nwy_dm_get_sn ret = %d", ret);
    nwy_ext_echo("\r\n dev sn:%s", sn);
}

/*
Important note:
This APGS account is a test account.
If it is commercial, please contact Zhongkewei to obtain an official commercial account.
*/
#define NWY_AGPS_URL   "www.gnss-aide.com"
#define NWY_AGPS_PORT  2621
#define NWY_AGPS_TIMEOUT  10000
#define AGPS_PKT_SIZE 10000
#define NWY_AGPS_USER "freetrial"
#define NWY_AGPS_PASS "123456"

void nwy_ext_location_menu()
{
    nwy_ext_echo("\r\nPlease select an option to test location listed below. \r\n");
    nwy_ext_echo("1. set position mode \r\n");
    nwy_ext_echo("2. set location update rate \r\n");
    nwy_ext_echo("3. set nmea statements output format \r\n");
    nwy_ext_echo("4. set startup mode \r\n");
    nwy_ext_echo("5. get nmea data \r\n");
	nwy_ext_echo("6. module dialing \r\n");
    nwy_ext_echo("7. set location server \r\n");
    nwy_ext_echo("8. open location base position \r\n");
    nwy_ext_echo("9. open assisted location \r\n");
	nwy_ext_echo("10. close location position \r\n");
	nwy_ext_echo("99. Exit location function \r\n");
}

void nwy_cipgsmloc_cb(char *text)
{
    nwy_log_cipgsmloc_result_t *param = (nwy_log_cipgsmloc_result_t *)text;
    if(NULL == param) return;
    nwy_ext_echo("\r\n cipgsmloc info\r\n");
    if(0 == param->result)
    {
        nwy_ext_echo("lat %lf \r\n", param->info.data.lat);
        nwy_ext_echo("lng %lf \r\n", param->info.data.lng);
        nwy_ext_echo("accuracy %lf \r\n", param->info.data.alt);
    }
    else
    {
        nwy_ext_echo(" %s\r\n", param->info.errmsg);
    }
    return ;
}

void nwy_location_test()
{
   int ret = -1;
   int result = -1;
   int result0 = -1;
   int result1 = -1;
   int result2 = -1;
   int nmea_freq = 1000;
   int msg_type = 255;
   int i = 0;
   char nmea_data[2048] = {0};
   nwy_loc_position_mode_t pos_mode;
   nwy_loc_startup_mode startup;
   nwy_lbs_info_t databuf;
   nwy_ncell_lbs_info_t data_info;

   bool loc_base_on = true;
   bool assisted_loc_on = true;
   char* sptr = nwy_ext_sio_recv_buff;
   char AGPS_URL[128] = {0};
   char AGPS_USER[128] = {0};
   char AGPS_PASS[128] = {0};
   int port = 0;

   nwy_ext_input_gets("\r\n set choice (1 == close output data to uart2, else output data to uart2)");
   if(1 == atoi(sptr))
   {
        nwy_loc_close_uart_nmea_data();
        nwy_ext_echo("\r\n close success");
   }

   ret = nwy_loc_start_navigation();
   if (ret) {
		nwy_ext_echo("\r\n open location success");
   } else {
		nwy_ext_echo("\r\n open location fail");
   }
   nwy_sleep(500);

   while(1)
   {
   	nwy_ext_location_menu();
    nwy_ext_input_gets("\r\n Please input option: ");
    switch(atoi(sptr))
    {
     case 1:
	 {
         #ifdef FEATURE_NWY_AT_HX_GNSS
         nwy_ext_input_gets("\r\n set position(1-gps  3-gps+bd ):");
         #else
         nwy_ext_input_gets("\r\n set position(1-gps 3-gps+bd 5-gps+glo 7-gps+bd+glo):");
         #endif
         pos_mode = atoi(sptr);
         if (pos_mode > 7 || pos_mode < 1) {
            result = false;
            nwy_ext_echo("\r\n set position mode fail,invalid param");
         } else {
            nwy_ext_echo("\r\n set position mode:%d", pos_mode);
            result = nwy_loc_set_position_mode(pos_mode);
         }
         if (result) {
            nwy_ext_echo("\r\n set position mode success");
         } else {
            nwy_ext_echo("\r\n set position mode fail");
         }
         break;
      }
      case 2:
      {
         nwy_ext_input_gets("\r\n set location update rate(1000-1HZ 500-2HZ)ms:");
         nmea_freq = atoi(sptr);
         if (nmea_freq > 1000 || nmea_freq < 500) {
            result = false;
            nwy_ext_echo("\r\n set location update rate fail,invalid param");
         } else {
            nwy_ext_echo("\r\n update rate:%d", nmea_freq);
            result = nwy_loc_nmea_format_mode(2,nmea_freq);
         }
         if (result) {
            nwy_ext_echo("\r\n set location update rate success");
          } else {
            nwy_ext_echo("\r\n set location update rate fail");
         }
          break;
      }
     case 3:
	 {
	     nwy_ext_input_gets("\r\n set nmea statements output format(128-255):");
	     msg_type = atoi(sptr);
	     nwy_ext_echo("\r\n output format:%d", msg_type);
	     result = nwy_loc_nmea_format_mode(3,msg_type);
	     if (result) {
			nwy_ext_echo("\r\n set nmea statements output format success");
	     } else {
			nwy_ext_echo("\r\n set nmea statements output format fail");
	     }
         break;
     }
	case 4:
	{
	    nwy_ext_input_gets("\r\n set startup mode(0-hot 1-warm 2-cold 4-factory):");
        startup = atoi(sptr);
        nwy_ext_echo("\r\n startup mode:%d",startup);
        result = nwy_loc_set_startup_mode(startup);
	    if (result) {
			nwy_ext_echo("\r\n set nmea statements output format success");
	    } else {
			nwy_ext_echo("\r\n set nmea statements output format fail");
	    }
        nwy_sleep(500);
	    break;
	}
	case 5:
	{
	   result = nwy_loc_get_nmea_data(nmea_data);
	   if (result) {
			nwy_ext_echo("\r\n get nmea data success");
	   } else {
			nwy_ext_echo("\r\n get nmea data fail");
	   }
       nwy_ext_echo("\r\n %s", nmea_data);
	   break;
	}
	case 6:
	{
		nwy_ext_echo("\r\n need module dail at+xiic = 1");
	    nwy_ext_virtual_at_test();
		break;
	}
	case 7:
	{
        nwy_sleep(50);

        nwy_ext_input_gets("\r\nPlease input agps server url:");
        memcpy(AGPS_URL,sptr,strlen(sptr));

        nwy_ext_input_gets("\r\nPlease input agps server port:");
        port = atoi(sptr);

        nwy_ext_input_gets("\r\nPlease input agps server user:");
        memcpy(AGPS_USER,sptr,strlen(sptr));

        nwy_ext_input_gets("\r\nPlease input agps server pass:");
        memcpy(AGPS_PASS,sptr,strlen(sptr));

        result = nwy_loc_set_server(AGPS_URL,port,AGPS_USER,AGPS_PASS);
        if (result) {
            nwy_ext_echo("\r\n set location server success");
        } else {
            nwy_ext_echo("\r\n set location server fail");
        }
		break;
	}
	case 8:
	{
		if (1 == nwy_ext_check_data_connect())
		{
			nwy_pdp_set_status(NWY_PDP_CONNECTED);
		}
		else
		{
			nwy_ext_echo("\r\n need module dail at+xiic = 1");
	    	nwy_ext_virtual_at_test();
		}

		nwy_sleep(50);

        nwy_ext_input_gets("\r\n set location base position(1-on 0-off):");
		loc_base_on = atoi(sptr);
	    nwy_ext_echo("\r\n lbs_mode:%d", loc_base_on);

		nwy_loc_get_lbs_info(&databuf);
		nwy_ext_echo("\r\n cell_id:%d lac:%d mcc:%d mnc:%d csq:%d imei:%s",databuf.cell_id,databuf.lac,databuf.mcc,databuf.mnc,
			databuf.rssi_csq,databuf.imei_str);

		memset(&data_info,0x00,sizeof(data_info));
        nwy_loc_get_ncell_lbs_info(&data_info);
		nwy_ext_echo("\r\n cid num: %d ",data_info.num);
		for(i = 0; i < data_info.num; i++)
		{
		   nwy_ext_echo("\r\n index:%d pci:%d lac:%d cell_id:%d mcc:%d mnc:%d arcfn:%d rsrp:%d",i,data_info.ncell_lbs_info[i].pci,data_info.ncell_lbs_info[i].lac,
		   	data_info.ncell_lbs_info[i].cell_id,data_info.ncell_lbs_info[i].mcc,data_info.ncell_lbs_info[i].mnc,data_info.ncell_lbs_info[i].arcfn,data_info.ncell_lbs_info[i].rsrp);
		}

		result1 = nwy_loc_cipgsmloc_open(loc_base_on, nwy_cipgsmloc_cb);
        if (result1) {
          nwy_ext_echo("\r\n lbs  success");
        } else {
          nwy_ext_echo("\r\n lbs fail");
        }
	    nwy_sleep(1000);
        break;
	}
	case 9:
	{
	    nwy_ext_input_gets("\r\n open assisted location(1-on 0-off):");
	    assisted_loc_on = atoi(sptr);
	    nwy_ext_echo("\r\n assisted_loc_on:%d", assisted_loc_on);
	    result2 = nwy_loc_agps_open(assisted_loc_on);
	    if (result2) {
			 nwy_ext_echo("\r\n assisted location success");
	    } else {
			 nwy_ext_echo("\r\n assisted location fail");
	    }
        break;
	 }
	 case 10:
	 {
	    result = nwy_loc_stop_navigation();
	    if (result) {
			nwy_ext_echo("\r\n close location position success");
	    } else {
			nwy_ext_echo("\r\n close location position fail");
	    }
	    break;
	  }
	  case 99:
      {
          return;
      }

       default:
           break;
	}
   }
}

static int capture_len = 0;
static uint8_t *audio_buf;
#define AUDIO_RECORDER_DATA_LEN (1024 * 48 * 2)
#define BUF_SIZE_APP_8K 320
#define BUF_SIZE_APP_16K 640

typedef enum
{
    NWY_AUDIO_POC_SampleRaet_8K = 0,
    NWY_AUDIO_POC_SampleRaet_16K,
} nwy_poc_samplerate_type_t;
nwy_poc_samplerate_type_t nwy_poc_samplerate = NWY_AUDIO_POC_SampleRaet_8K;

static int nwy_player_cb(nwy_player_status state)
{
  nwy_ext_echo("nwytest_player_cb state=%d", state);
}

void nwy_audio_playback_test()
{
    int read_index =0;
    int result =0;
    int tmp_buf_size = 320;

    nwy_ext_echo("audio player is running\r\n");

    if(NWY_AUDIO_POC_SampleRaet_8K == nwy_poc_samplerate)
        tmp_buf_size = BUF_SIZE_APP_8K;
    else if(NWY_AUDIO_POC_SampleRaet_16K == nwy_poc_samplerate)
        tmp_buf_size = BUF_SIZE_APP_16K;

    nwy_audio_player_open(nwy_player_cb);

    while(read_index < capture_len)
    {
      result = nwy_audio_player_play(&audio_buf[read_index], tmp_buf_size);
      read_index += tmp_buf_size;
    }
    nwy_audio_player_stop();
    nwy_audio_player_close();
}
static int capture_callback(unsigned char *pdata, unsigned int len)
{
    nwy_ext_echo("capture_callback len = %d\r\n", len);

    if((capture_len + len) < (AUDIO_RECORDER_DATA_LEN))
    {
        memcpy(audio_buf + capture_len, pdata, len);
        capture_len += len;
    }
    else
    {
        nwy_ext_echo("recorder data is overflow\r\n");
    }
}

void nwy_audio_capture_test()
{
   nwy_ext_echo("audio recorder is running\r\n");

    nwy_audio_recorder_open(capture_callback);

    nwy_audio_recorder_start();

    nwy_sleep(500 * 6);

    nwy_audio_recorder_stop();
    nwy_audio_recorder_close();
}

void nwy_audio_test()
{
    audio_buf = (uint8_t*)malloc(AUDIO_RECORDER_DATA_LEN);
    if(NULL == audio_buf){
        nwy_ext_echo("audio malloc failed\r\n");
        return;
    }
    memset(audio_buf, 0, AUDIO_RECORDER_DATA_LEN);
    nwy_audio_capture_test();
    nwy_sleep(500);
    nwy_audio_playback_test();
    nwy_ext_echo("audio test down\r\n");
    capture_len = 0;
    free(audio_buf);
    audio_buf = NULL;
}

void nwy_set_poc_sampleRate_test()
{
    nwy_ext_echo("nwy_set_poc_sampleRate_test\r\n");

    char* sptr = nwy_ext_sio_recv_buff;
    nwy_ext_echo("\r\n 0: 8K \r\n");
    nwy_ext_echo("1: 16K \r\n");

    nwy_ext_input_gets("\r\nPlease input audio poc samplerate : ");
    switch(atoi(sptr))
    {
       case 0:
           nwy_poc_samplerate = NWY_AUDIO_POC_SampleRaet_8K;
           nwy_ext_echo("case 0 \r\n");
           nwy_set_poc_sampleRate(8000);
           break;

       case 1:
           nwy_poc_samplerate = NWY_AUDIO_POC_SampleRaet_16K;
           nwy_ext_echo("case 1 \r\n");
           nwy_set_poc_sampleRate(16000);
           break;
       default:
           break;
    }
}

/* add by wangchen for simcross 20200601 begin */
int test_get_simid()
{
    char* sptr = nwy_ext_sio_recv_buff;
	uint8 nSwitchSimID = 0;

	nSwitchSimID = nwy_sim_get_simid();

	nwy_ext_echo("\r\n simid:%d \r\n", nSwitchSimID);
	return 0;
}

int test_set_simid()
{
    char* sptr = nwy_ext_sio_recv_buff;
	uint8 nSwitchSimID = 0;
	nwy_result_type ret = NWY_RES_OK;

	nwy_ext_input_gets("\r\nPlease input switch simid: ");
	nSwitchSimID = atoi(sptr);

	ret = nwy_sim_set_simid(nSwitchSimID);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "nwy set switch simid fail");
		return -1;
	}

	return 0;
}
/* add by wangchen for simcross 20200601 end */
/* add by wangchen for get number 20200804 begin */
int test_get_msisdn()
{
    char* sptr = nwy_ext_sio_recv_buff;
    char msisdn[128] = {0};
    int ret = 0;
    nwy_sim_id_t simid;

    nwy_ext_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);

    ret = nwy_sim_get_msisdn(simid, msisdn, 128);
    if(0 != ret)
    {
        OSI_LOGI(0, "=UIM= nwy get msisdn fail");
        return -1;
    }

    nwy_ext_echo("\r\n msisdn:%s \r\n", msisdn);
    return 0;
}
/* add by wangchen for get number 20200804 end */

int test_verify_pin()
{
    char* sptr = nwy_ext_sio_recv_buff;
    char pin[10] = {0};
    nwy_sim_id_t simid;
    int ret = 0;

    nwy_ext_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);

    nwy_ext_input_gets("\r\nPlease input pin string: ");
    memcpy(pin,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

    ret = nwy_sim_verify_pin(simid, pin);
    if(0 != ret)
    {
        OSI_LOGI(0, "=UIM= nwy verify pin fail");
        return -1;
    }

    return 0;
}

int test_verify_puk()
{
    char* sptr = nwy_ext_sio_recv_buff;
    char puk[10] = {0};
    char new_pin[10] = {0};
    nwy_sim_id_t simid;
    int ret = 0;

    nwy_ext_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);

    nwy_ext_input_gets("\r\nPlease input puk string: ");
    memcpy(puk,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

    nwy_ext_input_gets("\r\nPlease input new pin string: ");
    memcpy(new_pin,nwy_ext_sio_recv_buff,strlen(nwy_ext_sio_recv_buff));

     ret = nwy_sim_unblock(simid, puk, new_pin);
    if(0 != ret)
    {
        OSI_LOGI(0, "=UIM= nwy verify puk fail");
        return -1;
    }

    return 0;
}


/* add by wangchen for get IMEI 20200228 begin */
int test_get_imei()
{
    char* sptr = nwy_ext_sio_recv_buff;
	nwy_sim_result_type imei = {0};
	nwy_result_type ret = NWY_RES_OK;

	ret = nwy_sim_get_imei(&imei);
	if(NWY_SMS_SUCCESS != ret)
	{
	    OSI_LOGI(0, "=UIM= nwy get imei fail");
		return -1;
	}

    nwy_ext_echo("\r\n imei:%s \r\n", imei.nImei);
	return 0;
}

void nwy_device_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    int ret = 0;

    nwy_ext_echo("\r\nPlease select an option to test below. \r\n");
    nwy_ext_echo("1. get imei test \r\n");
    /* add by wangchen for simcross 20200601 begin */
    nwy_ext_echo("2. get simid test \r\n");
    nwy_ext_echo("3. set simid test \r\n");
    /* add by wangchen for simcross 20200601 end */
    /* add by wangchen for get number 20200804 begin */
    nwy_ext_echo("4. get msisdn test \r\n");
    /* add by wangchen for get number 20200804 end */
    nwy_ext_echo("5. verify pin test \r\n");
    nwy_ext_echo("6. verify puk test \r\n");

    nwy_ext_input_gets("\r\nPlease input option: ");

    switch(atoi(sptr))
    {
        case 1:
        {
            ret = test_get_imei();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get imei fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get imei success \r\n");
            }
            break;
        }
        /* add by wangchen for simcross 20200601 begin */
        case 2:
        {
            ret = test_get_simid();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get simid fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get simid success \r\n");
            }
            break;
        }
        case 3:
        {
            ret = test_set_simid();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n set simid fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n set simid success \r\n");
            }
            break;
        }
        /* add by wangchen for simcross 20200601 end */
        /* add by wangchen for get number 20200804 begin */
        case 4:
        {
            ret = test_get_msisdn();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n get msisdn fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n get msisdn success \r\n");
            }
            break;
        }
        /* add by wangchen for get number 20200804 end */
        case 5:
        {
            ret = test_verify_pin();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n verify pin fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n verify pin success \r\n");
            }
            break;
        }
        case 6:
        {
            ret = test_verify_puk();
            if(0 != ret)
            {
                nwy_ext_echo("\r\n verify puk fail \r\n");
            }
            else
            {
                nwy_ext_echo("\r\n verify puk success \r\n");
            }
            break;
        }
        default:
        {
            nwy_ext_echo("\r\n invalid input \r\n");
            break;
        }
    }
}
/* add by wangchen for get IMEI 20200228 end */

#ifndef FEATURE_NWY_OPEN_LITE_TTS_OFF
static void tts_play_callback(void *cb_para, nwy_neoway_result_t result)
{
    switch(result)
    {
    case PLAY_END:
        nwy_ext_echo("\r\n tts test down \r\n");
        break;
    }
}
void nwy_ext_tts_menu()
{
  nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
  nwy_ext_echo("1. input content\r\n");
  nwy_ext_echo("2. start play \r\n");
  nwy_ext_echo("3. stop play\r\n");
  nwy_ext_echo("4. Exit \r\n");
}

void nwy_tts_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_tts_encode_t encode_type = 0;
    char buff[1024]= {0};
    char *hexbuf = "cae8d3b0bae1d0b1cbaec7e5c7b3a3acb0b5cfe3b8a1b6afd4c2bbc6bbe8";
    strncpy(buff, hexbuf, strlen(hexbuf));
    nwy_tts_stop_play();

    while (1)
    {
        nwy_ext_tts_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        switch(atoi(sptr))
        {
            case 1:
            {
                memset(buff, 0, 1024);
                nwy_ext_input_gets("\r\nPlease input encode mode(0-gbk,1-utf16le,2-utf16be,3-utf8): ");
                encode_type = atoi(sptr);
                nwy_ext_input_gets("\r\nPlease input content: ");
                strncpy(buff, sptr, strlen(sptr));
                break;
            }
            case 2:
            {
                if(strlen(buff)==0)
                    nwy_tts_playbuf(hexbuf, strlen(hexbuf), ENCODE_GBK, tts_play_callback, NULL);
                else
                    nwy_tts_playbuf(buff, strlen(buff), encode_type, tts_play_callback, NULL);
                break;
            }
            case 3:
            {
                nwy_tts_stop_play();
                break;
            }
            case 4:
                return ;
            default:
                break;
        }
    }
}
#endif

void nwy_dtmf_test()
{
    char* tone = nwy_ext_sio_recv_buff;
    nwy_ext_input_gets("\r\nPlease input DTMF: ");
    nwy_audio_tone_play(tone, 200, 15);
}

void nwy_auto_poweroff(NWY_SVR_MSG_SERVICE_E msg, uint32_t param)
{
    nwy_ext_echo("nwy charger msg = %d\n", msg);
}


void nwy_charging_cb(NWY_SVR_MSG_SERVICE_E msg, uint32_t param)
{
    if(msg == NWY_CHARGE_START_IND)
        OSI_LOGI(0, "nwy start chargering");
    else if(msg == NWY_CHARGE_DISCONNECT)
        OSI_LOGI(0, "nwy disconnect chargering");
    else if(msg == NWY_CHARGE_FINISH)
        OSI_LOGI(0, "nwy charger finished");
}

void nwy_pm_auto_test(void)
{
    uint8_t nBcs = 0;
    uint8_t nBcl = 0;
    nwy_ext_input_gets("Please select which demo you want to test(0-auto poweroff,1-chaging instructions: ");
    int opt = atoi(nwy_ext_sio_recv_buff);
    if(opt == 0)
    {
        nwy_set_auto_poweroff(3300, 3000, 20, nwy_auto_poweroff);
    }
    else if(opt == 1)
    {
        nwy_chargering_instructions(nwy_charging_cb);
    }
    else if(opt == 2)
    {
        nwy_get_charge_info(&nBcs, &nBcl);
        OSI_LOGI(0, "yms charger return info = %u,battery = %u",nBcs,nBcl);
    }
}

void nwy_close_key_poweroff_test()
{
    nwy_ext_input_gets("Please input status(0-close,1(default)-open: ");
    int opt = atoi(nwy_ext_sio_recv_buff);
    nwy_powerkey_poweroff_ctrl(opt);

    nwy_ext_echo("\r\n nwy_close_key_poweroff_test in %d",opt);
}

void nwy_at_cmd_process_callback(void* handle, char* atcmd, int type, char* para0, char* para1, char* para2)
{
  switch(type)
  {
  case 0:
    nwy_at_forward_send(handle, "\r\n",2);
    nwy_at_forward_send(handle, atcmd, strlen(atcmd));
    if(para0)
    {
      nwy_at_forward_send(handle, para0, strlen(para0));
    }

    if(para1)
    {
      nwy_at_forward_send(handle, para1, strlen(para1));
    }

    if(para2)
    {
      nwy_at_forward_send(handle, para2, strlen(para2));
    }

    nwy_at_forward_send(handle, "\r\nOK\r\n", 6);
    break;
  case 1:
    nwy_at_forward_send(handle, "\r\nTEST\r\n", 8);
    break;
  case 2:
    nwy_at_forward_send(handle, "\r\nREAD\r\n", 8);
    break;
  case 3:
    nwy_at_forward_send(handle, "\r\nEXE\r\n", 7);
    break;
  }
}

extern void nwy_lcd_init(void);
extern void nwy_lcd_deinit(void);
extern void nwy_write_data_buf(void *buf, unsigned int len);
extern void nwy_lcd_block_write(unsigned char startx, unsigned char starty, unsigned char endx, unsigned char endy);

static unsigned short display_buffer[128*128];
void nwy_lcd_test(void)
{
    nwy_lcd_init();

    nwy_ext_input_gets("\r\nPlease input option:\r\n0:draw line\r\n1:draw chinese\r\n2:close lcd\r\n");
    int opt = atoi(nwy_ext_sio_recv_buff);
    switch(opt)
    {
        case 0:
          {
            nwy_lcd_block_write(0,96,127,96);
            for(int i=0;i<128;i++)
            {
                display_buffer[i] = 0x7BE0;
            }
            nwy_write_data_buf(display_buffer, 128*2);
            break;
          }
        case 1:
          {
            nwy_lcd_block_write(45,19, 108, 34);
            nwy_dispstrline(16,"��λ����",45, 19,0x0000, 0xC6B3);
            break;
          }
        case 2:
          {
            nwy_lcd_deinit();
            break;
          }
        default:
            nwy_ext_echo("\r\n nwy_lcd_test error",opt);
            break;
    }
}

void nwy_ext_appimg_info_proc(const uint8 *data, int length)
{
    memcpy((g_recv->data + g_recv->pos), data, length);
    g_recv->pos = g_recv->pos + length;

    nwy_ext_echo("\r\n nwy_app_info.rev_len = %d length = %d\r\n", g_recv->pos, length);
    if (g_recv->pos  == g_recv->size) {
        nwy_ext_send_sig(g_app_thread, NWY_EXT_APPIMG_FOTA_DATA_REC_END);
        nwy_update_flag = 0;
    }
    if (g_recv->pos > g_recv->size) {
        nwy_ext_send_sig(g_app_thread, NWY_EXT_APPIMG_FOTA_DATA_REC_END);
        nwy_update_flag = 0;
        nwy_ext_echo("\r\nAPP info is error,upgrade fail");
    }
}


void nwy_appimg_fota_test()
{
	int fd;
	ota_package_t ota_pack = {0};
	int ota_size = 0;
	int tmp_len = 0;
	int read_len = 0;
	char buff[NWY_APPIMG_FOTA_BLOCK_SIZE] = {0};
	int ret = -1;
	int len = 0;
	nwy_osiEvent_t event;
	char *fileanme = "app.bin";

    //nwy_get_fota_result();

    nwy_ext_input_gets("\r\n Please input app packet size:");
    int rtn;
    rtn  = atoi(nwy_ext_sio_recv_buff);
    if (rtn <= 0)
    {
        nwy_ext_echo("\r\n Fota Error : invalid packet size:%s", nwy_ext_sio_recv_buff);
        return ;
    }
    nwy_ext_echo("\r\n update packet size:%d", rtn);
    uint32_t size = (uint32_t)rtn;
    g_recv = malloc(sizeof(dataRecvContext_t) + size);
    if (g_recv == NULL)
    {
        nwy_ext_echo("\r\n Fota Error : malloc packet buffer fail");
        return ;
    }

    g_recv->size = size;
    g_recv->pos = 0;
    nwy_update_flag = 2;

    nwy_ext_echo("\r\n Please send app.bin... \r\n");

    //while(1)
    {
        memset(&event, 0, sizeof(event));
        nwy_wait_thead_event(g_app_thread, &event, 0);
        if (event.id == NWY_EXT_APPIMG_FOTA_DATA_REC_END){
            nwy_ext_echo("\r\n recv %d bytes appimg packet success", g_recv->size);

			if(nwy_sdk_fexist(fileanme))
			{
	            ret = nwy_sdk_file_unlink(fileanme);
	            nwy_ext_echo("\r\n del %s \r\n", fileanme);

				nwy_sleep(500);
	        }

            fd = nwy_sdk_fopen(fileanme, NWY_CREAT | NWY_RDWR | NWY_TRUNC);

            len = nwy_sdk_fwrite(fd, g_recv->data, g_recv->size);

            nwy_sdk_fclose(fd);
            if(len != g_recv->size)
                nwy_ext_echo("\r\nfile write: len=%d, return len=%d\r\n",g_recv->size, len);
            else
                nwy_ext_echo("\r\nfile write success and close\r\n");

            free(g_recv);
            g_recv = NULL;
        }
    }

	fd = nwy_sdk_fopen("app.bin", NWY_RDONLY);

	if(fd < 0)
	{
		nwy_ext_echo("\r\nopen appimg fail\r\n");
		return;
	}

	ota_pack.data = (unsigned char *)malloc(NWY_APPIMG_FOTA_BLOCK_SIZE);
	ota_pack.len = 0;
	ota_pack.offset = 0;

	if(ota_pack.data == NULL)
	{
		nwy_ext_echo("\r\nmalloc fail\r\n");
		return;
	}

	ota_size = nwy_sdk_fsize_fd(fd);

	nwy_ext_echo("ota_size:%d\r\n", ota_size);

	nwy_sdk_fseek(fd, 0, NWY_SEEK_SET);

	while(ota_size > 0)
	{
		tmp_len = NWY_APPIMG_FOTA_BLOCK_SIZE;
		if(ota_size < tmp_len)
		{
			tmp_len = ota_size;
		}
		read_len = nwy_sdk_fread(fd, buff, tmp_len);
		if(read_len <= 0)
		{
			nwy_ext_echo("read file error:%d\r\n", read_len);
			free(ota_pack.data);

			nwy_sdk_fclose(fd);
			return;
		}
		nwy_ext_echo("read len:%d\r\n", read_len);
		memcpy(ota_pack.data, buff, read_len);
		ota_pack.len = read_len;

		ret = nwy_fota_dm(&ota_pack);

		if(ret < 0)
		{
			nwy_ext_echo("write ota error:%d\r\n", ret);
			free(ota_pack.data);

			nwy_sdk_fclose(fd);
			return;
		}
		ota_pack.offset += read_len;
		memset(ota_pack.data, 0, read_len);
		ota_size -= read_len;
	}
	free(ota_pack.data);
	nwy_sdk_fclose(fd);

	nwy_ext_echo("write end\r\n");

	nwy_ext_echo("start checksum\r\n");

	ret = nwy_package_checksum();
	if(ret < 0)
	{
		nwy_ext_echo("checksum failed\r\n");
		return;
	}

	nwy_ext_echo("start update\r\n");
	ret = nwy_fota_ua();
	if(ret < 0)
	{
		nwy_ext_echo("update failed\r\n");
		return;
	}
}

bool nwy_check_str_isdigit(char *str)
{
    if(NULL == str)
        return false;
    char *p = str;
    while('\0' != *p)
    {
        if(!('0' <= *p && '9' >= *p))
            return false;
        p++;
    }

    return true;
}

void nwy_test_ftp_menu()
{
    nwy_ext_echo("\r\nPlease select an option to test ftp below. \r\n");
    nwy_ext_echo("1. nwy_ftp_login \r\n");
    nwy_ext_echo("2. nwy_ftp_get \r\n");
    nwy_ext_echo("3. nwy_ftp_put \r\n");
    nwy_ext_echo("4. nwy_ftp_filesize \r\n");
    nwy_ext_echo("5. nwy_ftp_logout \r\n");
    nwy_ext_echo("6. check file exist \r\n");
    nwy_ext_echo("7. delete file \r\n");
    nwy_ext_echo("8. Exit ftp function test \r\n");
}
void nwy_channel_test_ftp_menu()
{
    nwy_ext_echo("\r\nPlease select an option to test multiplex ftp below. \r\n");
    nwy_ext_echo("1. nwy_multiplex_ftp_login \r\n");
    nwy_ext_echo("2. nwy_multiplex_ftp_get \r\n");
    nwy_ext_echo("3. nwy_multiplex_ftp_put \r\n");
    nwy_ext_echo("4. nwy_multiplex_ftp_filesize \r\n");
    nwy_ext_echo("5. nwy_multiplex_ftp_logout \r\n");
    nwy_ext_echo("6. check file exist \r\n");
    nwy_ext_echo("7. delete file \r\n");
    nwy_ext_echo("8. Exit ftp function test \r\n");
}

int nwy_test_ftp_write_file(nwy_file_ftp_info_s *pFileFtp, unsigned char* data,unsigned int len)
{
    char file_name[128+1] = "/nwy/";
    strcat(file_name,pFileFtp->filename);
    int fs =nwy_sdk_fopen(file_name, NWY_CREAT | NWY_RDWR);
    if(fs < 0)
    {
        OSI_LOGE(0, "nwy file open fail");
        return -1;
    }

    nwy_sdk_fseek(fs, pFileFtp->pos, 0);
    nwy_sdk_fwrite(fs, data, len);
    nwy_sdk_fclose(fs);
    pFileFtp->pos += len;
    OSI_LOGE(0, "nwy file write %d size success.", len);
    return 0;
}

void nwy_ftp_result_cb(nwy_ftp_result_t *param)
{
    int *size;
    int ret;
    if(NULL == param)
    {
        nwy_ext_echo("event is NULL\r\n");
    }

    OSI_LOGI(0, "event is %d", param->event);
    if(NWY_FTP_EVENT_LOGIN == param->event)
    {
        nwy_ext_echo("\r\nFtp login success");
    }
    else if(NWY_FTP_EVENT_PASS_ERROR == param->event)
    {
        nwy_ext_echo("\r\nFtp passwd error");
    }
    else if(NWY_FTP_EVENT_FILE_NOT_FOUND == param->event)
    {
        nwy_ext_echo("\r\nFtp file not found");
    }
    else if(NWY_FTP_EVENT_FILE_SIZE_ERROR == param->event)
    {
        nwy_ext_echo("\r\nFtp file size error");
    }
    else if(NWY_FTP_EVENT_SIZE == param->event)
    {
        size = param->data;
        nwy_ext_echo("\r\nFtp size is %d", *size);
    }
    else if(NWY_FTP_EVENT_LOGOUT == param->event)
    {
        nwy_ext_echo("\r\nFtp logout");
    }
    else if(NWY_FTP_EVENT_CLOSED == param->event)
    {
        nwy_ext_echo("\r\nFtp connection closed");
    }
    else if(NWY_FTP_EVENT_SIZE_ZERO == param->event)
    {
        nwy_ext_echo("\r\nFtp size is zero");
    }
    else if(NWY_FTP_EVENT_FILE_DELE_SUCCESS == param->event)
    {
        nwy_ext_echo("\r\nFtp file del success");
    }
    else if(NWY_FTP_EVENT_DATA_PUT_FINISHED == param->event)
    {
        nwy_ext_echo("\r\nFtp put file success");
    }
    else if(NWY_FTP_EVENT_DNS_ERR == param->event || NWY_FTP_EVENT_OPEN_FAIL == param->event)
    {
        nwy_ext_echo("\r\nFtp login fail");
    }
    else if(NWY_FTP_EVENT_DATA_GET == param->event)
    {
        //nwy_ext_echo("\r\n recv data");
        if(NULL == param->data)
        {
            nwy_ext_echo("\r\nrecved data is NULL");
            return;
        }

        if(1 == g_fileftp.is_vaild)
        {
            if(param->data_len != 0 )
            {
                ret = nwy_test_ftp_write_file(&g_fileftp, param->data, param->data_len);
                if(0 != ret)
                {
                    nwy_ext_echo("\r\nwrite failed");
                }
                else
                {
                    //nwy_ext_echo("\r\n write success");
                }
            }
        }
        else
        {
            //nwy_ext_echo("\r\ndata_size is %d", param->data_len);
            //nwy_ext_echo("\r\nparam->data is %s", param->data);
        }
    }
    else if(NWY_FTP_EVENT_DATA_CLOSED == param->event)
    {
        g_fileftp.is_vaild = 0;
        nwy_ext_echo("\r\nfile %s download success.", g_fileftp.filename);
    }
    else if(NWY_FTP_EVENT_DATA_OPEN_FAIL == param->event)
    {
        nwy_ext_echo("\r\nfile data open faile.");
    }
    else if(NWY_FTP_EVENT_DATA_SETUP_ERR == param->event)
    {
        nwy_ext_echo("\r\nFtp data setup error");
    }
    else
    {
        //nwy_ext_echo("\r\n data_size is %d", param->data_len);
        //nwy_ext_echo("\r\n param->data is %s", param->data);
    }
    return;
}


void nwy_test_http_menu()
{
    nwy_ext_echo("\r\nPlease select an option to test http/https below. \r\n");
    nwy_ext_echo("0. nwy_http_setup_ex \r\n");
    nwy_ext_echo("1. nwy_http_setup \r\n");
    nwy_ext_echo("2. nwy_http_get \r\n");
    nwy_ext_echo("3. nwy_http_head \r\n");
    nwy_ext_echo("4. nwy_http_post \r\n");
    nwy_ext_echo("5. nwy_http_close \r\n");
    nwy_ext_echo("6. Exit http function test \r\n");
}
void nwy_test_multiplex_http_menu()
{
    nwy_ext_echo("\r\nPlease select an option to test multiplex http/https below. \r\n");
    nwy_ext_echo("1. nwy multiplex http setup \r\n");
    nwy_ext_echo("2. nwy multiplex http get \r\n");
    nwy_ext_echo("3. nwy multiplex http head\r\n");
    nwy_ext_echo("4. nwy multiplex http post \r\n");
    nwy_ext_echo("5. nwy multiplex http close \r\n");
    nwy_ext_echo("6. Exit multiplex http function test \r\n");
}

void nwy_http_result_ex_cb(nwy_ftp_result_t *param, void *context)
{
    int *size;
    int ret;
    if(NULL == param)
    {
        nwy_ext_echo("event is NULL\r\n");
        return;
    }

    nwy_ext_echo("context address is %p value is %d\r\n", context, *(int *)context);

    OSI_LOGI(0, "event is %d", param->event);
    if(NWY_HTTP_DNS_ERR == param->event)
    {
        nwy_ext_echo("\r\nHTTP dns err");
    }
    else if(NWY_HTTP_OPEN_FAIL == param->event)
    {
        nwy_ext_echo("\r\nHTTP open fail");
    }
    else if(NWY_HTTP_OPENED == param->event )
    {
        nwy_ext_echo("\r\nHTTP setup success");
    }
    else if(NWY_HTTPS_SSL_CONNECTED == param->event)
    {
        nwy_ext_echo("\r\nHTTPS setup success");
    }
    else if(NWY_HTTP_CLOSED_PASV == param->event || NWY_HTTP_CLOSED == param->event)
    {
        nwy_ext_echo("\r\nHTTP closed");
    }
    else if(NWY_HTTP_DATA_RECVED == param->event)
    {
        nwy_ext_echo("\r\nHTTP recv data len %d.\r\n",param->data_len);
        char *buff = (char *)malloc(NWY_EXT_SIO_RX_MAX);
        if(NULL == buff)
        {
            nwy_ext_echo("%s\r\n", param->data);
            return;
        }
        int offset = 0;
        int tmeplen = 0;
        while(param->data_len)
        {
            memset(buff, 0x00, NWY_EXT_SIO_RX_MAX);
            tmeplen = param->data_len > NWY_EXT_SIO_RX_MAX ? NWY_EXT_SIO_RX_MAX : param->data_len;
            memcpy(buff, param->data + offset, tmeplen);
            nwy_ext_echo("%s", buff);
            offset += tmeplen;
            param->data_len -= tmeplen;
            OSI_LOGI(0, "param->data_len %d", param->data_len);
        }
        free(buff);
        nwy_ext_echo("\r\n");
    }
    else if(NWY_HTTP_DATA_SEND_ERR == param->event)
    {
        nwy_ext_echo("\r\ndata send error");
    }
    else if(NWY_HTTP_DATA_SEND_FINISHED == param->event)
    {
        nwy_ext_echo("\r\ndata send finished");
    }
    else if(NWY_HTTPS_SSL_INIT_ERROR == param->event)
    {
        nwy_ext_echo("\r\nHTTPS SSL init fail");
    }
    else if(NWY_HTTPS_SSL_HANDSHAKE_ERROR == param->event)
    {
        nwy_ext_echo("\r\nHTTPS SSL handshare fail");
    }
    else if(NWY_HTTPS_SSL_AUTH_FAIL == param->event)
    {
        nwy_ext_echo("\r\nHTTPS SSL Authentication fail");
    }
    else
    {
        nwy_ext_echo("\r\n unkown error");
    }

    return;
}


void nwy_http_result_cb(nwy_ftp_result_t *param)
{
    int *size;
    int ret;
    if(NULL == param)
    {
        nwy_ext_echo("event is NULL\r\n");
    }

    OSI_LOGI(0, "event is %d", param->event);
    if(NWY_HTTP_DNS_ERR == param->event)
    {
        nwy_ext_echo("\r\nHTTP dns err");
    }
    else if(NWY_HTTP_OPEN_FAIL == param->event)
    {
        nwy_ext_echo("\r\nHTTP open fail");
    }
    else if(NWY_HTTP_OPENED == param->event )
    {
        nwy_ext_echo("\r\nHTTP setup success");
    }
    else if(NWY_HTTPS_SSL_CONNECTED == param->event)
    {
        nwy_ext_echo("\r\nHTTPS setup success");
    }
    else if(NWY_HTTP_CLOSED_PASV == param->event || NWY_HTTP_CLOSED == param->event)
    {
        nwy_ext_echo("\r\nHTTP closed");
    }
    else if(NWY_HTTP_DATA_RECVED == param->event)
    {
        nwy_ext_echo("\r\nHTTP recv data len %d.\r\n",param->data_len);
        char *buff = (char *)malloc(NWY_EXT_SIO_RX_MAX);
        if(NULL == buff)
        {
            nwy_ext_echo("%s\r\n", param->data);
            return;
        }
        int offset = 0;
        int tmeplen = 0;
        while(param->data_len)
        {
            memset(buff, 0x00, NWY_EXT_SIO_RX_MAX);
            tmeplen = param->data_len > NWY_EXT_SIO_RX_MAX ? NWY_EXT_SIO_RX_MAX : param->data_len;
            memcpy(buff, param->data + offset, tmeplen);
            nwy_ext_echo("%s", buff);
            offset += tmeplen;
            param->data_len -= tmeplen;
            OSI_LOGI(0, "param->data_len %d", param->data_len);
        }
        free(buff);
        nwy_ext_echo("\r\n");
    }
    else if(NWY_HTTP_DATA_SEND_ERR == param->event)
    {
        nwy_ext_echo("\r\ndata send error");
    }
    else if(NWY_HTTP_DATA_SEND_FINISHED == param->event)
    {
        nwy_ext_echo("\r\ndata send finished");
    }
    else if(NWY_HTTPS_SSL_INIT_ERROR == param->event)
    {
        nwy_ext_echo("\r\nHTTPS SSL init fail");
    }
    else if(NWY_HTTPS_SSL_HANDSHAKE_ERROR == param->event)
    {
        nwy_ext_echo("\r\nHTTPS SSL handshare fail");
    }
    else if(NWY_HTTPS_SSL_AUTH_FAIL == param->event)
    {
        nwy_ext_echo("\r\nHTTPS SSL Authentication fail");
    }
    else
    {
        nwy_ext_echo("\r\n unkown error");
    }

    return;
}

void nwy_http_test()
{
    int result;
    uint8_t channel;
    char url[NWY_HTTP_URL_MAX_LEN] = {0};
    char buf[NWY_HTTP_POST_MAX_DATA_LEN] = {0};
    char filename[128] = {0};
    int port;
    int keepalive;
    int offset;
    int size;
    uint8_t type;
    char* sptr = nwy_ext_sio_recv_buff;
    static int choice;
    nwy_app_ssl_conf_t ssl_cfg;
    nwy_osiEvent_t event;
    int test = 10;
    while(1)
    {
        nwy_test_http_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        int opt = atoi(sptr);
        switch(opt)
        {
            case 0:
            {
                nwy_ext_input_gets("\r\n input channel:(1-7)");
                channel = atoi(sptr);
                if ((channel <= 0) || (channel > 7))
                {
                  nwy_ext_echo("\r\nInvaild channel: %d ",channel);
                  break;
                }
                nwy_ext_input_gets("\r\n input url:");
                strcpy(url, sptr);
                nwy_ext_input_gets("\r\n input port:");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("input error");
                    break;
                }
                port = atoi(sptr);

                nwy_ext_echo("\r\n test address is %p: value is %d", &test, test);
                result = nwy_http_setup_ex(channel, url, port, (void *)&test, nwy_http_result_ex_cb);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_setup_ex success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_setup_ex fail");
                }
                break;
            }
            case 1:
            {
                nwy_ext_input_gets("\r\n input channel:(1-7)");
                channel = atoi(sptr);
                if ((channel <= 0) || (channel > 7))
                {
                  nwy_ext_echo("\r\nInvaild channel: %d ",channel);
                  break;
                }
                nwy_ext_input_gets("\r\n input url:");
                strcpy(url, sptr);
                nwy_ext_input_gets("\r\n input port:");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("input error");
                    break;
                }
                port = atoi(sptr);

                nwy_ext_input_gets("\r\n input 0 setup http, 1 setup https:");
                choice = atoi(sptr) ? true : false;
                if(choice)
                {
                    memset(&ssl_cfg, 0x00, sizeof(ssl_cfg));
                    nwy_ext_input_gets("\r\n input ssl version: 0:SSL3.0 1:TLS1.0 2:TLS1.1 3:TLS1.2 ");
                    ssl_cfg.ssl_version = atoi(sptr);
                    nwy_ext_input_gets("\r\n input authmode: 0:No authentication 1:Manage server authentication 2:Manage server and client authentication");
                    ssl_cfg.authmode = atoi(sptr);
                    if(0 != ssl_cfg.authmode)
                    {
                        nwy_ext_input_gets("\r\n input cacert name:");
                        strcpy(ssl_cfg.cacert.cert_name, sptr);
                        nwy_ext_input_gets("\r\n input clientcert name:");
                        strcpy(ssl_cfg.clientcert.cert_name, sptr);
                        nwy_ext_input_gets("\r\n input clientkey name:");
                        strcpy(ssl_cfg.clientkey.cert_name, sptr);
                    }

                    result = nwy_https_setup(channel, url, port, nwy_http_result_cb, &ssl_cfg);
                }
                else
                {
                    result = nwy_http_setup(channel, url, port, nwy_http_result_cb);
                }

                if (0 == result)
                {
                    nwy_ext_echo("\r\n %s success",choice ? "nwy_https_setup" : "nwy_http_setup");
                }
                else
                {
                    nwy_ext_echo("\r\n %s fail",choice ? "nwy_https_setup" : "nwy_http_setup");
                }
                break;
            }
            case 2:
            {
                nwy_ext_input_gets("\r\n input whether keepalive:(0 1)");
                keepalive = atoi(sptr);
                nwy_ext_input_gets("\r\n input offset:");
                offset = atoi(sptr);
                nwy_ext_input_gets("\r\n input size:");
                size = atoi(sptr);
                result = nwy_http_get(keepalive, offset, size, choice);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_get success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_get fail");
                }
                break;
            }
            case 3:
            {
                nwy_ext_input_gets("\r\n input whether keepalive:(0 1)");
                keepalive = atoi(sptr);
                result = nwy_http_head(keepalive, choice);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_head success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_head fail");
                }
                break;
            }
            case 4:
            {
                nwy_ext_input_gets("\r\n input whether keepalive:(0 1)");
                keepalive = atoi(sptr);
                nwy_ext_input_gets("\r\n input type: 0:x-www-form-urlencoded 1:text 2:json 3:xml 4:html 99:User-defined mode");
                type = atoi(sptr);
                nwy_ext_input_gets("\r\n input data:");
                strcpy(buf, sptr);
                result = nwy_http_post(keepalive, type, buf, strlen(buf),  choice);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_post success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_post fail");
                }
                break;
            }
            case 5:
            {
                nwy_ext_input_gets("\r\n input 0 close http, 1 close https:");
                choice = atoi(sptr) ? true : false;
                result = nwy_http_close(choice);
                if (0 == result)
                {
                  nwy_ext_echo("\r\n %s success",choice ? "nwy_https_close" : "nwy_http_close");
                }
                else
                {
                  nwy_ext_echo("\r\n %s fail",choice ? "nwy_https_close" : "nwy_http_close");
                }
                break;
            }
            case 6:
                return;
            default:
                nwy_ext_echo("\r\nPlease input vaild value");
                break;
        }
     }
}

void nwy_multiplex_http_test()
{
    int result;
    uint8_t channel;
    char url[NWY_HTTP_URL_MAX_LEN] = {0};
    char buf[NWY_HTTP_POST_MAX_DATA_LEN] = {0};
    char filename[128] = {0};
    int port;
    int keepalive;
    int offset;
    int size;
    uint8_t type;
    char* sptr = nwy_ext_sio_recv_buff;
    static int choice;
    nwy_app_ssl_conf_t ssl_cfg;
    nwy_osiEvent_t event;
    while(1)
    {
        nwy_test_multiplex_http_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        int opt = atoi(sptr);
        switch(opt)
        {
            case 1:
            {
                nwy_ext_input_gets("\r\n input channel:(1-7)");
                channel = atoi(sptr);
                nwy_ext_input_gets("\r\n input url:");
                strcpy(url, sptr);
                nwy_ext_input_gets("\r\n input port:");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("input error");
                    break;
                }
                port = atoi(sptr);

                nwy_ext_input_gets("\r\n input input 0 setup http 1 setup https:");
                choice = atoi(sptr) ? true : false;
                if(choice)
                {
                    memset(&ssl_cfg, 0x00, sizeof(ssl_cfg));
                    nwy_ext_input_gets("\r\n input ssl version: 0:SSL3.0 1:TLS1.0 2:TLS1.1 3:TLS1.2:");
                    ssl_cfg.ssl_version = atoi(sptr);
                    nwy_ext_input_gets("\r\n input authmode: 0:No authentication 1:Manage server authentication 2:Manage server and client authentication");
                    ssl_cfg.authmode = atoi(sptr);
                    if(0 != ssl_cfg.authmode)
                    {
                        nwy_ext_input_gets("\r\n input cacert name:");
                        strcpy(ssl_cfg.cacert.cert_name, sptr);
                        nwy_ext_input_gets("\r\n input clientcert name:");
                        strcpy(ssl_cfg.clientcert.cert_name, sptr);
                        nwy_ext_input_gets("\r\n input clientkey name:");
                        strcpy(ssl_cfg.clientkey.cert_name, sptr);
                    }

                    result = nwy_multiplex_https_setup(channel, url, port, nwy_http_result_cb, &ssl_cfg);
                }
                else
                {
                    result = nwy_multiplex_http_setup(channel, url, port, nwy_http_result_cb);
                }

                if (0 == result)
                {
                    nwy_ext_echo("\r\n %s success",choice ? "nwy_https_setup" : "nwy_http_setup");
                }
                else
                {
                    nwy_ext_echo("\r\n %s fail",choice ? "nwy_https_setup" : "nwy_http_setup");
                }
                break;
            }
            case 2:
            {
                nwy_ext_input_gets("\r\n input channel:(1-7)");
                channel = atoi(sptr);
                nwy_ext_input_gets("\r\n input whether keepalive:(0 1)");
                keepalive = atoi(sptr);
                nwy_ext_input_gets("\r\n input offset:");
                offset = atoi(sptr);
                nwy_ext_input_gets("\r\n input size:");
                size = atoi(sptr);
                result = nwy_multiplex_http_get(channel,keepalive, offset, size, choice);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_get success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_get fail");
                }
                break;
            }
            case 3:
            {
                nwy_ext_input_gets("\r\n input channel:(1-7)");
                channel = atoi(sptr);
                nwy_ext_input_gets("\r\n input whether keepalive:(0 1)");
                keepalive = atoi(sptr);
                result = nwy_multiplex_http_head(channel, keepalive, choice);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_head success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_head fail");
                }
                break;
            }
            case 4:
            {
                nwy_ext_input_gets("\r\n input channel:(1-7)");
                channel = atoi(sptr);
                nwy_ext_input_gets("\r\n input whether keepalive:(0 1)");
                keepalive = atoi(sptr);
                nwy_ext_input_gets("\r\n input type: 0:x-www-form-urlencoded 1:text 2:json 3:xml 4:html 99:User-defined mode");
                type = atoi(sptr);
                nwy_ext_input_gets("\r\n input data:");
                strcpy(buf, sptr);
                result = nwy_multiplex_http_post(channel,keepalive, type, buf, strlen(buf),  choice);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_post success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_post fail");
                }
                break;
            }
            case 5:
            {
                nwy_ext_input_gets("\r\n input channel:(1-7)");
                channel = atoi(sptr);
                result = nwy_multiplex_http_close(channel, choice);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_http_close success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_http_close fail");
                }
                break;
            }
            case 6:
               return ;
            default:
                nwy_ext_echo("\r\nPlease input vaild value");
                break;
        }
     }
}

void  nwy_ftp_test()
{
    nwy_ftp_login_t ftp_param;
    nwy_result_type result;
    char* sptr = nwy_ext_sio_recv_buff;
    char filename[128] = {0};
    char type;
    char mode;
    int len = 0;
    char buff[128] = {0};
    nwy_osiEvent_t event;
    int is_ftps = 0;
    nwy_sleep(100);
    memset(&g_fileftp, 0x00, sizeof(g_fileftp));
    while(1)
    {
        nwy_test_ftp_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        int opt = atoi(sptr);
        switch(opt)
        {
            case 1:
            {
                memset(&ftp_param, 0x00, sizeof(ftp_param));
                nwy_ext_input_gets("\r\n input channel(1-7):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                ftp_param.channel = atoi(sptr);
                if(ftp_param.channel < 1 || ftp_param.channel > 7)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input ip:");
                strcpy((char*)ftp_param.host, sptr);
                nwy_ext_input_gets("\r\n input port:");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                ftp_param.port = atoi(sptr);
                nwy_ext_input_gets("\r\n input FTPtype (active mode = 1 passive mode = 0):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                ftp_param.mode = atoi(sptr);
                if(ftp_param.mode < 0 || ftp_param.mode > 1)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input username:");
                strcpy((char*)ftp_param.username, sptr);
                nwy_ext_input_gets("\r\n input passwd:");
                strcpy((char*)ftp_param.passwd, sptr);

                ftp_param.timeout = 180;
                nwy_ext_input_gets("\r\n Is FTPS(FTP over TLS) (1 for FTPS and 0 for FTP) ? : ");
                if(atoi(sptr) == 1)
                {
                    is_ftps = 1;
                }

                if(is_ftps)
                {
                    nwy_app_ssl_conf_t ssl_cfg;
                    int ftps_type;
                    memset(&ssl_cfg, 0x00, sizeof(ssl_cfg));
                    nwy_ext_input_gets("\r\n input ssl version: 0:SSL3.0 1:TLS1.0 2:TLS1.1 3:TLS1.2 ");
                    ssl_cfg.ssl_version = atoi(sptr);
                    nwy_ext_input_gets("\r\n input authmode: 0:No authentication 1:Manage server authentication 2:Manage server and client authentication");
                    ssl_cfg.authmode = atoi(sptr);
                    if(0 != ssl_cfg.authmode)
                    {
                        nwy_ext_input_gets("\r\n input cacert name:");
                        strcpy(ssl_cfg.cacert.cert_name, sptr);
                        nwy_ext_input_gets("\r\n input clientcert name:");
                        strcpy(ssl_cfg.clientcert.cert_name, sptr);
                        nwy_ext_input_gets("\r\n input clientkey name:");
                        strcpy(ssl_cfg.clientkey.cert_name, sptr);
                    }
                    nwy_ext_input_gets("\r\n input FTPS type (explicit TLS = 1 implicit TLS = 2):");
                    ftps_type = atoi(sptr);
                    if(ftps_type != NWY_OPEN_FTPS_TYPE_EXPLICIT && ftps_type != NWY_OPEN_FTPS_TYPE_IMPLICIT)
                    {
                        nwy_ext_echo("\r\n input error");
                        break;
                    }

                    result = nwy_ftp_ssl_login(&ftp_param, &ssl_cfg, ftps_type, nwy_ftp_result_cb);
                }
                else
                {
                    result = nwy_ftp_login(&ftp_param, nwy_ftp_result_cb);
                }
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_login success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_login fail");
                }

                break;
            }
            case 2:
            {
                nwy_ext_input_gets("\r\n input type:(1 ASCII 2 Binary)");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                type = atoi(sptr);
                if(type < 1 || type > 2)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input filename:");
                strcpy(buff, sptr);
                g_fileftp.is_vaild = 1;
                strcpy(g_fileftp.filename, sptr);
                result = nwy_ftp_get(buff, type, 0, 0);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_get success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_get fail");
                }
                break;
            }
            case 3:
            {
                nwy_ext_input_gets("\r\n input filename:");
                strcpy(filename, sptr);
                nwy_ext_input_gets("\r\n input type:(1 ASCII 2 Binary)");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                type = atoi(sptr);
                if(type < 1 || type > 2)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input mode:(1 STOR 2 APPE 3 DELE)");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                mode = atoi(sptr);
                if(mode < 1 || mode > 3)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input put data len:");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                len = atoi(sptr);;
                g_recv = malloc(sizeof(dataRecvContext_t) + len);
                if (g_recv == NULL)
                {
                    nwy_ext_echo("\r\n malloc buffer fail");
                    return ;
                }

                memset(g_recv, 0, sizeof(dataRecvContext_t) + len);
                g_recv->size = len;
                g_recv->pos = 0;
                nwy_update_flag = 1;
                nwy_ext_echo("\r\nplease input data");
                nwy_wait_thead_event(g_app_thread, &event, 0);
                if (event.id == NWY_EXT_FOTA_DATA_REC_END)
                {
                    result = nwy_ftp_put(filename, type, mode, g_recv->data, g_recv->size);
                    if (0 == result)
                    {
                        nwy_ext_echo("\r\n nwy_ftp_put success");
                    } else
                    {
                        nwy_ext_echo("\r\n nwy_ftp_put fail");
                    }
                    free(g_recv);
                    g_recv = NULL;
                }
                break;
            }
            case 4:
            {
                nwy_ext_input_gets("\r\n input filename:");
                strcpy(buff, sptr);
                result = nwy_ftp_filesize(buff, 30);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_filesize success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_filesize fail");
                }
                break;
            }
            case 5:
            {
                nwy_ext_echo("\r\n ftp will logout");
                result = nwy_ftp_logout(30);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_logout success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_logout fail");
                }
                break;
            }
            case 6:
            {
                nwy_ext_echo("\r\n Check if the file exists");
                char file_name[128+1] = "/nwy/";
                nwy_ext_input_gets("\r\n input filename");
                strcat(file_name, nwy_ext_sio_recv_buff);
                nwy_ext_echo("\r\n file_name is %s", file_name);

                int testfd = nwy_sdk_fopen(file_name, NWY_RDONLY);
                if(testfd < 0)
                {
                    nwy_ext_echo("\r\nfile open %s fail\r\n", file_name);
                    break;
                }
                nwy_ext_echo("\r\nfile open %s success\r\n", file_name);
                nwy_sdk_fclose(testfd);
                int size = nwy_sdk_fsize(file_name);
                nwy_ext_echo("\r\n get %s file size = %d\r\n", file_name, size);
                break;
            }
            case 7:
            {
               nwy_ext_echo("\r\n delete get file");
               char file_name[128+1] = "/nwy/";
               nwy_ext_input_gets("\r\n input filename");
               strcat(file_name, nwy_ext_sio_recv_buff);

               if(!nwy_sdk_fexist(file_name)) {
                   nwy_ext_echo("\r\n %s not exist", file_name);
                   break;
               }
               nwy_sdk_file_unlink(file_name);
               nwy_ext_echo("\r\n delete %s file success", file_name);
               break;
            }
            case 8:
                return;
            default:
                nwy_ext_echo("\r\nPlease input vaild value");
                break;
       }
    }
}

void  nwy_multiplex_ftp_test()
{
    nwy_ftp_login_t ftp_param;
    nwy_result_type result;
    char* sptr = nwy_ext_sio_recv_buff;
    char filename[128] = {0};
    char type;
    char mode;
    int channel = 0;
    char *data = NULL;
    int len = 0;
    char buff[128] = {0};
    nwy_osiEvent_t event;

    nwy_sleep(100);
    memset(&g_fileftp, 0x00, sizeof(g_fileftp));
    while(1)
    {
        nwy_channel_test_ftp_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        int opt = atoi(sptr);
        switch(opt)
        {
            case 1:
            {
                memset(&ftp_param, 0x00, sizeof(ftp_param));
                nwy_ext_input_gets("\r\n input channel(1-7):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                ftp_param.channel = atoi(sptr);
                if(ftp_param.channel < 1 || ftp_param.channel > 7)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input ip:");
                strcpy(ftp_param.host, sptr);
                nwy_ext_input_gets("\r\n input port:");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                ftp_param.port = atoi(sptr);
                nwy_ext_input_gets("\r\n input FTPtype (active mode = 1 passive mode = 0):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                ftp_param.mode = atoi(sptr);
                if(ftp_param.mode < 0 || ftp_param.mode > 1)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input username:");
                strcpy(ftp_param.username, sptr);
                nwy_ext_input_gets("\r\n input passwd:");
                strcpy(ftp_param.passwd, sptr);

                ftp_param.timeout = 180;
                result = nwy_multiplex_ftp_login(&ftp_param, nwy_ftp_result_cb);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_login success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_login fail");
                }

                break;
            }
            case 2:
            {
                nwy_ext_input_gets("\r\n input channel(1-7):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                channel = atoi(sptr);
                if(channel < 1 || channel > 7)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input type:(1 ASCII 2 Binary)");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                type = atoi(sptr);
                if(type < 1 || type > 2)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input filename:");
                strcpy(buff, sptr);
                g_fileftp.is_vaild = 1;
                strcpy(g_fileftp.filename, sptr);
                result = nwy_multiplex_ftp_get(channel,buff, type, 0, 0);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_get success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_get fail");
                }
                break;
            }
            case 3:
            {
                nwy_ext_input_gets("\r\n input channel(1-7):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                channel = atoi(sptr);
                if(channel < 1 || channel > 7)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input filename:");
                strcpy(filename, sptr);
                nwy_ext_input_gets("\r\n input type:(1 ASCII 2 Binary)");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                type = atoi(sptr);
                if(type < 1 || type > 2)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input mode:(1 STOR 2 APPE 3 DELE)");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                mode = atoi(sptr);
                if(mode < 1 || mode > 3)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input put data len:");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                len = atoi(sptr);
                g_recv = malloc(sizeof(dataRecvContext_t) + len);
                if (g_recv == NULL)
                {
                    nwy_ext_echo("\r\n malloc buffer fail");
                    return ;
                }

                memset(g_recv, 0, sizeof(dataRecvContext_t) + len);
                g_recv->size = len;
                g_recv->pos = 0;
                nwy_update_flag = 1;
                nwy_ext_echo("\r\nplease input data");
                nwy_wait_thead_event(g_app_thread, &event, 0);
                if (event.id == NWY_EXT_FOTA_DATA_REC_END)
                {
                    result = nwy_multiplex_ftp_put(channel, filename, type, mode, g_recv->data, g_recv->size);
                    if (0 == result)
                    {
                        nwy_ext_echo("\r\n nwy_ftp_put success");
                    } else
                    {
                        nwy_ext_echo("\r\n nwy_ftp_put fail");
                    }
                    free(g_recv);
                    g_recv = NULL;
                }
                break;
            }
            case 4:
            {
                nwy_ext_input_gets("\r\n input channel(1-7):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                channel = atoi(sptr);
                if(channel < 1 || channel > 7)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n input filename:");
                strcpy(buff, sptr);
                result = nwy_multiplex_ftp_filesize(channel, buff, 30);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_filesize success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_filesize fail");
                }
                break;
            }
            case 5:
            {
                nwy_ext_input_gets("\r\n input channel(1-7):");
                if(!nwy_check_str_isdigit(sptr))
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                channel = atoi(sptr);
                if(channel < 1 || channel > 7)
                {
                    nwy_ext_echo("\r\n input error");
                    break;
                }
                nwy_ext_input_gets("\r\n ftp will logout");
                result = nwy_multiplex_ftp_logout(channel, 30);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_ftp_logout success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_ftp_logout fail");
                }
                break;
            }
            case 6:
            {
                nwy_ext_echo("\r\n test file is exist");
                char file_name[128+1] = "/nwy/";
                nwy_ext_input_gets("\r\n input filename");
                strcat(file_name, nwy_ext_sio_recv_buff);
                nwy_ext_echo("\r\n file_name is %s", file_name);

                int testfd = nwy_sdk_fopen(file_name, NWY_RDONLY);
                if(testfd < 0)
                {
                    nwy_ext_echo("\r\nfile open %s fail\r\n", file_name);
                    break;
                }
                nwy_ext_echo("\r\nfile open %s success\r\n", file_name);
                nwy_sdk_fclose(testfd);
                int size = nwy_sdk_fsize(file_name);
                nwy_ext_echo("\r\n get %s file size = %d\r\n", file_name, size);
                break;
            }
            case 7:
            {
               nwy_ext_echo("\r\n delete get file");
               char file_name[128+1] = "/nwy/";
               nwy_ext_input_gets("\r\n input filename");
               strcat(file_name, nwy_ext_sio_recv_buff);

               if(!nwy_sdk_fexist(file_name)) {
                   nwy_ext_echo("\r\n %s not exist", file_name);
                   break;
               }
               nwy_sdk_file_unlink(file_name);
               nwy_ext_echo("\r\n delete %s file success", file_name);
               break;
            }
            case 8:
                return;
            default:
                nwy_ext_echo("\r\nPlease input vaild value");
                break;
       }
    }
}

#ifdef FEATURE_NWY_ALI_MQTT
void nwy_alimqtt_test();
void nwy_mqtt_publish_test();
void nwy_mqtt_disconn_test();
void nwy_mqtt_subscribe_test();
void nwy_mqtt_state_test();
void nwy_mqtt_unsub_test();
#endif

void nwy_sdcart_test()
{
    bool ret = false;
    ret = nwy_sdk_sdcard_mount();
    if (ret != true) {
        nwy_ext_echo("\r\nsdcard mount fail");
        return;
    }


    int fd = nwy_sdk_fopen("/sdcard0/test.txt", NWY_CREAT | NWY_RDWR|NWY_TRUNC);
    if(fd < 0)
    {
      nwy_ext_echo("\r\nfile open fail\r\n");
      return;
    }

    nwy_ext_echo("\r\nfile write: 123456\r\n");
    int len = nwy_sdk_fwrite(fd, "123456", 6);
    nwy_sdk_fclose(fd);
    if(len != 6)
    {
      nwy_ext_echo("\r\nfile write: len=%d, return len=%d\r\n",6, len);
      return;
    }
    nwy_ext_echo("\r\nfile write success and close\r\n");


    fd = nwy_sdk_fopen("/sdcard0/test.txt", NWY_RDONLY);
    if(fd < 0)
    {
      nwy_ext_echo("\r\nfile open fail\r\n");
      return;
    }

    int size = nwy_sdk_fsize("/sdcard0/test.txt");
    nwy_ext_echo("\r\nfile size = %d\r\n", size);


    nwy_ext_echo("\r\nfile read\r\n");
    char buf[10]={0};
    len = nwy_sdk_fread(fd, buf, sizeof(buf));
    if (len <= 0) {
        nwy_ext_echo("\r\nfile read fail\r\n");
    } else {
        nwy_ext_echo("\r\nfile read content:%s\r\n", buf);
    }
}

void nwy_read_sdcard_test()
{
    int opt = nwy_read_sdcart_status();
    nwy_ext_echo("\r\nNWY read sd status = %d\r\n", opt);

    nwy_ext_input_gets("Please select unmount sdcard(1-yes,0-no): ");
    int status = atoi(nwy_ext_sio_recv_buff);
    if(status)
        nwy_sdk_sdcard_unmount();
}

void nwy_cert_menu()
{
    nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
    nwy_ext_echo("1. nwy add cert \r\n");
    nwy_ext_echo("2. nwy cert check \r\n");
    nwy_ext_echo("3. nwy cert del \r\n");
    nwy_ext_echo("4. Exit cert function test \r\n");
}

void nwy_cert_test()
{
    int result;
    char filename[128] = {0};
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_osiEvent_t event;
    int test = 10;
    while(1)
    {
        nwy_cert_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        int opt = atoi(sptr);
        switch(opt)
        {
            case 1:
            {
                nwy_ext_input_gets("\r\n begin input cacert name:");
                strcpy(filename, sptr);
                nwy_ext_input_gets("\r\n input cacert length:");
                int length = atoi(sptr);
                if (length <= 0)
                {
                    nwy_ext_echo("\r\n invalid cacert size:%d", length);
                    return ;
                }
                g_recv = malloc(sizeof(dataRecvContext_t) + length + CERT_EFS_CHECKSUM_STR_SIZE +1);
                if (g_recv == NULL)
                {
                    nwy_ext_echo("\r\n malloc buffer fail");
                    return ;
                }
                memset(g_recv, 0, sizeof(dataRecvContext_t) + length + CERT_EFS_CHECKSUM_STR_SIZE +1);
                g_recv->size = length;
                g_recv->pos = 0;
                nwy_update_flag = 1;
                nwy_ext_echo("\r\nplease input cert");

                memset(&event, 0, sizeof(event));
                nwy_wait_thead_event(g_app_thread, &event, 0);
                if (event.id == NWY_EXT_FOTA_DATA_REC_END)
                {
                    //nwy_ext_echo("\r\n recv %d bytes data success", g_recv->size);
                    unsigned wsize = nwy_cert_add(filename, g_recv->data, g_recv->size);
                    if (wsize != g_recv->size)
                    {
                        nwy_ext_echo("\r\nnwy_cert_add fail");
                    }
                    else
                    {
                        nwy_ext_echo("\r\nnwy_cert_add success");
                    }
                    free(g_recv);
                    g_recv = NULL;
                }
                nwy_ext_echo("\r\nadd end");
                break;
            }
            case 2:
            {
                nwy_ext_input_gets("\r\n input cacert name:");
                memset(filename, 0x00, sizeof(filename));
                strcpy(filename, sptr);
                result = nwy_cert_check(filename);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_cert_check success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_cert_check fail");
                }
                break;
            }
            case 3:
            {
                nwy_ext_input_gets("\r\n input cacert name:");
                memset(filename, 0x00, sizeof(filename));
                strcpy(filename, sptr);
                result = nwy_cert_del(filename);
                if (0 == result)
                {
                    nwy_ext_echo("\r\n nwy_cert_del success");
                }
                else
                {
                    nwy_ext_echo("\r\n nwy_cert_del fail");
                }
                break;
            }

            case 4 :
                return;
            default:
                nwy_ext_echo("\r\nPlease input vaild value");
                break;
        }
     }
}

/**
 * event, with ID and 3 parameters
 */
typedef struct osiEvent
{
    uint32_t id;     ///< event identifier
    uint32_t param1; ///< 1st parameter
    uint32_t param2; ///< 2nd parameter
    uint32_t param3; ///< 3rd parameter
} osiEvent_t;

#if 0

#define NWY_APP_BLE_MSG_FLAG 0x08000000
#define NWY_APP_BLE_COMM_BASE 0x00600000
#define NWY_APP_BLE_ADV_BASE 0x00610000

#define NWY_APP_BLE_CONNECT (NWY_APP_BLE_MSG_FLAG | (NWY_APP_BLE_COMM_BASE + 6))
#define NWY_APP_BLE_UPDATA_CONNECT (NWY_APP_BLE_MSG_FLAG | (NWY_APP_BLE_COMM_BASE + 8))

#define NWY_APP_BLE_SET_ADV_ENABLE (NWY_APP_BLE_MSG_FLAG | (NWY_APP_BLE_ADV_BASE + 3))
#define NWY_APP_BLE_SET_ADV_SCAN_RSP (NWY_APP_BLE_MSG_FLAG | (NWY_APP_BLE_ADV_BASE + 4))


static void nwy_uart_recv_ble (short _length, unsigned char *_data)
{
    if (_length > 255)
    {
        nwy_ext_echo("\r\n nwy ble receive tp data,but data length is too long.");
    }
    else
    {
        nwy_ext_echo("\r\n nwy_uart_send data %s,length = %d\r\n",_data,_length);
    }

    return;

}

void nwy_recv_cb(void *param)
{
    char TraceBuf[64] = {
        0,
    };
    osiEvent_t *pEvent = (osiEvent_t *)param;
    if (NULL == pEvent)
    {
        TraceBuf[0] = '\0';
        sprintf(TraceBuf, "%s(%d): param is NULL", __func__, __LINE__);
        OSI_LOGI(0, TraceBuf);
        return;
    }

    TraceBuf[0] = '\0';
    sprintf(TraceBuf, "%s(%d): APP event id = 0x%lx", __func__, __LINE__, pEvent->id);
    OSI_LOGI(0, TraceBuf);

    switch (pEvent->id)
    {
    //COMM
    case NWY_APP_BLE_UPDATA_CONNECT:
    {
        if (0 == ((int)pEvent->param1))
        {
            //add receive handle
            nwy_ble_register_callback(nwy_uart_recv_ble);
            nwy_ext_echo("\r\nNWY APP_BLE_UPDATA_CONNECT\r\n");
        }
        else
        {
            nwy_ext_echo("\r\nNWY APP_BLE_UPDATA_CONNECT error\r\n");
        }
        break;
    }
    case NWY_APP_BLE_CONNECT:
    {
        if (0 == ((int)pEvent->param1))
        {
            //add receive handle
            nwy_ble_register_callback(nwy_uart_recv_ble);
            nwy_ext_echo("\r\nNWY APP_BLE_CONNECT\r\n");
        }
        else
        {
        {
            nwy_ext_echo("\r\nNWY APP_BLE_CONNECT error\r\n");
        }
        break;
    }
    case NWY_APP_BLE_SET_ADV_SCAN_RSP:
    {
        if (0 == ((int)pEvent->param1))
        {
            nwy_ble_register_callback(nwy_uart_recv_ble);
            nwy_ext_echo("\r\nNWY APP_BLE_SET_ADV_SCAN_RSP\r\n");
        }
        else
        {
            nwy_ext_echo("\r\nNWY APP_BLE_SET_ADV_SCAN_RSP error\r\n");
        }
        break;
    }
    case NWY_APP_BLE_SET_ADV_ENABLE:
    {
        if (0 == ((int)pEvent->param1))
        {
            //add receive handle
            nwy_ble_register_callback(nwy_uart_recv_ble);
            nwy_ext_echo("\r\nNWY APP_BLE_SET_ADV_ENABLE\r\n");
        }
        else
        {
            nwy_ext_echo("\r\nNWY APP_BLE_SET_ADV_ENABLE error\r\n");
        }
        break;
    }
    default:
        break;
    }

    if (NULL != pEvent)
    {
        free(pEvent);
    }
        return;
    }
}
#endif

 void nwy_ble_recv_data_func ()
{
    UINT8 *length = NULL;
    UINT8 *precv = NULL;
    length = nwy_ble_receive_data(0);
    precv = nwy_ble_receive_data(1);
    if( (NULL != precv) & (NULL != length))
    {
        nwy_ext_echo("\r\n nwy_bel_receive_data:(%d),%s",length,precv);
    }
    else
    {
        nwy_ext_echo("nwy_ble receive data is null.");
    }
    nwy_ble_receive_data(2);

    return;

}

void nwy_ble_conn_status_func ()
{
    int conn_status = 0;
    conn_status= nwy_ble_get_conn_status();
    if(conn_status != 0)
    {
        nwy_ext_echo("\r\n Ble connect,status:%d",conn_status);
    }
    else
    {
        nwy_ext_echo("\r\n Ble disconnect,status:%d",conn_status);
    }

    return;

}

void nwy_ble_get_mac_addr_func()
{
    UINT8 *ble_mac = NULL;
    ble_mac = nwy_ble_get_mac_addr();
    if(NULL != ble_mac)
    {
        nwy_ext_echo("\r\n Ble mac addr:%02x:%02x:%02x:%02x:%02x:%02x",ble_mac[5],ble_mac[4],ble_mac[3],ble_mac[2],ble_mac[1],ble_mac[0]);
    }
    else
    {
        nwy_ext_echo("\r\n ble get mac addr is null.");
    }

}

void nwy_set_ble_test()
{
    int port;
    int valu;
    int lab;
    int enable;
    int ble;
    int ble_len;
    int conn_status;
    char *dev_ver = NULL;
    char dev_name[9] = "nwyble";
    UINT8 uuid[16] = {0xb9,0x40,0x7f,0x30,0xf5,0xf8,0x46,0x6e,0xaf,0xf9,0x25,0x55,0x6b,0x57,0xfe,0x6d};
    UINT8 major[2] = {0x00,0x01};
    UINT8 minor[2] = {0x00,0x02};
    UINT8 srv_uuid[] = {0xfe10};
    UINT8 char_uuid_1[] = {0xfe11};
    UINT8 char_uuid_2[] = {0xfe12};
	UINT8 Manufacture_Data[8] = {0x13, 0xff, 0};
    UINT8 char_prop;


    while (1 )
    {
    nwy_ext_echo("\r\nBLE function");
    nwy_ext_echo("\r\n0-open");
    nwy_ext_echo("\r\n1-adv");
    nwy_ext_echo("\r\n2-send data");
    nwy_ext_echo("\r\n3-status");
    nwy_ext_echo("\r\n4-recv data");
    nwy_ext_echo("\r\n5-get version");
    nwy_ext_echo("\r\n6-set ble name");
    nwy_ext_echo("\r\n7-update conn");
    nwy_ext_echo("\r\n8-beacon");
    nwy_ext_echo("\r\n9-set manufacture");
    nwy_ext_echo("\r\n10-set service");
    nwy_ext_echo("\r\n11-set character");
    nwy_ext_echo("\r\n12-conn status report");
    nwy_ext_echo("\r\n13-conn status read");
    nwy_ext_echo("\r\n14-ble mac addr");
    nwy_ext_echo("\r\n15-close");
    nwy_ext_echo("\r\nq-exit");
    nwy_ext_input_gets("\r\nset the ble mode:");
    if(nwy_ext_sio_recv_buff[0] == 'q')
    {
        nwy_ext_echo("\r\nexit now \r\n");
        return ;
    }
    port = atoi(nwy_ext_sio_recv_buff);
    nwy_ext_echo("\r\nset mode:%d \r\n", port);

    switch(port)
    {
        case 0:
            enable = nwy_ble_enable();
            nwy_ext_echo("nwy_ble_enable(%d) \r\n", enable);
            break;

        case 1:
            nwy_ext_input_gets("set the adv enable(0-disable,1-enable):");
            valu = atoi(nwy_ext_sio_recv_buff);
            nwy_ble_set_adv(valu);
            nwy_ext_echo("nwy_ble_set_adv(%d) \r\n", valu);
            break;

        case 2:
            nwy_ext_input_gets("\r\n send data):");
            nwy_ext_echo("\r\nnwy_ble_send_data(%s) \r\n", nwy_ext_sio_recv_buff);
            nwy_ble_send_data(sizeof(nwy_ext_sio_recv_buff),nwy_ext_sio_recv_buff);
            break;

        case 3:
            lab= nwy_read_ble_status();
            nwy_ext_echo("nwy_read_ble_status(%d) \r\n", lab);

            break;

        case 4:
            nwy_ble_register_callback(nwy_ble_recv_data_func);
            break;

        case 5:
            dev_ver = nwy_ble_get_version();
            if( NULL != dev_ver)
            {
                nwy_ext_echo("\r\n nwy_ble_get_version:%s\r\n", dev_ver);
            }
            else
            {
                nwy_ext_echo("nwy_ble version data is null.");
            }

            break;

        case 6:
            nwy_ble_set_device_name(dev_name);
            nwy_ext_echo("\n new_set_dev_name:(%s) \r\n", dev_name);
            break;

        case 7:
            nwy_ble_update_conn(0,60,80,4,500);
            break;

        case 8:
            nwy_ble_beacon(uuid, major, minor);
            nwy_ext_echo("\r\nSet Beacon parm,OK!\r\n");
            break;

        case 9:
            nwy_set_manufacture(Manufacture_Data);
            nwy_ext_echo("\r\nSet manufacture parm,OK!\r\n");
            break;

        case 10:
            if (!nwy_ble_set_service(srv_uuid))
                nwy_ext_echo("\r\n BLE HAS STARTED !\r\n");
            break;

        case 11:
            nwy_ext_echo("\r\n char_prop: 0:Write 1:Write | Notify\r\n");
            nwy_ext_input_gets("char index 0 prop:");
            char_prop = atoi(nwy_ext_sio_recv_buff);
            if(!nwy_ble_set_character(0,char_uuid_1,char_prop))
                nwy_ext_echo("\r\n BLE HAS STARTED !\r\n");

            nwy_ext_echo("\r\n char_prop: 0:Read 1:Read | Notify\r\n");
            nwy_ext_input_gets("char index 1 prop:");;
            char_prop = atoi(nwy_ext_sio_recv_buff);
            if (!nwy_ble_set_character(1,char_uuid_2,char_prop))
                nwy_ext_echo("\r\n BLE HAS STARTED !\r\n");
            break;

        case 12:
            nwy_ble_conn_status_cb(nwy_ble_conn_status_func);
            break;

        case 13:
            conn_status= nwy_ble_get_conn_status();
            nwy_ext_echo("nwy_ble_conn_status:%d \r\n", conn_status);
            break;

        case 14:
            nwy_ble_get_mac_addr_func();
            break;

        case 15:
            nwy_ble_disable();
            nwy_ext_echo("\r\n BLE CLOSE !\r\n");
            break;

        default:
            break;
    }
    }
}

void nwy_get_chip_id_test()
{
    uint8_t uid[8] = {0};
    nwy_get_chip_id(uid);
    for(int i=0; i<8;i++)
    {
        nwy_ext_echo("\r\n nwy_get_chip_id_test 0x%02x \r\n", uid[i]);
    }
}
void nwy_get_ver_test()
{
    char dev_ver[64] = {0,};
    char lin_ver[64] = {0,};
    nwy_dm_get_device_version(dev_ver,lin_ver);
    nwy_ext_echo("\r\n nwy_get_ver_test dev_ver:%s,lin_ver:%s\r\n", dev_ver, lin_ver);
}
#define flash_size (4 * 1024)
#define flash_addr 0x607b0000
void nwy_flash_test()
{
    int ret = -1;
    int len = 0;
    uint8_t data_str[NWY_UART_RECV_SINGLE_MAX + 1] = {0};
    nwyspiflash_t *fp = NULL;
    fp = nwy_flash_open(NAME_FLASH_1);

    if(fp == NULL)
    {
        nwy_ext_echo("flash open filed \r\n");
        return;
    }
    ret = nwy_flash_erase(fp, flash_addr, flash_size);
    if (ret == false)
    {
        nwy_ext_echo("nwy_flash_erase filed \r\n");
        return;
    }

    nwy_ext_input_gets("\r\n send data):");
    len = nwy_ext_sio_len > NWY_UART_RECV_SINGLE_MAX? NWY_UART_RECV_SINGLE_MAX:nwy_ext_sio_len;
    strcpy(data_str, nwy_ext_sio_recv_buff);
    ret = nwy_flash_write(fp, flash_addr, data_str, sizeof(data_str));
    if (ret == false)
    {
        nwy_ext_echo("nwy_flash_write filed \r\n");
        return;
    }
    memset(data_str,0,sizeof(data_str));

    ret = nwy_flash_read(fp, flash_addr, data_str, sizeof(data_str));
    if (ret == false)
    {
        nwy_ext_echo("nwy_flash_read filed \r\n");
        return;
    }
    for(int i=0 ; i<len; i++ )
        nwy_ext_echo("\r\n nwy_flash_read 0x%02x \r\n", data_str[i]);

    return;
}

void nwy_boot_causes_test(void)
{
    uint32_t causes = nwy_get_boot_causes();
    char string_buf[256];
    memset(string_buf, 0, sizeof(string_buf));
    int idx = sprintf(string_buf, "boot cause[%02x]:", causes);
    if(causes == NWY_BOOTCAUSE_UNKNOWN)
        idx += sprintf(string_buf + idx, " %s", "UNKNOWN");
    else
    {
        if(causes & NWY_BOOTCAUSE_PWRKEY)
            idx += sprintf(string_buf + idx, " %s", "PWRKEY");
        if(causes & NWY_BOOTCAUSE_PIN_RESET)
            idx += sprintf(string_buf + idx, " %s", "PIN_RESET");
        if(causes & NWY_BOOTCAUSE_ALARM)
            idx += sprintf(string_buf + idx, " %s", "ALARM");
        if(causes & NWY_BOOTCAUSE_CHARGE)
            idx += sprintf(string_buf + idx, " %s", "CHARGE");
        if(causes & NWY_BOOTCAUSE_WDG)
            idx += sprintf(string_buf + idx, " %s", "WDG");
        if(causes & NWY_BOOTCAUSE_PIN_WAKEUP)
            idx += sprintf(string_buf + idx, " %s", "PIN_WAKEUP");
        if(causes & NWY_BOOTCAUSE_PSM_WAKEUP)
            idx += sprintf(string_buf + idx, " %s", "PSM_WAKEUP");
    }
    nwy_ext_echo("%s\r\n", string_buf);

}

/*Begin: Add by shuiying for paho mqtt  in 2020.08.24*/
#ifdef FEATURE_NWY_PAHO_MQTT
MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
MQTTClient paho_mqtt_client;
unsigned char* g_nwy_paho_readbuf = NULL;
unsigned char* g_nwy_paho_writebuf = NULL;
nwy_osiThread_t *nwy_paho_task_id = NULL;
Network *paho_network = NULL;
#define NWY_EXT_SIO_PER_LEN 1024
char echo_buff[NWY_EXT_SIO_PER_LEN + 1] = {0};
nwy_paho_mqtt_at_param_type paho_mqtt_at_param = {0};
nwy_osiMutex_t *ext_mutex = NULL;

void nwy_ext_paho_mqtt_menu()
{
  nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
  nwy_ext_echo("1. mqtt connect\r\n");
  nwy_ext_echo("2. mqtt pub\r\n");
  nwy_ext_echo("3. mqtt sub\r\n");
  nwy_ext_echo("4. mqtt unsub\r\n");
  nwy_ext_echo("5. mqtt disconn\r\n");
#ifdef FEATURE_NWY_N58_OPEN_NIPPON
  nwy_ext_echo("6. mqtt 10k message pub\r\n");
#endif
  nwy_ext_echo("7. Exit mqtt\r\n");
}

void messageArrived(MessageData* md)
{
  char topic_name[PAHO_TOPIC_LEN_MAX] = {0};
  int i = 0;
  unsigned int remain_len = 0;

  strncpy(topic_name, md->topicName->lenstring.data, md->topicName->lenstring.len);
  nwy_lock_mutex(ext_mutex, 0);
  nwy_ext_echo("\r\n===messageArrived======");
  nwy_ext_echo("\r\npayloader len is %d", md->message->payloadlen);
  nwy_ext_echo("\r\n%s:\r\n", topic_name);
  remain_len = md->message->payloadlen;
  if(md->message->payloadlen > NWY_EXT_SIO_PER_LEN)
  {
    for(i = 0 ;i < ((md->message->payloadlen / NWY_EXT_SIO_PER_LEN ) + 1); i++)
    {
      memset(echo_buff, 0, sizeof(echo_buff));
      strncpy(echo_buff, md->message->payload + i*NWY_EXT_SIO_PER_LEN,
              remain_len > NWY_EXT_SIO_PER_LEN ? NWY_EXT_SIO_PER_LEN : remain_len);
      remain_len = md->message->payloadlen- (i+1)*NWY_EXT_SIO_PER_LEN;
      nwy_ext_echo(echo_buff);
    }
  }
  else
  {
    memset(echo_buff, 0, sizeof(echo_buff));
    strncpy(echo_buff, md->message->payload, md->message->payloadlen);
    nwy_ext_echo(echo_buff);
  }
  nwy_unlock_mutex(ext_mutex);
}

void nwy_paho_cycle(void)
{
  while(1)
  {
    while (MQTTIsConnected(&paho_mqtt_client))
    {
      MQTTYield(&paho_mqtt_client, 500);
      nwy_sleep(200);
    }
    nwy_ext_echo("\r\nMQTT disconnect ,Out paho cycle");
    nwy_suspend_thread(nwy_paho_task_id);
  }
  nwy_sleep(200);
}


nwy_osiThread_t * nwy_paho_yeild_task_init(void)
{
  if (nwy_paho_task_id == NULL)
  {
    nwy_osiThread_t * task_id = nwy_create_thread("neo_paho_yeild_task",nwy_paho_cycle,
                   NULL, NWY_OSI_PRIORITY_NORMAL,1024*2,4);
    nwy_paho_task_id = task_id;
  }
  else
    nwy_resume_thread(nwy_paho_task_id);
  return nwy_paho_task_id;
}

void nwy_paho_mqtt_test()
{
  char* sptr = nwy_ext_sio_recv_buff;
  MQTTPacket_connectData* options;
  int ssl_mode = 0, auth_mode, qos, retained;
  MQTTMessage pubmsg = {0};
  int rc, i;
  ext_mutex = nwy_create_mutex();

  while (1)
  {
    nwy_lock_mutex(ext_mutex, 0);
    nwy_ext_paho_mqtt_menu();
    nwy_ext_echo("\r\nPlease input option:");
    nwy_unlock_mutex(ext_mutex);
    nwy_ext_input_gets("\r\n ");
    switch(atoi(sptr))
    {
      case 1:
        if (MQTTIsConnected(&paho_mqtt_client) == 1)
        {
          nwy_ext_echo("\r\npaho mqtt already connect");
          break;
        }
        memset(&paho_mqtt_at_param, 0, sizeof(nwy_paho_mqtt_at_param_type));
        nwy_ext_input_gets("\r\nPlease input url/ip: ");
        strncpy(paho_mqtt_at_param.host_name, sptr, strlen(sptr));
        nwy_ext_input_gets("\r\nPlease input port: ");
        paho_mqtt_at_param.port = atoi(sptr);

        nwy_ext_input_gets("\r\nPlease input client_id: ");
        strncpy(paho_mqtt_at_param.clientID, sptr, strlen(sptr));
        nwy_ext_input_gets("\r\nPlease input usrname: ");
        strncpy(paho_mqtt_at_param.username, sptr, strlen(sptr));
        nwy_ext_input_gets("\r\nPlease input password: ");
        strncpy(paho_mqtt_at_param.password, sptr, strlen(sptr));
        nwy_ext_input_gets("\r\nPlease input sslmode(1-ssl,0-no ssl): ");
        if(atoi(sptr)>1 || atoi(sptr)<0)
        {
            nwy_ext_echo("\r\ninput sslmode error");
            break;
        }
        paho_mqtt_at_param.paho_ssl_tcp_conf.sslmode = atoi(sptr);
        if(g_nwy_paho_writebuf!=NULL)
        {
          free(g_nwy_paho_writebuf);
          g_nwy_paho_writebuf=NULL;
        }
        if (NULL == (g_nwy_paho_writebuf = (unsigned char *)malloc(PAHO_PLAYOAD_LEN_MAX))) {
            nwy_ext_echo("\r\nmalloc buffer g_nwy_paho_writebuf error");
            break;
        }
        if(g_nwy_paho_readbuf!=NULL)
        {
          free(g_nwy_paho_readbuf);
          g_nwy_paho_readbuf=NULL;
        }
        if (NULL == (g_nwy_paho_readbuf = (unsigned char *)malloc(PAHO_PLAYOAD_LEN_MAX))) {
            nwy_ext_echo("\r\nmalloc buffer g_nwy_paho_readbuf error");
           break;
        }
        memset(g_nwy_paho_readbuf, 0, sizeof(g_nwy_paho_readbuf));
        memset(g_nwy_paho_writebuf, 0, sizeof(g_nwy_paho_writebuf));
        if(paho_network != NULL)
        {
          free(paho_network);
          paho_network = NULL;
        }
        if (NULL == (paho_network = (unsigned char *)malloc(sizeof(Network)))) {
            nwy_ext_echo("\r\nmalloc buffer paho_network error");
           break;
        }
        memset(paho_network, 0, sizeof(Network));
        NetworkInit(paho_network);
        if (paho_mqtt_at_param.paho_ssl_tcp_conf.sslmode == 1)
        {
          nwy_ext_input_gets("\r\nPlease input auth_mode(0/1/2): ");
          paho_mqtt_at_param.paho_ssl_tcp_conf.authmode = atoi(sptr);
          if(paho_mqtt_at_param.paho_ssl_tcp_conf.authmode == 0)
          {
            paho_network->tlsConnectParams.pRootCALocation = NULL;
            paho_network->tlsConnectParams.pDeviceCertLocation = NULL;
            paho_network->tlsConnectParams.pDevicePrivateKeyLocation = NULL;
          }
          else if(paho_mqtt_at_param.paho_ssl_tcp_conf.authmode == 1)
          {
            nwy_ext_input_gets("\r\nPlease input ca: ");
            strncpy( paho_mqtt_at_param.paho_ssl_tcp_conf.cacert.cert_name, sptr, strlen(sptr));
            paho_network->tlsConnectParams.pRootCALocation = paho_mqtt_at_param.paho_ssl_tcp_conf.cacert.cert_name;
            paho_network->tlsConnectParams.pDeviceCertLocation = NULL;
            paho_network->tlsConnectParams.pDevicePrivateKeyLocation = NULL;
          }
          else
          {
            nwy_ext_input_gets("\r\nPlease input ca: ");
            strncpy( paho_mqtt_at_param.paho_ssl_tcp_conf.cacert.cert_name, sptr, strlen(sptr));
            paho_network->tlsConnectParams.pRootCALocation = paho_mqtt_at_param.paho_ssl_tcp_conf.cacert.cert_name;
            nwy_ext_input_gets("\r\nPlease input clientcert: ");
            strncpy(paho_mqtt_at_param.paho_ssl_tcp_conf.clientcert.cert_name, sptr, strlen(sptr));
            nwy_ext_input_gets("\r\nPlease input clientkey: ");
            strncpy(paho_mqtt_at_param.paho_ssl_tcp_conf.clientkey.cert_name, sptr, strlen(sptr));
            paho_network->tlsConnectParams.pDeviceCertLocation =  paho_mqtt_at_param.paho_ssl_tcp_conf.clientcert.cert_name;
            paho_network->tlsConnectParams.pDevicePrivateKeyLocation = paho_mqtt_at_param.paho_ssl_tcp_conf.clientkey.cert_name;
          }
          paho_network->tlsConnectParams.ServerVerificationFlag = paho_mqtt_at_param.paho_ssl_tcp_conf.authmode;
          paho_network->is_SSL = 1;
          paho_network->tlsConnectParams.timeout_ms = 5000;
          nwy_ext_input_gets("\r\nPlease input sslversion(0-3)");
          if(atoi(sptr)>3 || atoi(sptr)<0)
          {
            nwy_ext_echo("\r\ninput sslversion error");
            break;
          }
          paho_network->tlsConnectParams.sslversion = atoi(sptr);
        }
         else
           nwy_ext_echo( "\r\nis no-SSL NetworkConnect");
         nwy_ext_input_gets("\r\nPlease input clean_flag(0/1): ");
         if(atoi(sptr)>1 || atoi(sptr)<0)
         {
           nwy_ext_echo("\r\ninput clean_flag error");
           break;
         }
         paho_mqtt_at_param.cleanflag = (sptr[0] - 0x30);
         nwy_ext_input_gets("\r\nPlease input keep_alive: ");
         paho_mqtt_at_param.keepalive = atoi(sptr);
         nwy_ext_echo("\r\nip:%s, port :%d",paho_mqtt_at_param.host_name, paho_mqtt_at_param.port);
         rc = NetworkConnect(paho_network, paho_mqtt_at_param.host_name, paho_mqtt_at_param.port);
         if (rc < 0)
         {
           nwy_ext_echo("\r\nNetworkConnect err rc=%d", rc);
           break;
         }
         nwy_ext_echo("\r\nNetworkConnect ok");

         MQTTClientInit(&paho_mqtt_client, paho_network, 10000, g_nwy_paho_writebuf, PAHO_PLAYOAD_LEN_MAX,
                        g_nwy_paho_readbuf, PAHO_PLAYOAD_LEN_MAX);
         MQTTClientInit_defaultMessage(&paho_mqtt_client, messageArrived);
         data.clientID.cstring = paho_mqtt_at_param.clientID;
         if (0 != strlen((char*)paho_mqtt_at_param.username)
             && 0 != strlen((char*)paho_mqtt_at_param.password))
         {
           data.username.cstring = paho_mqtt_at_param.username;
           data.password.cstring = paho_mqtt_at_param.password;
         }
         data.keepAliveInterval = paho_mqtt_at_param.keepalive;
         data.cleansession = paho_mqtt_at_param.cleanflag;
         if (0 != strlen((char*)paho_mqtt_at_param.willtopic))
         {
           memset(&data.will, 0x0, sizeof(data.will));
           data.willFlag = 1;
           data.will.retained = paho_mqtt_at_param.willretained;
           data.will.qos = paho_mqtt_at_param.willqos;
           if (paho_mqtt_at_param.willmessage_len != 0)
           {
             data.will.topicName.lenstring.data = paho_mqtt_at_param.willtopic;
             data.will.topicName.lenstring.len = strlen((char*)paho_mqtt_at_param.willtopic);
             data.will.message.lenstring.data = paho_mqtt_at_param.willmessage;
             data.will.message.lenstring.len = paho_mqtt_at_param.willmessage_len;
           }
           else
           {
             data.will.topicName.cstring = paho_mqtt_at_param.willtopic;
             data.will.message.cstring = paho_mqtt_at_param.willmessage;
           }
           nwy_ext_echo("\r\nMQTT will ready");
         }
         nwy_ext_echo("\r\nConnecting MQTT");
        if (rc = nwy_MQTTConnect(&paho_mqtt_client, &data))
            nwy_ext_echo("\r\nFailed to create client, return code %d", rc);
        else
        {
            nwy_ext_echo("\r\nMQTT connect ok");
            nwy_osiTimer_t *task_id = nwy_paho_yeild_task_init();
            if (task_id == NULL)
              nwy_ext_echo("\r\npaho yeid task create failed ");
            else
              nwy_ext_echo("\r\npaho yeid task create ok ");
        }
      break;
      case 2:
        if (MQTTIsConnected(&paho_mqtt_client))
        {
            memset(paho_mqtt_at_param.topic , 0, sizeof(paho_mqtt_at_param.topic));
            nwy_ext_input_gets("\r\nPlease input topic: ");
            strncpy(paho_mqtt_at_param.topic, sptr, strlen(sptr));
            nwy_ext_input_gets("\r\nPlease input qos: ");
            paho_mqtt_at_param.qos = atoi(sptr);
            nwy_ext_input_gets("\r\nPlease input retained: ");
            paho_mqtt_at_param.retained = atoi(sptr);
            memset(paho_mqtt_at_param.message , 0, sizeof(paho_mqtt_at_param.message));
#ifdef FEATURE_NWY_N58_OPEN_NIPPON
            nwy_ext_input_gets("\r\nPlease input message(<= 2048): ");
            if (nwy_ext_sio_len > NWY_EXT_SIO_RX_MAX) {
              nwy_ext_echo("\r\nNo more than 2048 bytes at a time ");
              break;
            }
#else
            nwy_ext_input_gets("\r\nPlease input message(<= 512): ");
            if (nwy_ext_sio_len > 512) {
              nwy_ext_echo("\r\nNo more than 512 bytes at a time ");
              break;
            }
#endif
            strncpy(paho_mqtt_at_param.message, sptr, strlen(sptr));
            nwy_ext_echo("\r\nmqttpub param retained = %d, qos = %d, topic = %s, msg = %s",
	                          paho_mqtt_at_param.retained,
	                          paho_mqtt_at_param.qos,
	                          paho_mqtt_at_param.topic,
	                          paho_mqtt_at_param.message);
	        memset(&pubmsg, 0, sizeof(pubmsg));
            pubmsg.payload =(void *)paho_mqtt_at_param.message;
            pubmsg.payloadlen = strlen(paho_mqtt_at_param.message);
            pubmsg.qos = paho_mqtt_at_param.qos;
            pubmsg.retained = paho_mqtt_at_param.retained;
            pubmsg.dup = 0;
            rc = nwy_MQTTPublish(&paho_mqtt_client, paho_mqtt_at_param.topic, &pubmsg);
            nwy_ext_echo("\r\nmqtt publish rc:%d", rc );
        }
        else
            nwy_ext_echo("\r\nMQTT not connect");
        break;
      case 3:
        if (MQTTIsConnected(&paho_mqtt_client))
        {
          memset(paho_mqtt_at_param.topic , 0, sizeof(paho_mqtt_at_param.topic));
          nwy_ext_input_gets("\r\nPlease input topic: ");
          strncpy(paho_mqtt_at_param.topic, sptr, strlen(sptr));
          nwy_ext_input_gets("\r\nPlease input qos: ");
          paho_mqtt_at_param.qos = atoi(sptr);
          rc = MQTTSubscribe(&paho_mqtt_client,
                    (char*)paho_mqtt_at_param.topic,
                    paho_mqtt_at_param.qos,
                    messageArrived);
          if (rc ==SUCCESS)
            nwy_ext_echo("\r\nMQTT Sub ok");
          else
            nwy_ext_echo("\r\nMQTT Sub error:%d", rc);
        }
        else
          nwy_ext_echo("\r\nMQTT no connect");
        break;
      case 4:
        if (MQTTIsConnected(&paho_mqtt_client))
        {
          memset(paho_mqtt_at_param.topic , 0, sizeof(paho_mqtt_at_param.topic));
          nwy_ext_input_gets("\r\nPlease input topic: ");
          strncpy(paho_mqtt_at_param.topic, sptr, strlen(sptr));
          rc = MQTTUnsubscribe(&paho_mqtt_client, paho_mqtt_at_param.topic);
          if (rc ==SUCCESS)
            nwy_ext_echo("\r\nMQTT Unsub ok");
          else
            nwy_ext_echo("\r\nMQTT Unsub error:%d", rc);
        }
        else
          nwy_ext_echo("\r\nMQTT no connect");
        break;
      case 5:
        if(MQTTIsConnected(&paho_mqtt_client))
        {
            MQTTDisconnect(&paho_mqtt_client);
            NetworkDisconnect(paho_network);
            if (NULL != g_nwy_paho_writebuf) {
              free(g_nwy_paho_writebuf);
              g_nwy_paho_writebuf=NULL;
            }
            if (NULL != g_nwy_paho_readbuf) {
              free(g_nwy_paho_readbuf);
              g_nwy_paho_readbuf=NULL;
            }
            if(NULL != paho_network )
            {
              free(paho_network);
              paho_network = NULL;
            }
        }
        nwy_ext_echo("\r\nMQTT disconnect ok");
        break;
#ifdef FEATURE_NWY_N58_OPEN_NIPPON
      case 6:
        if (MQTTIsConnected(&paho_mqtt_client))
        {
          memset(paho_mqtt_at_param.topic , 0, sizeof(paho_mqtt_at_param.topic));
          nwy_ext_input_gets("\r\nPlease input topic: ");
          strncpy(paho_mqtt_at_param.topic, sptr, strlen(sptr));
          nwy_ext_input_gets("\r\nPlease input qos: ");
          paho_mqtt_at_param.qos = atoi(sptr);
          nwy_ext_input_gets("\r\nPlease input retained: ");
          paho_mqtt_at_param.retained = atoi(sptr);
          memset(paho_mqtt_at_param.message , 0, sizeof(paho_mqtt_at_param.message));
          nwy_ext_input_gets("\r\nPlease input 1k str,msg(10k) consists of 10 str");
          if (nwy_ext_sio_len != 1024) {
            nwy_ext_echo("\r\nmust be 1k message");
            break;
          }
          for( i = 0; i < 10; i++)
              strncpy(paho_mqtt_at_param.message + i*strlen(sptr), sptr, strlen(sptr));
          nwy_ext_echo("\r\nmqttpub param retained = %d, qos = %d, topic = %s, strlen is %d",
                        paho_mqtt_at_param.retained,
                        paho_mqtt_at_param.qos,
                        paho_mqtt_at_param.topic,
                        strlen(paho_mqtt_at_param.message));
          memset(&pubmsg, 0, sizeof(pubmsg));
          pubmsg.payload =(void *)paho_mqtt_at_param.message;
          pubmsg.payloadlen = strlen(paho_mqtt_at_param.message);
          pubmsg.qos = paho_mqtt_at_param.qos;
          pubmsg.retained = paho_mqtt_at_param.retained;
          pubmsg.dup = 0;
          rc = nwy_MQTTPublish(&paho_mqtt_client, paho_mqtt_at_param.topic, &pubmsg);
          nwy_ext_echo("\r\nmqtt publish rc:%d", rc );
        }
        else
            nwy_ext_echo("\r\nMQTT not connect");
        break;
#endif
     case 7:
        nwy_delete_mutex(ext_mutex);
        return ;
      default:
        break;
    }
  }
}
#endif
/*End:  Add by shuiying for paho mqtt  in 2020.08.24*/

void nwy_pwm_test()
{
    nwy_pwm_t* test_p;
    test_p = nwy_pwm_clk_div_init(NAME_PWM_1, 100, 2000, 40);

    if(test_p == NULL)
    {
        nwy_ext_echo("\r\nPWM init failed!");
        return;
    }
    nwy_pwm_start(test_p);

    nwy_ext_input_gets("\r\nPlease ensure you want to release the pwm(1-yes,0-no): ");
    int opt = atoi(nwy_ext_sio_recv_buff);

    if(opt)
    {
        nwy_pwm_stop(test_p);
        nwy_pwm_deinit(test_p);
        nwy_ext_echo("\r\nPWM is already closed!");
    }
}

void nwy_time_zone_test()
{
    nwy_time_zone_switch status = 0;
    char* sptr = nwy_ext_sio_recv_buff;

    nwy_ext_echo("\r\n1.Set time zone synchronization status");
    nwy_ext_echo("\r\n2.Get time zone synchronization status");
    nwy_ext_echo("\r\n0.Exit");
    nwy_ext_input_gets("\r\nPlease input option: ");


    switch(atoi(sptr))
    {
        case 0:
            return;
            break;
        case 1:
            nwy_ext_input_gets("\r\nPlease input set status(0-disable 1-enable): ");
            status = atoi(sptr);
            nwy_set_time_zone_switch(status);
            break;
        case 2:
            nwy_get_time_zone_switch(&status);
            nwy_ext_echo("\r\nget time zone synchronization is %d", status);
        default:
            break;
    }

}

void nwy_audio_file_play_test()
{
    char filename[16] = {0};
    char* sptr = nwy_ext_sio_recv_buff;

    nwy_ext_input_gets("\r\nPlease input filename: \r\n");
    strncpy(filename, sptr, strlen(sptr));
    nwy_ext_echo("\r\nfilename = %s \r\n", filename);

    nwy_audio_file_play(filename);
}

static int file_async_test_callback(void * param, nwy_PlayEvent_t status)
{
    if(NWY_PLAY_FINISH == status){
      nwy_ext_echo("\r\nfile play success\r\n");
      nwy_audio_file_play_stop();
    }
}

void nwy_play_file_async_test()
{
    char filename[16] = {0};
    char *sptr = nwy_ext_sio_recv_buff;
    nwy_ext_input_gets("\r\nPlease input filename: \r\n");

    strncpy(filename, sptr, strlen(sptr));
    nwy_ext_echo("\r\nfilename = %s \r\n", filename);

    nwy_audio_file_play_async(filename, file_async_test_callback);

    nwy_ext_echo("\r\nstart play done\r\n");

}

void nwy_audio_file_record_test()
{
    char filename[16] = {0};
    char* sptr = nwy_ext_sio_recv_buff;

    nwy_ext_input_gets("\r\nPlease input filename: \r\n");
    strncpy(filename, sptr, strlen(sptr));
    nwy_ext_echo("\r\nfilename = %s \r\n", filename);

    if(nwy_audio_file_record(filename,3) == NWY_ERROR)
    {
        nwy_ext_echo("record failed \r\n");
        nwy_audio_file_record_stop();
        return ;
    }
    nwy_sleep(1000 * 5);
    nwy_ext_echo("record success \r\n");
    nwy_audio_file_record_stop();
}

#ifdef FEATURE_NWY_OPEN_ZZD_SDK
typedef void(*POC_CALLBACK)(char *uf, int len);
void zzd_cb(char *uf, int len)
{
	nwy_ext_echo("\r\n%s", uf);
}
void nwy_zzd_test()
{
    char *ptr1 = "000000010101";
    oem_poc(ptr1, strlen(ptr1));
    nwy_ext_echo("\r\noem_poc %s success", ptr1);

    nwy_sleep(1000);

    char *ptr2 = "10000009";
    oem_poc(ptr1, strlen(ptr1));
    nwy_ext_echo("\r\noem_poc %s success", ptr2);
}
#endif

void nwy_audio_change_channel_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_audevOutput_t output_channel;
    nwy_audevInput_t input_channel;

    nwy_ext_input_gets("\r\nPlease input audio input channel: ");
    nwy_ext_echo("0: MAINMIC \r\n");
    nwy_ext_echo("1: AUXMIC \r\n");
    nwy_ext_echo("2: DUALMIC \r\n");
    nwy_ext_echo("3: HPMIC_L \r\n");
    nwy_ext_echo("4: HPMIC_R \r\n");
    switch(atoi(sptr))
    {
        case 0:
            input_channel = NWY_AUDEV_INPUT_MAINMIC;
            break;
        case 1:
            input_channel = NWY_AUDEV_INPUT_AUXMIC;
            break;
        case 2:
            input_channel = NWY_AUDEV_INPUT_DUALMIC;
            break;
        case 3:
            input_channel = NWY_AUDEV_INPUT_HPMIC_L;
            break;
        case 4:
            input_channel = NWY_AUDEV_INPUT_HPMIC_R;
            break;
        default:
            input_channel = NWY_AUDEV_INPUT_MAINMIC;
            break;
    }
    nwy_change_input_channel(input_channel);

    nwy_ext_input_gets("\r\nPlease input audio output channel: ");
    nwy_ext_echo("0: RECEIVER \r\n");
    nwy_ext_echo("1: HEADPHONE \r\n");
    nwy_ext_echo("2: SPEAKER \r\n");
    switch(atoi(sptr))
    {
        case 0:
            output_channel = NWY_AUDEV_OUTPUT_RECEIVER;
            break;
        case 1:
            output_channel = NWY_AUDEV_OUTPUT_HEADPHONE;
            break;
        case 2:
            output_channel = NWY_AUDEV_OUTPUT_SPEAKER;
            break;
        default:
            output_channel = NWY_AUDEV_OUTPUT_SPEAKER;
            break;
    }
    nwy_change_output_channel(output_channel);
}

#ifdef FEATURE_NWY_AUDIO_SET_HEADSETCB
void HeadsetCustCB(nwy_drvHeadsetNotifyMsg_t id, uint32_t param)
{
    switch (id)
    {

    case NWY_MSG_HEADSET_PLUGIN:
    {
        nwy_ext_echo("MSG_HEADSET_PLUGIN \r\n");
    }
    break;

    case NWY_MSG_HEADSET_PLUGOUT:
    {
        nwy_ext_echo("MSG_HEADSET_PLUGOUT \r\n");
    }
    break;

    case NWY_MSG_HEADSET_BTN_DOWN:
    {
        nwy_ext_echo("MSG_HEADSET_BTN_DOWN \r\n");
    }
    break;

    case NWY_MSG_HEADSET_BTN_UP:
    {
        nwy_ext_echo("MSG_HEADSET_BTN_UP \r\n");
    }
    break;

    }
}

void nwy_audio_set_headsetcb_test()
{
    nwy_ext_echo("nwy_audio_set_headsetcb_test \r\n");
    nwy_audio_set_headsetcb(HeadsetCustCB);
}
#endif
void nwy_get_send_ip_packet_proc(uint8_t *data, int len)
{
    OSI_LOGXI(OSI_LOGPAR_M, 0, "nwy_get_send_ip_packet_proc : %*s", len, data);
}

void nwy_get_recv_ip_packet_proc(uint8_t *data, int len)
{
    OSI_LOGXI(OSI_LOGPAR_M, 0, "nwy_recv_ip_packet_report : %*s", len, data);
}

void nwy_ip_packet_test()
{
    nwy_get_recv_ip_packet_reg(nwy_get_recv_ip_packet_proc);
    nwy_get_send_ip_packet_reg(nwy_get_send_ip_packet_proc);
}

void nwy_hp_type_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    nwy_hp_type_t cfg;
    nwy_ext_echo("\r\n 0: NWY_AUDIO_HP_NO \r\n");
    nwy_ext_echo("1: NWY_AUDIO_HP_NC \r\n");

    nwy_ext_input_gets("\r\nPlease input audio hp type: ");

    switch(atoi(sptr))
    {
        case 0:
            cfg = NWY_AUDIO_HP_NO;
            nwy_ext_echo("case 0 \r\n");
            nwy_audio_set_hp_type(cfg);
            break;
        case 1:
            cfg = NWY_AUDIO_HP_NC;
            nwy_ext_echo("case 1 \r\n");
            nwy_audio_set_hp_type(cfg);
            break;
        default:
            break;
    }

}

void nwy_socket_event_proc(int socketid, nwy_socket_event_cb event)
{
    OSI_LOGI(0, "nwy_socket_event_cb socketid = %d event = %d", socketid, event);
}

void nwy_auto_power_on_test(void)
{
    char* sptr = nwy_ext_sio_recv_buff;
    uint32_t enable, voltage, period;
    nwy_ext_echo("\r\n nwy auto power on test\r\n");

    nwy_pm_get_auto_power_on_cfg(&enable, &voltage, &period);
    nwy_ext_echo("\r\n saved cfg was enable=%d voltage=%d period=%d\r\n", enable, voltage, period);

    nwy_ext_input_gets("\r\nPlease input enable or disable:1/0");
    enable = atoi(sptr);
    nwy_ext_input_gets("\r\nPlease input power on voltage threshold: 2950~4050 mV");
    voltage = atoi(sptr);
    nwy_ext_input_gets("\r\nPlease input monitor period:5~24*3600 seconds");
    period = atoi(sptr);

    nwy_pm_set_auto_power_on_cfg(enable, voltage, period);
}

static void prvThreadEntry(void *param)
{
    char* sptr = nwy_ext_sio_recv_buff;
    int ret = 0;

    //nwy_usb_con_init();
#ifdef FEATURE_NWY_OPEN_ZZD_SDK
    nwy_register_poc_callback(zzd_cb);
#endif
    nwy_usb_serial_reg_recv_cb(nwy_ext_sio_data_cb);
    nwy_sdk_at_parameter_init();
    nwy_ext_init_unsol_reg();

    nwy_socket_event_report_reg(nwy_socket_event_proc);
    while (1)
    {
        nwy_ext_main_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");

        switch(atoi(sptr))
        {
            case 1:
                nwy_wifi_test();
                break;
            case 2:
                nwy_ext_virtual_at_test();
                break;
            case 3:
                nwy_sim_test();
                break;
            case 4:
                nwy_data_test();
                break;
            case 5:
                nwy_network_test_main();
                break;
			case 6:
                nwy_voice_test();
				break;
            /* added by wangchen for N58 sms api to test 20200215 begin */
            case 7:
                nwy_sms_test();
                break;
            /* added by wangchen for N58 sms api to test 20200215 end */
            case 8:
				nwy_location_test();
				break;
            case 9:
                nwy_fota_test();
                break;
            case 10:
                nwy_uart_test();
                break;
            case 11:
                nwy_i2c_test();
                break;
            case 12:
                nwy_spi_test();
                break;
            case 13:
                nwy_gpio_test();
                break;
            case 14:
                nwy_adc_test();
                break;
            case 15:
                nwy_pm_test();
                break;
            case 16:
                nwy_audio_test();
                break;
            case 17:
                nwy_led_backlight_test();
                break;
            case 18:
                nwy_keypad_test();
                break;
            case 19:
                nwy_device_test();
                break;
            case 20:
                nwy_file_test();
                break;
            case 21:
                nwy_tcp_test();
                break;
            case 22:
                nwy_udp_test();
                break;
            case 23:
#ifndef FEATURE_NWY_OPEN_LITE_TTS_OFF
                nwy_tts_test();
#endif
                break;
            case 24:
                nwy_dtmf_test();
                break;
            case 25:
                nwy_close_key_poweroff_test();
                break;
            case 26:
                nwy_set_at_forward_cb(1,"+FYTEST1", nwy_at_cmd_process_callback);
                break;
            case 27:
                {
                    nwy_time_t julian_time= {2020, 3,19,20,33,30};
                    char timezone =0;
                    nwy_set_time(&julian_time, timezone);
                    break;
                }
            case 28:
                {
                    nwy_time_t julian_time = {0};
                    char timezone =0;
                    nwy_get_time(&julian_time, &timezone);
                    nwy_ext_echo("\r\n%d-%d-%d %d:%d:%d", julian_time.year,julian_time.mon,julian_time.day, julian_time.hour,julian_time.min,julian_time.sec);
                    break;
                }
            case 29:
                {
                    nwy_lcd_test();
                    break;
                }
            case 30:
                nwy_timer_test();
                break;
            case 31:
                nwy_dm_test();
                break;
#ifdef FEATURE_NWY_ALI_MQTT
            case 32:
                nwy_alimqtt_test();
                break;
            case 33:
                nwy_mqtt_publish_test();
                break;
            case 34:
                nwy_mqtt_subscribe_test();
                break;
            case 35:
                nwy_mqtt_state_test();
                break;
            case 36:
                nwy_mqtt_disconn_test();
                break;
            case 37:
                nwy_mqtt_unsub_test();
                break;
#endif
            case 38:
                nwy_appimg_fota_test();
                break;
            case 39:
                nwy_semaphore_test();
                break;
            case 40:
                nwy_test_send_api();
                break;
            case 41:
                nwy_ftp_test();
                break;
            case 42:
                nwy_http_test();
                break;
            case 43:
                nwy_pm_auto_test();
                break;
            case 44:
                nwy_sdcart_test();
                break;
            case 45:
                nwy_read_sdcard_test();
                break;
            case 46:
                nwy_set_ble_test();
                break;
            case 47:
                nwy_get_chip_id_test();
                break;
            case 48:
                nwy_flash_test();
                break;
            case 49:
                nwy_multiplex_http_test();
                break;
            case 50:
                nwy_multiplex_ftp_test();
                break;
            case 51:
                nwy_get_ver_test();
                break;
			case 52:
                nwy_boot_causes_test();
				break;
#ifdef FEATURE_NWY_PAHO_MQTT
            case 53:
                nwy_paho_mqtt_test();
                break;
#endif
            case 54:
                nwy_rs485_test();
                break;
            /* added by wangchen for N58 ussd api to test 20200826 begin */
            case 55:
                nwy_ussd_test();
                break;
            /* added by wangchen for N58 ussd api to test 20200826 end */
            case 56:
                nwy_pwm_test();
                break;
            case 57:
                nwy_time_zone_test();
                break;
            case 58:
                nwy_audio_file_play_test();
                break;
#ifdef FEATURE_NWY_OPEN_ZZD_SDK
            case 59:
                nwy_zzd_test();
                break;
#endif
            case 60:
                nwy_audio_change_channel_test();
                break;
            case 61:
                nwy_spi_flash_mount_test();
                break;
#ifdef FEATURE_NWY_OPEN_LITE_AZURE
            case 62:
                nwy_azure_test();
                break;
#endif
            case 63:
                nwy_cert_test();
                break;
#ifdef FEATURE_NWY_AUDIO_SET_HEADSETCB
           case 64:
               nwy_audio_set_headsetcb_test();
               break;
#endif
            case 65:
                nwy_ip_packet_test();
                break;
            case 66:
                nwy_multi_socket_test();
                break;
#ifdef FEATURE_NWY_OPEN_LITE_LWM2M
            case 67:
                nwy_lwm2m_test();
                break;
#endif
            case 68:
                nwy_hp_type_test();
                break;
            case 69:
                nwy_audio_file_record_test();
                break;
            case 70:
                ret = nwy_cmdfunc_nftrfit();
                  nwy_ext_echo("\r\n nwy_cmdfunc_nftrfit ret = %d\r\n", ret);
                ret = nwy_cmdfunc_nftrcal();
                  nwy_ext_echo("\r\n nwy_cmdfunc_nftrcal ret = %d\r\n", ret);
                  break;
            case 71:
                nwy_ext_virtual_send_hex();
                break;
            case 72:
                nwy_set_poc_sampleRate_test();
                break;
            case 73:
#ifdef FEATURE_NWY_OPEN_LITE_AWS_MQTT
                nwy_aws_mqtt_test();
#endif
                break;
            case 74:
                if(nwy_get_poweroff_charge_mode()==false)
                {
                    nwy_set_poweroff_charge_mode(true);
                    nwy_power_off(2);//reset modem to enable nvitem...
                }
                break;
			#ifdef FEATURE_NWY_SECURITY_CHECK
            case 75:
                nwy_rhino_ids_test();
                break;
			#endif
            case 76:
                nwy_poc_dsds_test();
                break;
            case 77:
                nwy_play_file_async_test();
                break;
#ifdef FEATURE_NWY_STK_COMMAND
            case 78:
                nwy_stk_test();
                break;
#endif
            case 79:
                nwy_camera_test();
                break;
#ifdef FEATURE_NEOWAY_PIPECLOUD
            case 80:
                nwy_pipecloud_test();
                break;
#endif
            case 81:
                nwy_auto_power_on_test();
                break;
            default:
                break;
        }
    }
    nwy_exit_thread();
}


//warning:app version length <= 64,
static const char APP_VERSION[65] = "NWY_APP_V1.0.1";

int appimg_enter(void *param)
{
    //warning:app build time length <= 64
    const char APP_BUILD_TIME[65]= {0};
    char version[70]={0};

    sprintf(version,"\"%s\"", APP_VERSION);
    sprintf(APP_BUILD_TIME,"\"%s,%s\"", __DATE__,__TIME__);
    OSI_LOGI(0, "appimg_enter ...");
    if(false == nwy_app_version(version, APP_BUILD_TIME))
    {
      OSI_LOGI(0, "app set version fail");
      return 0;
    }

#if 0
    {
        extern void nwy_poc_demo(void);
        nwy_poc_demo();
    }
#endif

    g_app_thread = nwy_create_thread("mythread", prvThreadEntry, NULL, NWY_OSI_PRIORITY_NORMAL, 1024*10, 16);
    g_timer = nwy_timer_init(g_app_thread, nwy_app_timer_cb, NULL);

    return 0;
}

void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}

#ifdef  FEATURE_NWY_ALI_MQTT
static void *pclient=NULL;

#define MQTT_PRODUCT_KEY            "a1cPP8Xe4Ax"
#define MQTT_DEVICE_NAME            "TEST_T8"
#define MQTT_DEVICE_SECRET          "dP1UMS0gKlNsvBrPvEfiChBmpYnz2lNI"
#define MSG_LEN_MAX                 (1024)

static int mqtt_sub_flag = -1;
static int mqtt_pub_flag = -1;
static int mqtt_unsub_flag = -1;
typedef struct
{
    char topic_name[128];
    iotx_mqtt_event_handle_func_fpt topic_handle_func;
}topic_list_t;
static topic_list_t topic_list[5] = {0};

static void message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    nwy_ext_echo("topic_len:%d, Topic:%s, payload_len:%d, Payload:%s\r\n",
                ptopic_info->topic_len,ptopic_info->ptopic,
                ptopic_info->payload_len,ptopic_info->payload);
}

static void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            nwy_ext_echo("undefined event occur.\r\n");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            nwy_ext_echo("MQTT disconnect.\r\n");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            nwy_ext_echo("MQTT reconnect.\r\n");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            mqtt_sub_flag = 1;
            nwy_ext_echo("subscribe success, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            mqtt_sub_flag = 0;
            nwy_ext_echo("subscribe wait ack timeout, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            mqtt_sub_flag = 0;
            nwy_ext_echo("subscribe nack, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            mqtt_unsub_flag = 1;
            nwy_ext_echo("unsubscribe success, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            mqtt_unsub_flag = 0;
            nwy_ext_echo("unsubscribe timeout, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            mqtt_unsub_flag = 0;
            nwy_ext_echo("unsubscribe nack, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            mqtt_pub_flag = 1;
            nwy_ext_echo("publish success, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            mqtt_pub_flag = 0;
            nwy_ext_echo("publish timeout, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            mqtt_pub_flag = 0;
            nwy_ext_echo("publish nack, packet-id=%u\r\n", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            nwy_ext_echo( "topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s\r\n",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        default:
            nwy_ext_echo("Should NOT arrive here.\r\n");
            break;
    }
}

nwy_osiThread_t * task_id = NULL;

void nwy_ali_cycle(void)
{
  int ret = 0;
  char rspstr[60] = {0};
  for(;;)
  {
    while(ret == 0)
    {
        ret = IOT_MQTT_Yield(pclient, 300);
        nwy_sleep(100);
    	if(ret != 0)
    	{
            IOT_MQTT_Destroy(&pclient);
            break;
    	}
    }
    nwy_suspend_thread(task_id);
  }
}


nwy_osiThread_t * nwy_at_ali_yeild_task_init(void)
{
  if (task_id == NULL)
  {
     task_id = nwy_create_thread("neo_ali_yeild_task",nwy_ali_cycle,
                   NULL, NWY_OSI_PRIORITY_NORMAL,1024*15,4);
  }
  else
  {
    nwy_resume_thread(task_id);
  }

  nwy_ext_echo(" CreateThread neo_ali_yeild_task\n");
  return task_id;
}

int nwy_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;
    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);
    return rc;
}


int iotx_midreport_topic(char *topic_name, char *topic_head, char *product_key, char *device_name)
{
    int ret;
    /* reported topic name: "/sys/${productKey}/${deviceName}/thing/status/update" */
    int len = strlen(product_key) + strlen(device_name) + 128;
    ret = nwy_Snprintf(topic_name,
                       len,
                       "%s/sys/%s/%s/thing/status/update",
                       topic_head,
                       product_key,
                       device_name);
    return ret;
}

int check_topic_exit(char *topic_name)
{
    int i = 0;
    for (i = 0; i < 5; i++)
    {
        if(strlen(topic_list[i].topic_name) && strstr(topic_list[i].topic_name, topic_name))
            return 1;
    }
    return 0;
}
int update_topic_list(char *topic_name, int type)
{
    int i = 0;
    for (i = 0; i < 5; i++)
    {
        if(type == 1)
        {
            if(strlen(topic_list[i].topic_name) == 0)
            {
                strncpy(topic_list[i].topic_name, topic_name, strlen(topic_name));
                return 1;
            }
        }
        else
        {
            if(strstr(topic_list[i].topic_name, topic_name))
            {
                memset(topic_list[i].topic_name, 0, sizeof(topic_list[i].topic_name));
                return 1;
            }
        }
    }
    nwy_ext_echo("update failed\r\n");
    return 0;
}

void nwy_alimqtt_test()
{
    //static char msg_readbuf[MSG_LEN_MAX]  = {0};
    //static char msg_writebuf[MSG_LEN_MAX]  = {0};
    int domain_type = IOTX_CLOUD_DOMAIN_SH;
    char topic[128] = {0};
    static iotx_conn_info_pt pconn_info = NULL;
    static iotx_mqtt_param_t mqtt_params;
    nwy_osiThread_t *id = NULL;
    unsigned keepalive = 120000;
    unsigned int clean = 0;
    unsigned int timeout = 30000;
    int ret = 0;
    int dynamic_register = 0;

    iotx_mqtt_topic_info_t      topic_info;

    char msg[MSG_LEN_MAX] = {0};
    nwy_ext_echo("alimqtt test\r\n");
    //IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, &dynamic_register);
    IOT_Ioctl(IOTX_IOCTL_SET_REGION, (void *)&domain_type);
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, (void *)MQTT_PRODUCT_KEY);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, (void *)MQTT_DEVICE_NAME);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_SECRET, (void*)MQTT_DEVICE_SECRET);
    ret = IOT_SetupConnInfo(MQTT_PRODUCT_KEY, MQTT_DEVICE_NAME, MQTT_DEVICE_SECRET, (void**)&pconn_info);
    if (ret == 0)
    {
        if ((NULL == pconn_info) || (0 == pconn_info->client_id))
        {
            nwy_ext_echo("please auth fail\n");
            return;
        }
        nwy_ext_echo("IOT_SetupConnInfo SUCCESS\r\n");
        memset(&mqtt_params, 0x0, sizeof(mqtt_params));
        mqtt_params.port = pconn_info->port;
        mqtt_params.host = pconn_info->host_name;
        mqtt_params.client_id = pconn_info->client_id;
        mqtt_params.username = pconn_info->username;
        mqtt_params.password = pconn_info->password;
        mqtt_params.pub_key = pconn_info->pub_key;

        mqtt_params.request_timeout_ms = timeout;
        mqtt_params.clean_session = clean;
        mqtt_params.keepalive_interval_ms = keepalive;
        //mqtt_params.pread_buf = msg_readbuf;
        mqtt_params.read_buf_size = MSG_LEN_MAX;
        //mqtt_params.pwrite_buf = msg_writebuf;
        mqtt_params.write_buf_size = MSG_LEN_MAX;

        mqtt_params.handle_event.h_fp = event_handle;
        mqtt_params.handle_event.pcontext = NULL;
        ret = IOT_MQTT_CheckStateNormal(pclient);
        if(1 == ret)
        {
            nwy_ext_echo( "MQTT is connected\n");
            return;
        }
        pclient = IOT_MQTT_Construct(&mqtt_params);
        if (NULL == pclient)
            nwy_ext_echo("MQTT construct failed\n");
        else
        {
             nwy_ext_echo( "MQTT construct success\n");
             id = nwy_at_ali_yeild_task_init();
             if(NULL == id)
             {

                 nwy_ext_echo("MQTT construct failed\n");
                 return;
             }
        }
        memset(topic_list, 0, sizeof(topic_list));
        iotx_midreport_topic(topic, "", MQTT_PRODUCT_KEY, MQTT_DEVICE_NAME);
        nwy_ext_echo("topic = %s\r\n",topic);
        ret = IOT_MQTT_Subscribe(pclient, topic, IOTX_MQTT_QOS1, message_arrive, NULL);
        if(ret < 0)
        {
            nwy_ext_echo("IOT_MQTT_Subscribe error");
            return ;
        }

        while(1)
        {
            if(1 == mqtt_sub_flag)
            {
                nwy_ext_echo("IOT_MQTT_Subscribe OK\r\n");
                update_topic_list(topic, 1);
                break;
            }
            else if(0 == mqtt_sub_flag)
            {
                nwy_ext_echo("IOT_MQTT_Subscribe error\r\n");
                break;
            }
            nwy_sleep(100);
        }
        /*
        iotx_midreport_reqid(requestId,
                         dev->product_key,
                         dev->device_name)
        iotx_midreport_payload(msg, )/*/
        strcpy(msg, "{hello word}");
        topic_info.qos = IOTX_MQTT_QOS1;
        topic_info.payload = (void *)msg;
        topic_info.payload_len = strlen(msg);
        topic_info.retain = 0;
        topic_info.dup = 0;
        ret = IOT_MQTT_Publish(pclient, topic, &topic_info);
        if(ret < 0)
        {
            nwy_ext_echo("IOT_MQTT_Publish error");
            return;
        }

        while(1)
        {
            if(1 == mqtt_pub_flag)
            {
                nwy_ext_echo("IOT_MQTT_Publish OK\r\n");
                break;
            }
            else if(0 == mqtt_pub_flag)
            {
                nwy_ext_echo("IOT_MQTT_Publish error\r\n");
                break;
            }
            nwy_sleep(100);
        }
    }
    else
    {
        nwy_ext_echo("MQTT auth failed\n");
    }

}


void nwy_mqtt_publish_test()
{
    char msg[MSG_LEN_MAX] = {0};
    int ret = 0;
    iotx_mqtt_topic_info_t      topic_info;
    char topic[128] = {0};
    iotx_midreport_topic(topic, "", MQTT_PRODUCT_KEY, MQTT_DEVICE_NAME);
    strcpy(msg, "{\"id\":\"12\",\"params\":{\"temperature\":18,\"data\":\"2020/04/03\"}}");
    topic_info.qos = IOTX_MQTT_QOS1;
    topic_info.payload = (void *)msg;
    topic_info.payload_len = strlen(msg);
    topic_info.retain = 0;
    topic_info.dup = 0;
    ret = IOT_MQTT_Publish(pclient, topic, &topic_info);
    if(ret < 0)
    {
        nwy_ext_echo("IOT_MQTT_Publish error");
        return;
    }
    mqtt_pub_flag = -1;
    while(1)
    {
        if(1 == mqtt_pub_flag)
        {
            nwy_ext_echo("IOT_MQTT_Publish OK\r\n");
            break;
        }
        else if(0 == mqtt_pub_flag)
        {
            nwy_ext_echo("IOT_MQTT_Publish error\r\n");
            break;
        }
        nwy_sleep(100);
    }
}


void nwy_mqtt_subscribe_test()
{
     char topic[128] = {0};
     int ret = 0;
     iotx_midreport_topic(topic, "", MQTT_PRODUCT_KEY, MQTT_DEVICE_NAME);
     nwy_ext_echo("topic = %s\r\n",topic);
     //whether topic has been subcribed
     if(check_topic_exit(topic))
     {
        nwy_ext_echo("topic has been subscibe\r\n");
        return ;
     }
     ret = IOT_MQTT_Subscribe(pclient, topic, IOTX_MQTT_QOS1, message_arrive, NULL);
     if(ret < 0)
     {
         nwy_ext_echo("IOT_MQTT_Subscribe error");
         return;
     }
     mqtt_sub_flag = -1;
     while(1)
     {
         if(1 == mqtt_sub_flag)
         {
             nwy_ext_echo("IOT_MQTT_Subscribe OK\r\n");
             update_topic_list(topic, 1);
             break;
         }
         else if(0 == mqtt_sub_flag)
         {
             nwy_ext_echo("IOT_MQTT_Subscribe error\r\n");
             break;
         }
         nwy_sleep(100);
     }
}

void nwy_mqtt_unsub_test()
{
    int rc = 0;
    char topic[128] = {0};

    iotx_midreport_topic(topic, "", MQTT_PRODUCT_KEY, MQTT_DEVICE_NAME);
    rc = IOT_MQTT_Unsubscribe(pclient, topic);
    if(rc < 0)
    {
        nwy_ext_echo("IOT_MQTT_Unsubscribe error\r\n");
        return;
    }
    mqtt_unsub_flag = -1;
    while(1)
    {
        if(1 == mqtt_unsub_flag)
         {
             nwy_ext_echo("IOT_MQTT_Unsubscribe OK\r\n");
             update_topic_list(topic, 0);
             break;
         }
         else if(0 == mqtt_unsub_flag)
         {
             nwy_ext_echo("IOT_MQTT_Unsubscribe error\r\n");
             break;
         }
         nwy_sleep(100);
    }
}

void nwy_mqtt_state_test()
{
    int ret = 0;
    ret = IOT_MQTT_CheckStateNormal(pclient);
    nwy_ext_echo("MQTT state is %d\r\n", ret);
}



void nwy_mqtt_disconn_test()
{
    int ret = 0;
    ret = IOT_MQTT_CheckStateNormal(pclient);
    if(ret == 0)
        nwy_ext_echo("MQTT is disconnected \r\n");
    else
    {
  	    IOT_MQTT_Destroy(&pclient);
        pclient = NULL;
        nwy_ext_echo("MQTT is Destroy \r\n");
    }
}

/*add by neoway end*/
#endif





