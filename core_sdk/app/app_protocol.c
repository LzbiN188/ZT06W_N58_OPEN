#include "app_protocol.h"
#include "app_sys.h"
#include "app_gps.h"
#include "app_param.h"
#include "app_net.h"
#include "app_task.h"
#include "app_instructioncmd.h"
#include <math.h>
#include "nwy_fota_api.h"
#include "nwy_fota.h"
#include "nwy_file.h"
#include "app_ble.h"
#include "app_protocol_808.h"
#include "app_port.h"
#include "nwy_audio_api.h"
#include "app_customercmd.h"
#include "app_socket.h"
#include "app_mcu.h"
#include "app_kernal.h"
//联网状态变量
static NetWorkConnectStruct netconnect;
static uint8_t instructionid[4];
static uint8_t instructionid123[4];
static uint16_t instructionserier = 0;
static GPSRestoreStruct gpsres;
static UndateInfoStruct uis;
static AudioFileStruct audiofile;

static int createProtocolHead(char *dest, unsigned char Protocol_type)
{
    int pdu_len;
    pdu_len = 0;
    if (dest == NULL)
    {
        return 0;
    }
    dest[pdu_len++] = 0x78;
    dest[pdu_len++] = 0x78;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = Protocol_type;
    return pdu_len;
}
static int createProtocolTail_78(char *dest, int h_b_len, int serial_no)
{
    int pdu_len;
    unsigned short crc_16;

    if (dest == NULL)
    {
        return 0;
    }
    pdu_len = h_b_len;
    dest[pdu_len++] = (serial_no >> 8) & 0xff;
    dest[pdu_len++] = serial_no & 0xff;
    dest[2] = pdu_len - 1;
    crc_16 = GetCrc16(dest + 2, pdu_len - 2);
    dest[pdu_len++] = (crc_16 >> 8) & 0xff;
    dest[pdu_len++] = (crc_16 & 0xff);
    dest[pdu_len++] = 0x0d;
    dest[pdu_len++] = 0x0a;

    return pdu_len;
}

static int createProtocolTail_79(char *dest, int h_b_len, int serial_no)
{
    int pdu_len;
    unsigned short crc_16;

    if (dest == NULL)
    {
        return -1;
    }
    pdu_len = h_b_len;
    dest[pdu_len++] = (serial_no >> 8) & 0xff;
    dest[pdu_len++] = serial_no & 0xff;
    dest[2] = ((pdu_len - 2) >> 8) & 0xff;
    dest[3] = ((pdu_len - 2) & 0xff);
    crc_16 = GetCrc16(dest + 2, pdu_len - 2);
    dest[pdu_len++] = (crc_16 >> 8) & 0xff;
    dest[pdu_len++] = (crc_16 & 0xff);
    dest[pdu_len++] = 0x0d;
    dest[pdu_len++] = 0x0a;
    return pdu_len;
}

/*生成序列号
*/
static uint16_t createProtocolSerial(void)
{
    return netconnect.serial++;
}

/*打包IMEI
IMEI:设备sn号
dest：数据存放区
*/
static int packIMEI(char *IMEI, char *dest)
{
    int imei_len;
    int i;
    if (IMEI == NULL || dest == NULL)
    {
        return -1;
    }
    imei_len = strlen(IMEI);
    if (imei_len == 0)
    {
        return -2;
    }
    if (imei_len % 2 == 0)
    {
        for (i = 0; i < imei_len / 2; i++)
        {
            dest[i] = ((IMEI[i * 2] - '0') << 4) | (IMEI[i * 2 + 1] - '0');
        }
    }
    else
    {
        for (i = 0; i < imei_len / 2; i++)
        {
            dest[i + 1] = ((IMEI[i * 2 + 1] - '0') << 4) | (IMEI[i * 2 + 2] - '0');
        }
        dest[0] = (IMEI[0] - '0');
    }
    return (imei_len + 1) / 2;
}



static int protocolGPSpack(GPSINFO *gpsinfo, char *dest, int protocol, GPSRestoreStruct *gpsres)
{
    int pdu_len;
    unsigned long la;
    unsigned long lo;
    double f_la, f_lo;
    unsigned char speed, gps_viewstar, beidou_viewstar;
    int direc;

    DATETIME datetimenew;
    pdu_len = 0;
    la = lo = 0;
    /* UTC日期，DDMMYY格式 */
    datetimenew = changeUTCTimeToLocalTime(gpsinfo->datetime, sysparam.utc);

    dest[pdu_len++] = datetimenew.year % 100 ;
    dest[pdu_len++] = datetimenew.month;
    dest[pdu_len++] = datetimenew.day;
    dest[pdu_len++] = datetimenew.hour;
    dest[pdu_len++] = datetimenew.minute;
    dest[pdu_len++] = datetimenew.second;

    gps_viewstar = 0;//(unsigned char)gpsinfo->gpsviewstart;
    beidou_viewstar = (unsigned char)gpsinfo->used_star;
    if (gps_viewstar > 15)
    {
        gps_viewstar = 15;
    }
    if (beidou_viewstar > 15)
    {
        beidou_viewstar = 15;
    }
    if (protocol == PROTOCOL_12)
    {
        dest[pdu_len++] = (beidou_viewstar & 0x0f) << 4 | (gps_viewstar & 0x0f);
    }
    else
    {
        /* 前 4Bit为 GPS信息长度，后 4Bit为参与定位的卫星数 */
        if (gpsinfo->used_star   > 0)
        {
            dest[pdu_len++] = (12 << 4) | (gpsinfo->used_star & 0x0f);
        }
        else
        {
            /* 前 4Bit为 GPS信息长度，后 4Bit为参与定位的卫星数 */
            dest[pdu_len++] = (12 << 4) | (gpsinfo->gpsviewstart & 0x0f);
        }
    }
    /*
    纬度: 占用4个字节，表示定位数据的纬度值。数值范围0至162000000，表示0度到90度的范围，单位：1/500秒，转换方法如下：
    把GPS模块输出的经纬度值转化成以分为单位的小数；然后再把转化后的小数乘以30000，把相乘的结果转换成16进制数即可。
    如 ， ，然后转换成十六进制数为0x02 0x6B 0x3F 0x3E。
     */
    f_la  = gpsinfo->latitude * 60 * 30000;
    if (f_la < 0)
    {
        f_la = f_la * (-1);
    }
    la = (unsigned long)f_la;

    dest[pdu_len++] = (la >> 24) & 0xff;
    dest[pdu_len++] = (la >> 16) & 0xff;
    dest[pdu_len++] = (la >> 8) & 0xff;
    dest[pdu_len++] = (la) & 0xff;
    /*
    经度:占用4个字节，表示定位数据的经度值。数值范围0至324000000，表示0度到180度的范围，单位：1/500秒，转换方法
    和纬度的转换方法一致。
     */

    f_lo  = gpsinfo->longtitude * 60 * 30000;
    if (f_lo < 0)
    {
        f_lo = f_lo * (-1);
    }
    lo = (unsigned long)f_lo;
    dest[pdu_len++] = (lo >> 24) & 0xff;
    dest[pdu_len++] = (lo >> 16) & 0xff;
    dest[pdu_len++] = (lo >> 8) & 0xff;
    dest[pdu_len++] = (lo) & 0xff;


    /*
    速度:占用1个字节，表示GPS的运行速度，值范围为0x00～0xFF表示范围0～255公里/小时。
     */
    speed = (unsigned char)(gpsinfo->speed);
    dest[pdu_len++] = speed;
    /*
    航向:占用2个字节，表示GPS的运行方向，表示范围0～360，单位：度，以正北为0度，顺时针。
     */
    if (gpsres != NULL)
    {

        gpsres->year = datetimenew.year % 100 ;
        gpsres->month = datetimenew.month;
        gpsres->day = datetimenew.day;
        gpsres->hour = datetimenew.hour;
        gpsres->minute = datetimenew.minute;
        gpsres->second = datetimenew.second;

        gpsres->latititude[0] = (la >> 24) & 0xff;
        gpsres->latititude[1] = (la >> 16) & 0xff;
        gpsres->latititude[2] = (la >> 8) & 0xff;
        gpsres->latititude[3] = (la) & 0xff;


        gpsres->longtitude[0] = (lo >> 24) & 0xff;
        gpsres->longtitude[1] = (lo >> 16) & 0xff;
        gpsres->longtitude[2] = (lo >> 8) & 0xff;
        gpsres->longtitude[3] = (lo) & 0xff;

        gpsres->speed = speed;
    }

    direc = (int)gpsinfo->course;
    dest[pdu_len] = (direc >> 8) & 0x03;
    //GPS FIXED:
    dest[pdu_len] |= 0x10 ; //0000 1000
    /*0：南纬 1：北纬 */
    if (gpsinfo->NS == 'N')
    {
        dest[pdu_len] |= 0x04 ; //0000 0100
    }
    /*0：东经 1：西经*/
    if (gpsinfo->EW == 'W')
    {
        dest[pdu_len] |= 0x08 ; //0000 1000
    }
    if (gpsres != NULL)
    {
        gpsres->coordinate[0] = dest[pdu_len];
    }
    pdu_len++;
    dest[pdu_len] = (direc) & 0xff;
    if (gpsres != NULL)
    {
        gpsres->coordinate[1] = dest[pdu_len];
    }
    pdu_len++;
    return pdu_len;
}

