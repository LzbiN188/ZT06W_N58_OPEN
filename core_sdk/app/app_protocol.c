#include "app_protocol.h"
#include "app_sys.h"
#include "app_instructioncmd.h"
#include "stdio.h"
#include "math.h"
#include "app_net.h"
#include "app_gps.h"
#include "app_port.h"
#include "app_param.h"
#include "app_task.h"
#include "nwy_file.h"
#include "app_jt808.h"

static protocol_s 		protocolInfo;
static gpsRestore_s		gpsres;
static audioDownload_s 	audiofile;
static upgradeInfo_s   	upgradeInfo;

static void upgradeChangeFsm(upgrade_fsm_e newfsm);
static void upgradeLoginReady(void);
static void upgradeDoing(void);


static const uint16_t ztvm_crctab16[] =
{
    0X0000, 0X1189, 0X2312, 0X329B, 0X4624, 0X57AD, 0X6536, 0X74BF,
    0X8C48, 0X9DC1, 0XAF5A, 0XBED3, 0XCA6C, 0XDBE5, 0XE97E, 0XF8F7,
    0X1081, 0X0108, 0X3393, 0X221A, 0X56A5, 0X472C, 0X75B7, 0X643E,
    0X9CC9, 0X8D40, 0XBFDB, 0XAE52, 0XDAED, 0XCB64, 0XF9FF, 0XE876,
    0X2102, 0X308B, 0X0210, 0X1399, 0X6726, 0X76AF, 0X4434, 0X55BD,
    0XAD4A, 0XBCC3, 0X8E58, 0X9FD1, 0XEB6E, 0XFAE7, 0XC87C, 0XD9F5,
    0X3183, 0X200A, 0X1291, 0X0318, 0X77A7, 0X662E, 0X54B5, 0X453C,
    0XBDCB, 0XAC42, 0X9ED9, 0X8F50, 0XFBEF, 0XEA66, 0XD8FD, 0XC974,
    0X4204, 0X538D, 0X6116, 0X709F, 0X0420, 0X15A9, 0X2732, 0X36BB,
    0XCE4C, 0XDFC5, 0XED5E, 0XFCD7, 0X8868, 0X99E1, 0XAB7A, 0XBAF3,
    0X5285, 0X430C, 0X7197, 0X601E, 0X14A1, 0X0528, 0X37B3, 0X263A,
    0XDECD, 0XCF44, 0XFDDF, 0XEC56, 0X98E9, 0X8960, 0XBBFB, 0XAA72,
    0X6306, 0X728F, 0X4014, 0X519D, 0X2522, 0X34AB, 0X0630, 0X17B9,
    0XEF4E, 0XFEC7, 0XCC5C, 0XDDD5, 0XA96A, 0XB8E3, 0X8A78, 0X9BF1,
    0X7387, 0X620E, 0X5095, 0X411C, 0X35A3, 0X242A, 0X16B1, 0X0738,
    0XFFCF, 0XEE46, 0XDCDD, 0XCD54, 0XB9EB, 0XA862, 0X9AF9, 0X8B70,
    0X8408, 0X9581, 0XA71A, 0XB693, 0XC22C, 0XD3A5, 0XE13E, 0XF0B7,
    0X0840, 0X19C9, 0X2B52, 0X3ADB, 0X4E64, 0X5FED, 0X6D76, 0X7CFF,
    0X9489, 0X8500, 0XB79B, 0XA612, 0XD2AD, 0XC324, 0XF1BF, 0XE036,
    0X18C1, 0X0948, 0X3BD3, 0X2A5A, 0X5EE5, 0X4F6C, 0X7DF7, 0X6C7E,
    0XA50A, 0XB483, 0X8618, 0X9791, 0XE32E, 0XF2A7, 0XC03C, 0XD1B5,
    0X2942, 0X38CB, 0X0A50, 0X1BD9, 0X6F66, 0X7EEF, 0X4C74, 0X5DFD,
    0XB58B, 0XA402, 0X9699, 0X8710, 0XF3AF, 0XE226, 0XD0BD, 0XC134,
    0X39C3, 0X284A, 0X1AD1, 0X0B58, 0X7FE7, 0X6E6E, 0X5CF5, 0X4D7C,
    0XC60C, 0XD785, 0XE51E, 0XF497, 0X8028, 0X91A1, 0XA33A, 0XB2B3,
    0X4A44, 0X5BCD, 0X6956, 0X78DF, 0X0C60, 0X1DE9, 0X2F72, 0X3EFB,
    0XD68D, 0XC704, 0XF59F, 0XE416, 0X90A9, 0X8120, 0XB3BB, 0XA232,
    0X5AC5, 0X4B4C, 0X79D7, 0X685E, 0X1CE1, 0X0D68, 0X3FF3, 0X2E7A,
    0XE70E, 0XF687, 0XC41C, 0XD595, 0XA12A, 0XB0A3, 0X8238, 0X93B1,
    0X6B46, 0X7ACF, 0X4854, 0X59DD, 0X2D62, 0X3CEB, 0X0E70, 0X1FF9,
    0XF78F, 0XE606, 0XD49D, 0XC514, 0XB1AB, 0XA022, 0X92B9, 0X8330,
    0X7BC7, 0X6A4E, 0X58D5, 0X495C, 0X3DE3, 0X2C6A, 0X1EF1, 0X0F78,
};

/*
0 bit:1==>����,0==>����
1 bit:1==>ACC ON ,0==>ACC OFF
2 bit:1==>���,0==>δ���
3 ~5 bit:
	000: 0 �ޱ���
	001��1 �𶯱���
	010��2 �ϵ籨��
	100��3 �͵籨��
	100��4 SOS����
	101��5 ���ű���
	110��6 ���ر���
	111��7 �йⱨ��

6 bit:1==>GPS��λ,0==>δ��λ
7 bit:1==>�͵��,0==>�͵�ͨ
*/

//0λ����������
void terminalDefense(void)
{
    protocolInfo.terminalStatus |= 0x01;
}
void terminalDisarm(void)
{
    protocolInfo.terminalStatus &= ~0x01;
}
//1λ��acc״̬
uint8_t getTerminalAccState(void)
{
    return (protocolInfo.terminalStatus & 0x02);

}
void terminalAccon(void)
{
    protocolInfo.terminalStatus |= 0x02;
    jt808UpdateStatus(JT808_STATUS_ACC, 1);
}
void terminalAccoff(void)
{
    protocolInfo.terminalStatus &= ~0x02;
    jt808UpdateStatus(JT808_STATUS_ACC, 0);
}
//2λ�������
void terminalCharge(void)
{
    protocolInfo.terminalStatus |= 0x04;
}

void terminalunCharge(void)
{
    protocolInfo.terminalStatus &= ~0x04;
}
uint8_t getTerminalChargeState(void)
{
    return (protocolInfo.terminalStatus & 0x04);
}
//3,4,5
void terminalAlarmSet(terminal_warnning_type_e alarm)
{
    protocolInfo.terminalStatus &= ~(0x38);
    protocolInfo.terminalStatus |= (alarm << 3);
}
//6λ��gps״̬
void terminalGPSFixed(void)
{
    protocolInfo.terminalStatus |= 0x40;
}
void terminalGPSUnFixed(void)
{
    protocolInfo.terminalStatus &= ~0x40;
}

/**************************************************
@bref		����80Э��ָ��ID
@param
@return
@note
**************************************************/

void saveInstructionId(void)
{
    memcpy(protocolInfo.instructionIdSave, protocolInfo.instructionId, 4);
}

/**************************************************
@bref		����ָ��ID��21  Э��
@param
@return
@note
**************************************************/

void recoverInstructionId(void)
{
    memcpy(protocolInfo.instructionId, protocolInfo.instructionIdSave, 4);
}

/**************************************************
@bref		��ȡָ��ID
@param
@return		����4���ֽ�ָ��ID
@note
**************************************************/

