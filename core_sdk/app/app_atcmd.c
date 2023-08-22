#include "app_atcmd.h"
#include "app_sys.h"
#include "app_sn.h"
#include "app_net.h"
#include "nwy_pm.h"
#include "nwy_sim.h"
#include "nwy_wifi.h"
#include "nwy_vir_at.h"
#include "nwy_network.h"
#include "app_param.h"
#include "app_task.h"
#include "nwy_data.h"
#include "nwy_adc.h"
#include "nwy_socket.h"
#include "app_instructioncmd.h"
#include "app_gps.h"
#include "nwy_i2c.h"
#include "app_mir3da.h"
#include "nwy_file.h"
#include "app_param.h"
#include "nwy_voice.h"
#include "nwy_sms.h"
#include "app_protocol.h"
#include "nwy_ble.h"
#include "app_task.h"
#include "nwy_audio_api.h"
#include "app_port.h"
#include "app_customercmd.h"
#include "nwy_loc.h"
//#include "aes.h"
//串口指令列表
const CMDTABLE atcmdtable[] =
{
    {AT_SMS_CMD, "SMS"},
    {AT_FMPC_NMEA_CMD, "FMPC_NMEA"},
    {AT_FMPC_BAT_CMD, "FMPC_BAT"},
    {AT_FMPC_GSENSOR_CMD, "FMPC_GSENSOR"},
    {AT_FMPC_ACC_CMD, "FMPC_ACC"},
    {AT_FMPC_GSM_CMD, "FMPC_GSM"},
    {AT_FMPC_RELAY_CMD, "FMPC_RELAY"},
    {AT_FMPC_LDR_CMD, "FMPC_LDR"},
    {AT_FMPC_ADCCAL_CMD, "FMPC_ADCCAL"},
    {AT_FMPC_CSQ_CMD, "FMPC_CSQ"},
    {AT_ZTSN_CMD, "ZTSN"},
    {AT_IMEI_CMD, "IMEI"},
    {AT_FMPC_IMSI_CMD, "FMPC_IMSI"},
    {AT_FMPC_CHKP_CMD, "FMPC_CHKP"},
    {AT_FMPC_CM_CMD, "FMPC_CM"},
    {AT_FMPC_CMGET_CMD, "FMPC_CMGET"},
    {AT_DEBUG_CMD, "DEBUG"},
    {AT_NMEA_CMD, "NMEA"},

};
//获取指令
int16_t getatcmdid(uint8_t *cmdstr)
{
    uint16_t i = 0;
    for (i = 0; i < AT_MAX_CMD; i++)
    {
        if (my_strpach((char *)atcmdtable[i].cmdstr, (const char *)cmdstr))
            return atcmdtable[i].cmdid;
    }
    return -1;
}
//*************************************************************************
void appSimTest(void)
{
    nwy_sim_status sim_status = NWY_SIM_STATUS_NOT_INSERT;
    sim_status = nwy_sim_get_card_status();
    switch (sim_status)
    {
        case NWY_SIM_STATUS_READY:
            LogPrintf(DEBUG_FACTORY, "SIM card ready\r\n");
            break;
        case NWY_SIM_STATUS_NOT_INSERT:
            LogPrintf(DEBUG_FACTORY, "SIM card not insert\r\n");
            break;
        case NWY_SIM_STATUS_PIN1:
            LogPrintf(DEBUG_FACTORY, "SIM card lock pin\r\n");
            break;
        case NWY_SIM_STATUS_PUK1:
            LogPrintf(DEBUG_FACTORY, "SIM card lock puk\r\n");
            break;
        case NWY_SIM_STATUS_BUSY:
            LogPrintf(DEBUG_FACTORY, "SIM card busy\r\n");
            break;
        default:
            break;
    }

}
//*************************************************************************
void appWifiScanTest(void)
{
    int i = 0;
    nwy_wifi_scan_list_t scan_list;
    memset(&scan_list, 0, sizeof(scan_list));
    nwy_wifi_scan(&scan_list);
    for (i = 0; i < scan_list.num; i++)
    {
        LogPrintf(DEBUG_FACTORY, "AP MAC:%02x:%02x:%02x:%02x:%02x:%02x",
                  scan_list.ap_list[i].mac[5], scan_list.ap_list[i].mac[4], scan_list.ap_list[i].mac[3], scan_list.ap_list[i].mac[2],
                  scan_list.ap_list[i].mac[1], scan_list.ap_list[i].mac[0]);
        LogPrintf(DEBUG_FACTORY, ",channel = %d", scan_list.ap_list[i].channel);
        LogPrintf(DEBUG_FACTORY, ",rssi = %d\r\n", scan_list.ap_list[i].rssival);
    }
    if (i == 0)
    {
        LogPrintf(DEBUG_FACTORY, "Wifi scan nothing\r\n");
    }
}
//*************************************************************************
void appVirtualAtTest(char *cmdstr)
{
    int ret = -1;
    static char resp[256] = {0};
    static nwy_at_info at_cmd;
    memset(&at_cmd, 0, sizeof(nwy_at_info));
    at_cmd.length = strlen(cmdstr);
    memcpy(at_cmd.at_command, cmdstr, at_cmd.length);
    LogPrintf(DEBUG_FACTORY, "CMD(%d):%s", at_cmd.length, cmdstr);
    memset(resp, 0, sizeof(resp));
    ret =  nwy_sdk_at_cmd_send(&at_cmd, resp, 30);
    if (ret == NWY_SUCESS)
    {
        LogPrintf(DEBUG_FACTORY, "CmdRespon:%s\r\n", resp);
    }
    else if (ret == NWY_AT_GET_RESP_TIMEOUT)
    {
        LogMessage(DEBUG_FACTORY, "Cmd TimeOut\r\n");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "Cmd Error\r\n");
    }
}

