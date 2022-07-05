/******************** (C) COPYRIGHT 2018 MiraMEMS *****************************
* File Name     : mir3da.c
* Author        : ycwang@miramems.com
* Version       : V1.0
* Date          : 05/18/2018
* Description   : Demo for configuring mir3da
*******************************************************************************/
#include "app_mir3da.h"
#include <stdio.h>
#include "nwy_osi_api.h"
#include "nwy_i2c.h"
#include "app_sys.h"

u8_m i2c_DEV_ADDR = (0x26 << 1);

s8_m mir3da_register_read(u8_m addr, u8_m *data_m, u8_m len)
{
    uint8_t read = 0;
    int bus;
    int ret;
    bus = nwy_i2c_init(NAME_I2C_BUS_2, NWY_I2C_BPS_100K);
    if (NWY_SUCESS > bus)
    {
        LogPrintf(DEBUG_ALL, "I2c Error : bus init fail");
        return 0;
    }
    ret = nwy_i2c_raw_put_byte(bus, i2c_DEV_ADDR & 0xFE, 1, 0);
    if (ret != 0)
    {
        LogPrintf(DEBUG_ALL, "no Write Respon");
    }
    ret = nwy_i2c_raw_put_byte(bus, addr, 0, 1);
    if (ret != 0)
    {
        LogPrintf(DEBUG_ALL, "Write Reg addr fail");
    }
    ret = nwy_i2c_raw_put_byte(bus, i2c_DEV_ADDR | 0x01, 1, 0);
    if (ret != 0)
    {
        LogPrintf(DEBUG_ALL, "no Read Respon");
    }
    read = 0;
    ret = nwy_i2c_raw_get_byte(bus, &read, 0, 1);
    if (ret != 0)
    {
        LogPrintf(DEBUG_ALL, "Read Reg fail");
    }
    *data_m = read;
    nwy_i2c_deinit(bus);
    return 0;

}

s8_m mir3da_register_write(u8_m addr, u8_m data_m)
{
    int bus;
    int ret;
    bus = nwy_i2c_init(NAME_I2C_BUS_2, NWY_I2C_BPS_100K);
    if (NWY_SUCESS > bus)
    {
        LogPrintf(DEBUG_ALL, "I2c Error : bus init fail");
        return 0;
    }
    ret = nwy_i2c_raw_put_byte(bus, i2c_DEV_ADDR & 0xFE, 1, 0);
    if (ret != 0)
    {
        LogPrintf(DEBUG_ALL, "no Write Respon");
    }
    ret = nwy_i2c_raw_put_byte(bus, addr, 0, 0);
    if (ret != 0)
    {
        LogPrintf(DEBUG_ALL, "Write Reg addr fail 0x%02X", addr);
    }
    ret = nwy_i2c_raw_put_byte(bus, data_m, 0, 1);
    if (ret != 0)
    {
        LogPrintf(DEBUG_ALL, "Write data fail 0x%02X", data_m);
    }
    nwy_i2c_deinit(bus);
    return 0;

}

//Initialization
s8_m mir3da_init(void)
{
    s8_m res = 0;
    u8_m data_m = 0;
    //Retry 3 times
    res = mir3da_register_read(NSA_REG_WHO_AM_I, &data_m, 1);
    if (data_m != 0x13)
    {
        res = mir3da_register_read(NSA_REG_WHO_AM_I, &data_m, 1);
        if (data_m != 0x13)
        {
            res = mir3da_register_read(NSA_REG_WHO_AM_I, &data_m, 1);
            if (data_m != 0x13)
            {
                LogPrintf(DEBUG_ALL, "mir3da_init==>Read chip error=%X", data_m);
                return -1;
            }
        }
    }

    LogPrintf(DEBUG_ALL, "mir3da_init==>Read chip id=%X", data_m);

    res |= mir3da_register_write(NSA_REG_SPI_I2C, 0x24);
    nwy_sleep(20);

    res |= mir3da_register_write(NSA_REG_G_RANGE, 0x00);               //+/-2G,14bit
    res |= mir3da_register_write(NSA_REG_POWERMODE_BW, 0x34);          //normal mode
    res |= mir3da_register_write(NSA_REG_ODR_AXIS_DISABLE, 0x06);      //ODR = 62.5hz

    //Engineering mode
    res |= mir3da_register_write(NSA_REG_ENGINEERING_MODE, 0x83);
    res |= mir3da_register_write(NSA_REG_ENGINEERING_MODE, 0x69);
    res |= mir3da_register_write(NSA_REG_ENGINEERING_MODE, 0xBD);

    nwy_sleep(50);

    //Reduce power consumption
    if (i2c_DEV_ADDR == 0x26)
    {
        mir3da_register_write(NSA_REG_SENS_COMP, 0x00);
    }

    return res;
}

