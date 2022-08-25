#include "app_mcu.h"
#include "app_sys.h"
#include "app_port.h"
#include "app_task.h"
#include "app_protocol.h"
#include "app_protocol_808.h"
#include "app_param.h"

MCU_UPGRADE_INFO upgradeInfo;


static void doVersionResponCmd(uint8_t *buf, uint16_t len)
{
    char version[40];
    strncpy(version, (char *)buf, len);
    version[len] = 0;
    updateMcuVersion(version);
	strcpy((char *)sysinfo.mcuVersion,version);
    LogPrintf(DEBUG_ALL, "MCUVersion:%s\r\n", version);
}



static void doGenericResponCmd(uint8_t *buf, uint16_t len)
{

    switch (buf[1])
    {
        case MCU_VERSION_CMD:
            doVersionResponCmd(buf + 2, len - 2);
            break;
        case MCU_WRITE_CMD:
            upgradeInfo.upgradeRsp = 1;
            if (buf[2] == 1)
            {
                upgradeInfo.upgradeRet = 1;
                LogMessage(DEBUG_ALL, "Upgrade OK\r\n");
            }
            else
            {
                upgradeInfo.upgradeRet = 0;
                LogMessage(DEBUG_ALL, "Upgrade Error\r\n");
            }
            break;
    }
}
/*
Byte[0]:CmdId
Byte[1~N]:Content
*/
void appMcuRxContent(uint8_t *buf, uint16_t len)
{
    uint8_t debugStr[101];
    uint8_t debugLen;
    if (len < 0)
        return;
    debugLen = len > 50 ? 50 : len;
    changeByteArrayToHexString(buf, debugStr, debugLen);
    debugStr[debugLen * 2] = 0;
    LogPrintf(DEBUG_ALL, "Msg:%s\r\n", debugStr);
    switch (buf[0])
    {
        case MCU_RESPON_CMD:
            doGenericResponCmd(buf, len);
            break;
    }
}

/*
| 帧头 | 长度 | 内容 |校验 | 帧尾 |
  0x0c  0xXXXX  xxxx  0xXX   0x0D

*/

void appMcuSend(uint8_t *content, uint16_t len)
{
    uint8_t buf[10];
    uint8_t crc = 0;
    uint16_t i;
    buf[0] = 0x0C;
    buf[1] = len >> 8 & 0xFF;
    buf[2] = len & 0xFF;
    appUartSend(&usart2_ctl, (uint8_t *)buf, 3);
    appUartSend(&usart2_ctl, (uint8_t *)content, len);
    for (i = 0; i < len; i++)
    {
        crc ^= content[i];
    }
    buf[0] = crc;
    buf[1] = 0x0D;
    appUartSend(&usart2_ctl, (uint8_t *)buf, 2);
}

//一帧解析
//0c 00 00 00 0D
//0c 00 01 01 01 0D
//0C 00 0F 2B 50 4F 57 45 52 4F 46 46 3A 20 4F 4B 0D 0A 22 0D
int16_t appFrameParser(uint8_t *content, uint16_t len)
{
    uint8_t crc = 0;
    uint16_t length, i;
    if (len < 5)
        return -1;
    if (content[0] != 0x0C)
        return -2;
    length = content[1] << 8 | content[2];
    if (length + 5 > len)
    {
        return -3;
    }
    if (content[length + 4] != 0x0D)
    {
        return -4;
    }
    for (i = 0; i < length; i++)
    {
        crc ^= content[i + 3];
    }
    if (crc != content[length + 3])
    {
        return -5;
    }
    appMcuRxContent(content + 3, length);
    return length + 5;
}


void appMcuRxParser(const char *msg, uint32_t len)
{
    static uint8_t Buffer[APP_RECEIVE_BUFF_SIZE + 1];
    static uint16_t size = 0;
    uint16_t i;
    int16_t ret;
    uint16_t lastIndex;
    if (len == 0)
        return;
    if (len + size > APP_RECEIVE_BUFF_SIZE)
    {
        size = 0;
        len = len > APP_RECEIVE_BUFF_SIZE ? APP_RECEIVE_BUFF_SIZE : len;
    }
    memcpy(Buffer + size, msg, len);
    size += len;
    lastIndex = 0;
    for (i = 0; i < size; i++)
    {
        if (Buffer[i] == 0x0C)
        {
            ret = appFrameParser(Buffer + i, size - i);
            if (ret > 0)
            {
                lastIndex = i + ret;
                i = lastIndex - 1;
            }
        }
    }
    if (lastIndex != 0)
    {
        size -= lastIndex;
        if (size != 0)
        {
            memmove(Buffer, Buffer + lastIndex, size);
        }
    }

}