static int protocolLBSPack(char *dest, GPSINFO *gpsinfo)
{
    int pdu_len;
    uint32_t hightvalue;
    uint8_t fixAccuracy;
    if (dest == NULL)
    {
        return -1;
    }
    pdu_len = 0;

    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;

    hightvalue = fabs(gpsinfo->hight * 10);
    hightvalue &= ~(0xF00000);
    if (gpsinfo->hight < 0)
    {
        hightvalue |= 0x100000;
    }
    fixAccuracy = gpsinfo->fixAccuracy;
    hightvalue |= (fixAccuracy & 0x07) << 21;
    dest[pdu_len++] = (hightvalue >> 16) & 0xff;
    dest[pdu_len++] = (hightvalue >> 8) & 0xff;
    dest[pdu_len++] = (hightvalue) & 0xff;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    return pdu_len;
}

/*生成登录协议01*/
static int createProtocol01(char *IMEI, unsigned short Serial, char *dest)
{
    int pdu_len;
    int ret;
    pdu_len = createProtocolHead(dest, 0x01);
    ret = packIMEI(IMEI, dest + pdu_len);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len += ret;
    ret = createProtocolTail_78(dest, pdu_len,  Serial);
    if (ret < 0)
    {
        return -2;
    }
    pdu_len = ret;
    return pdu_len;
}

/*生成定位协议12*/
static int createProtocol12(GPSINFO *gpsinfo,  unsigned short Serial, char *dest)
{
    int pdu_len;
    int ret;
    unsigned char gsm_level_value;

    if (gpsinfo == NULL)
        return -1;
    pdu_len = createProtocolHead(dest, 0x12);
    /* Pack GPS */
    ret = protocolGPSpack(gpsinfo,  dest + pdu_len, PROTOCOL_12, &gpsres);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len += ret;
    /* Pack LBS */
    ret = protocolLBSPack(dest + pdu_len, gpsinfo);
    if (ret < 0)
    {
        return -2;
    }
    pdu_len += ret;
    /* Add Language Reserved */
    gsm_level_value = getModuleRssi();
    gsm_level_value |= 0x80;
    dest[pdu_len++] = gsm_level_value;
    dest[pdu_len++] = 0;
    /* Pack Tail */
    ret = createProtocolTail_78(dest, pdu_len,  Serial);
    if (ret <=  0)
    {
        return -3;
    }
    gpsinfo->hadupload = 1;
    pdu_len = ret;
    return pdu_len;
}
/*生成定位协议12--位置补报*/
void gpsRestoreDataSend(GPSRestoreStruct *grs, char *dest	, uint16_t *len)
{
    char debug[200];
    int pdu_len;
    pdu_len = createProtocolHead(dest, 0x12);
    //时间戳
    dest[pdu_len++] = grs->year ;
    dest[pdu_len++] = grs->month;
    dest[pdu_len++] = grs->day;
    dest[pdu_len++] = grs->hour;
    dest[pdu_len++] = grs->minute;
    dest[pdu_len++] = grs->second;
    //数量
    dest[pdu_len++] = 0;
    //维度
    dest[pdu_len++] = grs->latititude[0];
    dest[pdu_len++] = grs->latititude[1];
    dest[pdu_len++] = grs->latititude[2];
    dest[pdu_len++] = grs->latititude[3];
    //经度
    dest[pdu_len++] = grs->longtitude[0];
    dest[pdu_len++] = grs->longtitude[1];
    dest[pdu_len++] = grs->longtitude[2];
    dest[pdu_len++] = grs->longtitude[3];
    //速度
    dest[pdu_len++] = grs->speed;
    //定位信息
    dest[pdu_len++] = grs->coordinate[0];
    dest[pdu_len++] = grs->coordinate[1];
    //mmc
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    //mnc
    dest[pdu_len++] = 0;
    //lac
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    //cellid
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    //signal
    dest[pdu_len++] = 0;
    //语言位
    dest[pdu_len++] = 1;
    pdu_len = createProtocolTail_78(dest, pdu_len,	createProtocolSerial());
    *len = pdu_len;
    changeByteArrayToHexString((uint8_t *)dest, (uint8_t *)debug, pdu_len);
    debug[pdu_len * 2] = 0;
    LogMessage(DEBUG_ALL, "TCP Send:");
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "\n");
}

