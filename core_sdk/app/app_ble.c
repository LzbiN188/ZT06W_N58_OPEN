#include "app_ble.h"
#include "app_sys.h"
#include "app_param.h"
#include "app_instructioncmd.h"
#include "app_task.h"
#include "nwy_ble.h"
#include "stdio.h"
#include "stdlib.h"
#include "aes.h"
#include "app_port.h"
#include "nwy_ble_client.h"
#include "app_net.h"
#include "app_protocol.h"
#include "aes.h"

static bleServer_s bleServInfo;
static bleClinet_s bleClientInfo;
static bleConnectShchedule_s bleSchedule;

static void bleServRecvParser(uint8_t *buf, uint8_t len);
static void bleRecvParser(uint8_t *data, uint8_t len);
static void bleClientSendEvent(ble_clientevent_e event);

/***********************************************************************/
//                         蓝牙从机
/***********************************************************************/


/**************************************************
@bref		切换蓝牙状态机
@param
    fsm		新状态
@return
@note
**************************************************/

static void bleServChangeFsm(bleFsm_e fsm)
{
    bleServInfo.bleFsm = fsm;
}
/**************************************************
@bref		蓝牙接收回调
@param
@return
@note
**************************************************/

static void bleServCallBack(void)
{
    int length;
    char *precv = NULL;
    length = (int)nwy_ble_receive_data(0);
    precv = nwy_ble_receive_data(1);
    if ((NULL != precv) & (0 != length))
    {
        bleServRecvParser((uint8_t *)precv, length);
    }
    else
    {
        LogMessage(DEBUG_ALL, "Ble Recv Null");
    }
    nwy_ble_receive_data(2);
}
/**************************************************
@bref		蓝牙连接状态
@param
@return
@note
**************************************************/

static void bleServConnStatusChange(void)
{
    static int8_t lastState = 0;
    int8_t curState = 0;
    curState = nwy_ble_get_conn_status();

    if (curState == lastState)
    {
        return;
    }
    lastState = curState;

    if (curState)
    {
        LogMessage(DEBUG_ALL, "Bluetooth connected");
    }
    else
    {
        LogMessage(DEBUG_ALL, "Bluetooth disconnected");
    }
    return;
}

/**************************************************
@bref		蓝牙配置广播名称
@param
@return
@note
**************************************************/

static void bleServBroadCastCfg(void)
{
    int ret;
    char blename[30];
    sprintf(blename, "AUTO-%s", sysparam.SN + 9);
    ret = nwy_ble_set_device_name(blename);
    LogPrintf(DEBUG_ALL, "Config BLE braodcast to %s , ret %d", blename, ret);
}

/**************************************************
@bref		请求暂时关闭蓝牙
@param
@return
@note
**************************************************/

void bleServCloseRequestSet(void)
{
    bleServInfo.bleCloseReq = 1;
}

/**************************************************
@bref		取消暂时关闭蓝牙
@param
@return
@note
**************************************************/

void bleServCloseRequestClear(void)
{
    bleServInfo.bleCloseReq = 0;
}

/**************************************************
@bref		请求开启蓝牙5分钟
@param
@return
@note		如果要开蓝牙主机，则不开从机
**************************************************/

void bleServRequestOn5Minutes(void)
{
    if (bleScheduleGetCnt() != 0)
    {
        return ;
    }
    sysinfo.bleOnBySystem = 1;
}


/**************************************************
@bref		蓝牙从机运行状态机
@param
@return
@note
**************************************************/

void bleServRunTask(void)
{
    static uint16_t bleRunTick = 0;
    int ret;

    if (sysinfo.bleOnBySystem != 0)
    {
        if (++bleRunTick >= 300)
        {
            sysinfo.bleOnBySystem = 0;
            bleRunTick = 0;
        }
    }

    bleServConnStatusChange();
    switch (bleServInfo.bleFsm)
    {
        case BLE_CHECK_STATE:

            if ((sysparam.bleen == 0 && sysinfo.bleOnBySystem == 0) || bleServInfo.bleCloseReq == 1)
            {
                return;
            }
            ret = nwy_read_ble_status();
            LogPrintf(DEBUG_ALL, "Device BLE was %s", ret ? "Open" : "Close");
            if (ret)
            {
                nwy_ble_set_adv(1);
                bleServInfo.bleState = 1;
                nwy_ble_register_callback(bleServCallBack);
                bleServChangeFsm(BLE_NORMAL);
            }
            else
            {
                bleServBroadCastCfg();
                bleServInfo.bleState = 0;
                bleServChangeFsm(BLE_OPEN);
            }
            break;
        case BLE_OPEN:
            nwy_ble_enable();
            LogMessage(DEBUG_ALL, "open BLE");
            bleServChangeFsm(BLE_CHECK_STATE);
            break;
        case BLE_NORMAL:

            if ((sysparam.bleen == 0 && sysinfo.bleOnBySystem == 0) || bleServInfo.bleCloseReq == 1)
            {
                nwy_ble_disable();
                LogMessage(DEBUG_ALL, "close BLE");
                bleServChangeFsm(BLE_CHECK_STATE);
            }
            break;
        default:
            bleServChangeFsm(BLE_CHECK_STATE);
            break;
    }
}


/**************************************************
@bref		蓝牙数据接收
@param
@return
@note
**************************************************/

