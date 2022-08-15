#include "app_gps.h"
#include "app_sys.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "app_protocol.h"
#include "app_param.h"
#include "app_net.h"
#include "app_task.h"
#include "app_port.h"
#include "app_jt808.h"

#define PIA 3.1415926

static gpsinfo_s gpsinfonow;
static gpsfifo_s gpsfifo;
static lastUploadPosition_s lastuploadgpsinfo;

/**************************************************
@bref		打印gps信息
@param
@return
@note
**************************************************/

void showgpsinfo(void)
{
    char debug[300];
    uint8_t i, total;
    uint32_t laValue, loValue;
    gpsinfo_s *gpsinfo;
    gpsinfo = &gpsinfonow;
    sprintf(debug, "NMEA:%02u/%02u/%02u %02u:%02u:%02u;", gpsinfo->datetime.year, gpsinfo->datetime.month,
            gpsinfo->datetime.day, gpsinfo->datetime.hour, gpsinfo->datetime.minute, gpsinfo->datetime.second);
    laValue = gpsinfo->latitude * 1000000;
    loValue = gpsinfo->longtitude * 1000000;
	
    sprintf(debug + strlen(debug), "%c %lu.%lu %c %lu.%lu\r\n", gpsinfo->EW, laValue / 1000000, laValue % 1000000, gpsinfo->NS,
            loValue / 1000000, loValue % 1000000);
    sprintf(debug + strlen(debug), "%s;", gpsinfo->fixstatus ? "FIXED" : "Invalid");
    loValue = gpsinfo->pdop * 100;
    sprintf(debug + strlen(debug), "PDOP=%lu.%lu;", loValue / 100, loValue % 100);
    sprintf(debug + strlen(debug), "fixmode=%u;", gpsinfo->fixmode);
    sprintf(debug + strlen(debug), "GPS View=%u;Beidou View=%u;", gpsinfo->gpsviewstart, gpsinfo->beidouviewstart);
    sprintf(debug + strlen(debug), "Use Star=%u;", gpsinfo->used_star);
    sprintf(debug + strlen(debug), "GPS CN:");
    total = sizeof(gpsinfo->gpsCn);
    for (i = 0; i < total; i++)
    {
        if (gpsinfo->gpsCn[i] != 0)
        {
            sprintf(debug + strlen(debug), "%u;", gpsinfo->gpsCn[i]);
        }
    }
    sprintf(debug + strlen(debug), "BeiDou CN:");
    total = sizeof(gpsinfo->beidouCn);
    for (i = 0; i < total; i++)
    {
        if (gpsinfo->beidouCn[i] != 0)
        {
            sprintf(debug + strlen(debug), "%u;", gpsinfo->beidouCn[i]);
        }
    }
    LogMessage(DEBUG_ALL, debug);
}

/**************************************************
@bref		gps过滤
@param
@return
@note
**************************************************/

static uint8_t  gpsFilter(gpsinfo_s *gpsinfo)
{
    if (gpsinfo->fixstatus == 0)
    {
        return 0;
    }
    if (gpsinfo->fixmode != 3)
    {
        return 0 ;
    }
    if (gpsinfo->pdop > sysparam.pdop)
    {
        return 0;
    }
    if (gpsinfo->datetime.day == 0)
    {
        return 0;
    }
    return 1;
}
/**************************************************
@bref		添加新gps信息到队列中
@param
@return
@note
**************************************************/
static void addGpsInfo(gpsinfo_s *gpsinfo)
{

    if (gpsinfo->fixstatus)
    {
        sysinfo.gpsLastFixTick = sysinfo.sysTick;
        if (sysinfo.updateLocalTimeReq)
        {
            sysinfo.updateLocalTimeReq = 0;
            portUpdateLocalTime(gpsinfo->datetime.year, gpsinfo->datetime.month,
                                gpsinfo->datetime.day, gpsinfo->datetime.hour,
                                gpsinfo->datetime.minute, gpsinfo->datetime.second,
                                sysparam.utc);
        }

    }


    if (gpsFilter(gpsinfo) == 0)
    {
        gpsinfo->fixstatus = 0;
    }
    else
    {
        memcpy(&gpsfifo.lastfixgpsinfo, gpsinfo, sizeof(gpsinfo_s));
    }

    sysinfo.gpsUpdateTick = sysinfo.sysTick;

    gpsfifo.currentindex = (gpsfifo.currentindex + 1) % GPSFIFOSIZE;
    //往队列中添加数据
    memcpy(&gpsfifo.gpsinfohistory[gpsfifo.currentindex], gpsinfo, sizeof(gpsinfo_s));
    //showgpsinfo();
}
/**************************************************
@bref		清除当前gps
@param
@return
@note
**************************************************/

