#include "app_socket.h"
#include "app_sys.h"
#include "nwy_sim.h"
#include "nwy_data.h"
#include "nwy_network.h"
#include "nwy_data.h"
#include "app_kernal.h"
#include "app_port.h"
#include "stdlib.h"
#include "app_net.h"
#include "app_task.h"
#include "app_param.h"
#include "app_protocol.h"

static socketInfo_s 	socketlist[SOCKET_LIST_MAX];
static networkInfo_s	networkInfo;

/**************************************************
@bref		socket列表初始化
@param
@note
**************************************************/
void socketListInit(void)
{
    memset(&socketlist, 0, sizeof(socketlist));
}
/**************************************************
@bref		网络信息初始化
@param
@note
**************************************************/

void networkInfoInit(void)
{
    memset(&networkInfo, 0, sizeof(networkInfo));
    networkInfo.csqSearchTime = 120;
}

/**************************************************
@bref		是否开启网络
@param
	onoff	1：开		0：关
@note
**************************************************/

void networkConnectCtl(unsigned char onoff)
{
    static uint8_t beforeState = 2;
    if (beforeState == onoff)
    {
        return;
    }
    networkInfo.networkonoff = onoff;
    beforeState = networkInfo.networkonoff;
    LogPrintf(DEBUG_ALL, "%s network", onoff ? "Enable" : "Disable");
    if (onoff == 0)
    {
        netStopDataCall();
        portSetRadio(0);
    }
    else
    {
        portSetRadio(1);
    }
}



/**************************************************
@bref		重设信号搜索时长
@param
@note
**************************************************/

void netResetCsqSearch(void)
{
    networkInfo.csqSearchTime = 120;
}

/**************************************************
@bref		添加socket
@param
	sockId	套接字ID
	domian	服务器域名
	port	端口号
	rxFun	数据接收接口
@return
	1 		添加成功
	<0		添加失败
@note
**************************************************/
int socketAdd(unsigned char sockId, char *domain, unsigned int port, void (*rxFun)(struct SOCK_INFO *socketinfo,
              char *rxbuf, uint16_t len))
{
    if (sockId > SOCKET_LIST_MAX)
    {
        return -1;
    }
    if (socketlist[sockId].useFlag != 0)
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
    if (domain[0] == 0)
    {
        return -6;
    }
    strcpy(socketlist[sockId].domain, domain);
    socketlist[sockId].port = port;
    socketlist[sockId].useFlag = 1;
    socketlist[sockId].rxFun = rxFun;
    socketlist[sockId].socketId = -1;
    socketlist[sockId].index = sockId;
    LogPrintf(DEBUG_ALL, "socketAdd[%d]", sockId);
    return 1;
}
/**************************************************
@bref		删除socket
@param
	socketinfo socket信息
@return
	>0 		成功
	<0		失败
@note
**************************************************/

static int socketDel(socketInfo_s *socketinfo)
{
    int ret = 0;
    if (socketinfo->useFlag == 0)
    {
        LogPrintf(DEBUG_ALL, "socket[%d] not exit", socketinfo->socketId);
        return ret;
    }

    if (socketinfo->socketConnect)
    {
        nwy_socket_shutdown(socketinfo->socketId, SHUT_RD);
    }
    if (socketinfo->socketOpen)
    {
        nwy_socket_close(socketinfo->socketId);
    }
    ret = 1;
    LogPrintf(DEBUG_ALL, "Delete socket[%d],SocketId[%d]", socketinfo->index, socketinfo->socketId);
    memset(socketinfo, 0, sizeof(socketInfo_s));
    socketinfo->socketId = -1;
    return ret;
}
/**************************************************
@bref		删除所有socket
@param
	socketinfo socket信息
@return
	none
@note
**************************************************/

void socketDeleteAll(void)
{
    uint8_t i = 0, flag = 0;
    for (i = 0; i < SOCKET_LIST_MAX; i++)
    {
        if (socketlist[i].useFlag)
        {
            flag = 1;
            LogPrintf(DEBUG_ALL, "Delete socket[%d]", i);
            socketDel(&socketlist[i]);
        }
    }
    if (flag)
    {
        LogMessage(DEBUG_ALL, "Delete all socket done");
    }
}


/**************************************************
@bref		删除某个socket
@param
	sockId 	对应socket的ID
@return
	1		成功
	0		失败
@note
**************************************************/

