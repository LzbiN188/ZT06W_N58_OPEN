#include "app_sys.h"
#include "nwy_usb_serial.h"

#define LOGPRINTF_MAX	2048

SYSTEM_INFO sysinfo;


void usbSend(char *data, uint16_t len)
{
    uint16_t actualen = 0;
    uint16_t totalen = 0;
    while (totalen < len)
    {
        actualen = nwy_usb_serial_send(data + totalen, len - totalen);
        if (actualen <= 0)
            break;
        totalen += actualen;
    }
}



void LogPrintf(uint8_t level, const char *debug, ...)
{
    static char LOGDEBUG[LOGPRINTF_MAX];
    va_list args;
    va_start(args, debug);
    vsnprintf(LOGDEBUG, LOGPRINTF_MAX, debug, args);
    va_end(args);
    LogMessageWL(level, LOGDEBUG, strlen(LOGDEBUG));
}


void LogMessage(uint8_t level, char *debug)
{
    uint16_t year = 0;
    uint8_t  month = 0, date = 0, hour = 0, minute = 0, second = 0;
    char msg[20];
    if (sysinfo.logmessage < level)
        return;
    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    sprintf(msg, "[%02d:%02d:%02d] ", hour, minute, second);
//    appUartSend(&usart2_ctl, (uint8_t *)msg, strlen(msg));
//    appUartSend(&usart2_ctl, (uint8_t *)debug, strlen(debug));

    usbSend(msg, strlen(msg));
    usbSend(debug, strlen(debug));
}

void LogMessageWL(uint8_t level, char *buf, uint16_t len)
{
    uint16_t year = 0;
    uint8_t  month = 0, date = 0, hour = 0, minute = 0, second = 0;
    char msg[20];
    if (sysinfo.logmessage < level)
        return;
    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    sprintf(msg, "[%02d:%02d:%02d] ", hour, minute, second);

//    appUartSend(&usart2_ctl, (uint8_t *)msg, strlen(msg));
//    appUartSend(&usart2_ctl, (uint8_t *)buf, len);
    usbSend(msg, strlen(msg));
    usbSend(buf, len);
}


void LogWrite(uint8_t level, char *buf, uint16_t len)
{
    if (sysinfo.logmessage < level)
        return;
//    appUartSend(&usart2_ctl, (uint8_t *)buf, len);
    usbSend(buf, len);
}



/*****************************************************************************************/

static const unsigned short ztvm_crctab16[] =
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

unsigned short GetCrc16(const char *pData, int nLength)
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
/*****************************************************************************************/
/*
0 bit:1==>布防,0==>撤防
1 bit:1==>ACC ON ,0==>ACC OFF
2 bit:1==>充电,0==>未充电
3 ~5 bit:
	000: 0 无报警
	001：1 震动报警
	010：2 断电报警
	100：3 低电报警
	100：4 SOS报警
	101：5 车门报警
	110：6 开关报警
	111：7 感光报警

6 bit:1==>GPS定位,0==>未定位
7 bit:1==>油电断,0==>油电通
*/

//0位，撤防布防
void terminalDefense(void)
{
    sysinfo.terminalStatus |= 0x01;
}
void terminalDisarm(void)
{
    sysinfo.terminalStatus &= ~0x01;
}
//1位，acc状态
uint8_t getTerminalAccState(void)
{
    return (sysinfo.terminalStatus & 0x02);

}
void terminalAccon(void)
{
    sysinfo.terminalStatus |= 0x02;
}
void terminalAccoff(void)
{
    sysinfo.terminalStatus &= ~0x02;
}
//2位，充电检测
void terminalCharge(void)
{
    sysinfo.terminalStatus |= 0x04;
}

void terminalunCharge(void)
{
    sysinfo.terminalStatus &= ~0x04;
}
uint8_t getTerminalChargeState(void)
{
    return (sysinfo.terminalStatus & 0x04);
}
//3,4,5
void terminalAlarmSet(TERMINAL_WARNNING_TYPE alarm)
{
    sysinfo.terminalStatus &= ~(0x38);
    sysinfo.terminalStatus |= (alarm << 3);
}
//6位，gps状态
void terminalGPSFixed(void)
{
    sysinfo.terminalStatus |= 0x40;
}
void terminalGPSUnFixed(void)
{
    sysinfo.terminalStatus &= ~0x40;
}



/*****************************************************************************************/
/*获取字符ch在str中的位置*/
int getCharIndex(uint8_t *src, int src_len, char ch)
{
    int i;
    for (i = 0; i < src_len; i++)
    {
        if (src[i] == ch)
            return i;
    }
    return -1;
}

/*判断str2是否属于str1的头部*/
int my_strpach(char *str1, const char *str2)
{
    int i = 0, len1, len2;
    if (str1 == NULL || str2 == NULL)
        return 0;
    len1 = strlen(str1);
    len2 = strlen(str2);
    if (len1 < len2)
        return 0;
    for (i = 0; i < len2; i++)
    {
        if (str1[i] != str2[i])
            return 0;
    }
    return 1;
}

/*在str1的有效长度内,识别str2是否存在于str1中*/
int my_strstr(char *str1, const char *str2, int str1_len)
{
    int strsize;
    int i = 0;
    strsize = strlen(str2);
    for (i = 0; i <= (str1_len - strsize); i++)
    {
        if (str1[i] == str2[0])
        {
            if (my_strpach(&str1[i], str2))
            {
                return 1;
            }
        }
    }
    return 0;
}
/*获取str2在str1中的位置*/
int my_getstrindex(char *str1, const char *str2, int len)
{
    uint16_t strsize;
    uint16_t i = 0;
    if (str1 == NULL || str2 == NULL || len <= 0)
        return -1;
    strsize = strlen(str2);
    if (len < strsize)
        return -2;
    for (i = 0; i <= (len - strsize); i++)
    {
        if (str1[i] == str2[0])
        {
            if (my_strpach(&str1[i], str2))
            {
                return i;
            }
        }
    }
    return -3;
}
/*识别OK*/
int distinguishOK(char *buf)
{
    if (strstr(buf, "OK") != NULL)
        return 1;
    return 0;
}

