#include "nwy_test_cli_utils.h"
#include "nwy_osi_api.h"
#include "nwy_file.h"
#include "nwy_lcd_bus.h"
#include "nwy_pm.h"
#include "osi_log.h"
#include "nwy_uart.h"
#include "nwy_gpio_open.h"
#include "nwy_i2c.h"
#include "nwy_spi.h"
#include "osi_compiler.h"
#include "nwy_fota_api.h"
#include "nwy_adc.h"
#include "nwy_pm.h"
#include "nwy_keypad.h"
#include "nwy_pwm.h"
#include "nwy_file.h"
#include "nwy_audio_api.h"

/**************************UART*********************************/
int hd;
void nwy_test_cli_uart_init()
{
    char *opt;
    uint8_t port, mode;
    uint32_t name;
    opt = nwy_test_cli_input_gets("\r\nPlease input the uart id(1-URT1,2-URT2 or 3-URT3):");
    port = atoi(opt);
    if (port == 1)
        name = NWY_NAME_UART1;
    else if (port == 2)
        name = NWY_NAME_UART2;
    else if (port == 3)
        name = NWY_NAME_UART3;
    else
    {
        nwy_test_cli_echo("\r\n Input uart id is invalid!");
        return;
    }
    nwy_test_cli_echo("\r\nTest port = %d!\r\n", port); //delet

    opt = nwy_test_cli_input_gets("\r\nPlease input the uart mode(0-AT,1-DATA):");
    mode = atoi(opt);
    nwy_test_cli_echo("\r\nTest port = %d!\r\n", mode); //delet

    hd = nwy_uart_init(name, mode);
    if (hd < 0)
        nwy_test_cli_echo("\r\nTest uart error!\r\n"); //delet
    nwy_test_cli_echo("\r\nTest uart success!\r\n");   //delet
}

void nwy_test_cli_uart_set_baud()
{
    char *opt;
    uint32_t baud;
    opt = nwy_test_cli_input_gets("\r\nPlease input the uart baud:");
    baud = atoi(opt);
    nwy_test_cli_echo("\r\nTest baud = %d,hd = %d!\r\n", baud, hd); //delet

    if (1200 <= baud && 8000000 >= baud)
    {
        nwy_uart_set_baud(hd, baud);
        nwy_test_cli_echo("\r\nSet uart baud success!\r\n");
    }
    else
        nwy_test_cli_echo("\r\nSet uart baud param error!\r\n");
}

void nwy_test_cli_uart_get_baud()
{
    uint32_t baud;
    nwy_uart_get_baud(hd, &baud);
    nwy_test_cli_echo("\r\nRead uart id = %d, baud = %d!\r\n", hd + 1, baud);
}

void nwy_test_cli_uart_set_para()
{
    nwy_uart_parity_t parity;
    nwy_uart_data_bits_t data_size;
    nwy_uart_stop_bits_t stop_size;
    bool flow_ctrl;
    char *opt;
    opt = nwy_test_cli_input_gets("\r\nTest in set the uart param:");

    switch (*opt++)
    {
    case 'p':
        nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
        parity = atoi(opt);
        if (0 <= parity && 2 >= parity)
        {
            nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
            nwy_test_cli_echo("\r\nSwitch parity to:%d\r\n", parity);
        }
        else
            nwy_test_cli_echo("\r\nTest invalid parity Supported!\r\n", parity);
        break;

    case 'd':
        nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
        data_size = atoi(opt);
        if (7 <= data_size && 8 >= data_size)
        {
            nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
            nwy_test_cli_echo("\r\nSwitch data_size to:%d\r\n", data_size);
        }
        else
            nwy_test_cli_echo("\r\nInvalid data_size:%d\r\n", data_size);
        break;

    case 's':
        nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
        stop_size = atoi(opt);
        if (1 <= stop_size && 2 >= stop_size)
        {
            nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
            nwy_test_cli_echo("\r\nSwitch stop_size to:%d\r\n", stop_size);
        }
        else
            nwy_test_cli_echo("\r\nInvalid stop_size:%d\r\n", stop_size);
        break;

    case 'f':
        nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
        flow_ctrl = atoi(opt);
        if (0 <= parity && 1 >= parity)
        {
            nwy_uart_set_para(hd, parity, data_size, stop_size, flow_ctrl);
            nwy_test_cli_echo("\r\nSwitch flow_ctrl to:%d\r\n", flow_ctrl);
        }
        else
            nwy_test_cli_echo("\r\nInvalid flow_ctrl:%d\r\n", flow_ctrl);
        break;

    default:
        break;
    }
}

void nwy_test_cli_uart_get_para()
{
    nwy_uart_parity_t parity;
    nwy_uart_data_bits_t data_size;
    nwy_uart_stop_bits_t stop_size;
    bool flow_ctrl;

    nwy_uart_get_para(hd, &parity, &data_size, &stop_size, &flow_ctrl);
    nwy_test_cli_echo("\r\nUart parity = %d\r\n", parity);
    nwy_test_cli_echo("\r\nUart data_size = %d\r\n", data_size);
    nwy_test_cli_echo("\r\nUart stop_size = %d\r\n", stop_size);
    nwy_test_cli_echo("\r\nUart flow_ctrl = %d\r\n", flow_ctrl);
}

void nwy_test_cli_uart_set_tout()
{
    int timeout;
    char *opt;
    opt = nwy_test_cli_input_gets("\r\nTest in set the uart receive timeout(default:32ms):");
    timeout = atoi(opt);

    nwy_set_rx_frame_timeout(hd, timeout);
    nwy_test_cli_echo("\r\nSet rx frame timeout = %d\r\n", timeout);
}

void nwy_test_cli_uart_send()
{
    char *opt;
    opt = nwy_test_cli_input_gets("\r\nTest in set the uart send data:");

    nwy_uart_send_data(hd, (uint8 *)opt, strlen(opt));
}

static void nwy_uart_recv_handle(const char *str, uint32_t length)
{
    nwy_uart_send_data(hd, (uint8 *)str, length);
    nwy_test_cli_echo("\r\nUart send data length = %d\r\n", length);
}

void nwy_test_cli_uart_reg_rx_cb()
{
    nwy_uart_reg_recv_cb(hd, nwy_uart_recv_handle);
}

#define RS485_GPIO_PORT (2)
#define RS485_DIR_TX (1) //hight level for send
#define RS485_DIR_RX (0) //low level for recv
/*if send completly, the callback func will set RS485 as rx state*/
static void nwy_rs485_direction_switch(int port, int value)
{
    nwy_gpio_set_direction(port, 1);
    nwy_gpio_set_value(port, (nwy_value_t)value);
}
static void nwy_uart_send_complet_handle(int param)
{
    nwy_sleep(10);
    nwy_rs485_direction_switch(RS485_GPIO_PORT, RS485_DIR_RX);
    nwy_test_cli_echo("\r\nUart send complet handle success!\r\n");
}

void nwy_test_cli_uart_reg_tx_cb()
{
    char *pstsnd = "hellors485";
    nwy_rs485_direction_switch(RS485_GPIO_PORT, RS485_DIR_RX);

    /*register cb func to uart drv */
    nwy_uart_reg_tx_cb(hd, nwy_uart_send_complet_handle);

    /* for send, set RS485 as tx state */
    nwy_rs485_direction_switch(RS485_GPIO_PORT, RS485_DIR_TX);
    nwy_uart_send_data(hd, (uint8_t *)pstsnd, strlen(pstsnd));
}

void nwy_test_cli_uart_deinit()
{
    int close;
    char *opt;
    opt = nwy_test_cli_input_gets("\r\nSure to close this uart(0-no, 1-yes):");

    close = atoi(opt);
    if (close)
        nwy_uart_deinit(hd);
}

