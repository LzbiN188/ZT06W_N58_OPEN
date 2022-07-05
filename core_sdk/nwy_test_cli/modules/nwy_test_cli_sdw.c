#include "nwy_test_cli_utils.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "nwy_ble.h"
#include "nwy_ble_client.h"
#include "nwy_wifi.h"

/**************************BLE*********************************/

void nwy_test_cli_ble_open()
{
    int status = 0;

    status = nwy_ble_enable();
    if (status)
    {
        nwy_test_cli_echo("\r\n Ble Opened Successfully!\r\n");
    }
}

void nwy_test_cli_ble_set_adv()
{
    int valu = 0;
    char *input_valu = NULL;

    input_valu = nwy_test_cli_input_gets("\r\nset the adv enable(0-disable,1-enable): ");
    valu = atoi(input_valu);
    nwy_ble_set_adv(valu);
    if (valu == 1)
        nwy_test_cli_echo("\r\n %d->Ble Adv Open \r\n", valu);
    else
        nwy_test_cli_echo("\r\n %d->Ble Adv Close \r\n", valu);
}

void nwy_test_cli_ble_send()
{
    char data_str[1024] = {0};
    char *send_data = NULL;
    int status = 0, value = 0,data_len = 0;
    char *input_value = NULL;

    input_value = nwy_test_cli_input_gets("\r\n ble send mode(0-notify,1-indify): ");
    value = atoi(input_value);

    input_value = nwy_test_cli_input_gets("\r\n send data length: ");
    data_len = atoi(input_value);

    send_data = nwy_test_cli_input_gets("\r\n send data:");
    memcpy(data_str, send_data,data_len);
    nwy_test_cli_echo("\r\nnwy_ble_send_data(%s) ", data_str);

    if (value == 0)
        status = nwy_ble_send_data(data_len, data_str);
    else
        status = nwy_ble_send_indify_data(data_len, data_str);
    if (status)
        nwy_test_cli_echo("\r\nBle Send Successfully!\r\n");
}

void nwy_ble_recv_data_func()
{
    char *length = NULL;
    char *precv = NULL;

    length = nwy_ble_receive_data(0);
    precv = nwy_ble_receive_data(1);
    if ((NULL != precv) & (NULL != length))
    {
        nwy_test_cli_echo("\r\n nwy_ble_receive_data");
        nwy_test_cli_echo("\r\n string print:(%d),%s", length, precv);
        nwy_test_cli_echo("\r\n hex print:length=%d,data:", length);
		for(int i=0;i<length;i++)
			nwy_test_cli_echo("0x%02x,",precv[i]);
    }
    else
    {
        nwy_test_cli_echo("nwy_ble receive data is null.");
    }
    nwy_ble_receive_data(2);

    return;
}

void nwy_test_cli_ble_recv()
{
    //nwy_ble_register_callback(nwy_ble_recv_data_func);
    nwy_ble_open_register_cb(nwy_ble_recv_data_func, NWY_BLE_WRITE_FLAG);
    nwy_test_cli_echo("\r\n nwy ble register callback OK");
}

void nwy_test_cli_ble_updata_connt()
{
    nwy_ble_update_conn(0, 60, 80, 4, 500);
}

void nwy_test_cli_ble_get_status()
{
    int status = 0;

    status = nwy_read_ble_status();
    nwy_test_cli_echo("\r\nBle status:%d", (status ? 1 : 0));
}

void nwy_test_cli_ble_get_ver()
{
    char *dev_ver = NULL;

    dev_ver = nwy_ble_get_version();
    if (NULL != dev_ver)
    {
        nwy_test_cli_echo("\r\n nwy_ble_get_version:%s\r\n", dev_ver);
    }
    else
    {
        nwy_test_cli_echo("nwy_ble version data is null.");
    }
}

