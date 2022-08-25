#include "app_protocol_808.h"
#include "app_sys.h"
#include "app_net.h"
#include "app_task.h"
#include "app_gps.h"
#include "app_param.h"
#include "app_port.h"
#include "app_atcmd.h"
#include "app_instructioncmd.h"
static JT808_NETWORK_STRUCT jt808_info;
static JT808_BASE_POSITION_INFO jt808_position;
static JT808_TERMINAL_GENERIC_RESPON jt808_terminalRespon;
static GPSRestoreStruct jt808_gpsres;
static void jt808ChangeFsm(JT808_FSM_STATE fsm);

/************************************************************************************/
void jt808InfoInit(void)
{
    memset(&jt808_info, 0, sizeof(JT808_NETWORK_STRUCT));
    memset(&jt808_position, 0, sizeof(JT808_BASE_POSITION_INFO));
    memset(&jt808_terminalRespon, 0, sizeof(JT808_TERMINAL_GENERIC_RESPON));
    memset(&jt808_gpsres, 0, sizeof(GPSRestoreStruct));
}
static void jt808ICCIDChangeHexToBcd(uint8_t *bcdiccd, uint8_t *hexiccid)
{
    uint8_t i;
    uint8_t iccidbuff[20];
    for (i = 0; i < 20; i++)
    {
        if (hexiccid[i] >= 'A' && hexiccid[i] <= 'F')
        {
            iccidbuff[i] = hexiccid[i] - 'A';

        }
        else if (hexiccid[i] >= 'a' && hexiccid[i] <= 'f')
        {
            iccidbuff[i] = hexiccid[i] - 'a';

        }
        else if (hexiccid[i] >= '0' && hexiccid[i] <= '9')
        {
            iccidbuff[i] = hexiccid[i] - '0';
        }

    }
    for (i = 0; i < 10; i++)
    {
        bcdiccd[i] = iccidbuff[i * 2];
        bcdiccd[i] <<= 4;
        bcdiccd[i] |= iccidbuff[i * 2 + 1];
    }
}
/*生成808协议sn号*/
void jt808CreateSn(uint8_t *sn, uint8_t snlen)
{
    uint8_t i;
    uint8_t newsn[12];
    memset(newsn, 0, 12);
    if (snlen > 12)
    {
        snlen = 12;
    }
    for (i = 0; i < snlen; i++)
    {
        newsn[11 - i] = sn[snlen - 1 - i] - '0';
    }
    for (i = 0; i < 6; i++)
    {
        sysparam.jt808sn[i] = newsn[i * 2];
        sysparam.jt808sn[i] <<= 4;
        sysparam.jt808sn[i] |= newsn[i * 2 + 1];
    }
    sysparam.jt808isRegister = 0;
    paramSaveAll();
}

void jt808UpdateStatus(uint32_t bit, uint8_t onoff)
{
    if (onoff)
    {
        jt808_position.status |= bit;
    }
    else
    {
        jt808_position.status &= ~bit;
    }
}

void jt808UpdateAlarm(uint32_t bit, uint8_t onoff)
{
    if (onoff)
    {
        jt808_position.alarm |= bit;
    }
    else
    {
        jt808_position.alarm &= ~bit;
    }
}

static uint8_t byteToBcd(uint8_t Value)
{
    uint32_t bcdhigh = 0U;
    uint8_t Param = Value;

    while (Param >= 10U)
    {
        bcdhigh++;
        Param -= 10U;
    }

    return ((uint8_t)(bcdhigh << 4U) | Param);
}


/************************************************************************************/

/*消息流水*/
static uint16_t jt808GetSerial(void)
{
    return jt808_info.serial++;
}
/*消息体属性*/
static uint16_t jt808PackMsgAttribute(uint8_t subpackage, uint8_t encrypt, uint16_t msgLen)
{
    uint16_t attr = 0;
    attr = msgLen & 0x3FF; // 0~9 bit 为消息体长度
    attr |= (encrypt & 0x07) << 10;
    attr |= (subpackage & 0x01) << 13;
    return attr;
}

