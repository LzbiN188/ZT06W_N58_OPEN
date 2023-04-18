#ifndef APP_INSTRUCTIONCMD
#define APP_INSTRUCTIONCMD

#include "nwy_osi_api.h"

typedef enum
{
    PARAM_INS,
    STATUS_INS,
    VERSION_INS,
    SN_INS,
    SERVER_INS,
    MODE_INS,
    HBT_INS,
    TTS_INS,
    JT_INS,
    POSITION_INS,
    APN_INS,
    UPS_INS,
    LOWW_INS,
    LED_INS,
    POITYPE_INS,
    RESET_INS,
    UTC_INS,
    ALARMMODE_INS,
    DEBUG_INS,
    ACCCTLGNSS_INS,
    PDOP_INS,
    BF_INS,
    CF_INS,
    FENCE_INS,
    FACTORY_INS,
    RELAY_INS,
    VOL_INS,
    ACCDETMODE_INS,
    SETAGPS_INS,
    BLEEN_INS,
    SOS_INS,
    FCG_INS,
    FACTORYTEST_INS,
    JT808SN_INS,
    JT808PARAM_INS,
    HIDESERVER_INS,
    SETBLEMAC_INS,
    READPARAM_INS,
    SETBLEPARAM_INS,
    SETBLEWARNPARAM_INS,
    RELAYFUN_INS,
    RELAYSPEED_INS,
    BLESERVER_INS,
    RELAYFORCE_INS,
    BLESCAN_INS,
    BLERELAYCTL_INS,
    SETRFHOLD_INS,
} cmd_e;



typedef struct
{
    cmd_e cmdid;
    char *cmdstr;
} instruction_s;

typedef enum
{
    SERIAL_MODE,
    MESSAGE_MODE,
    NETWORK_MODE,
    JT808_MODE,
    BLE_MODE,
} instructionMode_e;


typedef struct{
	uint8_t link;
	char * telNum;
	instructionMode_e	mode;
}instructionParam_s;

void instructionParser(uint8_t *str, uint16_t len, instructionParam_s *param);
void doPositionRespon(void);
void doBleScanRespon(char * message);

#endif
