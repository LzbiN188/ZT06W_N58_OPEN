#include "app_socket.h"
#include "app_sys.h"
#include "nwy_sim.h"
#include "nwy_data.h"
#include "nwy_network.h"
#include "nwy_data.h"
#include "nwy_socket.h"
#include "app_net.h"
#include "app_customercmd.h"
#include "app_param.h"
#include "app_task.h"
#include "app_protocol.h"



static SOCKET_INFO socketlist[SOCKET_LIST_MAX];
static NETWORK_INFO	networkInfo;

static void changeNetFsm(NETWORK_FSM fsm);


void socketListInit(void)
{
    memset(&socketlist, 0, sizeof(socketlist));
}
void networkInit(void)
{
    memset(&networkInfo, 0, sizeof(networkInfo));
}

void networkConnectCtl(unsigned char onoff)
{
    networkInfo.networkonoff = onoff;
    LogPrintf(DEBUG_ALL, "%s network\r\n", onoff ? "Enable" : "Disable");
    if (onoff == 0)
    {
        socketDeleteAll();
		netStopDataCall();
		changeNetFsm(CHECK_SIM);
    }
}
int socketAdd(unsigned char index, char *domain, unsigned int port, void (*rxFun)(SOCKET_INFO *socketinfo, char *rxbuf,
              uint16_t len))
{
    if (index > SOCKET_LIST_MAX)
    {
        return -1;
    }
    if (socketlist[index].useFlag != 0)
    {
        return -2;
    }
    if (domain == NULL || port == 0)
    {
        return -3;
    }
    if (networkInfo.networkonoff == 0)
    {
        return -4;
    }
    if (networkInfo.netFsm != CHECK_SOCKET)
    {
        return -5;
    }
    strcpy(socketlist[index].domain, domain);
    socketlist[index].port = port;
    socketlist[index].useFlag = 1;
    socketlist[index].rxFun = rxFun;
    socketlist[index].socketId = -1;
    socketlist[index].index = index;
    LogPrintf(DEBUG_ALL, "socketAdd==>%d\r\n", index);
    return 1;
}

int socketDel(SOCKET_INFO *socketinfo)
{
    int ret = 0;
    if (socketinfo->useFlag != 1)
    {
        LogPrintf(DEBUG_ALL, "socket %d no exit\r\n", socketinfo->socketId);
        return ret;
    }

    if (socketinfo->socketConnect)
    {
        LogMessage(DEBUG_ALL, "socket shutdown\r\n");
        nwy_socket_shutdown(socketinfo->socketId, SHUT_RD);
    }
    if (socketinfo->socketOpen)
    {
        LogMessage(DEBUG_ALL, "socket close\r\n");
        nwy_socket_close(socketinfo->socketId);
    }
    ret = 1;
    LogPrintf(DEBUG_ALL, "Delete Index[%d],SocketId[%d]\r\n", socketinfo->index, socketinfo->socketId);
    memset(socketinfo, 0, sizeof(SOCKET_INFO));
    socketinfo->socketId = -1;
    return ret;
}


void socketDeleteAll(void)
{
    uint8_t i = 0;
    LogMessage(DEBUG_ALL, "Delete all socket\r\n");
    for (i = 0; i < SOCKET_LIST_MAX; i++)
    {
        if (socketlist[i].useFlag)
        {
            LogPrintf(DEBUG_ALL, "Delete index:%d\r\n", i);
            socketClose(socketlist[i].index);
        }
    }
}

int socketClose(uint8_t i)
{
    if (i > SOCKET_LIST_MAX)
        return -1;
    if (socketlist[i].useFlag != 1)
        return -2;
    LogPrintf(DEBUG_ALL, "socketClose==>%d\r\n", i);
    if (socketlist[i].index != NORMAL_LINK)
    {
        if (socketlist[i].socketConnect)
        {
            customerLogPrintf("%d,CLOSE\r\n", socketlist[i].index);
        }
    }
    socketDel(&socketlist[i]);
    return 1;
}

