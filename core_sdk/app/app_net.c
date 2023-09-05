#include "app_net.h"
#include "stdio.h"

#include "app_socket.h"
#include "app_protocol.h"
#include "app_param.h"
#include "app_sys.h"
#include "app_task.h"
#include "app_port.h"
#include "app_jt808.h"
#include "stdlib.h"
#include "app_db.h"


static serverConnect_s serverConnect;
static serverConnect_s hiddenConnect;
static serverConnect_s bleServConnect;
static jt808_Connect_s jt808ServConn;
static recordUpload_s recinfo;
static bleInfo_s *bleHead = NULL;

/**************************************************
@bref		servr状态切换
@param
	none
@return
	none
@note
**************************************************/

static void serverChangeFsm(serverfsm_e newFsm)
{
    serverConnect.connectFsm = newFsm;
    serverConnect.runTick = 0;
}
/**************************************************
@bref		数据接收回调
@param
	none
@return
	none
@note
**************************************************/

static void soketDataRecv(struct SOCK_INFO *socketinfo, char *rxbuf, uint16_t len)
{
    socketRecvPush(socketinfo->index, rxbuf, len);
}
/**************************************************
@bref		数据发送接口
@param
	none
@return
	1		发送成功
	!=1		发送失败
@note
**************************************************/

static int SoketDataSend(uint8_t link, uint8_t *data, uint16_t len)
{
    int ret;
    ret = socketSendData(link, data, len);
    return ret;
}

/**************************************************
@bref		服务器链接任务
@param
	none
@return
	none
@note
**************************************************/

static void serverConnRunTask(void)
{
    if (isNetworkNormal() == 0)
    {
        ledStatusUpdate(SYSTEM_LED_LOGIN, 0);
        return;
    }
    if (socketGetUsedFlag(NORMAL_LINK) != 1)
    {
        protocolRegisterTcpSend(SoketDataSend);
        ledStatusUpdate(SYSTEM_LED_LOGIN, 0);
        serverChangeFsm(SERV_LOGIN);
        socketAdd(NORMAL_LINK, sysparam.Server, sysparam.ServerPort, soketDataRecv);
        return;
    }
    if (socketGetConnectStatus(NORMAL_LINK) != 1)
    {
        return;
    }



    serverConnect.runTick++;
    switch (serverConnect.connectFsm)
    {
        case SERV_LOGIN:
            if (strcmp(sysparam.SN, "888888887777777") == 0)
            {
                LogMessage(DEBUG_ALL, "no SN");
                return;
            }
            protoclUpdateSn(sysparam.SN);
            LogMessage(DEBUG_ALL, "Send login message");
			protocolUpdateSlaverMac();
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_01, NULL);
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_F1, NULL);
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_F6, NULL);
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_8A, NULL);
            serverChangeFsm(SERV_LOGIN_WAIT);
            sysinfo.dbFileUpload = 1;
            break;
        case SERV_LOGIN_WAIT:
            if (serverConnect.runTick >= 60)
            {
                serverConnect.runTick = 0;
                serverConnect.loginCnt++;
                if (serverConnect.loginCnt >= 3)
                {
                    //登录三次超时未成功
                    LogMessage(DEBUG_ALL, "Login fail");
                    socketClose(NORMAL_LINK);
                }
                else
                {
                    LogMessage(DEBUG_ALL, "Login timeout");
                    serverChangeFsm(SERV_LOGIN);
                }
            }
            break;
        case SERV_READY:
            if (serverConnect.runTick >= 180)
            {
                serverConnect.runTick = 0;
                protocolUpdateRssi(portGetModuleRssi());
                protocolUpdateSomeInfo(sysinfo.outsideVol, sysinfo.batteryVol, portCapacityCalculate(sysinfo.batteryVol),
                                       sysparam.startUpCnt, sysparam.runTime);
                sendProtocolToServer(NORMAL_LINK, PROTOCOL_13, NULL);
            }

            if (socketGetNonAck(NORMAL_LINK) == 0)
            {
				if (sysparam.uploadGap == 0) 
				{
					if (dbPost() == 0)
					{
						if (jt808ServConn.runTick % 3 == 0)
						{
							if (sysinfo.dbFileUpload == 1)
							{
								sysinfo.dbFileUpload = gpsRestoreReadData();
							}
						}
					}
				}
            }
            break;
        default:
            serverChangeFsm(SERV_LOGIN);
            break;
    }
}
/**************************************************
@bref		服务器登录回复
@param
	none
@return
	none
@note
**************************************************/

