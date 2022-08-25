#include "app_gps.h"
#include "app_port.h"
#include "app_sys.h"
#include "app_task.h"
#include "app_protocol.h"
#include "app_protocol_808.h"
#include "app_param.h"
#include "math.h"
#include "app_customercmd.h"
#define PIA 3.1415926

static GPSINFO gpsinfonow;
static GPSFIFO gpsfifo;
static LASTUPLOADGPSINFO lastuploadgpsinfo;


static void addGpsInfo(GPSINFO *gpsinfo);

/*计算NMEA 的CRC校验*/
static unsigned char nemaCalcuateCrc(char *str, int len)
{
    int i, index, size;
    unsigned char crc;
    crc = str[1];
    index = getCharIndex((uint8_t *)str, len, '*');
    size = len - index;
    for (i = 2; i < len - size; i++)
    {
        crc ^= str[i];
    }
    return crc;
}
static unsigned char chartohexcharvalue(char value)
{
    if (value >= '0' && value <= '9')
        return value - '0';
    if (value >= 'a' && value <= 'z')
        return value - 'a' + 10;
    if (value >= 'A' && value <= 'Z')
        return value - 'A' + 10;
    return 0;
}

/*将字符串的16进制转换为数值*/
unsigned int charstrToHexValue(char *value)
{
    unsigned int calvalue = 0;
    unsigned char i, j, len = strlen(value);
    j = 0;
    j = len;
    for (i = 0; i < len; i++)
    {
        value[i] = chartohexcharvalue(value[i]);
        calvalue += value[i] * pow(16, j - 1);
        j--;
    }
    return calvalue;
}