int socketClose(uint8_t sockId)
{
    if (sockId > SOCKET_LIST_MAX)
        return -1;
    if (socketlist[sockId].useFlag == 0)
        return -2;
    LogPrintf(DEBUG_ALL, "close socket[%d]", sockId);
    socketDel(&socketlist[sockId]);
    return 1;
}
/**************************************************
@bref		切换状态
@param
	fsm		新状态
@return
	none
@note
**************************************************/

static void changeNetFsm(socketFsm_e fsm)
{
    networkInfo.netFsm = fsm;
    networkInfo.netTick = 0;
}
/**************************************************
@bref		网络注册检查
@param
	none
@return
	1		成功
	0		失败
@note
**************************************************/

static uint8_t netCheckRegister(void)
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
            LogPrintf(DEBUG_ALL, "Data Register==>Reg:%d,Roam:%d,RadioTech:%d", reg_info.data_regs.regs_state,
                      reg_info.data_regs.roam_state,
                      reg_info.data_regs.radio_tech);
            portSetApn(sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
        }
        //语音域
        if (reg_info.voice_regs_valid)
        {
            LogPrintf(DEBUG_ALL, "Voice Register==>Reg:%d,Roam:%d,RadioTech:%d", reg_info.voice_regs.regs_state,
                      reg_info.voice_regs.roam_state,
                      reg_info.voice_regs.radio_tech);
        }
    }
    return result;
}

/**************************************************
@bref		数据拨号状态切换
@param
	none
@return
	none
@note
**************************************************/

static void netchangeDataCallFsm(uint8_t fsm)
{
    networkInfo.dataCallFsm = fsm;
}

/**************************************************
@bref		数据拨号状态返回
@param
	hndl
	ind_state
@return
	none
@note
**************************************************/

static void netdataStateCallBack(int hndl,  nwy_data_call_state_t ind_state)
{
    LogPrintf(DEBUG_ALL, "Profile id:%d,State:%d", hndl, ind_state);
    networkInfo.dataCallState = ind_state;
    if (networkInfo.dataCallState == NWY_DATA_CALL_DISCONNECTED)
    {
        LogMessage(DEBUG_ALL, "DataCall Disconnected");
        if (networkInfo.dataCallFsm == DATA_CALL_START)
        {
            netchangeDataCallFsm(DATA_CALL_STOP);
            LogMessage(DEBUG_ALL, "change to datacall stop");
        }
        changeNetFsm(CHECK_SIM);
    }
}


void netStopDataCall(void)
{
    LogMessage(DEBUG_ALL, "netStopDataCall");
    socketDeleteAll();
    if (networkInfo.dataServerHandle > 0)
    {
        nwy_data_relealse_srv_handle(networkInfo.dataServerHandle);
        networkInfo.dataServerHandle = 0;
        networkInfo.dataCallState = NWY_DATA_CALL_INVALID;
        netchangeDataCallFsm(DATA_CALL_GET_SOURSE);
    }

    changeNetFsm(CHECK_SIM);
}
/**************************************************
@bref		数据拨号
@param
	none
@return
	1		成功
	0		失败
@note
**************************************************/