void nwy_test_cli_ble_set_dev_name()
{
    char *input_value = NULL;
    char *ble_name = NULL;
    int sel = 0;
    input_value = nwy_test_cli_input_gets("\r\nset name:0:,get name:1");
    sel = atoi(input_value);
    if(sel == 0)
    {
        input_value = nwy_test_cli_input_gets("\r\n ble name: ");
        nwy_ble_set_device_name(input_value);
        nwy_test_cli_echo("\r\n new_set_dev_name:(%s) \r\n", input_value);
    }
    else
    {
        ble_name = nwy_ble_get_device_name();
        nwy_test_cli_echo("\r\n get_dev_name:%s", ble_name);
    }
}

void nwy_test_cli_ble_set_beacon()
{
    int status = 0;
    UINT8 uuid[16] = {0xb9, 0x40, 0x7f, 0x30, 0xf5, 0xf8, 0x46, 0x6e, 0xaf, 0xf9, 0x25, 0x55, 0x6b, 0x57, 0xfe, 0x6d};
    UINT8 major[2] = {0x00, 0x01};
    UINT8 minor[2] = {0x00, 0x02};

    status = nwy_ble_beacon(uuid, major, minor);
    if (status)
        nwy_test_cli_echo("\r\nSet Beacon parm,OK!\r\n");
}

void nwy_test_cli_ble_set_manufacture()
{
    int status = 0;
    UINT8 Manufacture_Data[8] = {0x13, 0xff, 0};

    status = nwy_set_manufacture(Manufacture_Data);
    if (status)
        nwy_test_cli_echo("\r\nSet manufacture parm,OK!\r\n");
}

void nwy_test_cli_ble_set_adv_data()
{
  int status;
  // adv attribute: ble name -> nwy_test manufacture_data -> 1~5
  UINT8 Adv_Data[31] = {0x02, 0x01, 0x05, 0x09, 0x09, 0x6E, 0x77, 0x79, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x06, 0xFF, 0x01, 0x02, 0x03, 0x04, 0x05};
  status = nwy_ble_set_adv_data(Adv_Data);
  if (status)
    nwy_test_cli_echo("\r\nSet adv data parm,OK!\r\n");
}

void nwy_test_cli_ble_set_scanrsp_data()
{
  int status;
  // adv attribute: ble name -> nwy_test manufacture_data -> 5~1
  UINT8 Scanrsp_data[31] = {0x02, 0x01, 0x05, 0x09, 0x09, 0x6E, 0x77, 0x79, 0x5F, 0x74, 0x65, 0x73, 0x74, 0x06, 0xFF, 0x05, 0x04, 0x03, 0x02, 0x01};
  status = nwy_ble_set_scanrsp_data(Scanrsp_data);
  if (status)
    nwy_test_cli_echo("\r\nSet scanrsp data parm,OK!\r\n");
}

void nwy_test_cli_ble_set_adv_server_uuid()
{
    int status = 0;
    UINT8 adv_uuid[2] = {0x13, 0xff};

    status = nwy_set_adv_server_uuid(adv_uuid);
    if (status)
        nwy_test_cli_echo("\r\nSet adv server uuid ok!\r\n");
}

void nwy_test_cli_ble_set_srv()
{
    UINT8 srv_uuid[16] = {0xf5, 0x89, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x10, 0xfe, 0x00, 0x00};

    if (!nwy_ble_set_service(srv_uuid))
        nwy_test_cli_echo("\r\n BLE HAS STARTED !\r\n");
}

void nwy_test_cli_ble_set_char()
{
    char *input_value = NULL;
    UINT8 char_uuid_1[16] = {0xf5, 0x89, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x11, 0xfe, 0x00, 0x00};
    UINT8 char_uuid_2[16] = {0xf5, 0x89, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0xfe, 0x00, 0x00};
    UINT8 char_prop = 0;

    nwy_test_cli_echo("\r\n char_prop: 0:Write 1:Write | Notify | indify\r\n");
    input_value = nwy_test_cli_input_gets("char index 0 prop:");
    char_prop = atoi(input_value);
    if (!nwy_ble_set_character(0, char_uuid_1, char_prop))
        nwy_test_cli_echo("\r\n BLE HAS STARTED !\r\n");

    nwy_test_cli_echo("\r\n char_prop: 0:Read 1:Read | Notify | indify\r\n");
    input_value = nwy_test_cli_input_gets("char index 1 prop:");
    char_prop = atoi(input_value);
    if (!nwy_ble_set_character(1, char_uuid_2, char_prop))
        nwy_test_cli_echo("\r\n BLE HAS STARTED !\r\n");
}