//*************************************************************************
void appADCTest(uint8_t c, uint8_t s)
{
    int adc;
    LogPrintf(DEBUG_FACTORY, "Channel %d , level %d\r\n", c, s);
    adc = nwy_adc_get(c, s);
    LogPrintf(DEBUG_FACTORY, "ADC:%d\r\n", adc);
}
//*************************************************************************

void appPmTest(uint8_t a, uint16_t b, uint8_t c)
{
    int ret;
    if (c)
    {
        LogPrintf(DEBUG_FACTORY, "PMU:%d\r\n", a);
        ret = nwy_subpower_switch(a, true, false);
        LogPrintf(DEBUG_FACTORY, "nwy_subpower_switch ret= %d\r\n", ret);
        ret = nwy_set_pmu_power_level(a, b);
        LogPrintf(DEBUG_FACTORY, "nwy_set_pmu_power_level(%d) ret= %d\r\n", b, ret);
    }
    else
    {

        LogPrintf(DEBUG_FACTORY, "PMU:%d\r\n", a);
        ret = nwy_subpower_switch(a, false, false);
        LogPrintf(DEBUG_FACTORY, "nwy_subpower_switch ret= %d\r\n", ret);
    }
}
//*************************************************************************

static void gpiocallback(int param)
{
    LogPrintf(DEBUG_FACTORY, "gpiocallback===>%d\r\n", param);
}
void appGpioTest(uint8_t read, uint8_t gpio, uint8_t outin, uint8_t value, uint8_t interrupt)
{
    int ret;
    if (interrupt == 0)
    {
        if (read == 0)
        {
            if (outin == 1)
            {
                ret = nwy_gpio_set_direction(gpio, 1);
                LogPrintf(DEBUG_FACTORY, "Set gpio %d ouput %s\r\n", gpio, ret > 0 ? "Success" : "Fail");
                ret = nwy_gpio_set_value(gpio, value);
                LogPrintf(DEBUG_FACTORY, "Set gpio %d ouput %s %s\r\n", gpio, value ? "High" : "Low", ret > 0 ? "Success" : "Fail");
            }
            else
            {
                ret = nwy_gpio_set_direction(gpio, 0);
                LogPrintf(DEBUG_FACTORY, "Set gpio %d input %s\r\n", gpio, ret > 0 ? "Success" : "Fail");
            }
        }
        else
        {

            ret = nwy_gpio_get_value(gpio);
            LogPrintf(DEBUG_FACTORY, "read gpio %d, state=%d\r\n", gpio, ret);

        }
    }
    else
    {
        nwy_close_gpio(gpio);
        nwy_gpio_open_irq_enable(gpio);
        ret = nwy_open_gpio_irq_config(gpio, nwy_irq_rising_falling, gpiocallback);
        LogPrintf(DEBUG_FACTORY, "Open gpio %d irg %s\r\n", gpio, ret > 0 ? "success" : "fail");
    }
}