uint8_t *getProtoclInstructionid(void)
{
    return protocolInfo.instructionId;
}
/**************************************************
@bref		CRC16У�����
@param
	pData	��У������
	nLength	���ݳ���
@return		������
@note
**************************************************/
uint16_t GetCrc16(const char *pData, int nLength)
{
    uint16_t fcs = 0xffff;
    while (nLength > 0)
    {
        fcs = (fcs >> 8) ^ ztvm_crctab16[(fcs ^ (*pData)) & 0xff];
        nLength--;
        pData++;
    }
    return ~fcs;

}
/**************************************************
@bref		����Э��ͷ
@param
	dest			���Э������
	Protocol_type	Э������
@return
@note
**************************************************/
int createProtocolHead(char *dest, uint8_t Protocol_type)
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
/**************************************************
@bref		����7878Э��β
@param
	dest			���Э������
	h_b_len			���������Ѵ�ŵ����ݳ���
	serial_no		���к�
@return
@note
**************************************************/
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
/**************************************************
@bref		����7979Э��β
@param
	dest			���Э������
	h_b_len 		���������Ѵ�ŵ����ݳ���
	serial_no		���к�

@return
@note
**************************************************/

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

/**************************************************
@bref		IMEIת��BCD��
@param
	IMEI
@return
@note
**************************************************/
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

/**************************************************
@bref		������¼Э��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/
int createProtocol01(char *dest)
{
    int pdu_len;
    int ret;
    pdu_len = createProtocolHead(dest, 0x01);
    ret = packIMEI(protocolInfo.sn, dest + pdu_len);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len += ret;
    ret = createProtocolTail_78(dest, pdu_len,  protocolInfo.Serial);
    if (ret < 0)
    {
        return -2;
    }
    pdu_len = ret;
    return pdu_len;
}

/**************************************************
@bref		��������Э��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

int createProtocol13(char *dest)
{
    int pdu_len;
    int ret;
    uint16_t value;

    pdu_len = createProtocolHead(dest, 0x13);
    dest[pdu_len++] = protocolInfo.terminalStatus;

    value  = 0;

    value |= ((protocolInfo.beidouSatelliteUsed & 0x1F) << 10);
    value |= ((protocolInfo.gpsSatelliteUsed & 0x1F) << 5);
    value |= ((protocolInfo.rssi & 0x1F));
    value |= 0x8000;

    dest[pdu_len++] = (value >> 8) & 0xff;
    dest[pdu_len++] = value & 0xff;
    dest[pdu_len++ ] = 0;
    dest[pdu_len++ ] = 0;


    dest[pdu_len++ ] = protocolInfo.batteryLevel;//����


    dest[pdu_len++ ] = 0;


    if (protocolInfo.outsideVol < 3.0)
    {
        value = (uint16_t)(protocolInfo.batteryVol * 100);
    }
    else
    {
        value = (uint16_t)(protocolInfo.outsideVol * 100);
    }


    dest[pdu_len++ ] = (value >> 8) & 0xff; //��ѹ
    dest[pdu_len++ ] = value & 0xff;
    dest[pdu_len++ ] = 0;//�й�


    dest[pdu_len++ ] = (protocolInfo.startUpCnt >> 8) & 0xff; //ģʽһ����
    dest[pdu_len++ ] = protocolInfo.startUpCnt & 0xff;
    dest[pdu_len++ ] = (protocolInfo.runCnt >> 8) & 0xff; //ģʽ������
    dest[pdu_len++ ] = protocolInfo.runCnt & 0xff;


    ret = createProtocolTail_78(dest, pdu_len,  protocolInfo.Serial);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len = ret;
    return pdu_len;

}

/**************************************************
@bref		���gps�����Ϣ
@param
	gpsinfo		gps��Ϣ
	dest		Э��������
	protocol 	Э������
	gpsres		��¼gps��Ϣ�����ڴ洢
@return
	int		Э���ܳ���
@note
**************************************************/

static int protocolGPSpack(gpsinfo_s *gpsinfo, char *dest, int protocol, gpsRestore_s *gpsres)
{
    int pdu_len;
    unsigned long la;
    unsigned long lo;
    double f_la, f_lo;
    unsigned char speed, gps_viewstar, beidou_viewstar;
    int direc;

    datetime_s datetimenew;

    uint16_t year ;
    uint8_t  month, date, hour, minute, second;

    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);

    pdu_len = 0;
    la = lo = 0;
    /* UTC���ڣ�DDMMYY��ʽ */
    datetimenew = changeUTCTimeToLocalTime(gpsinfo->datetime, sysparam.utc);

    if (protocol == PROTOCOL_16)
    {
        dest[pdu_len++] = year % 100;
        dest[pdu_len++] = month;
        dest[pdu_len++] = date;
        dest[pdu_len++] = hour;
        dest[pdu_len++] = minute;
        dest[pdu_len++] = second;
    }
    else
    {
        dest[pdu_len++] = datetimenew.year % 100 ;
        dest[pdu_len++] = datetimenew.month;
        dest[pdu_len++] = datetimenew.day;
        dest[pdu_len++] = datetimenew.hour;
        dest[pdu_len++] = datetimenew.minute;
        dest[pdu_len++] = datetimenew.second;
    }
    gps_viewstar = protocolInfo.gpsSatelliteUsed;
    beidou_viewstar = protocolInfo.beidouSatelliteUsed;
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
        /* ǰ 4BitΪ GPS��Ϣ���ȣ��� 4BitΪ���붨λ�������� */
        if (gpsinfo->used_star   > 0)
        {
            dest[pdu_len++] = (12 << 4) | (gpsinfo->used_star & 0x0f);
        }
        else
        {
            /* ǰ 4BitΪ GPS��Ϣ���ȣ��� 4BitΪ���붨λ�������� */
            dest[pdu_len++] = (12 << 4) | (gpsinfo->gpsviewstart & 0x0f);
        }
    }
    /*
    γ��: ռ��4���ֽڣ���ʾ��λ���ݵ�γ��ֵ����ֵ��Χ0��162000000����ʾ0�ȵ�90�ȵķ�Χ����λ��1/500�룬ת���������£�
    ��GPSģ������ľ�γ��ֵת�����Է�Ϊ��λ��С����Ȼ���ٰ�ת�����С������30000������˵Ľ��ת����16���������ɡ�
    �� �� ��Ȼ��ת����ʮ��������Ϊ0x02 0x6B 0x3F 0x3E��
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
    ����:ռ��4���ֽڣ���ʾ��λ���ݵľ���ֵ����ֵ��Χ0��324000000����ʾ0�ȵ�180�ȵķ�Χ����λ��1/500�룬ת������
    ��γ�ȵ�ת������һ�¡�
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
    �ٶ�:ռ��1���ֽڣ���ʾGPS�������ٶȣ�ֵ��ΧΪ0x00��0xFF��ʾ��Χ0��255����/Сʱ��
     */
    speed = (unsigned char)(gpsinfo->speed);
    dest[pdu_len++] = speed;
    /*
    ����:ռ��2���ֽڣ���ʾGPS�����з��򣬱�ʾ��Χ0��360����λ���ȣ�������Ϊ0�ȣ�˳ʱ�롣
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
    /*0����γ 1����γ */
    if (gpsinfo->NS == 'N')
    {
        dest[pdu_len] |= 0x04 ; //0000 0100
    }
    /*0������ 1������*/
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

/**************************************************
@bref		�����վ��Ϣ
@param
	dest	Э��������
	gpsinfo gps��Ϣ
@return
	int		Э���ܳ���
@note
**************************************************/

static int protocolLBSPack(char *dest, gpsinfo_s *gpsinfo)
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

