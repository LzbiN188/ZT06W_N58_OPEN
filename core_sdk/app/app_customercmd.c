#include "app_customercmd.h"
#include "app_sys.h"
#include "nwy_sim.h"
#include "nwy_network.h"
#include "app_net.h"
#include "nwy_pm.h"
#include "nwy_ble.h"
#include "app_param.h"
#include "app_task.h"
#include "app_port.h"
#include "app_protocol.h"
#include "app_ble.h"
#include "nwy_audio_api.h"
#include "app_socket.h"
#include "app_customercmd.h"

#define TCP_LIST_SIZE	SOCKET_LIST_MAX

#define TCP_FIFO_SIZE	1560
#define BLE_FIFO_SIZE	256

#define FIFO_MAX_SIZE  	TCP_FIFO_SIZE
#define GPS_MAX_SIZE	1024


static CUSTOMER_CTL cusctl;
static SMS_LIST smslist;


FIFO_DATA_STRUCT  socketDataInfo[TCP_LIST_SIZE];
FIFO_DATA_STRUCT  bleDataInfo;
uint8_t tcpRxFIFO[TCP_LIST_SIZE][TCP_FIFO_SIZE];
uint8_t bleRxFIFO[BLE_FIFO_SIZE];
char gpsOutput[GPS_MAX_SIZE];
uint16_t gpsOutputLen = 0;



/*用户自定义指令集*/
CUSTOMERCMDTABLE custable[] =
{
    {CFUN, "CFUN"},
    {CSQ, "CSQ"},
    {CPIN, "CPIN"},
    {CIMI, "CIMI"},
    {GSN, "GSN"},
    {ENPWRSAVE, "ENPWRSAVE"},
    {NETAPN, "NETAPN"},
    {CIPOPEN, "CIPOPEN"},
    {CIPSEND, "CIPSEND"},
    {CIPCLOSE, "CIPCLOSE"},
    {NETCLOSE, "NETCLOSE"},
    {CIPRXGET, "CIPRXGET"},
    {CNUM, "CNUM"},
    {CPBW, "CPBW"},
    {CPSI, "CPSI"},
    {CMGS, "CMGS"},
    {CMGR, "CMGR"},
    {CGNSS, "CGNSS"},
    {CGNSSNMEA, "CGNSSNMEA"},
    {CTTSPLAY, "CTTSPLAY"},
    {CTTSSTOP, "CTTSSTOP"},
    {CTTSPARAM, "CTTSPARAM"},
    {CBLE, "CBLE"},
    {BLEREN, "BLEREN"},
    {BLESEND, "BLESEND"},
    {BLERXGET, "BLERXGET"},
    {BLESTA, "BLESTA"},
    {SFTP, "SFTP"},
    {CUPDATE, "CUPDATE"},
    {CICCID, "CICCID"},
    {CGSV, "CGSV"},
};
/*识别用户指令ID*/
int16_t getcustomercmdid(uint8_t *cmdstr)
{
    uint16_t i = 0;
    for (i = 0; i < sizeof(custable) / sizeof(custable[0]); i++)
    {
        if (my_strpach((char *)custable[i].cmdstr, (const char *)cmdstr))
            return custable[i].cmdid;
    }
    return -1;
}

static void cfunParser(uint8_t *str, uint16_t len)
{
    uint8_t onoff;
    if (str[0] == '1' || str[0] == '0')
    {
        str[1] = 0;
        onoff = atoi((char *)str);
        customerLogPrintf("+CFUN: %d,OK\r\n", onoff);
        appSendThreadEvent(CFUN_EVENT, onoff);
        //        if (onoff)
        //        {
        //            networkConnectCtl(1);
        //            sendModuleCmd(CFUN_CMD, "1");
        //        }
        //        else
        //        {
        //            networkConnectCtl(0);
        //            sendModuleCmd(CFUN_CMD, "4");
        //        }
    }
    else
    {
        customerLogOut("ERROR\r\n");
    }
}

static void csqParser(void)
{

    uint8_t csq;
    nwy_nw_get_signal_csq(&csq);
    if (csq > 31)
        csq = 99;
    customerLogPrintf("+CSQ: %d,99\r\n", csq);
}


static void cpinParser(void)
{
    nwy_sim_status simstatus = nwy_sim_get_card_status();
    switch (simstatus)
    {
        case NWY_SIM_STATUS_READY:
            customerLogOut("+CPIN: READY\r\n");
            break;
        case NWY_SIM_STATUS_NOT_INSERT:
            customerLogOut("+CPIN: NOT INSERTED\r\n");
            break;
        default:
            customerLogOut("+CPIN: ERROR\r\n");
            break;
    }
}

static void cimiParser(void)
{
    nwy_result_type ret;
    nwy_sim_result_type simResult;
    ret = nwy_sim_get_imsi(&simResult);
    if (ret == NWY_RES_OK)
    {
        customerLogPrintf("+CIMI: %s\r\n", simResult.imsi);
    }
    else
    {
        customerLogOut("+CIMI: ERROR\r\n");
    }

}

