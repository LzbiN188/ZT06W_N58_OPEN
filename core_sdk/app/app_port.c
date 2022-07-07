#include "app_port.h"
#include "app_sys.h"
#include "app_mir3da.h"
#include "app_task.h"
#include "app_net.h"
#include "app_param.h"
#include "app_instructioncmd.h"
#include "app_protocol.h"
#include "stdio.h"
#include "stdlib.h"

#include "nwy_usb_serial.h"
#include "nwy_uart.h"
#include "nwy_pm.h"
#include "nwy_adc.h"
#include "nwy_network.h"
#include "nwy_sim.h"
#include "nwy_file.h"
#include "nwy_audio_api.h"
#include "nwy_fota.h"
#include "nwy_fota_api.h"
#include "nwy_sms.h"
#include "nwy_vir_at.h"
#include "nwy_voice.h"
#include "nwy_sms.h"

usartCtrl_s usart1_ctl = {.init = 0};
usartCtrl_s usart2_ctl = {.init = 0};
usartCtrl_s usart3_ctl = {.init = 0};

static record_s record;
static tts_info_s *ttslist = NULL;

/**************************************************
@bref		ģ��AT����
@param
@note
**************************************************/
static const moduleAtCmd_s cmd_table[MAX_NUM] =
{
    {AT_CMD, "ATE0"},
    {NWBTBLEMAC_CMD, "AT+NWBTBLEMAC"},
    {CLIP_CMD, "AT+CLIP"},
    {CFUN_CMD, "AT+CFUN"},
    {CGDCONT_CMD, "AT+CGDCONT"},
    {XGAUTH_CMD, "AT+XGAUTH"},
    {NWBTBLEMAC, "AT+NWBTBLEMAC"},
    {MAX_NUM, "AT"},
};

/**************************************************
@bref		USB��ʼ��
@param
@note
**************************************************/
void portUSBCfg(nwy_sio_recv_cb_t usbRecvCallback)
{
    nwy_usb_serial_reg_recv_cb(usbRecvCallback);
}

/**************************************************
@bref		USB���ݷ���
@param
	data	��������
	len	���ݳ���
@note
**************************************************/
void portUsbSend(char *data, uint16_t len)
{
    uint16_t actualen = 0;
    uint16_t totalen = 0;
    while (totalen < len)
    {
        actualen = nwy_usb_serial_send(data + totalen, len - totalen);
        if (actualen <= 0)
            break;
        totalen += actualen;
    }
}
/**************************************************
@bref		��������
@param
	type	�������ͣ���UARTTYPE
	onoff	1:��		0:��
	baudrate 		������
	rxhandlefun		���ջص�����
@note
**************************************************/
void portUartCfg(uarttype_e type, uint8_t onoff, uint32_t baudrate, void (*rxhandlefun)(char *, uint32_t))
{
    int ret = -1;
    int hd = -1;
    switch (type)
    {
        case APPUSART1:
            if (onoff == 0)
            {
                LogMessage(DEBUG_ALL, "portUartCfg==>Close uart1");
                nwy_uart_deinit(usart1_ctl.uart_handle);
                usart1_ctl.init = 0;
            }
            else
            {
                hd = nwy_uart_init(NWY_NAME_UART1, 1);
                if (hd < 0)
                {
                    break;
                }
                ret = nwy_uart_set_baud(hd, baudrate);
                if (ret == 0)
                {
                    break;
                }
                ret = nwy_uart_set_para(hd, NWY_UART_NO_PARITY, NWY_UART_DATA_BITS_8, NWY_UART_STOP_BITS_1, false);
                if (ret == 0)
                {
                    break;
                }
                if (rxhandlefun != NULL)
                {
                    nwy_uart_reg_recv_cb(hd, (nwy_uart_recv_callback_t)rxhandlefun);
                }
                usart1_ctl.uart_handle = hd;
                usart1_ctl.init = 1;
                LogPrintf(DEBUG_ALL, "portUartCfg==>OpenUart1 , baudrate %d", baudrate);
            }
            break;
        case APPUSART2:
            if (onoff == 0)
            {
                LogMessage(DEBUG_ALL, "portUartCfg==>Close uart2");
                nwy_uart_deinit(usart2_ctl.uart_handle);
                usart2_ctl.init = 0;
            }
            else
            {
                hd = nwy_uart_init(NWY_NAME_UART2, 1);
                if (hd < 0)
                {
                    break;
                }
                ret = nwy_uart_set_baud(hd, baudrate);
                if (ret == 0)
                {
                    break;
                }
                ret = nwy_uart_set_para(hd, NWY_UART_NO_PARITY, NWY_UART_DATA_BITS_8, NWY_UART_STOP_BITS_1, false);
                if (ret == 0)
                {
                    break;
                }
                if (rxhandlefun != NULL)
                {
                    nwy_uart_reg_recv_cb(hd, (nwy_uart_recv_callback_t)rxhandlefun);
                }
                usart2_ctl.uart_handle = hd;
                usart2_ctl.init = 1;
                LogPrintf(DEBUG_ALL, "portUartCfg==>OpenUart2 , baudrate %d", baudrate);
            }
            break;
        case APPUSART3:
            if (onoff == 0)
            {
                LogMessage(DEBUG_ALL, "portUartCfg==>Close uart3");
                nwy_uart_deinit(usart3_ctl.uart_handle);
                usart3_ctl.init = 0;
            }
            else
            {
                hd = nwy_uart_init(NWY_NAME_UART3, 1);
                if (hd < 0)
                {
                    break;
                }
                ret = nwy_uart_set_baud(hd, baudrate);
                if (ret == 0)
                {
                    break;
                }
                ret = nwy_uart_set_para(hd, NWY_UART_NO_PARITY, NWY_UART_DATA_BITS_8, NWY_UART_STOP_BITS_1, false);
                if (ret == 0)
                {
                    break;
                }
                if (rxhandlefun != NULL)
                {
                    nwy_uart_reg_recv_cb(hd, (nwy_uart_recv_callback_t)rxhandlefun);
                }
                usart3_ctl.uart_handle = hd;
                usart3_ctl.init = 1;
                LogPrintf(DEBUG_ALL, "portUartCfg==>OpenUart3 , baudrate %d", baudrate);
            }
            break;
    }

}

