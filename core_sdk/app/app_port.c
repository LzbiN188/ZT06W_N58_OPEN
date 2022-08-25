#include "app_port.h"
#include "nwy_uart.h"
#include "app_sys.h"
#include "nwy_adc.h"
#include "nwy_pm.h"
#include "app_param.h"
#include "app_mir3da.h"
#include "nwy_audio_api.h"
#include "nwy_file.h"
#include "app_customercmd.h"
#include "app_task.h"


UART_RXTX_CTL usart1_ctl;
UART_RXTX_CTL usart2_ctl;
UART_RXTX_CTL usart3_ctl;

/*----------------------------------------------------------------------------------------------------------------------*/
//UART
void appUartConfig(UARTTYPE type, uint8_t onoff, uint32_t baudrate, void (*rxhandlefun)(char *, uint32_t))
{
    int ret = -1;
    int hd = -1;
    switch (type)
    {
        case APPUSART1:
            if (onoff == 0)
            {
                LogMessage(DEBUG_ALL, "Close uart1\r\n");
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
                LogPrintf(DEBUG_ALL, "Open uart1==>%d\r\n", baudrate);
            }
            break;
        case APPUSART2:
            if (onoff == 0)
            {
                LogMessage(DEBUG_ALL, "Close uart2\r\n");
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
                LogPrintf(DEBUG_ALL, "Open uart2==>%d\r\n", baudrate);
            }
            break;
        case APPUSART3:
            if (onoff == 0)
            {
                LogMessage(DEBUG_ALL, "Close uart3\r\n");
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
                LogPrintf(DEBUG_ALL, "Open uart3==>%d\r\n", baudrate);
            }
            break;
    }

    if (ret <= 0 && onoff == 1)
    {
        LogPrintf(DEBUG_ALL, "Open uart%d error\r\n", type + 1);
    }
}

void appUartSend(UART_RXTX_CTL *uart_ctl, uint8_t *buf, uint16_t len)
{
    if (uart_ctl->init)
    {
        nwy_uart_send_data(uart_ctl->uart_handle, buf, len);
    }
}
/*----------------------------------------------------------------------------------------------------------------------*/
//RTC
void getRtcDateTime(uint16_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute, uint8_t *second)
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
void updateRTCdatetime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    nwy_time_t rtc_time;
    rtc_time.year = year + 2000;
    rtc_time.mon = month;
    rtc_time.day = date;
    rtc_time.hour = hour;
    rtc_time.min = minute;
    rtc_time.sec = second;
    LogPrintf(DEBUG_ALL, "UpdateRTC:%d-%d-%d %d:%d:%d TZ:%d\r\n", rtc_time.year, rtc_time.mon, rtc_time.day,
              rtc_time.hour,
              rtc_time.min, rtc_time.sec, sysparam.utc);
    nwy_set_time(&rtc_time, sysparam.utc * 4);

}