/*封装消息头*/
static uint16_t jt808PackMessageHead(uint8_t *dest, uint16_t msgId, uint8_t *sn, uint16_t serial, uint16_t msgAttribute)
{
    uint16_t len = 0;
    uint8_t i;
    //标志位
    dest[len++] = 0x7E;
    /********消息头*********/
    //消息ID
    dest[len++] = (msgId >> 8) & 0xff;
    dest[len++] = msgId & 0xff;
    //消息体属性
    dest[len++] = (msgAttribute >> 8) & 0xff;
    dest[len++] = msgAttribute & 0xff;
    //终端手机号/设备号
    for (i = 0; i < 6; i++)
    {
        dest[len++] = sn[i];
    }
    //消息流水
    dest[len++] = (serial >> 8) & 0xff;
    dest[len++] = serial & 0xff;
    //消息包封装
    //暂时空
    /********消息头*********/
    return len;
}

/*计算校验*/
static uint16_t jt808PackMessageTail(uint8_t *dest, uint16_t len)
{
    uint8_t crc = 0;
    uint16_t i, attribute;

    attribute = jt808PackMsgAttribute(0, 0, len - 13);

    dest[3] = (attribute >> 8) & 0xff;
    dest[4] = attribute & 0xff;

    for (i = 1; i < len; i++)
    {
        crc ^= dest[i];
    }
    dest[len++] = crc;
    dest[len++] = 0x7E;
    return len;
}
/*转义*/
static uint16_t jt808Escape(uint8_t *dest, uint16_t len)
{
    uint16_t i, j, k;
    uint16_t msgLen;
    uint8_t senddata[300];
    msgLen = len;
    for (i = 1; i < msgLen - 1; i++)
    {
        if (dest[i] == 0x7E)
        {
            //剩余的数据长度j
            j = msgLen - 1 - i;
            //剩余的数据往后移1个字节
            for (k = 0; k < j; k++)
            {
                dest[msgLen - k] = dest[msgLen - k - 1];
            }
            msgLen += 1;
            dest[i] = 0x7d;
            dest[i + 1] = 0x02;
        }
        else if (dest[i] == 0x7D)
        {
            //剩余的数据长度j
            j = msgLen - 1 - i;
            //剩余的数据往后移1个字节
            for (k = 0; k < j; k++)
            {
                dest[msgLen - k] = dest[msgLen - k - 1];
            }
            msgLen += 1;
            dest[i] = 0x7d;
            dest[i + 1] = 0x01;
        }
    }

    if (msgLen < 150)
    {
        changeByteArrayToHexString(dest, senddata, msgLen);
        senddata[msgLen * 2] = 0;
        LogPrintf(DEBUG_ALL, "jt808Escape==>%s\r\n", senddata);
    }
    return msgLen;
}

/*反转义*/
static uint16_t jt808Unescape(uint8_t *src, uint16_t len)
{
    uint16_t i, j, k;
    for (i = 0; i < (len - 1); i++)
    {
        if (src[i] == 0x7d)
        {
            if (src[i + 1] == 0x02)
            {
                src[i] = 0x7e;
                j = len - (i + 2);
                for (k = 0; k < j; k++)
                {
                    src[i + 1 + k] = src[i + 2 + k];
                }
                len -= 1;
            }
            else if (src[i + 1] == 0x01)
            {
                src[i] = 0x7d;
                j = len - (i + 2);
                for (k = 0; k < j; k++)
                {
                    src[i + 1 + k] = src[i + 2 + k];
                }
                len -= 1;
            }
        }
    }
    return len;
}


