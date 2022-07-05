// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

#include "osi_log.h"

#include "nwy_data.h"
#include "nwy_socket.h"
#include "nwy_osi_api.h"
#include "nwy_sim.h"
#define NWY_MULTI_SOCKET_MAX     2
#define NWY_MULTI_DATA_MAX     2

extern char nwy_ext_sio_recv_buff[2*1024 + 1];
extern void nwy_data_cb_fun(int hndl, nwy_data_call_state_t ind_state);
extern void nwy_ext_input_gets(char* msg, ...);
extern void nwy_ext_echo(char* fmt, ...);
extern int ipaddr_aton(const char *cp, ip_addr_t *addr);
typedef struct {
    int hndl;
    int call_state;
    int profile_id;
}nwy_multi_call_info;

static nwy_multi_call_info s_nwy_multi_call[NWY_MULTI_DATA_MAX] = {0};
static int s_nwy_multi_socket[NWY_MULTI_SOCKET_MAX] = {0};

void nwy_multi_main_menu()
{
    nwy_ext_echo("\r\n1. create  data call\r\n");
    nwy_ext_echo("2. multi socket test \r\n");
    nwy_ext_echo("3. exit \r\n");
}

void nwy_start_multi_call()
{
    int ret = 0;
    nwy_data_start_call_v02_t param_1;
    nwy_data_start_call_v02_t param_2;
    OSI_LOGI(0, "nwy_start_multi_call start");
    s_nwy_multi_call[0].hndl = nwy_data_get_srv_handle(nwy_data_cb_fun);
    s_nwy_multi_call[1].hndl = nwy_data_get_srv_handle(nwy_data_cb_fun);

    if (s_nwy_multi_call[0].hndl <= 0 || s_nwy_multi_call[1].hndl <=0) {
        return;
    }

    memset(&param_1, 0, sizeof(param_1));
    memset(&param_1, 0, sizeof(param_2));

    param_1.profile_idx = 1;
    param_1.auto_interval_ms = 100;
    param_1.auto_re_max = 100;
    param_1.is_auto_recon = 1;

    param_2.profile_idx = 2;
    param_2.auto_interval_ms = 100;
    param_2.auto_re_max = 100;
    param_2.is_auto_recon = 1;

    ret = nwy_data_start_call(s_nwy_multi_call[0].hndl,&param_1);
    if (ret < 0) {
        OSI_LOGI(0, "nwy_multi_socket_test hndl_1 call fail");
        return;
    }


    ret = nwy_data_start_call(s_nwy_multi_call[1].hndl,&param_2);
    if (ret < 0) {
        OSI_LOGI(0, "nwy_multi_socket_test hndl_2 call fail");
        return;
    }

    s_nwy_multi_call[0].profile_id = 1;
    s_nwy_multi_call[1].profile_id = 2;
    OSI_LOGI(0, "nwy_start_multi_call end");

    nwy_sleep(1000);
}

void nwy_realse_multi_call()
{
    int i = 0;
    for (i = 0; i< 2; i++) {
        nwy_data_stop_call(s_nwy_multi_call[i].hndl);
        nwy_sleep(1000);
        nwy_data_relealse_srv_handle(s_nwy_multi_call[i].hndl);
    }

    memset(&s_nwy_multi_call, 0, 2*sizeof(nwy_multi_call_info));
}


static void nwy_multi_socket_destory()
{
    int i = 0;
    for (i = 0; i < NWY_MULTI_SOCKET_MAX; i++) {
        if (s_nwy_multi_socket[i] > 0) {
            nwy_socket_close(s_nwy_multi_socket[i]);
            s_nwy_multi_socket[i] = 0;
        }
    }
}


