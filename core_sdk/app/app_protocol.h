#ifndef APP_PROTOCOL
#define APP_PROTOCOL
#include "nwy_osi_api.h"
#include "app_gps.h"

#define NORMAL_LINK 5
#define DOUBLE_LINK 4
#define GPS_RESTORE_FILE_NAME	"gps.save"

#define UPDATE_MODULE_OBJECT		0
#define UPDATE_MCU_OBJECT			1



typedef enum
{
    PROTOCOL_01,//登录
    PROTOCOL_12,//定位
    PROTOCOL_13,//心跳
    PROTOCOL_16,//定位
    PROTOCOL_19,//多基站
    PROTOCOL_21,//蓝牙
    PROTOCOL_8A,//TIME
    PROTOCOL_F1,//ICCID
    PROTOCOL_F3,//WIFI
    PROTOCOL_UP,
    PROTOCOL_51,
    //PROTOCOL_52,
    //PROTOCOL_53,
} PROTOCOLTYPE;

typedef enum
{
    NETWORK_LOGIN,
    NETWORK_LOGIN_WAIT,
    NETWORK_LOGIN_READY,


    NETWORK_DOWNLOAD_DOING,
    NETWORK_DOWNLOAD_WAIT,

    NETWORK_MCU_START_UPGRADE,
    NETWORK_MCU_EARSE,
    NETWORK_FIRMWARE_WRITE_DOING,

    NETWORK_DOWNLOAD_DONE,
    NETWORK_UPGRAD_NOW,
    NETWORK_DOWNLOAD_ERROR,
    NETWORK_WAIT_JUMP,
    NETWORK_DOWNLOAD_END,
    NETWORK_UPGRADE_CANCEL,
} NetWorkFsmState;
typedef struct
{
    NetWorkFsmState fsmstate;
    unsigned int heartbeattick;
    unsigned short serial;
    uint8_t logintick;
    uint8_t loginCount;
    uint8_t getVerCount;
} NetWorkConnectStruct;

typedef struct
{
    uint8_t ssid[6];
    int8_t signal;
} WIFI_ONE;

typedef struct
{
    WIFI_ONE ap[16];
    uint8_t apcount;
} WIFI_INFO;

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t latititude[4];
    uint8_t longtitude[4];
    uint8_t speed;
    uint8_t coordinate[2];
    uint8_t temp[3];
} GPSRestoreStruct;

typedef struct
{
    char curCODEVERSION[50];
    char newCODEVERSION[50];
    char rxsn[50];
    char rxcurCODEVERSION[50];
    uint8_t updateOK;
    uint8_t updateObject;
    uint32_t file_id;
    uint32_t file_offset;
    uint32_t file_len;
    uint32_t file_totalsize;

    uint32_t rxfileOffset;//已接收文件长度
} UndateInfoStruct;


typedef struct
{
    uint8_t audioType;
    uint16_t audioCnt;
    uint32_t audioSize;
    uint32_t audioId;
} AudioFileStruct;

void netConnectReset(void);
void protocolRunFsm(void);
void protocolReceivePush(uint8_t line, char *protocol, int size);
void sendProtocolToServer(uint8_t link, PROTOCOLTYPE protocol, void *param);
uint8_t isProtocolReday(void);

void save123InstructionId(void);
void reCover123InstructionId(void);
uint8_t *getInstructionId(void);

void updateMcuVersion(char *version);
void updateUISInit(uint8_t object);
void UpdateProtocolRunFsm(void);

uint8_t getBatteryLevel(void);
void gpsRestoreWriteData(GPSRestoreStruct *gpsres);

uint8_t gpsRestoreReadData(void);

void createProtocol61(char *dest, char *datetime, uint32_t totalsize, uint8_t filetye, uint16_t packsize);
void createProtocol62(char *dest, char *datetime, uint16_t packnum, uint8_t *recdata, uint16_t reclen);

void getFirmwareInThreadEvent(void);
void UpdateStop(void);


void protocolReceivePush(uint8_t line, char *protocol, int size);
void upgradeResultProcess(uint8_t upgradeResult, uint32_t offset, uint32_t size);

#endif




