#include "app_param.h"
#include "app_sys.h"
#include "app_net.h"
#include "nwy_file.h"

#define PARAM_FILENAME	"param.save"
SYSTEM_FLASH_DEFINE sysparam;

void paramSaveAll(void)
{
    uint16_t paramsize, fileOperation;
    int fd, writelen;
    paramsize = sizeof(SYSTEM_FLASH_DEFINE);
    if (nwy_sdk_fexist(PARAM_FILENAME) == true)
    {
        fileOperation = NWY_WRONLY;
    }
    else
    {
        fileOperation = NWY_CREAT | NWY_RDWR;
    }
    fd = nwy_sdk_fopen(PARAM_FILENAME, fileOperation);
    if (fd < 0)
    {
        LogMessage(DEBUG_ALL, "paramSaveAll==>Open error\r\n");
        return;
    }
    writelen = nwy_sdk_fwrite(fd, &sysparam, paramsize);
    if (writelen != paramsize)
    {
        LogMessage(DEBUG_ALL, "paramSaveAll==>Error\r\n");
    }
    else
    {
        LogMessage(DEBUG_ALL, "paramSaveAll==>Success\r\n");
    }
    nwy_sdk_fsync(fd);
    nwy_sdk_fclose(fd);
}


void paramGetAll(void)
{
    uint16_t paramsize, fileOperation;
    int fd, readlen;
    paramsize = sizeof(SYSTEM_FLASH_DEFINE);
    if (nwy_sdk_fexist(PARAM_FILENAME) == true)
    {
        fileOperation = NWY_RDONLY;
    }
    else
    {
        LogMessage(DEBUG_ALL, "Not file\r\n");
        return;
    }
    fd = nwy_sdk_fopen(PARAM_FILENAME, fileOperation);
    if (fd < 0)
    {
        LogMessage(DEBUG_ALL, "paramGetAll==>Open error\r\n");
        return;
    }
    readlen = nwy_sdk_fread(fd, &sysparam, paramsize);
    if (readlen != paramsize)
    {
        LogMessage(DEBUG_ALL, "paramGetAll==>paramChange\r\n");
    }
    else
    {
        LogMessage(DEBUG_ALL, "paramGetAll==>Success\r\n");
    }
    nwy_sdk_fclose(fd);
}




void paramInit(void)
{
    char *imei;
    paramGetAll();
    if (sysparam.VERSION != EEPROM_VER)
    {
        paramDefaultInit(0);
    }
    sysinfo.updateStatus = 0;
    sysinfo.lowvoltage = sysparam.lowvoltage / 10.0;

    strncpy((char *)sysparam.jt808terminalType, "OPEN_N716", 9);
    strncpy((char *)sysparam.jt808manufacturerID, "00001", 5);
    strncpy((char *)sysparam.jt808terminalID, "0000000", 7);
    imei = readModuleIMEI();
    if (imei[0] != 0)
    {
        strncpy((char *)sysparam.SN, imei, 15);
        paramSaveAll();
    }

}
void paramDefaultInit(uint8_t level)
{
    uint8_t i;
    SYSTEM_FLASH_DEFINE	thisparam;

    memset(&thisparam, 0, sizeof(SYSTEM_FLASH_DEFINE));

    if (level == 0)
    {
        thisparam.ServerPort = 9998;
        strcpy((char *)thisparam.SN, "888888887777777");
        strcpy((char *)thisparam.Server, "jzwz.basegps.com");
        strcpy((char *)thisparam.apn, "cmnet");
        thisparam.adccal = 22.684311;
    }
    else
    {
        thisparam.ServerPort = sysparam.ServerPort;
        strcpy((char *)thisparam.SN, (char *)sysparam.SN);
        strcpy((char *)thisparam.Server, (char *)sysparam.Server);
        strcpy((char *)thisparam.apn, (char *)sysparam.apn);
        strcpy((char *)thisparam.apnuser, (char *)sysparam.apnuser);
        strcpy((char *)thisparam.apnpassword, (char *)sysparam.apnpassword);
        thisparam.adccal = sysparam.adccal;
    }
    for (i = 0; i < 5; i++)
    {
        thisparam.AlarmTime[i] = 0xFFFF;
    }
    strcpy((char *)thisparam.agpsServer, "www.gnss-aide.com");
    thisparam.agpsPort = 2621;
    thisparam.MODE = 2;
    thisparam.lightAlarm = 1;
    thisparam.gapDay = 1;
    thisparam.heartbeatgap = 180;
    thisparam.poitype = 2;
    thisparam.accctlgnss = 1;
    thisparam.lowvoltage = 115;
    thisparam.fence = 30;
    thisparam.utc = 8;
    thisparam.gpsuploadgap = 10;
    thisparam.pdop = 600;
    thisparam.VERSION = EEPROM_VER;
    strcpy((char *)thisparam.updateServer, "agps.domilink.cn");
    thisparam.updateServerPort = 9999;
    thisparam.overspeed = 20;
    thisparam.overspeedTime = 5;
    thisparam.rettsPlaytime = 30;
    thisparam.vol = 100;
    thisparam.volTable1 = 8;
    thisparam.volTable2 = 9;
    thisparam.volTable3 = 10;
    thisparam.volTable4 = 11;
    thisparam.volTable5 = 12;
    thisparam.volTable6 = 13;
    memcpy(&sysparam, &thisparam, sizeof(SYSTEM_FLASH_DEFINE));
    paramSaveAll();
    LogMessage(DEBUG_ALL, "paramDefaultInit\r\n");
}