static int nwy_multi_socket_connect( int socket, struct sockaddr_in *sa)
{
    uint64_t start ;
    int ret = NWY_GEN_E_INVALID_PARA;
    start = nwy_get_ms();
    do
    {
    if((nwy_get_ms()-start)>= 10000 )
    {
        nwy_ext_echo("\r\nsocket connect timeout\r\n");
        break;
    }
    ret = nwy_socket_connect(socket, (struct sockaddr *)sa, sizeof(struct sockaddr));
    if(ret != NWY_SUCESS)
    {
        if( EISCONN == nwy_socket_errno() )
        {
            nwy_ext_echo( "\r\nnwy_net_connect_tcp connect ok..");
            return 0;
        }
        if (EINPROGRESS != nwy_socket_errno() && EALREADY != nwy_socket_errno() )
        {
            nwy_ext_echo("\r\nconnect errno = %d", nwy_socket_errno());
            break;
        }
    }
        nwy_sleep(1000);
    }while(1);

    return NWY_GEN_E_UNKNOWN;
}


void nwy_multi_socket_communite()
{
    int simid = 0;
    int len = 0;
    int ret = 0;
    int port = 0;
    char* sptr = nwy_ext_sio_recv_buff;
    char ip_str[64] = {0};
    ip_addr_t addr = {0};
    struct sockaddr_in sa;
    struct sockaddr_in *to4 = (struct sockaddr_in *)&sa;

    nwy_ext_input_gets("\r\nPlease input url/IP: ");
    memcpy(ip_str, sptr, strlen(sptr));

    nwy_ext_input_gets("\r\nPlease input port(1-65535): ");

    port = atoi(sptr);

    s_nwy_multi_socket[0]= nwy_socket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    s_nwy_multi_socket[1]= nwy_socket_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s_nwy_multi_socket[0] <= 0 || s_nwy_multi_socket[1] <= 0) {
        OSI_LOGI(0, "nwy_multi_socket_communite: create socket fail");
        nwy_multi_socket_destory();
        return;
    }

    simid = nwy_sim_get_simid();
    nwy_socket_bind_netif(s_nwy_multi_socket[0], simid, s_nwy_multi_call[0].profile_id);
    nwy_socket_bind_netif(s_nwy_multi_socket[1], simid, s_nwy_multi_call[1].profile_id);

    if (ipaddr_aton(ip_str, &addr) == 0) {
        nwy_ext_echo("\r\nnwy ipaddr_aton fail");
        nwy_multi_socket_destory();
        return ;
    }

    to4->sin_len = sizeof(struct sockaddr_in);
    to4->sin_family = AF_INET;
    to4->sin_port = htons(port);
    inet_addr_from_ip4addr(&to4->sin_addr, ip_2_ip4(&addr));

    ret = nwy_multi_socket_connect(s_nwy_multi_socket[0], to4);
    if (ret < 0 ) {
        nwy_ext_echo("\r\nnwy_multi_socket_connect1 fail");
        nwy_multi_socket_destory();
        return ;
    }
    ret = nwy_multi_socket_connect(s_nwy_multi_socket[1], to4);
    if (ret < 0 ) {
        nwy_ext_echo("\r\nnwy_multi_socket_connect2 fail");
        nwy_multi_socket_destory();
        return ;
    }

    len = nwy_socket_send(s_nwy_multi_socket[0], "I'm soket1", strlen("I'm soket1"), 0);
    if(len != strlen("I'm soket1"))
        nwy_ext_echo("\r\nsend len=%d, return len=%d", strlen("I'm soket1"),len);
    else
        nwy_ext_echo("\r\nsend ok");

    len = nwy_socket_send(s_nwy_multi_socket[1], "I'm soket2", strlen("I'm soket2"), 0);
    if(len != strlen("I'm soket2"))
        nwy_ext_echo("\r\nsend len=%d, return len=%d", strlen("I'm soket2"),len);
    else
        nwy_ext_echo("\r\nsend ok");

    nwy_multi_socket_destory();
}


void nwy_multi_socket_test()
{
    char* sptr = nwy_ext_sio_recv_buff;

    while(1) {
        nwy_multi_main_menu();
        nwy_ext_input_gets("\r\nPlease input : ");

        switch(atoi(sptr)) {
            case 1:
                nwy_start_multi_call();
                break;
            case 2:
                nwy_multi_socket_communite();
                break;
            case 3:
                nwy_realse_multi_call();
                return;
            default:
                break;
        }
    }
}