void nwy_ble_conn_status_func()
{
    int conn_status = 0;

    conn_status = nwy_ble_get_conn_status();
    if (conn_status != 0)
    {
        nwy_test_cli_echo("\r\nBle connect,status:%d", conn_status);
    }
    else
    {
        nwy_test_cli_echo("\r\nBle disconnect,status:%d", conn_status);
    }
}

void nwy_test_cli_ble_conn_status_report()
{
    nwy_ble_conn_status_cb(nwy_ble_conn_status_func);
}

void nwy_test_cli_ble_conn_status()
{
    int conn_status = 0;
    nwy_open_ble_conn_info *conn_info = NULL;
    char *str_mac = NULL;
    conn_status= nwy_ble_get_conn_status();
    conn_info = nwy_ble_get_client_macaddr();
    if(conn_info != NULL)
    {
        nwy_test_cli_echo("\r\nnwy_ble_conn_info:%d,%02x:%02x:%02x:%02x:%02x:%02x,%d \r\n", conn_status,
        conn_info->ble_addr[5],conn_info->ble_addr[4],conn_info->ble_addr[3],
        conn_info->ble_addr[2],conn_info->ble_addr[1],conn_info->ble_addr[0],conn_info->connected);
    }
    else 
    {
        nwy_test_cli_echo("\r\nnwy_ble_conn_info fail\r\n");
    }
}

void nwy_test_cli_ble_mac_addr()
{
    char *ble_mac = NULL;

    ble_mac = nwy_ble_get_mac_addr();
    if (NULL != ble_mac)
    {
        nwy_test_cli_echo("\r\nBle mac addr:%02x:%02x:%02x:%02x:%02x:%02x", ble_mac[5], ble_mac[4], ble_mac[3],
                          ble_mac[2], ble_mac[1], ble_mac[0]);
    }
    else
    {
        nwy_test_cli_echo("\r\nble get mac addr is null.");
    }
}

void nwy_test_cli_ble_add_server()
{
    uint8_t uuid0[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x89, 0x00, 0x00};
    uint8_t uuid1[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xe1, 0xff, 0x00, 0x00};
    uint8_t uuid2[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xe2, 0xff, 0x00, 0x00};

    nwy_ble_service_info ser_info_0;
    nwy_ble_service_info ser_info_1;
    nwy_ble_service_info ser_info_2;

    ser_info_0.ser_index = 1;
    ser_info_1.ser_index = 2;
    ser_info_2.ser_index = 3;

    memcpy(ser_info_0.ser_uuid, uuid0, 16);
    memcpy(ser_info_1.ser_uuid, uuid1, 16);
    memcpy(ser_info_2.ser_uuid, uuid2, 16);

    ser_info_0.char_num = 2;
    ser_info_1.char_num = 2;
    ser_info_2.char_num = 2;

    ser_info_0.p = 1;
    ser_info_1.p = 1;
    ser_info_2.p = 1;

    if (!nwy_ble_add_service(&ser_info_0))
        nwy_test_cli_echo("\r\nadd ble service 1 error");
    if (!nwy_ble_start_service(1))
        nwy_test_cli_echo("\r\nstart ble service 1 error");

    if (!nwy_ble_add_service(&ser_info_1))
        nwy_test_cli_echo("\r\nadd ble service 2 error");
    if (!nwy_ble_start_service(2))
        nwy_test_cli_echo("\r\nstart ble service 2 error");

    if (!nwy_ble_add_service(&ser_info_2))
        nwy_test_cli_echo("\r\nadd ble service 3 error");
    if (!nwy_ble_start_service(3))
        nwy_test_cli_echo("\r\nstart ble service 3 error");
}

