#ifndef APP_NET
#define APP_NET

#include "nwy_osi_api.h"

#define NORMAL_LINK		0
#define JT808_LINK		1
#define HIDE_LINK		2
#define AGPS_LINK		3
#define UPGRADE_LINK	5


#define REC_UPLOAD_ONE_PACK_SIZE 4096


typedef enum
{
    SERV_LOGIN,
    SERV_LOGIN_WAIT,
    SERV_READY,
} serverfsm_e;

typedef struct
{
    serverfsm_e connectFsm;
    uint8_t 	loginCnt;
    uint16_t	runTick;
} serverConnect_s;
typedef enum
{
    REC_REQ_SEND,
    REC_REQ_WAIT,
    REC_DATA_SEND,
    REC_DATA_WAIT,
} recordUploadFsm_e;

typedef enum
{
    JT808_REGISTER,
    JT808_AUTHENTICATION,
    JT808_NORMAL,
} jt808_connfsm_s;



typedef struct
{
    jt808_connfsm_s connectFsm;
    uint8_t runTick;
    uint8_t regCnt;
    uint8_t authCnt;
    uint16_t hbtTick;
} jt808_Connect_s;


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

} recordUpload_s;


void serverLoginRespon(void);
uint8_t serverIsReady(void);
void serverReconnect(void);

void hiddenServCloseRequest(void);
void hiddenServCloseClear(void);

void hiddenServLoginRespon(void);
void hiddenServReconnect(void);
uint8_t hiddenServIsReady(void);


void upgradeRunTask(void);

void agpsRequestSet(void);
void agpsRequestClear(void);
void agpsConnRunTask(void);


uint8_t isRecUploadRun(void);
void recSetUploadFile(uint8_t *filename, uint32_t filesize, uint8_t *recContent);

void recUploadRsp(void);
void recordUploadTask(void);


void jt808serverReconnect(void);
void jt808AuthOk(void);

void serverConnTask(void);


#endif