/**************************************************
@bref		����λ��Э��
@param
	gpsinfo	gps��Ϣ
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

static int createProtocol12(gpsinfo_s *gpsinfo, char *dest)
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
    gsm_level_value = protocolInfo.rssi;
    gsm_level_value |= 0x80;
    dest[pdu_len++] = gsm_level_value;
    dest[pdu_len++] = 0;
    /* Pack Tail */
    ret = createProtocolTail_78(dest, pdu_len, protocolInfo.Serial);
    if (ret <=  0)
    {
        return -3;
    }
    pdu_len = ret;
    return pdu_len;
}

/**************************************************
@bref		������վЭ��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

int createProtocol19(char *dest)
{
    int pdu_len;
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    pdu_len = createProtocolHead(dest, 0x19);
    dest[pdu_len++] = year % 100;
    dest[pdu_len++] = month;
    dest[pdu_len++] = date;
    dest[pdu_len++] = hour;
    dest[pdu_len++] = minute;
    dest[pdu_len++] = second;
    dest[pdu_len++] = protocolInfo.mcc >> 8;
    dest[pdu_len++] = protocolInfo.mcc;
    dest[pdu_len++] = protocolInfo.mnc;
    dest[pdu_len++] = 1;
    dest[pdu_len++] = protocolInfo.lac >> 8;
    dest[pdu_len++] = protocolInfo.lac;
    dest[pdu_len++] = protocolInfo.cid >> 24;
    dest[pdu_len++] = protocolInfo.cid >> 16;
    dest[pdu_len++] = protocolInfo.cid >> 8;
    dest[pdu_len++] = protocolInfo.cid;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    pdu_len = createProtocolTail_78(dest, pdu_len,  protocolInfo.Serial);
    return pdu_len;
}

/**************************************************
@bref		����WIFIЭ��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

int createProtocolF3(char *dest)
{
    uint8_t i, j;

    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    uint16_t pdu_len;
    pdu_len = createProtocolHead(dest, 0xF3);
    dest[pdu_len++] = year % 100;
    dest[pdu_len++] = month;
    dest[pdu_len++] = date;
    dest[pdu_len++] = hour;
    dest[pdu_len++] = minute;
    dest[pdu_len++] = second;
    dest[pdu_len++] = protocolInfo.wifiList.apcount;
    for (i = 0; i < protocolInfo.wifiList.apcount; i++)
    {
        dest[pdu_len++] = 0;
        dest[pdu_len++] = 0;
        for (j = 0; j < 6; j++)
        {
            dest[pdu_len++] = protocolInfo.wifiList.ap[i].ssid[j];
        }
    }
    pdu_len = createProtocolTail_78(dest, pdu_len,  protocolInfo.Serial);
    return pdu_len;
}

/**************************************************
@bref		��������Э��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

int createProtocol16(char *dest)
{
    int pdu_len, ret, i;
    gpsinfo_s *gpsinfo;
    pdu_len = createProtocolHead(dest, 0x16);
    gpsinfo = getLastFixedGPSInfo();
    ret = protocolGPSpack(gpsinfo, dest + pdu_len, PROTOCOL_16, NULL);
    pdu_len += ret;


    /**********************/

    dest[pdu_len++] = 0xFF;
    dest[pdu_len++] = (protocolInfo.mcc >> 8) & 0xff;
    dest[pdu_len++] = protocolInfo.mcc & 0xff;
    dest[pdu_len++] = protocolInfo.mnc;
    dest[pdu_len++] = (protocolInfo.lac >> 8) & 0xff;
    dest[pdu_len++] = protocolInfo.lac & 0xff;
    dest[pdu_len++] = (protocolInfo.cid >> 24) & 0xff;
    dest[pdu_len++] = (protocolInfo.cid >> 16) & 0xff;
    dest[pdu_len++] = (protocolInfo.cid >> 8) & 0xff;
    dest[pdu_len++] = protocolInfo.cid & 0xff;
    for (i = 0; i < 36; i++)
        dest[pdu_len++] = 0;

    /**********************/

    dest[pdu_len++] = protocolInfo.terminalStatus;
    protocolInfo.terminalStatus &= ~0x38;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = protocolInfo.event;
    if (protocolInfo.event == 0)
    {
        dest[pdu_len++] = 0x01;
    }
    else
    {
        dest[pdu_len++] = 0x81;
    }
    pdu_len = createProtocolTail_78(dest, pdu_len,  protocolInfo.Serial);
    return pdu_len;

}

/**************************************************
@bref		���������ظ�Э��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

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

/**************************************************
@bref		���������ظ�Э��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

static int createProtocol52(char *dest)
{
    int pdu_len = 0;
    int ret;
    int Serial;
    pdu_len = createProtocolHead(dest, 0x52);
    dest[pdu_len++] = (audiofile.audioCurPack >> 8) & 0xFF;
    dest[pdu_len++] = (audiofile.audioCurPack) & 0xFF;

    dest[pdu_len++] = (audiofile.audioId >> 24) & 0xFF;
    dest[pdu_len++] = (audiofile.audioId >> 16) & 0xFF;
    Serial = audiofile.audioId & 0xFFFF;
    ret = createProtocolTail_78(dest, pdu_len,  Serial);
    pdu_len = ret;
    return pdu_len;
}


/**************************************************
@bref		��������ȷ��Э��
@param
	dest	Э��������
@return
	int		Э���ܳ���
@note
**************************************************/

static int createProtocol53(char *dest)
{
    int pdu_len;
    int ret;
    int Serial;
    pdu_len = createProtocolHead(dest, 0x53);

    //Ӧ��
    dest[pdu_len++] = (audiofile.audioCnt >> 8) & 0xFF;
    dest[pdu_len++] = (audiofile.audioCnt) & 0xFF;
    //ʵ��
    dest[pdu_len++] = (audiofile.audioCnt >> 8) & 0xFF;
    dest[pdu_len++] = (audiofile.audioCnt) & 0xFF;
    //��ȱʧ������
    //�ļ��ܴ�С
    dest[pdu_len++] = (audiofile.audioSize >> 24) & 0xFF;
    dest[pdu_len++] = (audiofile.audioSize >> 16) & 0xFF;
    dest[pdu_len++] = (audiofile.audioSize >> 8) & 0xFF;
    dest[pdu_len++] = (audiofile.audioSize) & 0xFF;
    //��Ƶ��־ID
    dest[pdu_len++] = (audiofile.audioId >> 24) & 0xFF;
    dest[pdu_len++] = (audiofile.audioId >> 16) & 0xFF;
    Serial = audiofile.audioId & 0xFFFF;
    ret = createProtocolTail_78(dest, pdu_len,  Serial);
    pdu_len = ret;
    return pdu_len;
}


/**************************************************
@bref		����¼���ļ�����Э��
@param
	dest		Э��������
	datetime	¼��ʱ��ʱ������ yymmddhhmmss��ʽ
	totalsize	�ܳ��ȴ�С
	filetye		�ļ�����
	packsize	�ְ�����

@return
	int		Э���ܳ���
@note
**************************************************/

static int createProtocol61(char *dest, char *datetime, uint32_t totalsize, uint8_t filetye, uint16_t packsize)
{
    int pdu_len;
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
    pdu_len = createProtocolTail_78(dest, pdu_len, protocolInfo.Serial);
    return pdu_len;
}


/**************************************************
@bref		����¼���ļ�����Э��
@param
	dest		Э��������
	datetime	¼��ʱ��ʱ������ yymmddhhmmss��ʽ
	packnum		�ְ���
	recdata		��Ƶ����
	reclen		��Ƶ����

@return
	int		Э���ܳ���
@note
**************************************************/

static int createProtocol62(char *dest, char *datetime, uint16_t packnum, uint8_t *recdata, uint16_t reclen)
{
    int pdu_len, i;
    pdu_len = 0;
    dest[pdu_len++] = 0x79; //Э��ͷ
    dest[pdu_len++] = 0x79;
    dest[pdu_len++] = 0x00; //ָ��ȴ���
    dest[pdu_len++] = 0x00;
    dest[pdu_len++] = 0x62; //Э���
    changeHexStringToByteArray_10in((uint8_t *)dest + 5, (uint8_t *)datetime, 6);
    pdu_len += 6;
    dest[pdu_len++] = (packnum >> 8) & 0xff;
    dest[pdu_len++] = packnum & 0xff;
    for (i = 0; i < reclen; i++)
    {
        dest[pdu_len++] = recdata[i];
    }
    pdu_len = createProtocolTail_79(dest, pdu_len, protocolInfo.Serial);
    return pdu_len;
}