void serverLoginRespon(void)
{
    serverChangeFsm(SERV_READY);
    serverConnect.runTick = 180;
    serverConnect.loginCnt = 0;
    LogMessage(DEBUG_ALL, "Login Success");
    ledStatusUpdate(SYSTEM_LED_LOGIN, 1);
}
/**************************************************
@bref		查看与服务器是否连接正常
@param
	none
@return
	none
@note
**************************************************/

uint8_t serverIsReady(void)
{
    if (isNetworkNormal() == 0)
    {
        return	0;
    }
    if (sysparam.protocol == JT808_PROTOCOL_TYPE)
    {
        if (jt808ServConn.connectFsm == JT808_NORMAL)
        {
            return 1;
        }
    }
    else
    {
        if (serverConnect.connectFsm == SERV_READY)
        {
            return 1;
        }
    }
    return 0;
}
/**************************************************
@bref		服务器重连
@param
	none
@return
	none
@note
**************************************************/

void serverReconnect(void)
{
    LogMessage(DEBUG_ALL, "serverReconnect");
    socketClose(NORMAL_LINK);
}





/**************************************************
@bref		服务器重连
@param
	none
@return
	none
@note
**************************************************/

void hiddenServReconnect(void)
{
    LogMessage(DEBUG_ALL, "hiddenServReconnect");
    socketClose(HIDE_LINK);
}

/**************************************************
@bref		发送一次心跳包来检查链路
@param
	none
@return
	none
@note
**************************************************/
void checkPrivateServer(void)
{
    protocolUpdateRssi(portGetModuleRssi());
    protocolUpdateSomeInfo(sysinfo.outsideVol, sysinfo.batteryVol, portCapacityCalculate(sysinfo.batteryVol),
                           sysparam.startUpCnt, sysparam.runTime);
    sendProtocolToServer(NORMAL_LINK, PROTOCOL_13, NULL);
    //sysinfo.socketStatus = 0;
}



/**************************************************
@bref		servr状态切换
@param
	none
@return
	none
@note
**************************************************/

static void hideServerChangeFsm(serverfsm_e newFsm)
{
    hiddenConnect.connectFsm = newFsm;
    hiddenConnect.runTick = 0;
}
/**************************************************
@bref		隐藏服务器登录回复
@param
	none
@return
	none
@note
**************************************************/

void hiddenServLoginRespon(void)
{
    hideServerChangeFsm(SERV_READY);
    hiddenConnect.runTick = 180;
    hiddenConnect.loginCnt = 0;
    LogMessage(DEBUG_ALL, "hidden login success");
}

/**************************************************
@bref		请求关闭隐藏链路
@param
@return
@note
**************************************************/

void hiddenServCloseRequest(void)
{
    sysinfo.hiddenServCloseReq = 1;
    LogMessage(DEBUG_ALL, "hidden serv close request");
}


/**************************************************
@bref		清除关闭隐藏链路的请求
@param
@return
@note
**************************************************/

void hiddenServCloseClear(void)
{
    sysinfo.hiddenServCloseReq = 0;
    LogMessage(DEBUG_ALL, "hidden serv close clear");
}


/**************************************************
@bref		检查是否需要关闭隐藏链路
@param
@return
	1		需要关闭
	0		不需要
@note
**************************************************/