//*************************************************************************


void appVoiceTest(uint8_t no, uint8_t onoff)
{
    char info[128];
    switch (no)
    {
        case 1:
            LogPrintf(DEBUG_FACTORY, "nwy_voice_setvolte(0,%d)\r\n", onoff);
            nwy_voice_setvolte(0, onoff);
            break;
        case 2:
            nwy_voice_call_start(0, "18814125495");
            break;
        case 3:
            nwy_voice_call_end(0);
            break;
        case 4:
            memset(info, 0, 128);
            nwy_get_voice_callerid(info);
            LogPrintf(DEBUG_FACTORY, "caller id:%s\r\n", info);
            break;
    }
}
//*************************************************************************

void sendMessagef(void *parm)
{
    int ret;
    nwy_sms_info_type_t sms;
    strcpy((char *)sms.phone_num, (char *) "18814125495");
    strcpy((char *)sms.msg_contxet, (char *)"hello");
    sms.msg_context_len = strlen((char *)sms.msg_contxet);
    sms.msg_format = NWY_SMS_MSG_GSM7_DCS;
    LogPrintf(DEBUG_FACTORY, "SendMessage <%s>[%d]:%s\r\n", sms.phone_num, sms.msg_context_len, sms.msg_contxet);
    ret = nwy_sms_send_message(&sms);
    if (ret == 0)
    {
        LogMessage(DEBUG_FACTORY, "Send message success\r\n");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "Send message fail\r\n");
    }

}
//*************************************************************************


void appSmsTest(uint8_t no, uint8_t *tel, uint8_t stype)
{
    int ret;
    nwy_sms_storage_type_e type;
    nwy_sms_recv_info_type_t sms_data;
    static nwy_osiTimer_t *messagetimer = NULL;
    switch (no)
    {
        case 1:
            if (messagetimer == NULL)
            {
                messagetimer = nwy_timer_init(myAppEventThread, sendMessagef, NULL);
            }
            nwy_start_timer(messagetimer, 1000);
            break;

        case 2:
            LogPrintf(DEBUG_FACTORY, "Set new report option 2,2\r\n");
            nwy_set_report_option(2, 2, 0, 0, 0);
            break;
        case 3:
            type = nwy_sms_get_storage();
            LogPrintf(DEBUG_FACTORY, "Old storage:%d\r\n");
            type = stype;
            ret = nwy_sms_set_storage(type);
            if (ret == 0)
            {
                LogPrintf(DEBUG_FACTORY, "Set storage:%d OK\r\n", type);
            }
            else
            {

                LogPrintf(DEBUG_FACTORY, "Set storage:%d Fail\r\n", type);
            }
            break;
        case 4:
            ret = nwy_sms_delete_message_by_type(NWY_SMS_MSG_DFLAG_ALL, NWY_SMS_STORAGE_TYPE_SM);
            if (ret == 0)
            {
                LogPrintf(DEBUG_FACTORY, "Delete message all ok\r\n");
            }
            else
            {

                LogPrintf(DEBUG_FACTORY, "Delete message all fail\r\n");
            }
            break;
        case 5:
            memset(&sms_data, 0, sizeof(sms_data));
            nwy_sms_recv_message(&sms_data);
            LogPrintf(DEBUG_FACTORY, "Tel[%d]:%s,Data[%d]:%s,Mt:%d,ShortId:%d,DCS:%d,Index:%d\r\n", sms_data.oa_size, sms_data.oa,
                      sms_data.nDataLen, sms_data.pData, sms_data.cnmi_mt, sms_data.nStorageId, sms_data.dcs, sms_data.nIndex);
            break;
        case 6:
            LogPrintf(DEBUG_FACTORY, "Read Message Index:%d\r\n", stype);
            memset(&sms_data, 0, sizeof(sms_data));
            nwy_sms_read_message(stype, &sms_data);
            LogPrintf(DEBUG_FACTORY, "Tel[%d]:%s,Data[%d]:%s,Mt:%d,ShortId:%d,DCS:%d,Index:%d\r\n", sms_data.oa_size, sms_data.oa,
                      sms_data.nDataLen, sms_data.pData, sms_data.cnmi_mt, sms_data.nStorageId, sms_data.dcs, sms_data.nIndex);
            break;
    }
}
//*************************************************************************

