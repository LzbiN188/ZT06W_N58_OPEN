#include "app_task.h"
#include "app_port.h"
#include "app_sys.h"
#include "app_gps.h"
#include "app_atcmd.h"
#include "app_net.h"
#include "app_param.h"
#include "app_protocol.h"
#include "nwy_vir_at.h"
#include "nwy_sim.h"
#include "nwy_voice.h"
#include "nwy_wifi.h"
#include "app_instructioncmd.h"
#include "nwy_sms.h"
#include "app_port.h"
#include "nwy_pm.h"
#include "nwy_ble.h"
#include "app_mir3da.h"
#include "app_protocol.h"
#include "app_ble.h"
#include "app_protocol_808.h"
#include "nwy_loc.h"
#include "app_gps.h"
#include "nwy_audio_api.h"
#include "app_customercmd.h"
#include "nwy_usb_serial.h"
#include "app_socket.h"
#include "app_mcu.h"
#include "app_kernal.h"

/*-------------------------------------------------------------------------------------*/
//显示时间
void showSystemTimeTask(void)
{
    uint16_t year = 0;
    uint8_t  month = 0, date = 0, hour = 0, minute = 0, second = 0;
    if (sysinfo.taskSuspend)
        return;
    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    LogPrintf(DEBUG_ALL, "<%d-%d-%2d %02d:%02d:%02d>\r\n", year, month, date, hour, minute, second);
}
/*-------------------------------------------------------------------------------------*/
/*LED 控制相关功能*/
static SystemLEDInfo sysledinfo;
static void gpsLedTask(void)
{
    static uint8_t tick = 0;
    if (sysinfo.System_Tick >= 300)
    {
        if (sysparam.ledctrl == 0 || sysinfo.GPSRequest == 0)
        {
            GPSLEDOFF;
            return ;
        }
    }
    if (sysledinfo.sys_gps_led_on_time == 0)
    {
        GPSLEDOFF;
        return ;
    }
    else if (sysledinfo.sys_gps_led_off_time == 0)
    {
        GPSLEDON;
        return ;
    }
    tick++;
    //on status
    if (sysledinfo.sys_gps_led_onoff == 1)
    {
        GPSLEDON;
        if (tick >= sysledinfo.sys_gps_led_on_time)
        {
            tick = 0;
            sysledinfo.sys_gps_led_onoff = 0;
        }
    }
    //off status
    else
    {
        GPSLEDOFF;
        if (tick >= sysledinfo.sys_gps_led_off_time)
        {
            tick = 0;
            sysledinfo.sys_gps_led_onoff = 1;
        }
    }
}

static void signalLedTask(void)
{
    static uint8_t tick = 0;
    if (sysinfo.System_Tick >= 300)
    {
        if (sysparam.ledctrl == 0 || sysinfo.GPSRequest == 0)
        {
            SIGNALLEDOFF;
            return ;
        }
    }
    //如果开时长为0,那么LED 常灭
    if (sysledinfo.sys_led_on_time == 0)
    {
        SIGNALLEDOFF;
        return ;
    }
    //如果关时长为0,那么LED 常亮
    else if (sysledinfo.sys_led_off_time == 0)
    {
        SIGNALLEDON;
        return ;
    }
    tick++;
    if (sysledinfo.sys_led_onoff == 1) //on status
    {
        SIGNALLEDON;
        if (tick >= sysledinfo.sys_led_on_time)
        {
            tick = 0;
            sysledinfo.sys_led_onoff = 0;
        }
    }
    else//off status
    {
        SIGNALLEDOFF;
        if (tick >= sysledinfo.sys_led_off_time)
        {
            tick = 0;
            sysledinfo.sys_led_onoff = 1;
        }
    }
}
static void serverLedTask(void)
{

    static uint8_t tick = 0;

    if (sysinfo.System_Tick >= 300)
    {
        if (sysparam.ledctrl == 0 || sysinfo.GPSRequest == 0)
        {
            SERVERLEDOFF;
            return ;
        }
    }
    if (sysledinfo.sys_server_led_on_time == 0)
    {
        SERVERLEDOFF;
        return ;
    }
    else if (sysledinfo.sys_server_led_off_time == 0)
    {
        SERVERLEDON;
        return ;
    }
    tick++;
    //on status
    if (sysledinfo.sys_server_led_onoff == 1)
    {
        SERVERLEDON;
        if (tick >= sysledinfo.sys_server_led_on_time)
        {
            tick = 0;
            sysledinfo.sys_server_led_onoff = 0;
        }
    }
    //off status
    else
    {
        SERVERLEDOFF;
        if (tick >= sysledinfo.sys_server_led_off_time)
        {
            tick = 0;
            sysledinfo.sys_server_led_onoff = 1;
        }
    }
}
static void ledSetPeriod(uint8_t ledtype, uint8_t on_time, uint8_t off_time)
{
    if (ledtype == NETREGLED)
    {
        //系统信号灯
        sysledinfo.sys_led_on_time = on_time;
        sysledinfo.sys_led_off_time = off_time;
    }
    else if (ledtype == GPSLED)
    {
        //系统gps灯
        sysledinfo.sys_gps_led_on_time = on_time;
        sysledinfo.sys_gps_led_off_time = off_time;
    }

    else if (ledtype == SERVERLED)
    {
        //系统网络灯
        sysledinfo.sys_server_led_on_time = on_time;
        sysledinfo.sys_server_led_off_time = off_time;
    }
}
void updateSystemLedStatus(uint8_t status, uint8_t onoff)
{
    if (onoff == 1)
    {
        sysinfo.systemledstatus |= status;
    }
    else
    {
        sysinfo.systemledstatus &= ~status;
    }
    if ((sysinfo.systemledstatus & SYSTEM_LED_RUN) == SYSTEM_LED_RUN)
    {
        //慢闪
        ledSetPeriod(NETREGLED, 10, 10);
        ledSetPeriod(GPSLED, 10, 10);
        //ledSetPeriod(SERVERLED, 10, 10);
        //已注册在网
        if ((sysinfo.systemledstatus & SYSTEM_LED_NETREGOK) == SYSTEM_LED_NETREGOK)
        {
            ledSetPeriod(NETREGLED, 1, 9);
            //已连接平台
            if ((sysinfo.systemledstatus & SYSTEM_LED_SERVEROK) == SYSTEM_LED_SERVEROK)
            {
                //常亮
                //ledSetPeriod(SERVERLED, 1, 0);
                ledSetPeriod(NETREGLED, 1, 0);
            }
        }
        //GPS
        if ((sysinfo.systemledstatus & SYSTEM_LED_GPSOK) == SYSTEM_LED_GPSOK)
        {
            //常亮
            ledSetPeriod(GPSLED, 1, 0);

        }

        if ((sysinfo.systemledstatus & SYSTEM_LED_UPDATE) == SYSTEM_LED_UPDATE)
        {
            //常亮
            ledSetPeriod(GPSLED, 1, 1);
        }

    }
    else
    {
        SIGNALLEDOFF;
        GPSLEDOFF;
        SERVERLEDOFF;
        ledSetPeriod(NETREGLED, 0, 1);
        ledSetPeriod(GPSLED, 0, 1);
        ledSetPeriod(SERVERLED, 0, 1);
    }
}