static uint8_t netCheckDataCall(void)
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
                networkInfo.dataCallState = NWY_DATA_CALL_INVALID;
                networkInfo.dataServerHandle = nwy_data_get_srv_handle(netdataStateCallBack);
                LogPrintf(DEBUG_ALL, "Create data server handle %s ==>%d", networkInfo.dataServerHandle > 0 ? "success" : "fail",
                          networkInfo.dataServerHandle);
                if (networkInfo.dataServerHandle <= 0)
                {
                    //连续7次获取失败，直接重启
                    getSrvTick++;
                    if (getSrvTick > 7)
                    {
                        getSrvTick = 0;
                        if (sysparam.simSel == SIM_1 && portSimGet() == SIM_1)
                        {
                            LogPrintf(DEBUG_ALL, "netCheckDataCall ERROR, try to use SIM2");
                            portSimSet(SIM_2);
                        }
                        else if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
                        {
							LogPrintf(DEBUG_ALL, "netCheckDataCall ERROR, try to use SIM1");
							portSimSet(SIM_1);
                        }
                        portSystemReset();
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
                }
                else
                {
                    if (dataCallTick++ > 30)
                    {
                        dataCallTick = 0;
                        //数据拨号超时
                        networkInfo.dataCallCount++;
                        netchangeDataCallFsm(DATA_CALL_STOP);
                    }
                }
                LogPrintf(DEBUG_ALL, "Start datacall %s", ret == 0 ? "success" : "fail");
            }
            else
            {
                LogMessage(DEBUG_ALL, "Datacall connected");
                result = 1;
                dataCallTick = 0;
                networkInfo.dataCallCount = 0;
            }
            break;
        //停止拨号
        case DATA_CALL_STOP:
            socketDeleteAll();
            ret = nwy_data_stop_call(networkInfo.dataServerHandle);
            LogPrintf(DEBUG_ALL, "Stop call %s", ret == 0 ? "Success" : "Fail");
            netchangeDataCallFsm(DATA_CALL_RELEALSE);
            break;
        //释放资源
        case DATA_CALL_RELEALSE:
            nwy_data_relealse_srv_handle(networkInfo.dataServerHandle);
            LogPrintf(DEBUG_ALL, "Relealse data handle %d", networkInfo.dataServerHandle);
            networkInfo.dataServerHandle = 0;
            netchangeDataCallFsm(DATA_CALL_GET_SOURSE);
            if (networkInfo.dataCallCount >= 3)
            {
                networkInfo.dataCallCount = 0;
                LogMessage(DEBUG_ALL, "Data Call too much time");
                if (sysparam.simSel == SIM_1 && portSimGet() == SIM_1)
                {
                    LogPrintf(DEBUG_ALL, "netCheckDataCall ERROR, try to use SIM2");
                    portSimSet(SIM_2);
                }
                else if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
                {
					LogPrintf(DEBUG_ALL, "netCheckDataCall ERROR, try to use SIM1");
					portSimSet(SIM_1);
                }
                portSystemReset();
            }
            else
            {
                //拨号失败，则查卡
                LogMessage(DEBUG_ALL, "recheckout sim");
                changeNetFsm(CHECK_SIM);
            }
            break;
        default:
            netchangeDataCallFsm(DATA_CALL_GET_SOURSE);
            break;
    }
    return result;
}

/**************************************************
@bref		数据接收
@param
	sockinfo	套接字信息
@return
	none
@note
**************************************************/

static void socketDataRecv(socketInfo_s *sockinfo)
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
        LogPrintf(DEBUG_ALL, "socket[%d],Rx[%d]: %s", sockinfo->index, ret, debugstr);
        if (sockinfo->rxFun != NULL)
        {
            sockinfo->rxFun(sockinfo, rxbuf, ret);
        }
    }
    else if (ret == 0)
    {
        //释放socket资源

        LogPrintf(DEBUG_ALL, "socketDataRecv==>Socket[%d] was closed", sockinfo->index);
        if (sockinfo->index == AGPS_LINK)
        {
            agpsRequestClear();
        }
        appSendThreadEvent(THREAD_EVENT_SOCKET_CLOSE, sockinfo->index);
        //socketDel(sockinfo);
    }
}



/**************************************************
@bref		查找套接字信息
@param
	socketid	套接字ID
@return
	none
@note
**************************************************/

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

/**************************************************
@bref		处理套接字错误
@param
	socketid	套接字ID
@return
	none
@note
**************************************************/

static void errorSocket(int socketid)
{
    uint8_t i;
    for (i = 0; i < SOCKET_LIST_MAX; i++)
    {
        if (socketid == socketlist[i].socketId)
        {
            socketlist[i].socketConnect = 0;
            LogPrintf(DEBUG_ALL, "socket[%d] ERR", i);
            break;
        }
    }

}
/**************************************************
@bref		套接字状态回调
@param
	socketid	套接字ID
	event		套接字状态事件
@return
	none
@note
**************************************************/

static void socketCallBack(int socketid, nwy_socket_event event)
{
    switch (event)
    {
        case NWY_LWIP_EVENT_ACCEPT:
            break;
        case NWY_LWIP_EVENT_SENT:
            break;
        case NWY_LWIP_EVENT_RECV:
            searchSocket(socketid);
            break;
        case NWY_LWIP_EVENT_CONNECTED:
            break;
        case NWY_LWIP_EVENT_POLL:
            searchSocket(socketid);
            break;
        case NWY_LWIP_EVENT_ERR:
            errorSocket(socketid);
            break;
    }
}