/*终端注册*/
static void jt808TerminalRegister(uint8_t *sn, uint8_t *manufacturerID, uint8_t *terminalType, uint8_t *terminalID,
                                  uint8_t color)
{
    uint8_t dest[128];
    uint16_t len;
    uint8_t i;
    len = jt808PackMessageHead(dest, TERMINAL_REGISTER_MSGID, sn, jt808GetSerial(), 0);
    //省域ID
    dest[len++] = 0x00;
    dest[len++] = 0x00;
    //市域ID
    dest[len++] = 0x00;
    dest[len++] = 0x00;
    //制造商ID
    for (i = 0; i < 5; i++)
    {
        dest[len++] = manufacturerID[i];
    }
    //终端型号
    for (i = 0; i < 20; i++)
    {
        dest[len++] = terminalType[i];
    }
    //终端ID
    for (i = 0; i < 7; i++)
    {
        dest[len++] = terminalID[i];
    }
    //车牌颜色
    dest[len++] = color;
    len = jt808PackMessageTail(dest, len);
    len = jt808Escape(dest, len);
    sendDataToServer(NORMAL_LINK, dest, len);
}
/*终端鉴权*/
static void jt808TerminalAuthentication(uint8_t *sn, uint8_t *auth, uint8_t authlen)
{
    uint8_t dest[128];
    uint16_t len;
    uint8_t i;
    len = jt808PackMessageHead(dest, TERMINAL_AUTHENTICATION_MSGID, sn, jt808GetSerial(), 0);
    for (i = 0; i < authlen; i++)
    {
        dest[len++] = auth[i];
    }
    len = jt808PackMessageTail(dest, len);
    len = jt808Escape(dest, len);
    sendDataToServer(NORMAL_LINK, dest, len);
}

/*终端心跳*/
static void jt808TerminalHeartbeat(uint8_t *sn)
{
    uint8_t dest[128];
    uint16_t len;
    len = jt808PackMessageHead(dest, TERMINAL_HEARTBEAT_MSGID, sn, jt808GetSerial(), 0);
    len = jt808PackMessageTail(dest, len);
    len = jt808Escape(dest, len);
    sendDataToServer(NORMAL_LINK, dest, len);
}

/*计算行驶里程*/
static void jt808CalculateMileate(GPSINFO *gpsinfo)
{
    static double lastLatitude = 0;
	static double lastlongtitude = 0;
    double mileage = 0;
    if (gpsinfo->fixstatus !=1)
    {
        return ;
    }
    if (lastLatitude == 0 || lastlongtitude == 0)
    {
        lastLatitude = gpsinfo->latitude;
        lastlongtitude = gpsinfo->longtitude;
        return ;
    }
    mileage = (calculateTheDistanceBetweenTwoPonits(gpsinfo->latitude, gpsinfo->longtitude, lastLatitude, lastlongtitude));
    lastLatitude = gpsinfo->latitude;
    lastlongtitude = gpsinfo->longtitude;
    sysparam.mileage += mileage;
    paramSaveAll();
}

static void jt808PackGpsinfo(GPSINFO *gpsinfo)
{
    double value;
    DATETIME datetimenew;
    /*--------------纬度-----------------*/
    value = gpsinfo->latitude;
    value *= 1000000;
    if (value < 0)
    {
        value *= -1;
    }
    jt808_position.latitude = value;
    /*--------------经度-----------------*/
    value = gpsinfo->longtitude;
    value *= 1000000;
    if (value < 0)
    {
        value *= -1;
    }
    jt808_position.longtitude = value;
    /*--------------其他-----------------*/
    jt808_position.course = gpsinfo->course;
    jt808_position.speed = (uint16_t)(gpsinfo->speed * 10);
    jt808_position.hight = gpsinfo->hight;
    /*--------------日期时间-----------------*/
    datetimenew = changeUTCTimeToLocalTime(gpsinfo->datetime, 8);
    jt808_position.year = byteToBcd(datetimenew.year % 100);
    jt808_position.month = byteToBcd(datetimenew.month);
    jt808_position.day = byteToBcd(datetimenew.day);
    jt808_position.hour = byteToBcd(datetimenew.hour);
    jt808_position.mintues = byteToBcd(datetimenew.minute);
    jt808_position.seconds = byteToBcd(datetimenew.second);
    /*--------------状态位-----------------*/
    if (gpsinfo->fixstatus)
    {
        jt808UpdateStatus(JT808_STATUS_FIX, 1);
    }
    else
    {
        jt808UpdateStatus(JT808_STATUS_FIX, 0);
    }

    if (gpsinfo->NS == 'S')
    {
        jt808UpdateStatus(JT808_STATUS_LATITUDE, 1);
    }
    else
    {
        jt808UpdateStatus(JT808_STATUS_LATITUDE, 0);
    }

    if (gpsinfo->EW == 'W')
    {
        jt808UpdateStatus(JT808_STATUS_LONGTITUDE, 1);
    }
    else
    {
        jt808UpdateStatus(JT808_STATUS_LONGTITUDE, 0);
    }
    /*-------------------------------*/
}

