#ifndef APP_BLE
#define APP_BLE

#include "nwy_osi_api.h"

/*蓝牙扫描时长*/
#define BLE_CLIENT_SCAN_TIME	5
/*蓝牙连接列表*/
#define BLE_CONNECT_LIST_SIZE	5
/*蓝牙接收缓冲区*/
#define BLE_RECV_BUFF_SIZE		128
/*蓝牙保持时长*/
#define BLE_CONN_HOLD_TIME		120

#define CMD_GET_SHIELD_CNT        			0x00  //获取屏蔽次数
#define CMD_CLEAR_SHIELD_CNT      			0x01  //清除计数器，并接通继电器
#define CMD_DEV_ON                			0x02  //主动接通继电器
#define CMD_DEV_OFF               			0x03  //主动断开继电器
#define CMD_SET_VOLTAGE           			0x04  //设置屏蔽电压阈值
#define CMD_GET_VOLTAGE           			0x05  //读取屏蔽电压阈值
#define CMD_GET_ADCV              			0x06  //读取屏蔽电压
#define CMD_SET_OUTVOLTAGE		  			0x07  //设置外部电压阈值
#define CMD_GET_OUTVOLTAGE		  			0x08  //读取外部电压阈值
#define CMD_GET_OUTV			  			0x09  //读取外部电压值
#define CMD_GET_ONOFF			  			0x0A  //读取继电器状态
#define CMD_ALARM			      			0x0B  //报警发送
#define CMD_SPEED_FLAG			  			0x0C  //速度标志
#define CMD_AUTODIS				  			0x0D  //设置倒计时参数
#define CMD_CLEAR_ALARM			  			0x0E  //清除报警
#define CMD_PREALARM		      			0x0F  //预警发送
#define CMD_CLEAR_PREALARM		  			0x10  //清除预警
#define CMD_SET_PRE_ALARM_PARAM   			0x13  //设置屏蔽预警参数
#define CMD_GET_PRE_ALARM_PARAM   			0x14  //读取屏蔽预警参数
#define CMD_GET_DISCONNECT_PARAM  			0x15  //读取倒计时参数
#define CMD_SET_SHIEL_ALARM_HOLD_PARAM		0x16  //设置屏蔽电压保持参数
#define CMD_GET_SHIEL_ALARM_HOLD_PARAM		0x17  //读取屏蔽电压保持参数


#define CMD_SET_RTC               0xA8//更新RTC时间


#define BLE_EVENT_GET_OUTV		  0x00000001 //读取蓝牙继电器的外部电压
#define BLE_EVENT_GET_RFV		  0x00000002 //读取蓝牙继电器的屏蔽电压
#define BLE_EVENT_GET_RF_THRE	  0x00000004 //读取屏蔽电压阈值
#define BLE_EVENT_GET_OUT_THRE    0x00000008 //读取外部电压阈值
#define BLE_EVENT_GET_RFHOLD	  0x00000010 //读取屏蔽时间阈值
#define BLE_EVENT_SET_RFHOLD	  0x00000020 //设置屏蔽时间阈值

#define BLE_EVENT_SET_DEVON		  0x00000100 //继电器on
#define BLE_EVENT_SET_DEVOFF	  0x00000200 //继电器off
#define BLE_EVENT_CLR_CNT		  0x00000400 //清除屏蔽次数
#define BLE_EVENT_SET_RF_THRE     0x00000800 //设置屏蔽电压阈值
#define BLE_EVENT_SET_OUTV_THRE   0x00001000 //设置acc检测电压阈值
#define BLE_EVENT_SET_AD_THRE     0x00002000 //设置自动断连参数阈值
#define BLE_EVENT_CLR_ALARM       0x00004000 //清除报警
#define BLE_EVENT_CLR_PREALARM	  0x00008000 //清除预警
#define BLE_EVENT_SET_PRE_PARAM	  0x00010000 //设置预警参数
#define BLE_EVENT_GET_PRE_PARAM	  0x00020000 //读取预警参数
#define BLE_EVENT_GET_AD_THRE  	  0x00040000 //读取自动断连参数
#define BLE_EVENT_SET_RTC	  	  0x00080000 //更新RTC