void nwy_test_cli_ble_add_char()
{
    int sel = 0;
    char *input_value = NULL;
    uint8_t uuid0[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x01, 0x89, 0x00, 0x00};
    uint8_t uuid1[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x02, 0x89, 0x00, 0x00};
    uint8_t uuid2[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x03, 0x89, 0x00, 0x00};
    uint8_t uuid3[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x04, 0x89, 0x00, 0x00};
    uint8_t uuid4[16] = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x05, 0x89, 0x00, 0x00};

    nwy_ble_char_info char_info_0;
    nwy_ble_char_info char_info_1;
    nwy_ble_char_info char_info_2;
    nwy_ble_char_info char_info_3;
    nwy_ble_char_info char_info_4;

    char_info_0.ser_index = 1;
    char_info_1.ser_index = 1;
    char_info_2.ser_index = 1;
    char_info_3.ser_index = 1;
    char_info_4.ser_index = 1;

    char_info_0.char_index = 0;
    char_info_1.char_index = 1;
    char_info_2.char_index = 2;
    char_info_3.char_index = 3;
    char_info_4.char_index = 4;

    memcpy(char_info_0.char_uuid, uuid0, 16);
    memcpy(char_info_1.char_uuid, uuid1, 16);
    memcpy(char_info_2.char_uuid, uuid2, 16);
    memcpy(char_info_3.char_uuid, uuid3, 16);
    memcpy(char_info_4.char_uuid, uuid4, 16);

    char_info_0.char_per = 2;
    char_info_1.char_per = 2;
    char_info_2.char_per = 2;
    char_info_3.char_per = 2;
    char_info_4.char_per = 2;

    char_info_0.char_des = 1;
    char_info_1.char_des = 1;
    char_info_2.char_des = 1;
    char_info_3.char_des = 1;
    char_info_4.char_des = 1;

    nwy_test_cli_echo("\r\nproperties: 0:write 1:read 2:notify 3:indify 4:write | notify 5:read | notify 6:all");
    input_value = nwy_test_cli_input_gets("\r\nchar_info_0.char_cp:");
    sel = atoi(input_value);
    char_info_0.char_cp = sel;
    input_value = nwy_test_cli_input_gets("\r\nchar_info_1.char_cp:");
    sel = atoi(input_value);
    char_info_1.char_cp = sel;
    input_value = nwy_test_cli_input_gets("\r\nchar_info_2.char_cp:");
    sel = atoi(input_value);
    char_info_2.char_cp = sel;
    input_value = nwy_test_cli_input_gets("\r\nchar_info_3.char_cp:");
    sel = atoi(input_value);
    char_info_3.char_cp = sel;
    input_value = nwy_test_cli_input_gets("\r\nchar_info_4.char_cp:");
    sel = atoi(input_value);
    char_info_4.char_cp = sel;

    if (!nwy_add_ble_charcter(&char_info_0))
        nwy_test_cli_echo("\r\nadd ble charcter 0 error");

    if (!nwy_add_ble_charcter(&char_info_1))
        nwy_test_cli_echo("\r\nadd ble charcter 1 error");

    if (!nwy_add_ble_charcter(&char_info_2))
        nwy_test_cli_echo("\r\nadd ble charcter 2 error");

    if (!nwy_add_ble_charcter(&char_info_3))
        nwy_test_cli_echo("\r\nadd ble charcter 3 error");

    if (!nwy_add_ble_charcter(&char_info_4))
        nwy_test_cli_echo("\r\nadd ble charcter 4 error");
}

void nwy_ble_add_recv_data_func()
{
    nwy_ble_recv_info recv_data;
    nwy_ble_add_recv_data(&recv_data);
    nwy_test_cli_echo("\r\nrecv_data info:srv_id=%d,char_id=%d", recv_data.ser_id, recv_data.char_id);
    nwy_test_cli_echo("\r\n nwy_ble_receive_data");
    nwy_test_cli_echo("\r\n string print:(%d),%s",  recv_data.datalen, recv_data.data);
    nwy_test_cli_echo("\r\n hex print:length=%d,data:", recv_data.datalen);
    for(int i=0;i<recv_data.datalen;i++)
        nwy_test_cli_echo("0x%02x,",recv_data.data[i]);

    return;
}