static uint8_t hiddenServCloseChecking(void)
{
    if (sysparam.hiddenServOnoff == 0)
    {
        return 1;
    }
    if (sysparam.protocol == ZT_PROTOCOL_TYPE)
    {
        if (sysparam.ServerPort == sysparam.hiddenPort)
        {
            if (strcmp(sysparam.Server, sysparam.hiddenServer) == 0)
            {
                //if use the same server and port ,abort use hidden server.
                return	1;
            }
        }
    }
    if (sysinfo.hiddenServCloseReq)
    {
        //it is the system request of close hidden server,maybe the socket error.
        return 1;
    }
    return 0;
}

uint8_t hiddenServIsReady(void)
{
    if (isNetworkNormal() == 0)
    {
        return 0 ;
    }
    if (hiddenConnect.connectFsm == SERV_READY)
    {
        return 1;
    }
    return 0;
}


/**************************************************
@bref		隐藏链路服务器
@param
	none
@return
	none
@note
**************************************************/

static void hiddenServerConnTask(void)
{
    if (isNetworkNormal() == 0)
    {
        return;
    }
    if (hiddenServCloseChecking())
    {
        if (socketGetUsedFlag(HIDE_LINK) == 1)
        {
            LogMessage(DEBUG_ALL, "hidden server abort");
            socketClose(HIDE_LINK);
        }
        hideServerChangeFsm(SERV_LOGIN);
        return;
    }

    if (socketGetUsedFlag(HIDE_LINK) != 1)
    {
        protocolRegisterTcpSend(SoketDataSend);
        hideServerChangeFsm(SERV_LOGIN);
        socketAdd(HIDE_LINK, sysparam.hiddenServer, sysparam.hiddenPort, soketDataRecv);
        return;
    }
    if (socketGetConnectStatus(HIDE_LINK) != 1)
    {
        return;
    }



    hiddenConnect.runTick++;
    switch (hiddenConnect.connectFsm)
    {
        case SERV_LOGIN:
            if (strcmp(sysparam.SN, "888888887777777") == 0)
            {
                return;
            }
            protoclUpdateSn(sysparam.SN);
            LogMessage(DEBUG_ALL, "hidden login");
            sendProtocolToServer(HIDE_LINK, PROTOCOL_01, NULL);
            sendProtocolToServer(HIDE_LINK, PROTOCOL_F1, NULL);
            hideServerChangeFsm(SERV_LOGIN_WAIT);
            break;
        case SERV_LOGIN_WAIT:
            if (hiddenConnect.runTick >= 60)
            {
                hiddenConnect.runTick = 0;
                hiddenConnect.loginCnt++;
                if (hiddenConnect.loginCnt >= 3)
                {
                    //登录三次超时未成功
                    LogMessage(DEBUG_ALL, "hidden login fail");
                    socketClose(HIDE_LINK);
                }
                else
                {
                    LogMessage(DEBUG_ALL, "hidden login timeout");
                    hideServerChangeFsm(SERV_LOGIN);
                }
            }
            break;
        case SERV_READY:
            if (hiddenConnect.runTick >= 180)
            {
                hiddenConnect.runTick = 0;
                protocolUpdateRssi(portGetModuleRssi());
                protocolUpdateSomeInfo(sysinfo.outsideVol, sysinfo.batteryVol, portCapacityCalculate(sysinfo.batteryVol),
                                       sysparam.startUpCnt, sysparam.runTime);
                sendProtocolToServer(HIDE_LINK, PROTOCOL_13, NULL);
            }
            break;
        default:
            hideServerChangeFsm(SERV_LOGIN);
            break;
    }
}



/**************************************************
@bref		jt808状态机切换状态
@param
	nfsm	新状态
@return
	none
@note
**************************************************/

static void jt808ChangeFsm(jt808_connfsm_s nfsm)
{
    jt808ServConn.connectFsm = nfsm;
    jt808ServConn.runTick = 0;
}

/**************************************************
@bref		数据接收回调
@param
	none
@return
	none
@note
**************************************************/

static void jt808SoketDataRecv(struct SOCK_INFO *socketinfo, char *rxbuf, uint16_t len)
{
    jt808ReceiveParser((uint8_t *)rxbuf, len);
}

/**************************************************
@bref		jt808联网状态机
@param
@return
@note
**************************************************/