/**************************I2C*********************************/
int i2c_bus;
void nwy_test_cli_i2c_init()
{
    char *opt;
    uint8_t port;
    char *name;
    opt = nwy_test_cli_input_gets("\r\nPlease input the I2C id(1-I2C1,2-I2C2 or 3-I2C3):");
    port = atoi(opt);
    if (port == 1)
    {
        name = NAME_I2C_BUS_1;
    }
    else if (port == 2)
    {
        name = NAME_I2C_BUS_2;
    }
    else if (port == 3)
    {
        name = NAME_I2C_BUS_3;
    }
    else
    {
        nwy_test_cli_echo("\r\n Input I2C id is invalid!");
        return;
    }

    i2c_bus = nwy_i2c_init(name, NWY_I2C_BPS_100K);
    if (NWY_SUCESS > i2c_bus)
    {
        nwy_test_cli_echo("\r\nI2c Error : bus:%s init fail\r\n", name);
        return;
    }
}

#define BMA400_DEV_ADDR 0x14
void nwy_test_cli_i2c_read()
{
    uint8_t rtn, sensor_id;
    rtn = nwy_i2c_read(i2c_bus, BMA400_DEV_ADDR, 0x00, &sensor_id, 1);
    if (NWY_SUCESS == rtn)
        nwy_test_cli_echo("\r\nNWY get sensor id = 0x%x!\r\n", sensor_id);
    else
        nwy_test_cli_echo("\r\nNWY read I2C error!\r\n");
}

void nwy_test_cli_i2c_write()
{
    uint8_t temp = 0x1c;
    uint8_t *data = &temp;
    uint8_t rtn;
    rtn = nwy_i2c_write(i2c_bus, BMA400_DEV_ADDR, 0xf4, data, 1);
    if (NWY_SUCESS == rtn)
        nwy_test_cli_echo("\r\nNWY write I2C success!\r\n");
    else
        nwy_test_cli_echo("\r\nNWY write I2C error!\r\n");
}

void nwy_test_cli_i2c_put_raw()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_i2c_get_raw()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_i2c_deinit()
{
    int close;
    char *opt;
    opt = nwy_test_cli_input_gets("\r\nSure to close this i2c(0-no, 1-yes):");

    close = atoi(opt);
    if (close)
        nwy_i2c_deinit(i2c_bus);
}

/**************************SPI*********************************/
int spi_bus;
void nwy_test_cli_spi_init()
{
    char *opt;
    uint8_t port;
    char *name;
    opt = nwy_test_cli_input_gets("\r\nPlease input the SPI id(1-spi1,2-spi1):");
    port = atoi(opt);
    if (port == 1)
    {
        name = NAME_SPI_BUS_1;
    }
    else if (port == 2)
    {
        name = NAME_SPI_BUS_2;
    }
    else
    {
        nwy_test_cli_echo("\r\n Input SPI id is invalid!");
        return;
    }

    spi_bus = nwy_spi_init(name, SPI_MODE_0, 1000000, 8);
    if (NWY_SUCESS > spi_bus)
    {
        nwy_test_cli_echo("\r\nSPI Error : bus:%s init fail\r\n", name);
        return;
    }
}

#define SPI_DUMMY 0xFF
#define w25x_jedecDeviceID 0x9f
void nwy_test_cli_spi_trans()
{
    uint8_t OSI_ALIGNED(16) Command[4] = {w25x_jedecDeviceID, 0xFF, 0xFF, 0xFF};
    uint8_t OSI_ALIGNED(16) FlashId[4] = {0};
    int rtn = nwy_spi_transfer(spi_bus, SPI_CS_0, Command, FlashId, 4);

    if (SPI_EC_SUCESS == rtn)
        nwy_test_cli_echo("\r\nSpi flash read id %02x,%02x,%02x\r\n", FlashId[1], FlashId[2], FlashId[3]);
    else
        nwy_test_cli_echo("\r\nSpi error:transfer fail!\r\n");
}

void nwy_test_cli_spi_deinit()
{
    int close;
    char *opt;
    opt = nwy_test_cli_input_gets("\r\nSure to close this spi(0-no, 1-yes):");

    close = atoi(opt);
    if (close)
        nwy_spi_deinit(spi_bus);
}

/**************************GPIO*********************************/
#include "nwy_gpio_open.h"
void nwy_test_cli_gpio_set_val()
{
    char *opt;
    uint8_t port, vol;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);
    opt = nwy_test_cli_input_gets("\r\nSet the gpio value(0-low level,1-high level):");
    vol = atoi(opt);

    nwy_gpio_set_value(port, (nwy_value_t)vol);
}

void nwy_test_cli_gpio_get_val()
{
    char *opt;
    uint32_t port, vol;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);

    vol = nwy_gpio_get_value(port);
    nwy_test_cli_echo("\r\nGet the GPIO value = %d\r\n", vol);
}

void nwy_test_cli_gpio_set_dirt()
{
    char *opt;
    uint8_t port, dir;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO dir(0-input,1-output):");
    dir = atoi(opt);

    nwy_gpio_set_direction(port, dir);
}

void nwy_test_cli_gpio_get_dirt()
{
    char *opt;
    uint8_t port, dir;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);

    dir = nwy_gpio_get_direction(port);
    nwy_test_cli_echo("\r\nGet the GPIO dir = %d\r\n", dir);
}

static void _gpioisropen(int param)
{
    nwy_test_cli_echo("\r\nGPIO isr set success\r\n");
}

void nwy_test_cli_gpio_config_irq()
{
    char *opt;
    uint8_t port, mode;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO irq mode(0-rising,2-rising&falling,3-high):");
    mode = atoi(opt);

    nwy_close_gpio(port);
    int data = nwy_open_gpio_irq_config(port, mode, _gpioisropen);

    if (data)
    {
        nwy_test_cli_echo("\r\nGpio isr config success!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\nGpio isr config failed!\r\n");
    }
}

void nwy_test_cli_gpio_enable_irq()
{
    char *opt;
    uint8_t port;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);

    nwy_gpio_open_irq_enable(port);

    nwy_test_cli_echo("\r\nGpio enable isr success!\r\n");
}

void nwy_test_cli_gpio_disable_irq()
{
    char *opt;
    uint8_t port;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);

    nwy_gpio_open_irq_disable(port);

    nwy_test_cli_echo("\r\nGpio enable isr success!\r\n");
}