static void gsnParser(void)
{
    nwy_result_type ret;
    nwy_sim_result_type simResult;
    ret = nwy_sim_get_imei(&simResult);
    if (ret == NWY_RES_OK)
    {
        simResult.nImei[15] = 0;
        customerLogPrintf("+GSN: %s\r\n", simResult.nImei);
    }
    else
    {
        customerLogOut("+GSN: ERROR\r\n");
    }

}

static void enpwrsaveParser(uint8_t *str, uint16_t len)
{
    uint8_t onoff;
    int ret;
    if (str[0] == '1' || str[0] == '0')
    {
        str[1] = 0;
        onoff = atoi((char *)str);
        ret = nwy_pm_state_set(onoff);
        customerLogPrintf("+ENPWRSAVE: %s,%s\r\n", str, ret == 0 ? "OK" : "ERROR");
    }
    else
    {
        customerLogOut("ERROR\r\n");
    }
}

static void netapnParser(uint8_t *str, uint16_t len)
{
    ITEM item;
    parserInstructionToItem(&item, str, len);

    if (strcmp((char *)sysparam.apn, item.item_data[0]) != 0 || strcmp((char *)sysparam.apnuser, item.item_data[1]) != 0 ||
            strcmp((char *)sysparam.apnpassword, item.item_data[2]) != 0)
    {
        LogMessage(DEBUG_ALL, "update apn\r\n");
        //apn 相同时不设置
        strcpy((char *)sysparam.apn, item.item_data[0]);
        strcpy((char *)sysparam.apnuser, item.item_data[1]);
        strcpy((char *)sysparam.apnpassword, item.item_data[2]);
        paramSaveAll();
        setAPN();
        setXGAUTH();
    }
    customerLogPrintf("+NETAPN: %s,%s,%s,OK\r\n", sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
}

static void customerRxData(SOCKET_INFO *socketInfo, char *rxbuf, uint16_t len)
{
    customerLogPrintf("+CIPRXGET: %d,%d\r\n", socketInfo->index, len);
    socketDataInfo[socketInfo->index].index = socketInfo->index;
    pushRxData(&socketDataInfo[socketInfo->index], (uint8_t *)rxbuf, len);
}


static void cipopenParser(uint8_t *str, uint16_t len)
{
    uint8_t index;
    int ret;
    ITEM item;
    parserInstructionToItem(&item, str, len);
    if (item.item_cnt != 4)
    {
        customerLogOut("+CIPOPEN: ERROR\r\n");
    }
    else
    {

        LogPrintf(DEBUG_ALL, "Index:%s,Domain:%s,Port:%s\r\n", item.item_data[0], item.item_data[2], item.item_data[3]);

        index = atoi(item.item_data[0]);
        if (index >= (SOCKET_LIST_MAX - 1))
        {
            customerLogOut("ERROR\r\n");
            return ;
        }


        ret = socketAdd(atoi(item.item_data[0]), item.item_data[2], atoi(item.item_data[3]), customerRxData);
        if (ret == 1)
        {
            LogMessage(DEBUG_ALL, "Add socket success\r\n");
        }
        else
        {
            customerLogPrintf("+CIPOPEN: <%s,TCP,%s,%s>,ERROR\r\n", item.item_data[0], item.item_data[2], item.item_data[3]);
            LogPrintf(DEBUG_ALL, "Add socket fail,ret=%d\r\n", ret);
        }
    }
}

//AT+CIPSEND=chn,len,asdasdasd
static void cipsendParser(uint8_t *str, uint16_t len)
{
    uint8_t link;
    uint16_t datalen;
    int16_t index;
    uint8_t *rebuf;
    uint16_t relen;
    char buff[5];
    int ret;
    index = getCharIndex(str, len, ',');

    memcpy(buff, str, index);
    buff[index] = 0;
    link = atoi(buff);
    rebuf = str + index + 1;
    relen = len - index - 1;
    index = getCharIndex(rebuf, relen, ',');
    if (index == 0 || index >= (SOCKET_LIST_MAX - 1))
    {
        customerLogOut("ERROR\r\n");
        return ;
    }
    memcpy(buff, rebuf, index);
    buff[index] = 0;
    datalen = atoi(buff);
    rebuf += index + 1;
    relen -= index + 1;
    LogPrintf(DEBUG_ALL, "Send to Index %d\r\n", link);
    ret = socketSendData(link, rebuf, datalen);
    if (ret == 1)
    {
        customerLogPrintf("+CIPSEND: %d,%d,OK\r\n", link, datalen);
    }
    else
    {
        LogPrintf(DEBUG_ALL, "socketSendData ret=%d\r\n", ret);
        customerLogPrintf("+CIPSEND: %d,%d,ERROR\r\n", link, datalen);
    }
}


static void cipcloseParser(uint8_t *str, uint16_t len)
{
    uint8_t index;
    if (len < 1)
    {
        customerLogOut("ERROR\r\n");
        return ;
    }
    index = str[0] - '0';
    if (index >= (SOCKET_LIST_MAX - 1))
    {
        customerLogOut("ERROR\r\n");
        return;
    }

    if (socketClose(str[0] - '0') == 1)
    {
        customerLogPrintf("+CIPCLOSE: %d,OK\r\n", str[0] - '0');
    }
    else
    {
        customerLogPrintf("+CIPCLOSE: %d,ERROR\r\n", str[0] - '0');
    }
}

static void netcloseParser(void)
{
    socketDeleteAll();
    customerLogOut("+NETCLOSE: OK\r\n");
}
static void ciprxgetParser(uint8_t *str, uint16_t len)
{
    uint8_t index;
    ITEM item;
    parserInstructionToItem(&item, str, len);
    index = atoi(item.item_data[0]);
    if (index >= TCP_LIST_SIZE)
    {
        customerLogOut("ERROR\r\n");
        return;
    }
    postRxData(&socketDataInfo[index], atoi(item.item_data[1]));
}


static void cnumParser(void)
{
    customerLogOut("+CNUM: ERROR\r\n");
}
static void cpbwParser(uint8_t *str, uint16_t len)
{
    customerLogOut("+CPBW: ERROR\r\n");
}
static void cpsiParser(void)
{
    nwy_sim_result_type simResult;
    int lac, cid, mcc, mnc;
    nwy_sim_get_lacid(&lac, &cid);
    nwy_sim_get_imsi(&simResult);
    mcc = (simResult.imsi[0] - '0') * 100 + (simResult.imsi[1] - '0') * 10 + simResult.imsi[2] - '0';
    mnc = (simResult.imsi[3] - '0') * 10 + simResult.imsi[4] - '0';
    customerLogPrintf("+CPSI: %d,%d,%d,%d\r\n", mcc, mnc, lac, cid);
}


static void cmgsParser(uint8_t *str, uint16_t len)
{
    char tel[30];
    int index;
    uint8_t *restr;
    uint16_t relen;
    index = getCharIndex(str, len, ',');
    if (index > 0 && index < 30)
    {
        memcpy(tel, str, index);
        tel[index] = 0;
        restr = str + index + 1;
        relen = len - index - 1;
        if (relen - 2 > 0)
        {
            strcpy((char *)cusctl.msgTel, (char *)tel);
            strncpy((char *)cusctl.msgContent, (char *)restr, relen - 2);
            cusctl.msgLen = relen - 2;
            cusctl.msgSend = 1;
        }
        else
        {
            customerLogOut("+CMGS: ERROR\r\n");
        }
    }
    else
    {
        customerLogOut("+CMGS: ERROR\r\n");
    }
}


static void cmgrParser(uint8_t *str, uint16_t len)
{
    SHORTMESSAGE *msg;
    ITEM item;
    uint8_t num, i;
    parserInstructionToItem(&item, str, len);
    num = atoi(item.item_data[0]);
    i = getSmsCount();
    if (num == 0 || i == 0)
    {
        customerLogOut("+CMGR: ERROR\r\n");
        return;
    }
    for (i = 0; i < num; i++)
    {
        msg = readSms();
        if (msg != NULL)
        {
            customerLogPrintf("+CMGR: %s,%d,%s> %s\r\n", msg->tel, strlen((char *)msg->content), msg->date, msg->content);
        }
        else
        {
            return;
        }
    }
}
static void cgnssParser(uint8_t *str, uint16_t len)
{
    uint8_t onoff;
    if (str[0] == '1' || str[0] == '0')
    {
        str[1] = 0;
        onoff = atoi((char *)str);
        if (onoff)
        {
            gpsRequestSet(GPS_REQUEST_CUSTOMER_CTL);
            sysinfo.gpsOutPut = 1;
            customerLogOut("+CGNSS: 1,OK\r\n");
        }
        else
        {
            gpsRequestClear(GPS_REQUEST_CUSTOMER_CTL);
            sysinfo.gpsOutPut = 0;
            customerLogOut("+CGNSS: 0,OK\r\n");
        }
    }
    else
    {
        customerLogOut("ERROR\r\n");
    }

}


static void cttsplayParser(uint8_t *str, uint16_t len)
{

    if (len - 2 > 0)
    {
        len = len > 250 ? 250 : len;
        memcpy(cusctl.ttsbuf, str, len - 2);
        cusctl.ttsbuf[len - 2] = 0;
        cusctl.ttsplay = 1;
    }
    else
    {
        customerLogOut("+CTTSPLAY: ERROR\r\n");
    }
}

static void cttsstopParser(void)
{
    appTTSStop();
    customerLogOut("+CTTSTOP: OK\r\n");
}
static void cttsparamParser(uint8_t *str, uint16_t len)
{
    ITEM item;
    uint8_t vol;
    parserInstructionToItem(&item, str, len);
    vol = atoi(item.item_data[0]);
    if (vol >= 0 && vol <= 100)
    {
        sysparam.vol = vol;
        paramSaveAll();
        nwy_audio_set_handset_vol(sysparam.vol);
        LogPrintf(DEBUG_ALL, "set vol:%d\r\n", vol);
        customerLogOut("+CTTSPARAM: OK\r\n");
    }
    else
    {
        customerLogOut("+CTTSPARAM: ERROR\r\n");
    }
}

static void cbleParser(uint8_t *str, uint16_t len)
{
    uint8_t onoff;
    if (str[0] == '1' || str[0] == '0')
    {
        str[1] = 0;
        onoff = atoi((char *)str);
        if (onoff)
        {
            sysinfo.bleOnoff = 1;
            BLE_PWR_ON;
            customerLogOut("+CBLE: 1,OK\r\n");
        }
        else
        {
            BLE_PWR_OFF;
            sysinfo.bleOnoff = 0;
            customerLogOut("+CBLE: 0,OK\r\n");
        }
    }
    else
    {
        customerLogOut("ERROR\r\n");
    }
}


static void blerenParser(uint8_t *str, uint16_t len)
{
    char bleName[20];
    char debug[100];
    if (len - 2 > 0 && len < 20)
    {
        memcpy(bleName, str, len - 2);
        bleName[len - 2] = 0;
        strcpy((char *)sysparam.blename, bleName);
        paramSaveAll();
        //appBleRestart();

        sprintf(debug, "AT+NAME=%s\r\n", sysparam.blename);
        cusBleCmdSend(debug, strlen(debug));
        //customerLogPrintf("+BLEREN: %s,OK\r\n", bleName);
    }
    else
    {
        customerLogOut("ERROR\r\n");
    }
}


static void blesendParser(uint8_t *str, uint16_t len)
{
    uint8_t restore[6];
    int16_t index, ret;
    uint8_t *rebuf;
    uint16_t relen, datalen;
    index = getCharIndex(str, len, ',');
    if (index > 0 && index < 5)
    {
        memcpy(restore, str, index);
        restore[index] = 0;
        datalen = atoi((char *)restore);
        rebuf = str + index + 1;
        relen = len - index - 1;
        if (relen > datalen)
        {
            ret = appBleSendData(rebuf, datalen);
            if (ret == 0)
            {
                customerLogOut("+BLESEND: ERROR\r\n");
            }
        }
        else
        {
            customerLogOut("ERROR\r\n");
        }
    }
}


static void blerxgetParser(uint8_t *str, uint16_t len)
{
    ITEM item;
    uint16_t readsize;
    parserInstructionToItem(&item, str, len);
    readsize = atoi(item.item_data[0]);
    LogPrintf(DEBUG_ALL, "ble Read size:%d\r\n", readsize);
    postRxData(&bleDataInfo, readsize);
}

static void blestatParser(void)
{
    char debug[30];
    strcpy(debug, "AT+STA\r\n");
    cusBleCmdSend(debug, strlen(debug));
    //    getBleMac();
    //    customerLogPrintf("+BLESTA: %s,%s,NULL,NULL\r\n", getModuleMAC(), sysparam.blename);
}

static void sftpParser(uint8_t *str, uint16_t len)
{
    ITEM item;
    parserInstructionToItem(&item, str, len);
    if (item.item_data[0][0] != 0 && item.item_data[1][0] != 0)
    {
        sysparam.updateServerPort = atoi(item.item_data[1]);
        strcpy((char *)sysparam.updateServer, item.item_data[0]);
        paramSaveAll();
        LogPrintf(DEBUG_ALL, "UpdateServer:%s:%d\r\n", sysparam.updateServer, sysparam.updateServerPort);
        customerLogPrintf("+SFTP: OK\r\n");
    }
    else
    {
        customerLogPrintf("+SFTP: ERROR\r\n");
    }
}

static void cupdateParser(void)
{
    if (sysparam.updateServerPort == 0)
    {
        customerLogPrintf("+CUPDATE: ERROR\r\n");
    }
    else
    {
        sysinfo.updateStatus = 1;
        updateUISInit(0);
        customerLogPrintf("+CUPDATE: OK\r\n");
    }
}

static void ciccidParser(void)
{
    int ret;
    ret = getSimInfo();
    if (ret == 0)
    {
        customerLogPrintf("+CICCID: ERROR\r\n");
    }
    else
    {
        customerLogPrintf("+CICCID: %s\r\n", getModuleICCID());
    }
}

static void cgsvParser(void)
{
    uint8_t i, cnt;
    GPSINFO *gpsinfo;
    gpsinfo = getCurrentGPSInfo();
    cnt = 0;
    for (i = 0; i < 30; i++)
    {
        if (gpsinfo->gpsCn[i] >= 38)
        {
            cnt++;
        }
    }

    for (i = 0; i < 30; i++)
    {
        if (gpsinfo->beidouCn[i] >= 38)
        {
            cnt++;
        }
    }
    if (cnt >= 2)
    {
        customerLogPrintf("+CGSV: OK\r\n");
    }
    else
    {
        customerLogPrintf("+CGSV: ERROR\r\n");
    }
}
/*用户指令解析*/
void atCustomerCmdParser(const char *buf, uint32_t len)
{
    int ret = 0, cmdlen = 0, cmdid = 0;
    char debug[100];
    char cmdbuf[51];
    //LogMessageWL(DEBUG_FACTORY, (char *)buf, len);
    //识别AT+
    if (buf[0] == 'A' && buf[1] == 'T' && buf[2] == '+')
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
                cmdid = getcustomercmdid((uint8_t *)cmdbuf);
                switch (cmdid)
                {
                    case CFUN:
                        cfunParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CSQ:
                        csqParser();
                        break;
                    case CPIN:
                        cpinParser();
                        break;
                    case CIMI:
                        cimiParser();
                        break;
                    case GSN:
                        gsnParser();
                        break;
                    case ENPWRSAVE:
                        enpwrsaveParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case NETAPN:
                        netapnParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CIPOPEN:
                        cipopenParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CIPSEND:
                        cipsendParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CIPCLOSE:
                        cipcloseParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case NETCLOSE:
                        netcloseParser();
                        break;
                    case CIPRXGET:
                        ciprxgetParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CNUM:
                        cnumParser();
                        break;
                    case CPBW:
                        cpbwParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CPSI:
                        cpsiParser();
                        break;
                    case CMGS:
                        cmgsParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CMGR:
                        cmgrParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CGNSS:
                        cgnssParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CGNSSNMEA:
                        break;
                    case CTTSPLAY:
                        cttsplayParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CTTSSTOP:
                        cttsstopParser();
                        break;
                    case CTTSPARAM:
                        cttsparamParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CBLE:
                        cbleParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case BLEREN:
                        blerenParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case BLESEND:
                        blesendParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case BLERXGET:
                        blerxgetParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case BLESTA:
                        blestatParser();
                        break;
                    case SFTP:
                        sftpParser((uint8_t *)buf + ret + 1, len - ret - 1);
                        break;
                    case CUPDATE:
                        cupdateParser();
                        break;
                    case CICCID:
                        ciccidParser();
                        break;
                    case CGSV:
                        cgsvParser();
                        break;
                    default:
                        sprintf(debug, "%s==>unknow cmd:%s\r\n", __FUNCTION__, cmdbuf);
                        LogMessage(DEBUG_FACTORY, debug);
                        cusBleCmdSend((char *)buf, len);
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
}


static CustomerData cusData = {0};

void cusDataInit(void)
{
    cusData.datalen = 0;
}

static void pushCusData(uint8_t *data, uint16_t len)
{
    if (cusData.datalen + len >= CUSTOMER_DATA_SIZE)
    {
        return;
    }
    memcpy(cusData.data + cusData.datalen, data, len);
    cusData.datalen += len;
}

uint8 postCusData(void)
{
    if (sysinfo.outputDelay != 0)
    {
        sysinfo.outputDelay = 0;
        return 0;
    }
    if (cusData.datalen == 0)
    {
        RING_OUT_LOW;
        return 0;
    }
    RING_OUT_HIGH;
    uartCtl(1);
    LogWrite(DEBUG_NONE, (char *)cusData.data, cusData.datalen);
    appUartSend(&usart1_ctl, (uint8_t *)cusData.data, cusData.datalen);
    cusData.datalen = 0;
    return 1;
}

void customerLogOut(char *str)
{
    customerLogOutWl(str, strlen(str));
}
void customerLogOutWl(char *str, uint16_t len)
{
    RING_OUT_HIGH;
    pushCusData((uint8_t *)str, len);
    if (sysinfo.dtrState)
    {
        sysinfo.outputDelay = 0;
        postCusData();
    }
    else if (sysinfo.outputDelay == 0)
    {
        sysinfo.outputDelay = 1;
        //插入一个延时
    }
}


void customerLogPrintf(const char *debug, ...)
{
    static char LOGDEBUG[256];
    va_list args;
    va_start(args, debug);
    vsnprintf(LOGDEBUG, 256, debug, args);
    va_end(args);
    LOGDEBUG[255] = 0;
    customerLogOutWl(LOGDEBUG, strlen(LOGDEBUG));
}

void txSocketData(uint8_t ind, uint8_t *buf, uint16_t len, uint16_t remain)
{
    if (len != 0)
    {
        customerLogPrintf("+CIPRXGET: %d,%d,%d,", ind, len, remain);
        customerLogOutWl((char *)buf, len);
    }
    else
    {
        customerLogPrintf("+CIPRXGET: %d,0,0", ind);
    }
    customerLogOut("\r\n");
}

void txBleData(uint8_t ind, uint8_t *buf, uint16_t len, uint16_t remain)
{
    if (len != 0)
    {
        customerLogPrintf("+BLERXGET: %d,%d,", len, remain);
        customerLogOutWl((char *)buf, len);
    }
    else
    {
        customerLogOut("+BLERXGET: 0,0");
    }
    customerLogOut("\r\n");
}


void socketInfoInit(void)
{
    uint8_t i;
    memset(socketDataInfo, 0, sizeof(socketDataInfo));
    for (i = 0; i < TCP_LIST_SIZE; i++)
    {
        socketDataInfo[i].rxbuf = tcpRxFIFO[i];
        socketDataInfo[i].rxbufsize = TCP_FIFO_SIZE;
        socketDataInfo[i].txhandlefun = txSocketData;
    }

    memset(&bleDataInfo, 0, sizeof(bleDataInfo));

    bleDataInfo.rxbuf = bleRxFIFO;
    bleDataInfo.rxbufsize = BLE_FIFO_SIZE;
    bleDataInfo.txhandlefun = txBleData;

    cusctl.ttsplay = 0;
}


//将接收的数据塞入接收环形队列中
int8_t pushRxData(FIFO_DATA_STRUCT *dataInfo, uint8_t *buf, uint16_t len)
{
    uint16_t spare, sparetoend;
    if (len == 0)
    {
        return -1;
    }
    if (dataInfo->rxend > dataInfo->rxbegin)
    {
        spare = dataInfo->rxbufsize - (dataInfo->rxend - dataInfo->rxbegin);
    }
    else if (dataInfo->rxbegin > dataInfo->rxend)
    {
        spare = dataInfo->rxbegin - dataInfo->rxend;
    }
    else
    {
        spare = dataInfo->rxbufsize;
    }

    if (len >= spare)
    {
        return -1;
    }
    if (dataInfo->rxend >= dataInfo->rxbegin)
    {
        sparetoend = dataInfo->rxbufsize - dataInfo->rxend;
        if (sparetoend > len)
        {
            memcpy(dataInfo->rxbuf + dataInfo->rxend, buf, len);
            dataInfo->rxend = (dataInfo->rxend + len) % dataInfo->rxbufsize;
        }
        else
        {
            memcpy(dataInfo->rxbuf + dataInfo->rxend, buf, sparetoend);
            dataInfo->rxend = (dataInfo->rxend + sparetoend) % dataInfo->rxbufsize;

            if (len - sparetoend == 0)
            {
                return 0;
            }

            memcpy(dataInfo->rxbuf + dataInfo->rxend, buf + sparetoend, len - sparetoend);
            dataInfo->rxend += (len - sparetoend);
        }
    }
    else
    {
        memcpy(dataInfo->rxbuf + dataInfo->rxend, buf, len);
        dataInfo->rxend += len;
    }

    LogPrintf(DEBUG_ALL, "PushBegin:%d,End:%d\r\n", dataInfo->rxbegin, dataInfo->rxend);
    return 0;
}

//查找循环队列中是否有接收的数据
void postRxData(FIFO_DATA_STRUCT *dataInfo	, uint16_t postlen)
{
    uint16_t realsize = 0;
    static uint8_t  buffer[FIFO_MAX_SIZE];
    uint16_t bufferLen;
    uint16_t rebegin = 0, remain = 0;
    if (postlen == 0)
    {
        return ;
    }
    if (dataInfo->txhandlefun == NULL)
    {
        return;
    }
    LogPrintf(DEBUG_ALL, "PostBegin:%d,End:%d\r\n", dataInfo->rxbegin, dataInfo->rxend);
    if (dataInfo->rxbegin == dataInfo->rxend)
    {
        dataInfo->txhandlefun(dataInfo->index, dataInfo->rxbuf + dataInfo->rxbegin, 0, 0);
        return;
    }
    if (dataInfo->rxend > dataInfo->rxbegin)
    {
        realsize = dataInfo->rxend - dataInfo->rxbegin;
        postlen = (postlen > realsize) ? realsize : postlen;
        rebegin = (dataInfo->rxbegin + postlen) % dataInfo->rxbufsize;
        remain = dataInfo->rxend - rebegin;
        LogPrintf(DEBUG_ALL, "Fun1==>rebegin:%d,postLen:%d,remain:%d\r\n", rebegin, postlen, remain);
        dataInfo->txhandlefun(dataInfo->index, dataInfo->rxbuf + dataInfo->rxbegin, postlen, remain);
        dataInfo->rxbegin = rebegin;
    }
    else
    {
        //查找begin后有多少个可读
        realsize = dataInfo->rxbufsize - dataInfo->rxbegin;
        if (postlen > realsize)
        {
            memcpy(buffer, dataInfo->rxbuf + dataInfo->rxbegin, realsize);
            bufferLen = realsize;
            dataInfo->rxbegin = 0;
            postlen -= realsize;
            realsize = dataInfo->rxend;
            if (postlen > realsize)
            {
                memcpy(buffer + bufferLen, dataInfo->rxbuf, realsize);
                bufferLen += realsize;

                LogPrintf(DEBUG_ALL, "Fun3==>rebegin:%d,postLen:%d,remain:%d\r\n", dataInfo->rxend, postlen, remain);
                dataInfo->txhandlefun(dataInfo->index, buffer, bufferLen, 0);
                dataInfo->rxbegin = dataInfo->rxend;


            }
            else
            {
                memcpy(buffer + bufferLen, dataInfo->rxbuf + dataInfo->rxbegin, postlen);
                bufferLen += postlen;
                rebegin = (dataInfo->rxbegin + postlen) % dataInfo->rxbufsize;
                remain = dataInfo->rxend - rebegin;

                LogPrintf(DEBUG_ALL, "Fun4==>rebegin:%d,postLen:%d,remain:%d\r\n", rebegin, postlen, remain);
                dataInfo->txhandlefun(dataInfo->index, buffer, bufferLen, remain);
                dataInfo->rxbegin = rebegin;

            }
        }
        else
        {
            rebegin = (dataInfo->rxbegin + postlen) % dataInfo->rxbufsize;
            remain = dataInfo->rxbufsize - rebegin + dataInfo->rxend;

            LogPrintf(DEBUG_ALL, "Fun2==>rebegin:%d,postLen:%d,remain:%d\r\n", rebegin, postlen, remain);
            dataInfo->txhandlefun(dataInfo->index, dataInfo->rxbuf + dataInfo->rxbegin, postlen, remain);
            dataInfo->rxbegin = rebegin;
        }
    }
}

void customtts(void)
{
    if (cusctl.ttsplay)
    {
        cusctl.ttsplay = 0;
        appTTSPlay((char *)cusctl.ttsbuf);
    }
    else if (cusctl.msgSend)
    {
        cusctl.msgSend = 0;
        if (sendShortMessage((char *)cusctl.msgTel, (char *) cusctl.msgContent, cusctl.msgLen) == 0)
        {
            customerLogOut("+CMGS: OK\r\n");
        }
        else
        {
            customerLogOut("+CMGS: ERROR\r\n");
        }
    }
}


void smsListInit(void)
{
    memset(&smslist, 0, sizeof(smslist));
}

void addSms(uint8_t *tel, uint8_t *date, uint8_t *content, uint16_t contentlen)
{
    if (smslist.smsCount >= MAX_SMS_LIST)
    {
        customerLogPrintf("+CMGR: %d\r\n", smslist.smsCount);
        return;
    }
    strcpy((char *)smslist.list[smslist.smsCount].tel, (char *)tel);
    strcpy((char *)smslist.list[smslist.smsCount].date, (char *)date);
    memcpy((char *)smslist.list[smslist.smsCount].content, (char *)content, contentlen);
    smslist.list[smslist.smsCount].content[contentlen] = 0;
    smslist.smsCount++;
    customerLogPrintf("+CMGR: %d\r\n", smslist.smsCount);
    return ;
}

SHORTMESSAGE *readSms(void)
{
    if (smslist.smsCount == 0)
        return NULL;
    smslist.smsCount--;
    return &smslist.list[smslist.smsCount];
}

uint8_t getSmsCount(void)
{
    return smslist.smsCount;
}

/*----------------------------------------------------------------------------------------*/
//static CustomerData cusBleSendData;

void cusBleSendDataInit(void)
{
    //    cusBleSendData.datalen = 0;
}

void pushCusBleSendData(uint8_t *data, uint16_t len)
{
    //    if (cusBleSendData.datalen + len >= CUSTOMER_DATA_SIZE)
    //    {
    //        return;
    //    }
    //    memcpy(cusBleSendData.data + cusBleSendData.datalen, data, len);
    //    cusBleSendData.datalen += len;
    //postCusBleSendData();
}

void postCusBleSendData(void)
{
    //    char debug[512];
    //    uint16_t len;
    //    if (cusBleSendData.datalen == 0)
    //    {
    //        return;
    //    }
    //    uartCtl(1);
    //    LogMessage(DEBUG_ALL, "Post ble data...\r\n");
    //    //nwy_ble_send_data(cusBleSendData.datalen, (char *)cusBleSendData.data);
    //    strcpy(debug, "AT+SEND=");
    //    len = strlen(debug);
    //    memcpy(debug + len, cusBleSendData.data, cusBleSendData.datalen);
    //    len += cusBleSendData.datalen;
    //    memcpy(debug + len, "\r\n", 2);
    //    len += 2;
    //    cusBleCmdSend(debug, len);
    //    cusBleSendData.datalen = 0;
}

/*----------------------------------------------------------------------------------------*/
/*
customerLogPrintf("+BLERXGET: %d\r\n", realen);
        pushRxData(&bleDataInfo, (uint8_t *)precv, realen);
+RECV: 3132
customerLogPrintf("+BLEREN: %s,OK\r\n", bleName);
+SEND: OK
+BLESTA: A28FDAE4C284,ZZXXTT1

*/


void cusBleRecvParser(char *buf, uint32_t len)
{
    int16 index, relen;
    char *rebuf;
    char debug[512];
    LogMessage(DEBUG_ALL, "========================\r\n");
    LogMessageWL(DEBUG_ALL, (char *)buf, len);
    LogMessage(DEBUG_ALL, "========================\r\n");

    index = my_getstrindex((char *)buf, "+CONNECT:", len);
    if (index >= 0)
    {
        LogMessage(DEBUG_ALL, "Device connected\r\n");
        customerLogOut("+BLECON: 1\r\n");
        appBleSetConnectState(1);
        return;
    }
    index = my_getstrindex((char *)buf, "+NAME:", len);
    if (index >= 0)
    {
        customerLogPrintf("+BLEREN: %s,OK\r\n", sysparam.blename);
        LogMessage(DEBUG_ALL, "Change name success\r\n");
        return;
    }

    index = my_getstrindex((char *)buf, "+DISC:", len);
    if (index >= 0)
    {
        LogMessage(DEBUG_ALL, "Device disconnected\r\n");
        customerLogOut("+BLECON: 0\r\n");
        appBleSetConnectState(0);
        return;
    }
    index = my_getstrindex((char *)buf, "+RECV:", len);
    if (index >= 0)
    {
        rebuf = buf + index + 7;
        relen = len - index - 7;
        index = getCharIndex((uint8_t *)rebuf, relen, '\r');
        if (index >= 0)
        {
            LogPrintf(DEBUG_ALL, "HexLen:%d\r\n", index);
            changeHexStringToByteArray((uint8_t *)debug, (uint8_t *) rebuf, index / 2);
            customerLogPrintf("+BLERXGET: %d\r\n", index / 2);
            pushRxData(&bleDataInfo, (uint8_t *)debug, index / 2);
        }
        return;
    }
    index = my_getstrindex((char *)buf, "+SEND: OK", len);
    if (index >= 0)
    {
        customerLogOut("+BLESEND: OK\r\n");
        return;
    }
    index = my_getstrindex((char *)buf, "+SEND: FAIL", len);
    if (index >= 0)
    {
        customerLogOut("+BLESEND: ERROR\r\n");
        return;
    }


    index = my_getstrindex((char *)buf, "+BLESTA:", len);
    if (index >= 0)
    {
        memcpy(debug, buf, len);
        debug[len] = 0;
        customerLogOut(debug);
        return;
    }
}

void cusBleCmdSend(char *buf, uint16_t len)
{
    BLE_DTR_HIGH;
    uartCtl(1);
    CreateNodeCmd(buf, len);
}

void cusDataSendD(void)
{
    if (cusctl.dataSend == 0)
    {
        return;
    }
    cusctl.dataSend--;
}

uint8_t bleDataIsSend(void)
{
    return cusctl.dataSend;
}
static void atSendCmd(uint8_t *buf, uint8_t len)
{
    char sendBuf[120];
    //char debug[100];
    uint8_t sendLen;
    strcpy(sendBuf, "AT+SEND=");
    sendLen = strlen(sendBuf);
    memcpy(sendBuf + sendLen, buf, len);
    sendLen += len;
    memcpy(sendBuf + sendLen, "\r\n", 2);
    sendLen += 2;
    cusBleCmdSend(sendBuf, sendLen);
    //changeByteArrayToHexString(buf, (uint8_t *)debug, len);
    //debug[len * 2] = 0;
    //LogPrintf(DEBUG_ALL, "BLESEND:%s\r\n", debug);
}

void bleSendData(uint8_t *buf, uint16_t len)
{
    uint16_t i, rlen;
    if (len == 0 || buf == NULL)
        return;
    i = 0;
    do
    {
        rlen = (len - i > 100) ? 100 : len - i;
        atSendCmd(buf + i, rlen);
        i += rlen;
    }
    while (i < len);
}

/*输入输出队列*/

/**
 * @数据输入输出队列
 * */

static NODEDATA *headNode = NULL;

uint8_t CreateNodeCmd(char *data, uint16_t datalen)
{
    NODEDATA *nextnode;
    NODEDATA *currentnode;
    BLE_DTR_HIGH;
    //如果链表头未创建，则创建链表头。
    if (headNode == NULL)
    {
        headNode = malloc(sizeof(NODEDATA));
        if (headNode != NULL)
        {
            headNode->data = NULL;
            headNode->data = malloc(datalen);
            if (headNode->data != NULL)
            {
                memcpy(headNode->data, data, datalen);
                headNode->datalen = datalen;
                headNode->nextnode = NULL;
                return 1;
            }
            else
            {
                free(headNode);
                headNode = NULL;
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    currentnode = headNode;
    do
    {
        nextnode = currentnode->nextnode;
        if (nextnode == NULL)
        {
            nextnode = malloc(sizeof(NODEDATA));
            if (nextnode != NULL)
            {
                nextnode->data = NULL;
                nextnode->data = malloc(datalen);
                if (nextnode->data != NULL)
                {
                    memcpy(nextnode->data, data, datalen);
                    nextnode->datalen = datalen;
                    nextnode->nextnode = NULL;
                    currentnode->nextnode = nextnode;
                    nextnode = nextnode->nextnode;
                }
                else
                {
                    free(nextnode);
                    nextnode = NULL;
                    return 0;
                }
            }
            else
            {
                return 0;
            }
        }

        currentnode = nextnode;
    }
    while (nextnode != NULL);

    return 1;
}

/*输出队列*/

void outPutNodeCmd(void)
{
    NODEDATA *nextnode;
    NODEDATA *currentnode;
    if (headNode == NULL)
        return ;
    currentnode = headNode;
    if (currentnode != NULL)
    {
        nextnode = currentnode->nextnode;
        //数据发送
        BLE_DTR_HIGH;
        cusctl.dataSend = 3;
        appUartSend(&usart2_ctl, (uint8_t *)currentnode->data, currentnode->datalen);
        LogMessageWL(DEBUG_ALL, currentnode->data, currentnode->datalen);
        free(currentnode->data);
        free(currentnode);
    }
    headNode = nextnode;
    if (headNode == NULL)
        BLE_DTR_LOW;
}


void customGpsInPut(char *nmea, uint8_t nmeaLen)
{
    if (nmeaLen + gpsOutputLen >= GPS_MAX_SIZE)
    {
        return;
    }
    memcpy(gpsOutput + gpsOutputLen, nmea, nmeaLen);
    gpsOutputLen += nmeaLen;
}

void customerGpsOutput(void)
{
    customerLogPrintf("+CGNSS: %d,", gpsOutputLen);
    customerLogOutWl(gpsOutput, gpsOutputLen);
    gpsOutputLen = 0;
}