void appBleTest(uint8_t no)
{
    int ret;
    switch (no)
    {
        case 1:
            ret = nwy_read_ble_status();
            LogPrintf(DEBUG_FACTORY, "Ble status:%d\r\n", ret);
            break;
        case 2:
            ret = nwy_ble_set_device_name((char *)&sysparam.SN);
            LogPrintf(DEBUG_FACTORY, "nwy_ble_set_device_name:%d\r\n", ret);
            break;
    }
}
//*************************************************************************
#define AUDIO_BUF_SIZE		(200*1024)
static uint8_t *audioSave = NULL;
static uint32_t audioLen = 0;

static int audioCallBack(unsigned char *pdata, unsigned int len)
{
    if ((audioLen + len) < (AUDIO_BUF_SIZE))
    {
        memcpy(audioSave + audioLen, pdata, len);
        audioLen += len;
    }
    else
    {
        LogPrintf(DEBUG_FACTORY, "audioCallBack==>over large %d\r\n", audioLen);
    }
    return 0;
}

static int palyerCallBack(nwy_player_status state)
{
    LogPrintf(DEBUG_FACTORY, "palyerCallBack==>%d\r\n", state);
    return 0;
}

void audioPlay(uint8_t sampletype)
{
    int read_index = 0;
    int result = 0;
    int tmp_buf_size = 320;

    if (sampletype == 0)
        tmp_buf_size = 320;
    else if (sampletype == 1)
        tmp_buf_size = 640;

    LogMessage(DEBUG_FACTORY, "Begin play\r\n");
    nwy_audio_player_open(palyerCallBack);
    while (read_index < audioLen)
    {
        result = nwy_audio_player_play(&audioSave[read_index], tmp_buf_size);
        read_index += tmp_buf_size;
        if (result != 0)
            LogMessage(DEBUG_FACTORY, "play error\r\n");
    }
    nwy_audio_player_stop();
    nwy_audio_player_close();
    LogMessage(DEBUG_FACTORY, "Stop play\r\n");
}

void appAudioTest(uint8_t no, uint8_t t1, uint8_t t2)
{
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    static uint8_t sampletype = 0;
    static char amrFileName[25];
    switch (no)
    {
        case 1:
            sampletype = 0;
            nwy_set_poc_sampleRate(8000);
            LogMessage(DEBUG_FACTORY, "set sample rate 8000\r\n");
            break;
        case 2:
            sampletype = 1;
            nwy_set_poc_sampleRate(16000);
            LogMessage(DEBUG_FACTORY, "set sample rate 16000\r\n");
            break;
        case 3:
            if (audioSave == NULL)
            {
                LogMessage(DEBUG_FACTORY, "malloc audio buff\r\n");
                audioSave = (uint8_t *)malloc(AUDIO_BUF_SIZE);
            }
            if (audioSave == NULL)
            {
                LogMessage(DEBUG_FACTORY, "Audio buff is null\r\n");
                return ;
            }
            LogMessage(DEBUG_FACTORY, "Begin record\r\n");
            getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
            sprintf(amrFileName, "%02d%02d%02d%02d%02d%02d", year % 100, month, date, hour, minute, second);
            audioLen = 0;
            nwy_audio_recorder_open(audioCallBack);
            nwy_audio_recorder_start();
            break;
        case 4:

            nwy_audio_recorder_stop();
            nwy_audio_recorder_close();
            LogPrintf(DEBUG_FACTORY, "Record size:%d\r\n", audioLen);
            LogMessage(DEBUG_FACTORY, "Stop record\r\n");
            break;
        case 6:
            audioPlay(sampletype);
            break;
        case 7:
            LogPrintf(DEBUG_FACTORY, "Type :%d ,Quality :%d\r\n", t1, t2);
            nwy_audio_set_record_format(t1, t2);
            break;
        case 8:
            setUploadFile((uint8_t *)amrFileName, audioLen, audioSave);
            break;
    }
}

void gpiofuntest(uint8_t no, uint8_t onoff)
{}
//*************************************************************************