/**************************************************
@bref		ip转换成uint32类型
@param
	ip		ip信息
@return
	none
@note
**************************************************/

static uint32_t netIpChange(char *ip)
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

/**************************************************
@bref		创建套接字，连接服务器
@param
	socketinfo		socket信息
@return
	none
@note
**************************************************/

static int netSocketConnectCheck(socketInfo_s *socketinfo)
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
            LogPrintf(DEBUG_ALL, "%s dns fail", socketinfo->domain);
            if (socketinfo->dnscount > 10)
            {
                socketinfo->dnscount = 0;
                LogMessage(DEBUG_ALL, "dns timeout");
                result = -1; //dns fail
            }
            return result;
        }
        socketinfo->dnscount = 0;
        LogPrintf(DEBUG_ALL, "DNS IP:%s", domainIP);
        socketinfo->socketInfo.sin_addr.s_addr = netIpChange(domainIP);
        socketinfo->domainIP = 1;
    }
    //获取socket资源
    if (socketinfo->socketOpen == 0)
    {
        LogMessage(DEBUG_ALL, "socket open ...");
        socketinfo->socketId = nwy_socket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socketinfo->socketId < 0)
        {
            result = -2;
            return result;//open socket fail
        }
        LogPrintf(DEBUG_ALL, "socket open success==>SockId:%d", socketinfo->socketId);
        socketinfo->socketInfo.sin_len = sizeof(struct sockaddr_in);
        socketinfo->socketInfo.sin_family = AF_INET;
        socketinfo->socketInfo.sin_port = htons(socketinfo->port);
        on = 1;
        opt = 1;
        nwy_socket_setsockopt(socketinfo->socketId, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on));
        nwy_socket_setsockopt(socketinfo->socketId, IPPROTO_TCP, TCP_NODELAY, (void *)&opt, sizeof(opt));
        if (nwy_socket_set_nonblock(socketinfo->socketId) != 0)
        {
            LogMessage(DEBUG_ALL, "socket set nonblock==>error");
            result = -3;
            return result;//set socket error
        }
        socketinfo->socketOpen = 1;
    }

    if (socketinfo->socketConnect == 0)
    {
        LogPrintf(DEBUG_ALL, "Connect to %s:%d", socketinfo->domain, socketinfo->port);
        nwy_socket_connect(socketinfo->socketId, &socketinfo->socketInfo, sizeof(socketinfo->socketInfo));
        LogPrintf(DEBUG_ALL, "Connect Result:%d", nwy_socket_errno());
        //创建链接15秒超时
        if (socketinfo->socketConnectTick++ > 15)
        {
            socketinfo->socketConnectTick = 0;
            LogMessage(DEBUG_ALL, "TCP Connect Timeout");
            result = -4;
            return result;//connect tcp error
        }
        //链接创建成功
        if (EISCONN == nwy_socket_errno())
        {
            LogMessage(DEBUG_ALL, "TCP Connect OK");

            socketinfo->socketConnect = 1;
            result = 1;
            socketinfo->socketConnectTick = 0;

            return result;
        }
        //链接创建失败
        if (EINPROGRESS != nwy_socket_errno() && EALREADY != nwy_socket_errno())
        {
            LogMessage(DEBUG_ALL, "Socket create fail");
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

static void netWorkRestore(void)
{
    networkConnectCtl(1);
}


/**************************************************
@bref		检查套接字信息
@param
	none
@return
	none
@note
**************************************************/

static void socketcheck(void)
{
    static uint8_t errCnt = 0;
    static uint8_t hiddenSocketErr = 0;
    int sockId = 0;
    int ret;
    for (sockId = 0; sockId < SOCKET_LIST_MAX; sockId++)
    {
        if (socketlist[sockId].useFlag)
        {
            ret = netSocketConnectCheck(&socketlist[sockId]);
            if (ret == 1)
            {
                LogPrintf(DEBUG_ALL, "SockId[%d]==>%s:%d Connect Success", sockId, socketlist[sockId].domain, socketlist[sockId].port);
                if (sockId == NORMAL_LINK || sockId == JT808_LINK)
                {
                    errCnt = 0;
                }
                else if (sockId == HIDE_LINK)
                {
                    hiddenSocketErr = 0;
                }
            }
            else if (ret < 0)
            {
                LogPrintf(DEBUG_ALL, "SockId[%d]==>%s:%d Connect Fail,Ret:%d", sockId, socketlist[sockId].domain,
                          socketlist[sockId].port, ret);
                socketClose(sockId);
                if ((sockId == NORMAL_LINK && sysparam.protocol == ZT_PROTOCOL_TYPE)\
                        || (sockId == JT808_LINK && sysparam.protocol == JT808_PROTOCOL_TYPE))
                {
                    ++errCnt;
                    if (errCnt >= 5)
                    {
                        LogMessage(DEBUG_ALL, "socket err too much time");
                        portSystemReset();
                    }
                    else
                    {
                        networkConnectCtl(0);
                        startTimer(20, netWorkRestore, 0);
                    }
                }
                if (sockId == HIDE_LINK)
                {
                    ++hiddenSocketErr;
                    if (hiddenSocketErr >= 5)
                    {
                        hiddenSocketErr = 0;
                        hiddenServCloseRequest();
                    }
                }
            }
        }
    }
}

/**************************************************
@bref		发送信息
@param
	sockId	对应套接字id
	data	待发送数据
	len		待发送长度
@return
	1		成功
	<0		失败
@note
**************************************************/

int socketSendData(unsigned char sockId, unsigned char *data, unsigned int len)
{
    int sendlen;
    int ret;
    if (sockId > SOCKET_LIST_MAX)
    {
        return -1;
    }
    if (socketlist[sockId].useFlag == 0)
    {
        return -2;
    }
    if (socketlist[sockId].socketConnect == 0)
    {
        return -3;
    }

    sendlen = nwy_socket_send(socketlist[sockId].socketId, data, len, 0);
    LogPrintf(DEBUG_ALL, "socket[%d]==>RequestSend:%d,SendOk:%d", sockId, len, sendlen);
    ret = 1;
    if (sendlen < 0)
    {
        socketDel(&socketlist[sockId]);
        ret = -4;
    }
    return ret;
}


/**************************************************
@bref		恢复网络
@param
@note
**************************************************/

static void radioRestore(void)
{
    portSetRadio(1);
}

/**************************************************
@bref		更细协议中必须包含的信息
@param
@note
**************************************************/

static void protocolUpdate(void)
{
    char imsi[25];
    char iccid[25];
    portGetModuleIMSI(imsi);
    portGetModuleICCID(iccid);
    protocolUpdateIccid(iccid);
    protocolUpdateImsi(imsi);
}

/**************************************************
@bref		自动获取SN号作为IMEI号
@param
@note
**************************************************/

static void getSn(void)
{
    int ret;
    char imei[20];
    ret = portGetModuleIMEI(imei);
    if (ret != 0)
    {
        LogMessage(DEBUG_ALL, "get sn error");
        return;
    }
    if (imei[0] == 0)
    {
        return;
    }
    if (strncmp(sysparam.SN, imei, 15) == 0)
    {
        return;
    }
    strncpy(sysparam.SN, imei, 15);
    paramSaveAll();
}
/**************************************************
@bref		负责网络连接
@param
	none
@return
	none
@note
**************************************************/

void networkConnectTask(void)
{
    uint8_t csq;
    if (networkInfo.networkonoff == 0)
    {
        return ;
    }

    if (sysIsInRun() == 0)
    {
        networkConnectCtl(0);
        return;
    }
	

    switch (networkInfo.netFsm)
    {
        case CHECK_SIM:
        	portSimGet();
            if (nwy_sim_get_card_status() == NWY_SIM_STATUS_READY)
            {
                getSn();
                LogMessage(DEBUG_ALL, "Sim OK");
                changeNetFsm(CHECK_SIGNAL);
            }
            else
            {
                LogMessage(DEBUG_ALL, "Sim check");
                if (nwy_sim_get_card_status() == NWY_SIM_STATUS_NOT_INSERT)
                {
                	//把卡报警的处理
                    if (sysparam.simpulloutalm && networkInfo.netTick >= 3 && portSimGet() == SIM_1)
                    {
                        if (sysparam.simpulloutLock != 0)
                        {
                            sysparam.relayCtl = 1;
                            paramSaveAll();
                            relayAutoRequest();
                            LogPrintf(DEBUG_ALL, "shutdown==>try to relay on");
                        }
                        portSimSet(SIM_2);
                        LogMessage(DEBUG_ALL, "no sim card");
                        alarmRequestSet(ALARM_SIMPULLOUT_REQUEST);
                        portSystemReset();
                    }
                }
                
                if (networkInfo.netTick >= 60)
                {
                    if (sysparam.simSel == SIM_1 && portSimGet() == SIM_1)
                    {
                        LogPrintf(DEBUG_ALL, "no sim , try to use SIM2");
                        portSimSet(SIM_2);
                    }
                    else if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
                    {
						LogPrintf(DEBUG_ALL, "no sim2 , try to use SIM1");
						portSimSet(SIM_1);
                    }
                    portSystemReset();
                }
                break;
            }

        case CHECK_SIGNAL:
            nwy_nw_get_signal_csq(&csq);
            if (csq >= 8 && csq <= 31)
            {
                networkInfo.csqSearchTime = 120;
                LogMessage(DEBUG_ALL, "Signal OK");
                changeNetFsm(CHECK_REGISTER);
            }
            else
            {
                LogPrintf(DEBUG_ALL, "Signal fail :%d", csq);
                if (networkInfo.netTick >= networkInfo.csqSearchTime)
                {
                    networkInfo.csqSearchTime += 120;
                    networkInfo.csqSearchTime = networkInfo.csqSearchTime > 3600 ? 3600 : networkInfo.csqSearchTime;
                    portSetRadio(0);
                    changeNetFsm(CHECK_SIM);
                    startTimer(50, radioRestore, 0);
                }
                break;
            }
        case CHECK_REGISTER:
            if (netCheckRegister())
            {
                networkInfo.netRegCnt = 0;
                LogMessage(DEBUG_ALL, "Register OK");
                changeNetFsm(CHECK_DATACALL);
            }
            else
            {
                LogMessage(DEBUG_ALL, "Register fail");
                if (networkInfo.netTick >= 90)
                {
                    LogMessage(DEBUG_ALL, "Register timeout");
                    changeNetFsm(CHECK_SIM);
                    if (++networkInfo.netRegCnt >= 4)
                    {
                        if (sysparam.simSel == SIM_1 && portSimGet() == SIM_1)
                        {
                            LogPrintf(DEBUG_ALL, "Register ERROR, try to use SIM2");
                            portSimSet(SIM_2);
                        }
                        else if (sysparam.simSel == SIM_1 && portSimGet() == SIM_2)
		                {
							LogPrintf(DEBUG_ALL, "Register ERROR , try to use SIM1");
							portSimSet(SIM_1);
		                }
                        portSystemReset();
                    }
                }
            }
            break;
        case CHECK_DATACALL:
            if (netCheckDataCall())
            {
                protocolUpdate();
                LogMessage(DEBUG_ALL, "DataCall success");
                nwy_socket_event_report_reg(socketCallBack);
                changeNetFsm(CHECK_SOCKET);
            }
            else
            {
                LogMessage(DEBUG_ALL, "DataCall waitting...");
                break;
            }

        case CHECK_SOCKET:
            socketcheck();
            break;
    }
    networkInfo.netTick++;
}

/**************************************************
@bref		查询网络状况是否正常
@param
	none
@return
	none
@note
**************************************************/

uint8_t isNetworkNormal(void)
{
    if (networkInfo.networkonoff == 0)
        return 0;
    if (networkInfo.netFsm == CHECK_SOCKET)
        return 1;
    return 0;
}

/**************************************************
@bref		查找对应socket是否被使用
@param
	none
@return
	none
@note
**************************************************/

uint8_t socketGetUsedFlag(uint8_t sockeId)
{
    if (sockeId >= SOCKET_LIST_MAX)
        return 0;

    return socketlist[sockeId].useFlag;
}

/**************************************************
@bref		查询socket连接状态
@param
	none
@return
	none
@note
**************************************************/

uint8_t socketGetConnectStatus(uint8_t sockeId)
{
    if (sockeId >= SOCKET_LIST_MAX)
        return 0;

    return socketlist[sockeId].socketConnect;
}

int socketGetNonAck(uint8_t socketId)
{
    int sent;
    int ack;
    if (socketId >= SOCKET_LIST_MAX)
    {
        return -1;
    }
    if (socketlist[socketId].socketConnect == 0)
    {
        return -2;
    }
    sent = nwy_socket_get_sent(socketlist[socketId].socketId);
    ack = nwy_socket_get_ack(socketlist[socketId].socketId);
    return sent - ack;
}