void nwy_test_cli_gpio_close()
{
    char *opt;
    uint8_t port;
    opt = nwy_test_cli_input_gets("\r\nSet the GPIO id:");
    port = atoi(opt);

    nwy_close_gpio(port);
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

/**************************ADC*********************************/
void nwy_test_cli_adc_read()
{
    char *opt;
    uint8_t port, mode;
    uint32_t adc_vol;
    opt = nwy_test_cli_input_gets("\r\nChoose the ADC channel(2-CHANNEL2,3-CHANNEL3,4-VBAT):");
    port = atoi(opt);
    opt = nwy_test_cli_input_gets("\r\nChoose the ADC scale(0-1V250,1-2V444,2-3V233,3-5V000):");
    mode = atoi(opt);

    adc_vol = nwy_adc_get(port, mode);
    nwy_test_cli_echo("\r\nAdc get value = %d\r\n", adc_vol);
}

/**************************PM*********************************/
void nwy_test_cli_pm_save_md()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_pm_get_pwr_st()
{
    int pwr_st = nwy_power_state();
    if (NWY_POWER_NORMAL_STATE == pwr_st)
        nwy_test_cli_echo("\r\nPower state in normal!\r\n");
    else
        nwy_test_cli_echo("\r\nPower state in drop!\r\n");
}

void nwy_test_cli_pm_pwr_off()
{
    char *opt;
    uint8_t mode;
    opt = nwy_test_cli_input_gets("\r\nChoose the power off mode(0-quickly,1-normal,2-reset):");
    mode = atoi(opt);
    nwy_power_off(mode);
}

void nwy_test_cli_pm_set_dtr()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_pm_pwr_key()
{
    char *opt;
    uint8_t mode;
    opt = nwy_test_cli_input_gets("\r\nPlease input status(0-close,1(default)-open:");
    mode = atoi(opt);

    nwy_powerkey_poweroff_ctrl(mode);

    nwy_test_cli_echo("\r\nClose key poweroff test in %d\r\n", mode);
}

void nwy_test_cli_pm_switch_sub_pwr()
{
    char *opt;
    uint8_t power_id, mode;
    opt = nwy_test_cli_input_gets("\r\nPlease input sub power id:");
    power_id = atoi(opt);

    nwy_subpower_switch(power_id, true, false);
    nwy_test_cli_echo("\r\nSet the power %d open!\r\n", power_id);

    opt = nwy_test_cli_input_gets("\r\nPlease sure to close this sub power:");
    mode = atoi(opt);

    if (mode)
    {
        nwy_subpower_switch(power_id, false, false);
        nwy_test_cli_echo("\r\nSet the power %d closed!\r\n", power_id);
    }

    nwy_test_cli_echo("\r\nSet the power %d already open!\r\n", power_id);
}

void nwy_test_cli_pm_set_sub_pwr()
{
    char *opt;
    uint8_t power_id, level;
    opt = nwy_test_cli_input_gets("\r\nPlease input sub power id:");
    power_id = atoi(opt);

    opt = nwy_test_cli_input_gets("\r\nPlease set the sub power level you want(mV):");
    level = atoi(opt);

    nwy_set_pmu_power_level(power_id, level);
    nwy_test_cli_echo("\r\nSet the power %d in %d mV!\r\n", power_id, level);
}

void nwy_shutdown_cb(NWY_SVR_MSG_SERVICE_E msg, uint32_t param)
{
    if (msg == NWY_WARNING_IND)
        OSI_LOGI(0, "nwy the capacity is low, should charge");
    else if (msg == NWY_SHUTDOWN_IND)
        OSI_LOGI(0, "nwy the capacity is very low and must shutdown");
}

void nwy_test_cli_pm_set_auto_off()
{
    char *opt;
    uint16_t shut_vol, dead_vol, count;
    opt = nwy_test_cli_input_gets("\r\nPlease input shutdown vol:");
    shut_vol = atoi(opt);

    opt = nwy_test_cli_input_gets("\r\nPlease input deadline vol:");
    dead_vol = atoi(opt);

    opt = nwy_test_cli_input_gets("\r\nPlease input count:");
    count = atoi(opt);
    nwy_set_auto_poweroff(shut_vol, dead_vol, count, nwy_shutdown_cb);
}

void nwy_charging_cb(NWY_SVR_MSG_SERVICE_E msg, uint32_t param)
{
    if (msg == NWY_CHARGE_START_IND)
        OSI_LOGI(0, "nwy start chargering");
    else if (msg == NWY_CHARGE_DISCONNECT)
        OSI_LOGI(0, "nwy disconnect chargering");
    else if (msg == NWY_CHARGE_FINISH)
        OSI_LOGI(0, "nwy charger finished");
}

void nwy_test_cli_pm_reg_charger_cb()
{
    nwy_chargering_instructions(nwy_charging_cb);
    nwy_test_cli_echo("\r\nTest in chager cb!\r\n");
}

/**************************KEYPAD*********************************/
static void _openkeypad(nwy_key_t key, nwy_keyState_t evt)
{
    uint8_t status;
    if (evt & key_state_press)
        status = 1;
    if (evt & key_state_release)
        status = 0;

    if (evt == key_state_press)
    {
        nwy_test_cli_echo("\r\nThis key%d is press\r\n", key);
    }
    else
    {
        nwy_test_cli_echo("\r\n key id %d,status %d", key, status);
    }
}

void nwy_test_cli_keypad_reg_cb()
{
    nwy_test_cli_echo("\r\nTest in keypad cb!\r\n");
    reg_nwy_key_cb(_openkeypad);
}

void nwy_test_cli_keypad_set_debouce()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

/**************************PWM*********************************/
nwy_pwm_t *test_p;
void nwy_test_cli_pwm_init()
{
    test_p = nwy_pwm_init(NAME_PWM_1, 100, 40);

    if (test_p == NULL)
    {
        nwy_test_cli_echo("\r\nPWM init failed!\r\n");
    }
    nwy_test_cli_echo("\r\nPWM init success!\r\n");
}

void nwy_test_cli_pwm_start()
{
    nwy_pwm_start(test_p);
    nwy_test_cli_echo("\r\nTest in pwm start!\r\n");
}

void nwy_test_cli_pwm_stop()
{
    nwy_pwm_stop(test_p);
    nwy_test_cli_echo("\r\nTest in pwm stop!\r\n");
}

void nwy_test_cli_pwm_deinit()
{
    nwy_pwm_deinit(test_p);
    nwy_test_cli_echo("\r\nTest in pwm deinit!\r\n");
}
/**************************LCD*********************************/
#define NWY_OPEN_TEST_LCD
#ifdef NWY_OPEN_TEST_LCD

#define ROW 128
#define COL 128

#define WIDTH 128
#define HEIGHT 128

#define ASC12_FILE_NAME "/ASC12"
#define ASC16_FILE_NAME "/ASC16"
#define HZK12_FILE_NAME "/HZK12"
#define HZK16_FILE_NAME "/HZK16"

#define LCD_DataWrite_ST7735(Data)                   \
    {                                                \
        while (nwy_lcd_bus_write_data(Data) != true) \
            ;                                        \
    }
#define LCD_CtrlWrite_ST7735(Cmd)                  \
    {                                              \
        while (nwy_lcd_bus_write_cmd(Cmd) != true) \
            ;                                      \
    }

static unsigned short display_buf[WIDTH * HEIGHT];

void nwy_write_data_buf(void *buf, unsigned int len)
{
    int i;
    unsigned char *data = (unsigned char *)buf;

    for (i = 0; i < len; i++)
        LCD_DataWrite_ST7735(data[i]);

    OSI_LOGI(0, "nwy_write_data_buf!");
}

void nwy_lcd_block_write(unsigned char startx, unsigned char starty, unsigned char endx, unsigned char endy)
{
    unsigned char buf[4];
    LCD_CtrlWrite_ST7735(0x2a);
    buf[0] = 0;
    buf[1] = startx;
    buf[2] = 0;
    buf[3] = endx;
    nwy_write_data_buf((unsigned short *)buf, sizeof(buf));

    LCD_CtrlWrite_ST7735(0x2B);
    buf[0] = 0;
    buf[1] = starty;
    buf[2] = 0;
    buf[3] = endy;
    nwy_write_data_buf((unsigned short *)buf, sizeof(buf));

    LCD_CtrlWrite_ST7735(0x2C);

    OSI_LOGI(0, "nwy LCD block write");
}

static void GetFontSize(unsigned int fontSize, unsigned int *GBK_W,
                        unsigned int *GBK_H, unsigned int *ASC_W, unsigned int *ASC_H)
{
    if (fontSize == 12)
    {
        if (GBK_W != NULL)
            *GBK_W = 12;
        if (GBK_H != NULL)
            *GBK_H = 12;
        if (ASC_W != NULL)
            *ASC_W = 6;
        if (ASC_H != NULL)
            *ASC_H = 12;
    }
    else if (fontSize == 16)
    {
        if (GBK_W != NULL)
            *GBK_W = 16;
        if (GBK_H != NULL)
            *GBK_H = 16;
        if (ASC_W != NULL)
            *ASC_W = 8;
        if (ASC_H != NULL)
            *ASC_H = 16;
    }
}

static void GetGbkOneBuf(unsigned int fontSize, unsigned char *gbk, unsigned short *gbk_buf, unsigned short TextColor, unsigned short BackColor)
{
    int qh;
    int wh;
    int offset = 0;
    int i, j, k;
    int flag;
    int n = 0;

    unsigned int GBK_W = 0;
    unsigned int GBK_H = 0;
    unsigned int WORD_SIZE = 0;
    char *filename = NULL;

    GetFontSize(fontSize, &GBK_W, &GBK_H, NULL, NULL);

    unsigned char buf[100] = {0};
    unsigned char key[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    qh = (int)(gbk[0] - 0xa0);
    wh = (int)(gbk[1] - 0xa0);

    if (fontSize == 12)
    {
        filename = HZK12_FILE_NAME;
        WORD_SIZE = 24;
        offset = (int)(94 * (qh - 1) + (wh - 1)) * WORD_SIZE;
    }
    else if (fontSize == 16)
    {
        filename = HZK16_FILE_NAME;
        WORD_SIZE = 32;
        offset = (int)(94 * (qh - 1) + (wh - 1)) * WORD_SIZE;
    }
    else
    {
        OSI_LOGE(0, "unsupport font size:%d", fontSize);
        return;
    }

    int fp = nwy_sdk_fopen(filename, NWY_RDONLY);
    if (0 > fp)
    {
        OSI_LOGE(0, "%s open fail\n", filename);
        return;
    }

    nwy_sdk_fseek(fp, offset, NWY_SEEK_SET);
    int read_len = nwy_sdk_fread(fp, buf, WORD_SIZE);

    if (read_len != WORD_SIZE)
    {
        OSI_LOGE(0, "%s read fail read_len=%d\n", filename, read_len);
    }

    nwy_sdk_fclose(fp);

    int total_word_size = WORD_SIZE / GBK_H;
    for (k = 0; k < GBK_H; k++)
    {
        for (j = 0; j < total_word_size; j++)
        {
            for (i = 0; i < 8; i++)
            {
                if (j * 8 + i >= GBK_W)
                    break;
                flag = buf[k * total_word_size + j] & key[i];
                if (flag)
                {
                    gbk_buf[n++] = TextColor;
                }
                else
                {
                    gbk_buf[n++] = BackColor;
                }
            }
        }
    }

    //LCD_BlockWrite(0,0, COL-1,ROW-1);
}

static void GetOneCharBuf(unsigned int fontSize, unsigned char ord, unsigned short *char_buf, unsigned short TextColor, unsigned short BackColor) // ord:0~95
{
    unsigned char i, j, k;
    unsigned char dat;
    int n = 0;

    unsigned int ASC_W = 0;
    unsigned int ASC_H = 0;
    unsigned int WORD_SIZE = 0;
    char *filename = NULL;
    unsigned char buf[100] = {0};

    GetFontSize(fontSize, NULL, NULL, &ASC_W, &ASC_H);

    if (fontSize == 12)
    {
        filename = ASC12_FILE_NAME;
        WORD_SIZE = 12;
    }
    else if (fontSize == 16)
    {
        filename = ASC16_FILE_NAME;
        WORD_SIZE = 16;
    }
    else
    {
        OSI_LOGE(0, "unsupport font size:%d", fontSize);
        return;
    }

    int fp = nwy_sdk_fopen(filename, NWY_RDONLY);
    if (0 > fp)
    {
        OSI_LOGE(0, "%s open fail\n", filename);
        return;
    }

    nwy_sdk_fseek(fp, ord * WORD_SIZE, NWY_SEEK_SET);
    int read_len = nwy_sdk_fread(fp, buf, WORD_SIZE);
    if (read_len != WORD_SIZE)
    {
        OSI_LOGE(0, "ASC16 read fail read_len=%d\n", read_len);
    }

    nwy_sdk_fclose(fp);

    int total_word_size = WORD_SIZE / ASC_H;

    for (k = 0; k < ASC_H; k++)
    {
        for (j = 0; j < total_word_size; j++)
        {
            dat = buf[k * total_word_size + j];
            for (i = 0; i < 8; i++)
            {
                if (j * 8 + i >= ASC_W)
                    break;
                if ((dat << i) & 0x80)
                {
                    char_buf[n++] = TextColor;
                }
                else
                {
                    char_buf[n++] = BackColor;
                }
            }
        }
    }
}

void nwy_dispstrline(unsigned int fontSize, unsigned char *str, unsigned int Xstart, unsigned int Ystart, unsigned short TextColor, unsigned short BackColor)
{
    int i, j;
    static unsigned short tmp_buf[256];
    int line_len;
    int str_index = 0;
    unsigned int Xend = Xstart;
    unsigned int GBK_W = 0;
    unsigned int GBK_H = 0;
    unsigned int ASC_W = 0;
    unsigned int ASC_H = 0;

    GetFontSize(fontSize, &GBK_W, &GBK_H, &ASC_W, &ASC_H);
    line_len = strlen((char *)str) * ASC_W;

    if (line_len + Xstart > WIDTH)
    {
        OSI_LOGE(0, "ERROR:::: line_len is too lone %d\r\n", line_len);
        return;
    }

    while (!(*str == '\0'))
    {
        if (*str > 0x80)
        {
            GetGbkOneBuf(fontSize, str, tmp_buf, TextColor, BackColor);
            for (i = 0; i < GBK_H; i++)
            {
                for (j = 0; j < GBK_W; j++)
                {
                    display_buf[i * line_len + str_index + j] = tmp_buf[i * GBK_W + j];
                }
            }
            str_index += GBK_W;

            if (Xstart > ((COL)-GBK_W))
            {
                Xstart = (COL)-GBK_W;
            }
            else
            {
                Xend = Xend + GBK_W;
            }

            if (Ystart > ((ROW)-GBK_H))
            {
                break;
            }

            str += 2;
        }
        else
        {
            GetOneCharBuf(fontSize, *str++, tmp_buf, TextColor, BackColor);
            for (i = 0; i < ASC_H; i++)
            {
                for (j = 0; j < ASC_W; j++)
                {
                    display_buf[i * line_len + str_index + j] = tmp_buf[i * ASC_W + j];
                }
            }
            str_index += ASC_W;

            if (Xstart > ((COL)-ASC_W * 2))
            {
                Xstart = (COL)-ASC_W * 2;
            }
            else
            {
                Xend = Xend + ASC_H;
            }

            if (Ystart > ((ROW)-ASC_H))
            {
                break;
            }
        }
    }

    nwy_lcd_block_write(Xstart, Ystart, Xstart + line_len - 1, Ystart + (GBK_H - 1));
    nwy_write_data_buf(display_buf, line_len * GBK_H * 2);
}

static inline void prvFillBufferWhiteScreen(uint16_t *buffer, unsigned width, unsigned height)
{
    memset(buffer, 0xff, width * height * sizeof(uint16_t));
}

static void prvLcdClear(void)
{
    prvFillBufferWhiteScreen(display_buf, WIDTH, HEIGHT);
    nwy_lcd_block_write(0, 0, WIDTH - 1, HEIGHT - 1);
    nwy_write_data_buf(display_buf, WIDTH * HEIGHT * 2);
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
/**************************************************************************************/
static void _st7735Init(void)
{
    OSI_LOGI(0, "lcd:  st7735Init ");
    nwy_sleep(200);
    LCD_CtrlWrite_ST7735(0x11);
    nwy_sleep(200);

    LCD_CtrlWrite_ST7735(0xB1);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);

    LCD_CtrlWrite_ST7735(0xB2);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);

    LCD_CtrlWrite_ST7735(0xB3);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x05);
    LCD_DataWrite_ST7735(0x3C);
    LCD_DataWrite_ST7735(0x3C);

    LCD_CtrlWrite_ST7735(0xB4);
    LCD_DataWrite_ST7735(0x03);

    LCD_CtrlWrite_ST7735(0xC0);
    LCD_DataWrite_ST7735(0x62);
    LCD_DataWrite_ST7735(0x02);
    LCD_DataWrite_ST7735(0x04);

    LCD_CtrlWrite_ST7735(0xC1);
    LCD_DataWrite_ST7735(0xC0);

    LCD_CtrlWrite_ST7735(0xC2);
    LCD_DataWrite_ST7735(0x0D);
    LCD_DataWrite_ST7735(0x00);

    LCD_CtrlWrite_ST7735(0xC3);
    LCD_DataWrite_ST7735(0x8D);
    LCD_DataWrite_ST7735(0x6A);

    LCD_CtrlWrite_ST7735(0xC4);
    LCD_DataWrite_ST7735(0x8D);
    LCD_DataWrite_ST7735(0xEE);

    LCD_CtrlWrite_ST7735(0xC5);
    LCD_DataWrite_ST7735(0x12);
    //turn right 90
    LCD_CtrlWrite_ST7735(0x36);
    LCD_DataWrite_ST7735(0x60);

    LCD_CtrlWrite_ST7735(0xE0);
    LCD_DataWrite_ST7735(0x03);
    LCD_DataWrite_ST7735(0x1B);
    LCD_DataWrite_ST7735(0x12);
    LCD_DataWrite_ST7735(0x11);
    LCD_DataWrite_ST7735(0x3F);
    LCD_DataWrite_ST7735(0x3A);
    LCD_DataWrite_ST7735(0x32);
    LCD_DataWrite_ST7735(0x34);
    LCD_DataWrite_ST7735(0x2F);
    LCD_DataWrite_ST7735(0x2B);
    LCD_DataWrite_ST7735(0x30);
    LCD_DataWrite_ST7735(0x3A);
    LCD_DataWrite_ST7735(0x00);
    LCD_DataWrite_ST7735(0x01);
    LCD_DataWrite_ST7735(0x02);
    LCD_DataWrite_ST7735(0x05);

    LCD_CtrlWrite_ST7735(0xE1);
    LCD_DataWrite_ST7735(0x03);
    LCD_DataWrite_ST7735(0x1B);
    LCD_DataWrite_ST7735(0x12);
    LCD_DataWrite_ST7735(0x11);
    LCD_DataWrite_ST7735(0x32);
    LCD_DataWrite_ST7735(0x2F);
    LCD_DataWrite_ST7735(0x2A);
    LCD_DataWrite_ST7735(0x2F);
    LCD_DataWrite_ST7735(0x2E);
    LCD_DataWrite_ST7735(0x2C);
    LCD_DataWrite_ST7735(0x35);
    LCD_DataWrite_ST7735(0x3F);
    LCD_DataWrite_ST7735(0x00);
    LCD_DataWrite_ST7735(0x00);
    LCD_DataWrite_ST7735(0x01);
    LCD_DataWrite_ST7735(0x05);

    LCD_CtrlWrite_ST7735(0xFC);
    LCD_DataWrite_ST7735(0x8C);

    LCD_CtrlWrite_ST7735(0x3A);
    LCD_DataWrite_ST7735(0x05);

    LCD_CtrlWrite_ST7735(0x29);
}