void nwy_test_cli_ble_add_recv_data()
{
    int ret = 0;

    ret = nwy_ble_open_register_cb(nwy_ble_add_recv_data_func, NWY_BLE_ADD_WRITE_FLAG);
    if (ret)
        nwy_test_cli_echo("\r\n recv data register ok");
    else
        nwy_test_cli_echo("\r\n recv data register error");
}
void nwy_test_cli_ble_add_send_data()
{
    char *send_data = NULL;
    int sel = 0;
    char *input_value = NULL;
    int status = 0,data_len = 0;
    nwy_ble_send_info ble_send_data;

    input_value = nwy_test_cli_input_gets("\r\nser_id:");
    sel = atoi(input_value);
    ble_send_data.ser_id = sel;

    input_value = nwy_test_cli_input_gets("\r\nchar_id:");
    sel = atoi(input_value);
    ble_send_data.char_id = sel;

    input_value = nwy_test_cli_input_gets("\r\nsend mode(0-notify,1-indify):");
    sel = atoi(input_value);
    ble_send_data.op = sel;

    input_value = nwy_test_cli_input_gets("\r\n send data length: ");
    data_len = atoi(input_value);

    send_data = nwy_test_cli_input_gets("\r\n send data:");
    memcpy((char *)ble_send_data.data, send_data,data_len);
    ble_send_data.datalen = data_len;
    nwy_test_cli_echo("\r\nnwy_ble_send_data(%d)%s ", ble_send_data.datalen, ble_send_data.data);

    status = nwy_ble_add_send_data(&ble_send_data);
    if (status)
        nwy_test_cli_echo("\r\nBle Send Successfully!\r\n");
}

void nwy_test_cli_ble_disconnect()
{
    if (nwy_ble_disconnect())
        nwy_test_cli_echo("\r\nBle disconnect ok!\r\n");
    else
        nwy_test_cli_echo("\r\nBle disconnect error!\r\n");
}

void nwy_ble_read_info_func()
{
    nwy_ble_read_info read_info;

    nwy_ble_read_req_info(&read_info);
    nwy_test_cli_echo("\r\nread_req info:srv_id=%d,char_id=%d,rw_flag=%d", read_info.ser_id, read_info.char_id, read_info.rw_flag);

    return;
}
void nwy_test_cli_ble_read_req()
{
    int ret = 0;

    ret = nwy_ble_open_register_cb(nwy_ble_read_info_func, NWY_BLE_READ_FLAG);
    if (ret)
        nwy_test_cli_echo("\r\n read info register ok");
    else
        nwy_test_cli_echo("\r\n read info register error");
}

void nwy_test_cli_ble_read_rsp()
{
    char *send_data = NULL;
    int sel = 0;
    char *input_value = NULL;
    nwy_ble_read_rsp read_rsp;

    input_value = nwy_test_cli_input_gets("\r\nser_id:(0~1)");
    sel = atoi(input_value);
    read_rsp.ser_id = sel;

    input_value = nwy_test_cli_input_gets("\r\nchar_id:(ser_id:0,0~1;ser_id:1,0~4)");
    sel = atoi(input_value);
    read_rsp.char_id = sel;

    input_value = nwy_test_cli_input_gets("\r\ndata_len:");
    sel = atoi(input_value);
    read_rsp.rsp_len = sel;

    send_data = nwy_test_cli_input_gets("\r\nsend_data:");
    memcpy(read_rsp.read_rsp, send_data, read_rsp.rsp_len);
    nwy_test_cli_echo("\r\nnwy_read_rsp_data:%s ", read_rsp.read_rsp);

    if (!nwy_ble_read_rsp_info(&read_rsp))
        nwy_test_cli_echo("\r\nnwy read rsp parm error");

}

