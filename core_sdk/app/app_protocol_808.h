#ifndef APP_SERVERPROTOCOL_808
#define APP_SERVERPROTOCOL_808


#include "nwy_osi_api.h"
#include "app_protocol.h"
#define JT808_LOWVOLTAE_ALARM		(1<<7)//�͵籨��
#define JT808_LOSTVOLTAGE_ALARM		(1<<8)//�ϵ籨��


#define JT808_STATUS_ACC			(1<<0)//acc on
#define JT808_STATUS_FIX			(1<<1)//gps ��λ
#define JT808_STATUS_LATITUDE		(1<<2)//��γ
#define JT808_STATUS_LONGTITUDE		(1<<3)//����
#define JT808_STATUS_MOVE			(1<<4)//��Ӫ״̬
#define JT808_STATUS_OIL			(1<<10)//��·�Ͽ�
#define JT808_STATUS_ELECTRICITY	(1<<11)//��·�Ͽ�
#define JT808_STATUS_USE_GPS		(1<<18)//gps��λ
#define JT808_STATUS_USE_BEIDOU		(1<<19)//������λ
#define JT808_STATUS_USE_GLONASS	(1<<20)//������˹��λ
#define JT808_STATUS_USE_GALILEO	(1<<21)//٤���Զ�λ


#define TERMINAL_REGISTER_MSGID				0x0100  //�ն�ע��
#define TERMINAL_AUTHENTICATION_MSGID		0x0102  //�ն˼�Ȩ
#define TERMINAL_HEARTBEAT_MSGID			0x0002  //�ն�����
#define TERMINAL_POSITION_MSGID				0x0200  //λ����Ϣ
#define TERMINAL_ATTRIBUTE_MSGID			0x0107  //�ն�����
#define TERMINAL_GENERIC_RESPON_MSGID		0x0001  //ͨ�ûش�
#define TERMINAL_DATAUPLOAD_MSGID			0x0900  //��������

#define PLATFORM_GENERIC_RESPON_MSGID		0x8001
#define PLATFORM_REGISTER_RESPON_MSGID		0x8100
#define PLATFORM_TERMINAL_CTL_MSGID			0x8105
#define PLATFORM_DOWNLINK_MSGID				0x8900
#define PLATFORM_TERMINAL_SET_PARAM_MSGID	0x8103
#define PLATFORM_QUERY_ATTR_MSGID			0x8107
#define PLATFORM_TEXT_MSGID					0x8300

#define TRANSMISSION_TYPE_F8	0xF8


typedef enum{
	JT808_REGISTER,
	JT808_AUTHENTICATION,
	JT808_NORMAL,
}JT808_FSM_STATE;

typedef enum{
	TERMINAL_REGISTER,
	TERMINAL_AUTH,
	TERMINAL_HEARTBEAT,
	TERMINAL_POSITION,
	TERMINAL_ATTRIBUTE,
	TERMINAL_GENERICRESPON,
	TERMINAL_DATAUPLOAD,
}JT808_PROTOCOL;

typedef struct
{
	//λ�û�����Ϣ
	uint32_t alarm;  		//������־
	uint32_t status; 		//״̬
	uint32_t latitude;		//ά��
	uint32_t longtitude;	//����
	uint16_t hight;			//�߶�
	uint16_t speed;			//�ٶ�
	uint16_t course;		//����
	uint8_t year;	//BCDʱ�� GMT+8
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t mintues;
	uint8_t seconds;
}JT808_BASE_POSITION_INFO;



typedef struct
{
	JT808_FSM_STATE fsmstate;
	uint16_t heartbeattick;
	uint16_t serial;
	uint16_t registertick;
	uint16_t authtick;

	uint8_t  registerCount;
	uint8_t  authCount;
}JT808_NETWORK_STRUCT;

typedef struct
{
	uint16_t platform_serial;
	uint16_t platform_id;
	uint8_t  result;
}JT808_TERMINAL_GENERIC_RESPON;

typedef struct
{
    uint8_t id;
    void *param;
} TIRETRAMMISSION;

void jt808InfoInit(void);
void jt808ReceiveParser(uint8_t *src, uint16_t len);
void jt808NetReset(void);
uint8_t isJt808Reday(void);
void jt808ProtocolRunFsm(void);
void jt808CreateSn(uint8_t *sn, uint8_t snlen);

void jt808UpdateStatus(uint32_t bit, uint8_t onoff);
void jt808UpdateAlarm(uint32_t bit,uint8_t onoff);
void jt808SendToServer(JT808_PROTOCOL protocol, void *param);

uint16_t jt808gpsRestoreDataSend(GPSRestoreStruct *grs, uint8_t *dest);

void jt808MessageSend(uint8_t *buf, uint16_t len);

#endif

