#ifndef APP_PORT
#define APP_PORT

#include "nwy_osi_api.h"
#include "nwy_gpio_open.h"

#define DEBUG_NONE			0
#define DEBUG_LOW			1
#define DEBUG_NET			2
#define DEBUG_GPS			3
#define DEBUG_FACTORY		4
#define DEBUG_ALL			9
#define DEBUG_BLE			10


#define GPSLNA_PORT

#define GPSLNAON	nwy_set_pmu_power_level(NWY_POWER_CAMA, 2800)
#define GPSLNAOFF	nwy_set_pmu_power_level(NWY_POWER_CAMA, 0)

#define GPSPOWERON
#define GPSPOWEROFF

#define GPSRSTHIGH
#define GPSRSTLOW


#define LED1_PORT
#define LED2_PORT
#define LED3_PORT


#define LED1_ON		//nwy_gpio_set_value(LED1_PORT,1)
#define LED1_OFF    //nwy_gpio_set_value(LED1_PORT,0)

#define LED2_ON		//nwy_gpio_set_value(LED2_PORT,1)
#define LED2_OFF    //nwy_gpio_set_value(LED2_PORT,0)

#define LED3_ON
#define LED3_OFF


#define GSPWR_GPIO
#define GSINT_PORT

//¸Ð¹â¼ì²â
#define LDR_PORT
#define LDR_DET		//nwy_gpio_get_value(LDR_PORT)

#define DTR_PORT	10
#define DTR_READ	nwy_gpio_get_value(DTR_PORT)

#define RING_PORT	12
#define RING_OUT_HIGH	nwy_gpio_set_value(RING_PORT,1)
#define RING_OUT_LOW	nwy_gpio_set_value(RING_PORT,0)


#define BLE_RING_PORT	26
#define BLE_RING_READ	nwy_gpio_get_value(BLE_RING_PORT)

#define BLE_DTR_PORT	25
#define BLE_DTR_HIGH	nwy_gpio_set_value(BLE_DTR_PORT,1)
#define BLE_DTR_LOW		nwy_gpio_set_value(BLE_DTR_PORT,0)

#define BLE_PWR_PORT	24
#define BLE_PWR_ON		nwy_gpio_set_value(BLE_PWR_PORT,1)
#define BLE_PWR_OFF		nwy_gpio_set_value(BLE_PWR_PORT,0)
#define AUDIOFILE   "myMusic.mp3"

typedef enum
{
    APPUSART1,
    APPUSART2,
    APPUSART3,
} UARTTYPE;

typedef struct
{
    int uart_handle;
    uint8_t init;
} UART_RXTX_CTL;

extern UART_RXTX_CTL usart1_ctl;
extern UART_RXTX_CTL usart2_ctl;
extern UART_RXTX_CTL usart3_ctl;

//UART
void appUartConfig(UARTTYPE type, uint8_t onoff, uint32_t baudrate, void (*rxhandlefun)(char *, uint32_t));
void appUartSend(UART_RXTX_CTL *uart_ctl, uint8_t *buf, uint16_t len);
//RTC
void getRtcDateTime(uint16_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second);
void updateRTCdatetime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
void setNextAlarmTime(void);
void setNextWakeUpTime(void);
//GSENSOR
void gsintPreConfig(void);
void gsensorConfig(uint8_t onoff);
//ADC
void getBatVoltage(void);
//LED
void ledConfig(void);
//PMU
void pmuConfig(void);
//GPS
void gpsConfig(void);
void gpioConfig(void);
//RESET
void appSystemReset(void);
void appSendThreadEvent(uint16 sig, uint32_t param1);
void appTTSPlay(char *ttsbuf);
void appTTSStop(void);

void appSaveAudio(uint8_t *audio, uint16_t len);
void appDeleteAudio(void);
void appPlayAudio(void);

void portBleGpioCfg(void);

#endif