//void aestest(void)
//{
//    uint8_t enlen, declen;
//    char enbuf[20];
//    char respon[100];
//    strcpy(enbuf, "123#############");
//    enlen = 16;
//    LogPrintf(DEBUG_FACTORY, "Enc:%s\r\n", enbuf);
//    aes(enbuf, enlen, AESKEY);
//    changeByteArrayToHexString(enbuf, respon, enlen);
//    respon[enlen * 2] = 0;
//    LogPrintf(DEBUG_FACTORY, "Dec:%s\r\n", respon);
//	deAes(enbuf,enlen,AESKEY);
//	enbuf[16]=0;
//    LogPrintf(DEBUG_FACTORY, "Dec:%s\r\n", enbuf);
//}
void protocoltest(void)
{
    char pro[20] = {0x78, 0x78, 0x05, 0x01, 0x00, 0x00, 0xC8, 0x55, 0x0D, 0x0A, 0x78, 0x78, 0x05};
    char pro2[20] = {0x79, 0x79, 0x00, 0x05, 0x52, 0x00, 0x00, 0xFF, 0xF3, 0x0D, 0x0A, 0x72, 0x71};
    protocolReceivePush(0, pro, 13);
    protocolReceivePush(0, pro2, 12);
}

extern uint16_t threadTickMs;
void atCmdDebugParase(uint8_t *buf, uint16_t len)
{
    int ret;
    char atcmdbuf[50];
    ITEM item;
    parserInstructionToItem(&item, buf, len);
    if (strstr(item.item_data[0], "SUSPEND") != NULL)
    {
        sysinfo.taskSuspend = 1;
        LogMessage(DEBUG_FACTORY, "Suspend all task\r\n");
    }
    else if (strstr(item.item_data[0], "RESUME") != NULL)
    {
        sysinfo.taskSuspend = 0;
        LogMessage(DEBUG_FACTORY, "Resume all task\r\n");
    }
    else if (strstr(item.item_data[0], "DELPARAM") != NULL)
    {

        if (nwy_sdk_fexist("param.save"))
        {
            if (nwy_sdk_file_unlink("param.save") == 0)
            {
                LogMessage(DEBUG_FACTORY, "Delete param.save success\r\n");
            }
        }
        else
        {
            LogMessage(DEBUG_FACTORY, "no param file\r\n");
        }
    }
    else
    {
        ret = atoi(item.item_data[0]);
        switch (ret)
        {
            case 10:
                appBleTest(atoi(item.item_data[1]));
                break;
            case 12:
                appGpioTest(0, atoi(item.item_data[1]), 0, 0, 0);
                break;
            case 13:
                appSimTest();
                break;
            case 14:
                appWifiScanTest();
                break;
            case 15:
                sprintf(atcmdbuf, "%s\r\n", item.item_data[1]);
                appVirtualAtTest(atcmdbuf);
                break;
            case 16:
                appADCTest(atoi(item.item_data[1]), atoi(item.item_data[2]));
                break;
            case 17:
                appPmTest(atoi(item.item_data[1]), atoi(item.item_data[2]), atoi(item.item_data[3]));
                break;
            case 18:
                appGpioTest(0, atoi(item.item_data[1]), 1, atoi(item.item_data[2]), 0);
                break;
            case 19:
                appGpioTest(1, atoi(item.item_data[1]), 0, 0, 0);
                break;
            case 20:
                gsensorConfig(atoi(item.item_data[1]));
                break;
            case 21:
                readInterruptConfig();
                break;
            case 22:
                appVoiceTest(atoi(item.item_data[1]), atoi(item.item_data[2]));
                break;
            case 23:
                appSmsTest(atoi(item.item_data[1]), (uint8_t *) item.item_data[2], atoi(item.item_data[3]));
                break;
            case 24:
                sysinfo.GPSRequest = 0;
                break;
            case 25:
                appGpioTest(0, atoi(item.item_data[1]), 0, 0, 1);
                break;
            case 26:
                appAudioTest(atoi(item.item_data[1]), atoi(item.item_data[2]), atoi(item.item_data[3]));
                break;
            case 27:
                gpiofuntest(atoi(item.item_data[1]), atoi(item.item_data[2]));
                break;
            case 28:
                ret = nwy_audio_get_handset_vol();
                LogPrintf(DEBUG_FACTORY, "handset vol=%d\r\n", ret);
                break;
            case 29:
                ret = atoi(item.item_data[1]);
                LogPrintf(DEBUG_FACTORY, "set handset vol=%d\r\n", ret);
                nwy_audio_set_handset_vol(ret);
                break;
            case 30:
                protocoltest();
                break;
            case 31:
                LogPrintf(DEBUG_ALL, "set nmea formate to 0x%X\r\n", atoi(item.item_data[1]));
                if (nwy_loc_nmea_format_mode(3, atoi(item.item_data[1])))
                {
                    LogMessage(DEBUG_ALL, "set success\r\n");
                }
                else
                {
                    LogMessage(DEBUG_ALL, "set fail\r\n");
                }
                break;
            default:
                sysinfo.logmessage = ret;
                LogPrintf(DEBUG_NONE, "Debug LEVEL:%d OK\r\n", sysinfo.logmessage);
                break;

        }
    }

}