/**************************************************
@bref		��ȡ0ʱ��ʱ��
@param
	dest		Э��������
@return
@note
**************************************************/
static int createProtocol8A(char *dest)
{
    int pdu_len;
    int ret;
    pdu_len = createProtocolHead(dest, 0x8A);
    ret = createProtocolTail_78(dest, pdu_len,  protocolInfo.Serial);
    pdu_len = ret;
    return pdu_len;
}
/**************************************************
@bref		��Ϣ�ش�
@param
	dest		Э��������

@return
@note
**************************************************/
static int createProtocolF1(char *dest)
{
    int pdu_len;
    pdu_len = createProtocolHead(dest, 0xF1);
    sprintf(dest + pdu_len, "%s&&%s&&%s&&%s", protocolInfo.sn, protocolInfo.imsi, protocolInfo.iccid, protocolInfo.bleMac);
    pdu_len += strlen(dest + pdu_len);
    pdu_len = createProtocolTail_78(dest, pdu_len, protocolInfo.Serial);
    return pdu_len;
}

/**************************************************
@bref		ָ��͸���ظ�
@param
	dest		Э��������
	data		�ش�����
	datalen		���ݳ���

@return
@note
**************************************************/
static int createProtocol21(char *dest, char *data, uint16_t datalen)
{
    int pdu_len = 0, i;
    dest[pdu_len++] = 0x79; //Э��ͷ
    dest[pdu_len++] = 0x79;
    dest[pdu_len++] = 0x00; //ָ��ȴ���
    dest[pdu_len++] = 0x00;
    dest[pdu_len++] = 0x21; //Э���
    dest[pdu_len++] = protocolInfo.instructionId[0]; //ָ��ID
    dest[pdu_len++] = protocolInfo.instructionId[1];
    dest[pdu_len++] = protocolInfo.instructionId[2];
    dest[pdu_len++] = protocolInfo.instructionId[3];
    dest[pdu_len++] = 1; //���ݱ���
    for (i = 0; i < datalen; i++) //��������
    {
        dest[pdu_len++] = data[i];
    }
    pdu_len = createProtocolTail_79(dest, pdu_len, protocolInfo.Serial);
    return pdu_len;
}

/**************************************************
@bref		��������Э��
@param
	dest		Э��������
	cmd			��������
				1����ѯ�����ļ�
				2����ȡ�����ļ�
				3����֪�������

@return
@note
**************************************************/

static int createUpdateProtocol(char *dest, uint8_t cmd)
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
    //����
    dest[pdu_len++] = cmd;
    if (cmd == 0x01)
    {
        //SN�ų���
        dest[pdu_len++] = strlen(protocolInfo.sn);
        //����SN��
        memcpy(dest + pdu_len, protocolInfo.sn, strlen(protocolInfo.sn));
        pdu_len += strlen(protocolInfo.sn);
        //�汾�ų���
        dest[pdu_len++] = strlen(upgradeInfo.curCODEVERSION);
        //����SN��
        memcpy(dest + pdu_len, upgradeInfo.curCODEVERSION, strlen(upgradeInfo.curCODEVERSION));
        pdu_len += strlen(upgradeInfo.curCODEVERSION);
    }
    else if (cmd == 0x02)
    {
        dest[pdu_len++] = (upgradeInfo.file_id >> 24) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.file_id >> 16) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.file_id >> 8) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.file_id) & 0xFF;

        //�ļ�ƫ��λ��
        dest[pdu_len++] = (upgradeInfo.rxfileOffset >> 24) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.rxfileOffset >> 16) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.rxfileOffset >> 8) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.rxfileOffset) & 0xFF;

        readfilelen = upgradeInfo.file_totalsize - upgradeInfo.rxfileOffset; //�õ�ʣ��δ���մ�С
        if (readfilelen > upgradeInfo.file_len)
        {
            readfilelen = upgradeInfo.file_len;
        }
        //�ļ���ȡ����
        dest[pdu_len++] = (readfilelen >> 24) & 0xFF;
        dest[pdu_len++] = (readfilelen >> 16) & 0xFF;
        dest[pdu_len++] = (readfilelen >> 8) & 0xFF;
        dest[pdu_len++] = (readfilelen) & 0xFF;
    }
    else if (cmd == 0x03)
    {
        dest[pdu_len++] = upgradeInfo.updateOK;
        //SN�ų���
        dest[pdu_len++] = strlen(protocolInfo.sn);
        //����SN��
        memcpy(dest + pdu_len, protocolInfo.sn, strlen(protocolInfo.sn));
        pdu_len += strlen(protocolInfo.sn);

        dest[pdu_len++] = (upgradeInfo.file_id >> 24) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.file_id >> 16) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.file_id >> 8) & 0xFF;
        dest[pdu_len++] = (upgradeInfo.file_id) & 0xFF;

        //�汾�ų���
        dest[pdu_len++] = strlen(upgradeInfo.newCODEVERSION);
        //����SN��
        memcpy(dest + pdu_len, upgradeInfo.newCODEVERSION, strlen(upgradeInfo.newCODEVERSION));
        pdu_len += strlen(upgradeInfo.newCODEVERSION);
    }
    else
    {
        return 0;
    }
    pdu_len = createProtocolTail_79(dest, pdu_len, protocolInfo.Serial);
    return pdu_len;
}


/**************************************************
@bref		��ӡ����tcp����
@param
	txdata
	txlen
@return
@note
**************************************************/
static void sendTcpDataDebugShow(char *txdata, int txlen)
{
    int debuglen, dlen;
    char senddata[300];
    debuglen = txlen > 128 ? 128 : txlen;
    strcpy(senddata, "TCP Send: ");
    dlen = strlen(senddata);
    changeByteArrayToHexString((uint8_t *)txdata, (uint8_t *)senddata + dlen, (uint16_t) debuglen);
    senddata[dlen + debuglen * 2] = 0;
    LogMessage(DEBUG_ALL, senddata);

}
/**************************************************
@bref		TCP���ݷ���
@param
	link
	txdata
	txlen
@return
	1		���ͳɹ�
	0		����ʧ��
@note
**************************************************/

