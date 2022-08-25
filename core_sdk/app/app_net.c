#include "app_net.h"
#include "app_sys.h"
#include "app_param.h"
#include "app_protocol.h"
#include "app_task.h"
#include "nwy_sim.h"
#include "nwy_data.h"
#include "nwy_network.h"
#include "nwy_data.h"
#include "nwy_socket.h"
#include "nwy_vir_at.h"
#include "nwy_sms.h"
#include "app_ble.h"
#include "app_protocol_808.h"
#include "app_customercmd.h"
#include "app_socket.h"

static NET_INFO netinfo = {0};

const CMD_STRUCT cmd_table[MAX_NUM] =
{
    {AT_CMD, "ATE0"},
    {NWBTBLEMAC_CMD, "AT+NWBTBLEMAC"},
    {CLIP_CMD, "AT+CLIP"},
    {CFUN_CMD, "AT+CFUN"},
    {CGDCONT_CMD, "AT+CGDCONT"},
    {XGAUTH_CMD, "AT+XGAUTH"},
    {NWBTBLEMAC, "AT+NWBTBLEMAC"},
    {MAX_NUM, "AT"},
};

/*-------------------------------------------------------------------------------------------------------*/

void nwbtblemacParser(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t *rebuf;
    uint16_t relen;
    rebuf = buf;
    relen = len;
    index = my_getstrindex((char *)rebuf, "+NWBTBLEMAC:", relen);
    if (index >= 0)
    {
        rebuf += index + 13;
        relen -= index + 13;
        if (relen >= 17)
        {
            netinfo.MAC[0] = rebuf[0];
            netinfo.MAC[1] = rebuf[1];
            netinfo.MAC[2] = rebuf[3];
            netinfo.MAC[3] = rebuf[4];
            netinfo.MAC[4] = rebuf[6];
            netinfo.MAC[5] = rebuf[7];
            netinfo.MAC[6] = rebuf[9];
            netinfo.MAC[7] = rebuf[10];
            netinfo.MAC[8] = rebuf[12];
            netinfo.MAC[9] = rebuf[13];
            netinfo.MAC[10] = rebuf[15];
            netinfo.MAC[11] = rebuf[16];
            netinfo.MAC[12] = 0;
            LogPrintf(DEBUG_ALL, "BLE MAC:%s\n", netinfo.MAC);
        }
    }
}

static void sendVirtualAT(char *cmdstr)
{
    int ret = -1;
    static nwy_at_info at_cmd;
    static char resp[128] = {0};
    memset(&at_cmd, 0, sizeof(nwy_at_info));
    at_cmd.length = strlen(cmdstr);
    memcpy(at_cmd.at_command, cmdstr, at_cmd.length);
    LogPrintf(DEBUG_ALL, "ATCmd:%s", cmdstr);
    memset(resp, 0, 128);
    ret =  nwy_sdk_at_cmd_send(&at_cmd, resp, NWY_AT_TIMEOUT_MIN);
    if (ret == NWY_SUCESS)
    {
        LogPrintf(DEBUG_ALL, "CmdRespon:%s\r\n", resp);
        nwbtblemacParser((uint8_t *)resp, strlen(resp));
    }
    else if (ret == NWY_AT_GET_RESP_TIMEOUT)
    {
        LogMessage(DEBUG_ALL, "Cmd TimeOut\r\n");
    }
    else
    {
        LogMessage(DEBUG_ALL, "Cmd Error\r\n");
    }
}

/*发送模组指令*/
uint8_t  sendModuleCmd(uint8_t cmd, char *param)
{
    uint8_t i;
    int cmdtype = -1;
    char sendData[128];
    if (cmd >= MAX_NUM)
        return 0;
    for (i = 0; i < MAX_NUM; i++)
    {
        if (cmd == cmd_table[i].cmd_type)
        {
            cmdtype = i;
            break;
        }
    }
    if (cmdtype < 0)
    {
        sprintf(sendData, "sendModuleCmd==>No cmd\n");
        LogMessage(DEBUG_ALL, sendData);
        return 0;
    }
    if (param != NULL && strlen(param) <= 128)
    {
        if (param[0] == '?')
        {
            sprintf(sendData, "%s?\r\n", cmd_table[cmdtype].cmd);

        }
        else
        {
            sprintf(sendData, "%s=%s\r\n", cmd_table[cmdtype].cmd, param);
        }
    }
    else if (param == NULL)
    {
        sprintf(sendData, "%s\r\n", cmd_table[cmdtype].cmd);
    }
    else
    {
        return 0;
    }
    sendVirtualAT(sendData);
    return 1;
}
/*-------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------------*/

uint8_t getSimInfo(void)
{
    nwy_sim_result_type simResult;
    nwy_result_type ret;
    ret = nwy_sim_get_iccid(&simResult);
    if (ret != NWY_RES_OK)
    {
        return 0;
    }
    ret = nwy_sim_get_imsi(&simResult);
    if (ret != NWY_RES_OK)
    {
        return 0;
    }
    ret = nwy_sim_get_imei(&simResult);
    if (ret != NWY_RES_OK)
    {
        return 0;
    }
    strncpy(netinfo.ICCID, (char *)simResult.iccid, 20);
    netinfo.ICCID[20] = 0;
    strncpy(netinfo.IMSI, (char *)simResult.imsi, 15);
    netinfo.IMSI[15] = 0;
    strncpy(netinfo.IMEI, (char *)simResult.nImei, 15);
    netinfo.IMEI[15] = 0;
    LogPrintf(DEBUG_ALL, "ICCID:%s;IMSI:%s;IMEI:%s;\r\n", netinfo.ICCID, netinfo.IMSI, netinfo.IMEI);

    sysinfo.mcc = (netinfo.IMSI[0] - '0') * 100 + (netinfo.IMSI[1] - '0') * 10 + netinfo.IMSI[2] - '0';
    sysinfo.mnc = (netinfo.IMSI[3] - '0') * 10 + netinfo.IMSI[4] - '0';
    LogPrintf(DEBUG_ALL, "MCC=%d,MNC=%d\n", sysinfo.mcc, sysinfo.mnc);

    return 1;
}