static void bleServRecvParser(uint8_t *buf, uint8_t len)
{
    char ret;
    char dec[256];
    char debug[101];
    uint8_t declen, debugLen;
    ITEM item;
    bleInfo_s devInfo;
    instructionParam_s insParam;
    LogPrintf(DEBUG_ALL, "BLERecv==>%d Bytes", len);
    debugLen = len > 50 ? 50 : len;
    LogMessage(DEBUG_ALL, "------------------");
    changeByteArrayToHexString(buf, (uint8_t *)debug, debugLen);
    debug[debugLen * 2] = 0;
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "------------------");

    ret = dencryptData(dec, &declen, (char *)buf, len);
    if (ret == 0)
    {
        return;
    }
    dec[declen] = '#';
    declen += 1;
    dec[declen] = 0;
    LogPrintf(DEBUG_ALL, "BLERECV==>%s", dec);

    if (dec[0] == 'B' && dec[1] == 'L' && dec[2] == ':')
    {
        //有问题这里，待解决
        memset(&insParam, 0, sizeof(instructionParam_s));
        insParam.mode = BLE_MODE;
        instructionParser((uint8_t *)dec + 3, declen - 3, &insParam);
    }
    else if (dec[0] == 'S' && dec[1] == 'N' && dec[2] == ':')
    {
        stringToItem(&item, (uint8_t *) dec + 3, declen - 3);
        if (item.item_cnt == 4)
        {
            strncpy(devInfo.imei, item.item_data[0], 15);
            devInfo.imei[15] = 0;
            devInfo.startCnt = atoi(item.item_data[1]);
            devInfo.vol = atof(item.item_data[2]);
            devInfo.batLevel = atoi(item.item_data[3]);
            bleServerAddInfo(devInfo);
        }
    }
    else if (dec[0] == 'R' && dec[1] == 'E' && dec[2] == ':')
    {
        setInsId();
        sendProtocolToServer(BLE_LINK, PROTOCOL_21, (void *)dec);
    }
}

/**************************************************
@bref		蓝牙数据发送
@param
@return
@note
**************************************************/

uint8_t bleServSendData(char *buf, uint16_t len)
{
    int ret;
    LogMessage(DEBUG_ALL, "bleServSendData==>sending...");
    ret = nwy_ble_send_data(len, buf);
    LogPrintf(DEBUG_ALL, "bleServSendData==>%s", ret == 1 ? "success" : "fail");
    return ret;
}


/***********************************************************************/
//                         蓝牙主机
/***********************************************************************/

/**************************************************
@bref		蓝牙主机信息初始化
@param
@return
@note
**************************************************/

void bleClientInfoInit(void)
{
    memset(&bleClientInfo, 0, sizeof(bleClinet_s));
}

/**************************************************
@bref		蓝牙主机设置待链接信息
@param
	mac
	type
@return
@note
**************************************************/

static void bleClientSetConnect(char *mac, uint8_t type)
{
    strcpy(bleClientInfo.connectMac, mac);
    bleClientInfo.addrType = type;
    LogPrintf(DEBUG_ALL, "MAC:%s", bleClientInfo.connectMac);
}

/**************************************************
@bref		蓝牙主机扫描回调
@param
@return
@note
**************************************************/

static void bleClientScanCallBack(void)
{
    char debug[50];
    nwy_ble_c_scan_dev scan;
    nwy_ble_client_scan_result(&scan);
    changeByteArrayToHexString(scan.bdAddress.addr, (uint8_t *)debug, 6);
    debug[12] = 0;
    LogPrintf(DEBUG_ALL, "MAC:%s,name:%s,rssi:%d,addrType:%d", debug, scan.name, -scan.rssi, scan.addr_type);
}
/**************************************************
@bref		蓝牙主机数据接收回调
@param
@return
@note
**************************************************/

static void bleClientRecvCallBack(void)
{
    //char debug[61];
    //uint8_t debuglen;
    nwy_ble_c_recv_info recv;
    nwy_ble_client_recv_data(&recv);
    //memset(debug, 0, 50);
    //debuglen = recv.len > 30 ? 30 : recv.len;
    //changeByteArrayToHexString((uint8_t *)recv.data, (uint8_t *)debug, debuglen);
    //debug[debuglen * 2] = 0;
    //LogPrintf(DEBUG_ALL, "RECV[%d,%d](%d):%s", recv.ser_id, recv.char_id, recv.len, debug);
    if (bleClientInfo.bleDataLen + recv.len >= BLE_RECV_BUFF_SIZE)
    {
        bleClientInfo.bleDataLen = 0;
    }
    memcpy(bleClientInfo.bleDataBuff + bleClientInfo.bleDataLen, recv.data, recv.len);
    bleClientInfo.bleDataLen += recv.len;
    bleClientSendEvent(BLE_CLIENT_RECV);
    //bleRecvParser(recv.data, recv.len);
}

/**************************************************
@bref		蓝牙主机查找服务特征
@param
@return
@note
**************************************************/