static int tcpSendData(uint8_t link, uint8_t *txdata, uint16_t txlen)
{
    int ret = 0;
    if (protocolInfo.tcpSend == NULL)
    {
        return ret;
    }
    ret = protocolInfo.tcpSend(link, txdata, txlen);
    ret = ret > 0 ? 1 : 0;
    return ret;
}
/**************************************************
@bref		����Э��
@param
	link	������·��0~5��
	type	Э������
	param	����
@return
@note
**************************************************/
void sendProtocolToServer(uint8_t link, int type, void *param)
{
    gpsinfo_s *gpsinfo = NULL;
    recordUploadInfo_s *recInfo = NULL;
    char txdata[512];
    char *debugP;
    int txlen = 0;

    if (sysparam.protocol != ZT_PROTOCOL_TYPE)
    {
        if (link == NORMAL_LINK)
        {
            return;
        }
    }

    debugP = txdata;

    switch (type)
    {
        case PROTOCOL_01:
            txlen = createProtocol01(txdata);
            break;
        case PROTOCOL_12:
            gpsinfo = (gpsinfo_s *)param;
            if (gpsinfo->fixstatus == 0 || gpsinfo->hadupload)
            {
                return;
            }
            txlen = createProtocol12(gpsinfo, txdata);
            gpsinfo->hadupload = 1;
            break;
        case PROTOCOL_13:
            txlen = createProtocol13(txdata);
            break;
        case PROTOCOL_16:
            txlen = createProtocol16(txdata);
            break;
        case PROTOCOL_19:
            txlen = createProtocol19(txdata);
            break;
        case PROTOCOL_51:
            txlen = createProtocol51(txdata);
            break;
        case PROTOCOL_52:
            txlen = createProtocol52(txdata);
            break;
        case PROTOCOL_53:
            txlen = createProtocol53(txdata);
            break;
        case PROTOCOL_61:
            recInfo = (recordUploadInfo_s *)param;
            txlen = createProtocol61(txdata, recInfo->dateTime, recInfo->totalSize, recInfo->fileType, recInfo->packSize);
            break;
        case PROTOCOL_62:
            recInfo = (recordUploadInfo_s *)param;
            if (recInfo == NULL ||  recInfo->dest == NULL)
            {
                LogMessage(DEBUG_ALL, "recInfo dest was null");
                return;
            }
            debugP = recInfo->dest;
            txlen = createProtocol62(recInfo->dest, recInfo->dateTime, recInfo->packNum, recInfo->recData, recInfo->recLen);
            break;

        case PROTOCOL_8A:
            txlen = createProtocol8A(txdata);
            break;
        case PROTOCOL_F1:
            txlen = createProtocolF1(txdata);
            break;
        case PROTOCOL_F3:
            txlen = createProtocolF3(txdata);
            break;
        case PROTOCOL_21:
            txlen = createProtocol21(txdata, (char *)param, strlen((char *)param));
            break;
        case PROTOCOL_UP:
            txlen = createUpdateProtocol(txdata, *(uint8_t *)param);
            break;
        default:
            return;
            break;
    }


    sendTcpDataDebugShow(debugP, txlen);



    switch (type)
    {
        case PROTOCOL_12:
            if (link == NORMAL_LINK)
            {
                if (serverIsReady() == 0 || tcpSendData(link, (uint8_t *)txdata, txlen) == 0)
                {
                    LogMessage(DEBUG_ALL, "gps send fail,save to file");
                    gpsRestoreWriteData(&gpsres,1);
                }
            }
            break;
        case PROTOCOL_61:
            tcpSendData(link, (uint8_t *)txdata, txlen);
            break;
        case PROTOCOL_62:
            tcpSendData(link, (uint8_t *)recInfo->dest, txlen);
            break;
        default:
            if (tcpSendData(link, (uint8_t *)txdata, txlen) == 0)
            {
                LogMessage(DEBUG_ALL, "TCP data send fail");
            }
            break;
    }
}
/**************************************************
@bref		������¼Э��
@param
@return
@note
**************************************************/
void protoclparser01(uint8_t link, char *protocol, int size)
{
    if (link == NORMAL_LINK)
    {
        serverLoginRespon();
    }
    else if (link == HIDE_LINK)
    {
        hiddenServLoginRespon();
    }
    else if (link == UPGRADE_LINK)
    {
        upgradeLoginReady();
    }
}
/**************************************************
@bref		��������Э��
@param
@return
@note
**************************************************/
void protoclparser13(uint8_t link, char *protocol, int size)
{
    LogMessage(DEBUG_ALL, "heartbeat respon");
}


/**************************************************
@bref		��������Э��
@param
@return
@note
**************************************************/
void protoclparser16(uint8_t link, char *protocol, int size)
{
    LogMessage(DEBUG_ALL, "alarm respon");
	alarmRequestClearSave();
}


/**************************************************
@bref		�����ļ��·�����
@param
@return
@note	78 78 0F 51 03 00 00 02 00 00 1B 00 00 00 4D 1C 55 D0 0D 0A
**************************************************/

static void protoclparser51(uint8_t link, char *protocol, int size)
{
    //�ļ�����
    audiofile.audioType = protocol[4];
    //�ְ���
    audiofile.audioCnt = protocol[6] << 8 | protocol[7];
    //�ļ���С
    audiofile.audioSize = protocol[8];
    audiofile.audioSize <<= 8;
    audiofile.audioSize |= protocol[9];
    audiofile.audioSize <<= 8;
    audiofile.audioSize |= protocol[10];
    audiofile.audioSize <<= 8;
    audiofile.audioSize |= protocol[11];
    //�ļ�ID
    audiofile.audioId = protocol[12];
    audiofile.audioId <<= 8;
    audiofile.audioId |= protocol[13];
    audiofile.audioId <<= 8;
    audiofile.audioId |= protocol[14];
    audiofile.audioId <<= 8;
    audiofile.audioId |= protocol[15];
    LogPrintf(DEBUG_ALL, "Type:%d,Cnt:%d,Size:%d,Id:%X", audiofile.audioType, audiofile.audioCnt, audiofile.audioSize,
              audiofile.audioId);
    portDeleteAudio();
    sendProtocolToServer(link, PROTOCOL_51, NULL);
}
/**************************************************
@bref		�����ļ��·���ʼ
@param
@return
@note	79 79 10 09 52 00 00 FF F3 68 C4 00 00 00
**************************************************/
static void protoclparser52(uint8_t link, char *protocol, int size)
{
    audiofile.audioCurPack = protocol[5] << 8 | protocol[6];
    portSaveAudio((uint8_t *)protocol + 7, size - 15);
    LogPrintf(DEBUG_ALL, "audio receive packId %d ,size %d Byte", audiofile.audioCurPack, size - 15);
    sendProtocolToServer(link, PROTOCOL_52, NULL);
    if ((audiofile.audioCurPack + 1) == audiofile.audioCnt)
    {
        LogMessage(DEBUG_ALL, "audio receive done");
        sendProtocolToServer(link, PROTOCOL_53, NULL);
        appSendThreadEvent(THREAD_EVENT_PLAY_AUDIO, 0);
    }
}

/**************************************************
@bref		����¼���ظ�Э��
@param
@return
@note
**************************************************/

static void protoclparser61(uint8_t link, char *protocol, int size)
{
    recUploadRsp();
}

/**************************************************
@bref		����¼���ظ�Э��
@param
@return
@note
**************************************************/

static void protoclparser62(uint8_t link, char *protocol, int size)
{
    recUploadRsp();
}

/**************************************************
@bref		����¼���ظ�Э��
@param
@return
@note
**************************************************/

static void protoclparser63(uint8_t link, char *protocol, int size)
{
    LogMessage(DEBUG_ALL, "record upoload done");
}

/**************************************************
@bref		����͸��Э��
@param
@return
@note
**************************************************/
static void protoclparser80(uint8_t link, char *protocol, int size)
{

    uint8_t instructionlen;
    instructionParam_s insParam;
    protocolInfo.instructionId[0] = protocol[5];
    protocolInfo.instructionId[1] = protocol[6];
    protocolInfo.instructionId[2] = protocol[7];
    protocolInfo.instructionId[3] = protocol[8];
    instructionlen = protocol[4] - 4;
    if (instructionlen + 17 != size)
    {
        LogPrintf(DEBUG_ALL, "protoclparser80==>%d,%d", instructionlen, size);
        return;
    }
    memset(&insParam, 0, sizeof(instructionParam_s));
    insParam.mode = NETWORK_MODE;
    insParam.link = link;
    instructionParser((uint8_t *)protocol + 9, instructionlen, &insParam);
}

/**************************************************
@bref		����ʱ��Э��
@param
@return
@note
**************************************************/

static void protoclparser8A(uint8_t link, char *protocol, int size)
{
    if (sysinfo.updateLocalTimeReq == 0)
    {
        return;
    }
    sysinfo.updateLocalTimeReq = 0;
    portUpdateLocalTime(protocol[4], protocol[5], protocol[6], protocol[7], protocol[8], protocol[9], sysparam.utc);
}