/*-------------------------------------------------------------------------------------------------------*/
//获取网络注册状态
uint8_t getRegisterState(void)
{
    uint8_t result = 0;
    int ret;
    nwy_nw_regs_info_type_t reg_info;
    memset(&reg_info, 0, sizeof(nwy_nw_regs_info_type_t));
    ret = nwy_nw_get_register_info(&reg_info);
    if (NWY_RES_OK == ret)
    {
        //数据域
        if (reg_info.data_regs_valid)
        {
            result = 1;
            LogPrintf(DEBUG_ALL, "Data Register==>Reg:%d,Roam:%d,RadioTech:%d\r\n", reg_info.data_regs.regs_state,
                      reg_info.data_regs.roam_state,
                      reg_info.data_regs.radio_tech);
        }
        //语音域
        if (reg_info.voice_regs_valid)
        {
            LogPrintf(DEBUG_ALL, "Voice Register==>Reg:%d,Roam:%d,RadioTech:%d\r\n", reg_info.voice_regs.regs_state,
                      reg_info.voice_regs.roam_state,
                      reg_info.voice_regs.radio_tech);
        }
    }
    return result;
}
/*-------------------------------------------------------------------------------------------------------*/

uint32_t netIpChange(char *ip)
{
    char number[5];
    uint8_t save[4];
    uint8_t iplen, i, j, k, val;
    uint32_t ipchange;
    iplen = strlen(ip);
    j = 0;
    k = 0;
    for (i = 0; i < iplen; i++)
    {
        if (ip[i] == '.')
        {
            number[j] = 0;
            j = 0;
            val = atoi(number);
            save[k++] = val;
        }
        else
        {
            number[j++] = ip[i];
            if (i == (iplen - 1))
            {
                number[j] = 0;
                val = atoi(number);
                save[k++] = val;
            }
        }
    }
    ipchange = save[3] << 24;
    ipchange |= save[2] << 16;
    ipchange |= save[1] << 8;
    ipchange |= save[0];
    return ipchange;
}

/*-------------------------------------------------------------------------------------------------------*/

void networkCtl(uint8_t onoff)
{

    LogPrintf(DEBUG_ALL, "%s network\r\n", netinfo.network ? "Enable" : "Disable");
}

/*-------------------------------------------------------------------------------------------------------*/

void reConnectServer(void)
{

}


/*-------------------------------------------------------------------------------------------------------*/

char getModuleRssi(void)
{
    uint8_t csq;
    nwy_nw_get_signal_csq(&csq);
    return csq;
}
char *getModuleIMSI(void)
{
    return netinfo.IMSI;
}
char *getModuleICCID(void)
{

    return netinfo.ICCID;
}
char *getModuleIMEI(void)
{
    return netinfo.IMEI;
}
char *getModuleMAC(void)
{
    return netinfo.MAC;
}

/*发送TCP数据*/
void sendDataToServer(uint8_t link, uint8_t *data, uint16_t len)
{
    socketSendData(link, data, len);
}

/*发送短信*/
uint8_t sendShortMessage(char *tel, char *content, uint16_t len)
{
    int ret;
    nwy_sms_info_type_t sms;
    strcpy((char *)sms.phone_num, tel);
    strncpy((char *)sms.msg_contxet, content, len);
    sms.msg_contxet[len] = 0;
    sms.msg_context_len = len;
    sms.msg_format = NWY_SMS_MSG_GSM7_DCS;
    LogPrintf(DEBUG_ALL, "SendSMS <%s>[%d]:%s\r\n", sms.phone_num, sms.msg_context_len, sms.msg_contxet);
    ret = nwy_sms_send_message(&sms);
    if (ret == 0)
    {
        LogMessage(DEBUG_ALL, "Send message success\r\n");
    }
    else
    {
        LogMessage(DEBUG_ALL, "Send message fail\r\n");
    }
    return ret;
}

void setAPN(void)
{
    char param[200];
    sprintf(param, "1,\"IP\",\"%s\"", (char *) sysparam.apn);
    sendModuleCmd(CGDCONT_CMD, param);
}
void setXGAUTH(void)
{
    char param[200];
    sprintf(param, "1,1,\"%s\",\"%s\"", (char *) sysparam.apnuser, (char *) sysparam.apnpassword);
    sendModuleCmd(XGAUTH_CMD, param);
}
void setCLIP(void)
{
    sendModuleCmd(CLIP_CMD, "1");
}

uint8_t isNetProcessNormal(void)
{
    if (netinfo.fsmState == NET_CHECK_NORMAL)
        return 1;
    return 0;
}


void getBleMac(void)
{
    sendModuleCmd(NWBTBLEMAC, "?");
}

char *readModuleIMEI(void)
{
    int ret;
    nwy_sim_result_type simResult;
    ret = nwy_sim_get_imei(&simResult);
    if (ret != NWY_RES_OK)
    {
        netinfo.IMEI[0] = 0;
        return netinfo.IMEI;
    }
    strncpy(netinfo.IMEI, (char *)simResult.nImei, 15);
    netinfo.IMEI[15] = 0;
    return netinfo.IMEI;
}

/*-------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/
/*Double Server*/
/*-------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------*/