static void bleClientDiscoverChar()
{
    nwy_ble_c_discover char_info[10];
    uint8_t char_num = 0;

    nwy_ble_client_discover_char(char_info);
    LogMessage(DEBUG_ALL, "SRVID--CHARID--CHARUUID--PROP");
    for (int i = 0; i < bleClientInfo.srvNum; i++)
    {
        char_num = char_info[i].charNum;
        for (int j = 0; j < char_num; j++)
        {
            LogPrintf(DEBUG_ALL, "%d,%d,[%04X],0x%02X", i, j, char_info[i].discover_char[j].uuid,
                      char_info[i].discover_char[j].properties);
        }
    }
}
/**************************************************
@bref		蓝牙主机数据发送
@param
@return
@note
**************************************************/

uint8_t bleClientSendData(uint8_t serId, uint8_t charId, uint8_t *data, uint8_t len)
{
    int ret;
    nwy_ble_c_send_param ssd;
    ssd.ser_id = serId;
    ssd.char_id = charId;
    ssd.data = data;
    ssd.len = len;
    ret = nwy_ble_client_send_data(&ssd);
    if (ret != 1)
    {
        LogMessage(DEBUG_ALL, "ble client send fail");
    }
    return ret;
}

static void bleClientSendEvent(ble_clientevent_e event)
{
    appSendThreadEvent(THREAD_EVENT_BLE_CLIENT, event);
}


/**************************************************
@bref		蓝牙主机事件处理
@param
@return
@note
**************************************************/

void bleClientDoEventProcess(uint8_t id)
{
    int ret, i;
    nwy_ble_c_discover srv_info[20];
    switch (id)
    {
        case BLE_CLIENT_OPEN:
            ret = nwy_ble_client_set_enable(1);
            if (ret == 1)
            {
                bleClientInfo.bleClientOnoff = 1;
                LogMessage(DEBUG_ALL, "ble client open success");
            }
            else
            {
                LogMessage(DEBUG_ALL, "ble client open fail");
            }
            nwy_ble_client_register_cb(bleClientScanCallBack, BLE_CLIENT_SCAN_DEV);
            nwy_ble_client_register_cb(bleClientRecvCallBack, BLE_CLIENT_RECV_DATA);
            break;
        case BLE_CLIENT_CLOSE:
            ret = nwy_ble_client_set_enable(0);
            bleClientInfo.bleClientOnoff = 0;
            if (ret == 0)
            {
                LogMessage(DEBUG_ALL, "ble client close success");
            }
            else
            {
                LogMessage(DEBUG_ALL, "ble client close fail");
            }
            break;
        case BLE_CLIENT_SCAN:
            LogMessage(DEBUG_ALL, "ble client begin scan");
            ret = nwy_ble_client_scan(BLE_CLIENT_SCAN_TIME);
            if (ret == 1)
            {
                LogMessage(DEBUG_ALL, "ble client scan success");
            }
            else
            {
                LogMessage(DEBUG_ALL, "ble client scan fail");
            }
            break;
        case BLE_CLIENT_CONNECT:
            LogPrintf(DEBUG_ALL, "try to connect %s", bleClientInfo.connectMac);
            bleClientInfo.bleConnState = 0;
            ret = nwy_ble_client_connect(bleClientInfo.addrType, bleClientInfo.connectMac);
            if (ret == 1)
            {
                bleClientInfo.bleConnState = 1;
                LogMessage(DEBUG_ALL, "ble connect success");
            }
            else
            {
                bleClientInfo.bleConnState = 0;
                LogMessage(DEBUG_ALL, "ble connect fail");
            }
            break;
        case BLE_CLIENT_DISCONNECT:

            ret = nwy_ble_client_disconnect();
            if (ret == 1)
            {
                LogMessage(DEBUG_ALL, "ble disconnect success");
            }
            else
            {
                LogMessage(DEBUG_ALL, "ble disconnect fail");
            }
            break;
        case BLE_CLIENT_DISCOVER:
            LogPrintf(DEBUG_ALL, "ble start discover");
            bleClientInfo.srvNum = nwy_ble_client_discover_srv(srv_info);
            if (bleClientInfo.srvNum != 0)
            {
                for (i = 0; i < bleClientInfo.srvNum; i++)
                {
                    LogPrintf(DEBUG_ALL, "%d==>UUID:[%04X]", i, srv_info[i].uuid);
                }
                bleClientDiscoverChar();
            }
            LogPrintf(DEBUG_ALL, "ble discover done");
            break;
        case BLE_CLIENT_RECV:
            bleRecvParser(bleClientInfo.bleDataBuff, bleClientInfo.bleDataLen);
            bleClientInfo.bleDataLen = 0;
            break;
    }
}

/***********************************************************************/
//                         蓝牙循环链接设计
/***********************************************************************/
/**************************************************
@bref		蓝牙主机调度器信息初始化
@param
@return
@note
**************************************************/

void bleScheduleInit(void)
{
    uint8_t i;
    memset(&bleSchedule, 0, sizeof(bleConnectShchedule_s));

    for (i = 0; i < 5; i++)
    {
        if (sysparam.bleConnMac[i][0] != 0)
        {
            bleScheduleInsert((char *)sysparam.bleConnMac[i]);
        }
    }
    if (bleSchedule.bleListCnt != 0)
    {
        bleScheduleCtrl(1);
    }

    if (sysparam.relayCtl)
    {
        //状态同步
        relayAutoRequest();
    }
}

/**************************************************
@bref		读取蓝牙主机调度器当前的待链接数量
@param
@return
@note
**************************************************/