static void changeNetFsm(NETWORK_FSM fsm)
{
    if (fsm == CHECK_SOCKET)
    {
        updateSystemLedStatus(SYSTEM_LED_NETREGOK, 1);
    }
    else
    {
        updateSystemLedStatus(SYSTEM_LED_NETREGOK, 0);
    }
    networkInfo.netFsm = fsm;
    networkInfo.netTick = 0;
}

uint8_t netCheckRegister(void)
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

static void netchangeDataCallFsm(uint8_t fsm)
{
    networkInfo.dataCallFsm = fsm;
}
static void netdataStateCallBack(int hndl,  nwy_data_call_state_t ind_state)
{
    LogPrintf(DEBUG_ALL, "Profile id:%d,State:%d\r\n", hndl, ind_state);
    networkInfo.dataCallState = ind_state;
    if (networkInfo.dataCallState == NWY_DATA_CALL_DISCONNECTED)
    {
        socketDeleteAll();
        changeNetFsm(CHECK_DATACALL);
		netchangeDataCallFsm(DATA_CALL_RELEALSE);
    }
}

void netStopDataCall(void)
{
	//int ret;
    if (networkInfo.dataServerHandle <= 0)
    {
        return;
    }
    //ret=nwy_data_stop_call(networkInfo.dataServerHandle);
	//LogPrintf(DEBUG_ALL, "netStopDataCall==>%s\r\n", ret == 0 ? "Success" : "Fail");
	LogMessage(DEBUG_ALL,"netStopDataCall\r\n");
    nwy_data_relealse_srv_handle(networkInfo.dataServerHandle);
    networkInfo.dataServerHandle = 0;
	networkInfo.dataCallState=NWY_DATA_CALL_INVALID;
    netchangeDataCallFsm(DATA_CALL_GET_SOURSE);
}
uint8_t netCheckDataCall(void)
{
    int ret;
    nwy_data_start_call_v02_t startCallParam;
    uint8_t result = 0;
    static uint8_t dataCallTick = 0;
    static uint8_t getSrvTick = 0;
    switch (networkInfo.dataCallFsm)
    {
        //获取数据资源句柄
        case DATA_CALL_GET_SOURSE:
            if (networkInfo.dataServerHandle <= 0)
            {
                networkInfo.dataServerHandle = nwy_data_get_srv_handle(netdataStateCallBack);
                LogPrintf(DEBUG_ALL, "Create data server handle %s ==>%d\r\n", networkInfo.dataServerHandle > 0 ? "success" : "fail",
                          networkInfo.dataServerHandle);
                if (networkInfo.dataServerHandle <= 0)
                {
                    //连续7次获取失败，直接重启
                    getSrvTick++;
                    if (getSrvTick > 7)
                    {
                        getSrvTick = 0;
                    }
                    break;
                }
                else
                {
                    getSrvTick = 0;
                }

            }
            else
            {
                netchangeDataCallFsm(DATA_CALL_START);
            }
        //开始拨号
        case DATA_CALL_START:

            if (networkInfo.dataCallState != NWY_DATA_CALL_CONNECTED)
            {
                startCallParam.profile_idx = 1;   //profile
                startCallParam.is_auto_recon = 1; //自动重连
                startCallParam.auto_re_max = 0;   //一直重连
                startCallParam.auto_interval_ms = 10000;//重连间隔
                ret = nwy_data_start_call(networkInfo.dataServerHandle, &startCallParam);
                if (ret == 0)
                {
                    result = 1;
                    dataCallTick = 0;
                    networkInfo.dataCallCount = 0;
                    LogPrintf(DEBUG_ALL, "Star data call success\r\n");
                }
                else
                {
                    LogPrintf(DEBUG_ALL, "Star data call fail\r\n");
                    if (dataCallTick++ > 30)
                    {
                        dataCallTick = 0;
                        //数据拨号超时
                        networkInfo.dataCallCount++;
                        netchangeDataCallFsm(DATA_CALL_RELEALSE);
                    }
                }
            }
            else
            {
                LogMessage(DEBUG_ALL, "Data call connected\r\n");
                result = 1;
                dataCallTick = 0;
                networkInfo.dataCallCount = 0;
            }
            break;
        //停止拨号
        case DATA_CALL_STOP:
            ret = nwy_data_stop_call(networkInfo.dataServerHandle);
            LogPrintf(DEBUG_ALL, "Stop call %s\r\n", ret == 0 ? "Success" : "Fail");
            netchangeDataCallFsm(DATA_CALL_RELEALSE);
            break;
        //释放资源
        case DATA_CALL_RELEALSE:
            socketDeleteAll();
            nwy_data_relealse_srv_handle(networkInfo.dataServerHandle);
            LogPrintf(DEBUG_ALL, "Relealse data handle %d\r\n", networkInfo.dataServerHandle);
            networkInfo.dataServerHandle = 0;
            netchangeDataCallFsm(DATA_CALL_GET_SOURSE);
            if (networkInfo.dataCallCount >= 3)
            {
                networkInfo.dataCallCount = 0;
                LogMessage(DEBUG_ALL, "Data Call too much time\r\n");
            }
            else
            {
                //拨号失败，则查卡
                LogMessage(DEBUG_ALL, "Data Call fail,recheck sim\r\n");
                changeNetFsm(CHECK_SIM);
            }
            break;
        default:
            netchangeDataCallFsm(DATA_CALL_GET_SOURSE);
            break;
    }
    return result;
}

