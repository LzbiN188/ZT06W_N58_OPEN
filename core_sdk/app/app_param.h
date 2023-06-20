#ifndef APP_PARAM
#define APP_PARAM

#include "nwy_osi_api.h"

#define PARAM_VER		0x20

#define EEPROM_VERSION	"N58_CA_V5.2.2"

#define JT808_PROTOCOL_TYPE			8
#define ZT_PROTOCOL_TYPE			0

typedef struct
{
    char paramVersion;
    char apn[50];
    char apnuser[50];
    char apnpassword[50];
    char SN[20];
    char Server[50];
    char updateServer[50];
    char agpsServer[50];
    char agpsUser[50];
    char agpsPswd[50];
    char sosnumber1[20];
    char sosnumber2[20];
    char sosnumber3[20];
    char jt808Server[50];
    char hiddenServer[50];
    signed char utc;

    uint8_t MODE;
    uint8_t lightAlarm;
    uint8_t gapDay;
    uint8_t ledctrl;
    uint8_t poitype;
    uint8_t accctlgnss;
    uint8_t bf;
    uint8_t cm;
    uint8_t protocol;
    uint8_t vol;
    uint8_t accdetmode;
    uint8_t bleen;
    uint8_t relayCtl;
    uint8_t hiddenServOnoff;

    uint8_t jt808isRegister;
    uint8_t jt808sn[7];
    uint8_t jt808AuthCode[50];
    uint8_t jt808AuthLen;


    uint8_t bleConnMac[5][20];
    uint8_t bleAutoDisc;
    uint8_t bleRfThreshold;
	uint8_t blePreShieldVoltage;
	uint8_t blePreShieldDetCnt;
	uint8_t blePreShieldHoldTime;


    uint16_t heartbeatgap;
    uint16_t gpsuploadgap;
    uint16_t fence;
    uint16_t updateServerPort;
    uint16_t AlarmTime[5];
    uint16_t gapMinutes;
    uint16_t startUpCnt;
    uint16_t ServerPort;
    uint16_t agpsPort;
    uint16_t jt808Port;
    uint16_t hiddenPort;
    uint16_t bleOutThreshold;
	uint16_t unused;//取消、不使用

    uint32_t runTime;

    float adccal;
    float pdop;
    float lowvoltage;
    float accOnVoltage;
    float accOffVoltage;

    //add new
    uint8_t relayFun;
	uint8_t relaySpeed;

	char bleServer[50];
	uint16_t bleServerPort;

	uint8_t bleErrCnt;
	uint8_t bleRelay;
	float   bleVoltage;

	uint32_t alarmRequest;
	uint8_t shutdownalm;
	uint8_t uncapalm;
	uint8_t simpulloutalm;
	uint8_t simSel;

	uint8_t shutdownLock;
	uint8_t uncapLock;
	uint8_t simpulloutLock;
} systemParam_s;

extern systemParam_s sysparam;


void paramSaveAll(void);
void paramGetAll(void);
void paramSetDefault(uint8_t lev);
void paramInit(void);

#endif
