#ifndef __NWY_LCD_BUS_H__
#define __NWY_LCD_BUS_H__

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// nwy_lcd_bus_cs_t
// -----------------------------------------------------------------------------
/// Defines the Chip Select use to select the LCD screen connected to the
/// GOUDA_LCD controller.
// =============================================================================
typedef enum
{
    NWY_LCD_BUS_CS_0 = 0,
    NWY_LCD_BUS_CS_1 = 1,
    NWY_LCD_BUS_CS_QTY,
    NWY_LCD_BUS_MEMORY_IF = 2,
    NWY_LCD_BUS_IN_RAM = 3

} nwy_lcd_bus_cs_t;

// =============================================================================
// nwy_lcd_bus_config_t
// -----------------------------------------------------------------------------
/// Describes the configuration of the GOUDA LCD interface.
// =============================================================================
typedef struct
{
    nwy_lcd_bus_cs_t cs : 2;
    bool highByte : 1;
    /// \c FALSE is active low, \c TRUE is active high.
    bool cs0Polarity : 1;
    /// \c FALSE is active low, \c TRUE is active high.
    bool cs1Polarity : 1;
    /// \c FALSE is active low, \c TRUE is active high.
    bool rsPolarity : 1;
    /// \c FALSE is active low, \c TRUE is active high.
    bool wrPolarity : 1;
    /// \c FALSE is active low, \c TRUE is active high.
    bool rdPolarity : 1;
    bool resetb : 1;
    uint32_t clk;
} nwy_lcd_bus_config_t;

void nwy_lcd_bus_init(nwy_lcd_bus_config_t *lcd_bus_config);
bool nwy_lcd_bus_write_cmd(unsigned char cmd);
bool nwy_lcd_bus_write_data(unsigned char data);
unsigned int nwy_lcd_bus_write_datas(void *data, unsigned int size);
unsigned int nwy_lcd_bus_read_datas(unsigned short int addr, unsigned char *pData, unsigned int len);
void nwy_lcd_bus_deinit(void);
void nwy_lcd_bus_power(unsigned int power_id, unsigned int mv);

#ifdef __cplusplus
}
#endif

#endif/*__NWY_LCD_BUS_H__*/

