#include "app_instructioncmd.h"
#include "app_sys.h"
#include "stdio.h"
#include "stdlib.h"
#include "app_protocol.h"
#include "app_task.h"
#include "app_param.h"
#include "app_task.h"
#include "app_port.h"
#include "app_gps.h"
#include "app_net.h"
#include "app_sn.h"
#include "app_kernal.h"
#include "app_ble.h"
#include "app_mir3da.h"
#include "aes.h"
#include "app_jt808.h"

static instructionMode_e mode123;
static uint8_t link123;
static char telnum123[30];
static uint8_t serverType;
const instruction_s instructiontable[] =
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
    {BF_INS, "BF"},
    {CF_INS, "CF"},
    {FACTORYTEST_INS, "FACTORYTEST"},
    {FENCE_INS, "FENCE"},
    {FACTORY_INS, "FACTORY"},
    {RELAY_INS, "RELAY"},
    {VOL_INS, "VOL"},
    {ACCDETMODE_INS, "ACCDETMODE"},
    {SETAGPS_INS, "SETAGPS"},
    {BLEEN_INS, "BLEEN"},
    {SOS_INS, "SOS"},
    {FCG_INS, "FCG"},
    {JT808SN_INS, "JT808SN"},
    {JT808PARAM_INS, "JT808PARAM"},
    {HIDESERVER_INS, "HIDESERVER"},
    {SETBLEMAC_INS, "SETBLEMAC"},
    {READPARAM_INS, "READPARAM"},
    {SETBLEPARAM_INS, "SETBLEPARAM"},
    {SETBLEWARNPARAM_INS, "SETBLEWARNPARAM"},
    {RELAYFUN_INS, "RELAYFUN"},
    {RELAYSPEED_INS, "RELAYSPEED"},
    {BLESERVER_INS, "BLESERVER"},
    {SN_INS, "*"},
};

/**************************************************
@bref		获取指令ID
@param
@return
@note
**************************************************/

static int getInstructionid(uint8_t *cmdstr)
{
    uint16_t i = 0;
    if (cmdstr == NULL)
    {
        LogMessage(DEBUG_ALL, "getInstructionid==>no cmd");
        return -1;
    }
    for (i = 0; i < sizeof(instructiontable) / sizeof(instructiontable[0]); i++)
    {
        if (mycmdPatch(cmdstr, (uint8_t *)instructiontable[i].cmdstr) != 0)
            return instructiontable[i].cmdid;
    }
    return -1;
}

static void bleSendEncryptData(uint8_t *buf, uint16_t len)
{
    uint8_t enclen;
    char respon[300];
    encryptData(respon, &enclen, respon, len);
    bleServSendData(respon, enclen);
}


/**************************************************
@bref		指令匹配，2条指令必须完全匹配
@param
@return
@note
**************************************************/

static void sendMessageWithDifMode(uint8_t *buf, uint16_t len, instructionMode_e mode, char *telnum, uint8_t link)
{
    if (len == 0 || buf == NULL)
        return;
    switch (mode)
    {
        case SERIAL_MODE:
            LogMessage(DEBUG_FACTORY, "----------Content----------");
            LogMessageWL(DEBUG_FACTORY, (char *)buf, len);
            LogMessage(DEBUG_FACTORY, "-----------------------------");
            break;
        case MESSAGE_MODE:
            portSendSms(telnum, (char *) buf, len);
            break;
        case NETWORK_MODE:
            sendProtocolToServer(link, PROTOCOL_21, (void *)buf);
            break;
        case JT808_MODE:
            jt808MessageSend(buf, len);
            break;
        case BLE_MODE:
            LogMessage(DEBUG_FACTORY, "-----------BLEMODE-----------");
            LogMessageWL(DEBUG_FACTORY, (char *)buf, len);
            LogMessage(DEBUG_FACTORY, "-----------------------------");
            bleSendEncryptData(buf, len);
            break;
        default:
            break;
    }
}

static void doParamInstruction(ITEM *item, char *message)
{
    uint8_t i;
    uint8_t debugMsg[15];
    if (sysparam.protocol == JT808_PROTOCOL_TYPE)
    {
        changeByteArrayToHexString(sysparam.jt808sn, debugMsg, 6);
        debugMsg[12] = 0;
        sprintf(message + strlen(message), "JT808SN:%s;SN:%s;IP:%s:%d;", debugMsg, sysparam.SN, sysparam.jt808Server,
                sysparam.jt808Port);
    }
    else
    {
        sprintf(message + strlen(message), "SN:%s;IP:%s:%d;", sysparam.SN, sysparam.Server, sysparam.ServerPort);
    }
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
            if (sysparam.gpsuploadgap != 0)
            {
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
            }
            else
            {
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
            }
            break;

        case MODE3:
            sprintf(message + strlen(message), "Mode3: %d minutes;", sysparam.gapMinutes);
            break;
        case MODE23:
            sprintf(message + strlen(message), "Mode23: %d minutes;", sysparam.gapMinutes);
            break;
    }
    sprintf(message + strlen(message), "StartUp:%d;RunTime:%ld;", sysparam.startUpCnt, sysparam.runTime);
}