uint8_t bleScheduleGetCnt(void)
{
    return bleSchedule.bleListCnt;
}


/**************************************************
@bref		蓝牙调度器控制开关
@param
@return
@note
**************************************************/

void bleScheduleCtrl(uint8_t onoff)
{
    bleSchedule.bleDo = onoff;
    LogPrintf(DEBUG_ALL, "%s schedule", onoff ? "start" : "stop");
}

/**************************************************
@bref		调度器状态切换
@param
@return
@note
**************************************************/

static void bleScheduleChangeFsm(ble_schedule_fsm_e fsm)
{
    bleSchedule.bleSchFsm = fsm;
    bleSchedule.runTick = 0;
}

/**************************************************
@bref		蓝牙连接状态切换
@param
@return
@note
**************************************************/

static void bleConnTryChangeFsm(ble_conn_fsm_e fsm)
{
    bleSchedule.bleConnFsm = fsm;
    bleSchedule.connTick = 0;
}

/**************************************************
@bref		蓝牙发送协议
@param
	cmd		指令类型
	data	数据
	data_len数据长度
@return
@note
**************************************************/

static void bleSendProtocol(unsigned char cmd, unsigned char *data, int data_len)
{
    unsigned char i, size_len, lrc;
    //char message[50];
    char mcu_data[32];
    size_len = 0;
    mcu_data[size_len++] = 0x0c;
    mcu_data[size_len++] = data_len + 1;
    mcu_data[size_len++] = cmd;
    i = 3;
    if (data_len > 0 && data == NULL)
    {
        return;
    }
    while (data_len)
    {
        mcu_data[size_len++] = *data++;
        i++;
        data_len--;
    }
    lrc = 0;
    for (i = 1; i < size_len; i++)
    {
        lrc += mcu_data[i];
    }
    mcu_data[size_len++] = lrc;
    mcu_data[size_len++] = 0x0d;
    //changeByteArrayToHexString((uint8_t *)mcu_data, (uint8_t *)message, size_len);
    //message[size_len * 2] = 0;
    //LogPrintf(DEBUG_ALL, "ble send :%s", message);
    bleClientSendData(0, 0, (uint8_t *) mcu_data, size_len);


}

/**************************************************
@bref		协议解析
@param
	data
	len
@return
@note		0C 04 80 09 04 E7 78 0D
**************************************************/

static void bleRecvParser(uint8_t *data, uint8_t len)