void gpsClearCurrentGPSInfo(void)
{
    memset(&gpsfifo.gpsinfohistory[gpsfifo.currentindex], 0, sizeof(gpsinfo_s));
}
/**************************************************
@bref		获取最新gps信息
@param
@return
@note
**************************************************/

gpsinfo_s *getCurrentGPSInfo(void)
{
    return &gpsfifo.gpsinfohistory[gpsfifo.currentindex];
}
/**************************************************
@bref		获取最后定位
@param
@return
@note
**************************************************/

gpsinfo_s *getLastFixedGPSInfo(void)
{
    return &gpsfifo.lastfixgpsinfo;
}
/**************************************************
@bref		获取gps信息队列
@param
@return
@note
**************************************************/

gpsfifo_s *getGSPfifo(void)
{
    return &gpsfifo;
}



/**************************************************
@bref		维度转换
@param
	latitude		原始维度
	Direction		方向，E/W
@return
	新维度
@note
**************************************************/

static double latitude_to_double(double latitude, char Direction)
{
    double f_lat;
    unsigned long degree, fen, tmpval;
    if (latitude == 0 || (Direction != 'N' && Direction != 'S'))
    {
        return 0.0;
    }
    //扩大10万倍,忽略最后一个小数点,变成整型
    tmpval = (unsigned long)(latitude * 100000.0);
    //获取到维度--度
    degree = tmpval / (100 * 100000);
    //获取到维度--秒
    fen = tmpval % (100 * 100000);
    if (Direction == 'S')
    {
        f_lat = ((double)degree + ((double)fen) / 100000.0 / 60.0) * (-1);
    }
    else
    {
        f_lat = (double)degree + ((double)fen) / 100000.0 / 60.0;
    }
    return f_lat;
}

/**************************************************
@bref		经度转换
@param
	longitude		原始经度
	Direction		方向，E/W
@return
	新经度
@note
**************************************************/

static double longitude_to_double(double longitude, char Direction)
{
    double  f_lon;
    unsigned long degree, fen, tmpval;
    if (longitude == 0 || (Direction != 'E' && Direction != 'W'))
    {
        return 0.0;
    }
    tmpval = (unsigned long)(longitude * 100000.0);
    degree = tmpval / (100 * 100000);
    fen = tmpval % (100 * 100000);
    if (Direction == 'W')
    {
        f_lon = ((double)degree + ((double)fen) / 100000.0 / 60.0) * (-1);
    }
    else
    {
        f_lon = (double)degree + ((double)fen) / 100000.0 / 60.0;
    }
    return f_lon;

}