static void doStatusInstruction(ITEM *item, char *message)
{
    gpsinfo_s *gpsinfo;
    sprintf(message, "OUT-V=%.2fV;BAT-V=%.2fV;", sysinfo.outsideVol, sysinfo.batteryVol);
    if (sysinfo.gpsOnoff)
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
    sprintf(message + strlen(message), "SIGNAL=%d;", portGetModuleRssi());
    sprintf(message + strlen(message), "BATTERY=%s;", getTerminalChargeState() > 0 ? "Charging" : "Uncharged");
    sprintf(message + strlen(message), "LOGIN=%s;", serverIsReady() ? "Yes" : "No");
}
static void doVersionInstruction(ITEM *item, char *message)
{
    sprintf(message, "Version:%s;Compile:%s %s;", EEPROM_VERSION, __DATE__, __TIME__);

}

static void doSNInstruction(ITEM *item, char *message)
{
    char IMEI[15];
    uint8_t sndata[30];
    if (item->item_data[1])
    {
        if (my_strpach(item->item_data[1], "ZTINFO") && my_strpach(item->item_data[2], "SN"))
        {
            changeHexStringToByteArray(sndata, (uint8_t *)item->item_data[3], strlen(item->item_data[3]) / 2);
            decryptSN(sndata, IMEI);
            LogPrintf(DEBUG_ALL, "Decrypt: %s", IMEI);
            strncpy((char *)sysparam.SN, IMEI, 15);
            sysparam.SN[15] = 0;
            jt808CreateSn(sysparam.jt808sn, (uint8_t *)sysparam.SN + 3, 12);
            sysparam.jt808isRegister = 0;
            sysparam.jt808AuthLen = 0;
            jt808RegisterLoginInfo(sysparam.jt808sn, sysparam.jt808isRegister, sysparam.jt808AuthCode, sysparam.jt808AuthLen);
            paramSaveAll();
        }
    }
    sprintf(message, "Update Sn : %s", sysparam.SN);
}

static void serverChangeCallBack(void)
{
    if (serverType == JT808_PROTOCOL_TYPE)
    {
        sysparam.protocol = JT808_PROTOCOL_TYPE;
        sysparam.jt808isRegister = 0;
        jt808serverReconnect();
    }
    else
    {
        sysparam.protocol = ZT_PROTOCOL_TYPE;
        serverReconnect();
    }
    paramSaveAll();
}


static void doServerInstruction(ITEM *item, char *message)
{
    if (item->item_data[2][0] != 0 && item->item_data[3][0] != 0)
    {
        serverType = atoi(item->item_data[1]);
        if (serverType == JT808_PROTOCOL_TYPE)
        {
            strncpy((char *)sysparam.jt808Server, item->item_data[2], 50);
            stringToLowwer(sysparam.jt808Server, strlen(sysparam.jt808Server));
            sysparam.jt808Port = atoi((const char *)item->item_data[3]);
            sprintf(message, "Update jt808 domain %s:%d;", sysparam.jt808Server, sysparam.jt808Port);

        }
        else
        {
            strncpy((char *)sysparam.Server, item->item_data[2], 50);
            stringToLowwer(sysparam.Server, strlen(sysparam.Server));
            sysparam.ServerPort = atoi((const char *)item->item_data[3]);
            sprintf(message, "Update domain %s:%d;", sysparam.Server, sysparam.ServerPort);
        }
        startTimer(30, serverChangeCallBack, 0);
    }
    else
    {
        sprintf(message, "Update domain fail,please check your param");
    }

}