/*位置信息上报*/
static uint16_t jt808TerminalPosition(uint8_t *dest, uint8_t *sn, JT808_BASE_POSITION_INFO *positionInfo)
{
    uint16_t len;
    uint32_t jt808mileage;

    len = jt808PackMessageHead(dest, TERMINAL_POSITION_MSGID, sn, jt808GetSerial(), 0);
    //报警
    dest[len++] = (positionInfo->alarm >> 24) & 0xFF;
    dest[len++] = (positionInfo->alarm >> 16) & 0xFF;
    dest[len++] = (positionInfo->alarm >> 8) & 0xFF;
    dest[len++] = (positionInfo->alarm) & 0xFF;
    //LogPrintf(DEBUG_ALL,"Alarm 0x%08X\r\n",positionInfo->alarm);
    //状态
    dest[len++] = (positionInfo->status >> 24) & 0xFF;
    dest[len++] = (positionInfo->status >> 16) & 0xFF;
    dest[len++] = (positionInfo->status >> 8) & 0xFF;
    dest[len++] = (positionInfo->status) & 0xFF;
    //LogPrintf(DEBUG_ALL,"Status 0x%08X\r\n",positionInfo->status);
    //维度
    dest[len++] = (positionInfo->latitude >> 24) & 0xFF;
    dest[len++] = (positionInfo->latitude >> 16) & 0xFF;
    dest[len++] = (positionInfo->latitude >> 8) & 0xFF;
    dest[len++] = (positionInfo->latitude) & 0xFF;
    //经度
    dest[len++] = (positionInfo->longtitude >> 24) & 0xFF;
    dest[len++] = (positionInfo->longtitude >> 16) & 0xFF;
    dest[len++] = (positionInfo->longtitude >> 8) & 0xFF;
    dest[len++] = (positionInfo->longtitude) & 0xFF;
    //高度
    dest[len++] = (positionInfo->hight >> 8) & 0xFF;
    dest[len++] = (positionInfo->hight) & 0xFF;
    //速度
    dest[len++] = (positionInfo->speed >> 8) & 0xFF;
    dest[len++] = (positionInfo->speed) & 0xFF;
    //方向
    dest[len++] = (positionInfo->course >> 8) & 0xFF;
    dest[len++] = (positionInfo->course) & 0xFF;
    //时间
    dest[len++] = positionInfo->year;
    dest[len++] = positionInfo->month;
    dest[len++] = positionInfo->day;
    dest[len++] = positionInfo->hour;
    dest[len++] = positionInfo->mintues;
    dest[len++] = positionInfo->seconds;
    /*------------------附加消息--------------------*/
    //网络信号强度
    dest[len++] = 0x30;
    dest[len++] = 0x01;
    dest[len++] = getModuleRssi();
    //里程
    jt808mileage = (sysparam.mileage * 1.05) / 100.0;
    dest[len++] = 0x01;
    dest[len++] = 0x04;
    dest[len++] = (jt808mileage >> 24) & 0xFF;
    dest[len++] = (jt808mileage >> 16) & 0xFF;
    dest[len++] = (jt808mileage >> 8) & 0xFF;
    dest[len++] = (jt808mileage) & 0xFF;

    len = jt808PackMessageTail(dest, len);
    len = jt808Escape(dest, len);
    return len;
}


uint16_t jt808gpsRestoreDataSend(GPSRestoreStruct *grs, uint8_t *dest)
{
    GPSINFO gpsinfo;
    uint32_t value;
    uint16_t len;
    gpsinfo.datetime.year = grs->year;
    gpsinfo.datetime.month = grs->month;
    gpsinfo.datetime.day = grs->day;
    gpsinfo.datetime.hour = grs->hour;
    gpsinfo.datetime.minute = grs->minute;
    gpsinfo.datetime.second = grs->second;

    value = (grs->latititude[0] << 24) | (grs->latititude[1] << 16) | (grs->latititude[2] << 8) | (grs->latititude[3]);
    gpsinfo.latitude = (double)value / 1000000.0;
    value = (grs->longtitude[0] << 24) | (grs->longtitude[1] << 16) | (grs->longtitude[2] << 8) | (grs->longtitude[3]);
    gpsinfo.longtitude = (double)value / 1000000.0;

    gpsinfo.speed = grs->speed;
    gpsinfo.course = (grs->coordinate[0] << 8) | grs->coordinate[1];
    gpsinfo.NS = grs->temp[0];
    gpsinfo.EW = grs->temp[1];
    gpsinfo.fixstatus = 1;

    jt808PackGpsinfo(&gpsinfo);
    len = jt808TerminalPosition(dest, sysparam.jt808sn, &jt808_position);
    return len;
}