/*
$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh<CR><LF>
<1> UTC时间，hhmmss（时分秒）格式
<2> 定位状态，A=有效定位，V=无效定位
<3> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
<4> 纬度半球N（北半球）或S（南半球）
<5> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
<6> 经度半球E（东经）或W（西经）
<7> 地面速率（000.0~999.9节，前面的0也将被传输）
<8> 地面航向（000.0~359.9度，以真北为参考基准，前面的0也将被传输）
<9> UTC日期，ddmmyy（日月年）格式
<10> 磁偏角（000.0~180.0度，前面的0也将被传输）
<11> 磁偏角方向，E（东）或W（西）
<12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）
*/
static void parse_RMC(gpsitem_s *item)
{
    gpsinfo_s *gpsinfo;
    gpsinfo = &gpsinfonow;
    if (item->item_data[1][0] != 0)
    {
        gpsinfo->datetime.hour = (item->item_data[1][0] - '0') * 10 + (item->item_data[1][1] - '0');
        gpsinfo->datetime.minute = (item->item_data[1][2] - '0') * 10 + (item->item_data[1][3] - '0');
        gpsinfo->datetime.second = (item->item_data[1][4] - '0') * 10 + (item->item_data[1][5] - '0');
    }
    if (item->item_data[2][0] != 0)
    {
        if (item->item_data[2][0] == 'A')
        {
            gpsinfo->fixstatus = GPSFIXED;
        }
        else
        {
            gpsinfo->fixstatus = GPSINVALID;
        }
    }
    if (item->item_data[3][0] != 0)
    {
        gpsinfo->latitude = atof(item->item_data[3]);
    }
    if (item->item_data[4][0] != 0)
    {
        gpsinfo->NS = item->item_data[4][0];
        gpsinfo->latitude = latitude_to_double(gpsinfo->latitude, gpsinfo->NS);
    }
    if (item->item_data[5][0] != 0)
    {
        gpsinfo->longtitude = atof(item->item_data[5]);
    }
    if (item->item_data[6][0] != 0)
    {
        gpsinfo->EW = item->item_data[6][0];
        gpsinfo->longtitude = longitude_to_double(gpsinfo->longtitude, gpsinfo->EW);
    }
    if (item->item_data[7][0] != 0)
    {
        gpsinfo->speed = atof(item->item_data[7]) * 1.852;
    }
    if (item->item_data[8][0] != 0)
    {
        gpsinfo->course = (uint16_t)atoi(item->item_data[8]);
    }
    if (item->item_data[9][0] != 0)
    {
        gpsinfo->datetime.day = (item->item_data[9][0] - '0') * 10 + (item->item_data[9][1] - '0');
        gpsinfo->datetime.month = (item->item_data[9][2] - '0') * 10 + (item->item_data[9][3] - '0');
        gpsinfo->datetime.year = (item->item_data[9][4] - '0') * 10 + (item->item_data[9][5] - '0');

    }
    addGpsInfo(gpsinfo);
    memset(gpsinfo, 0, sizeof(gpsinfo_s));
}
/*
$GPGSA
例：$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A
字段0：$GPGSA，语句ID，表明该语句为GPS DOP and Active Satellites（GSA）当前卫星信息
字段1：定位模式，A=自动手动2D/3D，M=手动2D/3D
字段2：定位类型，1=未定位，2=2D定位，3=3D定位
字段3：PRN码（伪随机噪声码），第1信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段4：PRN码（伪随机噪声码），第2信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段5：PRN码（伪随机噪声码），第3信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段6：PRN码（伪随机噪声码），第4信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段7：PRN码（伪随机噪声码），第5信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段8：PRN码（伪随机噪声码），第6信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段9：PRN码（伪随机噪声码），第7信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段10：PRN码（伪随机噪声码），第8信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段11：PRN码（伪随机噪声码），第9信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段12：PRN码（伪随机噪声码），第10信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段13：PRN码（伪随机噪声码），第11信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段14：PRN码（伪随机噪声码），第12信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段15：PDOP综合位置精度因子（0.5 - 99.9）
字段16：HDOP水平精度因子（0.5 - 99.9）
字段17：VDOP垂直精度因子（0.5 - 99.9）
字段18：校验值
*/
static void parse_GSA(gpsitem_s *item)
{
    gpsinfo_s *gpsinfo;
    gpsinfo = &gpsinfonow;

    if (item->item_data[2][0] != 0)
    {
        gpsinfo->fixmode = atoi(item->item_data[2]);
    }
    if (item->item_data[15][0] != 0)
    {
        gpsinfo->pdop = atof(item->item_data[15]);
    }
}
/*
$GPGSV
例：$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70
字段0：$GPGSV，语句ID，表明该语句为GPS Satellites in View（GSV）可见卫星信息
字段1：本次GSV语句的总数目（1 - 3）
字段2：本条GSV语句是本次GSV语句的第几条（1 - 3）
字段3：当前可见卫星总数（00 - 12）（前导位数不足则补0）
字段4：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段5：卫星仰角（00 - 90）度（前导位数不足则补0）
字段6：卫星方位角（00 - 359）度（前导位数不足则补0）
字段7：信噪比（00－99）dbHz
字段8：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段9：卫星仰角（00 - 90）度（前导位数不足则补0）
字段10：卫星方位角（00 - 359）度（前导位数不足则补0）
字段11：信噪比（00－99）dbHz
字段12：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段13：卫星仰角（00 - 90）度（前导位数不足则补0）
字段14：卫星方位角（00 - 359）度（前导位数不足则补0）
字段15：信噪比（00－99）dbHz
字段16：校验值
*/
static void parse_GSV(gpsitem_s *item)
{
    gpsinfo_s *gpsinfo;
    uint8_t gpskind = 99, currentpage;
    static uint8_t count = 0;
    gpsinfo = &gpsinfonow;
    if (my_strpach(item->item_data[0], (const char *)"$GP"))
    {
        gpskind = 0;
    }
    else if (my_strpach(item->item_data[0], (const char *)"$GB"))
    {
        gpskind = 1;
    }
    else if (my_strpach(item->item_data[0], (const char *)"$BD"))
    {
        gpskind = 1;
    }
    else if (my_strpach(item->item_data[0], (const char *)"$GL"))
    {
        gpskind = 2;
    }

    if (item->item_data[3][0] != 0)
    {
        currentpage = atoi(item->item_data[2]);
        switch (gpskind)
        {
            case 0:
                gpsinfo->gpsviewstart = atoi(item->item_data[3]);

                if (currentpage == 1)
                {
                    count = 0;
                    memset(gpsinfo->gpsCn, 0, sizeof(gpsinfo->gpsCn));
                }
                if (item->item_data[7][0] != 0)
                {
                    gpsinfo->gpsCn[count++] = atoi(item->item_data[7]);
                }
                if (item->item_data[11][0] != 0)
                {
                    gpsinfo->gpsCn[count++] = atoi(item->item_data[11]);
                }
                if (item->item_data[15][0] != 0)
                {
                    gpsinfo->gpsCn[count++] = atoi(item->item_data[15]);
                }
                if (item->item_data[19][0] != 0)
                {
                    gpsinfo->gpsCn[count++] = atoi(item->item_data[19]);
                }
                break;
            case 1:
                gpsinfo->beidouviewstart = atoi(item->item_data[3]);
                if (currentpage == 1)
                {
                    count = 0;
                    memset(gpsinfo->beidouCn, 0, sizeof(gpsinfo->beidouCn));
                }
                if (item->item_data[7][0] != 0)
                {
                    gpsinfo->beidouCn[count++] = atoi(item->item_data[7]);
                }
                if (item->item_data[11][0] != 0)
                {
                    gpsinfo->beidouCn[count++] = atoi(item->item_data[11]);
                }
                if (item->item_data[15][0] != 0)
                {
                    gpsinfo->beidouCn[count++] = atoi(item->item_data[15]);
                }
                if (item->item_data[19][0] != 0)
                {
                    gpsinfo->beidouCn[count++] = atoi(item->item_data[19]);
                }
                break;
            case 2:
                gpsinfo->glonassviewstart = atoi(item->item_data[3]);
                break;
        }
    }
}
/*
$GPGGA
例：$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F
字段0：$GPGGA，语句ID，表明该语句为Global Positioning System Fix Data（GGA）GPS定位信息
字段1：UTC 时间，hhmmss.sss，时分秒格式
字段2：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段3：纬度N（北纬）或S（南纬）
字段4：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段5：经度E（东经）或W（西经）
字段6：GPS状态，0=未定位，1=非差分定位，2=差分定位，3=无效PPS，6=正在估算
字段7：正在使用的卫星数量（00 - 12）（前导位数不足则补0）
字段8：HDOP水平精度因子（0.5 - 99.9）
字段9：海拔高度（-9999.9 - 99999.9）
字段10：地球椭球面相对大地水准面的高度
字段11：差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
字段12：差分站ID号0000 - 1023（前导位数不足则补0，如果不是差分定位将为空）
字段13：校验值
*/
static void parse_GGA(gpsitem_s *item)
{
    gpsinfo_s *gpsinfo;
    gpsinfo = &gpsinfonow;
    if (item->item_data[7][0] != 0)
    {
        gpsinfo->used_star = atoi(item->item_data[7]);
    }
}