static void atCmdFMPCnmeaParase(uint8_t *buf, uint16_t len)
{
    if (my_strstr((char *)buf, "ON", len))
    {
        LogMessage(DEBUG_FACTORY, "NMEA ON OK\r\n");
        sysinfo.nmeaoutputonoff = 1;
        gpsRequestSet(GPS_REQUEST_DEBUG_CTL);
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "NMEA OFF OK\r\n");
        gpsRequestClear(GPS_REQUEST_DEBUG_CTL);
        sysinfo.GPSRequest = 0;
        sysinfo.nmeaoutputonoff = 0;
    }
}
static void atCmdNmeaParase(uint8_t *buf, uint16_t len)
{
    if (strstr((char *)buf, "ON") != NULL)
    {
        sysinfo.nmeaoutputonoff = 1;
        LogMessage(DEBUG_FACTORY, "NMEA OPEN\r\n");
    }
    else
    {
        sysinfo.nmeaoutputonoff = 0;
        LogMessage(DEBUG_FACTORY, "NMEA CLOSE\r\n");
    }
}

static void atCmdFMPCrelayParase(uint8_t *buf, uint16_t len)
{
    ITEM item;
    parserInstructionToItem(&item, buf, len);
    if (strstr(item.item_data[0], "OFF") != NULL)
    {
        LogMessage(DEBUG_FACTORY, "Relay OFF OK\r\n");
    }
    else if (strstr(item.item_data[0], "ON") != NULL)
    {
        LogMessage(DEBUG_FACTORY, "Relay ON OK\r\n");
    }
}

static void atCmdFMPCbatParase(void)
{
    getBatVoltage();
    LogPrintf(DEBUG_FACTORY, "Vbat: %.3fv\r\n", sysinfo.outsidevoltage);
}

static void atCmdFMPCgsensorParase(void)
{
    read_gsensor_id();
}

static void atCmdFMPCaccParase(void)
{
    LogPrintf(DEBUG_FACTORY, "ACC is %s\r\n", getTerminalAccState() > 0 ? "ON" : "OFF");
}


static void atCmdFMPCgsmParase(void)
{
    if (getRegisterState())
    {
        LogMessage(DEBUG_FACTORY, "GSM SERVICE OK\r\n");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "GSM SERVICE FAIL\r\n");
    }
}

static void atCmdFmpcAdccalParase(void)
{
    sysinfo.outsidevoltage = nwy_adc_get(NWY_ADC_CHANNEL3, NWY_ADC_SCALE_3V233) / 1000.0;
    sysparam.adccal = 12.0 / sysinfo.outsidevoltage;
    paramSaveAll();
    LogPrintf(DEBUG_FACTORY, "Update the voltage calibration parameter to %f\r\n", sysparam.adccal);

}
static void atCmdFMPCLdrParase(void)
{
}

void atCmdFMPCCSQParse(void)
{
    LogPrintf(DEBUG_FACTORY, "+CSQ: %d,99\r\nOK\r\n", getModuleRssi());
}

void atCmdFmpcCMParse(void)
{
    sysparam.cm = 1;
    paramSaveAll();
    LogMessage(DEBUG_FACTORY, "CM OK\n");

}