void nwy_test_cli_ble_add_white_list()
{
    char *input_valu = NULL;
    unsigned char bleaddr[BT_ADDRESS_SIZE] = {0};
    int addr_type = 0;

    input_valu = nwy_test_cli_input_gets("\r\naddr type:");
    addr_type = atoi(input_valu);
    input_valu = nwy_test_cli_input_gets("\r\nble addr");
    if(!nwy_ble_client_string_to_BleAddr(input_valu, strlen(input_valu),(unsigned char *)bleaddr))
        nwy_test_cli_echo("\r\nstring to bleaddr false");

    if(nwy_ble_add_white_list(&bleaddr, addr_type))
        nwy_test_cli_echo("\r\nble add white list success!\r\n");
    else
        nwy_test_cli_echo("\r\nble add white list fail!\r\n");
}

void nwy_test_cli_ble_remove_white_list()
{
    char *input_valu = NULL;
    unsigned char bleaddr[BT_ADDRESS_SIZE] = {0};
    int addr_type = 0;

    input_valu = nwy_test_cli_input_gets("\r\naddr type:");
    addr_type = atoi(input_valu);
    input_valu = nwy_test_cli_input_gets("\r\nble addr");
    if(!nwy_ble_client_string_to_BleAddr(input_valu, strlen(input_valu),(unsigned char *)bleaddr))
        nwy_test_cli_echo("\r\nstring to bleaddr false");

    if(nwy_ble_remove_white_list(bleaddr, addr_type))
        nwy_test_cli_echo("\r\nble remove white list success!\r\n");
    else
        nwy_test_cli_echo("\r\nble remove white list fail!\r\n");
}

void nwy_test_cli_ble_clean_white_list()
{
    if(nwy_ble_clean_white_list())
        nwy_test_cli_echo("\r\nble clean white list success!\r\n");
    else
        nwy_test_cli_echo("\r\nble clean white list fail!\r\n");
}

void nwy_test_cli_ble_set_adv_param()
{
    char *input_valu = NULL;
    int adv_filter = 0;
    nwy_ble_open_adv_param adv_parm_info;
    adv_parm_info.ble_adv_max = 0xa0;
    adv_parm_info.ble_adv_min = 0x80;
    adv_parm_info.ble_adv_type = 0;
    nwy_test_cli_echo("\r\n0:Accept scan or connection from any device.\r\n");
    nwy_test_cli_echo("\r\n1:Only accept scan in the whitelist, but accept connection.\r\n");
    nwy_test_cli_echo("\r\n2:Accept scan, but only accept connection in the whitelist.\r\n");
    nwy_test_cli_echo("\r\n3:Only accept scan and connection in the whitelist.\r\n");
    input_valu = nwy_test_cli_input_gets("\r\nAdv Filter Policy:");
    adv_filter = atoi(input_valu);
    switch(adv_filter)
    {
        case 0:
            adv_parm_info.ble_adv_filter = 0x00;
            break;
        case 1:
            adv_parm_info.ble_adv_filter = 0x01;
            break;
        case 2:
            adv_parm_info.ble_adv_filter = 0x02;
            break;
        case 3:
            adv_parm_info.ble_adv_filter = 0x03;
            break;
        default:
            nwy_test_cli_echo("\r\nAdv Filter Param Error\r\n");
            return;
    }
    if(nwy_ble_set_adv_param(&adv_parm_info))
        nwy_test_cli_echo("\r\nble set adv param success!\r\n");
    else
        nwy_test_cli_echo("\r\nble set adv param fail!\r\n");
}

void nwy_test_cli_ble_close()
{
    nwy_ble_disable();
    nwy_test_cli_echo("\r\nBle Closed Successfully!\r\n");
}

/**************************BLE Client*********************************/

static int srv_num = 0;
static uint8_t scan_sel = 0;
static bool scan_dev_flag = true;

void nwy_test_cli_ble_client_set_enable()
{
    char *input_value = NULL;
    uint32_t sel = 0;

    nwy_test_cli_echo("\r\nset ble client open and close:1->open,0->close\r\n");
    input_value = nwy_test_cli_input_gets("select:");
    sel = atoi(input_value);

    if (nwy_ble_client_set_enable(sel))
        nwy_test_cli_echo("Ble Client Open");
    else
        nwy_test_cli_echo("Ble Client Close");
}

