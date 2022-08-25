#include "app_ble.h"
#include "app_sys.h"
#include "aes.h"
#include "app_task.h"
#include "app_param.h"
#include "app_net.h"
#include "nwy_network.h"
#include "nwy_data.h"
#include "nwy_socket.h"
#include "app_protocol.h"
#include "nwy_ble.h"
#include "app_instructioncmd.h"
#include "app_task.h"
#include "nwy_sim.h"
#include "app_customercmd.h"
static BleServerList bsl;

static void bleServerSnInsert(uint8_t *bsrsn, BLEDEVINFO devinfo);

void appBleInit(void)
{
    bsl.servcount = 0;
    bsl.servr = 0;
    bsl.servw = 0;
}

void appBleRecvParser(uint8_t *buf, uint8_t len)
{
    char dec[256];
    char restore[50];
    char sn[16];
    char ret;
    unsigned char declen = 0, i = 0, datalen = 0, cnt = 0;
    BLEDEVINFO devinfo;
    //LogPrintf(DEBUG_ALL,"BleRx[%d]:%s\r\n",len,buf);
    //instructionParser((uint8_t *)buf,len,BLE_DIRECT_MODE,NULL,NULL);
    ret = dencryptData(dec, &declen, (char *)buf, len);
    if (ret == 0)
    {
        return ;
    }
    dec[declen] = '#';
    declen += 1;
    dec[declen] = 0;

    if (dec[0] == 'R' && dec[1] == 'E' && dec[2] == ':')
    {
        LogMessage(DEBUG_ALL, "BLE instrucion respon\r\n");
        doSendBleResponToNet((uint8_t *)dec, declen);
    }
    else if (dec[0] == 'B' && dec[1] == 'L' && dec[2] == ':')
    {
        instructionParser((uint8_t *)dec + 3, declen - 3, BLE_MODE, NULL, NULL);
    }
    else
    {
        for (i = 0; i < declen; i++)
        {
            if (dec[i] == ',' || dec[i] == '\r' || dec[i] == '\n' || dec[i] == '#')
            {
                if (restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    switch (cnt)
                    {
                        case 1:
                            if (restore[0] == 'S' && restore[1] == 'N' && restore[2] == ':')
                            {
                                strncpy(sn, (char *)restore + 3, 15);
                                sn[15] = 0;
                            }
                            else
                            {
                                LogMessage(DEBUG_ALL, "Invalid ble data\n");
                                return ;
                            }
                            break;
                        case 2:
                            devinfo.modecount = atoi(restore);
                            break;
                        case 3:
                            devinfo.voltage = atof(restore);
                            break;
                        case 4:
                            devinfo.batterylevel = atoi(restore);
                            bleServerSnInsert((uint8_t *)sn, devinfo);
                            break;
                    }
                    restore[0] = 0;
                }
            }
            else
            {
                restore[datalen] = dec[i];
                datalen++;
                if (datalen >= 20)
                {
                    return ;
                }

            }
        }
    }
}



/*往队列中添加新的待处理SN*/
static void bleServerSnInsert(uint8_t *bsrsn, BLEDEVINFO devinfo)
{
    if (bsl.servcount >= SERVERMAX)
        return ;
    strcpy((char *)bsl.servsn[bsl.servw], (char *)bsrsn);
    bsl.devinfo[bsl.servw] = devinfo;
    bsl.servw++;
    bsl.servw = (bsl.servw) % SERVERMAX;
    bsl.servcount++;
    LogPrintf(DEBUG_ALL, "bleServerSnInsert==>%s\r\n", bsrsn);
}

BLEDEVINFO *getBleDevInfo(void)
{
    return &bsl.devinfo[bsl.servr];
}

uint8_t *getBleServSn(void)
{
    return bsl.servsn[bsl.servr];
}


static BleServerRunFsm bleRunInfo;

