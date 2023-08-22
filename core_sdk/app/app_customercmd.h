#ifndef APP_CUSTOMERCMD
#define APP_CUSTOMERCMD

#include "nwy_osi_api.h"

#define MAX_SMS_LIST	30
#define CUSTOMER_DATA_SIZE	2048

typedef enum
{
    CFUN,
    CSQ,
    CPIN,
    CIMI,
    GSN,
    ENPWRSAVE,
    NETAPN,
    CIPOPEN,
    CIPSEND,
    CIPCLOSE,
    NETCLOSE,
    CIPRXGET,
    CNUM,
    CPBW,
    CPSI,
    CMGS,
    CMGR,
    CGNSS,
    CGNSSNMEA,
    CTTSPLAY,
    CTTSSTOP,
    CTTSPARAM,
    CBLE,
    BLEREN,
    BLESEND,
    BLERXGET,
    BLESTA,
    SFTP,
    CUPDATE,
    CICCID,
    CGSV,
} CUSTOMERCMD;


typedef struct
{
    uint16_t cmdid;
    int8_t   cmdstr[20];
} CUSTOMERCMDTABLE;

typedef struct
{
    uint8_t *rxbuf;
    uint16_t rxbufsize;
    uint16_t rxbegin;//指向当前数据段开始的索引
    uint16_t rxend;	 //指向下一个存储的索引
    uint8_t index;

    //void (*rxhandlefun)(uint8_t *, uint16_t len);
    void (*txhandlefun)(uint8_t ind, uint8_t *buf, uint16_t len, uint16_t remain);

} FIFO_DATA_STRUCT;

typedef struct
{
    uint8_t ttsplay	: 1;
    uint8_t msgSend	: 1;
    uint8_t dataSend	;

    uint8_t ttsbuf[250];
    uint8_t msgTel[30];
    uint8_t msgContent[200];
    uint8_t msgLen;
} CUSTOMER_CTL;

typedef struct
{
    uint8_t tel[30];
    uint8_t date[30];
    uint8_t content[128];

} SHORTMESSAGE;

typedef struct
{
    uint8_t smsCount;
    SHORTMESSAGE list[MAX_SMS_LIST];
} SMS_LIST;

typedef struct
{
    uint8_t data[CUSTOMER_DATA_SIZE];
    uint16_t datalen;
} CustomerData;


typedef struct NODE
{
    char * data;
    uint16_t datalen;
    struct NODE *nextnode;
} NODEDATA;



extern FIFO_DATA_STRUCT  bleDataInfo;

void atCustomerCmdParser(const char *buf, uint32_t len);
void customerLogOut(char *str);
void customerLogOutWl(char *str, uint16_t len);
void customerLogPrintf(const char *debug, ...);

void socketInfoInit(void);
int8_t pushRxData(FIFO_DATA_STRUCT *dataInfo, uint8_t *buf, uint16_t len);
void postRxData(FIFO_DATA_STRUCT *dataInfo	, uint16_t postlen);

void customtts(void);

void smsListInit(void);
void addSms(uint8_t *tel, uint8_t *date, uint8_t *content, uint16_t contentlen);
SHORTMESSAGE *readSms(void);
uint8_t getSmsCount(void);

uint8 postCusData(void);
void cusDataInit(void);

void cusBleSendDataInit(void);
void pushCusBleSendData(uint8_t *data, uint16_t len);
void postCusBleSendData(void);

void cusBleRecvParser(char *buf, uint32_t len);
void cusBleCmdSend(char *buf, uint16_t len);

void cusDataSendD(void);
uint8_t bleDataIsSend(void);

void bleSendData(uint8_t *buf, uint16_t len);

uint8_t CreateNodeCmd(char *data, uint16_t datalen);
void outPutNodeCmd(void);

void customGpsInPut(char *nmea, uint8_t nmeaLen);
void customerGpsOutput(void);
void customerRecvCmdParser(char *buf, uint16_t len);


#endif