static void socketDataRecv(SOCKET_INFO *sockinfo)
{
    char rxbuf[1560];
    char debugstr[257];
    int debuglen;
    int ret;
    ret = nwy_socket_recv(sockinfo->socketId, rxbuf, 1560, 0);
    if (ret > 0)
    {
        debuglen = ret > 128 ? 128 : ret;
        changeByteArrayToHexString((uint8_t *)rxbuf, (uint8_t *)debugstr, (uint16_t) debuglen);
        debugstr[debuglen * 2] = 0;
        LogPrintf(DEBUG_ALL, "Ind%d,Rx[%d]:%s\r\n", sockinfo->index, ret, debugstr);
        if (sockinfo->rxFun != NULL)
        {
            sockinfo->rxFun(sockinfo, rxbuf, ret);
        }
    }
    else if (ret == 0)
    {
        //释放socket资源
        LogPrintf(DEBUG_ALL, "socketDataRecv==>Socket %d close\r\n", sockinfo->index);
        if (sockinfo->index != NORMAL_LINK)
        {
            customerLogPrintf("%d,CLOSED\r\n", sockinfo->index);
        }
        if (sockinfo->index == NORMAL_LINK)
        {
            UpdateStop();
        }
        socketDel(sockinfo);
    }
}


static void searchSocket(int socketid)
{
    uint8_t i;
    for (i = 0; i < SOCKET_LIST_MAX; i++)
    {
        if (socketlist[i].useFlag && socketid == socketlist[i].socketId)
        {
            socketDataRecv(&socketlist[i]);
            break;
        }
    }
}
static void errorSocket(int socketid)
{
    uint8_t i;
    for (i = 0; i < SOCKET_LIST_MAX; i++)
    {
        if (socketid == socketlist[i].socketId)
        {
            if (socketlist[i].index != NORMAL_LINK)
            {
                customerLogPrintf("%d,CLOSED\r\n", socketlist[i].index);

            }
            if (socketlist[i].index == NORMAL_LINK)
            {
                UpdateStop();
            }
            socketDel(&socketlist[i]);
            LogPrintf(DEBUG_ALL, "errorSocket==>%d\r\n", i);
            break;
        }
    }

}