#define LCD_BACK_LIGHT_POWER NWY_POWER_RGB_IB0
#define LCD_MAIN_POWER NWY_POWER_LCD
#define LCD_BUS_CLK_FREQ (20000000)

static bool lcd_init = false;
void nwy_lcd_init(void)
{
    if (lcd_init)
        return;

    nwy_subpower_switch(LCD_MAIN_POWER, true, true);
    nwy_subpower_switch(NWY_POWER_BACK_LIGHT, true, true);
    nwy_subpower_switch(LCD_BACK_LIGHT_POWER, true, true);
    nwy_sleep(256);
    nwy_lcd_bus_config_t lcd_bus_config =
        {
            .cs = NWY_LCD_BUS_CS_0,
            .cs0Polarity = false,
            .cs1Polarity = false,
            .resetb = true,
            .rsPolarity = false,
            .wrPolarity = false,
            .rdPolarity = false,
            .highByte = false,
            .clk = LCD_BUS_CLK_FREQ,
        };
    nwy_lcd_bus_init(&lcd_bus_config);
    nwy_sleep(32);
    _st7735Init();
    nwy_sleep(32);
    prvLcdClear();

    lcd_init = true;
}

void nwy_lcd_deinit(void)
{
    nwy_lcd_bus_deinit();
    nwy_subpower_switch(NWY_POWER_BACK_LIGHT, false, false);
    nwy_subpower_switch(LCD_BACK_LIGHT_POWER, false, false);
    nwy_subpower_switch(LCD_MAIN_POWER, false, false);

    lcd_init = false;
}
#endif // NWY_OPEN_TEST_LCD