/**************************************************
@bref		解析nmea类型
@param
	str
@return
	新经度
@note
**************************************************/

static nmeatype_e parseGetNmeaType(char *str)
{
    if (my_strstr(str, "RMC", 6))
    {
        return NMEA_RMC;
    }

    if (my_strstr(str, "GSA", 6))
    {
        return NMEA_GSA;
    }

    if (my_strstr(str, "GGA", 6))
    {
        return NMEA_GGA;
    }

    if (my_strstr(str, "GSV", 6))
    {
        return NMEA_GSV;
    }

    return 0;
}
/**************************************************
@bref		计算校验
@param
	str
	len
@return

@note
**************************************************/

static uint8_t nemaCalcuateCrc(char *str, int len)
{
    int i, index, size;
    unsigned char crc;
    crc = str[1];
    index = getCharIndex((uint8_t *)str, len, '*');
    if (index < 0)
    {
        return crc;
    }
    size = len - index;
    for (i = 2; i < len - size; i++)
    {
        crc ^= str[i];
    }
    return crc;
}
/**************************************************
@bref		char转换成数值
@param
	char
@return

@note
**************************************************/

static uint8_t chartohexcharvalue(char value)
{
    if (value >= '0' && value <= '9')
        return value - '0';
    if (value >= 'a' && value <= 'z')
        return value - 'a' + 10;
    if (value >= 'A' && value <= 'Z')
        return value - 'A' + 10;
    return 0;
}
/**************************************************
@bref		char字符串转换为数值
@param
	value
@return

@note
**************************************************/