/**************************************************
@bref		�������ݷ���
@param
	uart_ctl	���ڿ�����Ϣ
	buf			��������
	len			���ݳ���
@note
**************************************************/
void portUartSend(usartCtrl_s *uart_ctl, uint8_t *buf, uint16_t len)
{
    if (uart_ctl == NULL)
        return;
    if (uart_ctl->init)
    {
        nwy_uart_send_data(uart_ctl->uart_handle, buf, len);
    }
}
/**************************************************
@bref		��ȡrtcʱ��
@param
	year	��
	month	��
	date	��
	hour	ʱ
	minute	��
	second	��
@note
**************************************************/
void portGetRtcDateTime(uint16_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    nwy_time_t rtc_time;
    char timezone;
    nwy_get_time(&rtc_time, &timezone);
    *year = rtc_time.year;
    *month = rtc_time.mon;
    *date = rtc_time.day;
    *hour = rtc_time.hour;
    *minute = rtc_time.min;
    *second = rtc_time.sec;
}
/**************************************************
@bref		����rtcʱ��
@param
	year	��
	month	��
	date	��
	hour	ʱ
	minute	��
	second	��
@note
**************************************************/
void portSetRtcDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second,
                        int8_t utc)
{
    nwy_time_t rtc_time;
    rtc_time.year = year + 2000;
    rtc_time.mon = month;
    rtc_time.day = date;
    rtc_time.hour = hour;
    rtc_time.min = minute;
    rtc_time.sec = second;
    LogPrintf(DEBUG_ALL, "UpdateRTC:%02d-%02d-%02d %02d:%02d:%02d TZ:%d", rtc_time.year, rtc_time.mon, rtc_time.day,
              rtc_time.hour,
              rtc_time.min, rtc_time.sec, utc);
    nwy_set_time(&rtc_time, utc * 4);

}

/**************************************************
@bref			������һ������ʱ��
@param
	gapDay		�������
	AlarmTime	����ʱ�䣬��λ������
@note
**************************************************/

void setNextAlarmTime(uint8_t gapDay, uint16_t *AlarmTime)
{
    unsigned short  rtc_mins, next_ones;
    unsigned char next_date, set_nextdate = 1;
    uint16_t  YearToday;      /*��ǰ��*/
    uint8_t  MonthToday;     /*��ǰ��*/
    uint8_t  DateToday;      /*��ǰ��*/
    int i;
    uint16_t year = 0;
    uint8_t  month = 0;
    uint8_t date = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);

    //1����ȡ��ǰʱ�����ܷ�����
    rtc_mins = (hour & 0x1F) * 60;
    rtc_mins += (minute & 0x3f);
    //2����ȡ��ǰ����
    YearToday = year; //���㵱ǰ�꣬��2000�꿪ʼ����
    MonthToday = month;
    DateToday = date;
    //3�����ݵ�ǰ�£������¸�������
    if (MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + gapDay) % 30; //��ǰ���ڼ��ϼ���գ�������һ�ε�ʱ���
        if (next_date == 0)
            next_date = 30;
    }
    else if (MonthToday == 2)
    {
        //4�������2�£��ж��ǲ�������
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || (YearToday % 400 == 0))  //����
        {
            next_date = (DateToday + gapDay) % 29;
            if (next_date == 0)
                next_date = 29;
        }
        else
        {
            next_date = (DateToday + gapDay) % 28;
            if (next_date == 0)
                next_date = 28;
        }
    }
    else
    {
        next_date = (DateToday + gapDay) % 31;
        if (next_date == 0)
            next_date = 31;
    }
    next_ones = 0xFFFF;
    //5����������������Ƿ����ڵ�ǰʱ���֮���ʱ��
    for (i = 0; i < 5; i++)
    {
        if (AlarmTime[i] == 0xFFFF)
            continue;
        if (AlarmTime[i] > rtc_mins)   //����ǰʱ��ȶ�
        {
            next_ones = AlarmTime[i];  //�õ��µ�ʱ��
            set_nextdate = 0;
            break;
        }
    }


    if (next_ones == 0xFFFF)  //û������ʱ��
    {
        //Set Current Alarm Time
        next_ones = AlarmTime[0];
        if (next_ones == 0xFFFF)
        {
            next_ones = 720; //Ĭ��12:00
        }
    }
    //6�������´��ϱ������ں�ʱ��
    if (set_nextdate)
        LogPrintf(DEBUG_ALL, "NextAlarmTime:%d,%02d:%02d", next_date, (next_ones / 60) % 24, next_ones % 60);
    else
        LogPrintf(DEBUG_ALL, "NextAlarmTime:%d,%02d:%02d", date, (next_ones / 60) % 24, next_ones % 60);
}