#define BLACK 0x0000
#define NAVY 0x000F
#define DGREEN 0x03E0
#define DCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LGRAY 0xC618
#define DGRAY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

static unsigned short display_buffer[128 * 128];
void nwy_test_cli_lcd_open()
{
    nwy_lcd_init();
    nwy_test_cli_echo("\r\nlcd open success!\r\n");
}

void nwy_test_cli_lcd_draw_line()
{
    nwy_lcd_block_write(0, 96, 127, 96);
    for (int i = 0; i < 128; i++)
    {
        display_buffer[i] = OLIVE;
    }
    nwy_write_data_buf(display_buffer, 128 * 2);
    nwy_test_cli_echo("\r\nlcd draw line success!\r\n");
}

/* GB18030 chinese code */
static const char chinese_string[] = {
    0xd3, 0xd0, /* ÓÐ */
    0xb7, 0xbd, /* ·½ */
    0xba, 0xba, /* ºº */
    0xd7, 0xd6, /* ×Ö */
    0xd1, 0xdd, /* ÑÝ */
    0xca, 0xbe, /* Ê¾ */
    '\0'};

#define MK_COLOR(r, g, b) ((((r)&0x1f) << 11) + (((g)&0x3f) << 5) + (((b)&0x1f) << 0))
#define COLOR_WHITE MK_COLOR(0xff, 0xff, 0xff)
#define COLOR_BLACK MK_COLOR(0, 0, 0)

void nwy_test_cli_lcd_draw_chinese()
{
    nwy_lcd_block_write(16, 19, 16 + 16 * 6, 34);
    nwy_dispstrline(16, (unsigned char *)chinese_string, 16, 19, BLACK, MAGENTA);
    nwy_test_cli_echo("\r\nlcd draw chinese success!\r\n");
}

void nwy_test_cli_lcd_close()
{
    nwy_lcd_deinit();
    nwy_test_cli_echo("\r\nlcd close success!\r\n");
}

void nwy_test_cli_lcd_open_bl()
{
    nwy_subpower_switch(NWY_POWER_BACK_LIGHT, true, true);
    nwy_subpower_switch(LCD_BACK_LIGHT_POWER, true, true);
    nwy_test_cli_echo("\r\nlcd backlight open success!\r\n");
}

void nwy_test_cli_lcd_close_bl()
{
    nwy_subpower_switch(NWY_POWER_BACK_LIGHT, false, false);
    nwy_subpower_switch(LCD_BACK_LIGHT_POWER, false, false);
    nwy_test_cli_echo("\r\nlcd backlight close success!\r\n");
}

void nwy_test_cli_lcd_set_bl_level()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\n Please input light level[0~63]:");
    int level = atoi(sptr);
    nwy_set_back_light_level(NWY_POWER_RGB_IB0, level);
    nwy_set_back_light_level(NWY_POWER_RGB_IB1, level);
    nwy_set_back_light_level(NWY_POWER_RGB_IB2, level);
    nwy_test_cli_echo("\r\nlcd backlight set success!\r\n");
}

/**************************SD*********************************/
void nwy_test_cli_sd_get_st()
{
    nwy_test_cli_echo("\r\nsd state:%d\r\n", nwy_read_sdcart_status());
}

void nwy_test_cli_sd_mnt()
{
    nwy_test_cli_echo("\r\nsd mount:%d\r\n", nwy_sdk_sdcard_mount());
}