/*-----------------------------------------------------------------------*/


void appMcuUpgradeInit(void)
{
    memset(&upgradeInfo, 0, sizeof(upgradeInfo));
    upgradeInfo.firmWare = upgradeInfo.packBuf + MCU_PROTOCOL_HEAD;
}

void pushUpgradeFirmWare(uint8_t *firmware, uint32_t offset, uint16_t firmwareSize)
{
    memcpy(upgradeInfo.firmWare, firmware, firmwareSize);
    upgradeInfo.firmWareOffset = offset;
    upgradeInfo.firmWareSize = firmwareSize;
    LogPrintf(DEBUG_ALL, "push firmware==>Offset:0x%X,Size:%d\r\n", upgradeInfo.firmWareOffset, upgradeInfo.firmWareSize);
}

void doStartUpgrade(void)
{
    uint8_t buff[2];
    uint8_t sendSize = 0;
    buff[sendSize++] = MCU_STARTUPGRADE_CMD;
    appMcuSend(buff, sendSize);
    LogMessage(DEBUG_ALL, "do start upgrade\r\n");
}


void doResetMcu(void)
{
    uint8_t buff[2];
    uint8_t sendSize = 0;
    buff[sendSize++] = MCU_RESET_CMD;
    appMcuSend(buff, sendSize);
    LogMessage(DEBUG_ALL, "do reset mcu\r\n");
}




void doEarseCmd(uint32_t size)
{
    uint8_t buff[6];

    uint8_t sendSize = 0;
    buff[sendSize++] = MCU_EARSE_CMD;
    buff[sendSize++] = size >> 24 & 0xff;
    buff[sendSize++] = size >> 16 & 0xff;
    buff[sendSize++] = size >> 8 & 0xff;
    buff[sendSize++] = size & 0xff;
    appMcuSend(buff, sendSize);
    LogMessage(DEBUG_ALL, "do mcu earse\r\n");
}

void doDoneCmd(void)
{
    uint8_t buff[2];
    uint8_t sendSize = 0;
    buff[sendSize++] = MCU_DONE_CMD;
    appMcuSend(buff, sendSize);
    LogMessage(DEBUG_ALL, "flash download done\r\n");
}




void doGetVersionCmd(void)
{
    uint8_t buff[2];
    uint8_t sendSize = 0;
    buff[sendSize++] = MCU_VERSION_CMD;
    appMcuSend(buff, sendSize);
    LogMessage(DEBUG_ALL, "do getversion cmd\r\n");
}


void appMcuUpgradeTask(void)
{
    static uint8_t fsm = 0;
    static uint8_t tick = 0;
    switch (fsm)
    {
        case 0:
            if (upgradeInfo.firmWareSize == 0)
            {
                return;
            }
            upgradeInfo.packBuf[0] = MCU_WRITE_CMD;
            upgradeInfo.packBuf[1] = upgradeInfo.firmWareOffset >> 24 & 0xff;
            upgradeInfo.packBuf[2] = upgradeInfo.firmWareOffset >> 16 & 0xff;
            upgradeInfo.packBuf[3] = upgradeInfo.firmWareOffset >> 8 & 0xff;
            upgradeInfo.packBuf[4] = upgradeInfo.firmWareOffset & 0xff;
            upgradeInfo.packBuf[5] = upgradeInfo.firmWareSize >> 8 & 0xff;
            upgradeInfo.packBuf[6] = upgradeInfo.firmWareSize & 0xff;
            upgradeInfo.upgradeRsp = 0;
            appMcuSend(upgradeInfo.packBuf, upgradeInfo.firmWareSize + MCU_PROTOCOL_HEAD);
            fsm = 1;
            tick = 0;
            LogPrintf(DEBUG_ALL, "Write firmware==>Offset:0x%X,Size:%d\r\n", upgradeInfo.firmWareOffset, upgradeInfo.firmWareSize);
            break;
        case 1:
            ++tick;
            LogPrintf(DEBUG_ALL, "Wait respon %d sec\r\n", tick);
            if (upgradeInfo.upgradeRsp)
            {
                if (upgradeInfo.upgradeRet)
                {
                    fsm = 0;
                    upgradeResultProcess(0, upgradeInfo.firmWareOffset, upgradeInfo.firmWareSize);
                }
                else
                {
                    upgradeResultProcess(1, upgradeInfo.firmWareOffset, upgradeInfo.firmWareSize);
                }
                upgradeInfo.firmWareSize = 0;
                break;;
            }
            if (tick >= 50)
            {
                fsm = 0;
                LogMessage(DEBUG_ALL, "no mcu respon,try to send firmware again\r\n");
            }
            //
            break;
    }
}
/*-----------------------------------------------------------------------*/