//enable/disable the chip
s8_m mir3da_set_enable(u8_m enable)
{
    s8_m res = 0;
    if (enable)
        res = mir3da_register_write(NSA_REG_POWERMODE_BW, 0x30);
    else
        res = mir3da_register_write(NSA_REG_POWERMODE_BW, 0x80);

    return res;
}

//Read three axis data, 1024 LSB = 1 g
s8_m mir3da_read_data(s16_m *x, s16_m *y, s16_m *z)
{
    u8_m    tmp_data[6] = {0};

    if (mir3da_register_read(NSA_REG_ACC_X_LSB, tmp_data, 6) != 0)
    {
        return -1;
    }

    *x = ((s16_m)(tmp_data[1] << 8 | tmp_data[0])) >> 4;
    *y = ((s16_m)(tmp_data[3] << 8 | tmp_data[2])) >> 4;
    *z = ((s16_m)(tmp_data[5] << 8 | tmp_data[4])) >> 4;

    return 0;
}

//open active interrupt
s8_m mir3da_open_interrupt(u8_m th)
{
    s8_m   res = 0;

    res = mir3da_register_write(NSA_REG_INTERRUPT_SETTINGS1, 0x87);
    res = mir3da_register_write(NSA_REG_ACTIVE_DURATION, 0x00);
    res = mir3da_register_write(NSA_REG_ACTIVE_THRESHOLD, th);
    res = mir3da_register_write(NSA_REG_INTERRUPT_MAPPING1, 0x04);

    return res;
}

//close active interrupt
s8_m mir3da_close_interrupt(void)
{
    s8_m   res = 0;

    res = mir3da_register_write(NSA_REG_INTERRUPT_SETTINGS1, 0x00);
    res = mir3da_register_write(NSA_REG_INTERRUPT_MAPPING1, 0x00);

    return res;
}

s8_m read_gsensor_id(void)
{
    s8_m res = 0;
    u8_m data_m = 0;
    //Retry 3 times
    res = mir3da_register_read(NSA_REG_WHO_AM_I, &data_m, 1);
    if (data_m != 0x13)
    {
        res = mir3da_register_read(NSA_REG_WHO_AM_I, &data_m, 1);
        if (data_m != 0x13)
        {
            res = mir3da_register_read(NSA_REG_WHO_AM_I, &data_m, 1);
            if (data_m != 0x13)
            {
                LogPrintf(DEBUG_FACTORY, "Read gsensor chip id error =%x", data_m);
                return -1;
            }
        }
    }
    LogPrintf(DEBUG_FACTORY, "GSENSOR Chk. ID=0x%X\r\nGSENSOR CHK OK", data_m);
    return res;
}

s8_m readInterruptConfig(void)
{
    uint8_t data_m = 0;
    mir3da_register_read(NSA_REG_INTERRUPT_SETTINGS1, &data_m, 1);
    if (data_m != 0x87)
    {
        mir3da_register_read(NSA_REG_INTERRUPT_SETTINGS1, &data_m, 1);
        if (data_m != 0x87)
        {
            LogPrintf(DEBUG_ALL, "Gsensor fail %x", data_m);
            return -1;
        }
    }
    LogPrintf(DEBUG_ALL, "Gsensor OK");
    return 0;
}