/**************************************************
@bref		��������Э��
@param
@return
@note

����01�ظ���79790042F3
01 ����
01 �Ƿ���Ҫ����
00000174  �ļ�ID
0004EB34  �ļ��ܴ�С
0F		  SN����
363930323137303931323030303237   SN��
0B		  ��ǰ�汾����
5632303230373134303031
16		  �°汾�ų���
4137575F353030333030303330385F3131355F34374B
000D   ���к�
AD3B   У��
0D0A   ����

����02�ظ���79790073F3
02 ����
01 ������Ч��־
00000000   �ļ�ƫ����ʼ
00000064   �ļ���С
474D3930312D3031303030344542314333374330304331457F454C4601010100000000000000000002002800010000006C090300E4E9040004EA04000200000534002000010028000700060038009FE510402DE924108FE2090080E002A300EB24109FE5
0003  ���к�
27C6  У��
0D0A  ����

**************************************************/

static void protocolParserUpdate(uint8_t link, char *protocol, int size)
{
    uint8_t cmd, snlen, myversionlen, newversionlen;
    uint16_t index, filecrc, calculatecrc;
    uint32_t rxfileoffset, rxfilelen;
    char *codedata;
    int ret = 0;
    cmd = protocol[5];
    if (cmd == 0x01)
    {
        //�ж��Ƿ��и����ļ�
        if (protocol[6] == 0x01)
        {
            upgradeInfo.file_id = (protocol[7] << 24 | protocol[8] << 16 | protocol[9] << 8 | protocol[10]);
            upgradeInfo.file_totalsize = (protocol[11] << 24 | protocol[12] << 16 | protocol[13] << 8 | protocol[14]);
            snlen = protocol[15];
            index = 16;
            if (snlen > (sizeof(upgradeInfo.rxsn) - 1))
            {
                LogPrintf(DEBUG_ALL, "Sn too long %d", snlen);
                return ;
            }
            strncpy(upgradeInfo.rxsn, (char *)&protocol[index], snlen);
            upgradeInfo.rxsn[snlen] = 0;
            index = 16 + snlen;
            myversionlen = protocol[index];
            index += 1;
            if (myversionlen > (sizeof(upgradeInfo.rxcurCODEVERSION) - 1))
            {
                LogPrintf(DEBUG_ALL, "myversion too long %d", myversionlen);
                return ;
            }
            strncpy(upgradeInfo.rxcurCODEVERSION, (char *)&protocol[index], myversionlen);
            upgradeInfo.rxcurCODEVERSION[myversionlen] = 0;
            index += myversionlen;
            newversionlen = protocol[index];
            index += 1;
            if (newversionlen > (sizeof(upgradeInfo.newCODEVERSION) - 1))
            {
                LogPrintf(DEBUG_ALL, "newversion too long %d", newversionlen);
                return ;
            }
            strncpy(upgradeInfo.newCODEVERSION, (char *)&protocol[index], newversionlen);
            upgradeInfo.newCODEVERSION[newversionlen] = 0;
            LogPrintf(DEBUG_ALL, "File %08X , Total size=%d Bytes", upgradeInfo.file_id, upgradeInfo.file_totalsize);
            LogPrintf(DEBUG_ALL, "My Version:%s", upgradeInfo.rxcurCODEVERSION);
            LogPrintf(DEBUG_ALL, "New Version:%s", upgradeInfo.newCODEVERSION);


            if (upgradeInfo.rxfileOffset != 0)
            {
                LogMessage(DEBUG_ALL, "continue to upgrade");
            }

            upgradeDoing();
        }
        else
        {
            LogMessage(DEBUG_ALL, "No update file");
            upgradeChangeFsm(NETWORK_DOWNLOAD_END);
        }
    }
    else if (cmd == 0x02)
    {
        if (protocol[6] == 1)
        {
            rxfileoffset = (protocol[7] << 24 | protocol[8] << 16 | protocol[9] << 8 | protocol[10]); //�ļ�ƫ��
            rxfilelen = (protocol[11] << 24 | protocol[12] << 16 | protocol[13] << 8 | protocol[14]); //�ļ���С
            calculatecrc = GetCrc16(protocol + 2, size - 6); //�ļ�У��
            filecrc = (*(protocol + 15 + rxfilelen + 2) << 8) | (*(protocol + 15 + rxfilelen + 2 + 1));
            if (rxfileoffset < upgradeInfo.rxfileOffset)
            {
                LogMessage(DEBUG_ALL, "Receive the same firmware");
                upgradeDoing();
                return ;
            }
            if (calculatecrc == filecrc)
            {
                upgradeInfo.waitTimeOutCnt = 0;
                upgradeInfo.validFailCnt = 0;
                LogMessage(DEBUG_ALL, "Data validation OK,Writting...");
                codedata = protocol + 15;

                //(void)codedata;
                //ota_pack.offset = rxfileoffset;
                //ota_pack.len = rxfilelen;
                //ota_pack.data = (uint8_t *)codedata;

                //ret = nwy_fota_dm(&ota_pack);
                //ret = nwy_fota_download_core(&ota_pack);
                ret = portUpgradeWirte(rxfileoffset, rxfilelen, (uint8_t *)codedata);
                if (ret)
                {
                    upgradeInfo.rxfileOffset = rxfileoffset + rxfilelen;
                    LogPrintf(DEBUG_ALL, ">>>>>>>>>> Completed progress %.1f%% <<<<<<<<<<",
                              ((float)upgradeInfo.rxfileOffset / upgradeInfo.file_totalsize) * 100);
                    if (upgradeInfo.rxfileOffset == upgradeInfo.file_totalsize)
                    {
                        upgradeInfo.updateOK = 1;
                        upgradeChangeFsm(NETWORK_DOWNLOAD_DONE);
                    }
                    else if (upgradeInfo.rxfileOffset > upgradeInfo.file_totalsize)
                    {
                        LogMessage(DEBUG_ALL, "Recevie complete ,but total size is different,retry again");
                        upgradeInfo.rxfileOffset = 0;
                        upgradeChangeFsm(NETWORK_LOGIN);
                    }
                    else
                    {
                        upgradeDoing();

                    }
                }
                else
                {
                    LogPrintf(DEBUG_ALL, "Writing firmware error at 0x%X", upgradeInfo.rxfileOffset);
                    upgradeChangeFsm(NETWORK_DOWNLOAD_ERROR);
                }

            }
            else
            {
                upgradeInfo.validFailCnt++;
                LogMessage(DEBUG_ALL, "Data validation Fail");
                upgradeDoing();
            }
        }
        else
        {
            LogMessage(DEBUG_ALL, "receive unknow protocol type in upgrade procedure");
            upgradeChangeFsm(NETWORK_DOWNLOAD_END);
        }
    }
}

/**************************************************
@bref		�������յ��ķ�����Э��
@param
@return
@note
**************************************************/
static void protocolParser(uint8_t link, char *protocol, int size)
{
    if (protocol[0] == 0X78 && protocol[1] == 0X78)
    {
        switch (protocol[3])
        {
            case (uint8_t)0x01:
                protoclparser01(link, protocol, size);
                break;
            case (uint8_t)0x13:
                protoclparser13(link, protocol, size);
                break;
            case (uint8_t)0x16:
                protoclparser16(link, protocol, size);
                break;
            case (uint8_t)0x80:
                protoclparser80(link, protocol, size);
                break;
            case (uint8_t)0x8A:
                protoclparser8A(link, protocol, size);
                break;
            case (uint8_t)0x61:
                protoclparser61(link, protocol, size);
                break;
            case (uint8_t)0x62:
                protoclparser62(link, protocol, size);
                break;
            case (uint8_t)0x63:
                protoclparser63(link, protocol, size);
                break;
            case (uint8_t)0x51:
                protoclparser51(link, protocol, size);
                break;
        }
    }
    else if (protocol[0] == 0X79 && protocol[1] == 0X79)
    {
        switch (protocol[4])
        {
            case (uint8_t)0xF3:
                protocolParserUpdate(link, protocol, size);
                break;
            case (uint8_t)0x52:
                protoclparser52(link, protocol, size);
                break;
        }
    }
}