void nwy_test_cli_sd_unmnt()
{
    nwy_sdk_sdcard_unmount();
    nwy_test_cli_echo("\r\nsd unmount success\r\n");
}

/**************************FLASH*********************************/
void nwy_test_cli_flash_open()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_flash_erase()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_flash_write()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_flash_read()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

/**************************TTS*********************************/
char buff[1024] = {0};
nwy_tts_encode_t encode_type = 0;
char *hexbuf = "b8b8c4b8d4daa3acb2bbd4b6d3cea3acd3ceb1d8d3d0b7bd";

static void tts_play_callback(void *cb_para, nwy_neoway_result_t result)
{
    switch (result)
    {
    case PLAY_END:
        nwy_test_cli_echo("\r\n tts test down \r\n");
        break;
    default:
        break;
    }
}

void nwy_test_cli_tts_input()
{
    char *sptr;
    memset(buff, 0, 1024);
    sptr = nwy_test_cli_input_gets("\r\nPlease input encode mode(0-gbk,1-utf16le,2-utf16be,3-utf8): ");
    encode_type = atoi(sptr);
    sptr = nwy_test_cli_input_gets("\r\nPlease input content: ");
    strncpy(buff, sptr, strlen(sptr));
}

void nwy_test_cli_tts_play_start()
{
    if (strlen(buff) == 0)
        nwy_tts_playbuf(hexbuf, strlen(hexbuf), ENCODE_GBK, tts_play_callback, NULL);
    else
        nwy_tts_playbuf(buff, strlen(buff), encode_type, tts_play_callback, NULL);
}

void nwy_test_cli_tts_play_stop()
{
    nwy_tts_stop_play();
}

/**************************FOTA*********************************/
nwy_osiMessageQueue_t *nwy_download_msg_queue = NULL;
typedef struct
{
    uint32 len;
    void *data;
} nwy_download_queue_msg_t;

static void nwy_cli_recv_callback(unsigned char *data, uint32 length)
{
    nwy_download_queue_msg_t msg;
    if (!data || !length)
        return;
    msg.data = malloc(length);
    if (!msg.data)
        return;
    msg.len = length;
    memcpy(msg.data, data, msg.len);
    if (nwy_download_msg_queue)
        if (false == nwy_put_msg_que(nwy_download_msg_queue, &msg, 0xffffffff))
        {
            free(msg.data);
            NWY_CLI_LOG("put msg que failed drop the data:%d", length);
        }
}

typedef void (*nwy_test_cli_download_callback_t)(unsigned char *data, uint32 length, void *arg);

int nwy_test_cli_download_data(uint32 download_size, uint32 timeout, nwy_test_cli_download_callback_t fn, void *arg)
{
    uint32 size = 0;
    nwy_download_queue_msg_t msg;
    if (!download_size || !fn)
        return size;
    nwy_download_msg_queue = nwy_create_msg_Que(128, sizeof(nwy_download_queue_msg_t));
    if (!nwy_download_msg_queue)
        return size;
    nwy_test_cli_sio_enter_trans_mode((nwy_sio_trans_cb)nwy_cli_recv_callback);
    while (1)
    {
        memset(&msg, 0, sizeof(msg));
        if (nwy_get_msg_que(nwy_download_msg_queue, &msg, timeout))
        {
            if (msg.data && msg.len)
            {
                if ((size + msg.len) > download_size)
                    msg.len = download_size - size;
                fn(msg.data, msg.len, arg);
                free(msg.data);
                size += msg.len;
                if (size >= download_size)
                    break;
            }
        }
        else
            break;
    }
    nwy_test_cli_sio_quit_trans_mode();
    /* clear the rest fifo mem */
    while (1)
    {
        memset(&msg, 0, sizeof(msg));
        if (nwy_get_msg_que(nwy_download_msg_queue, &msg, 0xffffffff))
        {
            if (msg.data && msg.len)
            {
                free(msg.data);
            }
        }
        else
            break;
    }
    nwy_delete_msg_que(nwy_download_msg_queue);
    nwy_download_msg_queue = NULL;
    return size;
}

void nwy_test_cli_download_sdk_fota_pkt_cb(unsigned char *data, uint32 length, void *arg)
{
    ota_package_t *pkt = arg;
    pkt->data = data;
    pkt->len = length;
    if (!nwy_fota_download_core(pkt))
        pkt->offset += length;
    nwy_test_cli_echo("\r\ndownload pkt size:%d", pkt->offset);
}

void nwy_test_cli_fota_base_ver()
{
    char *sptr;
    int pkt_size;
    sptr = nwy_test_cli_input_gets("\r\n Please input firmware packet size:");
    pkt_size = atoi(sptr);
    if (pkt_size <= 0)
    {
        nwy_test_cli_echo("\r\n Fota Error : invalid packet size:%s", sptr);
        return;
    }
    nwy_test_cli_echo("\r\n Please input firmware:\r\n");
    ota_package_t ota_pkt;
    memset(&ota_pkt, 0, sizeof(ota_pkt));
    if (pkt_size <= nwy_test_cli_download_data(pkt_size, 8000, nwy_test_cli_download_sdk_fota_pkt_cb, &ota_pkt))
    {
        nwy_test_cli_echo("\r\n firmware download finish");
        nwy_test_cli_echo("\r\n system will reset for update");
        nwy_sleep(1000);
        nwy_version_core_update(true);
        nwy_test_cli_echo("\r\n firmware wrong");
    }
    nwy_test_cli_echo("\r\n what happened.");
}

void nwy_test_cli_download_app_fota_pkt_cb(unsigned char *data, uint32 length, void *arg)
{
    ota_package_t *pkt = arg;
    pkt->data = data;
    pkt->len = length;
    if (!nwy_fota_dm(pkt))
        pkt->offset += length;
    nwy_test_cli_echo("\r\ndownload pkt size:%d", pkt->offset);
}

void nwy_test_cli_fota_app_ver()
{
    char *sptr;
    int ret, pkt_size;
    sptr = nwy_test_cli_input_gets("\r\n Please input firmware packet size:");
    pkt_size = atoi(sptr);
    if (pkt_size <= 0)
    {
        nwy_test_cli_echo("\r\n Fota Error : invalid packet size:%s", sptr);
        return;
    }
    nwy_test_cli_echo("\r\n Please input firmware:\r\n");
    ota_package_t ota_pkt;
    memset(&ota_pkt, 0, sizeof(ota_pkt));
    if (pkt_size <= nwy_test_cli_download_data(pkt_size, 8000, nwy_test_cli_download_app_fota_pkt_cb, &ota_pkt))
    {
        nwy_test_cli_echo("\r\n firmware download finish");
        ret = nwy_package_checksum();
        if (ret < 0)
        {
            nwy_test_cli_echo("\r\nchecksum failed");
            return;
        }
        nwy_test_cli_echo("\r\n system will reset for update");
        nwy_sleep(1000);
        ret = nwy_fota_ua();
        if (ret < 0)
        {
            nwy_test_cli_echo("\r\nupdate failed");
            return;
        }
    }
    nwy_test_cli_echo("\r\n what happened.");
}

/**************************AUDIO*********************************/
static int capture_len = 0;
static uint8_t *audio_buf;
#define AUDIO_RECORDER_DATA_LEN (1024 * 48)
char nwy_ext_sio_recv_buff[NWY_EXT_SIO_RX_MAX + 1] = {0};

static int nwy_player_cb(nwy_player_status state)
{
    nwy_test_cli_echo("nwytest_player_cb state=%d", state);
    return 1;
}

void nwy_test_cli_audio_play()
{
    nwy_test_cli_echo("\r\naudio player is running\r\n");
    int read_index = 0;

    nwy_audio_player_open(nwy_player_cb);
    while (read_index < capture_len)
    {
        nwy_audio_player_play(&audio_buf[read_index], 320);
        read_index += 320;
    }
    nwy_audio_player_stop();
    nwy_audio_player_close();
    if (audio_buf != NULL)
    {
        capture_len = 0;
        free(audio_buf);
        audio_buf = NULL;
    }
    nwy_test_cli_echo("\r\naudio play done\r\n");
}