/**************************************************
@bref			������һ������ʱ��
@param
	gapMinutes	���Ӽ��,��λ������
@note			���Ҫ����2880
**************************************************/

void setNextWakeUpTime(uint16_t gapMinutes)
{
    uint16_t  YearToday;      /*��ǰ��*/
    uint8_t  MonthToday;     /*��ǰ��*/
    uint8_t  DateToday;      /*��ǰ��*/
    uint8_t  date = 0, next_date;
    uint16_t totalminutes;
    uint16_t year = 0;
    uint8_t  month = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    portGetRtcDateTime(&year, &month, &day, &hour, &minute, &second);
    YearToday = year; //���㵱ǰ�꣬��2000�꿪ʼ����
    MonthToday = month;
    DateToday = day;
    totalminutes = hour * 60 + minute;
    totalminutes += gapMinutes;
    if (totalminutes >= 1440)
    {
        date = 1;
        totalminutes -= 1440;
    }
    //3�����ݵ�ǰ�£������¸�������
    if (MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + date) % 30; //��ǰ���ڼ��ϼ���գ�������һ�ε�ʱ���
        if (next_date == 0)
            next_date = 30;
    }
    else if (MonthToday == 2)
    {
        //4�������2�£��ж��ǲ�������
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || (YearToday % 400 == 0))  //����
        {
            next_date = (DateToday + date) % 29;
            if (next_date == 0)
                next_date = 29;
        }
        else
        {
            next_date = (DateToday + date) % 28;
            if (next_date == 0)
                next_date = 28;
        }
    }
    else
    {
        next_date = (DateToday + date) % 31;
        if (next_date == 0)
            next_date = 31;
    }
    LogPrintf(DEBUG_ALL, "NextWakeUpTime:%d,%02d:%02d", next_date, (totalminutes / 60) % 24, totalminutes % 60);
}

/**************************************************
@bref		���߿���
@param
	mode	1:�Զ�����		0:�ر�
@note
**************************************************/

void portSleepCtrl(uint8_t mode)
{
    nwy_pm_state_set(mode);
}


/**************************************************
@bref		gpio��ʼ��
@param
@note
**************************************************/

void portGpioCfg(void)
{
    //LED gpio config
    nwy_gpio_set_direction(LED1_PORT, nwy_output);
    nwy_gpio_set_direction(LED2_PORT, nwy_output);
    LED1_OFF;
    LED2_OFF;

    //ldr�й��
    nwy_gpio_set_direction(LDR_PORT, nwy_input);
	nwy_gpio_pullup_or_pulldown(LDR_PORT,2);
    //relay �̵������ƿ�
    nwy_gpio_set_direction(RELAY_PORT, nwy_output);
    RELAY_OFF;

    //ACC���
    nwy_gpio_set_direction(ACC_PORT, nwy_input);

    //WDT���Ź�
    nwy_gpio_set_direction(WDT_PORT, nwy_output);

    //���ػ����
    nwy_gpio_set_direction(ONOFF_PORT, nwy_input);
    nwy_gpio_pullup_or_pulldown(ONOFF_PORT, 1);
}

/**************************************************
@bref		pmu��ʼ��
@param
@note
**************************************************/

void portPmuCfg(void)
{
    nwy_subpower_switch(NWY_POWER_SD, true, true);
    nwy_set_pmu_power_level(NWY_POWER_SD, 3000);

    nwy_subpower_switch(NWY_POWER_CAMD, true, true);
    nwy_set_pmu_power_level(NWY_POWER_CAMD, 1800);


}


void portOutsideGpsPwr(uint8_t onoff)
{
    nwy_subpower_switch(NWY_POWER_CAMA, true, true);
    if (onoff)
    {
        nwy_set_pmu_power_level(NWY_POWER_CAMD, 1800);
        LogMessage(DEBUG_ALL, "gps power on");
    }
    else
    {
        nwy_set_pmu_power_level(NWY_POWER_CAMD, 0);
        LogMessage(DEBUG_ALL, "gps power off");
    }
}
/**************************************************
@bref		gpio�жϻص�
@param
	param	�ж�io
@note
**************************************************/

static void gpioInterruptCallBack(int param)
{
    if (param == GSINT_PORT)
    {
        motionOccur();
        //LogMessage(DEBUG_ALL, "gsensor tap...");
    }
}

/**************************************************
@bref		gsensor��ʼ��
@param
@note
**************************************************/

void portGsensorCfg(uint8_t onoff)
{

    LogPrintf(DEBUG_ALL, "portGsensorCfg==>%s", onoff ? "Enable" : "Disable");
    nwy_close_gpio(GSINT_PORT);
    if (onoff)
    {
        GSPWR_ON;
        nwy_open_gpio_irq_config(GSINT_PORT, nwy_irq_rising, gpioInterruptCallBack);
        mir3da_init();
        mir3da_set_enable(1);
        mir3da_open_interrupt(10);
        nwy_gpio_open_irq_enable(GSINT_PORT);
        sysinfo.gsensorOnoff = 1;
        sysinfo.gsensorErr = 0;
    }
    else
    {
        GSPWR_OFF;
        nwy_gpio_open_irq_disable(GSINT_PORT);
        sysinfo.gsensorOnoff = 0;
    }
}