uint8_t getBatteryLevel(void)
{
    uint8_t level = 0;
    if (sysinfo.outsidevoltage >= sysparam.volTable6)
    {
        level = 100;
    }
    else if (sysinfo.outsidevoltage >= sysparam.volTable5 && sysinfo.outsidevoltage < sysparam.volTable6)
    {
        level = 90;
    }
    else if (sysinfo.outsidevoltage >= sysparam.volTable4 && sysinfo.outsidevoltage < sysparam.volTable5)
    {
        level = 70;
    }
    else if (sysinfo.outsidevoltage >= sysparam.volTable3 && sysinfo.outsidevoltage < sysparam.volTable4)
    {
        level = 50;
    }
    else if (sysinfo.outsidevoltage >= sysparam.volTable2 && sysinfo.outsidevoltage < sysparam.volTable3)
    {
        level = 30;
    }
    else if (sysinfo.outsidevoltage >= sysparam.volTable1 && sysinfo.outsidevoltage < sysparam.volTable2)
    {
        level = 10;
    }
    else if (sysinfo.outsidevoltage < sysparam.volTable1)
    {
        level = 0;
    }
    return level;
}
/*生成心跳协议*/
static int createProtocol13(uint8_t link, unsigned short Serial, char *dest)
{
    int pdu_len;
    int ret;
    uint16_t value;
    GPSINFO *gpsinfo;
    uint8_t gpsvewstar, beidouviewstar;
    BLEDEVINFO *bleinfo;
    bleinfo = getBleDevInfo();

    pdu_len = createProtocolHead(dest, 0x13);
    dest[pdu_len++] = sysinfo.terminalStatus;
    gpsinfo = getCurrentGPSInfo();

    value  = 0;
    gpsvewstar = 0;
    beidouviewstar = gpsinfo->used_star;
    if (sysinfo.GPSStatus == 0)
    {
        gpsvewstar = 0;
        beidouviewstar = 0;
    }
    value |= ((beidouviewstar & 0x1F) << 10);
    value |= ((gpsvewstar & 0x1F) << 5);
    value |= ((getModuleRssi() & 0x1F));
    value |= 0x8000;

    dest[pdu_len++] = (value >> 8) & 0xff;
    dest[pdu_len++] = value & 0xff;
    dest[pdu_len++ ] = 0;
    dest[pdu_len++ ] = 0;

    if (link == NORMAL_LINK)
    {
        dest[pdu_len++ ] = getBatteryLevel();//电量
    }
    else
    {
        dest[pdu_len++ ] = bleinfo->batterylevel;
    }

    dest[pdu_len++ ] = 0;

    if (link == NORMAL_LINK)
    {
        value = (uint16_t)(sysinfo.outsidevoltage * 100);
    }
    else
    {
        value = (uint16_t)(bleinfo->voltage * 100);
    }


    dest[pdu_len++ ] = (value >> 8) & 0xff; //电压
    dest[pdu_len++ ] = value & 0xff;
    dest[pdu_len++ ] = 0;//感光

    if (link == NORMAL_LINK)
    {
        dest[pdu_len++ ] = (sysparam.mode1startuptime >> 8) & 0xff; //模式一次数
        dest[pdu_len++ ] = sysparam.mode1startuptime & 0xff;
        dest[pdu_len++ ] = (sysparam.mode2worktime >> 8) & 0xff; //模式二次数
        dest[pdu_len++ ] = sysparam.mode2worktime & 0xff;
    }
    else
    {
        dest[pdu_len++ ] = (bleinfo->modecount >> 8) & 0xff; //模式一次数
        dest[pdu_len++ ] = bleinfo->modecount & 0xff;
        dest[pdu_len++ ] = 0;
        dest[pdu_len++ ] = 0;
    }

    ret = createProtocolTail_78(dest, pdu_len,  Serial);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len = ret;
    return pdu_len;

}
/*指令透传协议*/
static int createProtocol21(char *dest, char *data, uint16_t datalen)
{
    int pdu_len = 0, i;
    dest[pdu_len++] = 0x79; //协议头
    dest[pdu_len++] = 0x79;
    dest[pdu_len++] = 0x00; //指令长度待定
    dest[pdu_len++] = 0x00;
    dest[pdu_len++] = 0x21; //协议号
    dest[pdu_len++] = instructionid[0]; //指令ID
    dest[pdu_len++] = instructionid[1];
    dest[pdu_len++] = instructionid[2];
    dest[pdu_len++] = instructionid[3];
    dest[pdu_len++] = 1; //内容编码
    for (i = 0; i < datalen; i++) //返回内容
    {
        dest[pdu_len++] = data[i];
    }
    pdu_len = createProtocolTail_79(dest, pdu_len, instructionserier);
    return pdu_len;
}

/*WIFI上报协议*/
static uint8_t createProtocolF3(uint8_t      link, char *dest, WIFI_INFO *wap)
{
    uint8_t i, j;

    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    uint16_t pdu_len;
    pdu_len = createProtocolHead(dest, 0xF3);
    dest[pdu_len++] = year % 100;
    dest[pdu_len++] = month;
    dest[pdu_len++] = date;
    dest[pdu_len++] = hour;
    dest[pdu_len++] = minute;
    dest[pdu_len++] = second;
    dest[pdu_len++] = wap->apcount;
    for (i = 0; i < wap->apcount; i++)
    {
        dest[pdu_len++] = 0;
        dest[pdu_len++] = 0;
        for (j = 0; j < 6; j++)
        {
            dest[pdu_len++] = wap->ap[i].ssid[j];
        }
    }
    pdu_len = createProtocolTail_78(dest, pdu_len,  createProtocolSerial());
    return pdu_len;
}

/*模组信息回传协议*/
static int createProtocolF1(unsigned short Serial, char *dest)
{
    int pdu_len;
    pdu_len = createProtocolHead(dest, 0xF1);
    sprintf(dest + pdu_len, "%s&&%s&&%s&&%s", sysparam.SN, getModuleIMSI(), getModuleICCID(), getModuleMAC());
    pdu_len += strlen(dest + pdu_len);
    pdu_len = createProtocolTail_78(dest, pdu_len,  createProtocolSerial());
    return pdu_len;
}
/*报警上送协议*/
static int createProtocol16(unsigned short Serial, char *dest, uint8_t event)
{
    int pdu_len, ret, i;
    GPSINFO *gpsinfo;
    pdu_len = createProtocolHead(dest, 0x16);
    gpsinfo = getLastFixedGPSInfo();
    ret = protocolGPSpack(gpsinfo, dest + pdu_len, PROTOCOL_16, NULL);
    pdu_len += ret;
    for (i = 0; i < 46; i++)
        dest[pdu_len++] = 0;
    dest[pdu_len++] = sysinfo.terminalStatus;
    sysinfo.terminalStatus &= ~0x38;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = event;
    if (event == 0)
    {
        dest[pdu_len++] = 0x01;
    }
    else
    {
        dest[pdu_len++] = 0x81;
    }
    pdu_len = createProtocolTail_78(dest, pdu_len,  Serial);
    return pdu_len;

}

/*基站定位*/
static int createProtocol19(unsigned short Serial, char *dest)
{
    int pdu_len;
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    pdu_len = createProtocolHead(dest, 0x19);
    dest[pdu_len++] = year % 100;
    dest[pdu_len++] = month;
    dest[pdu_len++] = date;
    dest[pdu_len++] = hour;
    dest[pdu_len++] = minute;
    dest[pdu_len++] = second;
    dest[pdu_len++] = sysinfo.mcc >> 8;
    dest[pdu_len++] = sysinfo.mcc;
    dest[pdu_len++] = sysinfo.mnc;
    dest[pdu_len++] = 1;
    dest[pdu_len++] = sysinfo.lac >> 8;
    dest[pdu_len++] = sysinfo.lac;
    dest[pdu_len++] = sysinfo.cid >> 24;
    dest[pdu_len++] = sysinfo.cid >> 16;
    dest[pdu_len++] = sysinfo.cid >> 8;
    dest[pdu_len++] = sysinfo.cid;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    pdu_len = createProtocolTail_78(dest, pdu_len,  Serial);
    return pdu_len;
}
/*获取服务器时间协议*/
static int createProtocol_8A(unsigned short Serial, char *dest)
{
    int pdu_len;
    int ret;
    pdu_len = createProtocolHead(dest, 0x8A);
    ret = createProtocolTail_78(dest, pdu_len,  Serial);
    pdu_len = ret;
    return pdu_len;
}




static int createProtocol51(char *dest)
{
    int pdu_len;
    int ret;
    int Serial;
    pdu_len = createProtocolHead(dest, 0x51);
    dest[pdu_len++] = (audiofile.audioId >> 24) & 0xFF;
    dest[pdu_len++] = (audiofile.audioId >> 16) & 0xFF;
    Serial = audiofile.audioId & 0xFFFF;
    ret = createProtocolTail_78(dest, pdu_len,  Serial);
    pdu_len = ret;
    return pdu_len;
}