void atCmdCmGetParser(void)
{
    if (sysparam.cm == 1)
    {
        LogMessage(DEBUG_FACTORY, "CM OK\r\n");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "CM FAIL\r\n");
    }
}

void atCmdFmpcChkpParse(void)
{
    LogPrintf(DEBUG_FACTORY, "+FMPC_CHKP:%s,%s:%d\r\n", sysparam.SN, sysparam.Server, sysparam.ServerPort);
}



void atCmdZTSNParse(uint8_t *buf, uint16_t len)
{
    //    char IMEI[15];
    //    uint8_t sndata[30];
    //    changeHexStringToByteArray(sndata, buf, len / 2);
    //    decryptSN(sndata, IMEI);
    //    LogPrintf(DEBUG_FACTORY, "Decrypt:%s\r\n", IMEI);
    //    strncpy((char *)sysparam.SN, IMEI, 15);
    //    sysparam.SN[15] = 0;
    //    paramSaveAll();
    //    LogMessage(DEBUG_FACTORY, "Write Sn Ok\r\n");
}

void atCmdIMEIParse(void)
{
    getSimInfo();
    LogPrintf(DEBUG_FACTORY, "ZTINFO:%s:%s:%s\r\n", sysparam.SN, (char *)getModuleIMEI(), EEPROM_VERSION);
}

void atCmdFmpcIMSIParse(void)
{
    LogPrintf(DEBUG_FACTORY, "FMPC_IMSI_RSP OK, IMSI=%s&&%s&&%s\r\n", sysparam.SN, (char *)getModuleIMSI(),
              (char *)getModuleICCID());
}


void atCmdParserFunction(char *buf, uint32_t len)
{
    int ret = 0, cmdlen = 0, cmdid = 0;
    char debug[100];
    char cmdbuf[51];
    //LogMessageWL(DEBUG_FACTORY, (char *)buf, len);
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
                cmdid = getatcmdid((uint8_t *)cmdbuf);
                switch (cmdid)
                {
                    case AT_SMS_CMD:
                        instructionParser((uint8_t *)buf + ret + 1, len - ret - 1, AT_SMS_MODE, NULL, NULL);
                        break;
                    case AT_FMPC_NMEA_CMD :
                        atCmdFMPCnmeaParase((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_FMPC_BAT_CMD :
                        atCmdFMPCbatParase();
                        break;
                    case AT_FMPC_GSENSOR_CMD :
                        atCmdFMPCgsensorParase();
                        break;
                    case AT_FMPC_ACC_CMD :
                        atCmdFMPCaccParase();
                        break;
                    case AT_FMPC_GSM_CMD :
                        atCmdFMPCgsmParase();
                        break;
                    case AT_FMPC_CSQ_CMD:
                        atCmdFMPCCSQParse();
                        break;
                    case AT_FMPC_RELAY_CMD:
                        atCmdFMPCrelayParase((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_FMPC_LDR_CMD:
                        atCmdFMPCLdrParase();
                        break;
                    case AT_FMPC_ADCCAL_CMD:
                        atCmdFmpcAdccalParase();
                        break;
                    case AT_FMPC_IMSI_CMD:
                        atCmdFmpcIMSIParse();
                        break;
                    case AT_FMPC_CHKP_CMD:
                        atCmdFmpcChkpParse();
                        break;
                    case AT_FMPC_CM_CMD:
                        atCmdFmpcCMParse();
                        break;
                    case AT_FMPC_CMGET_CMD:
                        atCmdCmGetParser();
                        break;
                    case AT_NMEA_CMD:
                        atCmdNmeaParase((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_ZTSN_CMD:
                        atCmdZTSNParse((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case AT_IMEI_CMD:
                        atCmdIMEIParse();
                        break;
                    case AT_DEBUG_CMD:
                        atCmdDebugParase((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    default:
                        sprintf(debug, "%s==>unknow cmd:%s\r\n", __FUNCTION__, cmdbuf);
                        LogMessage(DEBUG_FACTORY, debug);
                        break;
                }
            }
        }
        else
        {
            sprintf(debug, "%s==>%s\r\n", __FUNCTION__, "please check you param");
            LogMessage(DEBUG_FACTORY, debug);
        }
    }
    else
    {
        //atCustomerCmdParser(buf, len);
    }
}



