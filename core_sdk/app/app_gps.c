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
@bref		��ӡgps��Ϣ
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
@bref		gps����
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
@bref		�����gps��Ϣ��������
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
    //���������������
    memcpy(&gpsfifo.gpsinfohistory[gpsfifo.currentindex], gpsinfo, sizeof(gpsinfo_s));
    //showgpsinfo();
}
/**************************************************
@bref		�����ǰgps
@param
@return
@note
**************************************************/

void gpsClearCurrentGPSInfo(void)
{
    memset(&gpsfifo.gpsinfohistory[gpsfifo.currentindex], 0, sizeof(gpsinfo_s));
}
/**************************************************
@bref		��ȡ����gps��Ϣ
@param
@return
@note
**************************************************/

gpsinfo_s *getCurrentGPSInfo(void)
{
    return &gpsfifo.gpsinfohistory[gpsfifo.currentindex];
}
/**************************************************
@bref		��ȡ���λ
@param
@return
@note
**************************************************/

gpsinfo_s *getLastFixedGPSInfo(void)
{
    return &gpsfifo.lastfixgpsinfo;
}
/**************************************************
@bref		��ȡgps��Ϣ����
@param
@return
@note
**************************************************/

gpsfifo_s *getGSPfifo(void)
{
    return &gpsfifo;
}