static void doHbtInstruction(ITEM *item, char *message)
{
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
}
static void doModeInstruction(ITEM *item, char *message)
{
    uint8_t workmode, i, j, timecount = 0, gapday = 1;
    uint16_t mode1time[7];
    uint16_t valueofminute;
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current Mode %d", sysparam.MODE);
        sprintf(message + strlen(message), ",gps upload gap %ds,%dm", sysparam.gpsuploadgap, sysparam.gapMinutes);
    }
    else
    {
        workmode = atoi(item->item_data[1]);
        gpsRequestClear(GPS_REQ_KEEPON);
        sysResetStartRun();
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
                    if (gpsRequestGet(GPS_REQ_ACC))
                    {
                        gpsRequestClear(GPS_REQ_ACC);
                    }
                    sysparam.MODE = MODE1;
                }
                else
                {
                    sysparam.MODE = MODE21;
                    portGsensorCfg(1);
                }
                sprintf(message, "Change to Mode%d,and work on at", workmode);
                for (i = 0; i < timecount; i++)
                {
                    sprintf(message + strlen(message), " %.2d:%.2d", sysparam.AlarmTime[i] / 60, sysparam.AlarmTime[i] % 60);
                }
                sprintf(message + strlen(message), ",every %d day", gapday);
                //setNextAlarmTime(sysparam.gapDay, sysparam.AlarmTime);
                break;
            case 2:
                //MODE,2,0,0
                //MODE,2,0,M
                //MODE,2,N,M
                sysparam.gpsuploadgap = (uint16_t)atoi((const char *)item->item_data[2]);
                sysparam.gapMinutes = atoi(item->item_data[3]);
                sysparam.MODE = MODE2;

                if (sysparam.accctlgnss == 0)
                {
                    gpsRequestSet(GPS_REQ_KEEPON);
                }
                if (sysparam.gpsuploadgap == 0)
                {
                    gpsRequestClear(GPS_REQ_ALL);
                    //运动不自动传GPS
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

                    if (getTerminalAccState())
                    {
                        if (sysparam.gpsuploadgap < GPS_UPLOAD_GAP_MAX)
                        {
                            gpsRequestSet(GPS_REQ_ACC);
                        }
                        else
                        {
                            gpsRequestClear(GPS_REQ_ACC);
                        }
                    }
                    else
                    {
                        gpsRequestClear(GPS_REQ_ACC);
                    }



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
                portGsensorCfg(1);
                break;
            case 3:
            case 23:

                sysparam.gapMinutes = atoi(item->item_data[2]);
                if (sysparam.gapMinutes <= 5)
                {
                    sysparam.gapMinutes = 5;
                }

                if (workmode == 3)
                {
                    terminalAccoff();
                    gpsRequestClear(GPS_REQ_ACC);
                    sysparam.MODE = MODE3;
                }
                else
                {
                    sysparam.MODE = MODE23;
                    portGsensorCfg(1);
                }
                sprintf(message, "Change to mode %d and update the startup interval time to %d minutes", workmode, sysparam.gapMinutes);
                break;
            default:
                strcpy(message, "Unsupport mode");
                break;
        }
        paramSaveAll();
    }
}
static void doTTSInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0)
    {
        strcpy(message, "TTS play error");
    }
    else
    {
        portPushTTS(item->item_data[1]);
        strcpy(message, "TTS play done");
    }
}
static void doJtInstruction(ITEM *item, char *message)
{
    uint8_t recordTime, ret;
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        recordTime = 30;
    }
    else
    {
        recordTime = atoi(item->item_data[1]);
        recordTime = recordTime == 0 ? 30 : recordTime;

    }
    ret = recordRequestSet(recordTime);
    if (ret == 0)
    {
        strcpy(message, "start recording fail");
    }

    else
    {
        sprintf(message, "start recording for %d seconds", recordTime);
    }

}

//(357784086584883)<Local Time:2020-09-23 16:09:33>http://maps.google.com/maps?q=22.58799,113.85864


static void do123Instruction(ITEM *item, char *message, instructionParam_s *param)
{
    mode123 = param->mode;
    link123 = param->link;
    if (param->telNum != NULL)
    {
        strcpy(telnum123, param->telNum);
    }
    lbsRequestSet();
    wifiRequestSet();
    gpsRequestSet(GPS_REQ_UPLOAD_ONE_POI);
    sysinfo.flag123 = 1;
    saveInstructionId();

}

static void updateApn(void)
{
    portSetApn(sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
}

static void doAPNInstruction(ITEM *item, char *message)
{
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
        startTimer(30, updateApn, 0);
        sprintf(message, "Update APN:%s,APN User:%s,APN Password:%s", sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
    }

}
static void doUPSInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] != 0 && item->item_data[2][0] != 0)
    {
        sysparam.updateServerPort = atoi(item->item_data[2]);
        strcpy((char *)sysparam.updateServer, item->item_data[1]);
        paramSaveAll();
    }
    upgradeStartInit();
    sprintf(message, "The device will download the firmware from %s:%d in 5 seconds", sysparam.updateServer,
            sysparam.updateServerPort);
}
static void doLOWWInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "The low voltage param is %.1fV", sysparam.lowvoltage);
    }
    else
    {
        sysparam.lowvoltage = atoi(item->item_data[1]) / 10.0;
        paramSaveAll();
        sprintf(message, "When the voltage is below %.1fV, the device will upload a alarm", sysparam.lowvoltage);
    }
}

static void doLEDInstruction(ITEM *item, char *message)
{
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
}
static void doResetInstruction(ITEM *item, char *message)
{
    sprintf(message, "System will reset after 5 seconds");
    paramSaveAll();
    startTimer(50, portSystemReset, 0);
}

static void doUTCInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "System time zone:UTC %s%d", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
    }
    else
    {
        sysparam.utc = atoi(item->item_data[1]);
        if (sysparam.utc < -12 || sysparam.utc > 12)
            sysparam.utc = 8;
        paramSaveAll();
        sprintf(message, "Update the system time zone to UTC %s%d", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
    }
}
static void doAlarmModeInstrucion(ITEM *item, char *message)
{
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
}