static uint8_t getDoubleSocketState(void)
{
    uint8_t result = 0;
    int  on, opt, ret, value;
    char *domainIP;
    static struct sockaddr_in socketInfo;
    static uint8_t socketConnectTick = 0;
    static uint8_t dnscount = 0;
    //域名解析，获取IP地址
    if (bleRunInfo.domainIP == 0)
    {
        value = 0;
        //域名解析
        domainIP = NULL;
        domainIP = nwy_gethostbyname1("jzwz.basegps.com", &value);
        if (domainIP == NULL || strlen(domainIP) == 0)
        {
            LogPrintf(DEBUG_ALL, "DNS fail\r\n");
            dnscount++;
            if (dnscount > 5)
            {
                dnscount = 0;
            }
            return result;
        }
        dnscount = 0;
        LogPrintf(DEBUG_ALL, "DNS IP:%s\r\n", domainIP);
        socketInfo.sin_addr.s_addr = netIpChange(domainIP);
        bleRunInfo.domainIP = 1;
    }
    //创建socket资源
    if (bleRunInfo.socketOpen == 0)
    {
        bleRunInfo.socketId = nwy_socket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (bleRunInfo.socketId < 0)
        {
            return result;
        }
        LogPrintf(DEBUG_ALL, "Create socket id:%d\r\n", bleRunInfo.socketId);
        socketInfo.sin_len = sizeof(struct sockaddr_in);
        socketInfo.sin_family = AF_INET;
        socketInfo.sin_port = htons(9998);
        on = 1;
        opt = 1;
        nwy_socket_setsockopt(bleRunInfo.socketId, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
        nwy_socket_setsockopt(bleRunInfo.socketId, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
        if (nwy_socket_set_nonblock(bleRunInfo.socketId) != 0)
        {
            nwy_socket_close(bleRunInfo.socketId);
            LogMessage(DEBUG_ALL, "nwy_socket_set_nonblock==>error\r\n");
            return result;;
        }
        socketConnectTick = 0;
        bleRunInfo.socketOpen = 1;
    }

    if (bleRunInfo.socketConnect == 0)
    {
        LogPrintf(DEBUG_ALL, "Connect to %s:%d\r\n", "jzwz.basegps.com", 9998);
        ret = nwy_socket_connect(bleRunInfo.socketId, &socketInfo, sizeof(socketInfo));
        LogPrintf(DEBUG_ALL, "nwy_socket_connect==>Result:%d\r\n", nwy_socket_errno());
        //创建链接15秒超时
        if (socketConnectTick++ > 15)
        {
            socketConnectTick = 0;
            nwy_socket_shutdown(bleRunInfo.socketId, SHUT_RD);
            nwy_socket_close(bleRunInfo.socketId);
            bleRunInfo.domainIP = 0;
            bleRunInfo.socketOpen = 0;
            LogMessage(DEBUG_ALL, "TCP Connect Timeout\r\n");
            return result;
        }
        //链接创建成功
        if (EISCONN == nwy_socket_errno())
        {
            LogMessage(DEBUG_ALL, "TCP Connect OK\r\n");
            //nwy_socket_event_report_reg(socketEventCallBack);
            bleRunInfo.socketConnect = 1;
            result = 1;
            socketConnectTick = 0;
            return result;
        }
        //链接创建失败
        if (EINPROGRESS != nwy_socket_errno() && EALREADY != nwy_socket_errno())
        {
            bleRunInfo.domainIP = 0;
            bleRunInfo.socketOpen = 0;
            nwy_socket_shutdown(bleRunInfo.socketId, SHUT_RD);
            ret = nwy_socket_close(bleRunInfo.socketId);
            LogPrintf(DEBUG_ALL, "nwy_socket_close==>Result:%d\r\n", ret);
            return result;
        }
    }
    else
    {
        result = 1;
    }
    return result;
}

static void closeDoubleSocket(void)
{
    LogMessage(DEBUG_ALL, "closeDoubleSocket\r\n");
    nwy_socket_shutdown(bleRunInfo.socketId, SHUT_RD);
    nwy_socket_close(bleRunInfo.socketId);
    bleRunInfo.domainIP = 0;
    bleRunInfo.socketOpen = 0;
    bleRunInfo.socketConnect = 0;
}
static void changeBleServFsm(uint8_t fsm)
{
    bleRunInfo.fsm = fsm;
    bleRunInfo.tick = 0;
}
static void receiveSocketData(void)
{


}

void sendDataToDoubleLink(uint8_t *buf, uint16_t len)
{
    int sendlen;
    if (bleRunInfo.socketConnect == 0)
    {
        LogMessage(DEBUG_ALL, "Double socket not ok\r\n");
        return ;
    }
    sendlen = nwy_socket_send(bleRunInfo.socketId, buf, len, 0);
    LogPrintf(DEBUG_ALL, "DRequestSend:%d,SendOk:%d\r\n", len, sendlen);
    if (sendlen < 0)
    {
        bleRunInfo.socketConnect = 0;
    }
}

void bleServerLoginOk(void)
{
    bleRunInfo.loginok = 1;
}

uint8_t bleSendDataProcess(void)
{
    static uint8_t bletick = 0;
    uint8_t result = 0;
    GPSINFO *gpsinfo;
    int lac, cid;
    switch (bleRunInfo.connectFsm)
    {
        case SEND_LBS:
            //获取基站
            lac = 0;
            cid = 0;
            nwy_sim_get_lacid(&lac, &cid);
            sysinfo.lac = lac;
            sysinfo.cid = cid;
            LogPrintf(DEBUG_ALL, "LAC:0x%X,CID:0x%X\r\n", sysinfo.lac, sysinfo.cid);
            sendProtocolToServer(DOUBLE_LINK, PROTOCOL_19, NULL);
            bleRunInfo.connectFsm = WAIT_GPS;
            bletick = 0;
            break;
        case WAIT_GPS:
            //等待GPS定位
            gpsinfo = getCurrentGPSInfo();
            if (gpsinfo->fixstatus)
            {
                sendProtocolToServer(DOUBLE_LINK, PROTOCOL_12, getLastFixedGPSInfo());
                bleRunInfo.connectFsm = END;
            }
            //最长定位180秒
            if (bletick++ > 180)
            {
                bleRunInfo.connectFsm = END;
            }
            break;
        case END:
            result = 1;
            break;;
    }
    return result;
}

void bleServerSnConnect(void)
{
    if (bsl.servcount == 0)
    {
        bleRunInfo.reCount = 0;
        bleRunInfo.tick = 0;
        if (gpsRequestGet(GPS_REQUEST_BLEUPLOAD_CTL))
        {
            gpsRequestClear(GPS_REQUEST_BLEUPLOAD_CTL);
        }
        return ;
    }
    receiveSocketData();
    switch (bleRunInfo.fsm)
    {
        //创建socket
        case BLE_CONNECTSERVER:
            LogMessage(DEBUG_ALL, "bleServerSnConnect==>socket connect\r\n");
            if (getDoubleSocketState())
            {
                bleRunInfo.reCount = 0;
                changeBleServFsm(BLE_LOGIN);
            }
            else
            {
                if (bleRunInfo.tick > 60)
                {
                    bleRunInfo.reCount++;
                    if (bleRunInfo.reCount >= 2)
                    {
                        bsl.servcount = 0;
                    }
                }
                break;
            }
        case BLE_LOGIN:
            //登录服务器
            if (bleRunInfo.loginok == 0)
            {
                if (bleRunInfo.tick % 30 == 0)
                {
                    if (bleRunInfo.reCount > 2)
                    {
                        //无法登录
                        //跳转到异常处理
                        changeBleServFsm(BLE_END);
                        break;
                    }
                    LogMessage(DEBUG_ALL, "Login to server\r\n");
                    sendProtocolToServer(DOUBLE_LINK, PROTOCOL_01, NULL);
                    sendProtocolToServer(DOUBLE_LINK, PROTOCOL_13, NULL);
                    bleRunInfo.reCount++;
                }

                break;
            }
            else
            {
                bleRunInfo.loginok = 0;
                bleRunInfo.connectFsm = SEND_LBS;
                changeBleServFsm(BLE_SENDDATA);
                gpsRequestSet(GPS_REQUEST_BLEUPLOAD_CTL);
            }
            break;
        case BLE_SENDDATA:
            //维持ble链接至少15秒
            if (bleSendDataProcess() == 1 && bleRunInfo.tick > 15)
            {
                changeBleServFsm(BLE_END);
            }

            break;
        case BLE_END:
            //结束
            LogMessage(DEBUG_ALL, "BleDone\n");
            if (bsl.servcount > 0)
            {
                bsl.servr++;
                bsl.servr = bsl.servr % SERVERMAX;
            }
            bsl.servcount--;
            closeDoubleSocket();
            changeBleServFsm(BLE_CONNECTSERVER);
            break;
    }
    bleRunInfo.tick++;
}


uint8_t appBleSendData(uint8_t *buf, uint16_t len)
{
    uint8_t ret = 0;
    if (bleRunInfo.bleConState)
    {
        LogMessage(DEBUG_ALL, "appBleSendData==>sending...\r\n");
        bleSendData(buf, len);
        //pushCusBleSendData(buf,len);
        //nwy_ble_send_data(len, (char *)buf);
        ret = 1;
    }
    else
    {
        LogMessage(DEBUG_ALL, "BLE was disconnect\r\n");
    }
    return ret;
}

void appBleSetConnectState(uint8_t onoff)
{
    bleRunInfo.bleConState = onoff;
}

