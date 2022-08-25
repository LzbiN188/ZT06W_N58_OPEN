#include "app_instructioncmd.h"
#include "app_sys.h"
#include "app_net.h"
#include "app_param.h"
#include "app_task.h"
#include "app_port.h"
#include "app_mir3da.h"
#include "app_protocol.h"
#include "app_sn.h"
#include "aes.h"
#include "app_ble.h"
#include "app_protocol_808.h"
#include "nwy_audio_api.h"
#include "app_kernal.h"
//平台指令列表
const INSTABLE instructiontable[MAX_INS] =
{
    {PARAM_INS, "PARAM"},
    {STATUS_INS, "STATUS"},
    {VERSION_INS, "VERSION"},
    {SERVER_INS, "SERVER"},
    {HBT_INS, "HBT"},
    {MODE_INS, "MODE"},
    {TTS_INS, "TTS"},
    {JT_INS, "JT"},
    {POSITION_INS, "123"},
    {APN_INS, "APN"},
    {UPS_INS, "UPS"},
    {LOWW_INS, "LOWW"},
    {LED_INS, "LED"},
    {POITYPE_INS, "POITYPE"},
    {RESET_INS, "RESET"},
    {UTC_INS, "UTC"},
    {ALARMMODE_INS, "ALARMMODE"},
    {DEBUG_INS, "DEBUG"},
    {ACCCTLGNSS_INS, "ACCCTLGNSS"},
    {PDOP_INS, "PDOP"},
    {SETBLEMAC_INS, "SETBLEMAC"},
    {BF_INS, "BF"},
    {CF_INS, "CF"},
    {FACTORYTEST_INS, "FACTORYTEST"},
    {FENCE_INS, "FENCE"},
    {FACTORY_INS, "FACTORY"},
    {BLEUNBIND_INS, "BLEUNBIND"},
    {BLEEN_INS, "BLEEN"},
    {PROTECTVOL_INS, "PROTECTVOL"},
    {ACCURACY_INS, "ACCURACY"},
    {SOS_INS, "SOS"},
    {PROTOCOL_INS, "PROTOCOL"},
    {JT808SN_INS, "JT808SN"},
    {RELAY_INS, "RELAY"},
    {ECFG_INS, "ECFG"},
    {VOL_INS, "VOL"},
    {ALARMMUSIC_INS, "ALARMMUSIC"},
    {FACTORYMODE_INS, "FACTORYMODE"},
    {ACCDETMODE_INS, "ACCDETMODE"},
    {BATVOLTABLE_INS, "BATVOLTABLE"},
    {SIRENON_INS, "SIRENON"},
    {SETAGPS_INS, "SETAGPS"},
    {SN_INS, "*"},
};

/*获取通用指令表对应指令ID*/
static int16_t getInstructionid(uint8_t *cmdstr)
{
    uint16_t i = 0;
    if (cmdstr == NULL)
    {
        LogMessage(DEBUG_ALL, "getInstructionid==>no cmd\n");
        return -1;
    }
    for (i = 0; i < MAX_INS; i++)
    {
        if (mycmdPatch(cmdstr, (uint8_t *)instructiontable[i].cmdstr) != 0)
            return instructiontable[i].cmdid;
    }
    return -1;
}

static void bleSend(uint8_t *buf, uint16_t len)
{
    uint8_t *ins;
    uint8_t enclen;
    char respon[300];
    ins = getInstructionId();
    //CMD[01020304]:
    sprintf(respon, "CMD[%02X%02X%02X%02X]:%s", ins[0], ins[1], ins[2], ins[3], buf);
    len += 14;
    encryptData(respon, &enclen, respon, len);
    appBleSendData((uint8_t *)respon, enclen);
}

static void bleSendDirect(uint8_t *buf, uint16_t len)
{
    appBleSendData(buf, len);
}
static void sendMessageWithDifMode(uint8_t *buf, uint16_t len, DOINSTRUCTIONMODE mode, char *telnum, uint8_t link)
{
    switch (mode)
    {
        case AT_SMS_MODE:
            LogMessage(DEBUG_FACTORY, "----------Content----------\r\n");
            LogMessage(DEBUG_FACTORY, (char *)buf);
            LogWrite(DEBUG_FACTORY, "\r\n", 2);
            LogMessage(DEBUG_FACTORY, "------------------------------\r\n");
            break;
        case SHORTMESSAGE_MODE:
            sendShortMessage(telnum, (char *) buf, len);
            break;
        case NETWORK_MODE:
            sendProtocolToServer(link, PROTOCOL_21, (void *)buf);
            break;
        case BLE_MODE:
            bleSend(buf, len);
            break;
        case BLE_DIRECT_MODE:
            bleSendDirect(buf, len);
            break;
        default:
            break;
    }
}


//指令透传至BLE设备
void doSendParamToBle(uint8_t *buf, uint16_t len)
{
    sendMessageWithDifMode(buf, len, BLE_MODE, NULL, DOUBLE_LINK);
}
//接收蓝牙回复的指令
void doSendBleResponToNet(uint8_t *respon, uint16_t len)
{
    reCover123InstructionId();
    sendMessageWithDifMode(respon, len, NETWORK_MODE, NULL, DOUBLE_LINK);
}

