#ifndef APP_TASK
#define APP_TASK
#include "nwy_osi_api.h"

/*----------------------------------------------*/

#define GPSLEDON    		LED1_ON
#define GPSLEDOFF   		LED1_OFF

#define SIGNALLEDON			LED2_ON
#define SIGNALLEDOFF		LED2_OFF

#define SERVERLEDON			//LED3_ON
#define SERVERLEDOFF		//LED3_OFF

#define SYSTEM_LED_RUN				0X01
#define SYSTEM_LED_NETREGOK			0X02
#define SYSTEM_LED_GPSOK			0X04
#define SYSTEM_LED_SERVEROK	  		0X08
#define SYSTEM_LED_UPDATE			0X10

#define NETREGLED					0
#define GPSLED						1
#define SERVERLED 					2
/*----------------------------------------------*/

#define ALARM_LIGHT_REQUEST			0x0001 //感光
#define ALARM_LOSTV_REQUEST			0x0002 //断电
#define ALARM_LOWV_REQUEST			0x0004 //低电
#define ALARM_SHUTTLE_REQUEST		0x0008//震动报警
#define ALARM_ACCLERATE_REQUEST		0X0010
#define ALARM_DECELERATE_REQUEST	0X0020
#define ALARM_RAPIDRIGHT_REQUEST	0X0040
#define ALARM_RAPIDLEFT_REQUEST		0X0080
#define ALARM_GUARD_REQUEST			0X0100
#define ALARM_BLE_REQUEST			0X0200
/*----------------------------------------------*/

#define GPS_REQUEST_UPLOAD_ONE			0X00000001
#define GPS_REQUEST_ACC_CTL				0X00000002
#define GPS_REQUEST_GPSKEEPOPEN_CTL		0X00000004
#define GPS_REQUEST_DEBUG_CTL			0X00000008
#define GPS_REQUEST_BLEUPLOAD_CTL		0X00000010
#define GPS_REQUEST_CUSTOMER_CTL		0X00000020
/*----------------------------------------------*/

#define SYSTEM_MODULE_STARTUP_REQUEST	0X0001
#define SYSTEM_MODULE_SHUTDOWN_REQUEST	0X0002
#define SYSTEM_ENTERSLEEP_REQUEST		0X0004
#define SYSTEM_POWERLOW_REQUEST			0X0008
#define SYSTEM_CLOSEUART1_REQUEST		0X0010
#define SYSTEM_OPENUART1_REQUEST		0X0020
#define SYSTEM_WDT_REQUEST				0X0040
#define SYSTEM_BLE_START_REQUEST		0X0080
#define SYSTEM_BLE_STOP_REQUEST		    0X0100
/*----------------------------------------------*/

#define MODE_START		0
#define MODE_RUNING		1
#define MODE_STOP		2
#define MODE_DONE		3

/*----------------------------------------------*/

#define REC_UPLOAD_ONE_PACK_SIZE 4096
/*----------------------------------------------*/

typedef enum
{
    GPSCLOSESTATUS,
    GPSOPENWAITSTATUS,
    GPSOPENSTATUS,

} GPSREQUESTFSMSTATUS;
/*----------------------------------------------*/


typedef struct
{
    uint32_t sys_tick;		//记录系统运行时间
    uint8_t sys_led_onoff;
    uint8_t sys_led_on_time;
    uint8_t sys_led_off_time;

    uint8_t sys_gps_led_onoff;
    uint8_t sys_gps_led_on_time;
    uint8_t sys_gps_led_off_time;

    uint8_t sys_server_led_onoff;
    uint8_t sys_server_led_on_time;
    uint8_t sys_server_led_off_time;
} SystemLEDInfo;

typedef enum{
	BLE_CHECK_STATE,
	BLE_OPEN,
	BLE_CLOSE,
	BLE_CONFIG,
	BLE_NORMAL,
}BLE_FSM;
typedef struct{
	uint8_t bleState	:1;
	uint8_t bleFsm;
}BLE_INFO;


void updateSystemLedStatus(uint8_t status, uint8_t onoff);

void alarmRequestSet(uint16_t request);
void alarmRequestClear(uint16_t request);
void gpsRequestOpen(void);
void gpsRequestClose(void);

void appBleRestart(void);

void gpsRequestSet(uint32_t flag);
void gpsRequestClear(uint32_t flag);
uint8_t gpsRequestGet(uint32_t flag);

void resetModeStartTime(void);
uint8_t isModeRun(void);
void setUploadFile(uint8_t *filename, uint32_t filesize, uint8_t *recContent);
void recordUploadRespon(void);
void uartCtl(uint8_t onoff);
uint8_t systemIsIdle(void);


void myAppRun(void *param);
void myAppEvent(void *param);

void myAppRunIn100Ms(void * param);
void myAppCusPost(void *param);

#endif

