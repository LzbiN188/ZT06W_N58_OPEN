
#ifndef __NWY_I2C_H__
#define __NWY_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_I2C_BUS_1  "I2C1"
#define NAME_I2C_BUS_2  "I2C2"
#define NAME_I2C_BUS_3  "I2C3"

typedef enum
{
    NWY_I2C_BPS_100K, ///< normal 100Kbps
    NWY_I2C_BPS_400K, ///< fast 400Kbps
    NWY_I2C_BPS_3P5M, ///< high speed 3.5Mbps
} nwy_i2c_bps_t;

/*---------------------------Function Definition--------------------------*/
int nwy_i2c_init(char * busName, nwy_i2c_bps_t busBps);
int nwy_i2c_read(int fd, unsigned char slaveAddr, unsigned short ofstAddr,
                 unsigned char* ptrBuff, unsigned short length);
int nwy_i2c_write(int fd, unsigned char slaveAddr, unsigned short ofstAddr,
                  unsigned char* ptrData, unsigned short length);
int nwy_i2c_raw_get_byte(int fd, uint8_t *data, int start_flag, int stop_flag);
int nwy_i2c_raw_put_byte(int fd, uint8_t data, int start_flag, int stop_flag);
int nwy_i2c_deinit(int fd);

#ifdef __cplusplus
   }
#endif

#endif 


