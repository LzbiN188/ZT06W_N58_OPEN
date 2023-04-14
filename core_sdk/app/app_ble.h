#ifndef APP_BLE
#define APP_BLE

#include "nwy_osi_api.h"

/*����ɨ��ʱ��*/
#define BLE_CLIENT_SCAN_TIME	5
/*���������б�*/
#define BLE_CONNECT_LIST_SIZE	5
/*�������ջ�����*/
#define BLE_RECV_BUFF_SIZE		128
/*��������ʱ��*/
#define BLE_CONN_HOLD_TIME		120

#define CMD_GET_SHIELD_CNT        			0x00  //��ȡ���δ���
#define CMD_CLEAR_SHIELD_CNT      			0x01  //���������������ͨ�̵���
#define CMD_DEV_ON                			0x02  //������ͨ�̵���
#define CMD_DEV_OFF               			0x03  //�����Ͽ��̵���
#define CMD_SET_VOLTAGE           			0x04  //�������ε�ѹ��ֵ
#define CMD_GET_VOLTAGE           			0x05  //��ȡ���ε�ѹ��ֵ
#define CMD_GET_ADCV              			0x06  //��ȡ���ε�ѹ
#define CMD_SET_OUTVOLTAGE		  			0x07  //�����ⲿ��ѹ��ֵ
#define CMD_GET_OUTVOLTAGE		  			0x08  //��ȡ�ⲿ��ѹ��ֵ
#define CMD_GET_OUTV			  			0x09  //��ȡ�ⲿ��ѹֵ
#define CMD_GET_ONOFF			  			0x0A  //��ȡ�̵���״̬
#define CMD_ALARM			      			0x0B  //��������
#define CMD_SPEED_FLAG			  			0x0C  //�ٶȱ�־
#define CMD_AUTODIS				  			0x0D  //���õ���ʱ����
#define CMD_CLEAR_ALARM			  			0x0E  //�������
#define CMD_PREALARM		      			0x0F  //Ԥ������
#define CMD_CLEAR_PREALARM		  			0x10  //���Ԥ��
#define CMD_SET_PRE_ALARM_PARAM   			0x13  //��������Ԥ������
#define CMD_GET_PRE_ALARM_PARAM   			0x14  //��ȡ����Ԥ������
#define CMD_GET_DISCONNECT_PARAM  			0x15  //��ȡ����ʱ����
#define CMD_SET_SHIEL_ALARM_HOLD_PARAM		0x16  //�������ε�ѹ���ֲ���
#define CMD_GET_SHIEL_ALARM_HOLD_PARAM		0x17  //��ȡ���ε�ѹ���ֲ���


#define CMD_SET_RTC               0xA8//����RTCʱ��


#define BLE_EVENT_GET_OUTV		  0x00000001 //��ȡ�����̵������ⲿ��ѹ
#define BLE_EVENT_GET_RFV		  0x00000002 //��ȡ�����̵��������ε�ѹ
#define BLE_EVENT_GET_RF_THRE	  0x00000004 //��ȡ���ε�ѹ��ֵ
#define BLE_EVENT_GET_OUT_THRE    0x00000008 //��ȡ�ⲿ��ѹ��ֵ
#define BLE_EVENT_GET_RFHOLD	  0x00000010 //��ȡ����ʱ����ֵ
#define BLE_EVENT_SET_RFHOLD	  0x00000020 //��������ʱ����ֵ

#define BLE_EVENT_SET_DEVON		  0x00000100 //�̵���on
#define BLE_EVENT_SET_DEVOFF	  0x00000200 //�̵���off
#define BLE_EVENT_CLR_CNT		  0x00000400 //������δ���
#define BLE_EVENT_SET_RF_THRE     0x00000800 //�������ε�ѹ��ֵ
#define BLE_EVENT_SET_OUTV_THRE   0x00001000 //����acc����ѹ��ֵ
#define BLE_EVENT_SET_AD_THRE     0x00002000 //�����Զ�����������ֵ
#define BLE_EVENT_CLR_ALARM       0x00004000 //�������
#define BLE_EVENT_CLR_PREALARM	  0x00008000 //���Ԥ��
#define BLE_EVENT_SET_PRE_PARAM	  0x00010000 //����Ԥ������
#define BLE_EVENT_GET_PRE_PARAM	  0x00020000 //��ȡԤ������
#define BLE_EVENT_GET_AD_THRE  	  0x00040000 //��ȡ�Զ���������
#define BLE_EVENT_SET_RTC	  	  0x00080000 //����RTC


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
    uint32_t dataReq;//��Ҫ���͵�ָ������
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