{
    uint8_t readInd, size, crc, i;
    uint16_t value16;
    float valuef;
    if (len <= 5)
    {
        return;
    }
    for (readInd = 0; readInd < len; readInd++)
    {
        if (data[readInd] != 0x0C)
        {
            continue;
        }
        if (readInd + 4 >= len)
        {
            //内容超长了
            break;
        }
        size = data[readInd + 1];
        if (readInd + 3 + size >= len)
        {
            continue;
        }
        if (data[readInd + 3 + size] != 0x0D)
        {
            continue;
        }
        crc = 0;
        for (i = 0; i < (size + 1); i++)
        {
            crc += data[readInd + 1 + i];
        }
        if (crc != data[readInd + size + 2])
        {
            continue;
        }
        //LogPrintf(DEBUG_ALL, "CMD[0x%02X]", data[readInd + 3]);
        /*状态更新*/
        bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.updateTick = sysinfo.sysTick;
        if (bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.bleLost == 1)
        {
            alarmRequestSet(ALARM_BLE_RESTORE_REQUEST);
            LogPrintf(DEBUG_ALL, "BLE %s restore", bleSchedule.bleList[bleSchedule.bleCurConnInd].bleMac);
        }
        bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.bleLost = 0;
        switch (data[readInd + 3])
        {
            case CMD_GET_SHIELD_CNT:
                value16 = data[readInd + 4];
                value16 = value16 << 8 | data[readInd + 5];
                LogPrintf(DEBUG_ALL, "BLE==>shield occur cnt %d", value16);
                break;
            case CMD_CLEAR_SHIELD_CNT:
                LogMessage(DEBUG_ALL, "BLE==>clear success");
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_CLR_CNT);
                break;
            case CMD_DEV_ON:
                LogMessage(DEBUG_ALL, "BLE==>relayon success");
                alarmRequestSet(ALARM_OIL_CUTDOWN_REQUEST);
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_SET_DEVON);
                break;
            case CMD_DEV_OFF:
                LogMessage(DEBUG_ALL, "BLE==>relayoff success");
                alarmRequestSet(ALARM_OIL_RESTORE_REQUEST);
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_SET_DEVOFF);
                break;
            case CMD_SET_VOLTAGE:
                LogMessage(DEBUG_ALL, "BLE==>set shiled voltage success");
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_SET_RF_THRE);
                break;
            case CMD_GET_VOLTAGE:

                valuef = data[readInd + 4] / 100.0;
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.rf_threshold = valuef;
                LogPrintf(DEBUG_ALL, "BLE==>get shield voltage range %.2fV", valuef);
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_GET_RF_THRE);
                break;
            case CMD_GET_ADCV:
                value16 = data[readInd + 4];
                value16 = value16 << 8 | data[readInd + 5];
                valuef = value16 / 100.0;
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.rfV = valuef;
                LogPrintf(DEBUG_ALL, "BLE==>shield voltage %.2fV", valuef);
                break;
            case CMD_SET_OUTVOLTAGE:
                LogMessage(DEBUG_ALL, "BLE==>set acc voltage success");
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_SET_OUTV_THRE);
                break;
            case CMD_GET_OUTVOLTAGE:
                value16 = data[readInd + 4];
                value16 = value16 << 8 | data[readInd + 5];
                valuef = value16 / 100.0;
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.out_threshold = valuef;
                LogPrintf(DEBUG_ALL, "BLE==>get outside voltage range %.2fV", valuef);
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_GET_OUT_THRE);
                break;
            case CMD_GET_OUTV:
                value16 = data[readInd + 4];
                value16 = value16 << 8 | data[readInd + 5];
                valuef = value16 / 100.0;
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.outV = valuef;
                LogPrintf(DEBUG_ALL, "BLE==>outside voltage %.2fV", valuef);
                break;
            case CMD_GET_ONOFF:
                LogPrintf(DEBUG_ALL, "BLE==>relay state %d", data[readInd + 4]);
                break;
            case CMD_ALARM:
                //顺便把继电器也给断了
                sysparam.relayCtl = 1;
                paramSaveAll();
                relayAutoRequest();
                LogMessage(DEBUG_ALL, "BLE==>shield alarm occur");
                LogMessage(DEBUG_ALL, "oh, 蓝牙屏蔽报警...");
                alarmRequestSet(ALARM_SHIELD_REQUEST);
                bleScheduleSetReq(bleSchedule.bleCurConnInd, BLE_EVENT_CLR_ALARM);
                break;
            case CMD_AUTODIS:
                LogMessage(DEBUG_ALL, "BLE==>set auto disconnect success");
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_SET_AD_THRE);
                break;
            case CMD_CLEAR_ALARM:
                LogMessage(DEBUG_ALL, "BLE==>clear alarm success");
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_CLR_ALARM);
                break;
            case CMD_PREALARM:
                LogMessage(DEBUG_ALL, "BLE==>preshield alarm occur");
                LogMessage(DEBUG_ALL, "oh, 蓝牙预警...");
                alarmRequestSet(ALARM_PREWARN_REQUEST);
                bleScheduleSetReq(bleSchedule.bleCurConnInd, BLE_EVENT_CLR_PREALARM);
                break;
            case CMD_CLEAR_PREALARM:
                LogMessage(DEBUG_ALL, "BLE==>clear prealarm success");
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_CLR_PREALARM);
                break;
            case CMD_SET_PRE_ALARM_PARAM:
                LogMessage(DEBUG_ALL, "BLE==>set prealarm param success");
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_SET_PRE_PARAM);
                break;
            case CMD_GET_PRE_ALARM_PARAM:
                LogPrintf(DEBUG_ALL, "BLE==>get prealarm param [%d,%d,%d] success", data[readInd + 4], data[readInd + 5],
                          data[readInd + 6]);
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.preV_threshold = data[readInd + 4];
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.preDetCnt_threshold = data[readInd + 5];
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.preHold_threshold = data[readInd + 6];
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_GET_PRE_PARAM);
                break;
            case CMD_GET_DISCONNECT_PARAM:
                LogPrintf(DEBUG_ALL, "BLE==>auto disc %d minutes", data[readInd + 4]);
                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleInfo.disc_threshold = data[readInd + 4];
                bleScheduleClearReq(bleSchedule.bleCurConnInd, BLE_EVENT_GET_AD_THRE);
                break;
        }
        readInd += size + 3;
    }
}

/**************************************************
@bref		读取链接列表中蓝牙的相关信息
@param
@return
	NULL	无值
	非NULL	有值
@note
**************************************************/

bleRelayInfo_s *bleGetDevInfo(uint8_t i)
{
    bleRelayInfo_s *info = NULL;
    if (i >= BLE_CONNECT_LIST_SIZE)
    {
        return NULL;
    }
    if (bleSchedule.bleList[i].bleUsed == 0)
    {
        return NULL;
    }
    info = &bleSchedule.bleList[i].bleInfo;
    return info;
}



/**************************************************
@bref		插入需要链接的蓝牙
@param
	mac		蓝牙地址，需要aa:bb:cc:dd:ee:ff 格式
@return
@note
**************************************************/

int8_t bleScheduleInsert(char *mac)
{
    int8_t ind = 0;
    if (bleSchedule.bleListCnt >= BLE_CONNECT_LIST_SIZE)
    {
        return -1;
    }

    for (ind = 0; ind < BLE_CONNECT_LIST_SIZE; ind++)
    {
        if (bleSchedule.bleList[ind].bleUsed == 0)
        {
            memset(&bleSchedule.bleList[ind], 0, sizeof(bleConnectList_s));
            bleSchedule.bleList[ind].bleUsed = 1;
            strncpy(bleSchedule.bleList[ind].bleMac, mac, 17);
            bleSchedule.bleList[ind].bleType = 0;
            bleSchedule.bleList[ind].bleInfo.updateTick = sysinfo.sysTick;
            LogPrintf(DEBUG_ALL, "BLE insert [%d]:%s", ind, mac);
            bleScheduleSetReq(ind, BLE_EVENT_SET_RF_THRE | BLE_EVENT_SET_OUTV_THRE | BLE_EVENT_SET_AD_THRE | BLE_EVENT_GET_AD_THRE |
                              BLE_EVENT_GET_RF_THRE | BLE_EVENT_GET_OUT_THRE | BLE_EVENT_GET_OUTV | BLE_EVENT_GET_RFV |
                              BLE_EVENT_GET_PRE_PARAM | BLE_EVENT_SET_PRE_PARAM | BLE_EVENT_GET_PRE_PARAM);
            bleSchedule.bleListCnt++;
            return ind;
        }
    }
    return -2;
}

