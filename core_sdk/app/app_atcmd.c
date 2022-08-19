#include "app_atcmd.h"
#include "app_sys.h"
#include "app_socket.h"
#include "app_port.h"
#include "app_kernal.h"
#include "app_gps.h"
#include "app_instructioncmd.h"
#include "app_protocol.h"
#include "app_net.h"
#include "app_sn.h"
#include "app_task.h"
#include "app_param.h"
#include "app_mir3da.h"
#include "app_jt808.h"
#include "app_ble.h"
#include "stdlib.h"


const atCmd_s atCmdTable[] =
{
    {AT_DEBUG_CMD, "DEBUG"},
    {AT_SMS_CMD, "SMS"},
    {AT_NMEA_CMD, "NMEA"},
    {AT_ZTSN_CMD, "ZTSN"},
    {AT_IMEI_CMD, "IMEI"},
    {AT_FMPC_NMEA_CMD, "FMPC_NMEA"},
    {AT_FMPC_BAT_CMD, "FMPC_BAT"},
    {AT_FMPC_GSENSOR_CMD, "FMPC_GSENSOR"},
    {AT_FMPC_ACC_CMD, "FMPC_ACC"},
    {AT_FMPC_GSM_CMD, "FMPC_GSM"},
    {AT_FMPC_RELAY_CMD, "FMPC_RELAY"},
    {AT_FMPC_LDR_CMD, "FMPC_LDR"},
    {AT_FMPC_ADCCAL_CMD, "FMPC_ADCCAL"},
    {AT_FMPC_CSQ_CMD, "FMPC_CSQ"},
    {AT_FMPC_IMSI_CMD, "FMPC_IMSI"},
    {AT_FMPC_CHKP_CMD, "FMPC_CHKP"},
    {AT_FMPC_CM_CMD, "FMPC_CM"},
    {AT_FMPC_CMGET_CMD, "FMPC_CMGET"},
    {AT_FMPC_EXTVOL_CMD, "FMPC_EXTVOL"},
    {AT_FMPC_WIFI_CMD, "FMPC_WIFI"},
    {AT_FMCP_WDTSTOP_CMD, "FMPC_WDTSTOP"},
    {AT_FMPC_AUDIO_CMD, "FMPC_AUDIO"},
};

/**************************************************
@bref		在列表中查找是否存在该指令
@param
	cmd		要查找的命令
@return

@note
**************************************************/

static int atCmdFindInList(char *cmd)
{
    uint8_t i;
    for (i = 0; i < sizeof(atCmdTable) / sizeof(atCmdTable[0]); i++)
    {
        if (strcmp(atCmdTable[i].cmdStr, cmd) == 0)
        {
            return atCmdTable[i].cmd;
        }
    }
    return -1;
}

/**************************************************
@bref		解析要设置log输出等级
@param
	buf
	len
@return
@note
**************************************************/

static void setDebugLevel(char *buf, uint16_t len)
{

    if (buf[0] >= '0' && buf[0] <= '9')
    {
        sysinfo.logLevel = buf[0] - '0';
        LogPrintf(DEBUG_NONE, "Debug LEVEL:%d OK", sysinfo.logLevel);
    }

}

/**************************************************
@bref		DEBUG 指令
@param
	buf
	len
@return
@note
**************************************************/