static void jt808SendPosition(GPSINFO *gpsinfo)
{
    uint8_t dest[100];
    uint8_t len;
	jt808CalculateMileate(gpsinfo);
    jt808PackGpsinfo(gpsinfo);
    jt808_gpsres.year = gpsinfo->datetime.year;
    jt808_gpsres.month = gpsinfo->datetime.month;
    jt808_gpsres.day = gpsinfo->datetime.day;
    jt808_gpsres.hour = gpsinfo->datetime.hour;
    jt808_gpsres.minute = gpsinfo->datetime.minute;
    jt808_gpsres.second = gpsinfo->datetime.second;
    jt808_gpsres.speed = gpsinfo->speed;
    jt808_gpsres.coordinate[0] = (gpsinfo->course >> 8) & 0xff;
    jt808_gpsres.coordinate[1] = (gpsinfo->course) & 0xff;
    jt808_gpsres.temp[0] = gpsinfo->NS;
    jt808_gpsres.temp[1] = gpsinfo->EW;
    jt808_gpsres.latititude[0] = (jt808_position.latitude >> 24) & 0xff;
    jt808_gpsres.latititude[1] = (jt808_position.latitude >> 16) & 0xff;
    jt808_gpsres.latititude[2] = (jt808_position.latitude >> 8) & 0xff;
    jt808_gpsres.latititude[3] = (jt808_position.latitude) & 0xff;
    jt808_gpsres.longtitude[0] = (jt808_position.longtitude >> 24) & 0xff;
    jt808_gpsres.longtitude[1] = (jt808_position.longtitude >> 16) & 0xff;
    jt808_gpsres.longtitude[2] = (jt808_position.longtitude >> 8) & 0xff;
    jt808_gpsres.longtitude[3] = (jt808_position.longtitude) & 0xff;
    if (isJt808Reday())
    {
        len = jt808TerminalPosition(dest, sysparam.jt808sn, &jt808_position);
        sendDataToServer(NORMAL_LINK,dest, len);
    }
    else if (gpsinfo->fixstatus)
    {
        gpsRestoreWriteData(&jt808_gpsres);
    }
}


static void jt808TerminalAttribute(uint8_t *sn, uint8_t *manufacturerID, uint8_t *terminalType, uint8_t *terminalID,
                                   uint8_t *hexiccid)
{
    uint8_t dest[128], i;
    uint16_t len;
    len = jt808PackMessageHead(dest, TERMINAL_ATTRIBUTE_MSGID, sn, jt808GetSerial(), 0);
    //终端类型
    dest[len++] = 0x05;
    dest[len++] = 0x00;
    //制造商ID
    for (i = 0; i < 5; i++)
    {
        dest[len++] = manufacturerID[i];
    }
    //终端型号
    for (i = 0; i < 20; i++)
    {
        dest[len++] = terminalType[i];
    }
    //终端ID
    for (i = 0; i < 7; i++)
    {
        dest[len++] = terminalID[i];
    }
    //ICCID
    jt808ICCIDChangeHexToBcd(dest + len, hexiccid);
    len += 10;
    //终端硬件版本长度
    dest[len++] = 2;
    //硬件版本号
    dest[len++] = 0x30;
    dest[len++] = 0x31;
    //终端固件版本长度
    dest[len++] = 2;
    //固件版本号
    dest[len++] = 0x32;
    dest[len++] = 0x33;
    //GNSS模块属性
    dest[len++] = 0x0f;
    //通信模块属性
    dest[len++] = 0xff;

    len = jt808PackMessageTail(dest, len);
    len = jt808Escape(dest, len);
    sendDataToServer(NORMAL_LINK, dest, len);
}