/**************************************************
@bref		socket���ݽ��ջ�����
@param
@return
@note
**************************************************/
void socketRecvPush(uint8_t link, char *protocol, int size)
{
    static uint8_t dataBuf[PROTOCOL_BUFSZIE];
    static uint16_t dataBufLen = 0;
    uint16_t remain, i, contentlen, lastindex = 0, beginindex;
    remain = PROTOCOL_BUFSZIE - dataBufLen;
    if (remain == 0)
    {
        dataBufLen = 0;
        remain = PROTOCOL_BUFSZIE;
    }
    size = size > remain ? remain : size;
    memcpy(dataBuf + dataBufLen, protocol, size);
    dataBufLen += size;
    //������Ѱ��Э��ͷ
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
                protocolParser(link, (char *)dataBuf + beginindex, lastindex - beginindex);
            }
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
                //LogPrintf(DEBUG_ALL, "Fint it ====>Begin:7979[%d,%d]", beginindex, lastindex - beginindex);
                protocolParser(link, (char *)dataBuf + beginindex, lastindex - beginindex);
            }
        }
    }
    if (lastindex != 0)
    {
        remain = dataBufLen - lastindex;
        if (remain != 0)
        {
            memcpy(dataBuf, dataBuf + lastindex, remain);
        }
        dataBufLen = remain;
    }

}


/**************************************************
@bref		Э����س�ʼ��
@param
@return
@note
**************************************************/
void protocolInit(void)
{
    memset(&protocolInfo, 0, sizeof(protocol_s));
}

/**************************************************
@bref		����sn����¼ʱʹ��
@param
@return
@note
**************************************************/
void protoclUpdateSn(char *sn)
{
    strcpy(protocolInfo.sn, sn);
    LogPrintf(DEBUG_ALL, "Device Sn:%s", protocolInfo.sn);
}

/**************************************************
@bref		���µ�ѹ����أ�������Ϣ
@param
	outvol	����ѹ
	batvol	��ص�ѹ
	batlev	������Ϣ
@return
@note
**************************************************/

void protocolUpdateVol(float outvol, float batvol, uint8_t batlev)
{
    protocolInfo.outsideVol = outvol;
    protocolInfo.batteryVol = batvol;
    protocolInfo.batteryLevel = batlev;
}

/**************************************************
@bref		�����ź�ֵ
@param
	rssi	�ź�
@return
@note
**************************************************/

void protocolUpdateRssi(uint8_t rssi)
{
    protocolInfo.rssi = rssi;
}

/**************************************************
@bref		����gpsʹ������
@param
	rssi	�ź�
@return
@note
**************************************************/

void protocolUpdateSatelliteUsed(uint8_t gps, uint8_t bd)
{
    protocolInfo.gpsSatelliteUsed = gps;
    protocolInfo.beidouSatelliteUsed = bd;
}


/**************************************************
@bref		���»�վ��Ϣ
@param
	mcc
	mnc
	lac
	cid
@return
@note
**************************************************/

void protocolUpdateLbsInfo(uint16_t mcc, uint8_t mnc, uint16_t lac, uint32_t cid)
{
    protocolInfo.mcc = mcc;
    protocolInfo.mnc = mnc;
    protocolInfo.lac = lac;
    protocolInfo.cid = cid;
}

/**************************************************
@bref		����������ַ
@param
	mac     ����mac��ַ��12���ֽ�
@return
@note
**************************************************/

void protocolUpdateBleMac(char *mac)
{
    memcpy(protocolInfo.bleMac, mac, 12);
    protocolInfo.bleMac[12] = 0;
}

/**************************************************
@bref		����WIFI�б�
@param
	wifilist	wifiɨ����
@return
@note
**************************************************/

void protocolUpdateWifiList(wifiList_s *wifilist)
{
    if (wifilist == NULL)
        return;
    protocolInfo.wifiList = *wifilist;
}


/**************************************************
@bref		���±����¼�
@param
	event	�����¼�
@return
@note
**************************************************/

void protocolUpdateEvent(uint8_t event)
{
    protocolInfo.event = event;
}
/**************************************************
@bref		ע�����ݷ��ͺ���
@param
@return
@note
**************************************************/
void protocolRegisterTcpSend(int (*tcpSend)(uint8_t, uint8_t *, uint16_t))
{
    protocolInfo.tcpSend = tcpSend;
}


/*-------------------------------------------------------------------------------*/

/**************************************************
@bref		������ʼ��ʼ��
@param
@return
@note
**************************************************/

void upgradeStartInit(void)
{
    memset(&upgradeInfo, 0, sizeof(upgradeInfo_s));
    strcpy(upgradeInfo.curCODEVERSION, EEPROM_VERSION);
    upgradeInfo.file_len = 1300;
    sysinfo.upgradeDoing = 1;
    LogPrintf(DEBUG_ALL, "Current Version:%s", upgradeInfo.curCODEVERSION);
}

/**************************************************
@bref		�޸�����״̬��
@param
@return
@note
**************************************************/

static void upgradeChangeFsm(upgrade_fsm_e newfsm)
{
    upgradeInfo.upgradeFsm = newfsm;
    upgradeInfo.runTick = 0;
}

/**************************************************
@bref		��ȡ��������С
@param
@return
@note
**************************************************/

uint32_t upgradeGetTotaoSize(void)
{
    return upgradeInfo.file_totalsize;

}
/**************************************************
@bref		����ƽ̨��¼�ɹ�
@param
@return
@note
**************************************************/

static void upgradeLoginReady(void)
{
    upgradeInfo.loginCnt = 0;
    upgradeChangeFsm(NETWORK_LOGIN_READY);
}

/**************************************************
@bref		Զ������״̬��
@param
@return
@note
**************************************************/

