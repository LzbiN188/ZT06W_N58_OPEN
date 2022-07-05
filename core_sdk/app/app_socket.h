#ifndef APP_SOCKET
#define APP_SOCKET

#include "nwy_osi_api.h"
#include "nwy_socket.h"


#define SOCKET_LIST_MAX	6



typedef enum
{
    CHECK_SIM,
    CHECK_SIGNAL,
    CHECK_REGISTER,
    CHECK_DATACALL,
    CHECK_SOCKET,
} socketFsm_e;


typedef enum
{
    DATA_CALL_GET_SOURSE,
    DATA_CALL_START,
    DATA_CALL_STOP,
    DATA_CALL_RELEALSE,
} dataCall_e;

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


typedef struct SOCK_INFO
{
    unsigned char domainIP		: 1;
    unsigned char socketOpen	: 1;
    unsigned char socketConnect	: 1;
    unsigned char useFlag		: 1;
    struct sockaddr_in socketInfo;
    unsigned char dnscount;
    unsigned char socketConnectTick;

    unsigned char index;
    char domain[50];
    int socketId;
    unsigned int port;
    void (*rxFun)(struct SOCK_INFO *socketinfo, char *rxbuf, uint16_t len);
} socketInfo_s;


typedef struct
{
    unsigned char networkonoff;
    unsigned char netFsm;	
	unsigned char netRegCnt;
    unsigned char dataCallFsm;
    unsigned char dataCallState;
    unsigned char dataCallCount;
    unsigned int  csqSearchTime;

    unsigned int  netTick;
    int dataServerHandle;
} networkInfo_s;



void socketListInit(void);
void networkInfoInit(void);
void networkConnectCtl(unsigned char onoff);

int socketAdd(unsigned char sockId, char *domain, unsigned int port, void (*rxFun)(struct SOCK_INFO *socketinfo,
              char *rxbuf, uint16_t len));
void socketDeleteAll(void);
int socketClose(uint8_t sockId);


void netStopDataCall(void);
void netResetCsqSearch(void);

int socketSendData(unsigned char link, unsigned char *data, unsigned int len);
void networkConnectTask(void);
uint8_t isNetworkNormal(void);
uint8_t socketGetUsedFlag(uint8_t sockeId);
uint8_t socketGetConnectStatus(uint8_t sockeId);


#endif
