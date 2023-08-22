#ifndef APP_PARAM
#define APP_PARAM
#include "nwy_osi_api.h"

#define EEPROM_VER				0X2B
#define EEPROM_VERSION			"WT07_V4.1.7"

#define CUPDATE_FLAT			0xAB

/*EPROM中的数据*/
typedef struct
{
    uint8_t VERSION;   /*当前软件版本号*/
    uint8_t MODE;      /*系统工作模式*/
    uint8_t lightAlarm;/*光感触发功能是否开启*/
    uint8_t gapDay;    /*模式一间隔天数*/
    uint8_t heartbeatgap;
    uint8_t ledctrl;
    uint8_t poitype;
    uint8_t accctlgnss;
    uint8_t apn[50];
    uint8_t apnuser[50];
    uint8_t apnpassword[50];
    uint8_t lowvoltage;
    uint8_t bf;
    uint8_t fence;
    uint8_t SN[20];
    uint8_t Server[50];
    uint8_t bleen;
    uint8_t cm;
    uint8_t updateServer[50];
    uint8_t agpsServer[50];
    uint8_t sosnumber1[20];
    uint8_t sosnumber2[20];
    uint8_t sosnumber3[20];

    int8_t utc;

    uint16_t updateServerPort;
    uint16_t agpsPort;
    uint16_t gpsuploadgap;
    uint16_t  AlarmTime[5];  /*每日时间，0XFFFF，表示未定义，单位分钟*/
    uint16_t  gapMinutes;    /*模式三间隔周期，单位分钟*/
    uint16_t  pdop;
    uint16_t  mode1startuptime;
	
    uint32_t  mode2worktime;  /*模式二的工作时间，单位分钟*/
    uint32_t ServerPort;

    double latitude;
    double longtitude;
    float  adccal;
    float protectVoltage;

	uint8_t protocol;	
    uint8_t jt808isRegister;
    uint8_t jt808sn[6];
    uint8_t jt808manufacturerID[5];
    uint8_t jt808terminalType[20];
    uint8_t jt808terminalID[7];
    uint8_t jt808AuthCode[50];
    uint8_t jt808AuthLen;

	uint8_t overspeed;
	uint8_t rettsPlaytime;
	uint8_t vol;

	uint8_t alarmMusicIndex;
	uint8_t accdetmode;
	
	uint8_t volTable1;
	uint8_t volTable2;
	uint8_t volTable3;
	uint8_t volTable4;
	uint8_t volTable5;
	uint8_t volTable6;

	uint32_t mileage;
	uint8_t overspeedTime;
	uint8_t blename[30];

    uint8_t agpsUser[50];
    uint8_t agpsPswd[50];

	uint8_t cUpdate;

} SYSTEM_FLASH_DEFINE;

extern SYSTEM_FLASH_DEFINE sysparam;


void paramInit(void);
void paramDefaultInit(uint8_t level);

void paramSaveAll(void);
void paramGetAll(void);


#endif
