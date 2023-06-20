#ifndef APP_TASK
#define APP_TASK
#include "nwy_osi_api.h"

#define SYSTEM_LED_RUN				0x01
#define SYSTEM_LED_LOGIN			0x02
#define SYSTEM_LED_GPS				0x04
#define SYSTEM_LED_WDT				0x08

#define SYS_LED1_CTRL				1
#define SYS_LED2_CTRL				2

#define GPS_REQ_UPLOAD_ONE_POI		0x01
#define GPS_REQ_ACC					0x02
#define GPS_REQ_KEEPON				0x04
#define GPS_REQ_DEBUG				0x08
#define GPS_REQ_BLE					0x10
#define GPS_REQ_MOVE				0x20

#define GPS_REQ_ALL					0xFF

#define ALARM_LIGHT_REQUEST			0x00000001 //感光
#define ALARM_LOSTV_REQUEST			0x00000002 //断电
#define ALARM_LOWV_REQUEST			0x00000004 //低电
#define ALARM_SHUTTLE_REQUEST		0x00000008//震动报警

#define ALARM_GUARD_REQUEST			0X00000010
#define ALARM_BLE_LOST_REQUEST		0X00000020
#define ALARM_BLE_RESTORE_REQUEST	0X00000040
#define ALARM_OIL_RESTORE_REQUEST	0X00000080
#define ALARM_SHIELD_REQUEST		0X00000100
#define ALARM_PREWARN_REQUEST		0X00000200
#define ALARM_OIL_CUTDOWN_REQUEST	0X00000400
#define ALARM_BLE_ERR_REQUEST		0X00000800
#define ALARM_GPS_NO_FIX_REQUEST	0X00001000
#define ALARM_TRIAL_REQUEST			0X00002000

#define ALARM_SHUTDOWN_REQUEST		0X00004000
#define ALARM_UNCAP_REQUEST			0X00008000
#define ALARM_SIMPULLOUT_REQUEST    0x00010000


#define ACCDETMODE0					0
#define ACCDETMODE1					1
#define ACCDETMODE2					2

#define GPS_UPLOAD_GAP_MAX			60
//GPS_UPLOAD_GAP_MAX 以下，gps常开，以上(包含GPS_UPLOAD_GAP_MAX),周期开启
#define SHUTDOWN_TIME				300
typedef enum
{
    THREAD_EVENT_PLAY_AUDIO = NWY_APP_EVENT_ID_BASE + 1,
    THREAD_EVENT_PLAY_REC,
    THREAD_EVENT_REC,
    THREAD_EVENT_SOCKET_CLOSE,
    THREAD_EVENT_BLE_CLIENT,
} thread_Event_e;

typedef enum
{
    THREAD_PARAM_REC_START,
    THREAD_PARAM_REC_STOP,

} thread_param_e;

typedef enum
{
    MOTION_STATIC = 0,
    MOTION_MOVING,
} motionState_e;

typedef enum
{
    GPS_STATE_IDLE,
    GPS_STATE_WAIT,
    GPS_STATE_RUN,
} gpsState_e;


typedef enum
{
    SYS_START,
    SYS_RUN,
    SYS_STOP,
    SYS_WAIT,
} runState_e;

typedef enum
{
    MODE1 = 1,
    MODE2,
    MODE3,
    MODE21,
    MODE23,
} system_mode_s;

typedef enum
{
    ACC_SRC,
    VOLTAGE_SRC,
    GSENSOR_SRC,
} motion_src_e;


typedef struct
{
    uint32_t sys_tick;		//记录系统运行时间
    uint8_t sysStatus;
    uint8_t sysLed1Onoff;
    uint8_t sysLed1OnTime;
    uint8_t sysLed1OffTime;
    uint8_t sysLed2Onoff;
    uint8_t sysLed2OnTime;
    uint8_t sysLed2OffTime;
} sysLedCtrl_s;

typedef struct
{
    uint8_t ind;
    motionState_e motionState;
    uint8_t tapInterrupt;
    uint8_t tapCnt[5];
} motionInfo_s;




typedef struct
{
    uint8_t gpsOnoff			: 1;
    uint8_t lbsRequest			: 1;
    uint8_t wifiRequest			: 1;
    uint8_t agpsRequest			: 1;
    uint8_t upgradeDoing		: 1;
    uint8_t nmeaOutput			: 1;
    uint8_t gsensorOnoff		: 1;
    uint8_t gsensorErr			: 1;
    uint8_t updateLocalTimeReq	: 1;
    uint8_t flag123				: 1;
    uint8_t hiddenServCloseReq	: 1;
    uint8_t ttsPlayNow			: 1;
    uint8_t doRelayFlag			: 1;
    uint8_t wdtTest				: 1;
    uint8_t recTest				: 1;
    uint8_t bleOnBySystem		: 1;
	uint8_t dbFileUpload;
    uint8_t logLevel;
    uint8_t gpsRequest;
    uint8_t runFsm;
    uint8_t recordTime;
    uint8_t softWdtTick;
	uint8_t debu;

    uint32_t alarmrequest;

    uint32_t sysTick;
    uint32_t sysStartTick;
    uint32_t gpsUpdateTick;
    uint32_t gpsLastFixTick;

    float outsideVol;
    float batteryVol;

} systemInfo_s;


extern systemInfo_s sysinfo;

void ledStatusUpdate(uint8_t status, uint8_t onoff);


void motionOccur(void);
motionState_e motionGetStatus(void);

void gpsRequestSet(uint32_t flag);
void gpsRequestClear(uint32_t flag);
uint8_t gpsRequestGet(uint32_t flag);
void lbsRequestSet(void);
void lbsRequestClear(void);

void wifiRequestSet(void);
void wifiRequestClear(void);

void alarmRequestSet(uint32_t request);
void alarmRequestClear(uint32_t request);
void alarmRequestSave(uint32_t request);
void alarmRequestClearSave(void);


uint8_t recordRequestSet(uint8_t recordTime);
void recordRequestClear(void);

uint8_t sysIsInRun(void);
void sysResetStartRun(void);

void appSendThreadEvent(uint16 threadEvent, uint32_t param1);

void relayAutoRequest(void);
void relayAutoClear(void);

void myAppRun(void *param);
void myApp100MSRun(void *param);
void myAppEvent(void *param);

#endif