/**************************************************
@bref		gps����
@param
	onoff	1����gps
			0����gps
@note
**************************************************/
void portGpsCtrl(uint8_t onoff)
{
    int ret;
    LogPrintf(DEBUG_ALL, "portGpsCtrl==>%s", onoff ? "Enable" : "Disable");
    if (onoff)
    {
        nwy_loc_close_uart_nmea_data();
        ret = nwy_loc_start_navigation();
        if (ret)
        {
            LogMessage(DEBUG_ALL, "portGpsCtrl==>gps open success");
        }
        else
        {
            LogMessage(DEBUG_ALL, "portGpsCtrl==>gps open fail");
        }
    }
    else
    {
        nwy_loc_set_startup_mode(0);
        ret = nwy_loc_stop_navigation();
        if (ret)
        {
            LogMessage(DEBUG_ALL, "portGpsCtrl==>gps close success");
        }
        else
        {
            LogMessage(DEBUG_ALL, "portGpsCtrl==>gps close fail");
        }
    }
    LogMessage(DEBUG_ALL, "portGpsCtrl==>Done");

}
/**************************************************
@bref		gps���ö�λģʽ
@param
	pos_mode	��nwy_loc_position_mode_t
@note
**************************************************/

void portGpsSetPositionMode(nwy_loc_position_mode_t pos_mode)
{
    int ret;
    ret = nwy_loc_set_position_mode(pos_mode);
    LogPrintf(DEBUG_ALL, "portGpsSetPositionMode==>%s", ret ? "Success" : "Fail");
}


/**************************************************
@bref		nmea���ݶ�ȡ
@param
	gpsRecv	nmea��ȡ�ص�����
@note
**************************************************/

void portGpsGetNmea(void (*gpsRecv)(uint8_t *, uint16_t))
{
    int ret;
    char nmea[2048];
    memset(nmea, 0, 2048);
    ret = nwy_loc_get_nmea_data(nmea);
    if (ret && gpsRecv != NULL)
    {
        gpsRecv((uint8_t *)nmea, strlen(nmea));
    }
}
/**************************************************
@bref		��ȡ�ⲿ��ѹadcֵ
@param
	float
@note
**************************************************/

float portGetOutSideVolAdcVol(void)
{
    return nwy_adc_get(NWY_ADC_CHANNEL3, NWY_ADC_SCALE_3V233) / 1000.0;
}
/**************************************************
@bref		��ȡ��ص�ѹadcֵ
@param
	float
@note
**************************************************/

float portGetBatteryAdcVol(void)
{
    return nwy_adc_get(NWY_ADC_CHANNEL_VBAT, NWY_ADC_SCALE_5V000) / 1000.0;
}

/**************************************************
@bref		��ȡģ���ź�ֵ
@param
	float
@note
**************************************************/

uint8_t portGetModuleRssi(void)
{
    uint8_t csq;
    nwy_nw_get_signal_csq(&csq);
    return csq;
}

/**************************************************
@bref		��ȡģ��ICCID
@param
	iccid	�ṩһ����С����21���ֽڵ�����
@note
**************************************************/

void portGetModuleICCID(char *iccid)
{
    nwy_result_type ret;
    nwy_sim_result_type simResult;
    if (iccid == NULL)
        return;
    ret = nwy_sim_get_iccid(&simResult);
    if (ret == NWY_RES_OK)
    {
        strncpy(iccid, (char *)simResult.iccid, 20);
        iccid[20] = 0;
    }
    else
    {
        iccid[0] = 0;
    }
}

/**************************************************
@bref		��ȡģ��IMSI
@param
	imsi	�ṩһ����С����16���ֽڵ�����
@note
**************************************************/

void portGetModuleIMSI(char *imsi)
{
    nwy_result_type ret;
    nwy_sim_result_type simResult;
    if (imsi == NULL)
        return;
    ret = nwy_sim_get_imsi(&simResult);
    if (ret == NWY_RES_OK)
    {
        strncpy(imsi, (char *)simResult.imsi, 15);
        imsi[15] = 0;
    }
    else
    {
        imsi[0] = 0;
    }
}
/**************************************************
@bref		��ȡģ��IMEI
@param
	imei	�ṩһ����С����16���ֽڵ�����
@note
**************************************************/

void portGetModuleIMEI(char *imei)
{
    nwy_result_type ret;
    nwy_sim_result_type simResult;
    if (imei == NULL)
        return;
    ret = nwy_sim_get_imei(&simResult);
    if (ret == NWY_RES_OK)
    {
        strncpy(imei, (char *)simResult.nImei, 15);
        imei[15] = 0;
    }
    else
    {
        imei[0] = 0;
    }
}

/**************************************************
@bref		��ȡmcc mnc
@param

@note
**************************************************/