static void socketCallBack(int socketid, nwy_socket_event event)
{
    switch (event)
    {
        case NWY_LWIP_EVENT_ACCEPT:
            LogMessage(DEBUG_ALL, "accept\r\n");
            break;
        case NWY_LWIP_EVENT_SENT:
            LogPrintf(DEBUG_ALL, "Socket :%d,send data\r\n", socketid);
            break;
        case NWY_LWIP_EVENT_RECV:
            searchSocket(socketid);
            break;
        case NWY_LWIP_EVENT_CONNECTED:
            //LogMessage(DEBUG_ALL, "connect\r\n");
            break;
        case NWY_LWIP_EVENT_POLL:
            searchSocket(socketid);
            break;
        case NWY_LWIP_EVENT_ERR:
            errorSocket(socketid);
            break;
    }
}


static int netSocketConnectCheck(SOCKET_INFO *socketinfo)
{
    int result = 0;
    int  on, opt, value;
    char *domainIP;

    //域名解析，获取IP地址
    if (socketinfo->domainIP == 0)
    {
        value = 0;
        //域名解析
        domainIP = NULL;
        domainIP = nwy_gethostbyname1(socketinfo->domain, &value);
        if (domainIP == NULL || strlen(domainIP) == 0)
        {
            socketinfo->dnscount++;
            LogPrintf(DEBUG_ALL, "%s dns fail\r\n", socketinfo->domain);
            if (socketinfo->dnscount > 5)
            {
                socketinfo->dnscount = 0;
                LogMessage(DEBUG_ALL, "dns timeout\r\n");
                result = -1; //dns fail
            }
            return result;
        }
        socketinfo->dnscount = 0;
        LogPrintf(DEBUG_ALL, "DNS IP:%s\r\n", domainIP);
        socketinfo->socketInfo.sin_addr.s_addr = netIpChange(domainIP);
        socketinfo->domainIP = 1;
    }
    //获取socket资源
    if (socketinfo->socketOpen == 0)
    {
        LogMessage(DEBUG_ALL, "socket open...\r\n");
        socketinfo->socketId = nwy_socket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socketinfo->socketId < 0)
        {
            result = -2;
            return result;//open socket fail
        }
        LogPrintf(DEBUG_ALL, "socket open success==>SockId:%d\r\n", socketinfo->socketId);
        socketinfo->socketInfo.sin_len = sizeof(struct sockaddr_in);
        socketinfo->socketInfo.sin_family = AF_INET;
        socketinfo->socketInfo.sin_port = htons(socketinfo->port);
        on = 1;
        opt = 1;
        nwy_socket_setsockopt(socketinfo->socketId, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
        nwy_socket_setsockopt(socketinfo->socketId, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
        if (nwy_socket_set_nonblock(socketinfo->socketId) != 0)
        {
            LogMessage(DEBUG_ALL, "socket set nonblock==>error\r\n");
            result = -3;
            return result;//set socket error
        }
        socketinfo->socketOpen = 1;
    }

    if (socketinfo->socketConnect == 0)
    {
        LogPrintf(DEBUG_ALL, "Connect to %s:%d\r\n", socketinfo->domain, socketinfo->port);
        nwy_socket_connect(socketinfo->socketId, &socketinfo->socketInfo, sizeof(socketinfo->socketInfo));
        LogPrintf(DEBUG_ALL, "Connect Result:%d\r\n", nwy_socket_errno());
        //创建链接15秒超时
        if (socketinfo->socketConnectTick++ > 15)
        {
            socketinfo->socketConnectTick = 0;
            LogMessage(DEBUG_ALL, "TCP Connect Timeout\r\n");
            result = -4;
            return result;//connect tcp error
        }
        //链接创建成功
        if (EISCONN == nwy_socket_errno())
        {
            LogMessage(DEBUG_ALL, "TCP Connect OK\r\n");
            nwy_socket_event_report_reg(socketCallBack);
            socketinfo->socketConnect = 1;
            result = 1;
            socketinfo->socketConnectTick = 0;

            return result;
        }
        //链接创建失败
        if (EINPROGRESS != nwy_socket_errno() && EALREADY != nwy_socket_errno())
        {
            LogMessage(DEBUG_ALL, "Socket create fail\r\n");
            result = -5;
            return result;
        }
    }
    else
    {
        result = 2;
    }
    return result;
}

void socketcheck(void)
{
    int i = 0;
    int ret;
    for (i = 0; i < SOCKET_LIST_MAX; i++)
    {
        if (socketlist[i].useFlag)
        {
            ret = netSocketConnectCheck(&socketlist[i]);
            if (ret == 1)
            {
                if (socketlist[i].index != NORMAL_LINK)
                {
                    customerLogPrintf("+CIPOPEN: <%d,TCP,%s,%d>,OK\r\n", i, socketlist[i].domain, socketlist[i].port);
                }
            }
            else if (ret < 0)
            {
                if (socketlist[i].index == NORMAL_LINK)
                {
                    UpdateStop();
                }
                else
                {
                    customerLogPrintf("+CIPOPEN: <%d,TCP,%s,%d>,ERROR,%d\r\n", i, socketlist[i].domain, socketlist[i].port, -ret);
                    socketDel(&socketlist[i]);
                }
            }
        }
    }
}

int socketSendData(unsigned char link, unsigned char *data, unsigned int len)
{
    int sendlen;
    int ret;
    if (link > SOCKET_LIST_MAX)
    {
        return -1;
    }
    if (socketlist[link].useFlag == 0)
    {
        return -2;
    }

    sendlen = nwy_socket_send(socketlist[link].socketId, data, len, 0);
    LogPrintf(DEBUG_ALL, "RequestSend:%d,SendOk:%d\r\n", len, sendlen);
    ret = 1;
    if (sendlen < 0)
    {
        socketDel(&socketlist[link]);
        ret = -3;
    }
    return ret;
}
void networkConnect(void)
{
    uint8_t csq;
    if (networkInfo.networkonoff == 0)
    {
        return ;
    }
    switch (networkInfo.netFsm)
    {
        case CHECK_SIM:
            if (nwy_sim_get_card_status() == NWY_SIM_STATUS_READY)
            {
                LogMessage(DEBUG_ALL, "Sim OK\r\n");
                changeNetFsm(CHECK_SIGNAL);
            }
            else
            {
                LogMessage(DEBUG_ALL, "no sim card\r\n");
                break;
            }
        case CHECK_SIGNAL:
            nwy_nw_get_signal_csq(&csq);
            if (csq >= 5 && csq <= 31)
            {
                LogMessage(DEBUG_ALL, "Signal OK\r\n");
                changeNetFsm(CHECK_REGISTER);
            }
            else
            {
                LogPrintf(DEBUG_ALL, "Signal fail :%d\r\n", csq);
                break;
            }
        case CHECK_REGISTER:
            if (netCheckRegister())
            {
                LogMessage(DEBUG_ALL, "Register OK\r\n");
                changeNetFsm(CHECK_DATACALL);
            }
            else
            {
                LogMessage(DEBUG_ALL, "Register fail\r\n");
                break;
            }
        case CHECK_DATACALL:
            if (netCheckDataCall())
            {
                LogMessage(DEBUG_ALL, "Data call ok\r\n");
                changeNetFsm(CHECK_SOCKET);
            }
            else
            {
                LogMessage(DEBUG_ALL, "Data call not ready\r\n");
                break;
            }

        case CHECK_SOCKET:
            socketcheck();
            break;
    }
    networkInfo.netTick++;
}

void firmWareRecv(SOCKET_INFO *socketinfo, char *rxbuf, uint16_t len)
{
    protocolReceivePush(NORMAL_LINK, rxbuf, len);
}

void updateFirmware(void)
{
    if (sysinfo.updateStatus == 0)
        return;
    if (socketlist[NORMAL_LINK].useFlag == 0)
    {
        socketAdd(NORMAL_LINK, (char *)sysparam.updateServer, sysparam.updateServerPort, firmWareRecv);
        return ;
    }
    if (socketlist[NORMAL_LINK].socketConnect)
    {
        UpdateProtocolRunFsm();
    }
    else
    {
        LogPrintf(DEBUG_ALL, "wait for update link normal\r\n");
    }
}
