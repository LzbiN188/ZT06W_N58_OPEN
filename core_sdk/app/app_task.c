#include "app_task.h"
#include "app_port.h"
#include "app_sys.h"
#include "app_atcmd.h"
#include "app_socket.h"
#include "app_kernal.h"
#include "app_protocol.h"
#include "app_param.h"
#include "app_net.h"
#include "app_gps.h"
#include "app_mir3da.h"
#include "app_instructioncmd.h"
#include "app_ble.h"
#include "app_jt808.h"
#include "app_db.h"

extern nwy_osiThread_t *myAppEventThread;
static sysLedCtrl_s sysLedCtrl;
static motionInfo_s motionInfo;

systemInfo_s sysinfo;


/**************************************************
@bref		系统灯1控制
@param
@note
**************************************************/

static void sysLed1CtrlTask(void)
{
    static uint8_t tick = 0;

    if (sysLedCtrl.sysLed1OnTime == 0)
    {
        LED1_OFF;
        return ;
    }
    else if (sysLedCtrl.sysLed1OffTime == 0)
    {
        LED1_ON;
        return ;
    }
    tick++;
    if (sysLedCtrl.sysLed1Onoff == 1)
    {
        LED1_ON;
        if (tick >= sysLedCtrl.sysLed1OnTime)
        {
            tick = 0;
            sysLedCtrl.sysLed1Onoff = 0;
        }
    }
    else
    {
        LED1_OFF;
        if (tick >= sysLedCtrl.sysLed1OffTime)
        {
            tick = 0;
            sysLedCtrl.sysLed1Onoff = 1;
        }
    }
}

/**************************************************
@bref		系统灯2控制
@param
@note
**************************************************/

static void sysLed2CtrlTask(void)
{
    static uint8_t tick = 0;

    if (sysLedCtrl.sysLed2OnTime == 0)
    {
        LED2_OFF;
        return ;
    }
    else if (sysLedCtrl.sysLed2OffTime == 0)
    {
        LED2_ON;
        return ;
    }
    tick++;
    if (sysLedCtrl.sysLed2Onoff == 1)
    {
        LED2_ON;
        if (tick >= sysLedCtrl.sysLed2OnTime)
        {
            tick = 0;
            sysLedCtrl.sysLed2Onoff = 0;
        }
    }
    else
    {
        LED2_OFF;
        if (tick >= sysLedCtrl.sysLed2OffTime)
        {
            tick = 0;
            sysLedCtrl.sysLed2Onoff = 1;
        }
    }
}

/**************************************************
@bref		设置灯闪烁实际
@param
	type	SYS_LED1_CTRL
			SYS_LED2_CTRL
	ontime	亮时长
	offtime 灭时长
@note
**************************************************/

static void ledSetPeriod(uint8_t type, uint8_t ontime, uint8_t offtime)
{
    switch (type)
    {
        case SYS_LED1_CTRL:
            sysLedCtrl.sysLed1OnTime = ontime;
            sysLedCtrl.sysLed1OffTime = offtime;
            break;
        case SYS_LED2_CTRL:
            sysLedCtrl.sysLed2OnTime = ontime;
            sysLedCtrl.sysLed2OffTime = offtime;
            break;
    }
}
/**************************************************
@bref		更新系统灯状态
@param
	status	系统状态
	onoff
@note
**************************************************/

void ledStatusUpdate(uint8_t status, uint8_t onoff)
{
    if (onoff)
    {
        sysLedCtrl.sysStatus |= status;
    }
    else
    {
        sysLedCtrl.sysStatus &= ~status;
    }

    if ((sysLedCtrl.sysStatus & SYSTEM_LED_RUN) != 0)
    {
        ledSetPeriod(SYS_LED1_CTRL, 10, 10);
        ledSetPeriod(SYS_LED2_CTRL, 10, 10);
        if ((sysLedCtrl.sysStatus & SYSTEM_LED_LOGIN) != 0)
        {
            ledSetPeriod(SYS_LED2_CTRL, 1, 9);

            if ((sysLedCtrl.sysStatus & SYSTEM_LED_GPS) != 0)
            {
                ledSetPeriod(SYS_LED2_CTRL, 1, 0);
            }
        }
    }
    else
    {
        ledSetPeriod(SYS_LED1_CTRL, 0, 10);
        ledSetPeriod(SYS_LED2_CTRL, 0, 10);
    }
    if ((sysLedCtrl.sysStatus & SYSTEM_LED_WDT) != 0)
    {

        ledSetPeriod(SYS_LED1_CTRL, 1, 1);
        ledSetPeriod(SYS_LED2_CTRL, 1, 1);
    }

}

/**************************************************
@bref		系统灯任务
@param
@note
**************************************************/

static void sysLedRunTask(void)
{

    if (sysinfo.sysTick >= 300 && sysinfo.wdtTest == 0)
    {
        if (sysparam.ledctrl == 0 || (sysparam.ledctrl == 1 && sysinfo.gpsRequest == 0))
        {
            LED1_OFF;
            LED2_OFF;
            return;
        }
    }

    sysLed1CtrlTask();
    sysLed2CtrlTask();
}


/**************************************************
@bref		更新运动或静止状态
@param
	src 		检测来源
	newState	新状态
@note
**************************************************/

void motionStateUpdate(motion_src_e src, motionState_e newState)
{
    char type[20];


    if (motionInfo.motionState == newState)
    {
        return;
    }
    motionInfo.motionState = newState;
    switch (src)
    {
        case ACC_SRC:
            strcpy(type, "acc");
            break;
        case VOLTAGE_SRC:
            strcpy(type, "voltage");
            break;
        case GSENSOR_SRC:
            strcpy(type, "gsensor");
            break;
        default:
            return;
            break;
    }
    LogPrintf(DEBUG_ALL, "Device %s , detected by %s", newState == MOTION_MOVING ? "moving" : "static", type);

    if (newState)
    {
        netResetCsqSearch();
        if (sysparam.gpsuploadgap != 0)
        {
            gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
            if (sysparam.gpsuploadgap < GPS_UPLOAD_GAP_MAX)
            {
                gpsRequestSet(GPS_REQ_ACC);
            }
        }
        terminalAccon();
        ClearLastMilePoint();
        hiddenServCloseClear();
        protocolUpdateSomeInfo(sysinfo.outsideVol, sysinfo.batteryVol, portCapacityCalculate(sysinfo.batteryVol),
                               sysparam.startUpCnt, sysparam.runTime);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_13, NULL);
        jt808SendToServer(TERMINAL_POSITION, getLastFixedGPSInfo());
        //jt808BatchPushIn(getCurrentGPSInfo());
    }
    else
    {
        if (sysparam.gpsuploadgap != 0)
        {
            gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
            gpsRequestClear(GPS_REQ_ACC);
        }
        terminalAccoff();
        protocolUpdateSomeInfo(sysinfo.outsideVol, sysinfo.batteryVol, portCapacityCalculate(sysinfo.batteryVol),
                               sysparam.startUpCnt, sysparam.runTime);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_13, NULL);
        jt808SendToServer(TERMINAL_POSITION, getLastFixedGPSInfo());
        //jt808BatchPushIn(getCurrentGPSInfo());
    }
}