static void doAtdebugCmd(char *buf, uint32_t len)
{
    ITEM item;
    stringToItem(&item, (uint8_t *) buf, len);
    if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"SUSPEND"))
    {
        LogMessage(DEBUG_FACTORY, "Suspend all task");
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"RESUME"))
    {
        LogMessage(DEBUG_FACTORY, "Resume all taskn");
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"ONOFFREAD"))
    {
        LogPrintf(DEBUG_FACTORY, "STATE:%d", ONOFF_READ);
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"SHUTDOWN"))
    {
        portSystemShutDown();
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"AGPS"))
    {
        agpsRequestSet();
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"COLD"))
    {
        portSetGpsStarupMode(COLD_START);
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"BLETEST"))
    {
        //portOutsideGpsPwr(atoi(item.item_data[1]));
        //portUartCfg(APPUSART1, 1, 9600, nmeaParser);
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"BLESCH"))
    {
        bleScheduleCtrl(atoi(item.item_data[1]));
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"BLE"))
    {
        appSendThreadEvent(THREAD_EVENT_BLE_CLIENT, atoi(item.item_data[1]));
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"INSERT"))
    {
        bleScheduleInsert(item.item_data[1]);
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"DELETE"))
    {
        bleScheduleDelete(atoi(item.item_data[1]));
    }
    else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"BLEINSERT"))
    {
    	bleInfo_s dev;
		dev.batLevel=88;
		strcpy(dev.imei,"862061044075777");
		dev.next=NULL;
		dev.startCnt=88;
		dev.vol=4.7;
        bleServerAddInfo(dev);
		LogPrintf(DEBUG_ALL,"Insert %s",dev.imei);
    }else if (mycmdPatch((uint8_t *)item.item_data[0], (uint8_t *)"SHOW"))
    {
        showBleList();
    }
    else
    {
        setDebugLevel(item.item_data[0], strlen(item.item_data[0]));
    }

}
/**************************************************
@bref		NMEA 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdNmeaParser(uint8_t *buf, uint16_t len)
{
    if (strstr((char *)buf, "ON") != NULL)
    {
        sysinfo.nmeaOutput = 1;
        LogMessage(DEBUG_FACTORY, "NMEA OPEN");
    }
    else
    {
        sysinfo.nmeaOutput = 0;
        LogMessage(DEBUG_FACTORY, "NMEA CLOSE");
    }
}
/**************************************************
@bref		ZTSN 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdZTSNParser(uint8_t *buf, uint16_t len)
{
    char IMEI[15];
    uint8_t sndata[30];
    changeHexStringToByteArray(sndata, buf, len / 2);
    decryptSN(sndata, IMEI);
    LogPrintf(DEBUG_FACTORY, "Decrypt: %s", IMEI);
    strncpy((char *)sysparam.SN, IMEI, 15);
    sysparam.SN[15] = 0;
    jt808CreateSn(sysparam.jt808sn, (uint8_t *)sysparam.SN + 3, 12);
    sysparam.jt808isRegister = 0;
    paramSaveAll();
    LogMessage(DEBUG_FACTORY, "Write Sn Ok");
}
/**************************************************
@bref		IMEI 指令
@param
	buf
	len
@return
@note
**************************************************/