void nwy_ble_client_scan_dev_func()
{
    nwy_ble_c_scan_dev scan_info;

    nwy_ble_client_scan_result(&scan_info);
    nwy_test_cli_echo("\r\n%02x:%02x:%02x:%02x:%02x:%02x,%d,%s,-%ddBm",
                      scan_info.bdAddress.addr[0],
                      scan_info.bdAddress.addr[1],
                      scan_info.bdAddress.addr[2],
                      scan_info.bdAddress.addr[3],
                      scan_info.bdAddress.addr[4],
                      scan_info.bdAddress.addr[5],
                      scan_info.addr_type,
                      scan_info.name,
                      scan_info.rssi);
    if (!scan_sel)
    {
        nwy_test_cli_echo("\r\nibeacon info:");
        for (int i = 0; i < 16; i++)
        {
            nwy_test_cli_echo("%02x", scan_info.beacon.uuid[i]);
            nwy_test_cli_echo("\r\nmajor:%04x->%d,minor:%04x->%d,tx_poer_l:%02x->%d",
                              scan_info.beacon.major, scan_info.beacon.major,
                              scan_info.beacon.minor, scan_info.beacon.minor,
                              scan_info.beacon.tx_power_level, scan_info.beacon.tx_power_level);
        }
    }
}

void nwy_test_cli_ble_client_scan()
{
    char *input_value = NULL;
    int status = 0;
    uint8_t scan_time = 1;

    if (scan_dev_flag)
    {
        scan_dev_flag = false;
        if (nwy_ble_client_register_cb(nwy_ble_client_scan_dev_func, BLE_CLIENT_SCAN_DEV))
            nwy_test_cli_echo("nwy ble client scan dev cb register ok");
    }

    input_value = nwy_test_cli_input_gets("\r\nsacn time:");
    scan_time = atoi(input_value);
    input_value = nwy_test_cli_input_gets("\r\nsacn sel:ble->1,ibeacon->0:");
    scan_sel = atoi(input_value);

    nwy_test_cli_echo("\r\nnwy ble client scanning");
    if (scan_sel)
        status = nwy_ble_client_scan(scan_time);
    else
        status = nwy_ble_client_scan_ibeacon(scan_time);
    if (status)
        nwy_test_cli_echo("\r\nnwy ble client scan end");
}

void nwy_test_cli_ble_client_connect()
{
    char data_str[18] = {0};
    char *mac_addr = NULL;
    int status = 0;
    char *input_value = NULL;
    uint32_t sel = 0;

    input_value = nwy_test_cli_input_gets("\r\naddr type:");
    sel = atoi(input_value);
    mac_addr = nwy_test_cli_input_gets("\r\n mac addr:");
    strcpy(data_str, mac_addr);
    nwy_test_cli_echo("\r\nnwy_ble_send_data(%s) ", data_str);

    status = nwy_ble_client_connect(sel, data_str);
    if (status)
        nwy_test_cli_echo("\r\nBle Client Conn Set Parm OK!\r\n");
    else
        nwy_test_cli_echo("\r\nBle Client Conn Set Parm Error!\r\n");
}

void nwy_test_cli_ble_client_disconnect()
{
    int status = 0;

    status = nwy_ble_client_disconnect();
    if (status)
        nwy_test_cli_echo("\r\nBle Client Disconnect OK!\r\n");
    else
        nwy_test_cli_echo("\r\nBle Client Disconnect Error!\r\n");
}

void nwy_test_cli_ble_client_discover_srv()
{
    nwy_ble_c_discover srv_info[20];

    srv_num = nwy_ble_client_discover_srv(srv_info);
    if (srv_num)
        for (int i = 0; i < srv_num; i++)
            nwy_test_cli_echo("\r\nser_id:%d,0x%04x", i, srv_info[i].uuid);
}