void ledRunTask(void *param)
{
    gpsLedTask();
    signalLedTask();
    serverLedTask();
}
/*-------------------------------------------------------------------------------------*/
/*报警 事件控制*/
void alarmRequestSet(uint16_t request)
{
    sysinfo.alarmrequest |= request;
}
void alarmRequestClear(uint16_t request)
{
    sysinfo.alarmrequest &= ~request;
}
void alarmUploadRequest(void)
{
    uint8_t alarm;
    if (sysinfo.alarmrequest == 0)
    {
        return ;
    }
    if (isProtocolReday() == 0)
    {
        return ;
    }
    //感光报警
    if (sysinfo.alarmrequest & ALARM_LIGHT_REQUEST)
    {
        alarmRequestClear(ALARM_LIGHT_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Light Alarm\n");
        terminalAlarmSet(TERMINAL_WARNNING_LIGHT);
        alarm = 0;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }

    //低电报警
    if (sysinfo.alarmrequest & ALARM_LOWV_REQUEST)
    {
        alarmRequestClear(ALARM_LOWV_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>LowVoltage Alarm\n");
        terminalAlarmSet(TERMINAL_WARNNING_LOWV);
        alarm = 0;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }

    //断电报警
    if (sysinfo.alarmrequest & ALARM_LOSTV_REQUEST)
    {
        alarmRequestClear(ALARM_LOSTV_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>lostVoltage Alarm\n");
        terminalAlarmSet(TERMINAL_WARNNING_LOSTV);
        alarm = 0;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }

    //急加速报警
    if (sysinfo.alarmrequest & ALARM_ACCLERATE_REQUEST)
    {
        alarmRequestClear(ALARM_ACCLERATE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Rapid Accleration Alarm\n");
        alarm = 9;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }
    //急减速报警
    if (sysinfo.alarmrequest & ALARM_DECELERATE_REQUEST)
    {
        alarmRequestClear(ALARM_DECELERATE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Rapid Deceleration Alarm\n");
        alarm = 10;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }

    //急左转报警
    if (sysinfo.alarmrequest & ALARM_RAPIDLEFT_REQUEST)
    {
        alarmRequestClear(ALARM_RAPIDLEFT_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Rapid LEFT Alarm\n");
        alarm = 11;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }

    //守卫报警
    if (sysinfo.alarmrequest & ALARM_GUARD_REQUEST)
    {
        alarmRequestClear(ALARM_GUARD_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Guard Alarm\n");
        alarm = 1;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }

    //蓝牙报警
    if (sysinfo.alarmrequest & ALARM_BLE_REQUEST)
    {
        alarmRequestClear(ALARM_BLE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE Alarm\n");
        alarm = 20;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, &alarm);
    }
}
/*-------------------------------------------------------------------------------------*/
/*GPS请求*/
void gpsRequestSet(uint32_t flag)
{
    char debug[40];
    sprintf(debug, "gpsRequestSet==>0x%04lX\n", flag);
    LogMessage(DEBUG_ALL, debug);
    sysinfo.GPSRequest |= flag;
}
void gpsRequestClear(uint32_t flag)
{
    char debug[40];
    sprintf(debug, "gpsRequestClear==>0x%04lX\n", flag);
    LogMessage(DEBUG_ALL, debug);
    sysinfo.GPSRequest &= ~flag;
}
uint8_t gpsRequestGet(uint32_t flag)
{
    return sysinfo.GPSRequest & flag;
}
/*-------------------------------------------------------------------------------------*/

static void gpsChangeFsmState(uint8_t state)
{
    sysinfo.GPSRequestFsm = state;
}
static void modeRequeUpdateRTC(void)
{
    if (sysparam.MODE == MODE1 || sysparam.MODE == MODE2 || sysparam.MODE == MODE21 || sysparam.MODE == MODE5)
    {
        updateRTCtimeRequest();
    }
    else if (sysparam.MODE == MODE23)
    {
        if (getTerminalAccState())
        {
            updateRTCtimeRequest();
        }
    }
}

void gpsSetPositionMode(void)
{
    int ret;
    ret = nwy_loc_set_position_mode(3);
    LogPrintf(DEBUG_ALL, "gpsSetPositionMode==>ret:%d\r\n", ret);
}

void gpsRequestOpen(void)
{
    LogMessage(DEBUG_ALL, "gpsRequestTask==>open GPS\n");
    GPSRSTHIGH;
    GPSLNAON;
    GPSPOWERON;
    sysinfo.gpsUpdatetick = sysinfo.System_Tick;
    sysinfo.GPSStatus = 1;
    gpsChangeFsmState(GPSOPENSTATUS);
    updateSystemLedStatus(SYSTEM_LED_GPSOK, 0);
    nwy_loc_close_uart_nmea_data();
    nwy_loc_start_navigation();
    startTimer(50, gpsSetPositionMode, 0);
}


void gpsRequestClose(void)
{
    LogMessage(DEBUG_ALL, "gpsRequestTask==>close GPS\n");
    GPSRSTLOW;
    GPSLNAOFF;
    GPSPOWEROFF;
    sysinfo.GPSStatus = 0;
    gpsChangeFsmState(GPSCLOSESTATUS);
    updateSystemLedStatus(SYSTEM_LED_GPSOK, 0);
    gpsClearCurrentGPSInfo();
    nwy_loc_set_startup_mode(0);
    nwy_loc_stop_navigation();

}

void gpsGetNmea(void)
{
    int ret, size;
    char nmea[2048];
    if (sysinfo.gpsOutPut == 0)
    {
        return ;
    }
    memset(nmea, 0, 2048);
    ret = nwy_loc_get_nmea_data(nmea);
    if (ret)
    {
        size = strlen(nmea);
        //        customerLogPrintf("+CGNSS: %d,", size);
        //        customerLogOutWl(nmea, size);
        nmeaParse((uint8_t *)nmea, size);
        customerGpsOutput();

    }
}

/*GPS开启控制*/
void gpsRequestTask(void)
{
    static uint8_t waittick = 0;
    int ret;
    if (sysinfo.System_Tick < 10)
        return;

    switch (sysinfo.GPSRequestFsm)
    {
        case GPSCLOSESTATUS:
            if (sysinfo.GPSRequest != 0)
            {
                modeRequeUpdateRTC();
                if (sysparam.agpsServer[0] != 0 && sysparam.agpsPort != 0 && (sysinfo.gpsLastFixedTick == 0 ||
                        sysinfo.System_Tick - sysinfo.gpsLastFixedTick >= 7200))
                {
                    ret = nwy_loc_set_server((char *)sysparam.agpsServer, sysparam.agpsPort, (char *)sysparam.agpsUser,
                                             (char *) sysparam.agpsPswd);
                    if (ret)
                    {
                        LogMessage(DEBUG_ALL, "set agps server ok\r\n");
                    }
                    else
                    {
                        LogMessage(DEBUG_ALL, "set agps server fail\r\n");
                    }
                    nwy_loc_agps_open(1);
                    if (ret)
                    {
                        LogMessage(DEBUG_ALL, "open agps ok\r\n");
                    }
                    else
                    {
                        LogMessage(DEBUG_ALL, "open agps fail\r\n");
                    }
                    waittick = 0;
                    gpsChangeFsmState(GPSOPENWAITSTATUS);
                }
                else
                {
                    gpsRequestOpen();
                }
            }
            break;
        case GPSOPENWAITSTATUS:
            if (waittick++ >= 3)
            {
                gpsRequestOpen();
            }
            break;
        case GPSOPENSTATUS:
            gpsGetNmea();
            if (sysinfo.GPSRequest == 0)
            {
                gpsRequestClose();
            }
            break;
        default:
            gpsChangeFsmState(GPSCLOSESTATUS);
            break;

    }
}

/*-------------------------------------------------------------------------------------*/

/*gps输出异常检测*/
static void gpsOutputCheckTask(void)
{
    if (sysinfo.GPSStatus == 0)
        return;
    if (sysinfo.System_Tick - sysinfo.gpsUpdatetick >= 20)
    {
        sysinfo.gpsUpdatetick = sysinfo.System_Tick;
        LogMessage(DEBUG_ALL, "No nmea output\n");
        gpsRequestClose();
    }
}

/*-------------------------------------------------------------------------------------*/
/*GPS 上送一个点*/
void gpsUplodOnePointTask(void)
{
    GPSINFO *gpsinfo;
    static uint16_t runtick = 0;
    static uint8_t	uploadtick = 0;
    //判断是否有请求该事件
    if (gpsRequestGet(GPS_REQUEST_UPLOAD_ONE) == 0)
    {
        runtick = 0;
        uploadtick = 0;
        return ;
    }
    gpsinfo = getCurrentGPSInfo();
    runtick++;
    if (runtick >= sysinfo.gpsuploadonepositiontime)
    {
        runtick = 0;
        uploadtick = 0;
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, getCurrentGPSInfo());
        gpsRequestClear(GPS_REQUEST_UPLOAD_ONE);
    }
    else
    {
        if (isProtocolReday() && gpsinfo->fixstatus)
        {
            if (++uploadtick >= 10)
            {
                uploadtick = 0;
                if (sysinfo.flag123)
                {
                    dorequestSend123();
                }
                sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, getCurrentGPSInfo());
                gpsRequestClear(GPS_REQUEST_UPLOAD_ONE);
            }
        }
    }


}

/*-------------------------------------------------------------------------------------*/
/*基站上报*/
void lbsUploadRequestTask(void)
{
    int lac, cid;
    if (sysinfo.lbsrequest == 0)
    {
        return ;
    }
    if (isProtocolReday() == 0)
    {
        return ;
    }
    sysinfo.lbsrequest = 0;
    lac = 0;
    cid = 0;
    nwy_sim_get_lacid(&lac, &cid);
    sysinfo.lac = lac;
    sysinfo.cid = cid;
    LogPrintf(DEBUG_ALL, "LAC:0x%X,CID:0x%X\r\n", sysinfo.lac, sysinfo.cid);
    sendProtocolToServer(NORMAL_LINK, PROTOCOL_19, NULL);
}

/*-------------------------------------------------------------------------------------*/
/*WIFI上报*/
static void startWifiScan(void)
{
    int i = 0;
    WIFI_INFO wifi_info;
    nwy_wifi_scan_list_t scan_list;
    GPSINFO *gpsinfo;
    gpsinfo = getCurrentGPSInfo();

    if (gpsinfo->fixstatus == 1 || sysinfo.GPSStatus == 0)
    {
        LogMessage(DEBUG_ALL, "stop wifi scan\r\n");
        return ;
    }

    memset(&scan_list, 0, sizeof(scan_list));
    LogMessage(DEBUG_ALL, "Start wifi scan\r\n");
    nwy_wifi_scan(&scan_list);
    for (i = 0; i < scan_list.num; i++)
    {
        LogPrintf(DEBUG_ALL, "AP MAC:%02x:%02x:%02x:%02x:%02x:%02x",
                  scan_list.ap_list[i].mac[5], scan_list.ap_list[i].mac[4], scan_list.ap_list[i].mac[3], scan_list.ap_list[i].mac[2],
                  scan_list.ap_list[i].mac[1], scan_list.ap_list[i].mac[0]);
        LogPrintf(DEBUG_ALL, ",channel = %d", scan_list.ap_list[i].channel);
        LogPrintf(DEBUG_ALL, ",rssi = %d\r\n", scan_list.ap_list[i].rssival);

        wifi_info.ap[i].ssid[0] = scan_list.ap_list[i].mac[5];
        wifi_info.ap[i].ssid[1] = scan_list.ap_list[i].mac[4];
        wifi_info.ap[i].ssid[2] = scan_list.ap_list[i].mac[3];
        wifi_info.ap[i].ssid[3] = scan_list.ap_list[i].mac[2];
        wifi_info.ap[i].ssid[4] = scan_list.ap_list[i].mac[1];
        wifi_info.ap[i].ssid[5] = scan_list.ap_list[i].mac[0];
    }
    wifi_info.apcount = scan_list.num;
    if (i == 0)
    {
        LogPrintf(DEBUG_ALL, "Wifi scan nothing\r\n");
    }
    else
    {
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_F3, &wifi_info);
    }

}
void wifiUploadRequestTask(void)
{
    if (sysinfo.wifirequest == 0)
        return ;
    if (isProtocolReday() == 0)
        return ;
    sysinfo.wifirequest = 0;
    startTimer(15, startWifiScan, 0);
}

/*-------------------------------------------------------------------------------------*/
//运行模式控制



void resetModeStartTime(void)
{
    sysinfo.runStartTick = sysinfo.System_Tick;
}

/*-------------------------------------------------------------------------------------*/
//LED
void ledRunTaskConfig(void)
{
    static nwy_osiTimer_t  *ledRunTimer = NULL;
    ledConfig();
    if (ledRunTimer == NULL)
    {
        ledRunTimer = nwy_timer_init(myAppEventThread, ledRunTask, NULL);
    }
    nwy_start_timer_periodic(ledRunTimer, 100);
}

/*-------------------------------------------------------------------------------------*/

static BLE_INFO bleinfo;

static void changeBleFsm(BLE_FSM fsm)
{
    bleinfo.bleFsm = fsm;
}
static void appBleRecv(void)
{
    char *length = NULL;
    char *precv = NULL;
    char recvlen[5];
    uint8_t realen;
    length = nwy_ble_receive_data(0);
    precv = nwy_ble_receive_data(1);
    if ((NULL != precv) & (NULL != length))
    {
        sprintf(recvlen, "%d", length);
        realen = atoi(recvlen);
        //nwy_ble_send_data(strlen(precv), precv);
        LogPrintf(DEBUG_ALL, "Ble receive %d bytes done\r\n", realen);
        //appBleRecvParser((uint8_t *)precv, realen);
        customerLogPrintf("+BLERXGET: %d\r\n", realen);
        pushRxData(&bleDataInfo, (uint8_t *)precv, realen);
    }
    else
    {
        LogMessage(DEBUG_ALL, "Ble Recv Null");
    }
    nwy_ble_receive_data(2);
}
static void appBleConnStatus(void)
{
    int conn_status = 0;
    conn_status = nwy_ble_get_conn_status();
    if (conn_status != 0)
    {
        LogPrintf(DEBUG_ALL, "Ble Connect:%d\r\n", conn_status);
        appBleSetConnectState(1);
        customerLogOut("+BLECON: 1\r\n");
    }
    else
    {
        LogMessage(DEBUG_ALL, "Ble Disconnect\r\n");
        appBleSetConnectState(0);
        customerLogOut("+BLECON: 0\r\n");
    }

    return;

}

static void configBleName(void)
{
    int ret;
    ret = nwy_ble_set_device_name((char *)sysparam.blename);
    LogPrintf(DEBUG_ALL, "SetBleName ret:%d\r\n", ret);
}

//static void bleUUidConfig(void)
//{
//	int ret;
//	uint8_t uuid[2];
//	uuid[0]=0xE5;
//	uuid[1]=0xFF;
//	ret=nwy_ble_set_service(uuid);
//	LogPrintf(DEBUG_ALL,"Add uuid ret=%d\r\n",ret);
//	uuid[0]=0xE9;
//	uuid[1]=0xFF;
//	ret=nwy_ble_set_character(0,uuid,1);
//	LogPrintf(DEBUG_ALL,"Add uuid ret=%d\r\n",ret);
//}


void appBleRestart(void)
{
    changeBleFsm(BLE_CLOSE);
}

void appBleAddService()
{
    uint8_t uuid0[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x89, 0x00, 0x00};

    nwy_ble_service_info ser_info_0;

    ser_info_0.ser_index = 1;


    memcpy(ser_info_0.ser_uuid, uuid0, 16);

    ser_info_0.char_num = 2;

    ser_info_0.p = 1;

    if (!nwy_ble_add_service(&ser_info_0))
        LogMessage(DEBUG_ALL, "add ble service 1 error\r\n");
    if (!nwy_ble_start_service(1))
        LogMessage(DEBUG_ALL, "start ble service 1 error\r\n");

}


void appBleTask(void)
{
    int ret;
    switch (bleinfo.bleFsm)
    {
        //查询蓝牙开启状态
        case BLE_CHECK_STATE:

            if (sysinfo.bleOnoff == 0)
                return ;
            ret = nwy_read_ble_status();
            LogPrintf(DEBUG_ALL, "Ble state:%s\r\n", ret ? "Open" : "Close");
            if (ret)
            {
                bleinfo.bleState = 1;
                changeBleFsm(BLE_CONFIG);
            }
            else
            {
                configBleName();
                bleinfo.bleState = 0;
                changeBleFsm(BLE_OPEN);
            }
            break;
        case BLE_OPEN:
            nwy_ble_enable();
            LogMessage(DEBUG_ALL, "Open BLE\r\n");
            changeBleFsm(BLE_CHECK_STATE);
            break;
        case BLE_CLOSE:
            nwy_ble_disable();
            changeBleFsm(BLE_CHECK_STATE);
            LogMessage(DEBUG_ALL, "Close BLE\r\n");
            break;
        case BLE_CONFIG:
            LogMessage(DEBUG_ALL, "Ble config\r\n");
            nwy_ble_register_callback(appBleRecv);
            nwy_ble_conn_status_cb(appBleConnStatus);
            changeBleFsm(BLE_NORMAL);
            break;
        case BLE_NORMAL:
            if (sysinfo.bleOnoff == 0)
            {
                changeBleFsm(BLE_CLOSE);
            }
            break;
    }
}



/*-------------------------------------------------------------------------------------*/

uint8_t systemIsIdle(void)
{
    sysinfo.dtrState = DTR_READ ? 1 : 0;
    if (sysinfo.bleRing > 0)
    {
        sysinfo.bleRing--;
    }

    if (sysinfo.GPSRequest != 0)
        return 1;
    if (sysinfo.updateStatus)
        return 2;
    if (sysinfo.dtrState != 0)
        return 3;
    if (bleDataIsSend() != 0)
        return 4;
    if (sysinfo.bleRing != 0)
        return 5;
    return 0;
}

void uartCtl(uint8_t onoff)
{
    if (onoff)
    {
        if (usart2_ctl.init == 0)
        {
            appUartConfig(APPUSART2, 1, 115200, cusBleRecvParser);//BLE
        }

        if (usart1_ctl.init == 0)
        {
            appUartConfig(APPUSART1, 1, 115200, atCmdParserFunction);//MCU
        }

    }
    else
    {
        if (usart2_ctl.init == 1)
        {
            appUartConfig(APPUSART2, 0, 115200, cusBleRecvParser);//DEBUG
        }
        if (usart1_ctl.init == 1)
        {
            appUartConfig(APPUSART1, 0, 115200, atCmdParserFunction);//DEBUG
        }
    }
}
void autosleepTask(void)
{
    uint8_t ret;
    ret = systemIsIdle();
    if (ret)//唤醒高电平
    {
        uartCtl(1);
    }
    else
    {
        uartCtl(0);
    }

}

/*-------------------------------------------------------------------------------------*/
static REC_UPLOAD_INFO recinfo;

void setUploadFile(uint8_t *filename, uint32_t filesize, uint8_t *recContent)
{
    memset(&recinfo, 0, sizeof(REC_UPLOAD_INFO));
    strcpy((char *)recinfo.recUploadFileName, (char *) filename);
    recinfo.recUploadFileSize = filesize;
    recinfo.recUploadContent = recContent;
    LogPrintf(DEBUG_ALL, "RecUpload==>%s:%d\r\n", filename, filesize);
    recinfo.recordUpload = 1;
}

static void recordUpload(void)
{
    char dest[4120];
    recinfo.needRead = recinfo.recUploadFileSize - recinfo.hadRead;
    if (recinfo.needRead > REC_UPLOAD_ONE_PACK_SIZE)
        recinfo.needRead = REC_UPLOAD_ONE_PACK_SIZE;
    LogPrintf(DEBUG_ALL, "File:%s , Total:%d , Offset:%d , Size:%d\r\n", recinfo.recUploadFileName,
              recinfo.recUploadFileSize, recinfo.hadRead, recinfo.needRead);
    createProtocol62(dest, (char *)recinfo.recUploadFileName, recinfo.hadRead / REC_UPLOAD_ONE_PACK_SIZE,
                     recinfo.recUploadContent + recinfo.hadRead, recinfo.needRead);

}

static void recordUploadInThread(void)
{
    recinfo.hadRead += recinfo.needRead;
    if (recinfo.hadRead >= recinfo.recUploadFileSize)
    {
        LogMessage(DEBUG_ALL, "Upload all complete\r\n");
        recinfo.recordUpload = 0;
        recinfo.recfsm = 0;
        return ;
    }
    recordUpload();
}


static void changeRecFsm(uint8_t fsm)
{
    recinfo.recfsm = fsm;
    recinfo.ticktime = 0;
}
void recordUploadRespon(void)
{
    changeRecFsm(3);
}
void recordFileUploadTask(void)
{
    char dest[50];
    if (recinfo.recordUpload == 0)
    {
        recinfo.ticktime = 0;
        return ;
    }
    recinfo.ticktime++;
    switch (recinfo.recfsm)
    {
        //上传文件名
        case 0:
            LogPrintf(DEBUG_ALL, "File:%s , Total:%d\r\n", recinfo.recUploadFileName, recinfo.recUploadFileSize);
            createProtocol61(dest, (char *)recinfo.recUploadFileName, recinfo.recUploadFileSize, 0, REC_UPLOAD_ONE_PACK_SIZE);
            changeRecFsm(1);
            break;
        //等待61返回
        case 1:
            if (recinfo.ticktime > 30)
            {
                recinfo.counttype1++;
                changeRecFsm(0);
                if (recinfo.counttype1 >= 3)
                {
                    recinfo.recordUpload = 0;
                }
            }
            break;
        //重发
        case 2:
            recordUpload();
            changeRecFsm(3);
            break;
        //等待62返回
        case 3:
            if (recinfo.ticktime > 30)
            {
                recinfo.counttype2++;
                changeRecFsm(2);
                if (recinfo.counttype2 >= 3)
                {
                    recinfo.recordUpload = 0;
                }
            }
            break;
    }
}
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
void gsensorTapTask(void)
{
    static uint8_t index = 0;
    static uint16_t aftick = 0;
    GPSINFO *gpsinfo;
    uint8_t i;
    uint16_t total = 0;
    if (sysparam.accdetmode != ACCDETMODE2)
    {
        return ;
    }
    if (sysparam.MODE != MODE2 && sysparam.MODE != MODE21 && sysparam.MODE != MODE23)
    {
        terminalAccoff();
        if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
        {
            gpsRequestClear(GPS_REQUEST_ACC_CTL);
        }
        aftick = 0;
        return ;
    }
    gpsinfo = getCurrentGPSInfo();
    sysinfo.onetaprecord[index++] = sysinfo.gsensortapcount;
    sysinfo.gsensortapcount = 0;
    if (index >= 15)
        index = 0;
    for (i = 0; i < 15; i++)
    {
        total += sysinfo.onetaprecord[i];
    }
    if (total >= 7	|| (sysinfo.gsensorerror == 1 && gpsinfo->fixstatus == 1 && gpsinfo->speed >= 15))
    {
        if (getTerminalAccState() == 0)
        {
            sysinfo.csqSearchTime = 60;
            terminalAccon();
            LogMessage(DEBUG_ALL, "Acc on detected by gsensor\n");
            if (isProtocolReday())
            {
                sendProtocolToServer(NORMAL_LINK, PROTOCOL_13, NULL);
                if (sysparam.protocol == USE_JT808_PROTOCOL)
                {
                    jt808UpdateStatus(JT808_STATUS_ACC, 1);
                    jt808SendToServer(TERMINAL_POSITION, getLastFixedGPSInfo());
                }
            }
            gpsRequestSet(GPS_REQUEST_ACC_CTL);
            gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
            memset(sysinfo.onetaprecord, 0, 15);
            aftick = 0;
        }
    }
    aftick++;
    if (total == 0)
    {
        //速度判断ACC
        if (gpsinfo->fixstatus == 1 && gpsinfo->speed >= 7)
        {
            aftick = 0;
        }
        if (aftick >= 90)
        {
            if (getTerminalAccState())
            {
                terminalAccoff();
                LogMessage(DEBUG_ALL, "Acc off detected by gsensor\n");
                if (isProtocolReday())
                {
                    sendProtocolToServer(NORMAL_LINK, PROTOCOL_13, NULL);
                    if (sysparam.protocol == USE_JT808_PROTOCOL)
                    {
                        jt808UpdateStatus(JT808_STATUS_ACC, 0);
                        jt808SendToServer(TERMINAL_POSITION, getLastFixedGPSInfo());
                    }
                }
                gpsRequestClear(GPS_REQUEST_ACC_CTL);
            }
        }
    }
    else
    {
        aftick = 0;
    }
}



/*-------------------------------------------------------------------------------------*/
//感光检测
void ldrCheckTask(void)
{
    //    static uint32_t darknessTick = 0;
    //    uint8_t curLdrState;
    //    curLdrState = LDR_DET;
    //
    //    if (curLdrState == 0)
    //    {
    //        //亮
    //        if (sysparam.lightAlarm && darknessTick >= 60)
    //        {
    //            LogMessage(DEBUG_ALL, "Light alarm\r\n");
    //            alarmRequestSet(ALARM_LIGHT_REQUEST);
    //        }
    //        darknessTick = 0;
    //    }
    //    else
    //    {
    //        //暗
    //        darknessTick++;
    //    }
}

/*-------------------------------------------------------------------------------------*/
static int usbReceiveData(void *data, size_t size)
{
    atCmdParserFunction((char *)data, size);
    return size;
}
static void usbInit(void)
{
    nwy_usb_serial_reg_recv_cb(usbReceiveData);
}
/*-------------------------------------------------------------------------------------*/




static void myAppConfig(void)
{
    memset(&sysinfo, 0, sizeof(SYSTEM_INFO));
    appUartConfig(APPUSART2, 1, 115200, cusBleRecvParser);//DEBUG
    appUartConfig(APPUSART1, 1, 115200, atCmdParserFunction);//mcu
    usbInit();
    paramInit();
    pmuConfig();
    gpioConfig();
    socketInfoInit();
    jt808InfoInit();
    smsListInit();
    cusDataInit();
    cusBleSendDataInit();
    //ledRunTaskConfig(); 取消LED
    portBleGpioCfg();
    nwy_pm_state_set(1);
    nwy_audio_set_handset_vol(sysparam.vol);
    sysinfo.logmessage = DEBUG_ALL;
    customerLogOut("+RDY\r\n");
    sysinfo.logmessage = DEBUG_NONE;

    socketListInit();
    networkInit();
    networkConnectCtl(1);
    updateSystemLedStatus(SYSTEM_LED_RUN, 1);
    if (sysparam.cUpdate == CUPDATE_FLAT)
    {
        customerLogOut("+UPDATE: SUCCESS\r\n");
        sysparam.cUpdate = 0;
        paramSaveAll();
    }
}

/*-------------------------------------------------------------------------------------*/

static void taskRunInOneSecond(void)
{
    sysinfo.System_Tick++;
    gpsRequestTask();
    gpsOutputCheckTask();
    if (sysinfo.taskSuspend == 1)
        return;
    networkConnect();
    updateFirmware();
    customtts();
    cusDataSendD();
    autosleepTask();

}
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*线程一：任务运行*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/

void myAppRun(void *param)
{
    myAppConfig();
    while (1)
    {
        taskRunInOneSecond();
        nwy_sleep(1000);
    }
}

/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
//+CLIP: "18814125495",129,,,,0
static void receiveCLIP(void)
{
    char buf[128];
    nwy_get_voice_callerid(buf);
    if (my_strpach(buf, "+CLIP"))
    {
        LogPrintf(DEBUG_ALL, "Phone ring:%s\r\n", buf);

        if (sysparam.sosnumber1[0] != 0)
        {
            if (strstr((char *)buf, (char *)sysparam.sosnumber1) != NULL)
            {
                LogPrintf(DEBUG_ALL, "SOS number %s,anaswer phone\n", sysparam.sosnumber1);
                nwy_voice_call_autoanswver();
                return ;
            }
        }
        if (sysparam.sosnumber2[0] != 0)
        {
            if (strstr((char *)buf, (char *)sysparam.sosnumber2) != NULL)
            {
                LogPrintf(DEBUG_ALL, "SOS number %s,anaswer phone\n", sysparam.sosnumber2);
                nwy_voice_call_autoanswver();
                return ;
            }
        }
        if (sysparam.sosnumber3[0] != 0)
        {
            if (strstr((char *)buf, (char *)sysparam.sosnumber3) != NULL)
            {
                LogPrintf(DEBUG_ALL, "SOS number %s,anaswer phone\n", sysparam.sosnumber3);
                nwy_voice_call_autoanswver();
                return ;
            }
        }
        LogMessage(DEBUG_ALL, "Unknow phone number\n");
    }
}

static void receiveCMT(void)
{
    nwy_sms_recv_info_type_t sms_data;
    char date[20];
    memset(&sms_data, 0, sizeof(sms_data));
    nwy_sms_recv_message(&sms_data);
    LogPrintf(DEBUG_ALL, "Tel[%d]:%s,Data[%d]:%s,Mt:%d,ShortId:%d,DCS:%d,Index:%d\r\n", sms_data.oa_size, sms_data.oa,
              sms_data.nDataLen, sms_data.pData, sms_data.cnmi_mt, sms_data.nStorageId, sms_data.dcs, sms_data.nIndex);
    sms_data.pData[sms_data.nDataLen + 1] = '#';
    sms_data.nDataLen += 1;
    //customerLogPrintf("+CMT: \"%s\",%d,%s\r\n", sms_data.oa, sms_data.nDataLen, sms_data.pData);

    sprintf(date, "%02d%02d%02d%02d%02d%02d", sms_data.scts.uYear % 100, sms_data.scts.uMonth, sms_data.scts.uDay,
            sms_data.scts.uHour, sms_data.scts.uMinute, sms_data.scts.uSecond);
    addSms((uint8_t *)sms_data.oa, (uint8_t *)date, (uint8_t *)sms_data.pData, sms_data.nDataLen);
    //instructionParser((uint8_t *)sms_data.pData, sms_data.nDataLen, SHORTMESSAGE_MODE, sms_data.oa, NULL);
}

static void sdkATCmdInit(void)
{
    nwy_init_sms_option();
    nwy_sdk_at_parameter_init();
    nwy_sdk_at_unsolicited_cb_reg("+CLIP:", receiveCLIP);
    nwy_sdk_at_unsolicited_cb_reg("+CMT:", receiveCMT);
    nwy_set_report_option(2, 2, 0, 0, 0);
    setCLIP();
    setAPN();
    setXGAUTH();
    getBleMac();

}

/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*线程二：接收事件*/
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/

char musicMp3[4][15] =
{
    {"alarm1.mp3"},
    {"alarm2.mp3"},
    {"alarm3.mp3"},
    {"alarm4.mp3"},
};

void myAppEvent(void *param)
{
    nwy_osiEvent_t event;
    sdkATCmdInit();
    while (1)
    {
        nwy_wait_thead_event(myAppEventThread, &event, 0);
        if (event.id == RECORD_UPLOAD_EVENT)
        {
            recordUploadInThread();
        }
        else if (event.id == GET_FIRMWARE_EVENT)
        {
            getFirmwareInThreadEvent();
        }
        else if (event.id == PLAY_MUSIC_EVENT)
        {
            if (sysinfo.ttsPlayNow == 0)
            {
                sysinfo.playMusicNow = 1;
                if (event.param1 == THREAD_PARAM_NONE)
                {
                    LogPrintf(DEBUG_ALL, "Alarm Music:%s\r\n", musicMp3[sysparam.alarmMusicIndex]);
                    nwy_audio_file_play(musicMp3[sysparam.alarmMusicIndex]);
                }
                else if (event.param1 == THREAD_PARAM_AUDIO)
                {
                    appPlayAudio();
                }
                else if (event.param1 == THREAD_PARAM_SIREN)
                {
                    LogMessage(DEBUG_ALL, "Siren Music\r\n");
                    nwy_audio_file_play(musicMp3[sysparam.alarmMusicIndex]);
                }
                sysinfo.playMusicNow = 0;
            }
        }
        else if (event.id == CFUN_EVENT)
        {
            if (event.param1 == 1)
            {
                networkConnectCtl(1);
                sendModuleCmd(CFUN_CMD, "1");
            }
            else
            {
                networkConnectCtl(0);
                sendModuleCmd(CFUN_CMD, "4");
            }
        }
    }
}
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------*/
void myAppRunIn100Ms(void *param)
{
    while (1)
    {
        nwy_sleep(100);
        appMcuUpgradeTask();
        kernalRun();
        outPutNodeCmd();
    }
}

void myAppCusPost(void *param)
{
    while (1)
    {
        nwy_sleep(200);
        postCusData();
    }
}

