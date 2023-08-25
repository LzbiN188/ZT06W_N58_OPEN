#include "app_db.h"
#include "app_param.h"
#include "app_sys.h"
#include "app_jt808.h"
#include "app_net.h"
#include "app_task.h"
static dbSave_s dbSave;


/**************************************************
@bref		初始化db
@param
@return
@note
**************************************************/

void dbSaveInit(void)
{
    dbSave.gpsNum = 0;
}

/**************************************************
@bref		存储gps数据
@param
@return
@note
**************************************************/

void dbSaveAll(void)
{
	uint8_t ret;
	if (sysparam.dbsize + sizeof(gpsRestore_s) * dbSave.gpsNum >= 40000)
	{
		return;
	}
    ret = gpsRestoreWriteData(dbSave.gpsBuff, dbSave.gpsNum);
    if (ret)
    {
    	sysparam.dbsize += sizeof(gpsRestore_s) * dbSave.gpsNum;
    	paramSaveAll();
    }
    LogPrintf(DEBUG_ALL, "dbSaveAll==>dbsize:%d, ret:%d\r\n", sysparam.dbsize, ret);
    sysinfo.dbFileUpload = 1;
    dbSave.gpsNum = 0;
}

/**************************************************
@bref		存储gps数据
@param
@return
@note
**************************************************/

void dbPush(gpsRestore_s *gpsresinfo)
{
    if (dbSave.gpsNum >= DB_MAX_SIZE)
    {
        LogMessage(DEBUG_ALL, "dbPush==>save all");
        dbSaveAll();
    }
    memcpy(&dbSave.gpsBuff[dbSave.gpsNum++], gpsresinfo, sizeof(gpsRestore_s));
    LogPrintf(DEBUG_ALL, "dbPush==>%d", dbSave.gpsNum);
}

/**************************************************
@bref		传送db中的gps数据
@param
@return
@note
**************************************************/

uint8_t dbPost(void)
{
    uint8_t i;
    uint8_t dest[1200];
    uint16_t destlen;
    uint16_t protocollen;
    if (dbSave.gpsNum == 0)
    {
        return 0;
    }
    LogPrintf(DEBUG_ALL, "dbPost==>%d", dbSave.gpsNum);
    destlen = 0;
    for (i = 0; i < dbSave.gpsNum; i++)
    {

        if (sysparam.protocol == JT808_PROTOCOL_TYPE)
        {
            protocollen = jt808gpsRestoreDataSend((uint8_t *)dest + destlen, &dbSave.gpsBuff[i]);
        }
        else
        {
            gpsRestoreDataSend(&dbSave.gpsBuff[i], (char *) dest + destlen, &protocollen);
        }
        destlen += protocollen;
    }
    dbSave.gpsNum = 0;

    if (sysparam.protocol == JT808_PROTOCOL_TYPE)
    {
        jt808TcpSend((uint8_t *)dest, destlen);
    }
    else
    {
        tcpSendData(NORMAL_LINK, (uint8_t *)dest, destlen);
    }
    return 1;
}