/*录音文件信息上送协议*/
void createProtocol61(char *dest, char *datetime, uint32_t totalsize, uint8_t filetye, uint16_t packsize)
{
    uint16_t pdu_len;
    char debug[200];
    uint16_t packnum;
    pdu_len = createProtocolHead(dest, 0x61);
    changeHexStringToByteArray_10in((uint8_t *)dest + 4, (uint8_t *)datetime, 6);
    pdu_len += 6;
    dest[pdu_len++] = filetye;
    packnum = totalsize / packsize;
    if (totalsize % packsize != 0)
    {
        packnum += 1;
    }
    dest[pdu_len++] = (packnum >> 8) & 0xff;
    dest[pdu_len++] = packnum & 0xff;
    dest[pdu_len++] = (totalsize >> 24) & 0xff;
    dest[pdu_len++] = (totalsize >> 16) & 0xff;
    dest[pdu_len++] = (totalsize >> 8) & 0xff;
    dest[pdu_len++] = totalsize & 0xff;
    pdu_len = createProtocolTail_78(dest, pdu_len,  createProtocolSerial());
    changeByteArrayToHexString((uint8_t *)dest, (uint8_t *)debug, pdu_len);
    debug[pdu_len * 2] = 0;
    LogMessage(DEBUG_ALL, "TCP Send:");
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "\n");
    sendDataToServer(NORMAL_LINK, (uint8_t *)dest, pdu_len);
}
/*录音文件内容上送协议*/
void createProtocol62(char *dest, char *datetime, uint16_t packnum, uint8_t *recdata, uint16_t reclen)
{
    char debug[50];
    uint16_t pdu_len, i;
    pdu_len = 0;
    dest[pdu_len++] = 0x79; //协议头
    dest[pdu_len++] = 0x79;
    dest[pdu_len++] = 0x00; //指令长度待定
    dest[pdu_len++] = 0x00;
    dest[pdu_len++] = 0x62; //协议号
    changeHexStringToByteArray_10in((uint8_t *)dest + 5, (uint8_t *)datetime, 6);
    pdu_len += 6;
    dest[pdu_len++] = (packnum >> 8) & 0xff;
    dest[pdu_len++] = packnum & 0xff;
    for (i = 0; i < reclen; i++)
    {
        dest[pdu_len++] = recdata[i];
    }
    pdu_len = createProtocolTail_79(dest, pdu_len,  createProtocolSerial());
    changeByteArrayToHexString((uint8_t *)dest, (uint8_t *)debug, 15);
    debug[30] = 0;
    LogMessage(DEBUG_ALL, "TCP Send:");
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "\n");
    sendDataToServer(NORMAL_LINK, (uint8_t *)dest, pdu_len);
}

static int createUpdateProtocol(char *IMEI, unsigned short Serial, char *dest, uint8_t cmd)
{
    int pdu_len = 0;
    uint32_t readfilelen;
    //head
    dest[pdu_len++] = 0x79;
    dest[pdu_len++] = 0x79;
    //len
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    //protocol
    dest[pdu_len++] = 0xF3;
    //命令
    dest[pdu_len++] = cmd;
    if (cmd == 0x01)
    {
        //SN号长度
        dest[pdu_len++] = strlen(IMEI);
        //拷贝SN号
        memcpy(dest + pdu_len, IMEI, strlen(IMEI));
        pdu_len += strlen(IMEI);
        //版本号长度
        dest[pdu_len++] = strlen(uis.curCODEVERSION);
        //拷贝SN号
        memcpy(dest + pdu_len, uis.curCODEVERSION, strlen(uis.curCODEVERSION));
        pdu_len += strlen(uis.curCODEVERSION);
    }
    else if (cmd == 0x02)
    {
        dest[pdu_len++] = (uis.file_id >> 24) & 0xFF;
        dest[pdu_len++] = (uis.file_id >> 16) & 0xFF;
        dest[pdu_len++] = (uis.file_id >> 8) & 0xFF;
        dest[pdu_len++] = (uis.file_id) & 0xFF;

        //文件偏移位置
        dest[pdu_len++] = (uis.rxfileOffset >> 24) & 0xFF;
        dest[pdu_len++] = (uis.rxfileOffset >> 16) & 0xFF;
        dest[pdu_len++] = (uis.rxfileOffset >> 8) & 0xFF;
        dest[pdu_len++] = (uis.rxfileOffset) & 0xFF;

        readfilelen = uis.file_totalsize - uis.rxfileOffset; //得到剩余未接收大小
        if (readfilelen > uis.file_len)
        {
            readfilelen = uis.file_len;
        }
        //文件读取长度
        dest[pdu_len++] = (readfilelen >> 24) & 0xFF;
        dest[pdu_len++] = (readfilelen >> 16) & 0xFF;
        dest[pdu_len++] = (readfilelen >> 8) & 0xFF;
        dest[pdu_len++] = (readfilelen) & 0xFF;
    }
    else if (cmd == 0x03)
    {
        dest[pdu_len++] = uis.updateOK;
        //SN号长度
        dest[pdu_len++] = strlen(IMEI);
        //拷贝SN号
        memcpy(dest + pdu_len, IMEI, strlen(IMEI));
        pdu_len += strlen(IMEI);

        dest[pdu_len++] = (uis.file_id >> 24) & 0xFF;
        dest[pdu_len++] = (uis.file_id >> 16) & 0xFF;
        dest[pdu_len++] = (uis.file_id >> 8) & 0xFF;
        dest[pdu_len++] = (uis.file_id) & 0xFF;

        //版本号长度
        dest[pdu_len++] = strlen(uis.newCODEVERSION);
        //拷贝SN号
        memcpy(dest + pdu_len, uis.newCODEVERSION, strlen(uis.newCODEVERSION));
        pdu_len += strlen(uis.newCODEVERSION);
    }
    else
    {
        return 0;
    }
    pdu_len = createProtocolTail_79(dest, pdu_len, Serial);
    return pdu_len;
}

/*gps 数据存储*/
void gpsRestoreWriteData(GPSRestoreStruct *gpsres)
{
    uint16_t paramsize, fileOperation;
    int fd, writelen;
    paramsize = sizeof(GPSRestoreStruct);
    if (nwy_sdk_fexist(GPS_RESTORE_FILE_NAME) == true)
    {
        fileOperation = NWY_WRONLY | NWY_APPEND;
    }
    else
    {
        fileOperation = NWY_CREAT | NWY_RDWR;
    }
    fd = nwy_sdk_fopen(GPS_RESTORE_FILE_NAME, fileOperation);
    if (fd < 0)
    {
        LogMessage(DEBUG_ALL, "gpsSave==>Open error\r\n");
        return;
    }
    writelen = nwy_sdk_fwrite(fd, gpsres, paramsize);
    if (writelen != paramsize)
    {
        LogMessage(DEBUG_ALL, "gpsSave==>Error\r\n");
    }
    else
    {
        LogMessage(DEBUG_ALL, "gpsSave==>Success\r\n");
    }
    nwy_sdk_fclose(fd);
}



uint8_t gpsRestoreReadData(void)
{
    static uint32_t readOffset = 0;
    int fd, readlen;
    uint8_t gpsget[400];
    char  dest[780];
    uint8_t gpscount, i;
    uint16_t  fileOperation, destlen, protocollen;
    uint32_t  fileSize;
    GPSRestoreStruct *gpsinfo;

    if (nwy_sdk_fexist(GPS_RESTORE_FILE_NAME) == true)
    {
        fileOperation = NWY_RDONLY;
    }
    else
    {
        readOffset = 0;
        LogMessage(DEBUG_ALL, "No gps file\r\n");
        return 0;
    }
    fd = nwy_sdk_fopen(GPS_RESTORE_FILE_NAME, fileOperation);
    if (fd < 0)
    {
        LogMessage(DEBUG_ALL, "gpsGet==>Open error\r\n");
        return 1;
    }
    fileSize = nwy_sdk_fsize_fd(fd);
    LogPrintf(DEBUG_ALL, "Total:%d,Offset:%d\r\n", fileSize, readOffset);
    nwy_sdk_fseek(fd, readOffset, NWY_SEEK_SET);
    readlen = nwy_sdk_fread(fd, gpsget, 400);
    nwy_sdk_fclose(fd);
    if (readlen > 0)
    {
        readOffset += 400;
        gpscount = readlen / 20;
        destlen = 0;
        for (i = 0; i < gpscount; i++)
        {
            gpsinfo = (GPSRestoreStruct *)(gpsget + (20 * i));
            if (sysparam.protocol == USE_JT808_PROTOCOL)
            {
                protocollen = jt808gpsRestoreDataSend(gpsinfo, (uint8_t *)dest + destlen);
            }
            else
            {
                gpsRestoreDataSend(gpsinfo, dest + destlen, &protocollen);
            }
            destlen += protocollen;
        }
        sendDataToServer(NORMAL_LINK, (uint8_t *)dest, destlen);
    }
    else
    {
        readlen = nwy_sdk_file_unlink(GPS_RESTORE_FILE_NAME);
        if (readlen == 0)
        {
            readOffset = 0;
            LogMessage(DEBUG_ALL, "Delete gps.save OK\r\n");
        }
        else
        {
            readOffset += 400;
            LogMessage(DEBUG_ALL, "Delete gps.save fail\r\n");
        }
    }
    return 1;
}