/**************************************************
@bref		ά��ת��
@param
	latitude		ԭʼά��
	Direction		����E/W
@return
	��ά��
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
    //����10��,�������һ��С����,�������
    tmpval = (unsigned long)(latitude * 100000.0);
    //��ȡ��ά��--��
    degree = tmpval / (100 * 100000);
    //��ȡ��ά��--��
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
@bref		����ת��
@param
	longitude		ԭʼ����
	Direction		����E/W
@return
	�¾���
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
<1> UTCʱ�䣬hhmmss��ʱ���룩��ʽ
<2> ��λ״̬��A=��Ч��λ��V=��Ч��λ
<3> γ��ddmm.mmmm���ȷ֣���ʽ��ǰ���0Ҳ�������䣩
<4> γ�Ȱ���N�������򣩻�S���ϰ���
<5> ����dddmm.mmmm���ȷ֣���ʽ��ǰ���0Ҳ�������䣩
<6> ���Ȱ���E����������W��������
<7> �������ʣ�000.0~999.9�ڣ�ǰ���0Ҳ�������䣩
<8> ���溽��000.0~359.9�ȣ����汱Ϊ�ο���׼��ǰ���0Ҳ�������䣩
<9> UTC���ڣ�ddmmyy�������꣩��ʽ
<10> ��ƫ�ǣ�000.0~180.0�ȣ�ǰ���0Ҳ�������䣩
<11> ��ƫ�Ƿ���E��������W������
<12> ģʽָʾ����NMEA0183 3.00�汾�����A=������λ��D=��֣�E=���㣬N=������Ч��
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
����$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A
�ֶ�0��$GPGSA�����ID�����������ΪGPS DOP and Active Satellites��GSA����ǰ������Ϣ
�ֶ�1����λģʽ��A=�Զ��ֶ�2D/3D��M=�ֶ�2D/3D
�ֶ�2����λ���ͣ�1=δ��λ��2=2D��λ��3=3D��λ
�ֶ�3��PRN�루α��������룩����1�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�4��PRN�루α��������룩����2�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�5��PRN�루α��������룩����3�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�6��PRN�루α��������룩����4�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�7��PRN�루α��������룩����5�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�8��PRN�루α��������룩����6�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�9��PRN�루α��������룩����7�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�10��PRN�루α��������룩����8�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�11��PRN�루α��������룩����9�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�12��PRN�루α��������룩����10�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�13��PRN�루α��������룩����11�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�14��PRN�루α��������룩����12�ŵ�����ʹ�õ�����PRN���ţ�00����ǰ��λ��������0��
�ֶ�15��PDOP�ۺ�λ�þ������ӣ�0.5 - 99.9��
�ֶ�16��HDOPˮƽ�������ӣ�0.5 - 99.9��
�ֶ�17��VDOP��ֱ�������ӣ�0.5 - 99.9��
�ֶ�18��У��ֵ
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
����$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70
�ֶ�0��$GPGSV�����ID�����������ΪGPS Satellites in View��GSV���ɼ�������Ϣ
�ֶ�1������GSV��������Ŀ��1 - 3��
�ֶ�2������GSV����Ǳ���GSV���ĵڼ�����1 - 3��
�ֶ�3����ǰ�ɼ�����������00 - 12����ǰ��λ��������0��
�ֶ�4��PRN �루α��������룩��01 - 32����ǰ��λ��������0��
�ֶ�5���������ǣ�00 - 90���ȣ�ǰ��λ��������0��
�ֶ�6�����Ƿ�λ�ǣ�00 - 359���ȣ�ǰ��λ��������0��
�ֶ�7������ȣ�00��99��dbHz
�ֶ�8��PRN �루α��������룩��01 - 32����ǰ��λ��������0��
�ֶ�9���������ǣ�00 - 90���ȣ�ǰ��λ��������0��
�ֶ�10�����Ƿ�λ�ǣ�00 - 359���ȣ�ǰ��λ��������0��
�ֶ�11������ȣ�00��99��dbHz
�ֶ�12��PRN �루α��������룩��01 - 32����ǰ��λ��������0��
�ֶ�13���������ǣ�00 - 90���ȣ�ǰ��λ��������0��
�ֶ�14�����Ƿ�λ�ǣ�00 - 359���ȣ�ǰ��λ��������0��
�ֶ�15������ȣ�00��99��dbHz
�ֶ�16��У��ֵ
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
����$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F
�ֶ�0��$GPGGA�����ID�����������ΪGlobal Positioning System Fix Data��GGA��GPS��λ��Ϣ
�ֶ�1��UTC ʱ�䣬hhmmss.sss��ʱ�����ʽ
�ֶ�2��γ��ddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
�ֶ�3��γ��N����γ����S����γ��
�ֶ�4������dddmm.mmmm���ȷָ�ʽ��ǰ��λ��������0��
�ֶ�5������E����������W��������
�ֶ�6��GPS״̬��0=δ��λ��1=�ǲ�ֶ�λ��2=��ֶ�λ��3=��ЧPPS��6=���ڹ���
�ֶ�7������ʹ�õ�����������00 - 12����ǰ��λ��������0��
�ֶ�8��HDOPˮƽ�������ӣ�0.5 - 99.9��
�ֶ�9�����θ߶ȣ�-9999.9 - 99999.9��
�ֶ�10��������������Դ��ˮ׼��ĸ߶�
�ֶ�11�����ʱ�䣨�����һ�ν��յ�����źſ�ʼ��������������ǲ�ֶ�λ��Ϊ�գ�
�ֶ�12�����վID��0000 - 1023��ǰ��λ��������0��������ǲ�ֶ�λ��Ϊ�գ�
�ֶ�13��У��ֵ
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
@bref		����nmea����
@param
	str
@return
	�¾���
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
@bref		����У��
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
@bref		charת������ֵ
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
@bref		char�ַ���ת��Ϊ��ֵ
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
@bref		����һ��nmea����
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
@bref		��������nmea����
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
@bref		�������ʱ�䰴ʱ������ת���õ���ʱ��
@param
	utctime			ʱ��
	localtimezone	ʱ��
@return
	ת�����ʱ��
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

    if (localtime.hour >= 24) //���˵ڶ����ʱ��
    {
        localtime.hour = localtime.hour % 24;
        localtime.day += 1; //���ڼ�1
        if (localtime.month == 4 || localtime.month == 6 || localtime.month == 9 || localtime.month == 11)
        {
            //30��
            if (localtime.day > 30)
            {
                localtime.day = 1;
                localtime.month += 1;
            }
        }
        else if (localtime.month == 2)//2�·�
        {
            //28���29��
            if ((((localtime.year + 2000) % 100 != 0) && ((localtime.year + 2000) % 4 == 0)) ||
                    ((localtime.year + 2000) % 400 == 0))//����
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
            //31��
            if (localtime.day > 31)
            {
                localtime.day = 1;
                if (localtime.month == 12)
                {
                    //��Ҫ����
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
    else if (localtime.hour < 0) //ǰһ���ʱ��
    {
        localtime.hour = localtime.hour + 24;
        localtime.day -= 1;
        if (localtime.day == 0)
        {
            localtime.month -= 1;
            if (localtime.month == 4 || localtime.month == 6 || localtime.month == 9 || localtime.month == 11)
            {
                //30��
                localtime.day = 30;

            }
            else if (localtime.month == 2)//2�·�
            {
                //28���29��
                if ((((localtime.year + 2000) % 100 != 0) && ((localtime.year + 2000) % 4 == 0)) ||
                        ((localtime.year + 2000) % 400 == 0))//����
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
                //31��
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
@bref		����2����ĳ���
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
@bref		��ʼ������
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
@bref		�Զ�Χ������
@param
@return
@note		������ϴ���һ������ΪΧ�����ģ�����Χ���������ϱ�λ��
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
@bref		�������
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
    //��ȡ��ǰ���µ�gps����
    cur_index = gpsfifo->currentindex;
    for (i = 0; i < 5; i++)
    {
        //���˲���
        if (gpsfifo->gpsinfohistory[cur_index].fixstatus == 0 || gpsfifo->gpsinfohistory[cur_index].speed < 6.5)
        {
            return -1;
        }
        if (gpsfifo->gpsinfohistory[cur_index].hadupload == 1)
        {
            return -2;
        }
        //����5����ķ���
        course[i] = gpsfifo->gpsinfohistory[cur_index].course;
        positionidnex[i] = cur_index;
        //��ȡ��һ������
        cur_index = (cur_index + GPSFIFOSIZE - 1) % GPSFIFOSIZE;
    }
    //����ת��Ƕ�
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
@bref		�켣����
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
@bref		ͨ��0ʱ��ʱ�䣬���±���ʱ��
@param
	y		��
	m		��
	d		��
	hh		ʱ
	mm		��
	ss		��
	utc		����ʱ��
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