/**************************************************
@bref		震动中断
@param
@note
**************************************************/

void motionOccur(void)
{
    motionInfo.tapInterrupt++;
}
/**************************************************
@bref		统计每一秒的中断次数
@param
@note
**************************************************/

static void motionCalculate(void)
{
    motionInfo.ind = (motionInfo.ind + 1) % sizeof(motionInfo.tapCnt);
    motionInfo.tapCnt[motionInfo.ind] = motionInfo.tapInterrupt;
    motionInfo.tapInterrupt = 0;
}
/**************************************************
@bref		获取这最近n秒的震动次数
@param
@note
**************************************************/

static uint16_t motionGetTotalCnt(void)
{
    uint16_t cnt;
    uint8_t i;
    cnt = 0;
    for (i = 0; i < sizeof(motionInfo.tapCnt); i++)
    {
        cnt += motionInfo.tapCnt[i];
    }
    return cnt;
}

/**************************************************
@bref		获取运动状态
@param
@note
**************************************************/

motionState_e motionGetStatus(void)
{
    return motionInfo.motionState;
}


/**************************************************
@bref		运动和静止的判断
@param
@note
**************************************************/

static void motionCheckTask(void)
{
    static uint16_t gsStaticTick = 0;
    static uint16_t autoTick = 0;
    static uint8_t  accOnTick = 0;
    static uint8_t  accOffTick = 0;

    static uint8_t  volOnTick = 0;
    static uint8_t  volOffTick = 0;

    static uint8_t alarmFlag = 0;
    static uint8_t detTick = 0;
    uint16_t totalCnt;

    gpsinfo_s *gpsinfo;

    motionCalculate();
    totalCnt = motionGetTotalCnt();

    if (sysparam.MODE == MODE1 || sysparam.MODE == MODE3)
    {
        terminalAccoff();
        if (gpsRequestGet(GPS_REQ_ACC))
        {
            gpsRequestClear(GPS_REQ_ACC);
        }
        if (gpsRequestGet(GPS_REQ_MOVE) == GPS_REQ_MOVE)
        {
            gpsRequestClear(GPS_REQ_MOVE);
        }
        gsStaticTick = 0;
        return ;
    }

    //gps间隔上送
    if (getTerminalAccState() && sysparam.gpsuploadgap >= GPS_UPLOAD_GAP_MAX)
    {
        if (++autoTick >= sysparam.gpsuploadgap)
        {
            autoTick = 0;
            gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
        }
    }
    else
    {
        autoTick = 0;
    }


    //拖车报警检测：运动后，如果没有ACC on，且在off下速度超过30km/h，则认为是拖车报警
    if ((sysparam.accdetmode == ACCDETMODE0 || sysparam.accdetmode == ACCDETMODE1) && getTerminalAccState() == 0)
    {
        //运动检测
        if (totalCnt >= 7)
        {
            if (gpsRequestGet(GPS_REQ_MOVE) == 0)
            {
                alarmFlag = 0;
                detTick = 0;
                gpsRequestSet(GPS_REQ_MOVE);
                LogMessage(DEBUG_ALL, "Device move !!!");
            }
        }
        if (totalCnt == 0)
        {
            gsStaticTick++;
            if (gsStaticTick >= 90)
            {
                if (gpsRequestGet(GPS_REQ_MOVE) == GPS_REQ_MOVE)
                {
                    alarmFlag = 0;
                    detTick = 0;
                    gpsRequestClear(GPS_REQ_MOVE);
                    LogMessage(DEBUG_ALL, "Device static !!!");
                }
            }
        }
        else
        {
            gsStaticTick = 0;
        }
        //是否拖车报警检测，判断速度
        if (alarmFlag == 0 && sysinfo.gpsOnoff)
        {
            gpsinfo = getCurrentGPSInfo();
            if (gpsinfo->fixstatus && gpsinfo->speed >= 30)
            {
                detTick++;
                if (detTick >= 60)
                {
                    //拖车报警
                    alarmFlag = 1;
                    alarmRequestSet(ALARM_TRIAL_REQUEST);
                }
            }
            else
            {
                detTick = 0;
            }
        }
        else
        {
            detTick = 0;
        }

    }
    else
    {
        alarmFlag = 0;
        //其他模式不需要这个request
        if (gpsRequestGet(GPS_REQ_MOVE) == GPS_REQ_MOVE)
        {
            gpsRequestClear(GPS_REQ_MOVE);
        }
    }



    if (ACC_READ == ACC_STATE_ON)
    {
        //线永远是第一优先级
        if (++accOnTick >= 10)
        {
            accOnTick = 0;
            motionStateUpdate(ACC_SRC, MOTION_MOVING);
        }
        accOffTick = 0;
        return;
    }
    accOnTick = 0;


    if (sysparam.accdetmode == ACCDETMODE0)
    {
        //仅由acc线控制
        if (++accOffTick >= 10)
        {
            accOffTick = 0;
            motionStateUpdate(ACC_SRC, MOTION_STATIC);
        }

        return;
    }


    if (sysparam.accdetmode == ACCDETMODE1)
    {
        //由acc线+电压控制
        if (sysinfo.outsideVol >= sysparam.accOnVoltage)
        {
            if (++volOnTick >= 15)
            {
                volOnTick = 0;
                motionStateUpdate(VOLTAGE_SRC, MOTION_MOVING);
            }
        }
        else
        {
            volOnTick = 0;
        }

        if (sysinfo.outsideVol < sysparam.accOffVoltage)
        {
            if (++volOffTick >= 15)
            {
                volOffTick = 0;
                motionStateUpdate(VOLTAGE_SRC, MOTION_STATIC);
            }
        }
        else
        {
            volOffTick = 0;
        }
        return;
    }
    //剩下的，由acc线+gsensor控制

    if (totalCnt >= 7)
    {
        motionStateUpdate(GSENSOR_SRC, MOTION_MOVING);
    }
    if (totalCnt == 0)
    {
        gsStaticTick++;
        if (gsStaticTick >= 90)
        {
            motionStateUpdate(GSENSOR_SRC, MOTION_STATIC);
        }
    }
    else
    {
        gsStaticTick = 0;
    }
}