static void jt808GenericRespon(uint8_t *sn, JT808_TERMINAL_GENERIC_RESPON *param)
{
    uint8_t dest[128];
    uint16_t len;
    len = jt808PackMessageHead(dest, TERMINAL_GENERIC_RESPON_MSGID, sn, jt808GetSerial(), 0);
    dest[len++] = (param->platform_serial >> 8) & 0xff;
    dest[len++] = param->platform_serial & 0xff;
    dest[len++] = (param->platform_id >> 8) & 0xff;
    dest[len++] = param->platform_id & 0xff;
    dest[len++] = param->result;
    len = jt808PackMessageTail(dest, len);
    len = jt808Escape(dest, len);
    sendDataToServer(NORMAL_LINK, dest, len);
}

static uint16_t transmissionF8(uint8_t *dest, uint16_t len, void *param ,JT808_TERMINAL_GENERIC_RESPON * jtgr)
{
    uint8_t *msg;
    uint16_t i, msglen;
    msg = (uint8_t *)param;
    msglen = strlen((char *)msg);
    dest[len++] = TRANSMISSION_TYPE_F8;
    dest[len++] = (jtgr->platform_serial >> 8) & 0xff;
    dest[len++] = jtgr->platform_serial & 0xff;
    for (i = 0; i < msglen; i++)
    {
        dest[len++] = msg[i];
    }
    return len;
}



static void jt808TranmissionData(uint8_t *sn, void *param)
{
    uint8_t dest[400];
    uint16_t len;
    TIRETRAMMISSION *transmission;
    transmission = (TIRETRAMMISSION *)param;
    len = jt808PackMessageHead(dest, TERMINAL_DATAUPLOAD_MSGID, sn, jt808GetSerial(), 0);
    switch (transmission->id)
    {
        case TRANSMISSION_TYPE_F8:
            len = transmissionF8(dest, len, transmission->param,&jt808_terminalRespon);
            break;
    }

    len = jt808PackMessageTail(dest, len);
    len = jt808Escape(dest, len);
    sendDataToServer(NORMAL_LINK, dest, len);
}
void jt808SendToServer(JT808_PROTOCOL protocol, void *param)
{
    switch (protocol)
    {
        case TERMINAL_REGISTER:
            jt808TerminalRegister(sysparam.jt808sn, sysparam.jt808manufacturerID, sysparam.jt808terminalType,
                                  sysparam.jt808terminalID, 0);
            break;
        case TERMINAL_AUTH:
            jt808TerminalAuthentication(sysparam.jt808sn, sysparam.jt808AuthCode, sysparam.jt808AuthLen);
            break;
        case TERMINAL_HEARTBEAT:
            jt808TerminalHeartbeat(sysparam.jt808sn);
            break;
        case TERMINAL_POSITION:
            jt808SendPosition((GPSINFO *)param);
            break;
        case TERMINAL_ATTRIBUTE:
            jt808TerminalAttribute(sysparam.jt808sn, sysparam.jt808manufacturerID, sysparam.jt808terminalType,
                                   sysparam.jt808terminalID, (uint8_t *)getModuleICCID());
            break;
        case TERMINAL_GENERICRESPON:
            jt808GenericRespon(sysparam.jt808sn, (JT808_TERMINAL_GENERIC_RESPON *)param);
            break;
		case TERMINAL_DATAUPLOAD:
            jt808TranmissionData(sysparam.jt808sn, param);
            break;
    }


}

void jt808MessageSend(uint8_t *buf, uint16_t len)
{
    TIRETRAMMISSION transmission;
    transmission.id = TRANSMISSION_TYPE_F8;
    transmission.param = buf;
    jt808SendToServer(TERMINAL_DATAUPLOAD, &transmission);
}