static void sendTcpDataDebugShow(char *txdata, int txlen)
{
    int debuglen;
    char senddata[300], slen;
    debuglen = txlen > 128 ? 128 : txlen;
    strcpy(senddata, "TCP Send:");
    slen = strlen(senddata);
    changeByteArrayToHexString((uint8_t *)txdata, (uint8_t *)senddata + slen, (uint16_t) debuglen);
    senddata[slen + debuglen * 2] = 0;
	strcat(senddata,"\r\n");
    LogMessage(DEBUG_ALL, senddata);

}

/*发送协议至服务器*/
void sendProtocolToServer(uint8_t link, PROTOCOLTYPE protocol, void *param)
{
    GPSINFO *gpsinfo;
    char txdata[512];
    int txlen = 0;
    switch (protocol)
    {
        case PROTOCOL_01:
            txlen = createProtocol01((char *)sysparam.SN, createProtocolSerial(), txdata);
            break;
        case PROTOCOL_12:
            gpsinfo = (GPSINFO *)param;
            if ((gpsinfo->hadupload == 1) || gpsinfo->fixstatus == 0)
            {
                return ;
            }
            txlen = createProtocol12((GPSINFO *)param, createProtocolSerial(), txdata);
            break;
        case PROTOCOL_13:
            if (isProtocolReday() == 0)
            {
                return ;
            }
            txlen = createProtocol13(link, createProtocolSerial(), txdata);
            break;
        case PROTOCOL_16:
            txlen = createProtocol16(createProtocolSerial(), txdata, *(uint8_t *)param);
            break;
        case PROTOCOL_19:
            txlen = createProtocol19(createProtocolSerial(), txdata);
            break;
        case PROTOCOL_21:
            txlen = createProtocol21(txdata, (char *)param, strlen((char *)param));
            break;
        case PROTOCOL_8A:
            txlen = createProtocol_8A(createProtocolSerial(), txdata);
            break;
        case PROTOCOL_F1:
            txlen = createProtocolF1(createProtocolSerial(), txdata);
            break;
        case PROTOCOL_F3:
            txlen = createProtocolF3(link, txdata, (WIFI_INFO *)param);
            break;
        case PROTOCOL_UP:
            txlen = createUpdateProtocol((char *)sysparam.SN, createProtocolSerial(), txdata, *(uint8_t *)param);
            break;
        case PROTOCOL_51:
            txlen = createProtocol51(txdata);
            break;
    }

    switch (protocol)
    {
        case PROTOCOL_12:
            if (isProtocolReday())
            {
                sendTcpDataDebugShow(txdata, txlen);
                sendDataToServer(link, (uint8_t *)txdata, txlen);
            }
            else
            {
                gpsRestoreWriteData(&gpsres);
                LogMessage(DEBUG_ALL, "Network error\n");
            }
            break;
        default:
            sendTcpDataDebugShow(txdata, txlen);
            sendDataToServer(link, (uint8_t *)txdata, txlen);
            break;
    }

}

/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/

/*重置状态机连接状态*/
void netConnectReset(void)
{
    netconnect.fsmstate = NETWORK_LOGIN;
    netconnect.heartbeattick = 0;
}
/*切换连接状态*/
void protocolFsmStateChange(NetWorkFsmState state)
{
    netconnect.fsmstate = state;
}
/*运行网络连接状态机,执行登录信息，心跳信息的发送，维持链接稳定*/
void protocolRunFsm(void)
{
    static uint8_t ret = 1;

    switch (netconnect.fsmstate)
    {
        case NETWORK_LOGIN:
            if (strstr((char *)sysparam.SN, "888888887777777") != NULL)
                break;

            sendProtocolToServer(NORMAL_LINK, PROTOCOL_01, NULL);
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_F1, NULL);
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_8A, NULL);
            protocolFsmStateChange(NETWORK_LOGIN_WAIT);
            netconnect.logintick = 0;
            sysinfo.hearbeatrequest = 1;
            netconnect.loginCount++;
            netconnect.heartbeattick = sysparam.heartbeatgap;
            ret = 1;
            break;
        case NETWORK_LOGIN_WAIT:
            if (++netconnect.logintick >= 60)
            {
                if (netconnect.loginCount >= 3)
                {
                    reConnectServer();
                }
                else
                {
                    protocolFsmStateChange(NETWORK_LOGIN);
                }
            }
            break;
        case NETWORK_LOGIN_READY:

            if (++netconnect.heartbeattick >= sysparam.heartbeatgap)
            {
                netconnect.heartbeattick = 0;
                sendProtocolToServer(NORMAL_LINK, PROTOCOL_13, NULL);
            }

            if (netconnect.heartbeattick % 3 == 0)
            {
                if (ret == 1)
                {
                    ret = gpsRestoreReadData();
                }
            }
            break;
        default:
            netconnect.fsmstate = NETWORK_LOGIN;
            netconnect.heartbeattick = 0;
            break;
    }
}
/*--------------------------------------------------------------------------------------------------*/
uint8_t isProtocolReday(void)
{
    uint8_t ret = 0;
    if (sysparam.protocol == USE_JT808_PROTOCOL)
    {
        ret = isJt808Reday();
    }
    else
    {
        if (netconnect.fsmstate == NETWORK_LOGIN_READY && isNetProcessNormal())
            ret = 1;
    }
    return ret;
}
/*--------------------------------------------------------------------------------------------------*/
/* 01 协议解析
登录包回复：78 78 05 01 00 00 C8 55 0D 0A
*/
static void protoclparser01(char *protocol, int size)
{
    netconnect.loginCount = 0;
    protocolFsmStateChange(NETWORK_LOGIN_READY);
    //    updateSystemLedStatus(SYSTEM_LED_SERVEROK, 1);
    LogMessage(DEBUG_ALL, "登录成功\n");
}
/* 13 协议解析
登录包回复：78 78 05 13 00 01 E9 F1 0D 0A
*/
static void protoclparser13(char *protocol, int size)
{
    protocolFsmStateChange(NETWORK_LOGIN_READY);
    LogMessage(DEBUG_ALL, "心跳回复\n");
}
/* 80 协议解析
登录包回复：78 78 13 80 0B 00 1D D9 E6 53 54 41 54 55 53 23 00 00 00 00 A7 79 0D 0A
*/
static void protoclparser80(char *protocol, int size, void *param)
{

    uint8_t instructionlen;
    char debug[128];
    instructionid[0] = protocol[5];
    instructionid[1] = protocol[6];
    instructionid[2] = protocol[7];
    instructionid[3] = protocol[8];
    instructionlen = protocol[4] - 4;
    instructionserier = (protocol[instructionlen + 11] << 8) | (protocol[instructionlen + 12]);
    memset(debug, 0, sizeof(debug));
    strncpy(debug, protocol + 9, instructionlen);
    instructionParser((uint8_t *)debug, instructionlen, NETWORK_MODE, NULL, param);
}