/**************************************************
@bref		重启开启gsensor
@param
@note
**************************************************/

static void gsRepair(void)
{
    portGsensorCfg(1);
}

/**************************************************
@bref		gsensor检查任务
@param
@note
**************************************************/

static void gsCheckTask(void)
{
    static uint8_t tick = 0;
    static uint8_t errorcount = 0;
    if (sysinfo.gsensorOnoff == 0)
    {
        tick = 0;
        return;
    }

    if (sysinfo.gsensorErr)
    {
        errorcount = 0;
        return;
    }
    tick++;
    if (tick % 60 == 0)
    {
        tick = 0;
        if (readInterruptConfig() != 0)
        {
            LogMessage(DEBUG_ALL, "gsensor error");
            errorcount++;
            portGsensorCfg(0);
            if (errorcount >= 3)
            {
                errorcount = 0;
                sysinfo.gsensorErr = 1;
            }
            else
            {
                startTimer(20, gsRepair, 0);
            }
        }
        else
        {
            errorcount = 0;
            sysinfo.gsensorErr = 0;
        }
    }
}




/**************************************************
@bref		设置GPS请求
@param
@note
**************************************************/

void gpsRequestSet(uint32_t flag)
{
    sysinfo.gpsRequest |= flag;
    LogPrintf(DEBUG_ALL, "gpsRequestSet==>0x%04X", flag);
}
/**************************************************
@bref		清除GPS请求
@param
@note
**************************************************/
void gpsRequestClear(uint32_t flag)
{
    sysinfo.gpsRequest &= ~flag;
    LogPrintf(DEBUG_ALL, "gpsRequestClear==>0x%04X", flag);
}
/**************************************************
@bref		读取GPS请求
@param
@note
**************************************************/

uint8_t gpsRequestGet(uint32_t flag)
{
    return sysinfo.gpsRequest & flag;
}

static void openGnssGpsBd(void)
{
    portGpsSetPositionMode(NWY_LOC_AUX_GNSS_GPS_BD);
}

static void saveLastFixLocation(void)
{
    gpsinfo_s *gpsinfo;
    double latitude, longtitude;
    gpsinfo = getLastFixedGPSInfo();
    if (gpsinfo->fixstatus != 0)
    {
        latitude = gpsinfo->latitude;
        longtitude = gpsinfo->longtitude;
        if (gpsinfo->NS == 'S')
        {
            if (latitude > 0)
            {
                latitude *= -1;
            }
        }
        if (gpsinfo->EW == 'W')
        {
            if (longtitude > 0)
            {
                longtitude *= -1;
            }
        }
        LogPrintf(DEBUG_ALL, "Save Latitude:%f,Longtitude:%f\r\n", latitude, longtitude);
        sysparam.latitude = gpsinfo->latitude;
        sysparam.longtitude = gpsinfo->longtitude;
        paramSaveAll();
    }
}



/**************************************************
@bref		gps开关控制任务
@param
@note
**************************************************/
static void gpsRequestTask(void)
{
    static gpsState_e gpsFsm = GPS_STATE_IDLE;
    static uint8_t runTick = 0;
    static uint16_t gpsInvalidTick = 0;
    gpsinfo_s *gpsinfo;
    uint32_t noNmeaOutputTick;
    static lbsInfo_s lastlbs, nowlbs;

    static uint8_t gpsInvalidFlag = 0, gpsInvalidFlagTick = 0;
    switch (gpsFsm)
    {
        case GPS_STATE_IDLE:
            if (sysinfo.gpsRequest != 0)
            {
                sysinfo.updateLocalTimeReq = 1;
                sysinfo.gpsOnoff = 1;
                sysinfo.gpsUpdateTick = sysinfo.sysTick;
                if ((sysinfo.sysTick - sysinfo.gpsLastFixTick >= 7200) || sysinfo.gpsLastFixTick == 0)
                {
                    runTick = 0;
                    gpsFsm = GPS_STATE_WAIT;
                }
                else
                {
                    gpsFsm = GPS_STATE_RUN;
                }

                portGpsCtrl(1);
                startTimer(80, openGnssGpsBd, 0);
                gpsClearCurrentGPSInfo();
                
            }

            break;
        case GPS_STATE_WAIT:
            if (++runTick < 2)
            {
                break;
            }
            gpsFsm = GPS_STATE_RUN;
            agpsRequestSet();
            LogMessage(DEBUG_ALL, "start agps");
            break;
        case GPS_STATE_RUN:
            portGpsGetNmea(nmeaParser);

            gpsinfo = getCurrentGPSInfo();
            if (gpsinfo->fixstatus)
            {
                //已定位
                ledStatusUpdate(SYSTEM_LED_GPS, 1);
                protocolUpdateSatelliteUsed(gpsinfo->gpsviewstart, gpsinfo->beidouviewstart);
            }
            else
            {
                //未定位
                ledStatusUpdate(SYSTEM_LED_GPS, 0);
                protocolUpdateSatelliteUsed(gpsinfo->gpsviewstart, gpsinfo->beidouviewstart);
            }

            noNmeaOutputTick = sysinfo.sysTick - sysinfo.gpsUpdateTick;
            if (sysinfo.gpsRequest == 0 || noNmeaOutputTick >= 20)
            {
                if (noNmeaOutputTick >= 20)
                {
                    LogMessage(DEBUG_ALL, "no nmea output , try to reboot gps");
                }
                else 
                {
					saveLastFixLocation();
                }
                portSetGpsStarupMode(HOT_START);
                sysinfo.gpsOnoff = 0;
                portGpsCtrl(0);
                portSleepCtrl(1);
                ledStatusUpdate(SYSTEM_LED_GPS, 0);
                protocolUpdateSatelliteUsed(0, 0);
                gpsClearCurrentGPSInfo();
                gpsFsm = GPS_STATE_IDLE;
                //jt808BatchPushOut();
            }
            break;
    }

    if (sysinfo.gpsRequest == 0 || getTerminalAccState() == 0)
    {
        gpsInvalidTick = 0;
        gpsInvalidFlag = 0;
        gpsInvalidFlagTick = 0;
        return;
    }
    gpsinfo = getCurrentGPSInfo();
    if (gpsinfo->fixstatus == 0)
    {
        if (++gpsInvalidTick >= 900)
        {
            gpsInvalidTick = 0;
            alarmRequestSet(ALARM_GPS_NO_FIX_REQUEST);
            //行车定位异常报警
            gpsInvalidFlag = 1;
            lastlbs = portGetLbsInfo();
        }
    }
    else
    {
        gpsInvalidTick = 0;
        gpsInvalidFlag = 0;
        gpsInvalidFlagTick = 0;
    }
    //进入行车定位异常后
	if (gpsInvalidFlag == 1)
	{
		if (++gpsInvalidFlagTick >= 10)
		{
			gpsInvalidFlagTick = 0;
			nowlbs = portGetLbsInfo();
			if (nowlbs.lac != lastlbs.lac || nowlbs.cid != lastlbs.cid)
			{
				protocolUpdateLbsInfo(nowlbs.mcc, nowlbs.mnc, nowlbs.lac, nowlbs.cid);
    			sendProtocolToServer(NORMAL_LINK, PROTOCOL_19, NULL);
			}
		}
	}
	else 
	{
		gpsInvalidFlagTick = 0;
	}


}



