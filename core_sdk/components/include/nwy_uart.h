#ifndef __NWY_UART_H__
#define __NWY_UART_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stdbool.h"
#if 0
/*----------------------------Type Definition-----------------------------*/
typedef enum {
    FC_NONE = 0,  // None Flow Control
    FC_RTSCTS,    // Hardware Flow Control (rtscts)
    FC_XONXOFF    // Software Flow Control (xon/xoff)
}nwy_flowctrl_t;

typedef enum {
    PB_NONE = 0,
    PB_ODD,
    PB_EVEN,
    PB_SPACE,
    PB_MARK
}nwy_paritybits_t;

typedef struct {
    unsigned int databits;
    unsigned int stopbits;
    nwy_paritybits_t parity;
    nwy_flowctrl_t flowctrl;
}nwy_sio_control_t;

/*---------------------------Function Definition--------------------------*/
void nwy_get_uart_para(nwy_sio_control_t *para);
void nwy_set_uart_para(nwy_sio_control_t *para);
#else
/*----------------------------Type Definition-----------------------------*/
#define NWY_MAKE_TAG(a, b, c, d) ((unsigned)(a) | ((unsigned)(b) << 8) | ((unsigned)(c) << 16) | ((unsigned)(d) << 24))

#define NWY_NAME_UART1    NWY_MAKE_TAG('U', 'R', 'T', '1')
#define NWY_NAME_UART2    NWY_MAKE_TAG('U', 'R', 'T', '2')
#define NWY_NAME_UART3    NWY_MAKE_TAG('U', 'R', 'T', '3')

#define NWY_ERROR   -1
typedef enum
{
    NWY_UART_NO_PARITY,   ///< No parity check
    NWY_UART_ODD_PARITY,  ///< Parity check is odd
    NWY_UART_EVEN_PARITY, ///< Parity check is even
} nwy_uart_parity_t;

typedef enum
{
    NWY_UART_DATA_BITS_7 = 7,
    NWY_UART_DATA_BITS_8 = 8
} nwy_uart_data_bits_t;

/**
 * @brief UART stop bits
 */
typedef enum
{
    NWY_UART_STOP_BITS_1 = 1,
    NWY_UART_STOP_BITS_2 = 2
} nwy_uart_stop_bits_t;

typedef enum
{
    NWY_UART_MODE_AT = 0,
    NWY_UART_MODE_DATA = 1
} nwy_uart_mode_t;

typedef void (*nwy_uart_recv_callback_t) (const char *str,uint32_t length);
typedef void(*nwy_uart_send_callback_t)(int param);

/*---------------------------Function Definition--------------------------*/
/**
 * @brief choose the uart which you want to open
 *
 * @param name : uart name (NWY_NAME_UART1,NWY_NAME_UART2,NWY_NAME_UART3)
 * @param mode : 0-at mode, 1-data mode
 
 * @return 0: NWY_NAME_UART1 open success
           1: NWY_NAME_UART2 open success
           2: NWY_NAME_UART3 open success
          -1: NWY_ERROR
 */
int  nwy_uart_init(uint32_t name,nwy_uart_mode_t mode);

/**
 * @brief set uart baud
 *
 * @param hd : hd = nwy_uart_init (name,mode);
 * @param baud : baud
 
 * @return 0: set failed
 *         1: set success
 */
bool nwy_uart_set_baud(uint8_t hd,uint32_t baud);

/**
 * @brief get uart baud
 *
 * @param hd : hd = nwy_uart_init (name,mode);
 * @param baud : baud
 
 * @return 0: get failed
 *         1: get success
 */
bool nwy_uart_get_baud(uint8_t hd,uint32_t *baud);

/**
 * @brief set uart para
 *
 * @param hd : hd = nwy_uart_init(name,mode);
 * @param parity : nwy_uart_parity_t
 * @param data_size : nwy_uart_data_bits_t
 * @param stop_size : nwy_uart_stop_bits_t
 * @param flow_ctrl : 0-close, 1-open
 
 * @return 0: set failed
 *         1: set success
 */
bool nwy_uart_set_para(uint8_t hd,nwy_uart_parity_t parity, nwy_uart_data_bits_t data_size, nwy_uart_stop_bits_t stop_size, bool flow_ctrl);

/**
 * @brief get uart para
 *
 * @param hd : hd = nwy_uart_init(name,mode);
 * @param parity : nwy_uart_parity_t
 * @param data_size : nwy_uart_data_bits_t
 * @param stop_size : nwy_uart_stop_bits_t
 * @param flow_ctrl : 0-close, 1-open
 
 * @return 0: get failed
 *         1: get success
 */
bool nwy_uart_get_para(uint8_t hd,nwy_uart_parity_t *parity, nwy_uart_data_bits_t *data_size, nwy_uart_stop_bits_t *stop_size, bool *flow_ctrl);

/**
 * @brief the uart send data
 *
 * @param hd : hd = nwy_uart_init(name,mode);
 * @param data_ptr : data you want to send
 * @param length : data length
 * @return
 */
int  nwy_uart_send_data(uint8_t hd,uint8_t *data_ptr, uint32_t length);

/**
 * @brief the uart recv cb
 *
 * @param hd : hd = nwy_uart_init(name,mode);
 * @param recv_cb : nwy_uart_recv_callback_t
 *
 * @return 0: failed
 *         1: success
 */
bool nwy_uart_reg_recv_cb(uint8_t hd,nwy_uart_recv_callback_t recv_cb);

/**
 * @brief the uart tx cb
 *
 * @param hd : hd = nwy_uart_init(name,mode);
 * @param tx_cb : nwy_uart_send_callback_t
 *
 * @return 0: failed
 *         1: success
 */
bool nwy_uart_reg_tx_cb(uint8_t hd, nwy_uart_send_callback_t tx_cb);

/**
 * @brief choose the uart which you want to close
*
 * @param hd : hd = nwy_uart_init(name,mode);
 
 * @return 0: failed
 *         1: success
 */
bool nwy_uart_deinit(uint8_t hd);

int nwy_set_rx_frame_timeout(uint8_t hd, int time);

typedef enum
{
    NWY_AT_UART1 = 1,
    NWY_AT_UART2 = 2
} nwy_uart_port_t;

int nwy_at_uart_send(nwy_uart_port_t port, void *data, size_t size);


#endif
#ifdef __cplusplus
   }
#endif

#endif

