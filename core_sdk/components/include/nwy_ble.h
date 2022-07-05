#ifndef __NWY_BLE_H__
#define __NWY_BLE_H__
#ifdef __cplusplus
extern "C" {
#endif
#define BT_ADDRESS_SIZE 6
#define OPEN_CREATE_SRV_MAX_NUM    3
#define OPEN_CREATE_CHAR_MAX_NUM   5
#define DATA_SIZE (2*1024)
#define NWY_BLE_READ_FLAG 0
#define NWY_BLE_WRITE_FLAG 1
#define NWY_BLE_ADD_WRITE_FLAG 2




/**The parameters of gatt adv*/
typedef struct
{
    uint8_t   ble_adv_type;     /**< The type of adv */
    uint16_t  ble_adv_min;      /**< The minimum adv interval, unit is 0.625ms, within range 20ms~10.24s */
    uint16_t  ble_adv_max;      /**< The maxmum adv interval, unit is 0.625ms, within range 20ms~10.24s */
    uint8_t   ble_adv_filter;   /**< The  policy of adv filter */
} nwy_ble_open_adv_param;

typedef struct nwy_st_white_list_info
{
    uint8 addr_type;
    uint8 addr[BT_ADDRESS_SIZE];
} nwy_st_white_list_info;

typedef struct
{
	int conn_id;
	int connected;
	uint8_t ble_addr[BT_ADDRESS_SIZE];
}nwy_open_ble_conn_info;

typedef void (*NwyAppBtMsgHandler_t)(void *param);
typedef void (*NWY_BLE_WRITE_UART_CALLBACK)(short dlen, uint8 *data);
typedef unsigned char UINT8;
typedef struct
{
    uint8_t ser_index;
    uint8_t char_index;
    uint8_t char_uuid[16];
    uint8_t char_des; 
    uint8_t char_per; 
    uint16_t permisssion;
    uint8_t char_cp;
}nwy_ble_char_info;

typedef struct
{
    uint32_t ser_index;
    uint8_t ser_uuid[16];
    uint8_t char_num;
    uint8_t p;
    nwy_ble_char_info ser_ble_char[OPEN_CREATE_CHAR_MAX_NUM];
}nwy_ble_service_info;

typedef struct
{
    uint8_t ser_id;
    uint8_t char_id;
    uint8_t data[DATA_SIZE];
    uint16_t datalen;
    uint8_t op;
}nwy_ble_send_info;

typedef struct
{
    uint8_t ser_id;
    uint8_t char_id;
    uint8_t data[244];
    uint16_t datalen;
}nwy_ble_recv_info;

typedef struct
{
    uint8_t ser_id;
    uint8_t char_id;
    uint8_t rw_flag;
}nwy_ble_read_info;

typedef struct
{
    uint8_t ser_id;
    uint8_t char_id;
    uint8_t read_rsp[244];
    uint8_t rsp_len;
}nwy_ble_read_rsp;


typedef struct
{
    uint16_t configurationBits;
    uint16_t aclHandle;
} nwy_gatt_chara_cb_o;



int nwy_ble_enable();
int nwy_ble_set_adv_param(nwy_ble_open_adv_param *adv_parm);
int nwy_ble_set_adv(bool enable);
int nwy_ble_send_data(uint16 datalen, char *data);
int nwy_ble_send_indify_data(uint16 datalen, char *data);
int nwy_read_ble_status();
int nwy_ble_disable();
int nwy_ble_register_callback(void (*ble_reg_cb)());
char * nwy_ble_receive_data( int sel );
char *nwy_ble_get_version();
int nwy_ble_set_device_name(char *local_name);
char *nwy_ble_get_device_name();
int nwy_ble_update_conn(uint16 handle, uint16 intervalMin, uint16 intervalMax, uint16 slaveLatency, uint16 timeoutMulti);
int nwy_ble_beacon(UINT8 uuid[16], UINT8 major[2], UINT8 minor[2]);
int nwy_set_manufacture(UINT8 Manufacture_Data[8]);
int nwy_ble_set_adv_data(UINT8 Adv_Data[31]);
int nwy_ble_set_scanrsp_data(UINT8 Scanrsp_Data[31]);
int nwy_set_adv_server_uuid(UINT8 adv_uuid[2]);
int nwy_ble_set_service(UINT8 srv_uuid[]);
int nwy_ble_set_character(UINT8 char_index ,UINT8 char_uuid[],UINT8 prop);
int nwy_ble_get_conn_status();
int nwy_ble_conn_status_cb(void (*ble_conn_cb)());
char *nwy_ble_get_mac_addr();
int nwy_ble_add_service(nwy_ble_service_info* serv_info);
int nwy_ble_start_service(uint8_t ser_id);
int nwy_add_ble_charcter(nwy_ble_char_info* char_info);
int nwy_ble_add_recv_data( nwy_ble_recv_info* recv_data);
int nwy_ble_add_send_data(nwy_ble_send_info* send_data);
int nwy_ble_disconnect();
int nwy_ble_open_register_cb(void (*ble_add_reg_cb)(),UINT8 flag);
int nwy_ble_read_req_info(nwy_ble_read_info* read_req_info);
int nwy_ble_read_rsp_info(nwy_ble_read_rsp* read_rsp);
int nwy_ble_add_white_list(uint8 *addr, uint8 addr_type);
int nwy_ble_remove_white_list(uint8 *addr,uint8_t addr_type);
int nwy_ble_clean_white_list();
nwy_open_ble_conn_info* nwy_ble_get_client_macaddr();


#ifdef __cplusplus
   }
#endif

#endif