/**************************************************
@bref		删除链接列表中的蓝牙
@param
	ind		蓝牙索引
@return
@note
**************************************************/

void bleScheduleDelete(uint8_t ind)
{
    if (ind >= BLE_CONNECT_LIST_SIZE)
    {
        return;
    }
    if (bleSchedule.bleListCnt == 0)
    {
        return ;
    }
    if (bleSchedule.bleList[ind].bleUsed)
    {
        bleSchedule.bleList[ind].bleUsed = 0;
        bleSchedule.bleListCnt--;
        LogPrintf(DEBUG_ALL, "BLE delete [%d]:%s", ind, bleSchedule.bleList[ind].bleMac);
    }
    else
    {
        LogPrintf(DEBUG_ALL, "BLE delete not find [%d]", ind);
    }
}
/**************************************************
@bref		清空链接列表
@param
@return
@note
**************************************************/

void bleScheduleWipeAll(void)
{
    memset(bleSchedule.bleList, 0, sizeof(bleSchedule.bleList));
    bleSchedule.bleListCnt = 0;
    bleSchedule.bleQuickRun = 0;
    LogPrintf(DEBUG_ALL, "BLE wipe schedule list");
}
/**************************************************
@bref		设置蓝牙需要发送的数据事件
@param
	ind		蓝牙编号
	event	数据事件
@return
@note
**************************************************/

void bleScheduleSetReq(uint8_t ind, uint32_t event)
{
    bleSchedule.bleList[ind].dataReq |= event;
    LogPrintf(DEBUG_ALL, "set bleList[%X] req to 0x%02x", ind, event);
}

/**************************************************
@bref		同步所有事件
@param
	event	数据事件
@return
@note
**************************************************/

void bleScheduleSetAllReq(uint32_t event)
{
    uint8_t i;
    for (i = 0; i < BLE_CONNECT_LIST_SIZE; i++)
    {
        bleSchedule.bleList[i].dataReq |= event;
    }
    bleSchedule.bleQuickRun = bleSchedule.bleListCnt;
    //LogPrintf(DEBUG_ALL, "set ble all req to 0x%02x", event);
}
/**************************************************
@bref		清除所有事件
@param
	event	数据事件
@return
@note
**************************************************/
void bleScheduleClearAllReq(uint32_t event)
{
    uint8_t i;
    for (i = 0; i < BLE_CONNECT_LIST_SIZE; i++)
    {
        bleSchedule.bleList[i].dataReq &= ~event;
    }
    //LogPrintf(DEBUG_ALL, "clear ble all req 0x%02x", event);
}

/**************************************************
@bref		清除蓝牙发送事件
@param
	ind		蓝牙编号
	event	数据事件
@return
@note
**************************************************/

void bleScheduleClearReq(uint8_t ind, uint32_t event)
{
    bleSchedule.bleList[ind].dataReq &= ~event;
    //LogPrintf(DEBUG_ALL, "clear bleList[%X] req 0x%02x", ind, event);
}

/**************************************************
@bref		蓝牙断连侦测
@param
@return
@note
**************************************************/

static void bleDiscDetector(void)
{
    uint8_t ind;
    uint32_t tick;
    if (bleSchedule.bleListCnt == 0)
    {
        return;
    }
    if (sysparam.bleAutoDisc == 0)
    {
        return;
    }
    for (ind = 0; ind < BLE_CONNECT_LIST_SIZE; ind++)
    {
        if (bleSchedule.bleList[ind].bleUsed != 1)
        {
            return;
        }
        if (bleSchedule.bleList[ind].bleInfo.bleLost == 1)
        {
            return;
        }
        tick = sysinfo.sysTick - bleSchedule.bleList[ind].bleInfo.updateTick;
        if (tick >= (sysparam.bleAutoDisc * 60))
        {
            bleSchedule.bleList[ind].bleInfo.bleLost = 1;
            alarmRequestSet(ALARM_BLE_LOST_REQUEST);
            LogPrintf(DEBUG_ALL, "oh ,BLE [%s] lost", bleSchedule.bleList[ind].bleMac);
        }
    }
}


/**************************************************
@bref		蓝牙数据发送
@param
@return
	0		等待超时
	1		提前退出
@note
**************************************************/