static uint32_t charstrToHexValue(char *value)
{
    unsigned int calvalue = 0;
    unsigned char i, j, len, hexVal;
    len = strlen(value);
    j = 0;
    j = len;
    if (len == 0)
    {
        return 0;
    }
    for (i = 0; i < len; i++)
    {
        hexVal = chartohexcharvalue(value[i]);
        calvalue += hexVal * pow(16, j - 1);
        j--;
    }
    return calvalue;
}
/**************************************************
@bref		解析一条nmea数据
@param
	str
	len
@return

@note
$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F
**************************************************/

static void parseGPS(uint8_t *str, uint16_t len)
{
    gpsitem_s item;
    uint8_t nmeatype, nmeacrc;
    int i = 0, data_len = 0;
    memset(&item, 0, sizeof(gpsitem_s));
    if (len == 0)
    {
        return;
    }
    for (i = 0; i < len; i++)
    {
        if (str[i] == ',' || str[i] == '*')
        {
            item.item_cnt++;
            data_len = 0;
            if (item.item_cnt >= GPSITEMCNTMAX)
            {
                return ;
            }

        }
        else
        {
            item.item_data[item.item_cnt][data_len] = str[i];
            data_len++;
            if (i + 1 == len)
            {
                item.item_cnt++;
            }
            if (data_len >= GPSITEMSIZEMAX)
            {
                return ;
            }
        }
    }

    if (item.item_cnt == 0)
    {
        return;
    }
    nmeacrc = nemaCalcuateCrc((char *)str, len);
    if (nmeacrc == charstrToHexValue(item.item_data[item.item_cnt - 1]))
    {
        nmeatype = parseGetNmeaType(item.item_data[0]);
        switch (nmeatype)
        {
            case NMEA_RMC:
                parse_RMC(&item);
                break;
            case NMEA_GSA:
                parse_GSA(&item);
                break;
            case NMEA_GGA:
                parse_GGA(&item);
                break;
            case NMEA_GSV:
                parse_GSV(&item);
                break;
        }

    }
    else
    {
        LogMessage(DEBUG_ALL, "parseGPS==>Check CRC Error");
    }
}
/**************************************************
@bref		解析多条nmea数据
@param
	str
	len
@return

@note
**************************************************/

void nmeaParser(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    char onenmeadata[100];
    uint8_t foundnmeahead, restoreindex;
    foundnmeahead = 0;
    restoreindex = 0;

    if (sysinfo.nmeaOutput)
    {
        LogMessageWL(DEBUG_FACTORY, (char *) buf, len);
    }

    for (i = 0; i < len; i++)
    {
        if (buf[i] == '$')
        {
            foundnmeahead = 1;
            restoreindex = 0;
        }
        if (foundnmeahead == 1)
        {
            if (buf[i] == '\r')
            {
                foundnmeahead = 0;
                onenmeadata[restoreindex] = 0;
                parseGPS((uint8_t *)onenmeadata, restoreindex);
            }
            else
            {
                onenmeadata[restoreindex] = buf[i];
                restoreindex++;
                if (restoreindex >= 99)
                {
                    foundnmeahead = 0;
                }
            }
        }
    }

}

/**************************************************
@bref		将输入的时间按时区进行转换得到新时间
@param
	utctime			时间
	localtimezone	时区
@return
	转换后的时间
@note
**************************************************/