lbsInfo_s portGetLbsInfo(void)
{
    char IMSI[20];
    int lac, cid;
    lbsInfo_s lbsinfo;
    memset(IMSI, 0, 20);
    portGetModuleIMSI(IMSI);

    lbsinfo.mcc = (IMSI[0] - '0') * 100 + (IMSI[1] - '0') * 10 + IMSI[2] - '0';
    lbsinfo.mnc = (IMSI[3] - '0') * 10 + IMSI[4] - '0';
    lac = 0;
    cid = 0;
    nwy_sim_get_lacid(&lac, &cid);
    lbsinfo.lac = lac;
    lbsinfo.cid = cid;

    LogPrintf(DEBUG_ALL, "MCC=%d,MNC=%d,LAC=%X,CID=%X", lbsinfo.mcc, lbsinfo.mnc, lbsinfo.lac, lbsinfo.cid);
    return lbsinfo;
}


/**************************************************
@bref		����AGPS
@param
	agpsServer
	agpsPort
	agpsUser
	agpsPswd
@return
	1		�ɹ�
	0		ʧ��
@note
**************************************************/

int portStartAgps(char *agpsServer, int agpsPort, char *agpsUser, char *agpsPswd)
{
    int ret;
    ret = nwy_loc_set_server(agpsServer, agpsPort, agpsUser, agpsPswd);
    if (ret)
    {
        ret = nwy_loc_agps_open(1);
        LogPrintf(DEBUG_ALL, "agps ret %d", ret);
        return ret;
    }
    return 0;
}

/**************************************************
@bref		����AGPS����
@param
	data
	len
@return
	1		�ɹ�
	0		ʧ��
@note
**************************************************/

int portSendAgpsData(char *data, uint16_t len)
{
	LogPrintf(DEBUG_ALL,"agps write %d bytes",len);
    return nwy_loc_send_agps_data(data, len);
}


/**************************************************
@bref		����gps����ģʽ
@param
	mode
@return
	1		�ɹ�
	0		ʧ��
@note
**************************************************/

int portSetGpsStarupMode(gps_startUpmode_e mode)
{
    switch (mode)
    {
        case HOT_START:
            LogMessage(DEBUG_ALL, "set gps hot start");
            break;
        case  WORM_START:
            LogMessage(DEBUG_ALL, "set gps worm start");
            break;
        case COLD_START:
            LogMessage(DEBUG_ALL, "set gps cold start");
            break;
        case FACTORY_START:
            LogMessage(DEBUG_ALL, "set gps factory start");
            break;
    }
    return nwy_loc_set_startup_mode(mode);
}
/**************************************************
@bref		ϵͳ��λ
@param
@note
**************************************************/

void portSystemReset(void)
{
    LogMessage(DEBUG_ALL, "portSystemReset");
    nwy_power_off(2);
}

/**************************************************
@bref		ϵͳ�ػ�
@param
@note
**************************************************/

void portSystemShutDown(void)
{
    LogMessage(DEBUG_ALL, "portSystemShutDown");
    nwy_power_off(1);
}


/**************************************************
@bref		������Ƶ���ݵ��ļ�
@param
	audio	��Ƶ����
	len		��Ƶ����
@note		��׷�ӷ�ʽ���浽�ļ�
**************************************************/

void portSaveAudio(uint8_t *audio, uint16_t len)
{
    uint16_t  fileOperation;
    int fd, writelen;
    if (nwy_sdk_fexist(AUDIOFILE) == true)
    {
        fileOperation = NWY_WRONLY | NWY_APPEND;
    }
    else
    {
        fileOperation = NWY_CREAT | NWY_RDWR;
    }
    fd = nwy_sdk_fopen(AUDIOFILE, fileOperation);
    if (fd < 0)
    {
        LogMessage(DEBUG_ALL, "portSaveAudio==>Open error");
        return;
    }
    writelen = nwy_sdk_fwrite(fd, audio, len);
    if (writelen != len)
    {
        LogPrintf(DEBUG_ALL, "portSaveAudio==>Error[%d]", writelen);
    }
    else
    {
        LogMessage(DEBUG_ALL, "portSaveAudio==>Success");
    }
    nwy_sdk_fclose(fd);
}

/**************************************************
@bref		ɾ����Ƶ�ļ�
@param
@note
**************************************************/

void portDeleteAudio(void)
{
    if (nwy_sdk_fexist(AUDIOFILE))
    {
        if (nwy_sdk_file_unlink(AUDIOFILE) == 0)
        {
            LogMessage(DEBUG_ALL, "Delete audio file success");
        }
        else
        {
            LogMessage(DEBUG_ALL, "Delete audio file fail");
        }
    }
    else
    {
        LogMessage(DEBUG_ALL, "Audio file not exist");
    }
}

/**************************************************
@bref		������Ƶ�ļ�
@param
@note
**************************************************/

void portPlayAudio(void)
{
    long size = 0;
    size = nwy_sdk_fsize(AUDIOFILE);
    LogPrintf(DEBUG_ALL, "Play Begin:%s[%d Byte]", AUDIOFILE, size);
    nwy_audio_file_play(AUDIOFILE);
    LogMessage(DEBUG_ALL, "Play Done");
    portDeleteAudio();
}

/**************************************************
@bref		д�������̼�
@param
	offset	ƫ����ʼ��ַ
	length	��������
	data	��������
@return
	1		д��ɹ�
	0		д��ʧ��
@note
**************************************************/


uint8_t portUpgradeWirte(unsigned int offset, unsigned int length, unsigned char *data)
{
    int ret;
    ota_package_t ota;
    ota.offset = offset;
    ota.len = length;
    ota.data = data;
    ret = nwy_fota_download_core(&ota);
    if (ret == 0)
    {
        return 1;
    }
    return 0;
}