static uint8_t bleDataSendTry(void)
{
    uint32_t event;
    uint8_t param[10];
    uint8_t ret = 0;
    uint16_t value16;

    event = bleSchedule.bleList[bleSchedule.bleCurConnInd].dataReq;

    //固定发送组
    if (bleSchedule.sendTick % 25 == 0)
    {
        if (event & BLE_EVENT_GET_OUTV)
        {
            LogMessage(DEBUG_ALL, "try to get outside voltage");
            bleSendProtocol(CMD_GET_OUTV, param, 0);
        }
        if (event & BLE_EVENT_GET_RFV)
        {
            LogMessage(DEBUG_ALL, "try to get rf voltage");
            bleSendProtocol(CMD_GET_ADCV, param, 0);
        }

    }
    //非固定发送组
    if (bleSchedule.sendTick % 2 == 0)
    {
        if (event & BLE_EVENT_SET_DEVON)
        {
            LogMessage(DEBUG_ALL, "try to set relay on");
            bleSendProtocol(CMD_DEV_ON, param, 0);
        }
        if (event & BLE_EVENT_SET_DEVOFF)
        {
            LogMessage(DEBUG_ALL, "try to set relay off");
            bleSendProtocol(CMD_DEV_OFF, param, 0);
        }
        if (event & BLE_EVENT_CLR_CNT)
        {
            LogMessage(DEBUG_ALL, "try to clear shield");
            bleSendProtocol(CMD_CLEAR_SHIELD_CNT, param, 0);
        }
        if (event & BLE_EVENT_SET_RF_THRE)
        {
            LogMessage(DEBUG_ALL, "try to set shield voltage");
            param[0] = sysparam.bleRfThreshold;
            bleSendProtocol(CMD_SET_VOLTAGE, param, 1);
        }
        if (event & BLE_EVENT_SET_OUTV_THRE)
        {
            LogMessage(DEBUG_ALL, "try to set accoff voltage");
            value16 = sysparam.bleOutThreshold;
            param[0] = value16 >> 8 & 0xFF;
            param[1] = value16 & 0xFF;
            bleSendProtocol(CMD_SET_OUTVOLTAGE, param, 2);
        }
        if (event & BLE_EVENT_SET_AD_THRE)
        {
            LogMessage(DEBUG_ALL, "try to set auto disconnect param");
            param[0] = sysparam.bleAutoDisc;
            bleSendProtocol(CMD_AUTODIS, param, 1);
        }
        if (event & BLE_EVENT_CLR_ALARM)
        {
            LogMessage(DEBUG_ALL, "try to clear alarm");
            bleSendProtocol(CMD_CLEAR_ALARM, param, 1);
        }

        if (event & BLE_EVENT_GET_RF_THRE)
        {
            LogMessage(DEBUG_ALL, "try to get rf threshold");
            bleSendProtocol(CMD_GET_VOLTAGE, param, 0);
        }

        if (event & BLE_EVENT_GET_OUT_THRE)
        {
            LogMessage(DEBUG_ALL, "try to get out threshold");
            bleSendProtocol(CMD_GET_OUTVOLTAGE, param, 0);
        }
        if (event & BLE_EVENT_CLR_PREALARM)
        {
            LogMessage(DEBUG_ALL, "try to clear prealarm");
            bleSendProtocol(CMD_CLEAR_PREALARM, param, 1);
        }
        if (event & BLE_EVENT_SET_PRE_PARAM)
        {
            LogMessage(DEBUG_ALL, "try to set preAlarm param");
            param[0] = sysparam.blePreShieldVoltage;
            param[1] = sysparam.blePreShieldDetCnt;
            param[2] = sysparam.blePreShieldHoldTime;
            bleSendProtocol(CMD_SET_PRE_ALARM_PARAM, param, 3);
        }
        if (event & BLE_EVENT_GET_PRE_PARAM)
        {
            LogMessage(DEBUG_ALL, "try to get preAlarm param");
            bleSendProtocol(CMD_GET_PRE_ALARM_PARAM, param, 0);
        }
        if (event & BLE_EVENT_GET_AD_THRE)
        {
            LogMessage(DEBUG_ALL, "try to get auto disc param");
            bleSendProtocol(CMD_GET_DISCONNECT_PARAM, param, 0);
        }

        if (bleSchedule.bleQuickRun != 0 && event & 0xFFFFFF00)
        {
            ret = 1;
            LogMessage(DEBUG_ALL, "oh , urgent event send done");
        }
    }

    bleSchedule.sendTick++;
    return ret;
}


/**************************************************
@bref		蓝牙链接管理状态机
@param
@return
@note
**************************************************/