static void jt808ServerConnRunTask(void)
{

    if (isNetworkNormal() == 0)
    {
        ledStatusUpdate(SYSTEM_LED_LOGIN, 0);
        return;
    }
    if (socketGetUsedFlag(JT808_LINK) != 1)
    {
        ledStatusUpdate(SYSTEM_LED_LOGIN, 0);
        jt808ChangeFsm(JT808_REGISTER);
        jt808RegisterTcpSend(SoketDataSend);
		jt808RegisterManufactureId(sysparam.jt808manufacturerID);
	 	jt808RegisterTerminalType(sysparam.jt808terminalType);
	 	jt808RegisterTerminalId(sysparam.jt808terminalID);
        socketAdd(JT808_LINK, sysparam.jt808Server, sysparam.jt808Port, jt808SoketDataRecv);
        return;
    }
    if (socketGetConnectStatus(JT808_LINK) != 1)
    {
        return;
    }


    switch (jt808ServConn.connectFsm)
    {
        case JT808_REGISTER:

            if (strcmp((char *)sysparam.jt808sn, "888777") == 0)
            {
                LogMessage(DEBUG_ALL, "no JT808SN");
                return;
            }

            if (sysparam.jt808isRegister)
            {
                //已注册过的设备不用重复注册
                jt808ChangeFsm(JT808_AUTHENTICATION);
                jt808ServConn.regCnt = 0;
            }
            else
            {
                //注册设备
                if (jt808ServConn.runTick % 60 == 0)
                {
                    if (jt808ServConn.regCnt++ > 3)
                    {
                        LogMessage(DEBUG_ALL, "Terminal register timeout");
                        jt808ServConn.regCnt = 0;
                        jt808serverReconnect();
                    }
                    else
                    {
                        LogMessage(DEBUG_ALL, "Terminal register");
                        jt808RegisterLoginInfo(sysparam.jt808sn, sysparam.jt808isRegister, sysparam.jt808AuthCode, sysparam.jt808AuthLen);
                        jt808SendToServer(TERMINAL_REGISTER, NULL);
                    }
                }
                break;
            }
        case JT808_AUTHENTICATION:

            if (jt808ServConn.runTick % 60 == 0)
            {
                sysinfo.dbFileUpload = 1;
                if (jt808ServConn.authCnt++ > 3)
                {
                    jt808ServConn.authCnt = 0;
                    sysparam.jt808isRegister = 0;
                    paramSaveAll();
                    jt808serverReconnect();
                    LogMessage(DEBUG_ALL, "Authentication timeout");
                }
                else
                {
                    LogMessage(DEBUG_ALL, "Terminal authentication");
                    jt808ServConn.hbtTick = sysparam.heartbeatgap;
                    jt808RegisterLoginInfo(sysparam.jt808sn, sysparam.jt808isRegister, sysparam.jt808AuthCode, sysparam.jt808AuthLen);
                    jt808SendToServer(TERMINAL_AUTH, NULL);
                }
            }
            break;
        case JT808_NORMAL:
            if (++jt808ServConn.hbtTick >= sysparam.heartbeatgap)
            {
                jt808ServConn.hbtTick = 0;
                LogMessage(DEBUG_ALL, "Terminal heartbeat");
                jt808SendToServer(TERMINAL_HEARTBEAT, NULL);
            }
			if (socketGetNonAck(JT808_LINK) == 0)
            {
            	if (sysparam.uploadGap == 0)
            	{
	                if (dbPost() == 0)
	                {
	                    if (jt808ServConn.runTick % 3 == 0)
	                    {
	                        if (sysinfo.dbFileUpload == 1)
	                        {
	                            sysinfo.dbFileUpload = gpsRestoreReadData();
	                        }
	                    }
	                }
                }
            }
            break;
        default:
            jt808ChangeFsm(JT808_AUTHENTICATION);
            break;
    }
    jt808ServConn.runTick++;
}


/**************************************************
@bref		jt808联网状态机
@param
@return
@note
**************************************************/