static int capture_callback(unsigned char *pdata, unsigned int len)
{
    if ((capture_len + len) < (AUDIO_RECORDER_DATA_LEN))
    {
        memcpy(audio_buf + capture_len, pdata, len);
        capture_len += len;
    }
    else
    {
        nwy_test_cli_echo("recorder data is overflow\r\n");
    }
    return 1;
}

void nwy_test_cli_audio_rec()
{
    capture_len = 0;
    audio_buf = (uint8_t *)malloc(AUDIO_RECORDER_DATA_LEN);
    if (NULL == audio_buf)
    {
        nwy_test_cli_echo("audio malloc failed\r\n");
        return;
    }
    memset(audio_buf, 0, AUDIO_RECORDER_DATA_LEN);

    nwy_test_cli_echo("\r\naudio recorder is running\r\n");
    nwy_audio_recorder_open(capture_callback);
    nwy_audio_recorder_start();
    nwy_sleep(500 * 6);
    nwy_audio_recorder_stop();
    nwy_audio_recorder_close();
}

void nwy_test_cli_audio_dtmf()
{
    char *tone = nwy_ext_sio_recv_buff;
    nwy_test_cli_input_gets("\r\nPlease input DTMF: ");
    nwy_audio_tone_play(tone, 200, 15);
}

/**************************FS*********************************/
#define NWY_FILE_NAME_MAX 64
static int nwy_test_fs_fd = -1;
static char nwy_test_file_name[NWY_FILE_NAME_MAX + 1] = {0};
void nwy_test_cli_fs_open(void)
{
    char *sptr;
    memset(nwy_test_file_name, 0, sizeof(nwy_test_file_name));
    sptr = nwy_test_cli_input_gets("\r\nPlease input filename(len <= %d): ", NWY_FILE_NAME_MAX);
    if (strlen(sptr) > NWY_FILE_NAME_MAX)
    {
        nwy_test_cli_echo("\r\nfile name can't beyond %d", NWY_FILE_NAME_MAX);
        return;
    }
    strcpy(nwy_test_file_name, sptr);
    nwy_test_fs_fd = nwy_sdk_fopen(nwy_test_file_name, NWY_CREAT | NWY_RDWR | NWY_TRUNC);
    if (nwy_test_fs_fd < 0)
        nwy_test_cli_echo("\r\nfile %s open error:%d\r\n", nwy_test_file_name, nwy_test_fs_fd);
    else
        nwy_test_cli_echo("\r\nfile %s open success:%d\r\n", nwy_test_file_name, nwy_test_fs_fd);
}

void nwy_test_cli_fs_write(void)
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input file write data(len <= 2000): ");
    int len = strlen(sptr);
    if (len > 2000)
    {
        nwy_test_cli_echo("\r\nfile write data can't beyond 2000");
        return;
    }
    int rtn = nwy_sdk_fwrite(nwy_test_fs_fd, sptr, len);
    if (rtn != len)
        nwy_test_cli_echo("\r\nfile %s write error:%d\r\n", nwy_test_file_name, rtn);
    else
        nwy_test_cli_echo("\r\nfile %s write success:%d\r\n", nwy_test_file_name, rtn);
}

void nwy_test_cli_fs_read()
{
    int size, rtn;
    char buffer[128];
    nwy_test_cli_echo("\r\n");
    size = rtn = 0;
    //nwy_sdk_fseek(nwy_test_fs_fd, 0, NWY_SEEK_SET);
    while (1)
    {
        rtn = nwy_sdk_fread(nwy_test_fs_fd, buffer, sizeof(buffer));
        NWY_CLI_LOG("read data size:%d", rtn);
        if (rtn > 0)
            nwy_test_cli_output(buffer, rtn);
        else
            break;
        size += rtn;
    }
    if (size)
        nwy_test_cli_echo("\r\nfile %s read success:%d\r\n", nwy_test_file_name, size);
}

void nwy_test_cli_fs_fsize()
{
    nwy_test_cli_echo("\r\nfile %s size:%d\r\n", nwy_test_file_name, nwy_sdk_fsize_fd(nwy_test_fs_fd));
}

void nwy_test_cli_fs_seek()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input file seek offset: ");
    int offset = atoi(sptr);
    int rtn = nwy_sdk_fseek(nwy_test_fs_fd, offset, NWY_SEEK_SET);
    if (rtn != offset)
        nwy_test_cli_echo("\r\nfile %s seek error:%d\r\n", nwy_test_file_name, rtn);
    else
        nwy_test_cli_echo("\r\nfile %s seek success:%d\r\n", nwy_test_file_name, rtn);
}

void nwy_test_cli_fs_sync()
{
    nwy_test_cli_echo("\r\nfile %s sync:%d\r\n", nwy_test_file_name, nwy_sdk_fsync(nwy_test_fs_fd));
}

void nwy_test_cli_fs_fstate()
{
    struct stat st;
    int rtn = nwy_sdk_get_stat_fd(nwy_test_fs_fd, &st);
    nwy_test_cli_echo("\r\nfile %s stat:%d st_size:%d\r\n", nwy_test_file_name, rtn, st.st_size);
}

void nwy_test_cli_fs_trunc()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input file trunc size: ");
    int size = atoi(sptr);
    int rtn = nwy_sdk_ftrunc_fd(nwy_test_fs_fd, size);
    if (rtn != size)
        nwy_test_cli_echo("\r\nfile %s trunc error:%d\r\n", nwy_test_file_name, rtn);
    else
        nwy_test_cli_echo("\r\nfile %s trunc success:%d\r\n", nwy_test_file_name, rtn);
}

void nwy_test_cli_fs_close()
{
    nwy_test_cli_echo("\r\nfile %s close:%d\r\n", nwy_test_file_name, nwy_sdk_fclose(nwy_test_fs_fd));
    nwy_test_fs_fd = -1;
}

void nwy_test_cli_fs_remove()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input file name: ");
    int rtn = nwy_sdk_file_unlink(sptr);
    if (rtn != 0)
        nwy_test_cli_echo("\r\nfile %s remove error:%d\r\n", sptr, rtn);
    else
        nwy_test_cli_echo("\r\nfile %s remove success:%d\r\n", sptr, rtn);
}

void nwy_test_cli_fs_rename()
{
    char *sptr;
    char old[64], new[64];
    memset(old, 0, sizeof(old));
    memset(new, 0, sizeof(new));
    sptr = nwy_test_cli_input_gets("\r\nPlease input file old name: ");
    strncpy(old, sptr, sizeof(old));
    sptr = nwy_test_cli_input_gets("\r\nPlease input file new name: ");
    strncpy(new, sptr, sizeof(new));
    int rtn = nwy_sdk_frename(old, new);
    if (rtn != 0)
        nwy_test_cli_echo("\r\nfile %s rename error:%d\r\n", old, rtn);
    else
        nwy_test_cli_echo("\r\nfile %s rename success:%d\r\n", new, rtn);
}

#define NWY_DIR_NAME_MAX 64
static nwy_dir *nwy_test_fs_dir = NULL;
static char nwy_test_dir_name[NWY_DIR_NAME_MAX + 1] = {0};
void nwy_test_cli_dir_open()
{
    char *sptr;
    memset(nwy_test_dir_name, 0, sizeof(nwy_test_dir_name));
    sptr = nwy_test_cli_input_gets("\r\nPlease input dir name(len <= %d): ", NWY_DIR_NAME_MAX);
    if (strlen(sptr) > NWY_DIR_NAME_MAX)
    {
        nwy_test_cli_echo("\r\ndir name can't beyond %d", NWY_DIR_NAME_MAX);
        return;
    }
    strcpy(nwy_test_dir_name, sptr);
    nwy_test_fs_dir = nwy_sdk_vfs_opendir(nwy_test_dir_name);
    if (nwy_test_fs_dir == NULL)
        nwy_test_cli_echo("\r\ndir %s open error:%d\r\n", nwy_test_dir_name, nwy_test_fs_dir);
    else
        nwy_test_cli_echo("\r\ndir %s open success:%d\r\n", nwy_test_dir_name, nwy_test_fs_dir);
}