static void bleConnectTry(void)
{
    uint8_t ind, cur, ret;
    switch (bleSchedule.bleConnFsm)
    {
        case BLE_CONN_IDLE:

            if (bleSchedule.bleListCnt == 0)
            {
                return;
            }
            //开始链接蓝牙从机
            cur = bleSchedule.bleCurConnInd;
            for (ind = 0; ind <= BLE_CONNECT_LIST_SIZE; ind++)
            {
                bleSchedule.bleCurConnInd = (cur + ind) % BLE_CONNECT_LIST_SIZE;
                if (bleSchedule.bleList[bleSchedule.bleCurConnInd].bleUsed == 1)
                {
                    break;
                }
            }
            bleScheduleSetReq(bleSchedule.bleCurConnInd, (BLE_EVENT_GET_OUTV | BLE_EVENT_GET_RFV));
            LogPrintf(DEBUG_ALL, "bleConnectTry==>Ind:%d,Req:0x%X", bleSchedule.bleCurConnInd,
                      bleSchedule.bleList[bleSchedule.bleCurConnInd].dataReq);
            bleClientSetConnect(bleSchedule.bleList[bleSchedule.bleCurConnInd].bleMac,
                                bleSchedule.bleList[bleSchedule.bleCurConnInd].bleType);

            bleClientInfo.bleConnState = 0;
            bleClientSendEvent(BLE_CLIENT_CONNECT);
            bleConnTryChangeFsm(BLE_CONN_WATI);
            break;
        case BLE_CONN_WATI:
            if (bleClientInfo.bleConnState == 1)
            {
                bleClientSendEvent(BLE_CLIENT_DISCOVER);
                bleSchedule.sendTick = 0;
                bleConnTryChangeFsm(BLE_CONN_RUN);
            }
            else
            {
                if (bleSchedule.connTick >= 6)
                {
                    LogMessage(DEBUG_ALL, "bleConnectTry==>connect timeout");
                    //client connect fail
                    if (++bleSchedule.bleConnFailCnt >= 3)
                    {
                        bleConnTryChangeFsm(BLE_CONN_CHANGE);
                    }
                    else
                    {
                        LogMessage(DEBUG_ALL, "bleConnectTry==>try again");
                        bleConnTryChangeFsm(BLE_CONN_IDLE);
                    }
                }
            }
            break;
        case BLE_CONN_RUN:
            //链路维护和数据发送处理
            ret = bleDataSendTry();
            if (bleSchedule.connTick >= BLE_CONN_HOLD_TIME || ret != 0)
            {
                //链接一定时间后，切换其他蓝牙
                bleConnTryChangeFsm(BLE_CONN_CHANGE);
            }
            break;
        case BLE_CONN_CHANGE:
            LogMessage(DEBUG_ALL, "bleConnectTry==>connect another");
            bleSchedule.bleCurConnInd = (bleSchedule.bleCurConnInd + 1) % BLE_CONNECT_LIST_SIZE;
            if (bleSchedule.bleQuickRun != 0)
            {
                bleSchedule.bleQuickRun--;
            }
            if (bleSchedule.bleConnFailCnt >= 3)
            {
                bleScheduleChangeFsm(BLE_SCH_CLOSE);
            }
            else
            {
                bleClientSendEvent(BLE_CLIENT_DISCONNECT);
            }

            bleSchedule.bleConnFailCnt = 0;
            bleConnTryChangeFsm(BLE_CONN_IDLE);
            break;
    }
    bleSchedule.connTick++;

}

/**************************************************
@bref		蓝牙断连侦测
@param
@return
@note
**************************************************/

static void bleErrAutoRestore(void)
{
    uint8_t ind;
    uint32_t tick;

    if (bleSchedule.bleListCnt == 0)
    {
        return;
    }
    if (sysparam.bleErrCnt >= 3)
    {
        return;
    }
    for (ind = 0; ind < BLE_CONNECT_LIST_SIZE; ind++)
    {
        if (bleSchedule.bleList[ind].bleUsed != 1)
        {
            return;
        }
        if (bleSchedule.bleList[ind].bleInfo.bleLost == 1)
        {
            return;
        }
        tick = sysinfo.sysTick - bleSchedule.bleList[ind].bleInfo.updateTick;
        if (tick >= 600)
        {
            tick = 0;
            sysparam.bleErrCnt++;
            paramSaveAll();
            LogPrintf(DEBUG_ALL, "bleErrAutoRestore==>%d",sysparam.bleErrCnt);
            portSystemReset();
        }
    }
}

/**************************************************
@bref		蓝牙调度任务
@param
@return
@note		负责蓝牙的开关、切换和数据发送功能
**************************************************/

void bleScheduleTask(void)
{

    /*蓝牙断连侦测，产生蓝牙断连报警*/
    bleDiscDetector();
    /*蓝牙异常侦测，每天重启前三次蓝牙错误，自动重启设备，之后蓝牙错误，需等待每天重启*/
    bleErrAutoRestore();

    switch (bleSchedule.bleSchFsm)
    {
        case BLE_SCH_OPEN:
            if (bleSchedule.bleDo == 0)
            {
                if (bleClientInfo.bleClientOnoff == 1)
                {
                    bleScheduleChangeFsm(BLE_SCH_CLOSE);
                }
                return;
            }
            if (bleClientInfo.bleClientOnoff == 1)
            {
                bleScheduleChangeFsm(BLE_SCH_RUN);
            }
            else
            {
                if (bleSchedule.bleTryOpen == 0)
                {
                    bleSchedule.bleTryOpen = 1;
                    bleClientSendEvent(BLE_CLIENT_OPEN);
                }
                if (bleSchedule.runTick >= 5)
                {
                    bleScheduleChangeFsm(BLE_SCH_CLOSE);
                }
            }
            break;
        case BLE_SCH_RUN:
            if (bleSchedule.bleDo == 0)
            {
                bleScheduleChangeFsm(BLE_SCH_CLOSE);
                bleConnTryChangeFsm(BLE_CONN_IDLE);
            }
            else
            {
                bleConnectTry();
                break;
            }
        case BLE_SCH_CLOSE:
            bleSchedule.bleTryOpen = 0;
            bleSchedule.bleRebootCnt++;
            if (bleSchedule.bleRebootCnt >= 100)
            {
                bleSchedule.bleRebootCnt = 0;
                alarmRequestSet(ALARM_BLE_ERR_REQUEST);
            }
            bleClientSendEvent(BLE_CLIENT_CLOSE);
            bleScheduleChangeFsm(BLE_SCH_OPEN);
            break;
    }

    bleSchedule.runTick++;

}