/*----------------------------------------------------------------------------------------------------------------------*/
//指令具体解析
void doParamInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t i;
    char message[150];
    char jt808sn[20];
    changeByteArrayToHexString(sysparam.jt808sn, (uint8_t *)jt808sn, 6);
    jt808sn[12] = 0;
    message[0] = 0;
    if (sysparam.protocol == USE_JT808_PROTOCOL)
    {
        sprintf(message, "JT808SN:%s;", jt808sn);
    }
    sprintf(message + strlen(message), "SN:%s;IP:%s:%ld;", sysparam.SN, sysparam.Server, sysparam.ServerPort);
    sprintf(message + strlen(message), "APN:%s;", sysparam.apn);
    sprintf(message + strlen(message), "UTC:%s%d;", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
    switch (sysparam.MODE)
    {
        case MODE1:
        case MODE21:
            if (sysparam.MODE == MODE1)
            {
                sprintf(message + strlen(message), "Mode1:");

            }
            else
            {
                sprintf(message + strlen(message), "Mode21:");

            }
            for (i = 0; i < 5; i++)
            {
                if (sysparam.AlarmTime[i] != 0xFFFF)
                {
                    sprintf(message + strlen(message), " %.2d:%.2d", sysparam.AlarmTime[i] / 60, sysparam.AlarmTime[i] % 60);
                }

            }
            sprintf(message + strlen(message), ", Every %d day;", sysparam.gapDay);
            break;
        case MODE2:
            if (sysparam.gapMinutes == 0)
            {
                //检测到震动，n 秒上送
                sprintf(message + strlen(message), "Mode2: %dS;", sysparam.gpsuploadgap);
            }
            else
            {
                //检测到震动，n 秒上送，未震动，m分钟自动上送
                sprintf(message + strlen(message), "Mode2: %dS,%dM;", sysparam.gpsuploadgap, sysparam.gapMinutes);

            }
            break;

        case MODE3:
            sprintf(message + strlen(message), "Mode3: %d minutes;", sysparam.gapMinutes);
            break;

        case MODE4:
            sprintf(message + strlen(message), "Mode4: %d minutes;", sysparam.gapMinutes);
            break;
        case MODE5:
            if (sysparam.gapMinutes == 0)
            {
                //保持在线，不上送
                sprintf(message + strlen(message), "Mode2: online;");
            }
            else
            {
                //保持在线，不检测震动，每隔m分钟，自动上送
                sprintf(message + strlen(message), "Mode2: %dM;", sysparam.gapMinutes);
            }
            break;
        case MODE23:
            sprintf(message + strlen(message), "Mode23: %d minutes;", sysparam.gapMinutes);
            break;
        case MODEFACTORY:
            sprintf(message + strlen(message),
                    "Mode: Factory;");
            break;
    }
    sprintf(message + strlen(message), "MODE1:%d;MODE2:%ld;", sysparam.mode1startuptime, sysparam.mode2worktime);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}


void doStatusInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    GPSINFO *gpsinfo;
    sprintf(message, "OUT-V=%.2fV,%d%%;", sysinfo.outsidevoltage, getBatteryLevel());
    sprintf(message + strlen(message), "BAT-V=%.2fV;", sysinfo.batvoltage);
    if (sysinfo.GPSStatus)
    {
        gpsinfo = getCurrentGPSInfo();
        sprintf(message + strlen(message), "GPS=%s;", gpsinfo->fixstatus ? "Fixed" : "Invalid");
        sprintf(message + strlen(message), "PDOP=%.2f;", gpsinfo->pdop);
    }
    else
    {
        sprintf(message + strlen(message), "GPS=Close;");
    }

    sprintf(message + strlen(message), "ACC=%s;", getTerminalAccState() > 0 ? "On" : "Off");
    sprintf(message + strlen(message), "SIGNAL=%d;", getModuleRssi());
    sprintf(message + strlen(message), "BATTERY=%s;", getTerminalChargeState() > 0 ? "Charging" : "Uncharged");
    sprintf(message + strlen(message), "LOGIN=%s;", isProtocolReday() ? "Yes" : "No");
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doSNInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
//    char debug[50];
//    char IMEI[15];
//    uint8_t sndata[30];
//    if (item->item_data[1])
//        if (my_strpach(item->item_data[1], "ZTINFO") && my_strpach(item->item_data[2], "SN"))
//        {
//            sprintf(debug, "%s\n", "Update sn number;");
//            LogMessage(DEBUG_ALL, debug);
//            changeHexStringToByteArray(sndata, (uint8_t *)item->item_data[3], strlen(item->item_data[3]) / 2);
//            decryptSN(sndata, IMEI);
//            sprintf(debug, "Decrypt:%s\n", IMEI);
//            jt808CreateSn((uint8_t *)IMEI + 3, 12);
//            LogMessage(DEBUG_ALL, debug);
//            strncpy((char *)sysparam.SN, IMEI, 15);
//            sysparam.SN[15] = 0;
//            paramSaveAll();
//        }
}

uint8_t Servertype = 0;

