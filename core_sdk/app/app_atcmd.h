#ifndef APP_ATCMD
#define APP_ATCMD

#include "nwy_osi_api.h"

typedef enum
{
    AT_SMS_CMD,
    AT_DEBUG_CMD,
    AT_NMEA_CMD,
    AT_ZTSN_CMD,
    AT_IMEI_CMD,
    AT_FMPC_NMEA_CMD,
    AT_FMPC_BAT_CMD,
    AT_FMPC_GSENSOR_CMD,
    AT_FMPC_ACC_CMD,
    AT_FMPC_GSM_CMD,
    AT_FMPC_RELAY_CMD,
    AT_FMPC_LDR_CMD,
    AT_FMPC_ADCCAL_CMD,
    AT_FMPC_CSQ_CMD,
    AT_FMPC_IMSI_CMD,
    AT_FMPC_CHKP_CMD,
    AT_FMPC_CM_CMD,
    AT_FMPC_CMGET_CMD,
} atCmd_e;


typedef struct
{
    atCmd_e cmd;
    char   *cmdStr;
} atCmd_s;

void atCmdRecvParser(char *buf, uint32_t len);

#endif