/* 8A 协议解析
登录包回复：78 78 0B 8A 14 0C 10 05 3B 2E 00 03 6B 5B 0D 0A
*/
static void protoclparser8A(char *protocol, int size)
{
    DATETIME datetime;
    if (sysinfo.localrtchadupdate == 1)
        return ;
    sysinfo.localrtchadupdate = 1;
    datetime.year = protocol[4];
    datetime.month = protocol[5];
    datetime.day = protocol[6];
    datetime.hour = protocol[7];
    datetime.minute = protocol[8];
    datetime.second = protocol[9];
    updateLocalRTCTime(&datetime);
}

static void protoclparser616263(uint8_t protocol)
{
    recordUploadRespon();
    appSendThreadEvent(RECORD_UPLOAD_EVENT, THREAD_PARAM_NONE);
}

/* F3 协议解析
命令01回复：79790042F3
01 命令
01 是否需要更新
00000174  文件ID
0004EB34  文件总大小
0F		  SN长度
363930323137303931323030303237   SN号
0B		  当前版本长度
5632303230373134303031
16		  新版本号长度
4137575F353030333030303330385F3131355F34374B
000D   序列号
AD3B   校验
0D0A   结束

命令02回复：79790073F3
02 命令
01 升级有效标志
00000000   文件偏移起始
00000064   文件大小
474D3930312D3031303030344542314333374330304331457F454C4601010100000000000000000002002800010000006C090300E4E9040004EA04000200000534002000010028000700060038009FE510402DE924108FE2090080E002A300EB24109FE5
0003  序列号
27C6  校验
0D0A  借宿
*/

void upgradeResultProcess(uint8_t upgradeResult, uint32_t offset, uint32_t size)
{
    if (upgradeResult == 0)
    {
        uis.rxfileOffset = offset + size;
        LogPrintf(DEBUG_ALL, "\n>>>>>>>>>> Completed progress %.1f%% <<<<<<<<<<\n\n",
                  ((float)uis.rxfileOffset / uis.file_totalsize) * 100);
        customerLogPrintf("+CUPDATE: %d%%\r\n", (uint16_t)(((float)uis.rxfileOffset / uis.file_totalsize) * 100));
        if (uis.rxfileOffset == uis.file_totalsize)
        {
            uis.updateOK = 1;
            protocolFsmStateChange(NETWORK_DOWNLOAD_DONE);
        }
        else if (uis.rxfileOffset > uis.file_totalsize)
        {
            LogMessage(DEBUG_ALL, "Recevie complete ,but total size is different,retry again\n");
            uis.rxfileOffset = 0;
            protocolFsmStateChange(NETWORK_LOGIN);
        }
        else
        {
            //protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
            appSendThreadEvent(GET_FIRMWARE_EVENT, THREAD_PARAM_NONE);
        }
    }
    else
    {
        LogPrintf(DEBUG_ALL, "Writing firmware error at 0x%X\n", uis.rxfileOffset);
        protocolFsmStateChange(NETWORK_DOWNLOAD_ERROR);
    }

}

static void protocolParserUpdate(char *protocol, int size)
{
    uint8_t cmd, snlen, myversionlen, newversionlen;
    uint16_t index, filecrc, calculatecrc;
    uint32_t rxfileoffset, rxfilelen;
    char *codedata;
    ota_package_t ota_pack;
    int ret;
    cmd = protocol[5];
    if (cmd == 0x01)
    {
        //判断是否有更新文件
        if (protocol[6] == 0x01)
        {
            uis.file_id = (protocol[7] << 24 | protocol[8] << 16 | protocol[9] << 8 | protocol[10]);
            uis.file_totalsize = (protocol[11] << 24 | protocol[12] << 16 | protocol[13] << 8 | protocol[14]);
            snlen = protocol[15];
            index = 16;
            if (snlen > (sizeof(uis.rxsn) - 1))
            {
                LogPrintf(DEBUG_ALL, "Sn too long %d\n", snlen);
                return ;
            }
            strncpy(uis.rxsn, (char *)&protocol[index], snlen);
            uis.rxsn[snlen] = 0;
            index = 16 + snlen;
            myversionlen = protocol[index];
            index += 1;
            if (myversionlen > (sizeof(uis.rxcurCODEVERSION) - 1))
            {
                LogPrintf(DEBUG_ALL, "myversion too long %d\n", myversionlen);
                return ;
            }
            strncpy(uis.rxcurCODEVERSION, (char *)&protocol[index], myversionlen);
            uis.rxcurCODEVERSION[myversionlen] = 0;
            index += myversionlen;
            newversionlen = protocol[index];
            index += 1;
            if (newversionlen > (sizeof(uis.newCODEVERSION) - 1))
            {
                LogPrintf(DEBUG_ALL, "newversion too long %d\n", newversionlen);
                return ;
            }
            strncpy(uis.newCODEVERSION, (char *)&protocol[index], newversionlen);
            uis.newCODEVERSION[newversionlen] = 0;
            LogPrintf(DEBUG_ALL, "File %08X , Total size=%d Bytes\n", uis.file_id, uis.file_totalsize);
            LogPrintf(DEBUG_ALL, "My SN:%s\nMy Ver:%s\nNew Ver:%s\n", uis.rxsn, uis.rxcurCODEVERSION, uis.newCODEVERSION);
            protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
            if (uis.rxfileOffset != 0)
            {
                LogMessage(DEBUG_ALL, "Update firmware continute\n");
            }
            else
            {
                customerLogOut("+CUPDATE: OLD\r\n");
                ota_pack.len = 0;
                ota_pack.offset = 0;
                if (uis.updateObject == UPDATE_MCU_OBJECT)
                {
                    protocolFsmStateChange(NETWORK_MCU_START_UPGRADE);
                }
                else
                {
                    protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
                }
            }
        }
        else
        {
            LogMessage(DEBUG_ALL, "No update file\n");
            customerLogOut("+CUPDATE: NEW\r\n");
            protocolFsmStateChange(NETWORK_UPGRADE_CANCEL);
        }
    }
    else if (cmd == 0x02)
    {
        if (protocol[6] == 1)
        {
            rxfileoffset = (protocol[7] << 24 | protocol[8] << 16 | protocol[9] << 8 | protocol[10]); //文件偏移
            rxfilelen = (protocol[11] << 24 | protocol[12] << 16 | protocol[13] << 8 | protocol[14]); //文件大小
            calculatecrc = GetCrc16(protocol + 2, size - 6); //文件校验
            filecrc = (*(protocol + 15 + rxfilelen + 2) << 8) | (*(protocol + 15 + rxfilelen + 2 + 1));
            if (rxfileoffset < uis.rxfileOffset)
            {
                LogMessage(DEBUG_ALL, "Receive the same firmware\n");
                //protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
                appSendThreadEvent(GET_FIRMWARE_EVENT, THREAD_PARAM_NONE);
                return ;
            }
            if (calculatecrc == filecrc)
            {
                LogMessage(DEBUG_ALL, "Data validation OK,Writting...\n");
                codedata = protocol + 15;

                ota_pack.offset = rxfileoffset;
                ota_pack.len = rxfilelen;
                ota_pack.data = (uint8_t *)codedata;

                //ret = nwy_fota_dm(&ota_pack);
                if (uis.updateObject == UPDATE_MODULE_OBJECT)
                {
                    ret = nwy_fota_download_core(&ota_pack);
                    upgradeResultProcess(ret, ota_pack.offset, ota_pack.len);
                }
                else
                {
                    pushUpgradeFirmWare(ota_pack.data, ota_pack.offset, ota_pack.len);
                    protocolFsmStateChange(NETWORK_FIRMWARE_WRITE_DOING);
                }

            }
            else
            {
                LogMessage(DEBUG_ALL, "Data validation Fail\n");
                //protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
                appSendThreadEvent(GET_FIRMWARE_EVENT, THREAD_PARAM_NONE);
            }
        }
        else
        {
            LogMessage(DEBUG_ALL, "未知\n");
        }
    }
}