void setNextAlarmTime(void)
{
    unsigned short  rtc_mins, next_ones;
    unsigned char next_date, set_nextdate = 1;
    uint16_t  YearToday;      /*当前年*/
    uint8_t  MonthToday;     /*当前月*/
    uint8_t  DateToday;      /*当前日*/
    int i;
    uint16_t year = 0;
    uint8_t  month = 0;
    uint8_t date = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);

    //1、读取当前时间点的总分钟数
    rtc_mins = (hour & 0x1F) * 60;
    rtc_mins += (minute & 0x3f);
    //2、读取当前年月
    YearToday = year; //计算当前年，从2000年开始算起
    MonthToday = month;
    DateToday = date;
    //3、根据当前月，计算下个月日期
    if (MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + sysparam.gapDay) % 30; //当前日期加上间隔日，计算下一次的时间点
        if (next_date == 0)
            next_date = 30;
    }
    else if (MonthToday == 2)
    {
        //4、如果是2月，判断是不是闰年
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || (YearToday % 400 == 0))  //闰年
        {
            next_date = (DateToday + sysparam.gapDay) % 29;
            if (next_date == 0)
                next_date = 29;
        }
        else
        {
            next_date = (DateToday + sysparam.gapDay) % 28;
            if (next_date == 0)
                next_date = 28;
        }
    }
    else
    {
        next_date = (DateToday + sysparam.gapDay) % 31;
        if (next_date == 0)
            next_date = 31;
    }
    next_ones = 0xFFFF;
    //5、查找闹铃表里面是否有在当前时间点之后的时间
    for (i = 0; i < 5; i++)
    {
        if (sysparam.AlarmTime[i] == 0xFFFF)
            continue;
        if (sysparam.AlarmTime[i] > rtc_mins)   //跟当前时间比对
        {
            next_ones = sysparam.AlarmTime[i];  //得到新的时间
            set_nextdate = 0;
            break;
        }
    }


    if (next_ones == 0xFFFF)  //没有配置时间
    {
        //Set Current Alarm Time
        next_ones = sysparam.AlarmTime[0];
        if (next_ones == 0xFFFF)
        {
            next_ones = 720; //默认12:00
        }
    }
    //6、设置下次上报的日期和时间
    if (set_nextdate)
        LogPrintf(DEBUG_ALL, "NextAlarmTime:%d,%02d:%02d\r\n", next_date, (next_ones / 60) % 24, next_ones % 60);
    else
        LogPrintf(DEBUG_ALL, "NextAlarmTime:%d,%02d:%02d\r\n", date, (next_ones / 60) % 24, next_ones % 60);
}


void setNextWakeUpTime(void)
{
    uint16_t  YearToday;      /*当前年*/
    uint8_t  MonthToday;     /*当前月*/
    uint8_t  DateToday;      /*当前日*/
    uint8_t  date = 0, next_date;
    uint16_t totalminutes;
    uint16_t year = 0;
    uint8_t  month = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;

    getRtcDateTime(&year, &month, &day, &hour, &minute, &second);
    YearToday = year; //计算当前年，从2000年开始算起
    MonthToday = month;
    DateToday = day;
    totalminutes = hour * 60 + minute;
    totalminutes += sysparam.gapMinutes;
    if (totalminutes >= 1440)
    {
        date = 1;
        totalminutes -= 1440;
    }
    //3、根据当前月，计算下个月日期
    if (MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + date) % 30; //当前日期加上间隔日，计算下一次的时间点
        if (next_date == 0)
            next_date = 30;
    }
    else if (MonthToday == 2)
    {
        //4、如果是2月，判断是不是闰年
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || (YearToday % 400 == 0))  //闰年
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
    LogPrintf(DEBUG_ALL, "NextWakeUpTime:%d,%02d:%02d\r\n", next_date, (totalminutes / 60) % 24, totalminutes % 60);
}

/*----------------------------------------------------------------------------------------------------------------------*/

//GSENSOR
void gpioInterrupt(int    param)
{

    if (param == DTR_PORT)
    {
        //wake up
        if (DTR_READ == 1)
        {
            sysinfo.dtrState = 1;
            uartCtl(1);
        }
        //sleep
        else
        {
            sysinfo.dtrState = 0;
        }
    }
    else if (param == BLE_RING_PORT)
    {
    	sysinfo.bleRing=3;
        uartCtl(1);
    }

}
/*----------------------------------------------------------------------------------------------------------------------*/