static void doDebugInstrucion(ITEM *item, char *message)
{
    uint16_t year = 0;
    uint8_t  month = 0, date = 0, hour = 0, minute = 0, second = 0;
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);
        sprintf(message, "Time: %.2d/%.2d/%.2d %.2d:%.2d:%.2d;", year, month, date, hour, minute, second);
        sprintf(message + strlen(message), "sysrun: %.2ld:%.2ld:%.2ld;", sysinfo.sysTick / 3600, sysinfo.sysTick % 3600 / 60,
                sysinfo.sysTick % 60);
        sprintf(message + strlen(message), "gpsLast: %.2ld:%.2ld:%.2ld;", sysinfo.gpsUpdateTick / 3600,
                sysinfo.gpsUpdateTick % 3600 / 60, sysinfo.gpsUpdateTick % 60);
        sprintf(message + strlen(message), "gpsrequest:0x%04X;", sysinfo.gpsRequest);
        sprintf(message + strlen(message), "hideLogin:%s;", hiddenServIsReady() ? "Yes" : "NO");
        sprintf(message + strlen(message), "bleErr:%d;", sysparam.bleErrCnt);
    }
    else
    {
        if (my_strpach(item->item_data[1], "MODECNTCLEAR"))
        {
            sysparam.startUpCnt = 0;
            sysparam.runTime = 0;
            paramSaveAll();
            strcpy(message, "Debug:clear mode count");
        }
        else
        {
            strcpy(message, "Debug:unknow");
        }
    }
}

static void doACCCTLGNSSInstrucion(ITEM *item, char *message)
{
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
                gpsRequestSet(GPS_REQ_KEEPON);
            }
            else
            {
                gpsRequestClear(GPS_REQ_KEEPON);
            }
        }
        sprintf(message, "%s", sysparam.accctlgnss ? "GPS will be automatically controlled by the program" :
                "The GPS will always be on");
        paramSaveAll();
    }

}
static void doPdopInstrucion(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current podp filter is %.2f", sysparam.pdop);
    }
    else
    {
        sysparam.pdop = atoi(item->item_data[1]) / 100.0;
        paramSaveAll();
        sprintf(message, "Update podp filter to %.2f", sysparam.pdop);
    }
}
static void doBFInstruction(ITEM *item, char *message)
{
    sysparam.bf = 1;
    paramSaveAll();
    strcpy(message, "BF OK");
}
static void doCFInstruction(ITEM *item, char *message)
{
    sysparam.bf = 0;
    paramSaveAll();
    strcpy(message, "CF OK");
}
static void doFenceInstrucion(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "The static drift fence is %d meters", sysparam.fence);
    }
    else
    {
        sysparam.fence = atoi(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "Set the static drift fence to %d meters", sysparam.fence);
    }
}
static void doFactoryInstrucion(ITEM *item, char *message)
{
    if (strstr(item->item_data[1], "ZTINFO8888") != NULL)
    {
        sprintf(message, "Factory all successfully");
        paramSetDefault(0);
    }
    else
    {
        sprintf(message, "Factory Settings restored successfully");
        paramSetDefault(1);
    }
}

static void doRelayInstrucion(ITEM *item, char *message)
{
    if (item->item_data[1][0] == '1')
    {
        sysparam.relayCtl = 1;
        paramSaveAll();
        relayAutoRequest();
        strcpy(message, "Relay on success");
    }
    else if (item->item_data[1][0] == '0')
    {
        RELAY_OFF;
        sysparam.relayCtl = 0;
        paramSaveAll();
        relayAutoClear();
        bleScheduleSetAllReq(BLE_EVENT_SET_DEVOFF | BLE_EVENT_CLR_CNT);
        bleScheduleClearAllReq(BLE_EVENT_SET_DEVON);
        strcpy(message, "Relay off success");
    }
    else
    {
        sprintf(message, "Relay status %s", sysparam.relayCtl == 1 ? "relay on" : "relay off");
    }
}
static void doVolInstrucion(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
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
        portSetVol(sysparam.vol);
    }
}

static void doAccdetmodeInstruction(ITEM *item, char *message)
{
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
            case ACCDETMODE1:
                sprintf(message, "The device is use acc wire first and voltage second to determine whether ACC is ON or OFF.");
                break;
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
}
static void doSetAgpsInstruction(ITEM *item, char *message)
{
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
}

static void bleServReOpen(void)
{
    bleServCloseRequestClear();
}


static void doBLEENInstrucion(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "BLE was %s", sysparam.bleen ? "Enable" : "Disable");
    }
    else
    {
        sysparam.bleen = atol(item->item_data[1]);
        if (sysparam.bleen)
        {
            memset(sysparam.bleConnMac, 0, sizeof(sysparam.bleConnMac));
            bleScheduleWipeAll();
            bleScheduleCtrl(0);
            bleServCloseRequestSet();
            startTimer(30, bleServReOpen, 0);
        }
        else
        {
            sysinfo.bleOnBySystem = 0;
        }

        paramSaveAll();
        sprintf(message, "%s the ble function", sysparam.bleen ? "Enable" : "Disable");
    }
}