typedef enum
{
    BLE_CHECK_STATE,
    BLE_OPEN,
    BLE_NORMAL,
} bleFsm_e;

typedef enum
{
    BLE_CLIENT_OPEN,
    BLE_CLIENT_CLOSE,
    BLE_CLIENT_SCAN,
    BLE_CLIENT_CONNECT,
    BLE_CLIENT_DISCONNECT,
    BLE_CLIENT_DISCOVER,
    BLE_CLIENT_RECV,
} ble_clientevent_e;
typedef enum
{
    BLE_SCH_OPEN,
    BLE_SCH_RUN,
    BLE_SCH_CLOSE,
    BLE_SCH_SCAN
} ble_schedule_fsm_e;

typedef enum
{
    BLE_CONN_IDLE,
    BLE_CONN_WATI,
    BLE_CONN_RUN,
    BLE_CONN_CHANGE_WAIT,
    BLE_CONN_CHANGE,
} ble_conn_fsm_e;

typedef struct
{
    uint8_t bleState		: 1;
    uint8_t bleCloseReq		: 1;
    uint8_t bleFsm;
} bleServer_s;

typedef struct
{
    uint8_t bleClientOnoff	: 1;
    uint8_t bleConnState	: 1;

    uint8_t bleDataBuff[BLE_RECV_BUFF_SIZE];
    uint8_t bleDataLen;
    char connectMac[20];
    uint8_t addrType;
    uint8_t srvNum;
} bleClinet_s;



typedef struct
{
    float outV;
    float rfV;

    float rf_threshold;
    float out_threshold;

    uint32_t updateTick;
	
    uint8_t bleLost;
    uint8_t preV_threshold;
    uint8_t preDetCnt_threshold;
    uint8_t preHold_threshold;
	uint8_t disc_threshold;
	uitn8_t rfHold_threshold;
} bleRelayInfo_s;


typedef struct
{
    char bleMac[20];
    uint8_t bleType;
    uint8_t bleUsed;
    uint32_t dataReq;//需要发送的指令类型
    bleRelayInfo_s bleInfo;
} bleConnectList_s;

typedef struct
{
    uint8_t bleDo	    : 1;
    uint8_t bleTryOpen	: 1;
	uint8_t bleTryScan	: 1;
    uint8_t bleSchFsm;
    uint8_t bleConnFsm;
    uint8_t bleCurConnInd;
    uint8_t bleListCnt;
    uint8_t bleConnFailCnt;
    uint8_t bleQuickRun;
	uint8_t bleKey[13];
	uint8_t bleKeyLen;

    uint16_t runTick;
    uint16_t connTick;
    uint16_t sendTick;
	uint16_t bleRebootCnt;
    bleConnectList_s	bleList[BLE_CONNECT_LIST_SIZE];
} bleConnectShchedule_s;



uint8_t bleServSendData(char *buf, uint16_t len);
void bleServCloseRequestSet(void);
void bleServCloseRequestClear(void);
void bleServRequestOn5Minutes(void);

void bleServRunTask(void);

void bleClientSendEvent(ble_clientevent_e event);

void bleClientInfoInit(void);
void bleClientDoEventProcess(uint8_t id);
uint8_t bleClientSendData(uint8_t serId, uint8_t charId, uint8_t *data, uint8_t len);

int8_t bleScheduleInsert(char *mac);
void bleScheduleDelete(uint8_t ind);
void bleScheduleWipeAll(void);
void bleScheduleSetAllReq(uint32_t event);
void bleScheduleClearAllReq(uint32_t event);
bleRelayInfo_s *bleGetDevInfo(uint8_t i);


void bleScheduleSetReq(uint8_t ind, uint32_t event);
void bleScheduleClearReq(uint8_t ind, uint32_t event);

void bleScheduleScan(void);
void bleChangToByte(uint8_t *byteMac, uint8_t *hexMac);


void bleScheduleInit(void);
uint8_t bleScheduleGetCnt(void);

void bleScheduleCtrl(uint8_t onoff);

void bleScheduleTask(void);

#endif

