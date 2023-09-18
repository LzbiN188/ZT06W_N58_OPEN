#ifndef APP_PORT
#define APP_PORT

#include "nwy_osi_api.h"
#include "nwy_usb_serial.h"
#include "nwy_gpio_open.h"
#include "nwy_pm.h"

#include "nwy_loc.h"
#define LED1_PORT
#define LED2_PORT		25

#define LED1_ON			//nwy_gpio_set_value(LED1_PORT,1)
#define LED1_OFF    	//nwy_gpio_set_value(LED1_PORT,0)

#define LED2_ON			nwy_gpio_set_value(LED2_PORT,1)
#define LED2_OFF    	nwy_gpio_set_value(LED2_PORT,0)


#define GSINT_PORT		2
#define GSPWR_ON		nwy_set_pmu_power_level(NWY_POWER_CAMD, 1800)
#define GSPWR_OFF		nwy_set_pmu_power_level(NWY_POWER_CAMD, 0)


#define LDR_PORT		3
#define LDR_READ		nwy_gpio_get_value(LDR_PORT)

#define LDR2_PORT		12
#define LDR2_READ		nwy_gpio_get_value(LDR2_PORT)

#define RELAY_PORT		28
#define RELAY_ON		nwy_gpio_set_value(RELAY_PORT,1)
#define RELAY_OFF		nwy_gpio_set_value(RELAY_PORT,0)

#define ACC_PORT		0
#define ACC_READ		nwy_gpio_get_value(ACC_PORT)
#define ACC_STATE_ON	0
#define ACC_STATE_OFF	1

#define WDT_PORT		24
#define WDT_FEED_H		nwy_gpio_set_value(WDT_PORT,1)
#define WDT_FEED_L		nwy_gpio_set_value(WDT_PORT,0)

#define ONOFF_PORT		27
#define ONOFF_READ		nwy_gpio_get_value(ONOFF_PORT)
#define ON_STATE		1
#define OFF_STATE		0

#define GPSLNA_ON		nwy_set_pmu_power_level(NWY_POWER_CAMA, 1800)
#define GPSLNA_OFF		nwy_set_pmu_power_level(NWY_POWER_CAMA, 0)

#define AUDIOFILE   "myMusic.mp3"
#define RECFILE		"rec.amr"
#define RECORD_BUFF_SIZE		(120*1024)

typedef enum
{
    SIM_1=0,
    SIM_2,
} SIM_USE;

typedef enum
{
    APPUSART1,
    APPUSART2,
    APPUSART3,
} uarttype_e;

typedef struct
{
    uint8_t init;
    int uart_handle;
} usartCtrl_s;

typedef struct
{
    uint8_t mnc;
    uint16_t mcc;
    uint16_t lac;
    uint32_t cid;
} lbsInfo_s;

typedef struct
{
    uint8_t recDoing;
    char recFileName[25];
    uint8_t recBuff[RECORD_BUFF_SIZE];
    uint32_t recSize;
} record_s;

/*定义AT指令集*/
typedef enum
{
    AT_CMD = 1,
    NWBTBLEMAC_CMD,
    CLIP_CMD,
    CFUN_CMD,
    CGDCONT_CMD,
    XGAUTH_CMD,
    NWBTBLEMAC,
    MAX_NUM,
} atCmdType_e;
/*指令集对应结构体*/
typedef struct
{
    atCmdType_e cmd_type;
    char *cmd;
} moduleAtCmd_s;

typedef struct ttsList
{
    uint8_t ttsBuf[144];
    uint8_t ttsBufLen;
    struct ttsList *next;
} tts_info_s;

typedef enum
{
    HOT_START = 0,
    WORM_START = 1,
    COLD_START = 2,
    FACTORY_START = 3,
} gps_startUpmode_e;

typedef enum
{
    PDP_IP = 1,
    PDP_IPV6,
    PDP_IPV4V6,
} pdp_type_e;
typedef enum
{
    AUTH_NONE,
    AUTH_PAP,
    AUTH_CHAP,
} auth_type_e;


extern usartCtrl_s usart1_ctl;
extern usartCtrl_s usart2_ctl;
extern usartCtrl_s usart3_ctl;


void portUSBCfg(nwy_sio_recv_cb_t usbRecvCallback);
void portUsbSend(char *data, uint16_t len);

void portUartCfg(uarttype_e type, uint8_t onoff, uint32_t baudrate, void (*rxhandlefun)(char *, uint32_t));
void portUartSend(usartCtrl_s *uart_ctl, uint8_t *buf, uint16_t len);

void portGetRtcDateTime(uint16_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
void portSetRtcDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second,
                        int8_t utc);
void setNextAlarmTime(uint8_t gapDay, uint16_t *AlarmTime);
void setNextWakeUpTime(uint16_t gapMinutes);



void portSleepCtrl(uint8_t mode);

void portGpioCfg(void);
void portPmuCfg(void);


void portGsensorCfg(uint8_t onoff);

void portGpsCtrl(uint8_t onoff);
void portGpsSetPositionMode(nwy_loc_position_mode_t pos_mode);
void portGpsGetNmea(void (*gpsRecv)(uint8_t *, uint16_t));

float portGetOutSideVolAdcVol(void);
float portGetBatteryAdcVol(void);

uint8_t portGetModuleRssi(void);
void portGetModuleICCID(char *iccid);
void portGetModuleIMSI(char *imsi);
int portGetModuleIMEI(char *imei);
lbsInfo_s portGetLbsInfo(void);

int portStartAgps(char *agpsServer, int agpsPort, char *agpsUser, char *agpsPswd);
int portSendAgpsData(char *data, uint16_t len);
int portSetGpsStarupMode(gps_startUpmode_e mode);


void portSystemReset(void);
void portSystemShutDown(void);


void portSaveAudio(char *filePath, uint8_t *audio, uint16_t len);
void portDeleteAudio(char *filePath);
void portPlayAudio(char *filePath);

uint8_t portUpgradeWirte(unsigned int offset, unsigned int length, unsigned char *data);
uint8_t portUpgradeStart(void);

void portRecInit(void);
int portRecStart(void);
void portRecStop(void);
void portRecUpload(void);
uint8_t portIsRecordNow(void);

void portSetRadio(uint8_t onoff);

uint8_t  portSendCmd(uint8_t cmd, char *param);

void setAPN(void);
void setXGAUTH(void);
void portAtCmdInit(void);

void portSendSms(char *tel, char *content, uint16_t len);
void portSetVol(uint8_t vol);
uint8_t portCapacityCalculate(float vol);

void portOutputTTS(void);
void portPushTTS(char *ttsbuf);

void portSetApn(char *apn, char *userName, char *password);

void portSimSet(SIM_USE sim);
SIM_USE portSimGet(void);


#endif