static uint8_t deleteSosNumber(char *sosnumber)
{
    LogPrintf(DEBUG_ALL, "Search %s", sosnumber);
    if (my_strpach((char *)sysparam.sosnumber1, (char *)sosnumber))
    {
        sysparam.sosnumber1[0] = 0;
        LogPrintf(DEBUG_ALL, "Delete sos num 1 %s", sysparam.sosnumber1);
        return 1;
    }
    if (my_strpach((char *)sysparam.sosnumber2, (char *)sosnumber))
    {
        sysparam.sosnumber2[0] = 0;
        LogPrintf(DEBUG_ALL, "Delete sos num 2 %s", sysparam.sosnumber2);
        return 1;
    }
    if (my_strpach((char *)sysparam.sosnumber3, (char *)sosnumber))
    {
        sysparam.sosnumber3[0] = 0;
        LogPrintf(DEBUG_ALL, "Delete sos num 3 %s", sysparam.sosnumber3);
        return 1;
    }
    return 0;
}

static void doSOSInstruction(ITEM *item, char *message)
{
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
                if (deleteSosNumber(item->item_data[2]))
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
                if (deleteSosNumber(item->item_data[3]))
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
                if (deleteSosNumber(item->item_data[4]))
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
}

static void doFcgInstrucion(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "acc on voltage is %.2f,acc off voltage is %.2f", sysparam.accOnVoltage, sysparam.accOffVoltage);
    }
    else
    {
        sysparam.accOnVoltage = atof(item->item_data[2]) / 10.0;
        sysparam.accOffVoltage = atof(item->item_data[3]) / 10.0;

        if (sysparam.accOnVoltage < 1.0)
        {
            sysparam.accOnVoltage = 13.2;
        }
        if (sysparam.accOffVoltage < 1.0)
        {
            sysparam.accOffVoltage = 12.8;
        }

        if (sysparam.accOffVoltage > sysparam.accOnVoltage)
        {
            sysparam.accOnVoltage = 13.2;
            sysparam.accOffVoltage = 12.8;
        }
        paramSaveAll();
        sprintf(message, "set acc on voltage to %.2f,acc off voltage to %.2f", sysparam.accOnVoltage, sysparam.accOffVoltage);
    }
}

static void doFactoryTestInstruction(ITEM *item, char *message)
{
    uint8_t total, i;
    gpsinfo_s *gpsinfo;
    //感光检测
    sprintf(message, "Light:%s;", LDR_READ ? "darkness" : "brightness");
    //acc线检测
    sprintf(message + strlen(message), "ACC:%s;", ACC_READ == ACC_STATE_ON ? "ON" : "OFF");
    sprintf(message + strlen(message), "CSQ:%d;", portGetModuleRssi());
    //Gsensor检测
    if (read_gsensor_id() != 0)
    {
        sprintf(message + strlen(message), "Gsensor:ERROR;");
    }
    else
    {
        sprintf(message + strlen(message), "Gsensor:OK;");
    }
    //gps
    gpsinfo = getCurrentGPSInfo();
    total = sizeof(gpsinfo->gpsCn);

    sprintf(message + strlen(message), "NMEA:%d/%d/%d %02d:%02d:%02d;", gpsinfo->datetime.year, gpsinfo->datetime.month,
            gpsinfo->datetime.day, gpsinfo->datetime.hour, gpsinfo->datetime.minute, gpsinfo->datetime.second);
    sprintf(message + strlen(message), "GPVIEW:%d,", gpsinfo->gpsviewstart);
    sprintf(message + strlen(message), "BDVIEW:%d,", gpsinfo->beidouviewstart);
    sprintf(message + strlen(message), "USE:%d;", gpsinfo->used_star);
    sprintf(message + strlen(message), "FIX:%s;", gpsinfo->fixstatus ? "Fixed" : "Invalid");
    sprintf(message + strlen(message), "MODE:%d,", gpsinfo->fixmode);
    sprintf(message + strlen(message), "PDOP:%.2f;", gpsinfo->pdop);
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


static void doJT808SNInstrucion(ITEM *item, char *message)
{
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
            jt808CreateSn(sysparam.jt808sn, (uint8_t *)item->item_data[1], snlen);
            changeByteArrayToHexString(sysparam.jt808sn, (uint8_t *)senddata, 6);
            senddata[12] = 0;
            sprintf(message, "Update SN:%s", senddata);
            sysparam.jt808isRegister = 0;
            sysparam.jt808AuthLen = 0;
            jt808RegisterLoginInfo(sysparam.jt808sn, sysparam.jt808isRegister, sysparam.jt808AuthCode, sysparam.jt808AuthLen);
            paramSaveAll();
        }
    }
}