void jt808serverReconnect(void)
{
    LogMessage(DEBUG_ALL, "jt808 reconnect server");
    socketClose(JT808_LINK);
}

/**************************************************
@bref		jt808鉴权成功回复
@param
@return
@note
**************************************************/

void jt808AuthOk(void)
{
    jt808ServConn.authCnt = 0;
    jt808ChangeFsm(JT808_NORMAL);
    ledStatusUpdate(SYSTEM_LED_LOGIN, 1);
}



/**************************************************
@bref		添加待登录的从设备信息
@param
@return
@note
	SN:999913436051195,292,3.77,46
**************************************************/

void bleServerAddInfo(bleInfo_s dev)
{
    bleInfo_s *next;
    if (bleHead == NULL)
    {
        bleHead = malloc(sizeof(bleInfo_s));
        if (bleHead != NULL)
        {
            strncpy(bleHead->imei, dev.imei, 15);
            bleHead->imei[15] = 0;
            bleHead->startCnt = dev.startCnt;
            bleHead->vol = dev.vol;
            bleHead->batLevel = dev.batLevel;
            bleHead->next = NULL;
        }
        else
        {
            LogPrintf(DEBUG_ALL, "bleServerAddInfo==>Fail");
        }
        return;
    }
    next = bleHead;
    while (next != NULL)
    {
        if (next->next == NULL)
        {
            next->next = malloc(sizeof(bleInfo_s));
            if (next->next != NULL)
            {
                next = next->next;

                strncpy(next->imei, dev.imei, 15);
                next->imei[15] = 0;
                next->startCnt = dev.startCnt;
                next->vol = dev.vol;
                next->batLevel = dev.batLevel;
                next->next = NULL;
                next = next->next;
            }
            else
            {
                LogPrintf(DEBUG_ALL, "bleServerAddInfo==>Fail");
                break;
            }
        }
        else
        {
            next = next->next;
        }
    }
}

/**************************************************
@bref		显示待连接队列
@param
@return
@note
**************************************************/

void showBleList(void)
{
    uint8_t cnt;
    bleInfo_s *dev;
    dev = bleHead;
    cnt = 0;
    while (dev != NULL)
    {
        LogPrintf(DEBUG_ALL, "Dev[%d]:%s", ++cnt, dev->imei);
        dev = dev->next;
    }
}

/**************************************************
@bref		servr状态切换
@param
	none
@return
	none
@note
**************************************************/

static void bleServerChangeFsm(serverfsm_e newFsm)
{
    bleServConnect.connectFsm = newFsm;
    bleServConnect.runTick = 0;
}

/**************************************************
@bref		蓝牙待连接链路登录成
@param
@return
@note
**************************************************/

void bleServerLoginReady(void)
{
    bleServerChangeFsm(SERV_READY);
    bleServConnect.loginCnt = 0;
    LogMessage(DEBUG_ALL, "ble login success");
}


/**************************************************
@bref		服务器链接任务
@param
	none
@return
	none
@note
**************************************************/

