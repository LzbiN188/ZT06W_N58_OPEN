#include "app_param.h"
#include "app_sys.h"
#include "app_task.h"
#include "nwy_file.h"
#include "app_ble.h"

#define PARAM_FILENAME	"param.save"

systemParam_s sysparam;

/**************************************************
@bref		系统参数保持至文件中
@param
@return
@note
**************************************************/
void paramSaveAll(void)
{
    uint16_t paramsize, fileOperation;
    int fd, writelen;
    paramsize = sizeof(systemParam_s);
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
        LogMessage(DEBUG_ALL, "paramSaveAll==>Open error");
        return;
    }
    writelen = nwy_sdk_fwrite(fd, &sysparam, paramsize);
    if (writelen != paramsize)
    {
        LogMessage(DEBUG_ALL, "paramSaveAll==>Error");
    }
    else
    {
        LogMessage(DEBUG_ALL, "paramSaveAll==>Success");
    }
    nwy_sdk_fsync(fd);
    nwy_sdk_fclose(fd);
}

/**************************************************
@bref		从文件中读取系统参数
@param
@return
@note
**************************************************/

void paramGetAll(void)
{
    uint16_t paramsize, fileOperation;
    int fd, readlen;
    paramsize = sizeof(systemParam_s);
    if (nwy_sdk_fexist(PARAM_FILENAME) == true)
    {
        fileOperation = NWY_RDONLY;
    }
    else
    {
        LogMessage(DEBUG_ALL, "Not file");
        return;
    }
    fd = nwy_sdk_fopen(PARAM_FILENAME, fileOperation);
    if (fd < 0)
    {
        LogMessage(DEBUG_ALL, "paramGetAll==>Open error");
        return;
    }
    readlen = nwy_sdk_fread(fd, &sysparam, paramsize);
    if (readlen != paramsize)
    {
        LogMessage(DEBUG_ALL, "paramGetAll==>paramChange");
    }
    else
    {
        LogMessage(DEBUG_ALL, "paramGetAll==>Success");
    }
    nwy_sdk_fclose(fd);
}

/**************************************************
@bref		参数重置
@param
	lev		0：所有参数重置
			1：部分参数重置
@return
@note
**************************************************/

void paramSetDefault(uint8_t lev)
{
    systemParam_s thisparam;
    memset(&thisparam, 0, sizeof(systemParam_s));
    //所有参数默认为0 ，如果为非零值，请指定默认值
    if (lev == 0)
    {
        //关键参数也重置
        strcpy(thisparam.SN, "888888887777777");
        strncpy((char *)thisparam.jt808sn, "888777", 6);
        strcpy(thisparam.Server, "jzwz.basegps.com");
        strcpy(thisparam.hiddenServer, "jzwz.basegps.com");
        strcpy((char *)thisparam.jt808Server, "47.106.96.28");
        strcpy(thisparam.apn, "cmnet");
        thisparam.ServerPort = 9998;
        thisparam.jt808Port = 9997;
        thisparam.hiddenPort = 9998;
        thisparam.adccal = 31.33;
        thisparam.hiddenServOnoff = 1;
        thisparam.protocol = ZT_PROTOCOL_TYPE;
    }
    else
    {
        strcpy(thisparam.SN, sysparam.SN);
        strcpy((char *)thisparam.jt808sn, (char *)sysparam.jt808sn);
        strcpy(thisparam.Server, sysparam.Server);
        strcpy(thisparam.hiddenServer, sysparam.hiddenServer);
        strcpy((char *)thisparam.jt808Server, (char *)sysparam.jt808Server);
        strcpy(thisparam.apn, sysparam.apn);
        strcpy(thisparam.apnuser, sysparam.apnuser);
        strcpy(thisparam.apnpassword, sysparam.apnpassword);
        thisparam.ServerPort = sysparam.ServerPort;
        thisparam.jt808Port = sysparam.jt808Port;
        thisparam.hiddenPort = sysparam.hiddenPort;
        thisparam.adccal = sysparam.adccal;
        thisparam.hiddenServOnoff = sysparam.hiddenServOnoff;
        thisparam.protocol = sysparam.protocol;
    }

    strcpy(thisparam.updateServer, "47.106.81.204");
    strcpy(thisparam.agpsServer, "agps.domilink.com");
    strcpy(thisparam.agpsUser, "123");
    strcpy(thisparam.agpsPswd, "123");

    thisparam.updateServerPort = 9998;
    thisparam.agpsPort = 10187;
    thisparam.paramVersion = PARAM_VER;
    thisparam.heartbeatgap = 180;
    thisparam.lightAlarm = 1;
    thisparam.accctlgnss = 1;
    thisparam.vol = 60;
    thisparam.accdetmode = 2;
    thisparam.MODE = 2;
    thisparam.utc = 8;
    thisparam.gpsuploadgap = 10;
    thisparam.gapMinutes = 0;
    thisparam.fence = 30;
    thisparam.gapDay = 1;
    thisparam.pdop = 6.0;
    thisparam.lowvoltage = 11.5;

    thisparam.AlarmTime[0] = 720;
    thisparam.AlarmTime[1] = 0xFFFF;
    thisparam.AlarmTime[2] = 0xFFFF;
    thisparam.AlarmTime[3] = 0xFFFF;
    thisparam.AlarmTime[4] = 0xFFFF;

    thisparam.accOnVoltage = 13.2;
    thisparam.accOffVoltage = 12.8;

    thisparam.bleRfThreshold = 48;
    thisparam.bleOutThreshold = 1500;
    thisparam.bleAutoDisc = 0;

    thisparam.blePreShieldVoltage = 48;
    thisparam.blePreShieldDetCnt = 10;
    thisparam.blePreShieldHoldTime = 1;

    thisparam.relaySpeed = 20;
    thisparam.bleRelay = 1;
    thisparam.bleVoltage = 11.5;

	thisparam.simpulloutalm=1;
	thisparam.simpulloutLock=1;
	thisparam.shutdownalm=1;
	thisparam.shutdownLock=1;
	thisparam.uncapalm=1;
	thisparam.uncapLock=1;

	thisparam.simSel = 0;
	thisparam.otaParamFlag = OTA_PARAM_FLAG;
    memcpy(&sysparam, &thisparam, sizeof(systemParam_s));
    paramSaveAll();
    LogMessage(DEBUG_ALL, "paramSetDefault");
}

/**************************************************
@bref		参数初始化
@param
@return
@note
**************************************************/

void paramInit(void)
{
    memset(&sysparam, 0, sizeof(systemParam_s));
    paramGetAll();
    if (sysparam.paramVersion != PARAM_VER)
    {
        paramSetDefault(0);
    }
    if (sysparam.otaParamFlag != OTA_PARAM_FLAG)
    {
		sysparam.otaParamFlag = OTA_PARAM_FLAG;
		sysparam.agpsPort = 10187;
		sysparam.latitude = 0.0;
		sysparam.longtitude = 0.0;
		sysparam.mileage = 0.0;
		sysparam.milecal = 5;
		paramSaveAll();
    }
    sysinfo.alarmrequest = sysparam.alarmRequest;

    stringToLowwer(sysparam.Server, strlen(sysparam.Server));
    stringToLowwer(sysparam.hiddenServer, strlen(sysparam.hiddenServer));
}