static void doJT808ParamInstrucion(ITEM *item, char *message)
{
    char senddata[40];
    changeByteArrayToHexString(sysparam.jt808sn, (uint8_t *)senddata, 6);
    senddata[12] = 0;
    sprintf(message, "JT808SN:%s;", senddata);
    sprintf(message + strlen(message), "SERVER:%s:%d;", sysparam.jt808Server, sysparam.jt808Port);

}

static void doHideServerInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "hidden server %s:%d was %s", sysparam.hiddenServer, sysparam.hiddenPort,
                sysparam.hiddenServOnoff ? "on" : "off");
    }
    else
    {

        if (item->item_data[1][0] == '1')
        {
            sysparam.hiddenServOnoff = 1;
            if (item->item_data[2][0] != 0 && item->item_data[3][0] != 0)
            {
                strncpy((char *)sysparam.hiddenServer, item->item_data[2], 50);
                stringToLowwer(sysparam.hiddenServer, strlen(sysparam.hiddenServer));
                sysparam.hiddenPort = atoi((const char *)item->item_data[3]);
                sprintf(message, "Update hidden server %s:%d and enable it", sysparam.hiddenServer, sysparam.hiddenPort);
            }
            else
            {
                strcpy(message, "please enter your param");
            }
        }
        else
        {
            sysparam.hiddenServOnoff = 0;
            strcpy(message, "Disable hidden server");
        }
        paramSaveAll();
    }
}
static void doSetBleMacInstruction(ITEM *item, char *message)
{
    uint8_t i, j, l, ind;
    char mac[20];
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        strcpy(message, "BLELIST:");
        for (i = 0; i < 5; i++)
        {
            if (strlen((char *)sysparam.bleConnMac[i]) != 0)
            {
                sprintf(message + strlen(message), " %s", sysparam.bleConnMac[i]);
            }
        }
        strcat(message, ";");
    }
    else
    {
        //互斥操作，开启蓝牙主机功能时，关闭蓝牙从机功能
        sysinfo.bleOnBySystem = 0;
        sysparam.bleen = 0;

        memset(sysparam.bleConnMac, 0, sizeof(sysparam.bleConnMac));
        bleScheduleWipeAll();
        ind = 0;
        //Enable ble function,Update BLEMAC1 :%s
        strcpy(message, "Enable ble function,Update MAC: ");
        for (i = 1; i < item->item_cnt; i++)
        {
            if (strlen(item->item_data[i]) != 12)
            {
                continue;
            }
            //aa bb cc dd ee ff
            //ff ee dd cc bb aa
            l = 5;
            for (j = 0; j < 3; j++)
            {
                memcpy(mac, &item->item_data[i][j * 2], 2);
                memcpy(&item->item_data[i][j * 2], &item->item_data[i][l * 2], 2);
                memcpy(&item->item_data[i][l * 2], mac, 2);
                l--;
            }


            l = 0;
            for (j = 0; j < 12; j += 2)
            {
                memcpy(mac + l, item->item_data[i] + j, 2);
                l += 2;
                if (j % 2 == 0 && (j + 2 < 12))
                {
                    mac[l++] = ':';
                }
            }
            mac[l] = 0;
            strcpy((char *)sysparam.bleConnMac[ind++], mac);
            bleScheduleInsert(mac);
            sprintf(message + strlen(message), " %s", mac);
        }
        paramSaveAll();
        if (ind == 0)
        {
            bleScheduleCtrl(0);
            strcpy(message, "Disable the ble function,and the ble mac was clear");
        }
        else
        {
            bleScheduleCtrl(1);
        }
    }
}

static void doReadParamInstruction(ITEM *item, char *message)
{
    uint8_t i, cnt;
    bleRelayInfo_s *bleinfo;
    cnt = 0;
    for (i = 0; i < BLE_CONNECT_LIST_SIZE; i++)
    {
        bleinfo = bleGetDevInfo(i);
        if (bleinfo != NULL)
        {
            cnt++;
            sprintf(message + strlen(message), "[T:%ld,V:(%.2fV,%.2fV)] ", sysinfo.sysTick - bleinfo->updateTick, bleinfo->rfV,
                    bleinfo->outV);
        }
    }
    if (cnt == 0)
    {
        sprintf(message, "no ble info");
    }
}