void gsintPreConfig(void)
{
    //nwy_close_gpio(GSINT_PORT);
    //nwy_open_gpio_irq_config(GSINT_PORT, nwy_irq_rising, gpioInterrupt);
}
void gsensorConfig(uint8_t onoff)
{
    //nwy_gpio_set_direction(GSPWR_GPIO, 1);
    //    if (onoff)
    //    {
    //        //nwy_gpio_set_value(GSPWR_GPIO, 0);
    //        mir3da_init();
    //        mir3da_set_enable(1);
    //        mir3da_open_interrupt(10);
    //        LogMessage(DEBUG_ALL, "gsensor on\r\n");
    //        nwy_gpio_open_irq_enable(0);
    //        sysinfo.gsensoronoff = 1;
    //    }
    //    else
    //    {
    //        //nwy_gpio_set_value(GSPWR_GPIO, 1);
    //        LogMessage(DEBUG_ALL, "gsensor off\r\n");
    //        nwy_gpio_open_irq_disable(0);
    //        sysinfo.gsensoronoff = 0;
    //    }
}
/*----------------------------------------------------------------------------------------------------------------------*/
//ADC
void getBatVoltage(void)
{
    sysinfo.batvoltage = nwy_adc_get(NWY_ADC_CHANNEL_VBAT, NWY_ADC_SCALE_5V000) / 1000.0;
    sysinfo.outsidevoltage = nwy_adc_get(NWY_ADC_CHANNEL3, NWY_ADC_SCALE_3V233) / 1000.0;
    sysinfo.outsidevoltage *= sysparam.adccal;
}
/*----------------------------------------------------------------------------------------------------------------------*/
//LED
void ledConfig(void)
{
    //nwy_gpio_set_direction(LED1_PORT, nwy_output);
    //nwy_gpio_set_direction(LED2_PORT, nwy_output);
    //nwy_gpio_set_direction(LED3_PORT, nwy_output);
}
/*----------------------------------------------------------------------------------------------------------------------*/
//PMU
void pmuConfig(void)
{
    nwy_subpower_switch(NWY_POWER_SD, true, true);
    nwy_set_pmu_power_level(NWY_POWER_SD, 3000);

    nwy_subpower_switch(NWY_POWER_CAMD, false, false);
    nwy_set_pmu_power_level(NWY_POWER_CAMD, 0);

    nwy_subpower_switch(NWY_POWER_CAMA, true, false);
    nwy_set_pmu_power_level(NWY_POWER_CAMA, 0);
}
/*----------------------------------------------------------------------------------------------------------------------*/
//GPS IO
void gpsConfig(void)
{
    //nwy_gpio_set_direction(GPSLNA_PORT, nwy_output);
}
/*----------------------------------------------------------------------------------------------------------------------*/
//通用gpio配置
void gpioConfig(void)
{
    //锁车控制
    //nwy_gpio_set_direction(LOCK_PORT, nwy_output);
    //上电控制
    //nwy_gpio_set_direction(SUPPLY_PORT, nwy_output);
    //超速检测
    //nwy_gpio_set_direction(SPEED_PORT, nwy_input);
    //nwy_close_gpio(SPEED_PORT);
    //nwy_open_gpio_irq_config(SPEED_PORT, nwy_irq_rising, gpioInterrupt);
    //转动检测
    //nwy_gpio_set_direction(ROLL_PORT, nwy_input);
    //nwy_close_gpio(ROLL_PORT);
    //nwy_open_gpio_irq_config(ROLL_PORT, nwy_irq_rising, gpioInterrupt);
    //点火检测
    //nwy_gpio_set_direction(ACC_PORT, nwy_input);
    //感光检测
    //nwy_gpio_set_direction(LDR_PORT, nwy_input);
    //nwy_gpio_pullup_or_pulldown(LDR_PORT, 1);


    nwy_gpio_set_direction(RING_PORT, nwy_output);
	nwy_gpio_set_direction(BLE_PWR_PORT,nwy_output);
	BLE_PWR_OFF;

    nwy_close_gpio(DTR_PORT);
    nwy_open_gpio_irq_config(DTR_PORT, nwy_irq_rising, gpioInterrupt);
    nwy_gpio_open_irq_enable(DTR_PORT);


}
/*----------------------------------------------------------------------------------------------------------------------*/
//系统重启
void appSystemReset(void)
{
    LogPrintf(DEBUG_ALL, "system reset\r\n");
    nwy_power_off(2);
}
/*----------------------------------------------------------------------------------------------------------------------*/
//触发事件
void appSendThreadEvent(uint16 sig, uint32_t param1)
{
    nwy_osiEvent_t event;
    memset(&event, 0, sizeof(event));
    event.id = sig;
    event.param1 = param1;
    nwy_send_thead_event(myAppEventThread, &event, 0);
}
/*----------------------------------------------------------------------------------------------------------------------*/
//TTS播放
static void ttsPlayCallBack(void *cb_para, nwy_neoway_result_t result)
{
    sysinfo.ttsPlayNow = 0;
    switch (result)
    {
        case PLAY_END:
            LogPrintf(DEBUG_ALL, "Play done\r\n");
            customerLogOut("+CTTSPLAY: OK\r\n");
            break;
        default:
            LogPrintf(DEBUG_ALL, "Play Error:%d\r\n", result);
            customerLogOut("+CTTSPLAY: ERROR\r\n");
            break;
    }
}

