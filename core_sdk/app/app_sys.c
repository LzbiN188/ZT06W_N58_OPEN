#include "app_sys.h"
#include "app_task.h"
#include "app_port.h"
#include <stdarg.h>
#include <stdio.h>
#include "nwy_osi_api.h"
#define LOGPRINTF_MAX		1024



/**************************************************
@bref		������Ϣ
@param
	level	��Ϣ�ȼ�
	debug	��������
@note
**************************************************/
void LogMessage(uint8_t level, char *debug)
{
    LogMessageWL(level, debug, strlen(debug));
}
/**************************************************
@bref		������Ϣ
@param
	level	��Ϣ�ȼ�
	debug	��������
	len		���ݳ���
@note
**************************************************/
void LogMessageWL(uint8_t level, char *debug, uint16_t len)
{
	static nwy_osiMutex_t * myMutex=NULL;
    uint16_t year = 0;
    uint8_t  month = 0, date = 0, hour = 0, minute = 0, second = 0;
    char msg[20];
    if (sysinfo.logLevel < level)
        return;
    if (myMutex == NULL)
    {
        myMutex = nwy_create_mutex();
        return;
    }
    if (myMutex == NULL)
    {
        return;
    }
    nwy_lock_mutex(myMutex, 0);
    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    sprintf(msg, "[%02d:%02d:%02d] ", hour, minute, second);
    portUartSend(SYS_DEBUG_UART, (uint8_t *)msg, strlen(msg));
    portUartSend(SYS_DEBUG_UART, (uint8_t *)debug, len);
    portUartSend(SYS_DEBUG_UART, (uint8_t *)"\r\n", 2);
    portUsbSend(msg, strlen(msg));
    portUsbSend(debug, len);
    portUsbSend("\r\n", 2);
    nwy_unlock_mutex(myMutex);
}

/**************************************************
@bref		��ʽ�����������Ϣ
@param
	level	��Ϣ�ȼ�
	debug	��������
	...
@note
**************************************************/
void LogPrintf(uint8_t level, const char *debug, ...)
{
    static char LOGDEBUG[LOGPRINTF_MAX];
    va_list args;
    if (sysinfo.logLevel < level)
        return;
    va_start(args, debug);
    vsnprintf(LOGDEBUG, LOGPRINTF_MAX, debug, args);
    va_end(args);
    LogMessageWL(level, LOGDEBUG, strlen(LOGDEBUG));
}

/**************************************************
@bref		��ָ�����ַ����У�����ĳ���ַ����ֵ�λ��
@param
	src		�ַ���
	src_len	�ַ�������
	ch		Ҫ���ҵ��ַ�
@return
	>0		Ҫ���ҵ��ַ����ַ����е�λ��
	-1		������
@note
**************************************************/

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

/**************************************************
@bref		���ֽ�����ת��Ϊ�ַ���
@param
	src		�ֽ���
	dest	ת��ʱ�������
	srclen	�ֽ�������
@return
	none
@note
**************************************************/

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
/*��src��ǰsize*2���ַ���ת����size���ֽ�,����ת����16����*/
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

/**************************************************
@bref		�ж�str1��ͷ���Ƿ��str2��ȫƥ��
@param
	str1	Ŀ��
	str2	ƥ��ֵ
@return
	0		��ƥ��
	1		ƥ��
@note
**************************************************/

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

/**************************************************
@bref		��str1����Ч�����ڲ����Ƿ����str2
@param
	str1		Ŀ��
	str2		ƥ��ֵ
	str1_len	str1����
@return
	0			������
	1			str1�д���str2
@note
**************************************************/

int my_strstr(char *str1, const char *str2, int str1_len)
{
    int strsize;
    int i = 0;
    strsize = strlen(str2);
    if (strsize == 0 || str1_len == 0 || str2 == NULL)
        return 0;
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
/**************************************************
@bref		����str2��str1��Ч�����е�λ��
@param
	str1		Ŀ��
	str2		ƥ��ֵ
@return
	>=0			��Чλ��
	<0			�����ڸ��ַ���
@note
**************************************************/
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

/**************************************************
@bref		ָ��ƥ�䣬2��ָ�������ȫƥ��
@param
@return
	0			��ͬ
	1			��ͬ
@note
**************************************************/

int mycmdPatch(uint8_t *cmd1, uint8_t *cmd2)
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

/**************************************************
@bref		��һ���ַ������item
@param
@return
@note
**************************************************/

void stringToItem(ITEM *item, uint8_t *str, uint16_t len)
{
    uint16_t i, data_len;
    memset(item, 0, sizeof(ITEM));
    item->item_cnt = 0;
    data_len = 0;
    //���ŷָ�
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
                return ;
            }
        }
    }
}

/**************************************************
@bref		��һ���ַ����е�Сд��ĸת�ɴ�д
@param
@return
@note
**************************************************/
void stringToUpper(char *str, uint16_t strlen)
{
    uint16_t i;
    for (i = 0; i < strlen; i++)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
        {
            str[i] = str[i] - 'a' + 'A';
        }
    }
}


/**************************************************
@bref		��һ���ַ����еĴ�д��ĸת��Сд
@param
@return
@note
**************************************************/
void stringToLowwer(char *str, uint16_t strlen)
{
    uint16_t i;
    for (i = 0; i < strlen; i++)
    {
        if (str[i] >= 'A' && str[i] <= 'Z')
        {
            str[i] = str[i] - 'A' + 'a';
        }
    }
}