/**************************************************
@bref		��ʼ����
@param
@return
	1		�ɹ�
	0		ʧ��
@note
**************************************************/

uint8_t portUpgradeStart(void)
{
    int ret;
    ret = nwy_version_update(true);
    if (ret == 0)
    {
        return 1;
    }
    return 0;
}


/**************************************************
@bref		¼������ص�
@param
	pdata	¼��������
	len		����������
@note
**************************************************/

static int recSaveCallBack(unsigned char *pdata, unsigned int len)
{
    if ((record.recSize + len) < (RECORD_BUFF_SIZE))
    {
        memcpy(record.recBuff + record.recSize, pdata, len);
        record.recSize += len;
    }
    else
    {
        LogPrintf(DEBUG_FACTORY, "audioCallBack==>over large %d", record.recSize);
        portRecStop();
        recordRequestClear();
    }
    return 0;
}


/**************************************************
@bref		¼����ʼ��
@param
@return
@note
**************************************************/

void portRecInit(void)
{
    memset(&record, 0, sizeof(record_s));
    nwy_set_poc_sampleRate(8000);
    nwy_audio_set_record_format(NWY_AUSTREAM_FORMAT_AMRNB, NWY_AUENCODER_QUALITY_HIGH);
}


/**************************************************
@bref		¼����ʼ
@param
@return
@note
**************************************************/

int portRecStart(void)
{
    int ret;
    uint16_t year;
    uint8_t  month, date, hour, minute, second;

    if (record.recDoing != 0)
    {
        LogMessage(DEBUG_ALL, "portRecStart==>Fail");
        return 0;
    }
    portGetRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    sprintf(record.recFileName, "%02d%02d%02d%02d%02d%02d", year % 100, month, date, hour, minute, second);
    record.recSize = 0;
    LogPrintf(DEBUG_ALL, "portRecStart==>[%s]", record.recFileName);
    ret = nwy_audio_recorder_open(recSaveCallBack);
    if (ret != 0)
    {
        return -1;
    }
    ret = nwy_audio_recorder_start();
    if (ret != 0)
    {
        return -2;
    }
    record.recDoing = 1;
    return 1;
}
/**************************************************
@bref		¼������
@param
@return
@note
**************************************************/

void portRecStop(void)
{
    if (record.recDoing == 0)
        return;
    LogPrintf(DEBUG_ALL, "portRecStop==>[%s],Size:[%d]", record.recFileName, record.recSize);
    nwy_audio_recorder_stop();
    nwy_audio_recorder_close();
    record.recDoing = 0;
}

/**************************************************
@bref		¼����ʼ����
@param
@return
@note
**************************************************/

void portRecUpload(void)
{
    recSetUploadFile((uint8_t *)record.recFileName, record.recSize, record.recBuff);
}

/**************************************************
@bref		�Ƿ�����¼��
@param
@return
@note
**************************************************/

uint8_t portIsRecordNow(void)
{
    return record.recDoing;
}

/**************************************************
@bref		���õ�ǰ Radio ״̬
@param
	onoff	1������ģʽ			0������ģʽ
@return
	1		�ɹ�
	0		ʧ��
@note
**************************************************/
void portSetRadio(uint8_t onoff)
{
    int ret = 0;
    ret = nwy_nw_set_radio_st(onoff);
    LogPrintf(DEBUG_ALL, "Change to %s mode %s", onoff ? "normal" : "airplane", ret == 0 ? "success" : "fail");

}

/**************************************************
@bref		������ַ����
@param
@return
@note
**************************************************/

void bleMacParser(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t *rebuf;
    uint16_t relen;
    char MAC[15];
    rebuf = buf;
    relen = len;
    index = my_getstrindex((char *)rebuf, "+NWBTBLEMAC:", relen);
    if (index >= 0)
    {
        rebuf += index + 13;
        relen -= index + 13;
        if (relen >= 17)
        {
            MAC[0] = rebuf[0];
            MAC[1] = rebuf[1];
            MAC[2] = rebuf[3];
            MAC[3] = rebuf[4];
            MAC[4] = rebuf[6];
            MAC[5] = rebuf[7];
            MAC[6] = rebuf[9];
            MAC[7] = rebuf[10];
            MAC[8] = rebuf[12];
            MAC[9] = rebuf[13];
            MAC[10] = rebuf[15];
            MAC[11] = rebuf[16];
            MAC[12] = 0;
            LogPrintf(DEBUG_ALL, "BLE MAC:%s", MAC);
            protocolUpdateBleMac(MAC);
        }
    }
}

/**************************************************
@bref		����ATָ��
@param
@return
@note
**************************************************/

static void sendVirtualAT(char *cmdstr)
{
    int ret = -1;
    static nwy_at_info at_cmd;
    static char resp[128] = {0};
    memset(&at_cmd, 0, sizeof(nwy_at_info));
    at_cmd.length = strlen(cmdstr);
    memcpy(at_cmd.at_command, cmdstr, at_cmd.length);
    LogPrintf(DEBUG_ALL, "ATCmd:%s", cmdstr);
    memset(resp, 0, 128);
    ret =  nwy_sdk_at_cmd_send(&at_cmd, resp, NWY_AT_TIMEOUT_MIN);
    if (ret == NWY_SUCESS)
    {
        LogPrintf(DEBUG_ALL, "CmdRespon:%s", resp);
        bleMacParser((uint8_t *)resp, strlen(resp));
    }
    else if (ret == NWY_AT_GET_RESP_TIMEOUT)
    {
        LogMessage(DEBUG_ALL, "Cmd TimeOut");
    }
    else
    {
        LogMessage(DEBUG_ALL, "Cmd Error");
    }
}