static void bleServerConnRunTask(void)
{
    static uint8_t tick = 0;
    bleInfo_s *next;
    gpsinfo_s *gpsinfo;

    if (isNetworkNormal() == 0 || bleHead == NULL)
    {
        if (gpsRequestGet(GPS_REQ_BLE))
        {
            gpsRequestClear(GPS_REQ_BLE);
        }
        return;
    }
    if (socketGetUsedFlag(BLE_LINK) != 1)
    {
        protocolRegisterTcpSend(SoketDataSend);
        bleServerChangeFsm(SERV_LOGIN);
        socketAdd(BLE_LINK, sysparam.bleServer, sysparam.bleServerPort, soketDataRecv);
        return;
    }
    if (socketGetConnectStatus(BLE_LINK) != 1)
    {
        return;
    }

    switch (bleServConnect.connectFsm)
    {
        case SERV_LOGIN:
            if (strcmp(bleHead->imei, "888888887777777") == 0)
            {
                LogMessage(DEBUG_ALL, "not valid sn");
                bleServerChangeFsm(SERV_END);
                return;
            }
            tick = 0;
            protoclUpdateSn(bleHead->imei);
            LogMessage(DEBUG_ALL, "ble try login");
            sendProtocolToServer(BLE_LINK, PROTOCOL_01, NULL);
            bleServerChangeFsm(SERV_LOGIN_WAIT);
            gpsRequestSet(GPS_REQ_BLE);
            break;
        case SERV_LOGIN_WAIT:
            if (++bleServConnect.runTick >= 30)
            {
                bleServConnect.loginCnt++;
                if (bleServConnect.loginCnt >= 2)
                {
                    //登录超时未成功
                    LogMessage(DEBUG_ALL, "ble Login fail");
                    bleServerChangeFsm(SERV_END);
                }
                else
                {
                    LogMessage(DEBUG_ALL, "ble Login timeout");
                    bleServerChangeFsm(SERV_LOGIN);
                }
            }
            break;
        case SERV_READY:
            if (bleServConnect.runTick++ == 0)
            {
                protocolUpdateRssi(portGetModuleRssi());
                protocolUpdateSomeInfo(sysinfo.outsideVol, bleHead->vol, bleHead->batLevel, bleHead->startCnt, 0);
                sendProtocolToServer(BLE_LINK, PROTOCOL_13, NULL);
            }
            gpsinfo = getCurrentGPSInfo();

            if (gpsinfo->fixstatus)
            {
                if (tick++ >= 10)
                {
                    sendProtocolToServer(BLE_LINK, PROTOCOL_12, gpsinfo);
                    bleServerChangeFsm(SERV_END);
                    break;
                }
            }
            else
            {
                tick = 0;
            }
            if (bleServConnect.runTick >= 180)
            {
                sendProtocolToServer(BLE_LINK, PROTOCOL_12, getLastFixedGPSInfo());
                bleServerChangeFsm(SERV_END);
            }

            break;
        case SERV_END:
            next = bleHead->next;
            free(bleHead);
            bleHead = next;
            socketClose(BLE_LINK);
            bleServerChangeFsm(SERV_LOGIN);
            LogPrintf(DEBUG_ALL, "ble server done");
            break;
        default:
            bleServerChangeFsm(SERV_LOGIN);
            break;
    }
}

/**************************************************
@bref		设备链接服务器时，协议选择
@param
@return
@note
**************************************************/
void serverConnTask(void)
{
    if (sysparam.protocol == JT808_PROTOCOL_TYPE)
    {
        jt808ServerConnRunTask();
    }
    else
    {
        serverConnRunTask();
    }
    hiddenServerConnTask();
    bleServerConnRunTask();
}




/**************************************************
@bref		固件升级socket的发送函数
@param
	none
@return
	none
@note
**************************************************/

int upgradeSoketDataSend(uint8_t *data, uint16_t len)
{
    int ret;
    ret = socketSendData(UPGRADE_LINK, data, len);
    return ret;
}

/**************************************************
@bref		固件升级任务
@param
	none
@return
	none
@note
**************************************************/

void upgradeRunTask(void)
{
    if (sysinfo.upgradeDoing == 0)
    {
        if (socketGetUsedFlag(UPGRADE_LINK))
        {
            LogMessage(DEBUG_ALL, "upgrade socket was open ,close it");
            socketClose(UPGRADE_LINK);
        }
        return;
    }
    if (isNetworkNormal() == 0)
    {
        return;
    }
    if (socketGetUsedFlag(UPGRADE_LINK) != 1)
    {
        socketAdd(UPGRADE_LINK, sysparam.updateServer, sysparam.updateServerPort, soketDataRecv);
        return;
    }
    if (socketGetConnectStatus(UPGRADE_LINK) != 1)
    {
        return;
    }
    upgradeFromServer();

}


/**************************************************
@bref		agps请求
@param
	none
@return
	none
@note
**************************************************/

void agpsRequestSet(void)
{
    sysinfo.agpsRequest = 1;
    LogMessage(DEBUG_ALL, "agpsRequestSet==>OK");
}