void nwy_test_cli_dir_read()
{
    char rsp[192 + 84];
    nwy_dirent *ent;
    struct stat st;
    size_t name_len = strlen(nwy_test_dir_name);
    bool trail_slash = (name_len > 0 && nwy_test_dir_name[name_len - 1] == '/');
    //nwy_sdk_vfs_seekdir(nwy_test_fs_dir, 0);
    while ((ent = nwy_sdk_vfs_readdir(nwy_test_fs_dir)) != NULL)
    {
        if (ent->d_type == NWY_DT_REG)
        {
            // borrow rsp for full_path
            sprintf(rsp, "%s/%s", nwy_test_dir_name, ent->d_name);
            nwy_sleep(50);
            if (nwy_sdk_get_stat_path(rsp, &st) != 0)
                continue;

            if (trail_slash)
                sprintf(rsp, "dir read \"%s%s\",%ld", nwy_test_dir_name, ent->d_name, st.st_size);
            else
                sprintf(rsp, "dir read \"%s/%s\",%ld", nwy_test_dir_name, ent->d_name, st.st_size);
            nwy_test_cli_echo("\r\n%s", rsp);
        }
        else if (ent->d_type == NWY_DT_DIR)
        {
            if (trail_slash)
                sprintf(rsp, "dir read \"%s%s\"", nwy_test_dir_name, ent->d_name);
            else
                sprintf(rsp, "dir read \"%s/%s\"", nwy_test_dir_name, ent->d_name);
            nwy_test_cli_echo("\r\n%s", rsp);
        }
    }
    nwy_test_cli_echo("\r\n");
}

void nwy_test_cli_dir_tell()
{
    nwy_test_cli_echo("\r\ndir %s tell:%d\r\n", nwy_test_dir_name, nwy_sdk_vfs_telldir(nwy_test_fs_dir));
}

void nwy_test_cli_dir_seek()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input dir seek offset: ");
    int offset = atoi(sptr);
    nwy_sdk_vfs_seekdir(nwy_test_fs_dir, offset);
    nwy_test_cli_echo("\r\ndir %s seek success:%d\r\n", nwy_test_dir_name, offset);
}

void nwy_test_cli_dir_rewind()
{
    nwy_sdk_vfs_rewinddir(nwy_test_fs_dir);
    nwy_test_cli_echo("\r\ndir %s rewind success\r\n", nwy_test_dir_name);
}

void nwy_test_cli_dir_close()
{
    nwy_test_cli_echo("\r\ndir %s close:%d\r\n", nwy_test_dir_name, nwy_sdk_vfs_closedir(nwy_test_fs_dir));
    nwy_test_fs_dir = NULL;
}

void nwy_test_cli_dir_mk()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input dir name(len <= %d): ", NWY_DIR_NAME_MAX);
    if (strlen(sptr) > NWY_DIR_NAME_MAX)
    {
        nwy_test_cli_echo("\r\ndir name can't beyond %d", NWY_DIR_NAME_MAX);
        return;
    }
    nwy_test_cli_echo("\r\ndir %s mk:%d\r\n", sptr, nwy_sdk_vfs_mkdir(sptr));
}

void nwy_test_cli_dir_remove()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input dir name(len <= %d): ", NWY_DIR_NAME_MAX);
    if (strlen(sptr) > NWY_DIR_NAME_MAX)
    {
        nwy_test_cli_echo("\r\ndir name can't beyond %d", NWY_DIR_NAME_MAX);
        return;
    }
    //nwy_sdk_vfs_rmdir(sptr);
    nwy_test_cli_echo("\r\ndir %s remove:%d\r\n", sptr, nwy_sdk_vfs_rmdir_recursive(sptr));
}

void nwy_test_cli_fs_free_size()
{
    nwy_test_cli_echo("\r\nfs free size:%d\r\n", nwy_sdk_vfs_ls());
}

void nwy_test_cli_safe_fs_init()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input filename(len <= %d): ", NWY_FILE_NAME_MAX);
    if (strlen(sptr) > NWY_FILE_NAME_MAX)
    {
        nwy_test_cli_echo("\r\nfile name can't beyond %d", NWY_FILE_NAME_MAX);
        return;
    }
    nwy_test_cli_echo("\r\nsfile %s init:%d\r\n", sptr, nwy_sdk_sfile_init(sptr));
}

void nwy_test_cli_safe_fs_read()
{
    char *sptr;
    int rtn, size;
    char fn[NWY_FILE_NAME_MAX];
    sptr = nwy_test_cli_input_gets("\r\nPlease input filename(len <= %d): ", NWY_FILE_NAME_MAX);
    if (strlen(sptr) > NWY_FILE_NAME_MAX)
    {
        nwy_test_cli_echo("\r\nfile name can't beyond %d", NWY_FILE_NAME_MAX);
        return;
    }
    memset(fn, 0, sizeof(fn));
    strncpy(fn, sptr, NWY_FILE_NAME_MAX);
    size = nwy_sdk_sfile_size(fn);
    if (size <= 0)
    {
        nwy_test_cli_echo("\r\nsfile %s read error:%d\r\n", fn, size);
        return;
    }
    char *buffer = malloc(size);
    if (buffer == NULL)
        return;
    rtn = nwy_sdk_sfile_read(fn, buffer, size);
    if (rtn > 0)
    {
        nwy_test_cli_echo("\r\n");
        nwy_test_cli_output(buffer, rtn);
        nwy_test_cli_echo("\r\nfile %s read success:%d\r\n", fn, rtn);
    }
    else
        nwy_test_cli_echo("\r\nfile %s read error:%d\r\n", fn, rtn);
    free(buffer);
}

void nwy_test_cli_safe_fs_write()
{
    char *sptr;
    int rtn, len;
    char fn[NWY_FILE_NAME_MAX];
    sptr = nwy_test_cli_input_gets("\r\nPlease input filename(len <= %d): ", NWY_FILE_NAME_MAX);
    if (strlen(sptr) > NWY_FILE_NAME_MAX)
    {
        nwy_test_cli_echo("\r\nfile name can't beyond %d", NWY_FILE_NAME_MAX);
        return;
    }
    memset(fn, 0, sizeof(fn));
    strncpy(fn, sptr, NWY_FILE_NAME_MAX);
    sptr = nwy_test_cli_input_gets("\r\nPlease input sfile write data(len <= 2000): ");
    len = strlen(sptr);
    if (len > 2000)
    {
        nwy_test_cli_echo("\r\nsfile write data can't beyond 2000");
        return;
    }
    NWY_CLI_LOG("write data[%d] to sfile %s: %s", len, fn, sptr);
    rtn = nwy_sdk_sfile_write(fn, sptr, len);
    if (rtn != len)
        nwy_test_cli_echo("\r\nsfile %s write error:%d\r\n", fn, rtn);
    else
        nwy_test_cli_echo("\r\nsfile %s write success:%d\r\n", fn, rtn);
}

void nwy_test_cli_safe_fs_fszie()
{
    char *sptr;
    sptr = nwy_test_cli_input_gets("\r\nPlease input filename(len <= %d): ", NWY_FILE_NAME_MAX);
    if (strlen(sptr) > NWY_FILE_NAME_MAX)
    {
        nwy_test_cli_echo("\r\nfile name can't beyond %d", NWY_FILE_NAME_MAX);
        return;
    }
    nwy_test_cli_echo("\r\nsfile %s size:%d\r\n", sptr, nwy_sdk_sfile_size(sptr));
}