/*-------------------------------------------------------------------------------*/
/* 51 协议解析
文件下发：78 78 0F 51 03 00 00 02 00 00 1B 00 00 00 4D 1C 55 D0 0D 0A
*/


static void protoclparser51(char *protocol, int size)
{
    //文件类型
    audiofile.audioType = protocol[4];
    //分包数
    audiofile.audioCnt = protocol[6] << 8 | protocol[7];
    //文件大小
    audiofile.audioSize = protocol[8];
    audiofile.audioSize <<= 8;
    audiofile.audioSize |= protocol[9];
    audiofile.audioSize <<= 8;
    audiofile.audioSize |= protocol[10];
    audiofile.audioSize <<= 8;
    audiofile.audioSize |= protocol[11];
    //文件ID
    audiofile.audioId = protocol[12];
    audiofile.audioId <<= 8;
    audiofile.audioId |= protocol[13];
    audiofile.audioId <<= 8;
    audiofile.audioId |= protocol[14];
    audiofile.audioId <<= 8;
    audiofile.audioId |= protocol[15];
    LogPrintf(DEBUG_ALL, "Type:%d,Cnt:%d,Size:%d,Id:%X\r\n", audiofile.audioType, audiofile.audioCnt, audiofile.audioSize,
              audiofile.audioId);
    appDeleteAudio();
    sendProtocolToServer(NORMAL_LINK, PROTOCOL_51, NULL);
}
/* 52 协议解析
文件下发：	79 79 10 09 52 00 00 FF F3 68 C4 00 00 00
*/

static void protoclparser52(char *protocol, int size)
{
    uint16_t packid;
    packid = protocol[5] << 8 | protocol[6];
    appSaveAudio((uint8_t *)protocol + 7, size - 15);
    LogPrintf(DEBUG_ALL, "Receive Audio Num:%d,size:%d\r\n", packid, size - 15);
    if ((packid + 1) == audiofile.audioCnt)
    {
        LogMessage(DEBUG_ALL, "Play audio\r\n");
        appSendThreadEvent(PLAY_MUSIC_EVENT, THREAD_PARAM_AUDIO);
    }
}

/*-------------------------------------------------------------------------------*/

/*解析接收到的服务器协议*/
static void protocolRxParser(uint8_t link, char *protocol, int size)
{
    if (protocol[0] == 0X78 && protocol[1] == 0X78)
    {
        switch (protocol[3])
        {
            case (uint8_t)0x01:
                if (link == NORMAL_LINK)
                {
                    protoclparser01(protocol, size);
                }
                else
                {
                    bleServerLoginOk();
                }
                break;
            case (uint8_t)0x13:
                protoclparser13(protocol, size);
                break;
            case (uint8_t)0x80:
                if (link == NORMAL_LINK)
                {
                    protoclparser80(protocol, size, NULL);
                }
                else
                {
                    protoclparser80(protocol, size, &link);
                }
                break;
            case (uint8_t)0x8A:
                protoclparser8A(protocol, size);
                break;
            case (uint8_t)0x61:
            case (uint8_t)0x62:
                protoclparser616263(protocol[3]);
                break;
            case (uint8_t)0x51:
                protoclparser51(protocol, size);
                break;
        }
    }
    else if (protocol[0] == 0X79 && protocol[1] == 0X79)
    {
        switch (protocol[4])
        {
            case (uint8_t)0xF3:
                protocolParserUpdate(protocol, size);
                break;
            case (uint8_t)0x52:
                protoclparser52(protocol, size);
                break;
        }
    }
    else
    {
        LogMessage(DEBUG_ALL, "protocolRxParase:Error\n");
    }
}


/*
78 78 05 01 00 00 C8 55 0D 0A
79 79 00 05 52 00 00 FF F3 0D 0A

*/
#define PROTOCOL_BUFSZIE	6144  //6k

void protocolReceivePush(uint8_t line, char *protocol, int size)
{
    static uint8_t dataBuf[PROTOCOL_BUFSZIE];
    static uint16_t dataBufLen = 0;
    uint16_t remain, i, contentlen, lastindex = 0, beginindex;
    //剩余空间大小
    remain = PROTOCOL_BUFSZIE - dataBufLen;
    if (remain == 0)
    {
        LogMessage(DEBUG_ALL, "buff full,clear all\r\n");
        dataBufLen = 0;
        remain = PROTOCOL_BUFSZIE;
    }
    //可写入内容
    size = size > remain ? remain : size;
    //LogPrintf(DEBUG_ALL, "Push %d,", size);
    //数据复制
    memcpy(dataBuf + dataBufLen, protocol, size);
    dataBufLen += size;
    //遍历，寻找7878
    //LogPrintf(DEBUG_ALL, "Size:%d\r\n", dataBufLen);
    for (i = 0; i < dataBufLen; i++)
    {
        beginindex = i;
        if (dataBuf[i] == 0x78)
        {
            if (i + 1 >= dataBufLen)
            {
                continue ;
            }
            if (dataBuf[i + 1] != 0x78)
            {
                continue ;
            }
            if (i + 2 >= dataBufLen)
            {
                continue ;
            }
            contentlen = dataBuf[i + 2];
            if ((i + 5 + contentlen) > dataBufLen)
            {
                continue ;
            }
            if (dataBuf[i + 3 + contentlen] == 0x0D && dataBuf[i + 4 + contentlen] == 0x0A)
            {
                i += (4 + contentlen);
                lastindex = i + 1;
                //LogPrintf(DEBUG_ALL, "Fint it ====>Begin:7878[%d,%d]\r\n", beginindex, lastindex - beginindex);
                protocolRxParser(line, (char *)dataBuf + beginindex, lastindex - beginindex);
            }
            //            else
            //            {
            //                LogMessage(DEBUG_ALL, "78no find\r\n");
            //            }
        }
        else if (dataBuf[i] == 0x79)
        {
            if (i + 1 >= dataBufLen)
            {
                continue ;
            }
            if (dataBuf[i + 1] != 0x79)
            {
                continue ;
            }
            //找长度
            if (i + 3 >= dataBufLen)
            {
                continue ;
            }
            contentlen = dataBuf[i + 2] << 8 | dataBuf[i + 3];
            if ((i + 6 + contentlen) > dataBufLen)
            {
                continue ;
            }
            if (dataBuf[i + 4 + contentlen] == 0x0D && dataBuf[i + 5 + contentlen] == 0x0A)
            {
                i += (5 + contentlen);
                lastindex = i + 1;
                LogPrintf(DEBUG_ALL, "Fint it ====>Begin:7979[%d,%d]\r\n", beginindex, lastindex - beginindex);
                protocolRxParser(line, (char *)dataBuf + beginindex, lastindex - beginindex);
            }
            //            else
            //            {
            //                LogMessage(DEBUG_ALL, "79no find\r\n");
            //            }
        }
    }
    if (lastindex != 0)
    {
        remain = dataBufLen - lastindex;
        //LogPrintf(DEBUG_ALL, "Remain:%d\r\n", remain);
        if (remain != 0)
        {
            memcpy(dataBuf, dataBuf + lastindex, remain);
        }
        dataBufLen = remain;
    }

}

