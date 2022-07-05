#ifndef __NWY_BLE_CLIENT_H__
#define __NWY_BLE_CLIENT_H__
#ifdef __cplusplus
extern "C" {
#endif
#define NWY_BLE_SCAN_INFO_MAX_COUNT             50

#define SRV_INDEX_MAX 10
#define CHAR_INDEX_MAX 10
#define AT_BT_NAME_MAX_SIZE 31
#define MGR_MAX_REMOTE_NAME_LEN 32
#define SIZE_OF_BDADDR 6
/// Advertising data maximum length
#define NWY_BLE_ADV_DATA_LEN_MAX               31
/// Scan response data maximum length
#define NWY_BLE_SCAN_RSP_DATA_LEN_MAX          31
#define IBEACON_INFO_LEN_MIN    0x1A
#define IBEACON_APPLE_VENDOR    0x004C


#define BLE_CLIENT_SCAN_DEV      0
#define BLE_CLIENT_RECV_DATA 1

#define NWY_BLE_FAIL 0
#define NWY_BLE_SUCCESS 1
#define NWY_BLE_ERROR -1

typedef struct _BLE_ADDRESS
{
    uint8 addr[SIZE_OF_BDADDR];
} BLE_ADDRESS;

typedef struct {
    UINT8 name_length;
    UINT8 name[MGR_MAX_REMOTE_NAME_LEN];
    uint8_t adv_data_raw[NWY_BLE_ADV_DATA_LEN_MAX];
    BLE_ADDRESS bdAddress;
    UINT8 addr_type;
    UINT8 event_type;
    int rssi;
    uint8_t adv_data_len;
    uint8_t scan_rsp_len;
} ble_scan_result_evt_param;

typedef struct {
    uint8_t flag_len;
    uint8_t flag_type;
    uint8_t flag_value;
    uint8_t len;
    uint8_t type;
    uint16_t apple;
    uint8_t ib_type;
    uint8_t ib_len;
    uint8_t uuid[16];
    uint16_t major;
    uint16_t minor;
    uint8_t tx_power_level;
} nwy_beacon_info;


typedef struct
{
    uint16_t uuid;
    uint16_t handle;
    uint32_t length;
    uint8_t *value;
} nwy_ble_chara_desc_t;

typedef struct
{
    uint16_t handle;
    uint16_t uuid;
    uint16_t valueHandle;
    uint16_t properties;
    int descNum;
    uint16_t *value;
    nwy_ble_chara_desc_t *pDescList;
} nwy_ble_chara_t;

/*ble client discover service and character*/
typedef struct
{
    uint16_t startHandle;
    uint16_t endHandle;
    uint16_t uuid;
    uint16_t charNum;
    nwy_ble_chara_t discover_char[CHAR_INDEX_MAX];
}nwy_ble_c_discover;

/*ble client send param */
typedef struct
{
    uint8_t ser_id;
    uint8_t char_id;
    uint8_t *data;
    int len;
    int type;
}nwy_ble_c_send_param;

/*ble client scan device info */
typedef struct
{
    uint8_t addr_type;
    BLE_ADDRESS bdAddress;
    uint8_t rssi;
    uint8_t name[MGR_MAX_REMOTE_NAME_LEN];
    UINT8 name_length;
    uint8_t adv_data_raw[NWY_BLE_ADV_DATA_LEN_MAX];
    uint8_t adv_data_len;
    uint8_t scan_rsp_len;
    uint8_t scan_rsp_raw[NWY_BLE_SCAN_RSP_DATA_LEN_MAX];
    nwy_beacon_info beacon;
}nwy_ble_c_scan_dev;

/*ble client recv info */
typedef struct
{
    uint8_t ser_id;
    uint8_t char_id;
    uint8_t *data;
    int len;
}nwy_ble_c_recv_info;

enum
{
    CHAR_DATA_CB_IDLE = 0,
    CHAR_DATA_CB_DISCOVER = 1,
    CHAR_DATA_CB_READ_VALUE = 2,
};
void nwy_ble_client_bleaddr_to_string(uint8_t *addr, uint8_t *str_ptr);
bool nwy_ble_client_string_to_BleAddr(uint8_t *str_ptr, uint8_t length, uint8_t *addr);

int nwy_ble_client_register_cb(void (*ble_client_reg_cb)(),uint8_t flag);
int nwy_ble_client_set_enable(uint32_t mode);
int nwy_ble_client_scan(uint8_t time);
int nwy_ble_client_scan_ibeacon(uint8_t time);
int nwy_ble_client_scan_result(nwy_ble_c_scan_dev* scan_result);
int nwy_ble_client_connect(uint8_t addr_type,char *str_mac);
int nwy_ble_client_disconnect();
int nwy_ble_client_discover_srv(nwy_ble_c_discover* srv_info);
int nwy_ble_client_discover_char(nwy_ble_c_discover* char_info      );
int nwy_ble_client_send_data(nwy_ble_c_send_param* client_send);
int nwy_ble_client_recv_data(nwy_ble_c_recv_info* recv_data);


#ifdef __cplusplus
   }
#endif

#endif