static void doRecon(void)
{

    sysparam.protocol = Servertype;
    sysparam.jt808isRegister = 0;
    paramSaveAll();
    reConnectServer();
}
void doServerInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[2][0] != 0 && item->item_data[3][0] != 0)
    {
        strcpy((char *)sysparam.Server, item->item_data[2]);
        sysparam.ServerPort = atoi((const char *)item->item_data[3]);
        sprintf(message, "Update domain %s:%ld;", sysparam.Server, sysparam.ServerPort);
        Servertype = atoi(item->item_data[1]);
        paramSaveAll();
        startTimer(30, doRecon, 0);
    }
    else
    {
        sprintf(message, "Update domain fail,please check your param");
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);

}
void doVersionInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    sprintf(message, "Version:%s;Compile:%s %s;", EEPROM_VERSION, __DATE__, __TIME__);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);

}
void doHbtInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "The time of the heartbeat interval is %d seconds;", sysparam.heartbeatgap);
    }
    else
    {
        sysparam.heartbeatgap = atoi((const char *)item->item_data[1]);
        paramSaveAll();
        sprintf(message, "Update the time of the heartbeat interval to %d seconds;", sysparam.heartbeatgap);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doModeInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t workmode, i, j, timecount = 0, gapday = 1;
    uint16_t mode1time[7];
    uint16_t valueofminute;
    char message[200];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current Mode %d", sysparam.MODE);
        sprintf(message + strlen(message), ",gps upload gap %ds,%dm", sysparam.gpsuploadgap, sysparam.gapMinutes);
    }
    else
    {
        workmode = atoi(item->item_data[1]);
        gpsRequestClear(GPS_REQUEST_GPSKEEPOPEN_CTL);
        switch (workmode)
        {
            case 1:
            case 21:
                //内容项如果大于2，说明有时间或日期
                if (item->item_cnt > 2)
                {
                    for (i = 0; i < item->item_cnt - 2; i++)
                    {
                        if (strlen(item->item_data[2 + i]) <= 4 && strlen(item->item_data[2 + i]) >= 3)
                        {
                            valueofminute = atoi(item->item_data[2 + i]);
                            mode1time[timecount++] = valueofminute / 100 * 60 + valueofminute % 100;
                        }
                        else
                        {
                            gapday = atoi(item->item_data[2 + i]);
                        }
                    }

                    for (i = 0; i < (timecount - 1); i++)
                    {
                        for (j = 0; j < (timecount - 1 - i); j++)
                        {
                            if (mode1time[j] > mode1time[j + 1]) //相邻两个元素作比较，如果前面元素大于后面，进行交换
                            {
                                valueofminute = mode1time[j + 1];
                                mode1time[j + 1] = mode1time[j];
                                mode1time[j] = valueofminute;
                            }
                        }
                    }

                }

                for (i = 0; i < 5; i++)
                {
                    sysparam.AlarmTime[i] = 0xFFFF;
                }
                sysparam.AlarmTime[0] = 720;
                //重现写入AlarmTime
                for (i = 0; i < timecount; i++)
                {
                    sysparam.AlarmTime[i] = mode1time[i];
                }
                sysparam.gapDay = gapday;
                if (workmode == MODE1)
                {
                    terminalAccoff();
                    if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
                    {
                        gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    }
                    sysparam.MODE = MODE1;
                }
                else
                {
                    sysparam.MODE = MODE21;
                }
                sprintf(message, "Change to Mode%d,and work on at", workmode);
                for (i = 0; i < timecount; i++)
                {
                    sprintf(message + strlen(message), " %.2d:%.2d", sysparam.AlarmTime[i] / 60, sysparam.AlarmTime[i] % 60);
                }
                sprintf(message + strlen(message), ",every %d day", gapday);
                setNextAlarmTime();
                break;
            case 2:
                //MODE,2,0,0
                //MODE,2,0,M
                //MODE,2,N,M
                sysparam.gpsuploadgap = (uint8_t)atoi((const char *)item->item_data[2]);
                sysparam.gapMinutes = atoi(item->item_data[3]);

                if (sysparam.accctlgnss == 0)
                {
                    gpsRequestSet(GPS_REQUEST_GPSKEEPOPEN_CTL);
                }
                if (sysparam.gpsuploadgap == 0)
                {
                    //运动不自动传GPS
                    sysparam.MODE = MODE5;
                    if (sysparam.gapMinutes == 0)
                    {

                        sprintf(message, "The device switches to mode 2 without uploading the location");
                    }
                    else
                    {
                        sprintf(message, "The device switches to mode 2 and uploads the position every %d minutes all the time",
                                sysparam.gapMinutes);
                    }
                }
                else
                {
                    sysparam.MODE = MODE2;
                    if (sysparam.gapMinutes == 0)
                    {
                        sprintf(message, "The device switches to mode 2 and uploads the position every %d seconds when moving",
                                sysparam.gpsuploadgap);

                    }
                    else
                    {
                        sprintf(message,
                                "The device switches to mode 2 and uploads the position every %d seconds when moving, and every %d minutes when standing still",
                                sysparam.gpsuploadgap, sysparam.gapMinutes);
                    }
                }
                break;
            case 3:
            case 4:
            case 23:
                if (item->item_cnt > 2)
                {
                    sysparam.gapMinutes = atoi(item->item_data[2]);
                    if (sysparam.gapMinutes <= 5)
                    {
                        sysparam.gapMinutes = 5;
                    }
                }
                setNextWakeUpTime();
                if (workmode == 3)
                {
                    terminalAccoff();
                    if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
                    {
                        gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    }
                    gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    sysparam.MODE = MODE3;
                }
                else if (workmode == 4)
                {
                    terminalAccoff();
                    if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
                    {
                        gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    }
                    gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    sysparam.MODE = MODE4;
                }
                else
                {
                    sysparam.MODE = MODE23;
                }
                sprintf(message, "Change to mode %d and update the startup interval time to %d minutes", workmode,
                        sysparam.gapMinutes);
                break;
            default:
                strcpy(message, "Unsupport mode");
                break;
        }
        resetModeStartTime();
        paramSaveAll();
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doTTSInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0)
    {
        strcpy(message, "TTS play error");
    }
    else
    {
        appTTSPlay(item->item_data[1]);
        strcpy(message, "TTS play done");
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doJtInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{}

//(357784086584883)<Local Time:2020-09-23 16:09:33>http://maps.google.com/maps?q=22.58799,113.85864

static DOINSTRUCTIONMODE mode123;
static char telnum123[30];

void dorequestSend123(void)
{
    char message[150];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    GPSINFO *gpsinfo;

    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    sysinfo.flag123 = 0;
    gpsinfo = getCurrentGPSInfo();
    sprintf(message, "(%s)<Local Time:%.2d/%.2d/%.2d %.2d:%.2d:%.2d>http://maps.google.com/maps?q=%f,%f", sysparam.SN, \
            year, month, date, hour, minute, second, gpsinfo->latitude, gpsinfo->longtitude);
    reCover123InstructionId();
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode123, telnum123, NORMAL_LINK);
}