/**************************************************
@bref		ָ���
@param
	cmd		ָ��ID,��atCmdType_e
	param	����
@return
	1		�ɹ�
	0		ʧ��
@note
**************************************************/

uint8_t  portSendCmd(uint8_t cmd, char *param)
{
    uint8_t i;
    int cmdtype = -1;
    char sendData[128];
    if (cmd >= MAX_NUM)
        return 0;
    for (i = 0; i < MAX_NUM; i++)
    {
        if (cmd == cmd_table[i].cmd_type)
        {
            cmdtype = i;
            break;
        }
    }
    if (cmdtype < 0)
    {
        sprintf(sendData, "portSendCmd==>No cmd");
        LogMessage(DEBUG_ALL, sendData);
        return 0;
    }
    if (param != NULL && strlen(param) <= 128)
    {
        if (param[0] == '?')
        {
            sprintf(sendData, "%s?\r\n", cmd_table[cmdtype].cmd);

        }
        else
        {
            sprintf(sendData, "%s=%s\r\n", cmd_table[cmdtype].cmd, param);
        }
    }
    else if (param == NULL)
    {
        sprintf(sendData, "%s\r\n", cmd_table[cmdtype].cmd);
    }
    else
    {
        return 0;
    }
    sendVirtualAT(sendData);
    return 1;
}

/**************************************************
@bref		����APN
@param
@return
@note
**************************************************/

void setAPN(void)
{
    char param[200];
    sprintf(param, "1,\"IP\",\"%s\"", (char *) sysparam.apn);
    portSendCmd(CGDCONT_CMD, param);
}
/**************************************************
@bref		����APN
@param
@return
@note
**************************************************/

void setXGAUTH(void)
{
    char param[200];
    sprintf(param, "1,1,\"%s\",\"%s\"", (char *) sysparam.apnuser, (char *) sysparam.apnpassword);
    portSendCmd(XGAUTH_CMD, param);
}

/**************************************************
@bref		�����绰����
@param
@return
@note
**************************************************/

void setCLIP(void)
{
    portSendCmd(CLIP_CMD, "1");
}

/**************************************************
@bref		�������
@param
@return
@note	+CLIP: "18814125495",129,,,,0
**************************************************/
static void receiveCLIP(void)
{
    char buf[128];
    nwy_get_voice_callerid(buf);
    if (my_strpach(buf, "+CLIP"))
    {
        LogPrintf(DEBUG_ALL, "Phone ring:%s", buf);

        if (sysparam.sosnumber1[0] != 0)
        {
            if (strstr((char *)buf, (char *)sysparam.sosnumber1) != NULL)
            {
                LogPrintf(DEBUG_ALL, "SOS number %s,anaswer phone", sysparam.sosnumber1);
                nwy_voice_call_autoanswver();
                return ;
            }
        }
        if (sysparam.sosnumber2[0] != 0)
        {
            if (strstr((char *)buf, (char *)sysparam.sosnumber2) != NULL)
            {
                LogPrintf(DEBUG_ALL, "SOS number %s,anaswer phone", sysparam.sosnumber2);
                nwy_voice_call_autoanswver();
                return ;
            }
        }
        if (sysparam.sosnumber3[0] != 0)
        {
            if (strstr((char *)buf, (char *)sysparam.sosnumber3) != NULL)
            {
                LogPrintf(DEBUG_ALL, "SOS number %s,anaswer phone", sysparam.sosnumber3);
                nwy_voice_call_autoanswver();
                return ;
            }
        }
        LogMessage(DEBUG_ALL, "Unknow phone number");
    }
}
/**************************************************
@bref		���Ž���
@param
@return
@note
**************************************************/

static void receiveCMT(void)
{
    nwy_sms_recv_info_type_t sms_data;
    instructionParam_s insParam;
    memset(&sms_data, 0, sizeof(sms_data));
    nwy_sms_recv_message(&sms_data);
    LogPrintf(DEBUG_ALL, "Tel[%d]:%s,Data[%d]:%s,Mt:%d,ShortId:%d,DCS:%d,Index:%d\r\n", sms_data.oa_size, sms_data.oa,
              sms_data.nDataLen, sms_data.pData, sms_data.cnmi_mt, sms_data.nStorageId, sms_data.dcs, sms_data.nIndex);
    sms_data.pData[sms_data.nDataLen + 1] = '#';
    sms_data.nDataLen += 1;
    memset(&insParam, 0, sizeof(instructionParam_s));
    insParam.mode = MESSAGE_MODE;
    insParam.telNum = sms_data.oa;
    instructionParser((uint8_t *)sms_data.pData, sms_data.nDataLen, &insParam);
}

/**************************************************
@bref		AT�����ʼ��
@param
@return
@note
**************************************************/

void portAtCmdInit(void)
{
    nwy_init_sms_option();
    nwy_sdk_at_parameter_init();
    nwy_sdk_at_unsolicited_cb_reg("+CLIP:", receiveCLIP);
    nwy_sdk_at_unsolicited_cb_reg("+CMT:", receiveCMT);
    nwy_set_report_option(2, 2, 0, 0, 0);
    setCLIP();
    portSendCmd(NWBTBLEMAC, "?");
}

