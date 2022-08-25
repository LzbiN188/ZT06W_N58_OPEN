#ifndef APP_MCU
#define APP_MCU

#include "nwy_osi_api.h"

#define APP_RECEIVE_BUFF_SIZE		256

#define MCU_POWERON_CMD             0x00
#define MCU_POWEROFF_CMD            0x01
#define MCU_VERSION_CMD             0x02
#define MCU_EARSE_CMD               0x03 //bootloader 实现
#define MCU_WRITE_CMD               0x04 //bootloaser 实现
#define MCU_DONE_CMD                0x05 //bootloaser 实现
#define MCU_RESET_CMD               0x06
#define MCU_STARTUPGRADE_CMD        0x07
#define MCU_BUZZER_CMD				0x08
#define MCU_INFO_CMD                0x09
#define MCU_RESEND_CMD				0x0A
#define MCU_CHARGE_CMD				0x0B
#define MCU_LOCK_CMD				0x0C
#define MCU_TAPACCURE_CMD			0x0D
#define MCU_VIBRATIONPARAM_CMD      0x0E
#define MCU_GETVIBPARAM_CMD         0x0F
#define MCU_GSENSOR_CMD             0x10
#define MCU_CHARGETEMP_CMD          0x11
#define MCU_QUERYCHARGETEMP_CMD     0x12
#define MCU_LDR_CMD                 0x13
#define MCU_ALARM_CMD               0x14
#define MCU_LIGHTALRMEN_CMD         0x15
#define MCU_AUTOWAKEUP_CMD			0x16
#define MCU_QUERYAUTOWAKEUP_CMD		0x17
#define MCU_SLEEP_CMD				0x18
#define MCU_QUERYSLEEP_CMD			0x19



#define MCU_RESPON_CMD              0x80



#define MCU_PROTOCOL_HEAD			7
#define MCU_FIRMWARE_PACK_SIZE		1024+MCU_PROTOCOL_HEAD
typedef struct
{
	uint8_t upgradeFlag;
	uint8_t upgradeRsp;
	uint8_t upgradeRet;
	uint8_t packBuf[MCU_FIRMWARE_PACK_SIZE];
	uint8_t *firmWare;
	uint16_t firmWareSize;
	uint32_t firmWareOffset;

} MCU_UPGRADE_INFO;


void appMcuSend(uint8_t *content, uint16_t len);
void appMcuRxParser(const char *msg, uint32_t len);

void appMcuUpgradeInit(void);
void pushUpgradeFirmWare(uint8_t *firmware, uint32_t offset, uint16_t firmwareSize);
void appMcuUpgradeTask(void);


void doStartUpgrade(void);
void doResetMcu(void);


void doEarseCmd(uint32_t size);
void doDoneCmd(void);
void doBuzzerCmd(uint8_t onoff);
void doInfoCmd(void);
void doResendCmd(void);
void doChargeCmd(uint8_t onoff);
void doGetVersionCmd(void);
void doLockCmd(uint8_t onoff);
void doVibrationParamCmd(uint8_t cnt , uint16_t time);
void doGetVibParamCmd(void);

void doGsensorCmd(void);
void doChargeTempCmd(float temp);
void doQueryChargeTempCmd(void);
void doQueryLdrStateCmd(void);
void doSetAlarmCmd(uint8 alarm);
void doSetAutoWakeCmd(uint32 sec);
void doQueryAutoWakeUpCmd(void);
void doSetSleepCmd(uint8 en);
void doQuerySleepCmd(void);


#endif

