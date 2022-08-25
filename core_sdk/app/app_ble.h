#ifndef APP_BLE
#define APP_BLE

#include "nwy_osi_api.h"

#define SERVERMAX	3

typedef struct
{
    uint16_t modecount;
    float    voltage;
    uint8_t  batterylevel;
} BLEDEVINFO;

typedef struct
{
    uint8_t servcount;
    uint8_t servw;
    uint8_t servr;
    uint8_t servsn[SERVERMAX][16];
    BLEDEVINFO devinfo[SERVERMAX];
} BleServerList;

typedef struct
{
    uint8_t devsn[SERVERMAX][16];
    uint16_t devParam[SERVERMAX][8];
} BleDevCfgList;

typedef struct
{
	uint8_t domainIP		:1;
	uint8_t socketOpen		:1;
	uint8_t socketConnect	:1;
	uint8_t loginok			:1;
	uint8_t bleConState		:1;
	int8_t socketId;
    uint8_t fsm;
    uint8_t connectFsm;
    uint8_t reCount;
    uint16_t tick;
} BleServerRunFsm;

enum
{
    BLE_CONNECTSERVER,
    BLE_LOGIN,
    BLE_SENDDATA,
    BLE_END,
};

enum
{
    SEND_LBS,
    WAIT_GPS,
    END,
};


void appBleRecvParser(uint8_t *buf, uint8_t len);
void sendDataToDoubleLink(uint8_t *buf, uint16_t len);
void bleServerLoginOk(void);
void bleServerSnConnect(void);
uint8_t appBleSendData(uint8_t *buf, uint16_t len);
void appBleSetConnectState(uint8_t onoff);

BLEDEVINFO *getBleDevInfo(void);
uint8_t *getBleServSn(void);


#endif