/**************************************************
@bref		���Ͷ���
@param
	tel		����
	content	����
	len		�ڴ泤��
@return
@note
**************************************************/

void portSendSms(char *tel, char *content, uint16_t len)
{
    int ret;
    nwy_sms_info_type_t sms;
    strcpy((char *)sms.phone_num, tel);
    strncpy((char *)sms.msg_contxet, content, len);
    sms.msg_contxet[len] = 0;
    sms.msg_context_len = len;
    sms.msg_format = NWY_SMS_MSG_GSM7_DCS;
    LogPrintf(DEBUG_ALL, "SendSMS <%s>[%d]:%s", sms.phone_num, sms.msg_context_len, sms.msg_contxet);
    ret = nwy_sms_send_message(&sms);
    if (ret == 0)
    {
        LogMessage(DEBUG_ALL, "Send SMS success");
    }
    else
    {
        LogMessage(DEBUG_ALL, "Send SMS fail");
    }
}

/**************************************************
@bref		��������
@param
	vol		������Χ��0~100
@return
@note
**************************************************/

void portSetVol(uint8_t vol)
{
    vol = vol > 100 ? 100 : vol;
    nwy_audio_set_handset_vol(vol);
    LogPrintf(DEBUG_ALL, "portSetVol==>%d", vol);
}

/**************************************************
@bref		�������
@param
	vol		��ص�ѹ
@return
@note
**************************************************/

uint8_t portCapacityCalculate(float vol)
{
    uint8_t level = 0;
    if (vol > 4.05)
    {
        level = 100;

    }
    else if (vol < 3.34)
    {
        level = 0;
    }
    else
    {
        level = (uint8_t)(((vol - 3.34) / 0.71 * 100));
    }
    return level;
}


/**************************************************
@bref		���tts���ֽ���tts����
@param
	ttsbuf		��������
	ttsLen		���ֳ���
@return
@note
**************************************************/

static void portAddTTS(uint8_t *ttsbuf, uint8_t ttsLen)
{
    tts_info_s *next = NULL;
    tts_info_s *prv = NULL;
    if (ttslist == NULL)
    {
        ttslist = malloc(sizeof(tts_info_s));
        if (ttslist == NULL)
        {
            LogMessage(DEBUG_ALL, "portAddTTS==>malloc error");
            return ;
        }
        ttslist->next = NULL;
        memcpy(ttslist->ttsBuf, ttsbuf, ttsLen);
        ttslist->ttsBufLen = ttsLen;
        ttslist->ttsBuf[ttsLen] = 0;
    }
    else
    {
        prv = ttslist;
        next = prv->next;
        while (next != NULL)
        {
            prv = next;
            next = prv->next;
        }

        next = malloc(sizeof(tts_info_s));
        if (next == NULL)
        {
            LogMessage(DEBUG_ALL, "portAddTTS==>malloc error");
            return ;
        }
        prv->next = next;
        next->next = NULL;
        memcpy(next->ttsBuf, ttsbuf, ttsLen);
        next->ttsBufLen = ttsLen;
        next->ttsBuf[ttsLen] = 0;
    }
}


/**************************************************
@bref		���tts���ֽ���tts����
@param
	ttsbuf		��������
	ttsLen		���ֳ���
@return
@note
**************************************************/

static void ttsPlayCallBack(void *cb_para, nwy_neoway_result_t result)
{
    sysinfo.ttsPlayNow = 0;
    switch (result)
    {
        case PLAY_END:
            LogMessage(DEBUG_ALL, "Play done");
            break;
        default:
            LogMessage(DEBUG_ALL, "Play error");
            break;
    }
}

/**************************************************
@bref		���tts
@param
@return
@note
**************************************************/

void portOutputTTS(void)
{
    uint8_t restore[289];
    tts_info_s *next;
    if (ttslist == NULL)
    {
        return;
    }
    if (sysinfo.ttsPlayNow == 1)
    {
        LogMessage(DEBUG_ALL, "portOutputTTS==>Waitting...");
        return ;
    }
    sysinfo.ttsPlayNow = 1;
    changeByteArrayToHexString(ttslist->ttsBuf, restore, ttslist->ttsBufLen);
    restore[ttslist->ttsBufLen * 2] = 0;
    LogPrintf(DEBUG_ALL, "portOutputTTS==>%s", ttslist->ttsBuf);
    nwy_tts_playbuf((char *)restore, ttslist->ttsBufLen * 2, ENCODE_GBK, ttsPlayCallBack, NULL);

    next = ttslist->next;
    free(ttslist);
    ttslist = next;
    if (ttslist == NULL)
    {
        LogMessage(DEBUG_ALL, "portOutputTTS==>Done");
    }

}

/**************************************************
@bref		ѹ��tts����
@param
	ttsbuf	��������
@return
@note
**************************************************/

void portPushTTS(char *ttsbuf)
{
    uint8_t ttslen;
    if (ttsbuf == NULL)
        return;
    ttslen = strlen(ttsbuf);
    if (ttslen == 0)
        return;
    ttslen = ttslen > 144 ? 144 : ttslen;
    portAddTTS((uint8_t *)ttsbuf, ttslen);
}