static void doSetBleParamInstruction(ITEM *item, char *message)
{
    uint8_t i, cnt;
    bleRelayInfo_s *bleinfo;
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        cnt = 0;

        for (i = 0; i < BLE_CONNECT_LIST_SIZE; i++)
        {
            bleinfo = bleGetDevInfo(i);
            if (bleinfo != NULL)
            {
                cnt++;
                sprintf(message + strlen(message), "(%d:[%.2f,%.2f,%d]) ", i, bleinfo->rf_threshold, bleinfo->out_threshold,
                        bleinfo->disc_threshold);
            }
        }
        if (cnt == 0)
        {
            sprintf(message, "no ble info");
        }
        else
        {
            sprintf(message + strlen(message), "DISC:%dm", sysparam.bleAutoDisc);
        }
    }
    else
    {
        for (i = 0; i < BLE_CONNECT_LIST_SIZE; i++)
        {
            bleinfo = bleGetDevInfo(i);
            if (bleinfo != NULL)
            {
                bleinfo->rf_threshold = 0;
                bleinfo->out_threshold = 0;
            }
        }
        sysparam.bleRfThreshold = atoi(item->item_data[1]);
        sysparam.bleOutThreshold = atoi(item->item_data[2]);
        sysparam.bleAutoDisc = atoi(item->item_data[3]);
        paramSaveAll();
        sprintf(message, "Update new ble param to %.2fv,%.2fv,%d", sysparam.bleRfThreshold / 100.0,
                sysparam.bleOutThreshold / 100.0, sysparam.bleAutoDisc);
        bleScheduleSetAllReq(BLE_EVENT_SET_RF_THRE | BLE_EVENT_SET_OUTV_THRE | BLE_EVENT_SET_AD_THRE | BLE_EVENT_GET_RF_THRE |
                             BLE_EVENT_GET_OUT_THRE | BLE_EVENT_GET_AD_THRE);
    }
}


static void doSetBleWarnParamInstruction(ITEM *item, char *message)
{
    uint8_t i, cnt;
    bleRelayInfo_s *bleinfo;

    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        cnt = 0;
        for (i = 0; i < BLE_CONNECT_LIST_SIZE; i++)
        {
            bleinfo = bleGetDevInfo(i);
            if (bleinfo != NULL)
            {
                cnt++;
                sprintf(message + strlen(message), "(%d:[%.2f,%d,%d]) ", i, bleinfo->preV_threshold / 100.0,
                        bleinfo->preDetCnt_threshold,
                        bleinfo->preHold_threshold);
            }
        }
        if (cnt == 0)
        {
            sprintf(message, "no ble info");
        }
    }
    else
    {
        sysparam.blePreShieldVoltage = atoi(item->item_data[1]);
        sysparam.blePreShieldDetCnt = atoi(item->item_data[2]);
        sysparam.blePreShieldHoldTime = atoi(item->item_data[3]);
        if (sysparam.blePreShieldDetCnt >= 60)
        {
            sysparam.blePreShieldDetCnt = 30;
        }
        else if (sysparam.blePreShieldDetCnt == 0)
        {
            sysparam.blePreShieldDetCnt = 10;
        }
        if (sysparam.blePreShieldHoldTime == 0)
        {
            sysparam.blePreShieldHoldTime = 1;
        }
        paramSaveAll();
        sprintf(message, "Update ble warnning param to %d,%d,%d", sysparam.blePreShieldVoltage, sysparam.blePreShieldDetCnt,
                sysparam.blePreShieldHoldTime);
        for (i = 0; i < BLE_CONNECT_LIST_SIZE; i++)
        {
            bleinfo = bleGetDevInfo(i);
            if (bleinfo != NULL)
            {
                bleinfo->preV_threshold = 0;
                bleinfo->preDetCnt_threshold = 0;
                bleinfo->preHold_threshold = 0;
            }
        }
        bleScheduleSetAllReq(BLE_EVENT_SET_PRE_PARAM | BLE_EVENT_GET_PRE_PARAM);
    }
}



static void doRelayFunInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current relayfun %d", sysparam.relayFun);
    }
    else
    {
        sysparam.relayFun = atoi(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "Update relayfun %d", sysparam.relayFun);
    }
}



static void doRelaySpeedInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current relaySpeed %d km/h", sysparam.relaySpeed);
    }
    else
    {
        sysparam.relaySpeed = atoi(item->item_data[1]);
        paramSaveAll();
        sprintf(message, "Update relaySpeed %d km/h", sysparam.relaySpeed);
    }
}

static void doBleServerInstruction(ITEM *item, char *message)
{
    if (item->item_data[1][0] == 0 || item->item_data[1][0] == '?')
    {
        sprintf(message, "ble server was %s:%d", sysparam.bleServer, sysparam.bleServerPort);
    }
    else
    {
        if (item->item_data[1][0] != 0 && item->item_data[2][0] != 0)
        {
            strncpy((char *)sysparam.bleServer, item->item_data[1], 50);
            stringToLowwer(sysparam.bleServer, strlen(sysparam.bleServer));
            sysparam.bleServerPort = atoi((const char *)item->item_data[2]);
            sprintf(message, "Update ble server %s:%d", sysparam.bleServer, sysparam.bleServerPort);
            paramSaveAll();
        }
        else
        {
            strcpy(message, "please enter your param");
        }

    }
}