/**************************************************
@bref		请求基站上报
@param
@note
**************************************************/

void lbsRequestSet(void)
{
    sysinfo.lbsRequest = 1;
}

/**************************************************
@bref		清除基站上报
@param
@note
**************************************************/

void lbsRequestClear(void)
{
    sysinfo.lbsRequest = 0;
}

/**************************************************
@bref		基站上报任务
@param
@note
**************************************************/

static void lbsRequestTask(void)
{
    lbsInfo_s lbs;
    if (sysinfo.lbsRequest == 0)
        return;
    if (serverIsReady() == 0)
        return;
    lbsRequestClear();
    lbs = portGetLbsInfo();
    protocolUpdateLbsInfo(lbs.mcc, lbs.mnc, lbs.lac, lbs.cid);
    sendProtocolToServer(NORMAL_LINK, PROTOCOL_19, NULL);
}


/**************************************************
@bref		wifi上送请求
@param
@note
**************************************************/

void wifiRequestSet(void)
{
    sysinfo.wifiRequest = 1;
}
/**************************************************
@bref		清除wifi请求
@param
@note
**************************************************/

void wifiRequestClear(void)
{
    sysinfo.wifiRequest = 0;
}

/**************************************************
@bref		开始wifi扫描
@param
@note
**************************************************/

static void wifiStartScan(void)
{
    int i = 0;
    wifiList_s wifi_info;
    nwy_wifi_scan_list_t scan_list;


    memset(&scan_list, 0, sizeof(scan_list));
    LogMessage(DEBUG_ALL, "Start wifi scan");
    nwy_wifi_scan(&scan_list);
    for (i = 0; i < scan_list.num; i++)
    {
        LogPrintf(DEBUG_ALL, "AP MAC:%02x:%02x:%02x:%02x:%02x:%02x,rssi %d dB",
                  scan_list.ap_list[i].mac[5], scan_list.ap_list[i].mac[4], scan_list.ap_list[i].mac[3], scan_list.ap_list[i].mac[2],
                  scan_list.ap_list[i].mac[1], scan_list.ap_list[i].mac[0], scan_list.ap_list[i].rssival);

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
        LogPrintf(DEBUG_ALL, "wifiscan nothing");
    }
    else
    {
        protocolUpdateWifiList(&wifi_info);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_F3, NULL);
    }
    bleServCloseRequestClear();

}

/**************************************************
@bref		wifi上报任务
@param
@note
**************************************************/

static void wifiRequestTask(void)
{
    if (sysinfo.wifiRequest == 0)
        return;
    if (serverIsReady() == 0)
        return;
    wifiRequestClear();
    bleServCloseRequestSet();
    startTimer(30, wifiStartScan, 0);
}

/**************************************************
@bref		设置报警上送请求
@param
	request	报警类型
@note
**************************************************/
void alarmRequestSet(uint32_t request)
{
    sysinfo.alarmrequest |= request;
    LogPrintf(DEBUG_ALL, "alarmRequestSet==>0x%04X", request);
    alarmRequestSave(sysinfo.alarmrequest);
    gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
}

/**************************************************
@bref		请求报警
@param
	request	报警类型
@note
**************************************************/

void alarmRequestClear(uint32_t request)
{
    sysinfo.alarmrequest &= ~request;
    LogPrintf(DEBUG_ALL, "alarmRequestClear==>0x%04X", request);
}

/**************************************************
@bref		保存特定报警信息值
@param
@note
**************************************************/

void alarmRequestSave(uint32_t request)
{
    sysparam.alarmRequest |= request;
    paramSaveAll();
}

/**************************************************
@bref		清除保存的报警值
@param
@note
**************************************************/

void alarmRequestClearSave(void)
{
    if (sysparam.alarmRequest != 0)
    {
        LogMessage(DEBUG_ALL, "clear saved alarm request");
        sysparam.alarmRequest = 0;
        paramSaveAll();
    }
}

/**************************************************
@bref		报警上送任务
@param
@note
**************************************************/