void atCmdIMEIParser(void)
{
    char imei[20];
    portGetModuleIMEI(imei);
    LogPrintf(DEBUG_FACTORY, "ZTINFO:%s:%s:%s\r\n", sysparam.SN, imei, EEPROM_VERSION);
}
/**************************************************
@bref		FMPC_NMEA 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcNmeaParser(uint8_t *buf, uint16_t len)
{
    if (my_strstr((char *)buf, "ON", len))
    {
        LogMessage(DEBUG_FACTORY, "NMEA ON OK");
        sysinfo.nmeaOutput = 1;
        gpsRequestSet(GPS_REQ_DEBUG);
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "NMEA OFF OK");
        gpsRequestClear(0xFFFF);
        sysinfo.nmeaOutput = 0;
    }
}
/**************************************************
@bref		FMPC_BAT 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcBatParser(void)
{
    LogPrintf(DEBUG_FACTORY, "Vbat: %.3fv", sysinfo.batteryVol);
}
/**************************************************
@bref		FMPC_GSENSOR 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcGsensorParser(void)
{
    read_gsensor_id();
}
/**************************************************
@bref		FMPC_ACC 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcAccParser(void)
{
    LogPrintf(DEBUG_FACTORY, "ACC is %s", ACC_READ == ACC_STATE_ON ? "ON" : "OFF");
}

/**************************************************
@bref		FMPC_GSM  指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcGsmParser(void)
{
    if (isNetworkNormal())
    {
        LogMessage(DEBUG_FACTORY, "GSM SERVICE OK");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "GSM SERVICE FAIL");
    }
}
/**************************************************
@bref		FMPC_RELAY 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcRelayParser(uint8_t *buf, uint16_t len)
{
    if (strstr((char *)buf, "ON") != NULL)
    {
        RELAY_ON;
        LogMessage(DEBUG_FACTORY, "Relay ON OK");
    }
    else
    {
        RELAY_OFF;
        LogMessage(DEBUG_FACTORY, "Relay OFF OK");
    }
}

/**************************************************
@bref		FMPC_LDR 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcLdrParser(void)
{
    if (LDR_READ)
    {
        LogMessage(DEBUG_FACTORY, "Light sensor detects darkness");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "Light sensor detects brightness");

    }
}

/**************************************************
@bref		FMPC_ADCCAL 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcAdcCalParser(void)
{
    float cal;
    cal = portGetOutSideVolAdcVol();
    sysparam.adccal = 12.0 / cal;
    paramSaveAll();
    LogPrintf(DEBUG_FACTORY, "Update the voltage calibration parameter to %f", sysparam.adccal);

}
/**************************************************
@bref		FMPC_CSQ 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcCsqParser(void)
{
    LogPrintf(DEBUG_FACTORY, "+CSQ: %d,99\r\nOK", portGetModuleRssi());
}

/**************************************************
@bref		FMPC_IMSI 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcIMSIParser(void)
{
    char imsi[25];
    char iccid[25];
    portGetModuleIMSI(imsi);
    portGetModuleICCID(iccid);
    LogPrintf(DEBUG_FACTORY, "FMPC_IMSI_RSP OK, IMSI=%s&&%s&&%s", sysparam.SN, imsi, iccid);
}
/**************************************************
@bref		FMPC_CHKP 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcChkpParser(void)
{
    LogPrintf(DEBUG_FACTORY, "+FMPC_CHKP:%s,%s:%d", sysparam.SN, sysparam.Server, sysparam.ServerPort);
}

/**************************************************
@bref		FMPC_CM 指令
@param
	buf
	len
@return
@note
**************************************************/

static void atCmdFmpcCmParser(void)
{
    sysparam.cm = 1;
    paramSaveAll();
    LogMessage(DEBUG_FACTORY, "CM OK");

}
/**************************************************
@bref		FMPC_CMGET 指令
@param
@return
@note
**************************************************/

static void atCmdCmGetParser(void)
{
    if (sysparam.cm == 1)
    {
        LogMessage(DEBUG_FACTORY, "CM OK");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "CM FAIL");
    }
}
/**************************************************
@bref		FMPC_EXTVOL 指令
@param
@return
@note
**************************************************/

static void atCmdFmpcExtvolParser(void)
{
    LogPrintf(DEBUG_FACTORY, "+FMPC_EXTVOL: %.2f", sysinfo.outsideVol);
}
/**************************************************
@bref		FMPC_WIFI 指令
@param
@return
@note
**************************************************/

static void atCmdFmpcWifiParser(void)
{
    nwy_wifi_scan_list_t scan_list;
    memset(&scan_list, 0, sizeof(scan_list));
    nwy_wifi_scan(&scan_list);
    if (scan_list.num != 0)
    {
        LogPrintf(DEBUG_FACTORY, "+FMPC_WIFI: %d", scan_list.num);
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "+FMPC_WIFI: FAIL");
    }
}
/**************************************************
@bref		FMPC_WDTSTOP 指令
@param
@return
@note
**************************************************/

static void atCmdFmpcWdtStopParser(void)
{
    sysinfo.wdtTest = 1;
    LogMessage(DEBUG_FACTORY, "+FMPC_WDTSTOP: OK");
}


static void recTestStop(void)
{
    appSendThreadEvent(THREAD_EVENT_REC, THREAD_PARAM_REC_STOP);
    LogPrintf(DEBUG_FACTORY, "+FMPC_AUDIO: DONE");
}

/**************************************************
@bref		FMPC_AUDIO 指令
@param
@return
@note
**************************************************/