void agpsRequestClear(void)
{
    sysinfo.agpsRequest = 0;
    LogMessage(DEBUG_ALL, "agpsRequestClear==>OK");
}
/**************************************************
@bref		agps数据接收
@param
	none
@return
	none
@note
**************************************************/

static void agpssoketDataRecv(struct SOCK_INFO *socketinfo, char *rxbuf, uint16_t len)
{
    if (sysinfo.gpsOnoff == 0)
        return;
    portSendAgpsData(rxbuf, len);
}

/**************************************************
@bref		agps服务器任务
@param
	none
@return
	none
@note
**************************************************/

void agpsConnRunTask(void)
{
    static uint8_t agpsFsm = 0;
    static uint8_t runTick = 0;
    char agpsBuff[150] = {0};
    uint16_t agpsLen = 0;
    int ret;
	gpsinfo_s *gpsinfo;
    if (sysinfo.agpsRequest == 0)
    {
        if (socketGetUsedFlag(AGPS_LINK))
        {
            LogMessage(DEBUG_ALL, "agps socket was open ,close it");
            socketClose(AGPS_LINK);
        }
        return;
    }
    if (isNetworkNormal() == 0)
    {
        return;
    }
    gpsinfo = getCurrentGPSInfo();
    if (sysinfo.gpsOnoff == 0 || gpsinfo->fixstatus)
    {
		if (socketGetUsedFlag(AGPS_LINK))
        {
            socketClose(AGPS_LINK);
        }
        agpsRequestClear();
    }
    if (socketGetUsedFlag(AGPS_LINK) != 1)
    {
        agpsFsm = 0;
        ret = socketAdd(AGPS_LINK, sysparam.agpsServer, sysparam.agpsPort, agpssoketDataRecv);
        if (ret != 1)
        {
            agpsRequestClear();
            LogMessage(DEBUG_ALL, "agps socket err");
        }
        return;
    }
    if (socketGetConnectStatus(AGPS_LINK) != 1)
    {
    	agpsFsm = 0;
        return;
    }
    switch (agpsFsm)
    {
        case 0:
//            sprintf(agpsBuff, "user=%s;pwd=%s;cmd=full;", sysparam.agpsUser, sysparam.agpsPswd);
//            socketSendData(AGPS_LINK, (uint8_t *) agpsBuff, strlen(agpsBuff));
			createProtocolA0(agpsBuff, &agpsLen);
			socketSendData(AGPS_LINK, (uint8_t *) agpsBuff, agpsLen);
            agpsFsm = 1;
            runTick = 0;
            break;
        case 1:
            if (++runTick >= 30)
            {
                agpsFsm = 0;
                runTick = 0;
                agpsRequestClear();
            }
            break;
        default:
            agpsFsm = 0;
            break;
    }
}

/**************************************************
@bref		是否正在上传录音
@param
	none
@return
	1		是
	0		否
@note
**************************************************/

uint8_t isRecUploadRun(void)
{
    return recinfo.recordUpload;
}


/**************************************************
@bref		设置需要上送的录音文件
@param
	none
@return
	none
@note
**************************************************/

void recSetUploadFile(uint8_t *filename, uint32_t filesize, uint8_t *recContent)
{
    memset(&recinfo, 0, sizeof(recordUpload_s));
    strcpy((char *)recinfo.recUploadFileName, (char *) filename);
    recinfo.recUploadFileSize = filesize;
    recinfo.recUploadContent = recContent;
    LogPrintf(DEBUG_ALL, "RecUpload==>%s:%d", filename, filesize);
    recinfo.recordUpload = 1;
}

/**************************************************
@bref		发送上传录音文件的请求协议
@param
	none
@return
	none
@note
**************************************************/


static void recSendReq(void)
{
    recordUploadInfo_s recUploadInfo;
    LogPrintf(DEBUG_ALL, "File:%s , Total:%d", recinfo.recUploadFileName, recinfo.recUploadFileSize);
    recUploadInfo.dateTime = (char *) recinfo.recUploadFileName;
    recUploadInfo.totalSize = recinfo.recUploadFileSize;
    recUploadInfo.fileType = 0;
    recUploadInfo.packSize = REC_UPLOAD_ONE_PACK_SIZE;
    sendProtocolToServer(NORMAL_LINK, PROTOCOL_61, &recUploadInfo);
}