static void alarmRequestTask(void)
{

    lbsInfo_s lbs;
    uint8_t alarm;
    if (sysinfo.alarmrequest == 0)
    {
        return ;
    }
    if (serverIsReady() == 0)
    {
        return ;
    }

    lbs = portGetLbsInfo();
    protocolUpdateLbsInfo(lbs.mcc, lbs.mnc, lbs.lac, lbs.cid);

    //感光报警
    if (sysinfo.alarmrequest & ALARM_LIGHT_REQUEST)
    {
        alarmRequestClear(ALARM_LIGHT_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Light Alarm");
        terminalAlarmSet(TERMINAL_WARNNING_LIGHT);
        alarm = 0;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

    //低电报警
    if (sysinfo.alarmrequest & ALARM_LOWV_REQUEST)
    {
        alarmRequestClear(ALARM_LOWV_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>LowVoltage Alarm");
        terminalAlarmSet(TERMINAL_WARNNING_LOWV);
        alarm = 0;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

    //断电报警
    if (sysinfo.alarmrequest & ALARM_LOSTV_REQUEST)
    {
        alarmRequestClear(ALARM_LOSTV_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>lostVoltage Alarm");
        terminalAlarmSet(TERMINAL_WARNNING_LOSTV);
        alarm = 0;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

    //守卫报警
    if (sysinfo.alarmrequest & ALARM_GUARD_REQUEST)
    {
        alarmRequestClear(ALARM_GUARD_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Guard Alarm");
        alarm = 0x01;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

    //蓝牙断连报警
    if (sysinfo.alarmrequest & ALARM_BLE_LOST_REQUEST)
    {
        alarmRequestClear(ALARM_BLE_LOST_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE disconnect Alarm");
        alarm = 0x14;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

    //蓝牙断连恢复报警
    if (sysinfo.alarmrequest & ALARM_BLE_RESTORE_REQUEST)
    {
        alarmRequestClear(ALARM_BLE_RESTORE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE restore Alarm");
        alarm = 0x1A;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

    //油电恢复报警
    if (sysinfo.alarmrequest & ALARM_OIL_RESTORE_REQUEST)
    {
        alarmRequestClear(ALARM_OIL_RESTORE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>oil restore Alarm");
        alarm = 0x19;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //信号屏蔽报警
    if (sysinfo.alarmrequest & ALARM_SHIELD_REQUEST)
    {
        alarmRequestClear(ALARM_SHIELD_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE shield Alarm");
        alarm = 0x17;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //伪信号屏蔽报警
    if (sysinfo.alarmrequest & ALARM_FAKE_SHIELD_REQUEST)
    {
		alarmRequestClear(ALARM_FAKE_SHIELD_REQUEST);
		LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE fake shield Alarm");
		alarm = 0x33;
		protocolUpdateEvent(alarm);
		sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

    //蓝牙预警报警
    if (sysinfo.alarmrequest & ALARM_PREWARN_REQUEST)
    {
        alarmRequestClear(ALARM_PREWARN_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE perwarn Alarm");
        alarm = 0x1D;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //蓝牙锁定报警
    if (sysinfo.alarmrequest & ALARM_OIL_CUTDOWN_REQUEST)
    {
        alarmRequestClear(ALARM_OIL_CUTDOWN_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE locked Alarm");
        alarm = 0x1E;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //蓝牙异常报警
    if (sysinfo.alarmrequest & ALARM_BLE_ERR_REQUEST)
    {
        alarmRequestClear(ALARM_BLE_ERR_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE err Alarm");
        alarm = 0x18;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //gps未定位异常报警
    if (sysinfo.alarmrequest & ALARM_GPS_NO_FIX_REQUEST)
    {
        alarmRequestClear(ALARM_GPS_NO_FIX_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>gps err Alarm");
        alarm = 0x1C;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //拖车报警
    if (sysinfo.alarmrequest & ALARM_TRIAL_REQUEST)
    {
        alarmRequestClear(ALARM_TRIAL_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>拖车报警");
        alarm = 0x1F;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //关机报警
    if (sysinfo.alarmrequest & ALARM_SHUTDOWN_REQUEST)
    {
        alarmRequestClear(ALARM_SHUTDOWN_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>关机报警");
        alarm = 0x30;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //开盖报警
    if (sysinfo.alarmrequest & ALARM_UNCAP_REQUEST)
    {
        alarmRequestClear(ALARM_UNCAP_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>开盖报警");
        alarm = 0x31;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }
    //拔卡报警
    if (sysinfo.alarmrequest & ALARM_SIMPULLOUT_REQUEST)
    {
        alarmRequestClear(ALARM_SIMPULLOUT_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>拔卡报警");
        alarm = 0x32;
        protocolUpdateEvent(alarm);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_16, NULL);
    }

}

/**************************************************
@bref		录音请求
@param
@note
**************************************************/

uint8_t recordRequestSet(uint8_t recordTime)
{
    if (portIsRecordNow())
    {
        return 0;
    }
    if (isRecUploadRun())
    {
        return 0;
    }
    sysinfo.recordTime = recordTime;
    return 1;
}

/**************************************************
@bref		录音结束
@param
@note
**************************************************/

void recordRequestClear(void)
{
    sysinfo.recordTime = 0;
}

/**************************************************
@bref		录音任务
@param
@note
**************************************************/

static void recordTask(void)
{
    static uint8_t openRecordFlag = 0;
    int ret;
    if (sysinfo.recordTime == 0)
    {
        if (openRecordFlag)
        {
            openRecordFlag = 0;
            portRecStop();
            portRecUpload();
        }
        return ;
    }
    if (openRecordFlag == 0)
    {
        ret = portRecStart();
        if (ret == 1)
        {
            openRecordFlag = 1;
        }
        else
        {
            LogPrintf(DEBUG_ALL, "open recorder fail , ret=%d", ret);
            sysinfo.recordTime = 0;
        }
    }
    LogPrintf(DEBUG_ALL, "stop recording count down %ds", sysinfo.recordTime);
    sysinfo.recordTime--;
}
/**************************************************
@bref		查询系统状态是否为运行状态
@param
@return
	1		是
	0		否
@note
**************************************************/

uint8_t sysIsInRun(void)
{
    if (sysinfo.runFsm == SYS_RUN)
    {
        return 1;
    }
    return 0;
}

void sysResetStartRun(void)
{
    sysinfo.sysStartTick = sysinfo.sysTick;
}

/**************************************************
@bref		系统状态切换
@param
	newState	见runState_e
@note
**************************************************/

static void sysChaneState(runState_e newState)
{
    sysinfo.runFsm = newState;
}

/**************************************************
@bref		start状态
@param
@note
**************************************************/

static void sysStart(void)
{
    LogPrintf(DEBUG_ALL, "sysStart==>MODE %d", sysparam.MODE);
    switch (sysparam.MODE)
    {
        case MODE1:
        case MODE3:
            sysparam.startUpCnt++;
            lbsRequestSet();
            wifiRequestSet();
            portGsensorCfg(0);
            paramSaveAll();
            break;
        case MODE2:
        case MODE21:
        case MODE23:
            if (sysparam.MODE == MODE2)
            {
                bleServRequestOn5Minutes();
                if (sysparam.accctlgnss == 0)
                {
                    gpsRequestSet(GPS_REQ_KEEPON);
                }
            }
            portGsensorCfg(1);
            break;
        default:

            break;
    }
    sysResetStartRun();
    gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
    networkConnectCtl(1);
    sysChaneState(SYS_RUN);
    if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
    {
        ledStatusUpdate(SYSTEM_LED_RUN, 0);
    }
    else
    {

        ledStatusUpdate(SYSTEM_LED_RUN, 1);
    }
}

/**************************************************
@bref		快速进入stop模式
@param
@note
**************************************************/

static void sysEnterStopQuickly(void)
{
    static uint8_t delaytick = 0;
    if (sysinfo.gpsRequest == 0 && sysinfo.wifiRequest == 0 && sysinfo.lbsRequest == 0 && getTerminalAccState() == 0)
    {
        delaytick++;
        if (delaytick >= 30)
        {
            LogMessage(DEBUG_ALL, "sysEnterStopQuickly");
            delaytick = 0;
            sysChaneState(SYS_STOP); //执行完毕，关机
        }
    }
    else
    {
        delaytick = 0;
    }
}

/**************************************************
@bref		记录运行时长
@param
@note
**************************************************/

static void sysRunTimeCnt(void)
{
    static uint8_t runTick = 0;
    if (++runTick >= 180)
    {
        runTick = 0;
        sysparam.runTime++;
        paramSaveAll();
    }
}

/**************************************************
@bref		run状态
@param
@note
**************************************************/

static void sysRun(void)
{
	gpsMileRecord();
    switch (sysparam.MODE)
    {
        case MODE1:
        case MODE3:
            if (sysinfo.sysTick - sysinfo.sysStartTick >= 210)
            {
                gpsRequestClear(GPS_REQ_ALL);
                lbsRequestClear();
                wifiRequestClear();
                sysChaneState(SYS_STOP);
            }
            sysEnterStopQuickly();
            break;
        case MODE2:
            gpsUploadPointToServer();
            sysRunTimeCnt();
            break;
        case MODE21:
        case MODE23:
            gpsUploadPointToServer();
            sysEnterStopQuickly();
            sysRunTimeCnt();
            break;
        default:
            break;
    }


}

/**************************************************
@bref		stop状态
@param
@note
**************************************************/

static void sysStop(void)
{
    if (sysparam.MODE == MODE1 || sysparam.MODE == MODE3)
    {
        portGsensorCfg(0);
    }
    networkConnectCtl(0);
    sysChaneState(SYS_WAIT);
    ledStatusUpdate(SYSTEM_LED_RUN, 0);
}

/**************************************************
@bref		wait状态
@param
@note
**************************************************/

static void sysWait(void)
{
    if (sysparam.MODE == MODE2 || sysinfo.gpsRequest != 0 || sysinfo.alarmrequest != 0 || sysinfo.lbsRequest != 0 ||
            sysinfo.wifiRequest != 0 || getTerminalAccState())
    {
        sysChaneState(SYS_START);
    }
}

/**************************************************
@bref		系统自动唤醒
@param
@note
**************************************************/

static void sysAutoReq(void)
{
    uint16_t year, curMinutes, i;
    uint8_t  month, date, hour, minute, second, ret;
    ret = 0;
    if (sysparam.MODE == MODE1 || sysparam.MODE == MODE21)
    {
        if (sysinfo.runFsm != SYS_WAIT)
        {
            return;
        }
        portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);
        curMinutes = hour * 60 + minute;
        for (i = 0; i < 5; i++)
        {
            if (sysparam.AlarmTime[i] != 0xFFFF)
            {
                if (sysparam.AlarmTime[i] == curMinutes)
                {
                    ret = 1;
                }
            }
        }
    }
    else if (sysparam.gapMinutes != 0)
    {
        if (sysinfo.sysTick % (sysparam.gapMinutes * 60) == 0)
        {
            ret = 1;
        }
    }
    if (ret)
    {
        LogMessage(DEBUG_ALL, "sysAutoReq");
        gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
    }
}



/**************************************************
@bref		系统任务
@param
@note
**************************************************/


static void sysRunTask(void)
{
    sysAutoReq();
    switch (sysinfo.runFsm)
    {
        case SYS_START:
            sysStart();
            break;
        case SYS_RUN:
            sysRun();
            break;
        case SYS_STOP:
            sysStop();
            break;
        case SYS_WAIT:
            sysWait();
            break;
        default:
            LogPrintf(DEBUG_ALL, "Debug:unknow:%d", sysinfo.runFsm);
            break;
    }
}
/**************************************************
@bref		硬件喂狗任务
@param
@note
**************************************************/
static void feedWdtTask(void)
{
    static uint8 flag = 0;
    sysinfo.softWdtTick = 0;

    if (sysinfo.wdtTest)
    {
        return;
    }
    flag ^= 1;
    if (flag)
    {
        WDT_FEED_H;
    }
    else
    {
        WDT_FEED_L;
    }
}


/**************************************************
@bref		电压检测任务
@param
@note
**************************************************/

static void voltageCheckTask(void)
{
    static uint8_t  lowwflag = 0;
    static uint8_t  LostVoltageFlag = 0;
    static uint16_t lowpowertick = 0;
    static uint16_t LostTick = 0;
    static uint32_t  LostVoltageTick = 0;

    static uint8_t bleCutFlag = 0;
    static uint8_t bleCutTick = 0;

    sysinfo.outsideVol = portGetOutSideVolAdcVol() * sysparam.adccal;
    sysinfo.batteryVol = portGetBatteryAdcVol();

    //低电报警
    if (sysinfo.outsideVol < sysparam.lowvoltage)
    {
        lowpowertick++;
        if (lowpowertick >= 30)
        {
            if (lowwflag == 0)
            {
                lowwflag = 1;
                LogPrintf(DEBUG_ALL, "power supply too low %.2fV", sysinfo.outsideVol);
                //低电报警
                jt808UpdateAlarm(JT808_LOWVOLTAE_ALARM, 1);
                alarmRequestSet(ALARM_LOWV_REQUEST);
                lbsRequestSet();
                wifiRequestSet();
                gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
            }

        }
    }
    else
    {
        lowpowertick = 0;
    }


    if (sysinfo.outsideVol >= sysparam.lowvoltage + 0.5)
    {
        lowwflag = 0;
        jt808UpdateAlarm(JT808_LOWVOLTAE_ALARM, 0);
    }

    if (sysinfo.outsideVol < 5.0)
    {
        LostTick++;
        if (LostVoltageFlag == 0 && LostTick >= 20)
        {
            LostVoltageFlag = 1;
            LostVoltageTick = sysinfo.sysTick;
            terminalunCharge();
            jt808UpdateAlarm(JT808_LOSTVOLTAGE_ALARM, 1);
            LogMessage(DEBUG_ALL, "Lost power supply");
            alarmRequestSet(ALARM_LOSTV_REQUEST);
            lbsRequestSet();
            wifiRequestSet();
            gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
            if (sysparam.bleRelay != 0 && bleCutFlag != 0)
            {
                LogMessage(DEBUG_ALL, "ble relay on immediately");
                sysparam.relayCtl = 1;
                paramSaveAll();
                if (sysparam.relayFun == 0)
                {
                    relayAutoClear();
                    RELAY_ON;
                    bleScheduleClearAllReq(BLE_EVENT_SET_DEVOFF);
                    bleScheduleSetAllReq(BLE_EVENT_SET_DEVON);
                }
                else
                {
                    relayAutoRequest();
                }
            }
            else
            {
                LogMessage(DEBUG_ALL, "relay on was disable");
            }
        }

    }
    else if (sysinfo.outsideVol > 6.0)
    {

        //电压小于设置的保护电压范围时，则运行蓝牙去执行断电报警
        if (sysinfo.outsideVol >= sysparam.bleVoltage)
        {
            if (bleCutTick++ >= 30)
            {
                bleCutFlag = 1;
            }
        }
        else
        {
            bleCutTick = 0;
            bleCutFlag = 0;
        }


        LostTick = 0;
        terminalCharge();
        if (LostVoltageFlag == 1)
        {
            LostVoltageFlag = 0;
            jt808UpdateAlarm(JT808_LOSTVOLTAGE_ALARM, 0);
            if (sysinfo.sysTick - LostVoltageTick >= 10)
            {
                LogMessage(DEBUG_ALL, "power supply resume");
                if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
                {
                    portSimSet(SIM_1);
                }
                portSystemReset();
            }
        }
    }

}

/**************************************************
@bref		感光检测任务
@param
@note
**************************************************/

static void lightDetectionTask(void)
{
    static uint32_t darknessTick = 0;
    static uint32_t FrontdarknessTick = 0;
    uint8_t curLdrState;
    curLdrState = LDR_READ;

    if (curLdrState == 0)
    {
        //亮
        if (darknessTick >= 60)
        {
            if (sysparam.lightAlarm != 0)
            {
                LogMessage(DEBUG_ALL, "Light alarm");
                alarmRequestSet(ALARM_LIGHT_REQUEST);
            }
        }
        darknessTick = 0;
    }
    else
    {
        //暗
        darknessTick++;
    }

    //前感光检测
    curLdrState = LDR2_READ;
    if (curLdrState == 0)
    {
        //亮
        if (FrontdarknessTick >= 60)
        {
            if (sysparam.uncapalm != 0)
            {
                LogMessage(DEBUG_ALL, "Uncap alarm");
                alarmRequestSet(ALARM_UNCAP_REQUEST);
                if (sysparam.uncapLock != 0)
                {
                    sysparam.relayCtl = 1;
                    paramSaveAll();
                    relayAutoRequest();
                    LogPrintf(DEBUG_ALL, "uncap==>try to relay on");
                }
            }
        }
        FrontdarknessTick = 0;
    }
    else
    {
        //暗
        FrontdarknessTick++;
    }
}

/**************************************************
@bref		上送1个gps位置点
@param
@note
**************************************************/

static void gpsRequestUploadOnePoiTask(void)
{
    gpsinfo_s *gpsinfo;
    static uint8_t runtick = 0;
    static uint8_t uploadtick = 0;
    //判断是否有请求该事件
    if (gpsRequestGet(GPS_REQ_UPLOAD_ONE_POI) == 0)
    {
        runtick = 0;
        uploadtick = 0;
        return ;
    }
    gpsinfo = getCurrentGPSInfo();
    runtick++;
    if (runtick >= 180)
    {
        runtick = 0;
        uploadtick = 0;
        doPositionRespon();
        gpsRequestClear(GPS_REQ_UPLOAD_ONE_POI);
    }
    else
    {
        if (gpsinfo->fixstatus)
        {
            if (++uploadtick >= 10)
            {
                uploadtick = 0;
                doPositionRespon();
                sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, getCurrentGPSInfo());
                jt808SendToServer(TERMINAL_POSITION, getCurrentGPSInfo());
                //jt808BatchPushIn(getCurrentGPSInfo());
                gpsRequestClear(GPS_REQ_UPLOAD_ONE_POI);
            }
        }
        else
        {
            uploadtick = 0;
        }
    }
}

/**************************************************
@bref		软件看门狗
@param
@note
**************************************************/

static void softWdtRun(void)
{
    if (sysinfo.softWdtTick++ >= 250)
    {
        sysinfo.softWdtTick = 0;
        portSystemReset();
    }
}

/**************************************************
@bref		每天重启
@param
@note
**************************************************/

static void rebootOneDay(void)
{
    sysinfo.sysTick++;

    if (sysinfo.sysTick < 86400)
        return ;
    if (sysinfo.gpsRequest != 0)
        return ;
    if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
    {
        portSimSet(SIM_1);
    }
    sysparam.bleErrCnt = 0;
    paramSaveAll();
    portSystemReset();
}

/**************************************************
@bref		继电器状态初始化
@param
@note
**************************************************/

static void relayInit(void)
{
    if (sysparam.relayCtl)
    {
        RELAY_ON;
    }
    else
    {
        RELAY_OFF;
    }
}

/**************************************************
@bref		系统关机任务
@param
@note		检测到关机引脚为高电平时关机
**************************************************/

static void autoShutDownTask(void)
{
    static uint16_t shutdownTick;
    if (ONOFF_READ == ON_STATE)
    {
        shutdownTick = 0;
        return;
    }
    ledStatusUpdate(SYSTEM_LED_RUN, 0);
    if (shutdownTick == 0 && sysparam.shutdownalm != 0)
    {
        alarmRequestSet(ALARM_SHUTDOWN_REQUEST);
        if (sysparam.shutdownLock != 0)
        {
            sysparam.relayCtl = 1;
            paramSaveAll();
            doRelayOn();
            LogPrintf(DEBUG_ALL, "shutdown==>try to relay on");
        }
    }
    else if (sysparam.shutdownalm == 0 || shutdownTick >= SHUTDOWN_TIME)
    {
        shutdownTick = 0;
        sysparam.bleErrCnt = 0;
        if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
        {
            portSimSet(SIM_1);
        }
        paramSaveAll();
        portSystemShutDown();
    }
    else if (bleScheduleCheckReq(BLE_EVENT_SET_DEVON))
    {
        LogPrintf(DEBUG_ALL, "wait ble event clear...");
    }
    else if (sysparam.alarmRequest == 0)
    {
    	LogPrintf(DEBUG_ALL, "wait alarmRequest clear...");
        shutdownTick = SHUTDOWN_TIME;
    }
    shutdownTick++;
}


/**************************************************
@bref		串口自动控制任务
@param
@note
**************************************************/

static void autoUartTask(void)
{

    if (sysinfo.gpsRequest || sysinfo.logLevel != DEBUG_NONE)
    {
        if (usart2_ctl.init == 0)
        {
            portUartCfg(APPUSART2, 1, 115200, atCmdRecvParser);
        }
    }
    else
    {
        if (usart2_ctl.init)
        {
            portUartCfg(APPUSART2, 0, 0, NULL);
        }
    }
}

/**************************************************
@bref		relayAutoRequest
@param
@note
**************************************************/

void relayAutoRequest(void)
{
    sysinfo.doRelayFlag = 1;
}

/**************************************************
@bref		relayAutoClear
@param
@note
**************************************************/

void relayAutoClear(void)
{
    sysinfo.doRelayFlag = 0;
}

/**************************************************
@bref		继电器自动控制
@param
@note
**************************************************/
void doRelayOn(void)
{
    relayAutoClear();
    RELAY_ON;
    bleScheduleClearAllReq(BLE_EVENT_SET_DEVOFF);
    bleScheduleSetAllReq(BLE_EVENT_SET_DEVON | BLE_EVENT_SET_RTC);
    LogMessage(DEBUG_ALL, "do relay on");

}

void relayAutoCtrl(void)
{
    static uint8_t runTick = 0;
    gpsinfo_s *gpsinfo;
    if (sysinfo.doRelayFlag == 0)
    {
        runTick = 0;
        return	;
    }
    if (getTerminalAccState() == 0)
    {
        //设备静止了，立即断油电，本机relay控制线断，如果有蓝牙，也一块断
        doRelayOn();
        return;
    }
    if (sysparam.relaySpeed == 0)
    {
        //没有配置速度规则，那就等acc off才断
        return;
    }
    if (sysinfo.gpsOnoff == 0)
    {
        return;
    }
    gpsinfo = getCurrentGPSInfo();
    if (gpsinfo->fixstatus == 0)
    {
        return;
    }
    if (gpsinfo->speed > sysparam.relaySpeed)
    {
        runTick = 0;
        return;
    }
    if (++runTick >= 5)
    {
        runTick = 0;
        doRelayOn();
    }
}

static void wdtTest(void)
{
    portUartSend(SYS_DEBUG_UART, (uint8_t *)"AT^FMPC_WDTSTOP\r\n", 17);
}
/**************************************************
@bref		主线程
@param
@note
**************************************************/

void myAppRun(void *param)
{
    memset(&motionInfo, 0, sizeof(motionInfo_s));
    memset(&sysinfo, 0, sizeof(systemInfo_s));
    sysinfo.logLevel = DEBUG_ALL;
    portUSBCfg((nwy_sio_recv_cb_t)atCmdRecvParser);
    portUartCfg(APPUSART2, 1, 115200, atCmdRecvParser);
    LogMessage(DEBUG_NONE, "Application Run");
    portPmuCfg();
    portGpioCfg();

    paramInit();
    socketListInit();
    networkInfoInit();
    kernalTimerInit();
    protocolInit();
    portRecInit();
    relayInit();
    jt808InfoInit();
    jt808BatchInit();
    bleClientInfoInit();
    bleScheduleInit();
    dbSaveInit();
    startTimer(10, wdtTest, 0);
    portSleepCtrl(1);
    while (1)
    {
        nwy_sleep(1000);
        rebootOneDay();
        feedWdtTask();
        networkConnectTask();
        serverConnTask();
        agpsConnRunTask();
        upgradeRunTask();
        motionCheckTask();
        gsCheckTask();
        voltageCheckTask();
        gpsRequestTask();
        gpsRequestUploadOnePoiTask();
        lbsRequestTask();
        wifiRequestTask();
        alarmRequestTask();
        lightDetectionTask();
        recordUploadTask();
        recordTask();
        sysRunTask();
        bleServRunTask();
        bleScheduleTask();
        autoShutDownTask();
        autoUartTask();
        relayAutoCtrl();
    }
}

/**************************************************
@bref		100ms线程
@param
@note
**************************************************/

void myApp100MSRun(void *param)
{
    while (1)
    {
        kernalRun();
        softWdtRun();
        sysLedRunTask();
        portOutputTTS();
        nwy_sleep(100);
    }
}


/**************************************************
@bref		处理事件线程
@param
@note
**************************************************/

void myAppEvent(void *param)
{
    nwy_osiEvent_t event;
    portAtCmdInit();
    while (1)
    {
        nwy_wait_thead_event(myAppEventThread, &event, 0);
        switch (event.id)
        {
            case THREAD_EVENT_PLAY_AUDIO:
                portPlayAudio(AUDIOFILE);
                break;
            case THREAD_EVENT_PLAY_REC:
                portPlayAudio(RECFILE);
                break;
            case THREAD_EVENT_REC:
                if (event.param1 == THREAD_PARAM_REC_START)
                {
                    portRecStart();
                }
                else if (event.param1 == THREAD_PARAM_REC_STOP)
                {
                    portRecStop();
                }
                break;
            case THREAD_EVENT_SOCKET_CLOSE:
                socketClose(event.param1);
                break;
            case THREAD_EVENT_BLE_CLIENT:
                bleClientDoEventProcess(event.param1);
                break;
        }
    }
}

/**************************************************
@bref		触发线程事件
@param
	threadEvent	事件类型
	param1		事件参数
@note
**************************************************/

void appSendThreadEvent(uint16 threadEvent, uint32_t param1)
{
    nwy_osiEvent_t event;
    memset(&event, 0, sizeof(event));
    event.id = threadEvent;
    event.param1 = param1;
    nwy_send_thead_event(myAppEventThread, &event, 0);
}


