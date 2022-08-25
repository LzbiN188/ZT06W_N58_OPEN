#ifndef APP_NET
#define APP_NET
#include "nwy_osi_api.h"

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
} CMD_TYPE;
/*指令集对应结构体*/
typedef struct
{
    CMD_TYPE cmd_type;
    char cmd[30];
} CMD_STRUCT;



typedef enum
{
    NET_CHECK_SIM,
    NET_GET_SIM_INFO,
    NET_CHECK_SIGNAL,
    NET_CHECK_REGISTER,
    NET_CHECK_DATACALL,
    NET_CHECK_SOCKET,
    NET_CHECK_NORMAL,
} NET_FSM_STATE;

typedef struct
{
    uint8_t domainIP		: 1;
    uint8_t socketOpen		: 1;
    uint8_t socketConnect	: 1;
    uint8_t network			: 1;
    uint8_t fsmState;
    uint8_t dataCallFsm;

    uint8_t networkRegisterCount;
    uint8_t dataCallCount;
    uint8_t socketConnectCount;

    int8_t dataServerHandle;//数据拨号句柄
    int8_t dataCallState;
    int8_t socketState;
    uint16_t tickTime;
    int socketId;
    char ICCID[21];
    char IMSI[16];
    char IMEI[16];
	char MAC[17];
} NET_INFO;
typedef struct
{
    uint8_t domainIP	: 1;
    uint8_t socketOpen	: 1;
    uint8_t socketConnect: 1;
    int socketId;
} AGPS_INFO;

typedef enum
{
    DATA_CALL_GET_SOURSE,
    DATA_CALL_START,
    DATA_CALL_STOP,
    DATA_CALL_RELEALSE,
} DATA_CALL_FSM;

uint8_t  sendModuleCmd(uint8_t cmd, char *param);
void setAPN(void);
void setXGAUTH(void);
void setCLIP(void);
uint8_t isNetProcessNormal(void);

uint8_t getRegisterState(void);

void networkCtl(uint8_t onoff);


void reConnectServer(void);

char getModuleRssi(void);
char *getModuleIMSI(void);
char *getModuleICCID(void);
char *getModuleIMEI(void);
char *getModuleMAC(void);
void sendDataToServer(uint8_t link, uint8_t *data, uint16_t len);
uint8_t sendShortMessage(char *tel, char *content, uint16_t len);

uint32_t netIpChange(char *ip);
void getBleMac(void);
uint8_t getSimInfo(void);
char *readModuleIMEI(void);

#endif