/*******************************************/
static void jt808RegisterResponParser(uint8_t *msg, uint16_t len)
{
    uint8_t  result, j;
    result = msg[2];
    if (result == 0)
    {
        for (j = 0; j < (len - 3); j++)
        {
            sysparam.jt808AuthCode[j] = msg[j + 3];
        }
        sysparam.jt808AuthLen = len - 3;
        sysparam.jt808isRegister = 1;
        paramSaveAll();
    }
}
static void jt808TerminalCtrlParser(uint8_t *msg, uint16_t len)
{
    LogPrintf(DEBUG_ALL, "8105 CmdType:0x%02X\n", msg[0]);
    //    switch (msg[0])
    //    {
    //    case 0x64:
    //        RELAY_ON;
    //        LogMessage(DEBUG_ALL, "Cut off\n");
    //        break;
    //    case 0x65:
    //        RELAY_OFF;
    //        LogMessage(DEBUG_ALL, "Restore\n");
    //        break;
    //    }
    jt808_terminalRespon.result = 0;
    jt808SendToServer(TERMINAL_GENERICRESPON, &jt808_terminalRespon);
}
static void jt808ReceiveF8(uint8_t *msg, uint16_t len)
{
    instructionParser(msg, len, NETWORK_MODE, NULL, NULL);
}

static void jt808DownlinkParser(uint8_t *msg, uint16_t len)
{
    LogPrintf(DEBUG_ALL, "8900 MsgType:0x%02X\n", msg[0]);
    switch (msg[0])
    {
        case TRANSMISSION_TYPE_F8:
            jt808ReceiveF8(msg + 1, len - 1);
            break;
    }
}


static void jt808GenericResponParser(uint8_t *msg, uint16_t len)
{
    uint16_t protocol;
    protocol = msg[2] << 8 | msg[3];
	LogMessage(DEBUG_ALL, "Generic Respon\r\n");
    switch (protocol)
    {
        case TERMINAL_AUTHENTICATION_MSGID:
            jt808_info.authCount = 0;
            LogMessage(DEBUG_ALL, "Authtication respon\n");
            updateSystemLedStatus(SYSTEM_LED_SERVEROK, 1);
            jt808ChangeFsm(JT808_NORMAL);
            break;
    }
}

//7E 8103 000A 784086963657 000B 01 00000001 04 000000B4 7A7E
static void jt808SetTerminalParamParser(uint8_t *msg, uint16_t len)
{
    uint8_t paramCount, paramLen, i;
    uint16_t j;
    uint32_t paramId, value;
    j = 0;
    paramCount = msg[j++];
    for (i = 0; i < paramCount; i++)
    {

        paramId = msg[j++];
        paramId = (paramId << 8) | msg[j++];
        paramId = (paramId << 8) | msg[j++];
        paramId = (paramId << 8) | msg[j++];
        paramLen = msg[j++];


        LogPrintf(DEBUG_ALL, "ParamId 0x%04X,ParamLen=%d\n", paramId, paramLen);
        switch (paramId)
        {
            //设置心跳间隔
            case 0x0001:
                value = msg[j++];
                value = (value << 8) | msg[j++];
                value = (value << 8) | msg[j++];
                value = (value << 8) | msg[j++];
                sysparam.heartbeatgap = value;
                paramSaveAll();
                LogPrintf(DEBUG_ALL, "Update heartbeat to %d\n", sysparam.heartbeatgap);
                j += paramLen;
                break;
            default:
                j += paramLen;
                break;
        }
    }
    jt808_terminalRespon.result = 0;
    jt808SendToServer(TERMINAL_GENERICRESPON, &jt808_terminalRespon);
}

static void jt808getTextMessageParser(uint8_t *msg, uint16_t len)
{
    uint8_t content[128];
    LogPrintf(DEBUG_ALL, "Msg type %d\r\n", msg[0]);
    strncpy((char *)content, (char *)(msg + 1), len - 1);
    content[len - 1] = 0;
    LogPrintf(DEBUG_ALL, "Get Msg:%s\n", content);
    instructionParser(content, len - 1, AT_SMS_MODE, NULL, NULL);
    jt808_terminalRespon.result = 0;
    jt808SendToServer(TERMINAL_GENERICRESPON, &jt808_terminalRespon);
}