void save123InstructionId(void)
{
    instructionid123[0] = instructionid[0];
    instructionid123[1] = instructionid[1];
    instructionid123[2] = instructionid[2];
    instructionid123[3] = instructionid[3];
}
void reCover123InstructionId(void)
{
    instructionid[0] = instructionid123[0];
    instructionid[1] = instructionid123[1];
    instructionid[2] = instructionid123[2];
    instructionid[3] = instructionid123[3];
}

uint8_t *getInstructionId(void)
{
    return instructionid;
}

/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/

void updateMcuVersion(char *version)
{
    strcpy(uis.curCODEVERSION, version);
}

void updateUISInit(uint8_t object)
{
    memset(&uis, 0, sizeof(UndateInfoStruct));

    object = object > 0 ? 1 : 0;

    uis.updateObject = object;
    if (object)
    {
        strcpy(uis.curCODEVERSION, "MCU_DEF");
        doGetVersionCmd();
        uis.file_len = 1024;
    }
    else
    {
        strcpy(uis.curCODEVERSION, EEPROM_VERSION);
        uis.file_len = 1300;
    }
    LogPrintf(DEBUG_ALL, "%s Version:%s\n", object ? "Mcu" : "Module", uis.curCODEVERSION);
}


uint32_t getUpgradeFileSize(void)
{
    return uis.file_totalsize;

}

//远程升级
void UpdateProtocolRunFsm(void)
{
    uint8_t cmd;
    static uint8_t dlErrCount = 0;
    static uint32_t lastOffset = 0;
    static uint8_t waittick = 0;
    switch (netconnect.fsmstate)
    {
        //登录
        case NETWORK_LOGIN:
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_01, NULL);
            protocolFsmStateChange(NETWORK_LOGIN_WAIT);
            netconnect.logintick = 0;
            netconnect.loginCount++;
            netconnect.heartbeattick = 0;
            dlErrCount = 0;
            break;
        //登录等待
        case NETWORK_LOGIN_WAIT:
            if (netconnect.logintick++ >= 45)
            {
                protocolFsmStateChange(NETWORK_LOGIN);
                if (netconnect.loginCount >= 3)
                {
                    netconnect.loginCount = 0;
                    LogMessage(DEBUG_ALL, "LoginFail\r\n");
                    protocolFsmStateChange(NETWORK_DOWNLOAD_END);
                }
            }
            break;
        case NETWORK_LOGIN_READY:
            //登录后获取新软件版本，未获取，每隔30秒重新获取
            if (netconnect.heartbeattick++ % 30 == 0)
            {
                cmd = 1;
                sendProtocolToServer(NORMAL_LINK, PROTOCOL_UP, &cmd);
                netconnect.getVerCount++;
                if (netconnect.getVerCount > 3)
                {
                    netconnect.getVerCount = 0;
                    LogMessage(DEBUG_ALL, "get version fail\r\n");
                    protocolFsmStateChange(NETWORK_DOWNLOAD_END);
                }
            }
            break;
        case NETWORK_DOWNLOAD_DOING:
            netconnect.getVerCount = 0;
            //发送下载协议,并进入等待状态
            cmd = 2;
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_UP, &cmd);
            protocolFsmStateChange(NETWORK_DOWNLOAD_WAIT);
            netconnect.heartbeattick = 0;
            break;
        case NETWORK_DOWNLOAD_WAIT:
            //等下固件下载，超过20秒未收到数据，重新发送下载协议
            LogPrintf(DEBUG_ALL, "Waitting firmware data...[%d]\n", netconnect.heartbeattick);
            if (netconnect.heartbeattick++ > 20)
            {
                protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
            }
            break;
        //mcu 开始升级
        case NETWORK_MCU_START_UPGRADE:
            LogMessage(DEBUG_ALL, "mcu start upgrade\r\n");
            doStartUpgrade();
            doResetMcu();
            protocolFsmStateChange(NETWORK_MCU_EARSE);
            break;
        case NETWORK_MCU_EARSE:
            LogMessage(DEBUG_ALL, "mcu earse\r\n");
            doEarseCmd(uis.file_totalsize);
            protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
            break;
        //MCU 升级
        case NETWORK_FIRMWARE_WRITE_DOING:
            LogMessage(DEBUG_ALL, "firmware writting...\r\n");
            //appMcuUpgradeTask();
            break;
        case NETWORK_DOWNLOAD_DONE:
            //下载写入完成
            LogMessage(DEBUG_ALL, "Download firmware complete!\n");
            cmd = 3;
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_UP, &cmd);
            protocolFsmStateChange(NETWORK_UPGRAD_NOW);
            LogMessage(DEBUG_ALL, "Wait upgrade\r\n");
            waittick = 0;
            break;
        case NETWORK_UPGRAD_NOW:
            if (++waittick >= 1)
            {
                LogMessage(DEBUG_ALL, "Start upgrade\r\n");
                customerLogOut("+CUPDATE: READY\r\n");
                if (uis.updateObject == UPDATE_MCU_OBJECT)
                {
                    doDoneCmd();
                    startTimer(10, doResetMcu, 0);
                    protocolFsmStateChange(NETWORK_DOWNLOAD_END);
                }
                else
                {
                    sysparam.cUpdate = CUPDATE_FLAT;
                    paramSaveAll();
                    nwy_version_update(true);
                    customerLogOut("+CUPDATE: FAIL\r\n");
                    sysparam.cUpdate = 0;
                    paramSaveAll();
                    //升级成功时，直接重启，不成功时，则返回
                    uis.updateOK = 0;
                    cmd = 3;
                    sendProtocolToServer(NORMAL_LINK, PROTOCOL_UP, &cmd);
                    protocolFsmStateChange(NETWORK_WAIT_JUMP);
                    LogMessage(DEBUG_ALL, "checksum failed\r\n");
                }
            }
            break;
        case NETWORK_DOWNLOAD_ERROR:

            LogMessage(DEBUG_ALL, "Download error\r\n");
            protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
            if (lastOffset != uis.rxfileOffset)
            {
                LogMessage(DEBUG_ALL, "Diff offset\r\n");
                lastOffset = uis.rxfileOffset;
                dlErrCount = 0;
            }
            dlErrCount++;
            if (dlErrCount >= 5)
            {
                LogMessage(DEBUG_ALL, "Download end\r\n");
                protocolFsmStateChange(NETWORK_DOWNLOAD_END);
            }
            break;
        case NETWORK_WAIT_JUMP:
            LogMessage(DEBUG_ALL, "Upgrade fail\r\n");
            protocolFsmStateChange(NETWORK_DOWNLOAD_END);
            break;
        case NETWORK_DOWNLOAD_END:
            customerLogOut("+CUPDATE: ERROR\r\n");
            protocolFsmStateChange(NETWORK_UPGRADE_CANCEL);
            break;
        case NETWORK_UPGRADE_CANCEL:
            updateSystemLedStatus(SYSTEM_LED_UPDATE, 0);
            sysinfo.updateStatus = 0;
            socketClose(NORMAL_LINK);
            netConnectReset();
            break;
    }
}





void getFirmwareInThreadEvent(void)
{
    uint8_t cmd;
    cmd = 2;
    sendProtocolToServer(NORMAL_LINK, PROTOCOL_UP, &cmd);
    protocolFsmStateChange(NETWORK_DOWNLOAD_WAIT);
    netconnect.heartbeattick = 0;
}

void UpdateStop(void)
{
    customerLogOut("+CUPDATE: ERROR\r\n");
    updateSystemLedStatus(SYSTEM_LED_UPDATE, 0);
    sysinfo.updateStatus = 0;
    socketClose(NORMAL_LINK);
    netConnectReset();
}