/*获取字符ch出现在str中的n个位置的所引*/
int16_t getCharIndexWithNum(uint8_t *src, uint16_t src_len, uint8_t ch, uint8_t num)
{
    int i, count = 0;
    if (src == NULL)
        return -1;
    for (i = 0; i < src_len; i++)
    {
        if (src[i] == ch)
        {
            ++count;
            if (count == num)
            {
                return i;
            }
        }
    }
    return -1;
}


/*将字节数组转换成16进制的字符串*/
void changeByteArrayToHexString(uint8_t *src, uint8_t *dest, uint16_t srclen)
{
    uint16_t i;
    uint8_t a, b;
    for (i = 0; i < srclen; i++)
    {
        a = (src[i] >> 4) & 0x0F;
        b = (src[i]) & 0x0F;
        if (a < 10)
        {
            dest[i * 2] = a + '0';
        }
        else
        {
            dest[i * 2] = a - 10 + 'A';
        }

        if (b < 10)
        {
            dest[i * 2 + 1] = b + '0';

        }
        else
        {
            dest[i * 2 + 1] = b - 10 + 'A';
        }
    }

}

static unsigned char  asciiToHex(char ch)
{
    if (ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 0x0A;
    }

    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 0x0A;
    }

    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }

    if (ch == ' ')
        return 0;
    return 0;
}
/*将src中前size*2个字符，转换成size个字节,最终转换成16进制*/
int16_t changeHexStringToByteArray(uint8_t *dest, uint8_t *src, uint16_t size)
{
    uint8_t temp_l, temp_h;
    uint16_t i = 0;

    if (src == NULL || dest == NULL || size == 0)
    {
        return -1;
    }
    for (i = 0; i < (size); i++)
    {
        temp_h = asciiToHex(src[i * 2]);
        temp_l = asciiToHex(src[i * 2 + 1]);
        dest[i] = temp_h << 4 | temp_l;
    }
    return i;
}


/*将src中前size*2个字符，转换成size个字节,最终转换成10进制*/
int16_t changeHexStringToByteArray_10in(uint8_t *dest, uint8_t *src, uint16_t size)
{
    uint8_t temp_l, temp_h;
    uint16_t i = 0;

    if (src == NULL || dest == NULL || size == 0)
    {
        return -1;
    }
    for (i = 0; i < (size); i++)
    {
        temp_h = asciiToHex(src[i * 2]) * 10;
        temp_l = asciiToHex(src[i * 2 + 1]);
        temp_h += temp_l;
        dest[i] = temp_h;
    }
    return i;

}
/*****************************************************************************************/
void parserInstructionToItem(ITEM *item, uint8_t *str, uint16_t len)
{
    uint16_t i, data_len;
    char debug[128];
    memset(item, 0, sizeof(ITEM));
    item->item_cnt = 0;
    data_len = 0;
    //逗号分隔
    for (i = 0; i < ITEMCNTMAX; i++)
    {
        item->item_data[i][0] = 0;
    }
    for (i = 0; i < len; i++)
    {
        if (str[i] == ',' || str[i] == '#' || str[i] == '\r' || str[i] == '\n' || str[i] == '=')
        {
            if (item->item_data[item->item_cnt][0] != 0)
            {
                item->item_cnt++;
                data_len = 0;
                if (item->item_cnt >= ITEMCNTMAX)
                {
                    break; ;
                }
            }
        }
        else
        {
            item->item_data[item->item_cnt][data_len] = str[i];
            data_len++;
            if (i + 1 == len)
            {
                item->item_cnt++;
            }
            if (data_len >= ITEMSIZEMAX)
            {
                sprintf(debug, "%s==>%s\n", __FUNCTION__, "item size max");
                LogMessage(DEBUG_ALL, debug);
                return ;
            }
            item->item_data[item->item_cnt][data_len] = 0;
        }
    }
    //sprintf(debug, "Item cnt=%d\n", item->item_cnt);
    //LogMessage(DEBUG_ALL, debug);
    //转大写
    //    if (item->item_cnt > 0)
    //    {
    //        data_len = strlen(item->item_data[0]);
    //        for (i = 0; i < data_len; i++)
    //        {
    //            if (item->item_data[0][i] >= 'a' && item->item_data[0][i] <= 'z')
    //            {
    //                item->item_data[0][i] = item->item_data[0][i] - 'a' + 'A';
    //            }
    //        }
    //
    //        sprintf(debug, "Instruction==>%s\n", item->item_data[0]);
    //        LogMessage(DEBUG_ALL, debug);
    //    }
}


uint8_t mycmdPatch(uint8_t *cmd1, uint8_t *cmd2)
{
    uint8_t ilen1, ilen2;
    if (cmd1 == NULL || cmd2 == NULL)
        return 0;
    ilen1 = strlen((char *)cmd1);
    ilen2 = strlen((char *)cmd2);
    if (ilen1 != ilen2)
        return 0;
    for (ilen1 = 0; ilen1 < ilen2; ilen1++)
    {
        if (cmd1[ilen1] != cmd2[ilen1])
            return 0;
    }
    return 1;
}


void updateRTCtimeRequest(void)
{
    sysinfo.localrtchadupdate = 0;
}