/**************************************************
@bref		发送录音数据
@param
	none
@return
	none
@note
**************************************************/

static void recSendData(void)
{
    char dest[REC_UPLOAD_ONE_PACK_SIZE + 50];
    recordUploadInfo_s recUploadInfo;

    recinfo.needRead = recinfo.recUploadFileSize - recinfo.hadRead;
    if (recinfo.needRead > REC_UPLOAD_ONE_PACK_SIZE)
    {
        recinfo.needRead = REC_UPLOAD_ONE_PACK_SIZE;
    }
    LogPrintf(DEBUG_ALL, "File:%s , Total:%d , Offset:%d , Size:%d", recinfo.recUploadFileName,
              recinfo.recUploadFileSize, recinfo.hadRead, recinfo.needRead);
    recUploadInfo.dest = dest;
    recUploadInfo.dateTime = (char *) recinfo.recUploadFileName;
    recUploadInfo.packNum = recinfo.hadRead / REC_UPLOAD_ONE_PACK_SIZE;
    recUploadInfo.recData = recinfo.recUploadContent + recinfo.hadRead;
    recUploadInfo.recLen = recinfo.needRead;

    sendProtocolToServer(NORMAL_LINK, PROTOCOL_62, &recUploadInfo);

}

/**************************************************
@bref		数据上送
@param
	none
@return
	none
@note
**************************************************/

static void recUploadQuick(void)
{
    recinfo.hadRead += recinfo.needRead;
    if (recinfo.hadRead >= recinfo.recUploadFileSize)
    {
        LogMessage(DEBUG_ALL, "Upload all complete");
        recinfo.recordUpload = 0;
        recinfo.recfsm = 0;
        return ;
    }
    recSendData();
}

/**************************************************
@bref		状态机切换
@param
	none
@return
	none
@note
**************************************************/


static void recChangeFsm(uint8_t fsm)
{
    recinfo.recfsm = fsm;
    recinfo.ticktime = 0;
}

/**************************************************
@bref		上传收到服务器回复
@param
	none
@return
	none
@note
**************************************************/

void recUploadRsp(void)
{
    recUploadQuick();
    recChangeFsm(REC_DATA_WAIT);
}

/**************************************************
@bref		录音上送
@param
	none
@return
	none
@note
**************************************************/

void recordUploadTask(void)
{
    if (recinfo.recordUpload == 0)
    {
        recinfo.counttype1 = 0;
        recinfo.counttype2 = 0;
        recChangeFsm(REC_REQ_SEND);
        return ;
    }
    if (serverIsReady() == 0)
    {
        return;
    }
    recinfo.ticktime++;
    switch (recinfo.recfsm)
    {
        //上传文件名
        case REC_REQ_SEND:
            recSendReq();
            recChangeFsm(REC_REQ_WAIT);
            break;
        //等待61返回
        case REC_REQ_WAIT:
            if (recinfo.ticktime > 30)
            {
                recinfo.counttype1++;
                recChangeFsm(REC_REQ_SEND);
                if (recinfo.counttype1 >= 3)
                {
                    recinfo.counttype1 = 0;
                    recinfo.recordUpload = 0;
                }
            }
            break;
        //重发
        case REC_DATA_SEND:
            recSendData();
            recChangeFsm(REC_DATA_WAIT);
            break;
        //等待62返回
        case REC_DATA_WAIT:
            if (recinfo.ticktime > 30)
            {
                recinfo.counttype2++;
                recChangeFsm(REC_DATA_SEND);
                if (recinfo.counttype2 >= 3)
                {
                    recinfo.counttype2 = 0;
                    recinfo.recordUpload = 0;
                }
            }
            break;
        default:
            recChangeFsm(REC_REQ_SEND);
            break;
    }
}