void do123Instruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    mode123 = mode;
    if (telnum != NULL)
    {
        strcpy(telnum123, telnum);
    }
    if (sysparam.poitype == 0)
    {
        sysinfo.lbsrequest = 1;
        LogMessage(DEBUG_ALL, "Only LBS reporting\n");
    }
    else if (sysparam.poitype == 1)
    {
        sysinfo.lbsrequest = 1;
        gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
        LogMessage(DEBUG_ALL, "LBS and GPS reporting\n");


    }
    else
    {
        sysinfo.lbsrequest = 1;
        sysinfo.wifirequest = 1;
        gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
        LogMessage(DEBUG_ALL, "LBS ,WIFI and GPS reporting\n");
    }
    sysinfo.flag123 = 1;
    save123InstructionId();
    resetModeStartTime();

}

void doAPNInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[200];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "APN:%s,APN User:%s,APN Password:%s", sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
    }
    else
    {
        if (item->item_data[1][0] != 0 && item->item_cnt >= 2)
        {
            sysparam.apn[0] = 0;
            sysparam.apnuser[0] = 0;
            sysparam.apnpassword[0] = 0;
            strcpy((char *)sysparam.apn, item->item_data[1]);
        }
        if (item->item_data[2][0] != 0 && item->item_cnt >= 3)
        {
            strcpy((char *)sysparam.apnuser, item->item_data[2]);

        }
        if (item->item_data[3][0] != 0 && item->item_cnt >= 4)
        {
            strcpy((char *)sysparam.apnpassword, item->item_data[3]);

        }
        paramSaveAll();
        sprintf(message, "Update APN:%s,APN User:%s,APN Password:%s", sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
    }

    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

static void reConnectForUpdate(void)
{
    sysinfo.updateStatus = 1;
    updateSystemLedStatus(SYSTEM_LED_UPDATE, 1);
    reConnectServer();
    LogMessage(DEBUG_ALL, "reConnectForUpdate\r\n");
}

void doUPSInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t object = 0;
    char message[200];

    if (item->item_data[1][0] != 0 && item->item_data[2][0] != 0)
    {
        sysparam.updateServerPort = atoi(item->item_data[2]);
        strcpy((char *)sysparam.updateServer, item->item_data[1]);
        paramSaveAll();
    }
    networkCtl(1);
    startTimer(50, reConnectForUpdate, 0);
    if (item->item_data[3][0] != 0)
    {
        object = atoi(item->item_data[3]);
    }
    object = 0;
    updateUISInit(object);
    sprintf(message, "The device will download the firmware for %s from %s:%d in 5 seconds",
            object > 0 ? "MCU" : "MODULE", sysparam.updateServer,
            sysparam.updateServerPort);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doLOWWInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sysinfo.lowvoltage = sysparam.lowvoltage / 10.0;
        sprintf(message, "The low voltage param is %.1fV", sysinfo.lowvoltage);

    }
    else
    {
        sysparam.lowvoltage = atoi(item->item_data[1]);
        sysinfo.lowvoltage = sysparam.lowvoltage / 10.0;
        paramSaveAll();
        sprintf(message, "When the voltage is below %.1fV, the device will upload a alarm", sysinfo.lowvoltage);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doLEDInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "LED was %s", sysparam.ledctrl ? "open" : "close");

    }
    else
    {
        sysparam.ledctrl = atoi(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "%s", sysparam.ledctrl ? "LED ON" : "LED OFF");
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}


void doPOITYPEInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        switch (sysparam.poitype)
        {
            case 0:
                strcpy(message, "Current poitype is only LBS reporting");
                break;
            case 1:
                strcpy(message, "Current poitype is LBS and GPS reporting");
                break;
            case 2:
                strcpy(message, "Current poitype is LBS ,WIFI and GPS reporting");
                break;
        }
    }
    else
    {
        if (strstr(item->item_data[1], "AUTO") != 0)
        {
            sysparam.poitype = 2;
        }
        else
        {
            sysparam.poitype = atoi(item->item_data[1]);
        }
        switch (sysparam.poitype)
        {
            case 0:
                strcpy(message, "Set to only LBS reporting");
                break;
            case 1:
                strcpy(message, "Set to LBS and GPS reporting");
                break;
            case 2:
                strcpy(message, "Set to LBS ,WIFI and GPS reporting");
                break;
            default:
                sysparam.poitype = 2;
                strcpy(message, "Unknow type,default set to LBS ,WIFI and GPS reporting");
                break;
        }
        paramSaveAll();

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

static void resetCallBack(void)
{
    appSystemReset();
}

void doResetInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[50];
    sprintf(message, "System will reset after 5 seconds");
    paramSaveAll();
    startTimer(50, resetCallBack, 0);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doUTCInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "System time zone:UTC %s%d", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
        updateRTCtimeRequest();
    }
    else
    {
        sysparam.utc = atoi(item->item_data[1]);
        updateRTCtimeRequest();
        LogPrintf(DEBUG_ALL, "utc=%d\n", sysparam.utc);
        if (sysparam.utc < -12 || sysparam.utc > 12)
            sysparam.utc = 8;
        paramSaveAll();
        sprintf(message, "Update the system time zone to UTC %s%d", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}
void doAlarmModeInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "The light-sensing alarm function was %s", sysparam.lightAlarm ? "enable" : "disable");
    }
    else
    {
        if (my_strpach(item->item_data[1], "L1"))
        {
            sysparam.lightAlarm = 1;
            sprintf(message, "Enables the light-sensing alarm function successfully");
        }
        else if (my_strpach(item->item_data[1], "L0"))
        {
            sysparam.lightAlarm = 0;
            sprintf(message, "Disable the light-sensing alarm function successfully");

        }
        else
        {
            sysparam.lightAlarm = 1;
            sprintf(message, "Unknow cmd,enable the light-sensing alarm function by default");
        }
        paramSaveAll();

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}
void doDebugInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[150];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;


    message[0] = 0;
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
        sprintf(message, "Time:%.2d/%.2d/%.2d %.2d:%.2d:%.2d;", year, month, date, hour, minute, second);
        sprintf(message + strlen(message), "Sysrun:%.2ld:%.2ld:%.2ld;gpsrequest:%02lX;gpslast:%.2ld:%.2ld:%.2ld;",
                sysinfo.System_Tick / 3600, sysinfo.System_Tick % 3600 / 60, sysinfo.System_Tick % 60, sysinfo.GPSRequest,
                sysinfo.gpsUpdatetick / 3600, sysinfo.gpsUpdatetick % 3600 / 60, sysinfo.gpsUpdatetick % 60);
        sprintf(message + strlen(message), "Idle:%d;", systemIsIdle());
    }
    else
    {
        if (my_strpach(item->item_data[1], "DEBUGUART"))
        {
            if (item->item_data[2][0] == '1')
            {
                strcpy(message, "Debug:open uart1");
            }
            else if (item->item_data[2][0] == '0')
            {
                strcpy(message, "Debug:close uart1");
            }
        }
        else if (my_strpach(item->item_data[1], "MODECNTCLEAR"))
        {
            sysparam.mode1startuptime = 0;
            sysparam.mode2worktime = 0;
            paramSaveAll();
            strcpy(message, "Debug:clear mode count");
        }

        else if (my_strpach(item->item_data[1], "BLEADVOPEN"))
        {
            strcpy(message, "Debug:open ble boradcast");
        }
        else if (my_strpach(item->item_data[1], "BLEADVCLOSE"))
        {
            strcpy(message, "Debug:close ble boradcast");
        }
        else if (my_strpach(item->item_data[1], "MUSIC"))
        {
            nwy_audio_file_play(item->item_data[2]);
            strcpy(message, "Debug:play music");
        }
        else if (my_strpach(item->item_data[1], "CLEARMILEAGE"))
        {
            sysparam.mileage = 0;
            paramSaveAll();
            strcpy(message, "Debug:clear mileage ok");
        }
        else
        {
            strcpy(message, "Debug:Unknow cmd");
        }
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doACCCTLGNSSInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "%s", sysparam.accctlgnss ? "GPS is automatically controlled by the program" :
                "The GPS is always be on");
    }
    else
    {
        sysparam.accctlgnss = (uint8_t)atoi((const char *)item->item_data[1]);
        if (sysparam.MODE == MODE2)
        {
            if (sysparam.accctlgnss == 0)
            {
                gpsRequestSet(GPS_REQUEST_GPSKEEPOPEN_CTL);
            }
            else
            {
                gpsRequestClear(GPS_REQUEST_GPSKEEPOPEN_CTL);
            }
        }
        sprintf(message, "%s", sysparam.accctlgnss ? "GPS will be automatically controlled by the program" :
                "The GPS will always be on");
        paramSaveAll();
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);

}



void doPdopInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current podp filter is %.2f", sysparam.pdop / 100.0);
    }
    else
    {
        sysparam.pdop = atoi(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "Update podp filter to %.2f", sysparam.pdop / 100.0);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}



void doSetblemacInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{}


void doBFInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    terminalDefense();
    sysparam.bf = 1;
    paramSaveAll();
    strcpy(message, "BF OK");
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}


void doCFInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    terminalDisarm();
    sysparam.bf = 0;
    paramSaveAll();
    strcpy(message, "CF OK");
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doFactoryTestInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t total, i;
    GPSINFO *gpsinfo;
    char message[300];
    message[0] = 0;
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        //感光检测
        //        sprintf(message, "Light:%s;", LDRDET ? "darkness" : "brightness");
        //acc线检测
        sprintf(message + strlen(message), "CSQ:%d;", getModuleRssi());
        //Gsensor检测
        if (read_gsensor_id() != 0)
        {
            sprintf(message + strlen(message), "Gsensor:ERROR;");
        }
        else
        {
            sprintf(message + strlen(message), "Gsensor:OK;");
        }
        //电压检测
        getBatVoltage();
        sprintf(message + strlen(message), "vBat:%.3fV;", sysinfo.outsidevoltage);
        //Apn
        sprintf(message + strlen(message), "APN:%s;APN User:%s;APN Password:%s;", sysparam.apn, sysparam.apnuser,
                sysparam.apnpassword);
        //gps
        gpsinfo = getCurrentGPSInfo();
        total = sizeof(gpsinfo->gpsCn);
        sprintf(message + strlen(message), "FixMode:%d;", gpsinfo->fixmode);
        sprintf(message + strlen(message), "High:%.2f;", gpsinfo->hight);
        sprintf(message + strlen(message), "GPS CN:");

        for (i = 0; i < total; i++)
        {
            if (gpsinfo->gpsCn[i] != 0)
            {
                sprintf(message + strlen(message), "%d,", gpsinfo->gpsCn[i]);
            }
        }

        sprintf(message + strlen(message), ";BeiDou CN:");
        total = sizeof(gpsinfo->beidouCn);

        for (i = 0; i < total; i++)
        {
            if (gpsinfo->beidouCn[i] != 0)
            {
                sprintf(message + strlen(message), "%d,", gpsinfo->beidouCn[i]);
            }
        }
        sprintf(message + strlen(message), ";");
    }
    else
    {

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doFenceInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "The static drift fence is %d meters", sysparam.fence);
    }
    else
    {
        sysparam.fence = atol(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "Set the static drift fence to %d meters", sysparam.fence);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doFactoryInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (strstr(item->item_data[1], "ZTINFO8888") != NULL)
    {
        sprintf(message, "Factory all successfully");
        paramDefaultInit(0);
    }
    else
    {
        sprintf(message, "Factory Settings restored successfully");
        paramDefaultInit(1);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

//BLEUNBIND,123456,123456,123456,123456#
void doBleUnbindInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{}

void doBLEENInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "BLE was %s", sysparam.bleen ? "Enable" : "Disable");
    }
    else
    {
        sysparam.bleen = atol(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "%s the ble function", sysparam.bleen ? "Enable" : "Disable");
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doProtectVolInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current protect voltage is %.2f V", sysparam.protectVoltage);
    }
    else
    {
        sysparam.protectVoltage = atoi(item->item_data[1]) / 10.0;
        sprintf(message, "Update protect voltage to %.2f V", sysparam.protectVoltage);
        paramSaveAll();
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doAccuracyInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{}

static uint8_t searchSosNumber(char *sosnumber)
{
    LogPrintf(DEBUG_ALL, "Search %s\n", sosnumber);
    if (my_strpach((char *)sysparam.sosnumber1, (char *)sosnumber))
    {
        sysparam.sosnumber1[0] = 0;
        LogPrintf(DEBUG_ALL, "Delete sos num 1 %s\n", sysparam.sosnumber1);
        return 1;
    }
    if (my_strpach((char *)sysparam.sosnumber2, (char *)sosnumber))
    {
        sysparam.sosnumber2[0] = 0;
        LogPrintf(DEBUG_ALL, "Delete sos num 2 %s\n", sysparam.sosnumber2);
        return 1;
    }
    if (my_strpach((char *)sysparam.sosnumber3, (char *)sosnumber))
    {
        sysparam.sosnumber3[0] = 0;
        LogPrintf(DEBUG_ALL, "Delete sos num 3 %s\n", sysparam.sosnumber3);
        return 1;
    }
    return 0;
}

void doSOSInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[200];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "SOS number : %s,", (sysparam.sosnumber1[0] == 0) ? "NULL" : (char *)sysparam.sosnumber1);
        sprintf(message + strlen(message), "%s,", (sysparam.sosnumber2[0] == 0) ? "NULL" : (char *)sysparam.sosnumber2);
        sprintf(message + strlen(message), "%s;", (sysparam.sosnumber3[0] == 0) ? "NULL" : (char *)sysparam.sosnumber3);
    }
    else
    {
        if (item->item_data[1][0] == 'A' || item->item_data[1][0] == 'a')
        {
            if (item->item_data[2][0] != 0)
            {
                strcpy((char *)sysparam.sosnumber1, item->item_data[2]);
            }
            if (item->item_data[3][0] != 0)
            {
                strcpy((char *)sysparam.sosnumber2, item->item_data[3]);
            }
            if (item->item_data[4][0] != 0)
            {
                strcpy((char *)sysparam.sosnumber3, item->item_data[4]);
            }
            sprintf(message, "ADD OK ,%s,", (sysparam.sosnumber1[0] == 0) ? "NULL" : (char *)sysparam.sosnumber1);
            sprintf(message + strlen(message), "%s,", (sysparam.sosnumber2[0] == 0) ? "NULL" : (char *)sysparam.sosnumber2);
            sprintf(message + strlen(message), "%s;", (sysparam.sosnumber3[0] == 0) ? "NULL" : (char *)sysparam.sosnumber3);

        }
        else if (item->item_data[1][0] == 'D' || item->item_data[1][0] == 'd')
        {
            if (item->item_data[2][0] != 0)
            {
                if (searchSosNumber(item->item_data[2]))
                {
                    sprintf(message, "Delete %s OK;", item->item_data[2]);
                }
                else
                {
                    sprintf(message, "Delete %s fail;", item->item_data[2]);
                }
            }
            if (item->item_data[3][0] != 0)
            {
                if (searchSosNumber(item->item_data[3]))
                {
                    sprintf(message + strlen(message), "Delete %s OK;", item->item_data[3]);

                }
                else
                {
                    sprintf(message + strlen(message), "Delete %s fail;", item->item_data[3]);

                }
            }
            if (item->item_data[4][0] != 0)
            {
                if (searchSosNumber(item->item_data[4]))
                {
                    sprintf(message + strlen(message), "Delete %s OK;", item->item_data[4]);

                }
                else
                {
                    sprintf(message + strlen(message), "Delete %s fail;", item->item_data[4]);

                }
            }

        }
        else
        {
            sprintf(message, "Unknow option\r\n");
        }
    }
    paramSaveAll();
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}


void doProtocolInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current protocol is %s protocol", sysparam.protocol == USE_JT808_PROTOCOL ? "jt808" : "normal");
    }
    else
    {
        sysparam.protocol = atoi(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "Use %s protocol", sysparam.protocol == USE_JT808_PROTOCOL ? "jt808" : "normal");
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doJT808SNInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    char senddata[40];
    uint8_t snlen;
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        changeByteArrayToHexString(sysparam.jt808sn, (uint8_t *)senddata, 6);
        senddata[12] = 0;
        sprintf(message, "JT808SN:%s", senddata);
    }
    else
    {
        snlen = strlen(item->item_data[1]);
        if (snlen > 12)
        {
            sprintf(message, "SN number too long");
        }
        else
        {
            jt808CreateSn((uint8_t *)item->item_data[1], snlen);
            changeByteArrayToHexString(sysparam.jt808sn, (uint8_t *)senddata, 6);
            senddata[12] = 0;
            sprintf(message, "Update SN:%s", senddata);

        }
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}
void doRelayInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == '1')
    {
        strcpy(message, "Relay on success");
    }
    else if (item->item_data[1][0] == '0')
    {
        strcpy(message, "Relay off success");
    }
    else
    {
        strcpy(message, "Unknow param");
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}
void doEcfgInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current overspeed:%dkm/h,replay time:%ds,overspeed time:%ds;", sysparam.overspeed,
                sysparam.rettsPlaytime, sysparam.overspeedTime);
    }
    else
    {
        sysparam.overspeed = atoi(item->item_data[1]);
        sysparam.rettsPlaytime = atoi(item->item_data[2]);
        sysparam.overspeedTime = atoi(item->item_data[3]);
        if (sysparam.overspeed < 10 && sysparam.overspeed > 0)
            sysparam.overspeed = 20;
        if (sysparam.rettsPlaytime < 10)
            sysparam.rettsPlaytime = 30;
        if (sysparam.overspeedTime < 3)
            sysparam.overspeedTime = 3;
        paramSaveAll();
        sprintf(message, "Set overspeed:%dkm/h,replay time:%ds,overspeed time:%ds;", sysparam.overspeed, sysparam.rettsPlaytime,
                sysparam.overspeedTime);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}
void doVolInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sysparam.vol = nwy_audio_get_handset_vol();
        sprintf(message, "Vol[0,100]:%d", sysparam.vol);
    }
    else
    {
        sysparam.vol = atoi(item->item_data[1]);
        if (sysparam.vol > 100 || sysparam.vol < 10)
        {
            sysparam.vol = 60;
        }
        paramSaveAll();
        sprintf(message, "Set VOL:%d", sysparam.vol);
        nwy_audio_set_handset_vol(sysparam.vol);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doAlarmMusicInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "AlarmMusic:%d", sysparam.alarmMusicIndex);
    }
    else
    {
        sysparam.alarmMusicIndex = atoi(item->item_data[1]);
        if (sysparam.alarmMusicIndex >= 4)
        {
            sysparam.alarmMusicIndex = 0;
        }
        paramSaveAll();
        sprintf(message, "Use alarm music %d", sysparam.alarmMusicIndex);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doFactoryModeInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    sysparam.bleen = 1;
    sysparam.MODE = MODEFACTORY;
    paramSaveAll();
    strcpy(message, "Enter factorymode success");
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doAccdetmodeInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        switch (sysparam.accdetmode)
        {
            case ACCDETMODE0:
                sprintf(message, "The device is use acc wire to determine whether ACC is ON or OFF.");
                break;
            case ACCDETMODE1:
                sprintf(message, "The device is use acc wire first and voltage second to determine whether ACC is ON or OFF.");
                break;
            case ACCDETMODE2:
                sprintf(message, "The device is use acc wire first and gsensor second to determine whether ACC is ON or OFF.");
                break;
        }

    }
    else
    {
        sysparam.accdetmode = atoi(item->item_data[1]);
        switch (sysparam.accdetmode)
        {
            case ACCDETMODE0:
                sprintf(message, "The device is use acc wire to determine whether ACC is ON or OFF.");
                break;
            //            case ACCDETMODE1:
            //                sprintf(message, "The device is use acc wire first and voltage second to determine whether ACC is ON or OFF.");
            //                break;
            case ACCDETMODE2:
                sprintf(message, "The device is use acc wire first and gsensor second to determine whether ACC is ON or OFF.");
                break;
            default:
                sysparam.accdetmode = ACCDETMODE0;
                sprintf(message,
                        "Unknow mode,Using acc wire to determine whether ACC is ON or OFF by default");
                break;
        }
        paramSaveAll();
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}
void doBatbalTableInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[200];

    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "[%d,%d):0%%~20%%,[%d,%d):20%%~40%%,[%d,%d):40%%~60%%,[%d,%d):60%%~80%%,[%d,%d):80%%~100%%", \
                sysparam.volTable1, sysparam.volTable2, sysparam.volTable2, sysparam.volTable3, sysparam.volTable3, sysparam.volTable4,
                sysparam.volTable4, sysparam.volTable5, sysparam.volTable5, sysparam.volTable6);
    }
    else
    {
        if (item->item_cnt == 7)
        {
            sysparam.volTable1 = atoi(item->item_data[1]);
            sysparam.volTable2 = atoi(item->item_data[2]);
            sysparam.volTable3 = atoi(item->item_data[3]);
            sysparam.volTable4 = atoi(item->item_data[4]);
            sysparam.volTable5 = atoi(item->item_data[5]);
            sysparam.volTable6 = atoi(item->item_data[6]);
            sprintf(message, "Update:[%d,%d):0%%~20%%,[%d,%d):20%%~40%%,[%d,%d):40%%~60%%,[%d,%d):60%%~80%%,[%d,%d):80%%~100%%", \
                    sysparam.volTable1, sysparam.volTable2, sysparam.volTable2, sysparam.volTable3, sysparam.volTable3, sysparam.volTable4,
                    sysparam.volTable4, sysparam.volTable5, sysparam.volTable5, sysparam.volTable6);
            paramSaveAll();
        }
        else
        {
            strcpy(message, "Please enter 6 group of voltage param");
        }
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);

}

void doSirenonInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t i;
    char message[50];
    strcpy(message, "Siren on ok");
    for (i = 0; i < 5; i++)
    {
        appSendThreadEvent(PLAY_MUSIC_EVENT, THREAD_PARAM_SIREN);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

void doSetAgpsInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[200];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Agps:%s,%d,%s,%s", sysparam.agpsServer, sysparam.agpsPort, sysparam.agpsUser, sysparam.agpsPswd);
    }
    else
    {
        if (item->item_data[1][0] != 0)
        {
            strcpy((char *)sysparam.agpsServer, item->item_data[1]);
        }
        if (item->item_data[2][0] != 0)
        {
            sysparam.agpsPort = atoi(item->item_data[2]);
        }
        if (item->item_data[3][0] != 0)
        {
            strcpy((char *)sysparam.agpsUser, item->item_data[3]);
        }
        if (item->item_data[4][0] != 0)
        {
            strcpy((char *)sysparam.agpsPswd, item->item_data[4]);
        }
        paramSaveAll();
        sprintf(message, "Update Agps info:%s,%d,%s,%s", sysparam.agpsServer, sysparam.agpsPort, sysparam.agpsUser,
                sysparam.agpsPswd);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);
}