datetime_s changeUTCTimeToLocalTime(datetime_s utctime, int8_t localtimezone)
{
    datetime_s localtime;
    if (utctime.year == 0 || utctime.month == 0 || utctime.day == 0)
    {
        return utctime;
    }
    localtime.second = utctime.second;
    localtime.minute = utctime.minute;
    localtime.hour = utctime.hour + localtimezone;
    localtime.day = utctime.day;
    localtime.month = utctime.month;
    localtime.year = utctime.year;

    if (localtime.hour >= 24) //到了第二天的时间
    {
        localtime.hour = localtime.hour % 24;
        localtime.day += 1; //日期加1
        if (localtime.month == 4 || localtime.month == 6 || localtime.month == 9 || localtime.month == 11)
        {
            //30天
            if (localtime.day > 30)
            {
                localtime.day = 1;
                localtime.month += 1;
            }
        }
        else if (localtime.month == 2)//2月份
        {
            //28天或29天
            if ((((localtime.year + 2000) % 100 != 0) && ((localtime.year + 2000) % 4 == 0)) ||
                    ((localtime.year + 2000) % 400 == 0))//闰年
            {
                if (localtime.day > 29)
                {
                    localtime.day = 1;
                    localtime.month += 1;
                }
            }
            else
            {
                if (localtime.day > 28)
                {
                    localtime.day = 1;
                    localtime.month += 1;
                }
            }
        }
        else
        {
            //31天
            if (localtime.day > 31)
            {
                localtime.day = 1;
                if (localtime.month == 12)
                {
                    //需要跨年
                    localtime.month = 1;
                    localtime.year += 1;

                }
                else
                {
                    localtime.month += 1;
                }
            }

        }
    }
    else if (localtime.hour < 0) //前一天的时间
    {
        localtime.hour = localtime.hour + 24;
        localtime.day -= 1;
        if (localtime.day == 0)
        {
            localtime.month -= 1;
            if (localtime.month == 4 || localtime.month == 6 || localtime.month == 9 || localtime.month == 11)
            {
                //30天
                localtime.day = 30;

            }
            else if (localtime.month == 2)//2月份
            {
                //28天或29天
                if ((((localtime.year + 2000) % 100 != 0) && ((localtime.year + 2000) % 4 == 0)) ||
                        ((localtime.year + 2000) % 400 == 0))//闰年
                {
                    localtime.day = 29;
                }
                else
                {
                    localtime.day = 28;
                }
            }
            else
            {
                //31天
                localtime.day = 31;
                if (localtime.month == 0)
                {
                    localtime.month = 12;
                    localtime.year -= 1;
                }
            }
        }
    }
    return localtime;

}


/**************************************************
@bref		计算2个点的长度
@param
@return
@note
**************************************************/

static double lengthOfPoints(double lat1, double lng1, double lat2, double lng2)
{
    double radLng1, radLng2;
    double a, b, s;
    radLng1 = lng1 * PIA / 180.0;
    radLng2 = lng2 * PIA / 180.0;
    a = (radLng1 - radLng2);
    b = (lat1 - lat2) * PIA / 180.0;
    s = 2 * asin(sqrt(sin(a / 2) * sin(a / 2) + cos(radLng1) * cos(radLng2) * sin(b / 2) * sin(b / 2))) * 6378.137 * 1000;
    return s;
}
/**************************************************
@bref		初始化最后点
@param
@return
@note
**************************************************/

static void initLastPoint(gpsinfo_s *gpsinfo)
{
    lastuploadgpsinfo.datetime = gpsinfo->datetime;
    lastuploadgpsinfo.latitude = gpsinfo->latitude;
    lastuploadgpsinfo.longtitude = gpsinfo->longtitude;
    lastuploadgpsinfo.init = 1;
    LogMessage(DEBUG_ALL, "Update last position");
}

/**************************************************
@bref		自动围栏计算
@param
@return
@note		以最后上传的一个点作为围栏中心，超过围栏中心则上报位置
**************************************************/

static int8_t autoFenceCalculate(void)
{
    gpsinfo_s *gpsinfo;
    double distance;
    char debug[100];
    gpsinfo = getCurrentGPSInfo();
    if (gpsinfo->fixstatus == 0)
    {
        return -1;
    }
    if (lastuploadgpsinfo.init == 0)
    {
        initLastPoint(gpsinfo);
        return -2;
    }

    distance = lengthOfPoints(gpsinfo->latitude, gpsinfo->longtitude, lastuploadgpsinfo.latitude,
                              lastuploadgpsinfo.longtitude);
    sprintf(debug, "distance of point =%.2fm", distance);
    LogMessage(DEBUG_ALL, debug);
    if (distance >= sysparam.fence)
    {
        return 1;
    }
    return 0;
}

