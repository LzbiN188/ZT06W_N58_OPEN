#ifndef APP_PROTOCOL
#define APP_PROTOCOL

#include "nwy_osi_api.h"

#define PROTOCOL_BUFSZIE	6144  //6k

#define GPS_RESTORE_FILE_NAME	"gps.save"


typedef enum
{
    PROTOCOL_01,	//登录
    PROTOCOL_12,	//定位
    PROTOCOL_13,	//心跳

    PROTOCOL_16,
    PROTOCOL_19,
    PROTOCOL_51,
    PROTOCOL_52,
    PROTOCOL_53,
    PROTOCOL_61,
    PROTOCOL_62,
    PROTOCOL_8A,	//时间
    PROTOCOL_F1,	//信息
    PROTOCOL_F6,
    PROTOCOL_F3,
    PROTOCOL_21,
    PROTOCOL_UP,
    PROTOCOL_A0,	//AGPS
} protocol_type_e;

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
} terminal_warnning_type_e;


typedef enum
{
    NETWORK_LOGIN,
    NETWORK_LOGIN_WAIT,
    NETWORK_LOGIN_READY,


    NETWORK_DOWNLOAD_DOING,
    NETWORK_DOWNLOAD_WAIT,
    NETWORK_DOWNLOAD_DONE,
    NETWORK_UPGRAD_NOW,
    NETWORK_DOWNLOAD_ERROR,
    NETWORK_WAIT_JUMP,
    NETWORK_DOWNLOAD_END
} upgrade_fsm_e;



typedef struct
{
    uint8_t ssid[6];
    int8_t signal;
} wifiInfo_s;
typedef struct
{
    wifiInfo_s ap[16];
    uint8_t apcount;
} wifiList_s;


typedef struct
{
    char sn[20];
    char iccid[21];
    char imsi[21];
    char bleMac[17];
    uint8_t terminalStatus;
    uint8_t gpsSatelliteUsed;
    uint8_t beidouSatelliteUsed;
    uint8_t rssi;
    uint8_t batteryLevel;
    uint8_t instructionId[4];
    uint8_t instructionIdSave[4];
    uint8_t instructionIdBle[4];
    uint8_t mnc;
    uint8_t event;
    uint8_t slaverMac[5][6];
    uint8_t slaverCnt;

    uint16_t Serial;
    uint16_t mcc;
    uint16_t lac;

    uint16_t startUpCnt;
    uint16_t runTime;

    uint32_t cid;
    float	outsideVol;
    float   batteryVol;

    wifiList_s wifiList;

    int (*tcpSend)(uint8_t, uint8_t *, uint16_t);
} protocol_s;

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
} gpsRestore_s;


typedef struct
{
    uint8_t audioType;
    uint16_t audioCnt;
    uint16_t audioCurPack;
    uint32_t audioSize;
    uint32_t audioId;
} audioDownload_s;

typedef struct
{
    char curCODEVERSION[50];
    char newCODEVERSION[50];
    char rxsn[50];
    char rxcurCODEVERSION[50];


    uint8_t updateOK;
    uint8_t upgradeFsm;
    uint8_t runTick;
    uint8_t loginCnt;
    uint8_t getVerCnt;
    uint8_t waitTimeOutCnt;
    uint8_t validFailCnt;
    uint8_t dlErrCnt;

    uint32_t file_id;
    uint32_t file_offset;
    uint32_t file_len;
    uint32_t file_totalsize;
    uint32_t rxfileOffset;//已接收文件长度
} upgradeInfo_s;

typedef struct
{
    char *dest;
    char *dateTime;
    uint8_t fileType;
    uint8_t *recData;
    uint16_t packNum;
    uint16_t recLen;
    uint32_t totalSize;
    uint16_t packSize;
} recordUploadInfo_s;




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
void terminalAlarmSet(terminal_warnning_type_e alarm);

void saveInstructionId(void);
void recoverInstructionId(void);
uint8_t *getProtoclInstructionid(void);

void createProtocolA0(char *dest, uint16_t *len);

uint16_t GetCrc16(const char *pData, int nLength);

int tcpSendData(uint8_t link, uint8_t *txdata, uint16_t txlen);

void sendProtocolToServer(uint8_t link, int type, void *param);
void socketRecvPush(uint8_t link, char *protocol, int size);

void protocolInit(void);

void protoclUpdateSn(char *sn);
void protocolUpdateIccid(char *iccid);
void protocolUpdateImsi(char *imsi);
void protocolUpdateSomeInfo(float outvol, float batvol, uint8_t batlev, uint16_t startCnt, uint16_t runTime);
void protocolUpdateRssi(uint8_t rssi);
void protocolUpdateSatelliteUsed(uint8_t gps, uint8_t bd);
void protocolUpdateLbsInfo(uint16_t mcc, uint8_t mnc, uint16_t lac, uint32_t cid);
void protocolUpdateBleMac(char *mac);
void protocolUpdateWifiList(wifiList_s *wifilist);
void protocolUpdateEvent(uint8_t event);

void protocolRegisterTcpSend(int (*tcpSend)(uint8_t, uint8_t *, uint16_t));
void protocolUpdateSlaverMac(void);

void upgradeStartInit(void);
void upgradeFromServer(void);

void gpsRestoreDataSend(gpsRestore_s *grs, char *dest	, uint16_t *len);
uint8_t gpsRestoreWriteData(gpsRestore_s *gpsres, uint8_t num);

uint8_t gpsRestoreReadData(void);

void getInsid(void);
void setInsId(void);


#endif