/*----------------------------------------------------------------------------------------------------------------------*/
static void doUnsupportCmd(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[50];
    if (mode == SHORTMESSAGE_MODE)
        return ;
    snprintf(message, 50, "Unsupport CMD:%s;", item->item_data[0]);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum, NORMAL_LINK);

}
//指令匹配函数
static void doInstruction(int16_t cmdid, ITEM *item, DOINSTRUCTIONMODE mode, char *telnum, void *param)
{
    switch (cmdid)
    {
        case PARAM_INS:
            doParamInstruction(item, mode, telnum);
            break;
        case STATUS_INS:
            doStatusInstruction(item, mode, telnum);
            break;
        case VERSION_INS:
            doVersionInstruction(item, mode, telnum);
            break;
        case SN_INS:
            doSNInstruction(item, mode, telnum);
            break;
        case SERVER_INS:
            doServerInstruction(item, mode, telnum);
            break;
        case HBT_INS:
            doHbtInstruction(item, mode, telnum);
            break;
        case MODE_INS:
            doModeInstruction(item, mode, telnum);
            break;
        case TTS_INS:
            doTTSInstruction(item, mode, telnum);
            break;
        case JT_INS:
            doJtInstruction(item, mode, telnum);
            break;
        case POSITION_INS:
            do123Instruction(item, mode, telnum);
            break;
        case APN_INS:
            doAPNInstruction(item, mode, telnum);
            break;
        case UPS_INS:
            doUPSInstruction(item, mode, telnum);
            break;
        case LOWW_INS:
            doLOWWInstruction(item, mode, telnum);
            break;
        case LED_INS:
            doLEDInstruction(item, mode, telnum);
            break;
        case POITYPE_INS:
            doPOITYPEInstruction(item, mode, telnum);
            break;
        case RESET_INS:
            doResetInstruction(item, mode, telnum);
            break;
        case UTC_INS:
            doUTCInstruction(item, mode, telnum);
            break;
        case ALARMMODE_INS:
            doAlarmModeInstrucion(item, mode, telnum);
            break;
        case DEBUG_INS:
            doDebugInstrucion(item, mode, telnum);
            break;
        case ACCCTLGNSS_INS:
            doACCCTLGNSSInstrucion(item, mode, telnum);
            break;
        case PDOP_INS:
            doPdopInstrucion(item, mode, telnum);
            break;
        case SETBLEMAC_INS:
            doSetblemacInstrucion(item, mode, telnum);
            break;
        case BF_INS:
            doBFInstruction(item, mode, telnum);
            break;
        case CF_INS:
            doCFInstruction(item, mode, telnum);
            break;
        case FACTORYTEST_INS:
            doFactoryTestInstruction(item, mode, telnum);
            break;
        case FENCE_INS:
            doFenceInstrucion(item, mode, telnum);
            break;
        case FACTORY_INS:
            doFactoryInstrucion(item, mode, telnum);
            break;
        case BLEUNBIND_INS:
            doBleUnbindInstrucion(item, mode, telnum);
            break;
        case BLEEN_INS:
            doBLEENInstrucion(item, mode, telnum);
            break;
        case PROTECTVOL_INS:
            doProtectVolInstrucion(item, mode, telnum);
            break;
        case ACCURACY_INS:
            doAccuracyInstrucion(item, mode, telnum);
            break;
        case SOS_INS:
            doSOSInstruction(item, mode, telnum);
            break;
        case PROTOCOL_INS:
            doProtocolInstruction(item, mode, telnum);
            break;
        case JT808SN_INS:
            doJT808SNInstrucion(item, mode, telnum);
            break;
        case RELAY_INS:
            doRelayInstrucion(item, mode, telnum);
            break;
        case ECFG_INS:
            doEcfgInstrucion(item, mode, telnum);
            break;
        case VOL_INS:
            doVolInstrucion(item, mode, telnum);
            break;
        case ALARMMUSIC_INS:
            doAlarmMusicInstrucion(item, mode, telnum);
            break;
        case FACTORYMODE_INS:
            doFactoryModeInstrucion(item, mode, telnum);
            break;
        case ACCDETMODE_INS:
            doAccdetmodeInstruction(item, mode, telnum);
            break;
        case BATVOLTABLE_INS:
            doBatbalTableInstruction(item, mode, telnum);
            break;
        case SIRENON_INS:
            doSirenonInstrucion(item, mode, telnum);
            break;
        case SETAGPS_INS:
            doSetAgpsInstruction(item, mode, telnum);
            break;
        default:
            doUnsupportCmd(item, mode, telnum);
            break;
    }
}
/*----------------------------------------------------------------------------------------------------------------------*/
//指令解析函数
void instructionParser(uint8_t *str, uint16_t len, DOINSTRUCTIONMODE mode, char *telnum, void *param)
{
    ITEM item;
    int16_t cmdid, data_len, i;
    char debug[100];

    if (param != NULL)
    {
        LogMessage(DEBUG_ALL, "SendtoBle\n");
        save123InstructionId();
        doSendParamToBle(str, len);
        return ;
    }

    parserInstructionToItem(&item, str, len);
    if (item.item_cnt > 0)
    {
        data_len = strlen(item.item_data[0]);
        for (i = 0; i < data_len; i++)
        {
            if (item.item_data[0][i] >= 'a' && item.item_data[0][i] <= 'z')
            {
                item.item_data[0][i] = item.item_data[0][i] - 'a' + 'A';
            }
        }

        sprintf(debug, "Instruction==>%s\n", item.item_data[0]);
        LogMessage(DEBUG_ALL, debug);
    }
    cmdid = getInstructionid((uint8_t *)item.item_data[0]);
    doInstruction(cmdid, &item, mode, telnum, param);
}