void atCmdFmpcAudioParser(void)
{
    if (sysinfo.recTest == 1)
    {
        return;
    }
    sysinfo.recTest = 1;
    appSendThreadEvent(THREAD_EVENT_REC, THREAD_PARAM_REC_START);
    LogPrintf(DEBUG_FACTORY, "+FMPC_AUDIO: START");
    startTimer(30, recTestStop, 0);
}


/**************************************************
@bref		指令解析
@param
	buf		要解析的数据
	len		数据长度
@return
@note
**************************************************/

void atCmdRecvParser(char *buf, uint32_t len)
{
    int ret = 0, cmdlen = 0, cmdid = 0;
    char cmdbuf[51];
    instructionParam_s insParam;
    LogMessageWL(DEBUG_FACTORY, (char *)buf, len);
    //识别AT^
    if (buf[0] == 'A' && buf[1] == 'T' && buf[2] == '^')
    {
        ret = getCharIndex((uint8_t *)buf, len, '=');
        if (ret < 0)
        {
            ret = getCharIndex((uint8_t *)buf, len, '\r');
        }
        if (ret >= 0)
        {
            cmdlen = ret - 3;
            if (cmdlen < 50)
            {
                strncpy(cmdbuf, (const char *)buf + 3, cmdlen);
                cmdbuf[cmdlen] = 0;
                cmdid = atCmdFindInList(cmdbuf);
                switch (cmdid)
                {
                    case AT_SMS_CMD:
                        memset(&insParam, 0, sizeof(instructionParam_s));
                        insParam.mode = SERIAL_MODE;
                        instructionParser((uint8_t *)buf + ret + 1, len - ret - 1, &insParam);
                        break;
                    case AT_DEBUG_CMD:
                        doAtdebugCmd(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_NMEA_CMD:
                        atCmdNmeaParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_ZTSN_CMD:
                        atCmdZTSNParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_IMEI_CMD:
                        atCmdIMEIParser();
                        break;
                    case AT_FMPC_NMEA_CMD:
                        atCmdFmpcNmeaParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_FMPC_BAT_CMD:
                        atCmdFmpcBatParser();
                        break;
                    case AT_FMPC_GSENSOR_CMD:
                        atCmdFmpcGsensorParser();
                        break;
                    case AT_FMPC_ACC_CMD:
                        atCmdFmpcAccParser();
                        break;
                    case AT_FMPC_GSM_CMD:
                        atCmdFmpcGsmParser();
                        break;
                    case AT_FMPC_RELAY_CMD:
                        atCmdFmpcRelayParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_FMPC_LDR_CMD:
                        atCmdFmpcLdrParser();
                        break;
                    case AT_FMPC_ADCCAL_CMD:
                        atCmdFmpcAdcCalParser();
                        break;
                    case AT_FMPC_CSQ_CMD:
                        atCmdFmpcCsqParser();
                        break;
                    case AT_FMPC_IMSI_CMD:
                        atCmdFmpcIMSIParser();
                        break;
                    case AT_FMPC_CHKP_CMD:
                        atCmdFmpcChkpParser();
                        break;
                    case AT_FMPC_CM_CMD:
                        atCmdFmpcCmParser();
                        break;
                    case AT_FMPC_CMGET_CMD:
                        atCmdCmGetParser();
                        break;
                    case AT_FMPC_EXTVOL_CMD:
                        atCmdFmpcExtvolParser();
                        break;
                    case AT_FMPC_WIFI_CMD:
                        atCmdFmpcWifiParser();
                        break;
                    case AT_FMCP_WDTSTOP_CMD:
                        atCmdFmpcWdtStopParser();
                        break;
                    case AT_FMPC_AUDIO_CMD:
                        atCmdFmpcAudioParser();
                        break;
                    default:
                        LogPrintf(DEBUG_FACTORY, "Unknow cmd:%s", cmdbuf);
                        break;
                }
            }
        }
        else
        {
            LogMessage(DEBUG_FACTORY, "not find \\r\\n");
        }
    }
}