void appTTSPlay(char *ttsbuf)
{
    uint8_t ttslen;
    uint8_t restore[289];
    ttslen = strlen(ttsbuf);
    if (ttsbuf == NULL || ttslen == 0)
        return;
    sysinfo.ttsPlayNow = 1;
    ttslen = ttslen > 144 ? 144 : ttslen;
    nwy_tts_stop_play();
    LogPrintf(DEBUG_ALL, "appTTSPlay==>%s\r\n", ttsbuf);
    changeByteArrayToHexString((uint8_t *)ttsbuf, restore, ttslen);
    restore[ttslen * 2] = 0;
    //LogPrintf(DEBUG_ALL, "appTTSPlay==>%s\r\n", restore);
    if (sysinfo.playMusicNow == 1)
    {
        LogMessage(DEBUG_ALL, "appTTSPlay==>Exit,music was play\r\b");
        return;
    }
    nwy_tts_playbuf((char *)restore, ttslen * 2, ENCODE_GBK, ttsPlayCallBack, NULL);
}

void appTTSStop(void)
{
    nwy_tts_stop_play();
}
void appSaveAudio(uint8_t *audio, uint16_t len)
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
        LogMessage(DEBUG_ALL, "appSaveAudio==>Open error\r\n");
        return;
    }
    writelen = nwy_sdk_fwrite(fd, audio, len);
    if (writelen != len)
    {
        LogPrintf(DEBUG_ALL, "appSaveAudio==>Error[%d]\r\n", writelen);
    }
    else
    {
        LogMessage(DEBUG_ALL, "appSaveAudio==>Success\r\n");
    }
    nwy_sdk_fclose(fd);
}

void appDeleteAudio(void)
{
    if (nwy_sdk_fexist(AUDIOFILE))
    {
        if (nwy_sdk_file_unlink(AUDIOFILE) == 0)
        {
            LogMessage(DEBUG_ALL, "Delete audio file success\r\n");
        }
        else
        {
            LogMessage(DEBUG_ALL, "Delete audio file fail\r\n");
        }
    }
    else
    {
        LogMessage(DEBUG_ALL, "Audio file not exist\r\n");
    }
}

void appPlayAudio(void)
{
    long size = 0;
    size = nwy_sdk_fsize(AUDIOFILE);
    LogPrintf(DEBUG_ALL, "Play Begin:%s[%d Byte]\r\n", AUDIOFILE, size);
    nwy_audio_file_play(AUDIOFILE);
    LogMessage(DEBUG_ALL, "Play Done\r\n");
    appDeleteAudio();
}

void portBleGpioCfg(void)
{
    nwy_close_gpio(BLE_RING_PORT);
    nwy_open_gpio_irq_config(BLE_RING_PORT, nwy_irq_rising, gpioInterrupt);
    nwy_gpio_open_irq_enable(BLE_RING_PORT);

    nwy_gpio_set_direction(BLE_DTR_PORT, nwy_output);
}
