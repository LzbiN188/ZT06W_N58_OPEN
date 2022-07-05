
#ifndef __NWY_SPI_H__
#define __NWY_SPI_H__

#include "drv_spi.h"


#ifdef __cplusplus
extern "C" {
#endif

#define SPI_EC_SUCESS 0
#define SPI_EC_ERROR  -1

#define SPI_MODE_CPOL     0x01
#define SPI_MODE_CPHA     0x02

#define SPI_MODE_0        (0x00          |          0x00)
#define SPI_MODE_1        (SPI_MODE_CPHA |          0x00)
#define SPI_MODE_2        (0x00          | SPI_MODE_CPOL)
#define SPI_MODE_3        (SPI_MODE_CPHA | SPI_MODE_CPOL)

#define SPI_CS_0  0x00
#define SPI_CS_1  0x01

#define NAME_SPI_BUS_1  "SPI1"
#define NAME_SPI_BUS_2  "SPI2"

/*---------------------------Function Definition--------------------------*/
/**
 * @brief init the spi bus
 *
 * @param spibus : spi bus name
 * @param mode : spi bus clock line mode
 * @param speed : spi bus speed
 * @param bits : spi bus data frame bits
 * @return
 *      - SPI_EC_ERROR    fail
 *      - otherwise the spi bus handle
 */
int nwy_spi_init(char *spibus, uint8_t mode, uint32_t speed, uint8_t bits);

/**
 * @brief Transmits and receive  datas via the SPI bus.
 *
 * @param hd : spi bus handle
 * @param cs : cs choice of spi device
 * @param tx : point to senddata address
 * @param rx : point to readaddr address
 * @param size : data len to be transfer.
 * @return
 *      - SPI_EC_ERROR      fail
 *      - SPI_EC_SUCESS     success
 */
int nwy_spi_transfer(int hd, uint8_t cs, uint8_t *tx, uint8_t *rx, uint32_t size);
/*---------------------------Function Definition--------------------------*/
/**
 * @brief deinit the spi bus
 *
 * @param hd : spi bus handle
 * @return
 *      - SPI_EC_ERROR      fail
 *      - SPI_EC_SUCESS     success
 */
int nwy_spi_deinit(int hd);

struct spi_gpio_platform_data {
    uint32_t    sck;
    uint32_t    mosi;
    uint32_t    miso;
    uint32_t    chipselect;
};
/**
 * @brief init the gpio spi bus
 *
 * @param gpio : gpio for spi
 * @param mode : spi bus clock line mode
 * @param speed : spi bus speed
 * @param bits : spi bus data frame bits
 * @return
 *      - SPI_EC_ERROR    fail
 *      - otherwise the spi bus handle
 */
int nwy_drv_gpio_spi_init(struct spi_gpio_platform_data *gpio, uint8_t mode, uint32_t speed, uint8_t bits);
/**
 * @brief Transmits and receive  datas via the GPIO SPI bus.
 *
 * @param hd : spi bus handle
 * @param tx : point to senddata address
 * @param rx : point to readaddr address
 * @param size : data len to be transfer
 * @return
 *      - SPI_EC_ERROR      fail
 *      - SPI_EC_SUCESS     success
 */
int nwy_drv_gpio_spi_transfer(int hd, void *tx, void *rx, uint32_t size);

/**
 * @brief deinit the gpio spi bus
 *
 * @param hd : spi bus handle
 * @return
 *      - SPI_EC_ERROR      fail
 *      - SPI_EC_SUCESS     success
 */
 int nwy_drv_gpio_spi_deinit(int hd);

/**
 * @brief read the flash id
 *
 * @param hd : spi bus handle
 * @param jedec_id : jedec device id cmd
 * @return
 *      - NULl
 */
int nwy_read_flash_id(int hd,int jedec_id);

/**
 * @brief erase spi flash sector
 *
 * @param hd : spi bus handle
 * @param write_enable : write enable cmd
 * @param read_status : read flash reg cmd
 * @param sector_era : sector erase cmd
 * @param sectoraddr : sector address
 * @return
 *      - NULl
 */
void nwy_spi_flash_sector_erase(int hd, int write_enable,int read_status,int sector_era, uint32_t sectoraddr);

/**
 * @brief write spi flash 
 *
 * @param hd : spi bus handle
 * @param write_enable : write enable cmd
 * @param read_status : read flash reg cmd
 * @param pageprogram : pageprogram cmd
 * @param pbuff : point to senddata
 * @param writeaddr :write address
 * @param size : data len to be transmitted.
 * @return
 *      - NULl
 */
void nwy_spi_flash_write(int hd, int write_enable,int read_status,int pageprogram,uint8_t *pbuff, uint32_t writeaddr, uint16_t size);

/**
 * @brief write spi flash 
 *
 * @param hd : spi bus handle
 * @param read_data : read data cmd
 * @param read_status : read flash reg cmd
 * @param pageprogram : pageprogram cmd
 * @param pbuff : point to senddata
 * @param ReadAddr :read address
 * @param size : data len to be received
 * @return
 *      - NULl
 */
void nwy_spi_flash_read(int hd, int read_data, uint8_t *pbuff, uint32_t ReadAddr, uint16_t size);

typedef struct _nwy_spi_flash_t
{
    uint32_t name;
    uint32_t block_size;
    uint32_t block_count;
    void    *priv;

    int (*read)(struct _nwy_spi_flash_t *dev, uint32_t offset, uint32_t size, void *buf);
    int (*write)(struct _nwy_spi_flash_t *dev, uint32_t offset, uint32_t size, const void *data);
    int (*erase)(struct _nwy_spi_flash_t *dev, uint32_t offset, uint32_t size);
} nwy_spi_flash_t;

typedef void nwy_block_device_t;

/**
 * @brief create block device for spi flash.
 *
 * @param dev : spi flash
 * @return
 *      - NULL         fail
 *      - block device success
 */
nwy_block_device_t *nwy_vfs_block_device_create(nwy_spi_flash_t *dev);
/**
 * @brief create logical block device for spi flash bigger than 16M bytes.
 * this api creates block devices that are not compatible with nwy_vfs_block_device_create
 * @param spibus : spi bus name
 * @param offset : physical start addr
 * @param size   : logical device size 
 * @return
 *      - NULL         fail
 *      - block device success
 */
nwy_block_device_t *nwy_vfs_logical_block_device_create(char *spibus, uint32_t offset, uint32_t size);
/**
 * @brief mount spi flash to file system.
 *
 * @param base_path : mount point
 * @param nwy_bdev  : flash block device
 * @return
 *      - SPI_EC_SUCESS success
 *      - otherwise     fail
 */
int nwy_vfs_mount(const char *base_path, nwy_block_device_t *nwy_bdev);
/**
 * @brief format spi flash.
 *
 * @param nwy_bdev  : flash block device
 * @return
 *      - SPI_EC_SUCESS success
 *      - otherwise     fail
 */
int nwy_vfs_mkfs(nwy_block_device_t *nwy_bdev);


drvSpiMaster_t * nwy_get_spi_handle(int hd);


#ifdef __cplusplus
   }
#endif

#endif // __NWY_SPI_H__