static void doInstruction(int16_t cmdid, ITEM *item, instructionParam_s *param)
{
    char message[512];
    message[0] = 0;
    switch (cmdid)
    {
        case PARAM_INS:
            doParamInstruction(item, message);
            break;
        case STATUS_INS:
            doStatusInstruction(item, message);
            break;
        case VERSION_INS:
            doVersionInstruction(item, message);
            break;
        case SN_INS:
            doSNInstruction(item, message);
            break;
        case SERVER_INS:
            doServerInstruction(item, message);
            break;
        case HBT_INS:
            doHbtInstruction(item, message);
            break;
        case MODE_INS:
            doModeInstruction(item, message);
            break;
        case TTS_INS:
            doTTSInstruction(item, message);
            break;
        case JT_INS:
            doJtInstruction(item, message);
            break;
        case POSITION_INS:
            do123Instruction(item, message, param);
            break;
        case APN_INS:
            doAPNInstruction(item, message);
            break;
        case UPS_INS:
            doUPSInstruction(item, message);
            break;
        case LOWW_INS:
            doLOWWInstruction(item, message);
            break;
        case LED_INS:
            doLEDInstruction(item, message);
            break;
        case RESET_INS:
            doResetInstruction(item, message);
            break;
        case UTC_INS:
            doUTCInstruction(item, message);
            break;
        case ALARMMODE_INS:
            doAlarmModeInstrucion(item, message);
            break;
        case DEBUG_INS:
            doDebugInstrucion(item, message);
            break;
        case ACCCTLGNSS_INS:
            doACCCTLGNSSInstrucion(item, message);
            break;
        case PDOP_INS:
            doPdopInstrucion(item, message);
            break;
        case BF_INS:
            doBFInstruction(item, message);
            break;
        case CF_INS:
            doCFInstruction(item, message);
            break;
        case FENCE_INS:
            doFenceInstrucion(item, message);
            break;
        case FACTORY_INS:
            doFactoryInstrucion(item, message);
            break;
        case RELAY_INS:
            doRelayInstrucion(item, message);
            break;
        case VOL_INS:
            doVolInstrucion(item, message);
            break;
        case ACCDETMODE_INS:
            doAccdetmodeInstruction(item, message);
            break;
        case SETAGPS_INS:
            doSetAgpsInstruction(item, message);
            break;
        case BLEEN_INS:
            doBLEENInstrucion(item, message);
            break;
        case SOS_INS:
            doSOSInstruction(item, message);
            break;
        case FCG_INS:
            doFcgInstrucion(item, message);
            break;
        case FACTORYTEST_INS:
            doFactoryTestInstruction(item, message);
            break;
        case JT808SN_INS:
            doJT808SNInstrucion(item, message);
            break;
        case JT808PARAM_INS:
            doJT808ParamInstrucion(item, message);
            break;
        case HIDESERVER_INS:
            doHideServerInstruction(item, message);
            break;
        case SETBLEMAC_INS:
            doSetBleMacInstruction(item, message);
            break;
        case READPARAM_INS:
            doReadParamInstruction(item, message);
            break;
        case SETBLEPARAM_INS:
            doSetBleParamInstruction(item, message);
            break;
        case SETBLEWARNPARAM_INS:
            doSetBleWarnParamInstruction(item, message);
            break;
        case RELAYFUN_INS:
            doRelayFunInstruction(item, message);
            break;
        case RELAYSPEED_INS:
            doRelaySpeedInstruction(item, message);
            break;
        case BLESERVER_INS:
            doBleServerInstruction(item, message);
            break;
        default:
            if (param->mode == MESSAGE_MODE)
                return ;
            snprintf(message, 50, "Unsupport CMD:%s;", item->item_data[0]);
            break;
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), param->mode, param->telNum, param->link);
}

/**************************************************
@bref		指令解析
@param
	str		待解析指令
	len		指令长度
	mode	发送模式
	telnum	手机号码
	param	其他参数
@note
**************************************************/

void instructionParser(uint8_t *str, uint16_t len, instructionParam_s *param)
{
    ITEM item;
    int cmdid;
    stringToItem(&item, str, len);
    stringToUpper(item.item_data[0], ITEMSIZEMAX);
    LogPrintf(DEBUG_ALL, "Instruction:%s", item.item_data[0]);
    cmdid = getInstructionid((uint8_t *)item.item_data[0]);
    doInstruction(cmdid, &item, param);
}

/**************************************************
@bref		123指令回复
@param
@return
@note
**************************************************/

void doPositionRespon(void)
{
    char message[150];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    gpsinfo_s *gpsinfo;
    if (sysinfo.flag123 == 0)
    {
        return;
    }

    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    sysinfo.flag123 = 0;
    gpsinfo = getCurrentGPSInfo();
    if (gpsinfo->fixstatus)
    {
        sprintf(message, "(%s)<Local Time:%.2d/%.2d/%.2d %.2d:%.2d:%.2d>http://maps.google.com/maps?q=%f,%f", sysparam.SN,
                year, month, date, hour, minute, second, gpsinfo->latitude, gpsinfo->longtitude);
    }
    else
    {
        strcpy(message, "Sorry,satellite positioning failure");
    }
    recoverInstructionId();
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode123, telnum123, link123);
}