/*
protocol :协议号
msg		 :消息体内容
len		 :消息体长度
*/
static void jt808ProtocolParser(uint16_t protocol, uint8_t *msg, uint16_t len)
{
    switch (protocol)
    {
        case PLATFORM_GENERIC_RESPON_MSGID:
            jt808GenericResponParser(msg, len);
            break;
        case PLATFORM_REGISTER_RESPON_MSGID:
            jt808RegisterResponParser(msg, len);
            break;
        case PLATFORM_TERMINAL_CTL_MSGID:
            jt808TerminalCtrlParser(msg, len);
            break;
        case PLATFORM_DOWNLINK_MSGID:
            jt808DownlinkParser(msg, len);
            break;
        case PLATFORM_TERMINAL_SET_PARAM_MSGID:
            jt808SetTerminalParamParser(msg, len);
            break;
        case PLATFORM_QUERY_ATTR_MSGID:
            jt808SendToServer(TERMINAL_ATTRIBUTE, NULL);
            break;
        case PLATFORM_TEXT_MSGID:
            jt808getTextMessageParser(msg, len);
            break;
    }
}
void jt808ReceiveParser(uint8_t *src, uint16_t len)
{
    uint16_t i, j, k;
    uint16_t msglen, crc = 0, protocol;
    len = jt808Unescape(src, len);
    for (i = 0; i < len; i++)
    {
        if (i + 15 > len)
        {
            //协议最短15个字节
            return ;
        }
        if (src[i] == 0x7E)
        {
            msglen = src[i + 3] << 8 | src[i + 4];
            msglen &= 0x3FF;
            k = i + 13 + msglen;

            if ((k + 1) > len)
            {
                return ;
            }
            if (src[k + 1] != 0x7E)
            {
                LogPrintf(DEBUG_ALL, "Tail not 7E,it was 0x%02X,k=%d\r\n", src[k + 1], k);
                continue ;
            }
            for (j = 1; j < k; j++)
            {
                crc ^= src[i + j];
            }
            if (crc == src[k])
            {
                protocol = src[i + 1] << 8 | src[i + 2];
                jt808_terminalRespon.platform_id = protocol;
                jt808_terminalRespon.platform_serial = src[i + 11] << 8 | src[i + 12];
                jt808ProtocolParser(protocol, src + i + 13, msglen);
                i = k + 1;
            }
        }
    }
}


/**********************************************/

void jt808NetReset(void)
{
    memset(&jt808_info, 0, sizeof(JT808_NETWORK_STRUCT));
}

static void jt808ChangeFsm(JT808_FSM_STATE fsm)
{
    jt808_info.fsmstate = fsm;
}

uint8_t isJt808Reday(void)
{
    if (isNetProcessNormal() && jt808_info.fsmstate == JT808_NORMAL)
        return 1;
    return 0;
}


void jt808ProtocolRunFsm(void)
{
    static uint8_t ret = 1;
    switch (jt808_info.fsmstate)
    {
        case JT808_REGISTER:

            if (sysparam.jt808isRegister)
            {
                //已注册过的设备不用重复注册
                jt808ChangeFsm(JT808_AUTHENTICATION);
                jt808_info.registertick = 0;
                jt808_info.authtick = 0;
                jt808_info.registerCount = 0;
            }
            else
            {
                //注册设备
                if (jt808_info.registertick++ % 60 == 0)
                {
                    LogMessage(DEBUG_ALL, "Terminal register\n");
                    jt808SendToServer(TERMINAL_REGISTER, NULL);
                    if (jt808_info.registerCount++ > 3)
                    {
                        jt808_info.registerCount = 0;
                        reConnectServer();
                    }
                }
            }
            break;
        case JT808_AUTHENTICATION:

            if (jt808_info.authtick++ % 60 == 0)
            {
                jt808_info.heartbeattick = sysparam.heartbeatgap;
                LogMessage(DEBUG_ALL, "Terminal authentication\n");
                jt808SendToServer(TERMINAL_AUTH, NULL);
                ret = 1;
                if (jt808_info.authCount++ > 3)
                {
                    jt808_info.authCount = 0;
                    sysparam.jt808isRegister = 0;
                    paramSaveAll();
                    reConnectServer();
                }
            }
            break;
        case JT808_NORMAL:
            if (jt808_info.heartbeattick++ >= sysparam.heartbeatgap)
            {
                jt808_info.heartbeattick = 0;
                LogMessage(DEBUG_ALL, "Terminal heartbeat\n");
                jt808SendToServer(TERMINAL_HEARTBEAT, NULL);
            }
            if (jt808_info.heartbeattick % 3 == 0)
            {
                if (ret == 1)
                {
                    ret = gpsRestoreReadData();
                }
            }
            break;
        default:
            jt808_info.fsmstate = JT808_REGISTER;
            break;
    }
}