void showgpsinfo(void)
{
    char debug[300];
    uint8_t i, total;
    GPSINFO *gpsinfo;
    gpsinfo = &gpsinfonow;
    sprintf(debug, "NMEA:%d/%d/%d  %02d:%02d:%02d;", gpsinfo->datetime.year, gpsinfo->datetime.month, gpsinfo->datetime.day,
            gpsinfo->datetime.hour, gpsinfo->datetime.minute, gpsinfo->datetime.second);
    sprintf(debug + strlen(debug), "%c %f %c %f;\n", gpsinfo->NS, gpsinfo->latitude, gpsinfo->EW, gpsinfo->longtitude);
    sprintf(debug + strlen(debug), "Speed=%.2fKm/h;", gpsinfo->speed);
    sprintf(debug + strlen(debug), "%s;", gpsinfo->fixstatus ? "FIXED" : "Invalid");
    sprintf(debug + strlen(debug), "PDOP=%.2f;fixmode=%d;", gpsinfo->pdop, gpsinfo->fixmode);
    sprintf(debug + strlen(debug), "GPS View=%d;Beidou View=%d;", gpsinfo->gpsviewstart, gpsinfo->beidouviewstart);
    sprintf(debug + strlen(debug), "Use Star=%d;", gpsinfo->used_star);
    sprintf(debug + strlen(debug), "GPS CN:");
    total = sizeof(gpsinfo->gpsCn);
    for (i = 0; i < total; i++)
    {
        if (gpsinfo->gpsCn[i] != 0)
        {
            sprintf(debug + strlen(debug), "%d;", gpsinfo->gpsCn[i]);
        }
    }
    sprintf(debug + strlen(debug), "BeiDou CN:");
    total = sizeof(gpsinfo->beidouCn);
    for (i = 0; i < total; i++)
    {
        if (gpsinfo->beidouCn[i] != 0)
        {
            sprintf(debug + strlen(debug), "%d;", gpsinfo->beidouCn[i]);
        }
    }
    sprintf(debug + strlen(debug), "\n");
    LogMessage(DEBUG_ALL, debug);
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
void parse_RMC(GPSITEM *item)
{
    GPSINFO *gpsinfo;
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
    gpsinfo->gpsticksec = gpsinfo->datetime.hour * 3600 + gpsinfo->datetime.minute * 60 + gpsinfo->datetime.second;
    //sysinfo.gpsUpdatetick = sysinfo.System_Tick;
    addGpsInfo(gpsinfo);
    memset(gpsinfo, 0, sizeof(GPSINFO));
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
void parse_GSA(GPSITEM *item)
{
    GPSINFO *gpsinfo;
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
void parse_GSV(GPSITEM *item)
{
    GPSINFO *gpsinfo;
    uint8_t gpskind = 0, currentpage;
    static uint8_t count = 0;
    gpsinfo = &gpsinfonow;

    jt808UpdateStatus(JT808_STATUS_USE_GPS, 0);
    jt808UpdateStatus(JT808_STATUS_USE_BEIDOU, 0);
    jt808UpdateStatus(JT808_STATUS_USE_GLONASS, 0);
    jt808UpdateStatus(JT808_STATUS_USE_GALILEO, 0);

    if (my_strpach(item->item_data[0], (const char *)"$GP"))
    {
        jt808UpdateStatus(JT808_STATUS_USE_GPS, 1);
        gpskind = 0;
    }
    else if (my_strpach(item->item_data[0], (const char *)"$GB"))
    {
        jt808UpdateStatus(JT808_STATUS_USE_BEIDOU, 1);
        gpskind = 1;
    }
    else if (my_strpach(item->item_data[0], (const char *)"$BD"))
    {
        jt808UpdateStatus(JT808_STATUS_USE_BEIDOU, 1);
        gpskind = 1;
    }
    else if (my_strpach(item->item_data[0], (const char *)"$GL"))
    {
        jt808UpdateStatus(JT808_STATUS_USE_GLONASS, 1);
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
void parse_GGA(GPSITEM *item)
{
    GPSINFO *gpsinfo;
    gpsinfo = &gpsinfonow;
    if (item->item_data[7][0] != 0)
    {
        gpsinfo->used_star = atoi(item->item_data[7]);
    }
}
static unsigned char parseGetNmeaType(char *str)
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

/*将单独一条数据分解成一个个ITEM项*/
void parseGPS(uint8_t *str, uint16_t len)
{
    GPSITEM item;
    char saveBuf[128];
    unsigned char nmeacrc, nmeatype;
    int i = 0, data_len = 0, index;
    memset(&item, 0, sizeof(GPSITEM));
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
            if (data_len >= GPSITEMSIZEMAX)
            {
                return ;
            }
        }
    }
    index = getCharIndex((uint8_t *)str, len, '*');
    memcpy(item.item_data[item.item_cnt], str + index + 1, len - index - 1);
    nmeacrc = nemaCalcuateCrc((char *)str, len);
    if (nmeacrc == charstrToHexValue(item.item_data[item.item_cnt]))
    {
        nmeatype = parseGetNmeaType(item.item_data[0]);
        memcpy(saveBuf, str, len);
        saveBuf[len] = 0;
        strcat(saveBuf, "\r\n");

        if (nmeatype == NMEA_RMC || nmeatype == NMEA_GSA || nmeatype == NMEA_GGA || sysinfo.nmeaoutputonoff)
        {
            customGpsInPut(saveBuf, strlen(saveBuf));
        }
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
        LogMessage(DEBUG_ALL, "parseGPS==>Check CRC Error\n");
    }
}
/*
$GNRMC,091602.00,A,2303.49865,N,11322.83066,E,0.026,,201219,,,A,V*19
$GNVTG,,T,,M,0.026,N,0.048,K,A*35
$GNGGA,091602.00,2303.49865,N,11322.83066,E,1,12,0.94,3.7,M,-5.2,M,,*52
$GNGSA,M,3,22,14,03,32,16,27,29,,,,,,1.70,0.94,1.41,1*0C
$GNGSA,M,3,05,24,25,09,,,,,,,,,1.70,0.94,1.41,3*0D
$GNGSA,M,3,16,06,07,10,01,02,21,24,29,03,,,1.70,0.94,1.41,4*0E
$GPGSV,3,1,11,03,26,287,34,04,16,321,,14,63,117,43,16,65,257,41,0*68
$GPGSV,3,2,11,22,29,263,41,23,11,320,19,26,70,344,,27,24,181,19,0*66
$GPGSV,3,3,11,29,16,048,18,31,44,039,,32,34,135,40,0*51
$GAGSV,3,1,10,02,07,247,,03,26,319,,05,76,276,38,09,43,157,40,0*7A
$GAGSV,3,2,10,11,09,148,,12,10,098,,18,44,111,44,24,50,013,14,0*7F
$GAGSV,3,3,10,25,49,281,40,31,09,048,,0*77
$GBGSV,4,1,16,01,50,128,36,02,47,234,35,03,62,187,39,04,32,111,33,0*7A
$GBGSV,4,2,16,06,75,120,37,07,74,329,16,08,00,171,,09,74,343,,0*7F
$GBGSV,4,3,16,10,58,260,33,14,33,301,,16,79,114,34,21,41,191,40,0*79
*/
void nmeaParse(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    char onenmeadata[100];
    uint8_t foundnmeahead, restoreindex;
    foundnmeahead = 0;
    restoreindex = 0;
    if (sysinfo.nmeaoutputonoff)
    {
        //LogMessageWL(DEBUG_FACTORY, (char *) buf, len);
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
/*
维度转化
*/
double latitude_to_double(double latitude, char Direction)
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

/*
经度转化
*/
double longitude_to_double(double longitude, char Direction)
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


/*------------------------------------------------------*/

//时区转换
DATETIME changeUTCTimeToLocalTime(DATETIME utctime, int8_t localtimezone)
{
    DATETIME localtime;
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
void updateLocalRTCTime(DATETIME *datetime)
{
    DATETIME localtime;
    localtime = changeUTCTimeToLocalTime(*datetime, sysparam.utc);
    updateRTCdatetime(localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);

    if (sysparam.MODE == MODE1 || sysparam.MODE == MODE21)
    {
        setNextAlarmTime();
    }
    else if (sysparam.MODE == MODE3 || sysparam.MODE == MODE23)
    {
        setNextWakeUpTime();
    }
};

/*------------------------------------------------------*/
/*------------------------------------------------------*/
//返回0则过滤
static uint8_t  gpsFilter(GPSINFO *gpsinfo)
{
    if (gpsinfo->fixstatus == 0)
        return 0;
    return 1;
}
static void addGpsInfo(GPSINFO *gpsinfo)
{

    if (gpsinfo->fixstatus)
    {

        sysinfo.gpsLastFixedTick = sysinfo.System_Tick;
        updateSystemLedStatus(SYSTEM_LED_GPSOK, 1);
        if (sysinfo.localrtchadupdate == 0)
        {
            if (gpsinfo->datetime.year != 0 && gpsinfo->datetime.month != 0 && gpsinfo->datetime.day != 0)
            {
                sysinfo.localrtchadupdate = 1;
                updateLocalRTCTime(&gpsinfo->datetime);
            }
        }
    }
    else
    {
        updateSystemLedStatus(SYSTEM_LED_GPSOK, 0);
    }


    //将添加进来的数据复制到队列中
    if (gpsFilter(gpsinfo) == 0)
    {
        gpsinfo->fixstatus = 0;
    }
    else
    {
        memcpy(&gpsfifo.lastfixgpsinfo, gpsinfo, sizeof(GPSINFO));
    }

    //    if (gpsinfo->gpsticksec == gpsfifo.gpsinfohistory[gpsfifo.currentindex].gpsticksec)
    //    {
    //        //当前GPS数据与队列中最新的数据时间一样时，将新的经纬度替换为最新的数据，队列不新增
    //        //往队列中添加数据
    //        memcpy(&gpsfifo.gpsinfohistory[gpsfifo.currentindex], gpsinfo, sizeof(GPSINFO));
    //        LogMessage(DEBUG_ALL, "replace the last gps data\n");
    //    }
    //    else
    //时间不同，队列新增
    gpsfifo.currentindex = (gpsfifo.currentindex + 1) % GPSFIFOSIZE;
    //往队列中添加数据
    memcpy(&gpsfifo.gpsinfohistory[gpsfifo.currentindex], gpsinfo, sizeof(GPSINFO));
    showgpsinfo();
    sysinfo.gpsUpdatetick = sysinfo.System_Tick;
}

void gpsClearCurrentGPSInfo(void)
{
    memset(&gpsfifo.gpsinfohistory[gpsfifo.currentindex],0,sizeof(GPSINFO));
    updateSystemLedStatus(SYSTEM_LED_GPSOK, 0);
}
GPSINFO *getCurrentGPSInfo(void)
{
    return &gpsfifo.gpsinfohistory[gpsfifo.currentindex];
}

GPSINFO *getLastFixedGPSInfo(void)
{
    return &gpsfifo.lastfixgpsinfo;
}

GPSFIFO *getGSPfifo(void)
{
    return &gpsfifo;
}
/*------------------------------------------------------*/
/*------------------------------------------------------*/
/*------------------------------------------------------*/
double calculateTheDistanceBetweenTwoPonits(double lat1, double lng1, double lat2, double lng2)
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

static void initLastPoint(GPSINFO *gpsinfo)
{
    lastuploadgpsinfo.datetime = gpsinfo->datetime;
    lastuploadgpsinfo.latitude = gpsinfo->latitude;
    lastuploadgpsinfo.longtitude = gpsinfo->longtitude;
    lastuploadgpsinfo.gpsticksec = gpsinfo->gpsticksec;
    lastuploadgpsinfo.init = 1;
    LogMessage(DEBUG_ALL, "Update last position\n");
}

//static uint8_t isStepHasUpdate(void)
//{
//    static uint16_t clastStep = 0;
//    if (sysparam.step == 0)
//    {
//        return 0;
//    }
//    if (sysinfo.stepCount == 0)
//    {
//        clastStep = 0;
//    }
//    if (sysinfo.stepCount != clastStep)
//    {
//        if (sysinfo.stepCount > (clastStep + sysparam.step))
//        {
//            clastStep = sysinfo.stepCount;
//			LogMessage(DEBUG_ALL,"isStepHasUpdate==>Step update\n");
//			return 1;
//        }
//    }
//    return 0;
//}

static int8_t calculateDistanceOfPoint(void)
{
    GPSINFO *gpsinfo;
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

    distance = calculateTheDistanceBetweenTwoPonits(gpsinfo->latitude, gpsinfo->longtitude, lastuploadgpsinfo.latitude,
               lastuploadgpsinfo.longtitude);
    sprintf(debug, "distance of point =%.2fm\n", distance);
    LogMessage(DEBUG_ALL, debug);
    if (distance > sysparam.fence)
    {
        return 1;
    }
    return 0;
}

static int8_t calculateTheGPSCornerPoint(void)
{
    GPSFIFO *gpsfifo;
    GPSINFO *gpscurrent;
    char debug[30];
    uint16_t course[5];
    uint8_t positionidnex[5];
    uint16_t coursechange;
    uint8_t cur_index, i;

    //    if (sysparam.turnalg == 0)
    //    {
    //        return 0;
    //    }

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
    sprintf(debug, "Total course=%d\n", coursechange);
    LogMessage(DEBUG_ALL, debug);
    if (coursechange >= 15 && coursechange <= 45)
    {
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[3]]);
    }
    else if (coursechange > 45)
    {
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[0]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[2]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[3]]);
        sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[4]]);

        gpscurrent = getCurrentGPSInfo();
        if (gpsfifo->gpsinfohistory[positionidnex[0]].gpsticksec == gpscurrent->gpsticksec)
        {
            gpscurrent->hadupload = 1;
        }
    }
    return 1;
}

void gpsUploadPointToServer(void)
{
    static uint64_t tick = 0;
    if (sysinfo.GPSStatus == 0)
    {
        return ;
    }
    tick++;
    calculateTheGPSCornerPoint();
    if (tick >= sysparam.gpsuploadgap)
    {
        if (calculateDistanceOfPoint() == 1)
        {
            sendProtocolToServer(NORMAL_LINK, PROTOCOL_12, getCurrentGPSInfo());
            initLastPoint(getCurrentGPSInfo());
            tick = 0;
        }
    }
}