/**************************************************
@bref		拐弯计算
@param
@return
@note
**************************************************/

int8_t calculateTheGPSCornerPoint(void)
{
    gpsfifo_s *gpsfifo;
    uint16_t course[5];
    uint8_t positionidnex[5];
    uint16_t coursechange;
    uint8_t cur_index, i;


    gpsfifo = getGSPfifo();
    //获取当前最新的gps索引
    cur_index = gpsfifo->currentindex;
    for (i = 0; i < 5; i++)
    {
        //过滤操作
        if (gpsfifo->gpsinfohistory[cur_index].fixstatus == 0 || gpsfifo->gpsinfohistory[cur_index].speed < 6.5)
        {
            return -1;
        }
        if (gpsfifo->gpsinfohistory[cur_index].hadupload == 1)
        {
            return -2;
        }
        //保存5个点的方向
        course[i] = gpsfifo->gpsinfohistory[cur_index].course;
        positionidnex[i] = cur_index;
        //获取上一条索引
        cur_index = (cur_index + GPSFIFOSIZE - 1) % GPSFIFOSIZE;
    }
    //计算转弯角度
    coursechange = abs(course[0] - course[4]);
    if (coursechange > 180)
    {
        coursechange = 360 - coursechange;
    }
    LogPrintf(DEBUG_ALL, "Total course=%d", coursechange);
    if (coursechange >= 15 && coursechange <= 45)
    {
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[3]]);

        jt808SendToServer(TERMINAL_POSITION, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        jt808SendToServer(TERMINAL_POSITION, &gpsfifo->gpsinfohistory[positionidnex[3]]);
    }
    else if (coursechange > 45)
    {
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[0]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[2]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[3]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[4]]);

        jt808SendToServer(TERMINAL_POSITION, &gpsfifo->gpsinfohistory[positionidnex[0]]);
        jt808SendToServer(TERMINAL_POSITION, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        jt808SendToServer(TERMINAL_POSITION, &gpsfifo->gpsinfohistory[positionidnex[2]]);
        jt808SendToServer(TERMINAL_POSITION, &gpsfifo->gpsinfohistory[positionidnex[3]]);
        jt808SendToServer(TERMINAL_POSITION, &gpsfifo->gpsinfohistory[positionidnex[4]]);

    }
    return 1;
}

/**************************************************
@bref		轨迹上送
@param
@return
@note
**************************************************/

void gpsUploadPointToServer(void)
{
    static uint64_t tick = 0;

    if (sysinfo.gpsOnoff == 0 ||  sysparam.gpsuploadgap == 0 || sysparam.gpsuploadgap >= GPS_UPLOAD_GAP_MAX)
    {
        tick = 0;
        return;
    }

    tick++;
    calculateTheGPSCornerPoint();
    if (tick >= sysparam.gpsuploadgap)
    {
        if (autoFenceCalculate() == 1)
        {
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, getCurrentGPSInfo());
            //jt808BatchPushIn(getCurrentGPSInfo());
            jt808SendToServer(TERMINAL_POSITION, getCurrentGPSInfo());
            initLastPoint(getCurrentGPSInfo());
            tick = 0;
        }
    }
}


/**************************************************
@bref		通过0时区时间，更新本地时间
@param
	y		年
	m		月
	d		日
	hh		时
	mm		分
	ss		秒
	utc		本地时区
@return
@note
**************************************************/

void portUpdateLocalTime(uint8_t y, uint8_t m, uint8_t d, uint8_t hh, uint8_t mm, uint8_t ss, int8_t utc)
{
    datetime_s datetime, nd;
    y = y % 100;
    datetime.year = y;
    datetime.month = m;
    datetime.day = d;
    datetime.hour = hh;
    datetime.minute = mm;
    datetime.second = ss;
    nd = changeUTCTimeToLocalTime(datetime, utc);
    LogPrintf(DEBUG_ALL, "UTC 0==>%02d/%02d/%02d %02d:%02d:%02d", y + 2000, m, d, hh, mm, ss);
    portSetRtcDateTime(nd.year, nd.month, nd.day, nd.hour, nd.minute, nd.second, utc);
}