void upgradeFromServer(void)
{
    uint8_t cmd;
    static uint32_t lastOffset = 0;
    switch (upgradeInfo.upgradeFsm)
    {
        //��¼
        case NETWORK_LOGIN:
            LogMessage(DEBUG_ALL, "login to upgrade server");
            sendProtocolToServer(UPGRADE_LINK, PROTOCOL_01, NULL);
            upgradeChangeFsm(NETWORK_LOGIN_WAIT);
            break;
        //��¼�ȴ�
        case NETWORK_LOGIN_WAIT:
            if (upgradeInfo.runTick++ >= 20)
            {
                upgradeChangeFsm(NETWORK_LOGIN);
                upgradeInfo.loginCnt++;
                if (upgradeInfo.loginCnt >= 3)
                {
                    upgradeInfo.loginCnt = 0;
                    LogMessage(DEBUG_ALL, "upgrade login fail");
                    upgradeChangeFsm(NETWORK_DOWNLOAD_END);
                }
            }
            break;
        case NETWORK_LOGIN_READY:
            //��¼���ȡ������汾��δ��ȡ��ÿ��30�����»�ȡ
            if (upgradeInfo.runTick % 30 == 0)
            {
                cmd = 1;
                sendProtocolToServer(UPGRADE_LINK, PROTOCOL_UP, &cmd);
                upgradeInfo.getVerCnt++;
                if (upgradeInfo.getVerCnt > 3)
                {
                    upgradeInfo.getVerCnt = 0;
                    LogMessage(DEBUG_ALL, "upgrade get version fail");
                    upgradeChangeFsm(NETWORK_DOWNLOAD_END);
                }
            }
            break;
        case NETWORK_DOWNLOAD_DOING:
            upgradeDoing();
            break;
        case NETWORK_DOWNLOAD_WAIT:
            //���¹̼����أ�����20��δ�յ����ݣ����·�������Э��
            LogPrintf(DEBUG_ALL, "Waitting firmware data...[%d]", upgradeInfo.runTick);
            if (upgradeInfo.runTick > 20)
            {
                upgradeInfo.waitTimeOutCnt++;
                if (upgradeInfo.waitTimeOutCnt >= 3)
                {
                    upgradeChangeFsm(NETWORK_DOWNLOAD_END);
                }
                else
                {
                    upgradeChangeFsm(NETWORK_DOWNLOAD_DOING);
                }
            }
            break;
        case NETWORK_DOWNLOAD_DONE:
            //����д�����
            LogMessage(DEBUG_ALL, "Download firmware complete!");
            cmd = 3;
            sendProtocolToServer(UPGRADE_LINK, PROTOCOL_UP, &cmd);
            upgradeChangeFsm(NETWORK_UPGRAD_NOW);
            LogMessage(DEBUG_ALL, "Wait upgrade");
            break;
        case NETWORK_UPGRAD_NOW:
            if (upgradeInfo.runTick < 3)
                break;
            LogMessage(DEBUG_ALL, "Start upgrade");
            portUpgradeStart();
            //�����ɹ�ʱ��ֱ�����������ɹ�ʱ���򷵻�
            LogMessage(DEBUG_ALL, "upgrade failed");
            upgradeInfo.updateOK = 0;
            cmd = 3;
            sendProtocolToServer(UPGRADE_LINK, PROTOCOL_UP, &cmd);
            upgradeChangeFsm(NETWORK_WAIT_JUMP);
            break;
        case NETWORK_DOWNLOAD_ERROR:

            LogMessage(DEBUG_ALL, "Download error");
            upgradeChangeFsm(NETWORK_DOWNLOAD_DOING);
            //ͬһ������������5��ʧ�ܣ���ֹͣ����
            if (lastOffset != upgradeInfo.rxfileOffset)
            {
                LogMessage(DEBUG_ALL, "Different offset");
                lastOffset = upgradeInfo.rxfileOffset;
                upgradeInfo.dlErrCnt = 0;
            }
            upgradeInfo.dlErrCnt++;
            if (upgradeInfo.dlErrCnt >= 5)
            {
                //����ʧ�ܳ������ʱ��ֹͣ����
                LogMessage(DEBUG_ALL, "Download end");
                upgradeChangeFsm(NETWORK_DOWNLOAD_END);
            }
            break;
        case NETWORK_WAIT_JUMP:
            if (upgradeInfo.runTick <= 3)
            {
                //���㹻ʱ�佫ʧ����Ϣ����ȥ
                return;
            }
            LogMessage(DEBUG_ALL, "Upgrade fail");
            upgradeChangeFsm(NETWORK_DOWNLOAD_END);
            break;
        case NETWORK_DOWNLOAD_END:
            //��������
            sysinfo.upgradeDoing = 0;
            break;
    }

    upgradeInfo.runTick++;
}

/**************************************************
@bref		����
@param
@return
@note
**************************************************/

static void upgradeDoing(void)
{
    uint8_t cmd;
    if (upgradeInfo.validFailCnt >= 3)
    {
        upgradeInfo.validFailCnt = 0;
        LogMessage(DEBUG_ALL, "valid fail too much time");
        upgradeChangeFsm(NETWORK_DOWNLOAD_END);
    }
    else
    {
        upgradeInfo.getVerCnt = 0;
        //��������Э��,������ȴ�״̬
        cmd = 2;
        sendProtocolToServer(UPGRADE_LINK, PROTOCOL_UP, &cmd);
        upgradeChangeFsm(NETWORK_DOWNLOAD_WAIT);
    }

}

/**************************************************
@bref		���ͻ���gps����
@param
	grs
	dest
	len
@return
@note
**************************************************/

static void gpsRestoreDataSend(gpsRestore_s *grs, char *dest	, uint16_t *len)
{
    int pdu_len;
    pdu_len = createProtocolHead(dest, 0x12);
    //ʱ���
    dest[pdu_len++] = grs->year ;
    dest[pdu_len++] = grs->month;
    dest[pdu_len++] = grs->day;
    dest[pdu_len++] = grs->hour;
    dest[pdu_len++] = grs->minute;
    dest[pdu_len++] = grs->second;
    //����
    dest[pdu_len++] = 0;
    //ά��
    dest[pdu_len++] = grs->latititude[0];
    dest[pdu_len++] = grs->latititude[1];
    dest[pdu_len++] = grs->latititude[2];
    dest[pdu_len++] = grs->latititude[3];
    //����
    dest[pdu_len++] = grs->longtitude[0];
    dest[pdu_len++] = grs->longtitude[1];
    dest[pdu_len++] = grs->longtitude[2];
    dest[pdu_len++] = grs->longtitude[3];
    //�ٶ�
    dest[pdu_len++] = grs->speed;
    //��λ��Ϣ
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
    //����λ
    dest[pdu_len++] = 1;
    pdu_len = createProtocolTail_78(dest, pdu_len, 0xFFFF);
    *len = pdu_len;
    sendTcpDataDebugShow(dest, pdu_len);
}

/**************************************************
@bref		���滺������
@param
	gpsres	��������
	num		��������
@return
@note
**************************************************/

void gpsRestoreWriteData(gpsRestore_s *gpsres , uint8_t num)
{
    uint16_t paramsize, fileOperation;
    int fd, writelen;
    paramsize = sizeof(gpsRestore_s)*num;
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
        LogMessage(DEBUG_ALL, "gpsSave==>Open error");
        return;
    }
    writelen = nwy_sdk_fwrite(fd, gpsres, paramsize);
    if (writelen != paramsize)
    {
        LogMessage(DEBUG_ALL, "gpsSave==>Error");
    }
    else
    {
        LogMessage(DEBUG_ALL, "gpsSave==>Success");
    }
    nwy_sdk_fclose(fd);
}


/**************************************************
@bref		��ȡ��������
@param
@return
@note
**************************************************/
uint8_t gpsRestoreReadData(void)
{
    static uint32_t readOffset = 0;
    int fd, readlen;
    uint8_t gpsget[400];
    char  dest[1200];
    uint8_t gpscount, i;
    uint16_t  fileOperation, destlen, protocollen=0;
    uint32_t  fileSize;
    gpsRestore_s *gpsinfo;

    if (protocolInfo.tcpSend == NULL)
    {
        return 0;
    }

    if (nwy_sdk_fexist(GPS_RESTORE_FILE_NAME) == true)
    {
        fileOperation = NWY_RDONLY;
    }
    else
    {
        readOffset = 0;
        LogPrintf(DEBUG_ALL, "No %s file", GPS_RESTORE_FILE_NAME);
        return 0;
    }
    fd = nwy_sdk_fopen(GPS_RESTORE_FILE_NAME, fileOperation);
    if (fd < 0)
    {
        LogMessage(DEBUG_ALL, "gpsRestoreReadData==>Open error");
        return 0;
    }
    fileSize = nwy_sdk_fsize_fd(fd);
    LogPrintf(DEBUG_ALL, "Total:%d,Offset:%d", fileSize, readOffset);
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
            gpsinfo = (gpsRestore_s *)(gpsget + (20 * i));
            if (sysparam.protocol == JT808_PROTOCOL_TYPE)
            {
            	//jt808BatchRestorePushIn(gpsinfo,1);
                protocollen = jt808gpsRestoreDataSend((uint8_t *)dest + destlen, gpsinfo);
            }
            else
            {
                gpsRestoreDataSend(gpsinfo, dest + destlen, &protocollen);
            }
            destlen += protocollen;
        }
        if (sysparam.protocol == JT808_PROTOCOL_TYPE)
        {
        	//jt808BatchPushOut();
            jt808TcpSend((uint8_t *)dest, destlen);
        }
        else
        {
            protocolInfo.tcpSend(NORMAL_LINK, (uint8_t *)dest, destlen);
        }
    }
    else
    {
        readlen = nwy_sdk_file_unlink(GPS_RESTORE_FILE_NAME);
        if (readlen == 0)
        {
            readOffset = 0;
            LogMessage(DEBUG_ALL, "Delete gps.save OK");
        }
        else
        {
            readOffset += 400;
            LogMessage(DEBUG_ALL, "Delete gps.save fail");
        }
    }
    return 1;
}

