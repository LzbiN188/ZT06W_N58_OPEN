#ifndef APP_SYS
#define APP_SYS
#include "nwy_osi_api.h"
#include "app_port.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"

#define MODE1						1
#define MODE2						2
#define MODE3						3
#define MODE4						4
#define MODE5						5
#define MODE21						6  //模式2的扩展，类似模式1
#define MODE23						7  //模式2的扩展，类似模式3
#define MODEFACTORY					99 //未激活模式

#define ITEMCNTMAX					8
#define ITEMSIZEMAX					150

#define USE_JT808_PROTOCOL			8

#define ACCDETMODE0					0
#define ACCDETMODE1					1
#define ACCDETMODE2					2

typedef enum
{
    RECORD_UPLOAD_EVENT = NWY_APP_EVENT_ID_BASE + 1,
    GET_FIRMWARE_EVENT,
    PLAY_MUSIC_EVENT,
    CFUN_EVENT,
} SYSTEM_THREAD_EVENT;

typedef enum
{
    THREAD_PARAM_NONE,
    THREAD_PARAM_AUDIO,
    THREAD_PARAM_SIREN,
} SYSTEM_THREAD_PARAM;

typedef enum
{
    TERMINAL_WARNNING_NORMAL = 0, /*0 		*/
    TERMINAL_WARNNING_SHUTTLE,    /*1：震动报警      */
    TERMINAL_WARNNING_LOSTV,      /*2：断电报警      */
    TERMINAL_WARNNING_LOWV,       /*3：低电报警      */
    TERMINAL_WARNNING_SOS,        /*4：SOS求救   */
    TERMINAL_WARNNING_CARDOOR,    /*5：车门求救     */
    TERMINAL_WARNNING_SWITCH,     /*6：开关   */
    TERMINAL_WARNNING_LIGHT,      /*7：感光报警     */
} TERMINAL_WARNNING_TYPE;


typedef struct
{
    unsigned char item_cnt;
    char item_data[ITEMCNTMAX][ITEMSIZEMAX];
} ITEM;

typedef struct
{
    uint8_t nmeaoutputonoff   		: 1;
    uint8_t lbsrequest				: 1;
    uint8_t wifirequest				: 1;
    uint8_t netModuleState			: 1; /*MTK电源状态*/
    uint8_t localrtchadupdate		: 1;
    uint8_t debuguartctrl			: 1;
    uint8_t flag123					: 1;
    uint8_t blerequest				: 1;
    uint8_t gsensoronoff			: 1;
    uint8_t gsensorerror			: 1;
    uint8_t recordingflag			: 1;
    uint8_t hearbeatrequest			: 1;
    uint8_t instructionqequest		: 1;
    uint8_t bleStopFlag				: 1;
    uint8_t systemSleepStatus		: 1;
    uint8_t updateStatus			: 1;
    uint8_t playMusic				: 1;
    uint8_t playMusicNow			: 1;
    uint8_t ttsPlayNow				: 1;
    uint8_t taskSuspend				: 1;
    uint8_t bleOnoff				: 1;
    uint8_t dtrState				: 1;
    volatile uint8_t outputDelay				: 1;
    volatile uint8_t gpsOutPut		: 1;
    uint8_t bleRing;
    uint8_t runFsm;
    uint8_t GPSRequestFsm;
    uint8_t GPSStatus;
    uint8_t gsensortapcount;
    uint8_t rollinterruptcount;
    uint8_t speedinterruptcount;
    uint8_t terminalStatus;
    uint8_t modulepowerstate;
    uint8_t systempowerlow;
    uint8_t systemledstatus;
    uint8_t SystaskID;
    uint8_t onetaprecord[15];
    uint8_t rollrecord[15];
    uint8_t speedrecord[15];
    uint8_t logmessage;
    uint8_t mnc;
    uint8_t  accOffDetTick;
    uint8_t mcuVersion[30];


    uint16_t systemRequest;
    uint16_t gpsuploadgap;
    uint16_t gpsuploadonepositiontime;
    uint16_t mcc;
    uint16_t lac;
    uint16_t alarmrequest;
    int16_t accuracyCode;

    uint32_t GPSRequest;	  /*GPS 开关请求*/
    uint32_t System_Tick;    /*系统节拍*/
    uint32_t runStartTick;  /*开机节拍*/
    uint32_t gpsUpdatetick;
    uint32_t agpsOpenTick;
    uint32_t cid;
    uint32_t csqSearchTime;
    uint32_t gpsLastFixedTick;

    float outsidevoltage;
    float batvoltage;
    float lowvoltage;

} SYSTEM_INFO;

typedef struct
{
    uint8_t recfsm;
    uint8_t recordUpload;
    uint8_t ticktime;
    uint8_t counttype1;
    uint8_t counttype2;
    uint8_t recUploadFileName[20];
    uint8_t *recUploadContent;

    uint32_t recUploadFileSize;
    uint32_t hadRead;
    uint32_t needRead;

} REC_UPLOAD_INFO;

extern SYSTEM_INFO sysinfo;


extern nwy_osiThread_t *myAppEventThread;
extern nwy_osiThread_t *myAppThread;


void nwy_ext_echo(char *fmt, ...);
void LogPrintf(uint8_t level, const char *debug, ...);
void LogMessage(uint8_t level, char *debug);
void LogMessageWL(uint8_t level, char *buf, uint16_t len);
void LogWrite(uint8_t level, char *buf, uint16_t len);


void terminalDefense(void);
void terminalDisarm(void);
uint8_t getTerminalAccState(void);
void terminalAccon(void);
void terminalAccoff(void);
void terminalCharge(void);
void terminalunCharge(void);
uint8_t getTerminalChargeState(void);
void terminalGPSFixed(void);
void terminalGPSUnFixed(void);
void terminalAlarmSet(TERMINAL_WARNNING_TYPE alarm);

unsigned short GetCrc16(const char *pData, int nLength);

int getCharIndex(uint8_t *src, int src_len, char ch);
int my_strpach(char *str1, const char *str2);
int my_strstr(char *str1, const char *str2, int str1_len);
int my_getstrindex(char *str1, const char *str2, int len);
int distinguishOK(char *buf);
int16_t getCharIndexWithNum(uint8_t *src, uint16_t src_len, uint8_t ch, uint8_t num);
void changeByteArrayToHexString(uint8_t *src, uint8_t *dest, uint16_t srclen);
int16_t changeHexStringToByteArray(uint8_t *dest, uint8_t *src, uint16_t size);
int16_t changeHexStringToByteArray_10in(uint8_t *dest, uint8_t *src, uint16_t size);
void parserInstructionToItem(ITEM *item, uint8_t *str, uint16_t len);

uint8_t mycmdPatch(uint8_t *cmd1, uint8_t *cmd2);

void updateRTCtimeRequest(void);

#endif

