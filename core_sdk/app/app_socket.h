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
} NETWORK_FSM;


typedef struct SOCK_INFO
{
    unsigned char domainIP		: 1;
    unsigned char socketOpen	: 1;
    unsigned char socketConnect	: 1;
	struct sockaddr_in socketInfo;
	unsigned char dnscount;
	unsigned char socketConnectTick;

	unsigned char index;
    char useFlag;
    char domain[50];
    int socketId;
    unsigned int port;
    void (*rxFun)(struct SOCK_INFO * socketinfo,char *rxbuf, uint16_t len);
} SOCKET_INFO;


typedef struct
{
	unsigned char networkonoff;
    unsigned char netFsm;
    unsigned char dataCallFsm;
    unsigned int  netTick;
    char dataServerHandle;
    char dataCallState;
    unsigned char dataCallCount;
} NETWORK_INFO;


void socketListInit(void);
void networkInit(void);
void networkConnectCtl(unsigned char onoff);

int socketAdd(unsigned char index, char *domain, unsigned int port, void (*rxFun)(SOCKET_INFO * socketinfo,char *rxbuf, uint16_t len));
void socketDeleteAll(void);
int socketDel(SOCKET_INFO *socketinfo);

int socketSendData(unsigned char link, unsigned char *data, unsigned int len);
int socketClose(uint8_t i);

void netStopDataCall(void);


void networkConnect(void);

void updateFirmware(void);


#endif