void nwy_test_cli_ble_client_discover_char()
{
    nwy_ble_c_discover char_info[10];
    uint8_t char_num = 0;

    nwy_ble_client_discover_char(char_info);
    nwy_test_cli_echo("\r\ndiscover_char printf format:srv_id,char_id,char uuid,prop");
    for (int i = 0; i < srv_num; i++)
    {
        char_num = char_info[i].charNum;
        for (int j = 0; j < char_num; j++)
        {
            nwy_test_cli_echo("\r\n%d,%d,0x%04x,0x%02x", i, j, char_info[i].discover_char[j].uuid,
                              char_info[i].discover_char[j].properties);
        }
    }
}

void nwy_test_cli_ble_client_send_data()
{
    int sel = 0;
    char *input_value = NULL;
    int status = 0,data_len = 0;
    nwy_ble_c_send_param ble_send_data;

    input_value = nwy_test_cli_input_gets("\r\nser_id:");
    sel = atoi(input_value);
    ble_send_data.ser_id = sel;

    input_value = nwy_test_cli_input_gets("\r\nchar_id:");
    sel = atoi(input_value);
    ble_send_data.char_id = sel;

    input_value = nwy_test_cli_input_gets("\r\n send data length: ");
    data_len = atoi(input_value);

    ble_send_data.data = (uint8_t *)nwy_test_cli_input_gets("\r\n send data:");
    nwy_test_cli_echo("\r\nnwy_ble_send_data ");
    ble_send_data.len = data_len;
    nwy_test_cli_echo("\r\nnwy_ble_send_data(%d)%s ", ble_send_data.len, (char *)ble_send_data.data);

    status = nwy_ble_client_send_data(&ble_send_data);
    if (status)
        nwy_test_cli_echo("\r\nBle Send Successfully!\r\n");
}

void nwy_ble_client_recv_data_func()
{
    nwy_ble_c_recv_info recv_data;
    nwy_ble_client_recv_data(&recv_data);
    nwy_test_cli_echo("\r\nrecv data:%d,%d", recv_data.ser_id, recv_data.char_id);
    nwy_test_cli_echo("\r\n nwy_ble_receive_data");
    nwy_test_cli_echo("\r\n string print:(%d),%s",  recv_data.len, recv_data.data);
    nwy_test_cli_echo("\r\n hex print:length=%d,data:", recv_data.len);
    for(int i=0;i<recv_data.len;i++)
        nwy_test_cli_echo("0x%02x,",recv_data.data[i]);
}

void nwy_test_cli_ble_client_recv_data()
{
    if (nwy_ble_client_register_cb(nwy_ble_client_recv_data_func, BLE_CLIENT_RECV_DATA))
        nwy_test_cli_echo("nwy ble client recv data register ok");
}

/**************************WIFI*********************************/

void nwy_cli_test_wifi_get_st()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_enable()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_set_work_md()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_set_ap_para()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_set_ap_para_adv()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_get_clit_info()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_sta_scan()
{
    int i = 0;
    nwy_wifi_scan_list_t scan_list;

    memset(&scan_list, 0, sizeof(scan_list));

    nwy_wifi_scan(&scan_list);

    for (i = 0; i < scan_list.num; i++)
    {
        nwy_test_cli_echo("\r\nwifi ap mac is %02x:%02x:%02x:%02x:%02x:%02x",
                          scan_list.ap_list[i].mac[5], scan_list.ap_list[i].mac[4], scan_list.ap_list[i].mac[3], scan_list.ap_list[i].mac[2],
                          scan_list.ap_list[i].mac[1], scan_list.ap_list[i].mac[0]);
        nwy_test_cli_echo("	channel = %d", scan_list.ap_list[i].channel);
        nwy_test_cli_echo("	rssi = %d", scan_list.ap_list[i].rssival);
    }
    if (i == 0)
    {
        nwy_test_cli_echo("\r\n Wifi scan nothing");
    }
}

void nwy_cli_test_wifi_sta_scan_ret()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_sta_connt()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_sta_disconnt()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_sta_get_hostpot_info()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_cli_test_wifi_disable()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}
