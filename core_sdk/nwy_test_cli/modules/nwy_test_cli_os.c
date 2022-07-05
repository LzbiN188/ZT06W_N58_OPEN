#include "nwy_test_cli_utils.h"
#include "nwy_dm.h"
#include "nwy_pm.h"
#include "nwy_sim.h"
#include "nwy_voice.h"

#include "nwy_vir_at.h"
#include "nwy_osi_api.h"
#include "nwy_dm.h"
#include "nwy_sms.h"
#include "osi_log.h"

static nwy_osiTimer_t *s_nwy_test_timer = NULL;
static int s_nwy_semaphore_count = 0;
nwy_osiSemaphore_t *s_nwy_cli_semaphore = NULL;
static int s_nwy_at_init_flag = 0;

static void nwy_test_cli_timer_cb(void *ctx)
{
    nwy_test_cli_echo("\r\nnwy_test_cli timer start");
}

static void nwy_cli_semaphore_taskA_proc()
{
    int i = 0;
    nwy_semaphore_acquire(s_nwy_cli_semaphore, 0);
    for (i = 0; i < 100; i++)
    {
        nwy_test_cli_echo("\r\nTaskA semaphore_count = %d", ++s_nwy_semaphore_count);
        nwy_sleep(10);
    }
    nwy_semahpore_release(s_nwy_cli_semaphore);
    nwy_exit_thread();
}
static void nwy_cli_semaphore_taskB_proc()
{
    int i = 0;
    nwy_semaphore_acquire(s_nwy_cli_semaphore, 0);
    for (i = 0; i < 50; i++)
    {
        nwy_test_cli_echo("\r\nTaskB semaphore_count = %d", --s_nwy_semaphore_count);
        nwy_sleep(10);
    }
    nwy_semahpore_release(s_nwy_cli_semaphore);
    nwy_exit_thread();
}

static void nwy_cli_virtual_at_test()
{
    char *sptr = NULL;
    int ret = -1;
    char resp[2048] = {0};
    nwy_at_info at_cmd;
    nwy_sleep(500);

    sptr = nwy_test_cli_input_gets("\r\nSend AT cmd :");

    memset(&at_cmd, 0, sizeof(nwy_at_info));
    memcpy(at_cmd.at_command, sptr, strlen(sptr));
    at_cmd.at_command[strlen(sptr)] = '\r';
    at_cmd.at_command[strlen(sptr) + 1] = '\n';
    at_cmd.length = strlen(at_cmd.at_command);
    ret = nwy_sdk_at_cmd_send(&at_cmd, resp, NWY_AT_TIMEOUT_DEFAULT);
    if (ret == NWY_SUCESS)
    {
        nwy_test_cli_echo("\r\n Resp:%s", resp);
    }
    else if (ret == NWY_AT_GET_RESP_TIMEOUT)
    {
        nwy_test_cli_echo("\r\n AT timeout");
    }
    else
    {
        nwy_test_cli_echo("\r\n AT ERROR");
    }
}

static void nwy_at_cmd_process_callback(void *handle, char *atcmd, int type, char *para0, char *para1, char *para2)
{
    switch (type)
    {
    case 0:
        nwy_at_forward_send(handle, "\r\n", 2);
        nwy_at_forward_send(handle, atcmd, strlen(atcmd));
        if (para0)
        {
            nwy_at_forward_send(handle, para0, strlen(para0));
        }

        if (para1)
        {
            nwy_at_forward_send(handle, para1, strlen(para1));
        }

        if (para2)
        {
            nwy_at_forward_send(handle, para2, strlen(para2));
        }

        nwy_at_forward_send(handle, "\r\nOK\r\n", 6);
        break;
    case 1:
        nwy_at_forward_send(handle, "\r\nTEST\r\n", 8);
        break;
    case 2:
        nwy_at_forward_send(handle, "\r\nREAD\r\n", 8);
        break;
    case 3:
        nwy_at_forward_send(handle, "\r\nEXE\r\n", 7);
        break;
    }
}

static void nwy_cli_pull_out_sim()
{
    nwy_test_cli_echo("\r\n SIM pull out");
}

static void nwy_cli_recv_sms()
{
    nwy_sms_recv_info_type_t sms_data = {0};

    nwy_sms_recv_message(&sms_data);

    if (1 == sms_data.cnmi_mt)
    {
        nwy_test_cli_echo("\r\n recv one sms index:%d", sms_data.nIndex);
    }
    else if (2 == sms_data.cnmi_mt)
    {
        nwy_test_cli_echo("\r\n recv one sms from:%s msg_context:%s", sms_data.oa, sms_data.pData);
    }
}

static void nwy_cli_incom_voice()
{
    char string_clip[128] = {0};
    nwy_get_voice_callerid(string_clip);
    nwy_test_cli_echo("\r\n %s", string_clip);
}

static void nwy_cli_voice_ind()
{
    char state[64] = {0};
    nwy_get_voice_state(state);
    nwy_test_cli_echo("\r\n %s", state);
}
static void nwy_cli_tcpsetup_cb(char *data)
{
    nwy_test_cli_echo("\r\nTCPSETUP: ");
    nwy_test_cli_echo("%s\r\n", data);
}
static void nwy_cli_tcprecvs_cb(char *data)
{
    nwy_test_cli_echo("\r\nTCPRECV(S): ");
    nwy_test_cli_echo("%s\r\n", data);
}
static void nwy_cli_tcp_close_cb(char *data)
{
    nwy_test_cli_echo("\r\nTCPCLOSE:");
    nwy_test_cli_echo("%s\r\n", data);
}

static void nwy_cli_tcprecv_cb(char *data)
{
    nwy_test_cli_echo("\r\nTCPRECV:");
    nwy_test_cli_echo("%s\r\n", data);
}
static void nwy_cli_acpt_close_cb(char *data)
{
    nwy_test_cli_echo("\r\nCLOSECLIENT: ");
    nwy_test_cli_echo("%s\r\n", data);
}

static void nwy_cli_udprecv_cb(char *data)
{
    nwy_test_cli_echo("\r\nUDPRECV:");
    nwy_test_cli_echo("%s\r\n", data);
}
static void nwy_cli_client_acpt_cb(char *data)
{
    nwy_test_cli_echo("\r\nConnect AcceptSocket=");
    nwy_test_cli_echo("%s\r\n", data);
}

static void nwy_cli_gprs_disconnect_cb(char *data)
{
    nwy_test_cli_echo("\r\nGPRS DISCONNECTION");
    nwy_test_cli_echo("%s\r\n", data);
}

static void nwy_cli_sms_list_resp_cb(char *data)
{
    nwy_test_cli_echo("\r\n+CMGL: ");
    nwy_test_cli_echo("%s\r\n", data);
}

static void nwy_cli_init_unsol_reg()
{
    nwy_sdk_at_unsolicited_cb_reg("+EUSIM", nwy_cli_pull_out_sim);
    /* added by wangchen for N58 sms api to test 20200215 begin */
    nwy_sdk_at_unsolicited_cb_reg("+CMT", nwy_cli_recv_sms);
    /* added by wangchen for N58 sms api to test 20200215 end */
    nwy_sdk_at_unsolicited_cb_reg("+CLIP", nwy_cli_incom_voice);
    nwy_sdk_at_unsolicited_cb_reg("+ATD", nwy_cli_voice_ind);
    nwy_sdk_at_unsolicited_cb_reg("Connect AcceptSocket=", nwy_cli_client_acpt_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPRECV(S): ", nwy_cli_tcprecvs_cb);
    nwy_sdk_at_unsolicited_cb_reg("+CLOSECLIENT: ", nwy_cli_acpt_close_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPSETUP: ", nwy_cli_tcpsetup_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPRECV: ", nwy_cli_tcprecv_cb);
    nwy_sdk_at_unsolicited_cb_reg("+TCPCLOSE: ", nwy_cli_tcp_close_cb);
    nwy_sdk_at_unsolicited_cb_reg("+UDPRECV: ", nwy_cli_udprecv_cb);
    nwy_sdk_at_unsolicited_cb_reg("GPRS DISCONNECTION", nwy_cli_gprs_disconnect_cb);
    nwy_sdk_at_unsolicited_cb_reg("+CMGL: ", nwy_cli_sms_list_resp_cb);
}

void nwy_test_cli_get_model()
{
    char buff[128] = {0};

    nwy_dm_get_dev_model(buff, 100);
    nwy_test_cli_echo("\r\nDev model : %s", buff);
}

void nwy_test_cli_get_imei()
{
    nwy_sim_result_type imei = {0};
    nwy_result_type ret = NWY_RES_OK;

    ret = nwy_sim_get_imei(&imei);
    if (NWY_RES_OK != ret)
    {
        nwy_test_cli_echo("\r\n Get IMEI error \r\n");
        return;
    }

    nwy_test_cli_echo("\r\nIMEI:%s \r\n", imei.nImei);
}

void nwy_test_cli_get_chipid()
{
    uint8_t uid[8] = {0};
    nwy_get_chip_id(uid);
    nwy_test_cli_echo("\r\nChip ID : 0x");
    for (int i = 0; i < 8; i++)
    {
        nwy_test_cli_echo("%02x", uid[i]);
    }
    nwy_test_cli_echo("\r\n");
}

void nwy_test_cli_get_boot_cause()
{
    unsigned int causes = nwy_get_boot_causes();
    char string_buf[256];
    memset(string_buf, 0, sizeof(string_buf));
    int idx = sprintf(string_buf, "boot cause[%02x]:", causes);
    if (causes == NWY_BOOTCAUSE_UNKNOWN)
        idx += sprintf(string_buf + idx, " %s", "UNKNOWN");
    else
    {
        if (causes & NWY_BOOTCAUSE_PWRKEY)
            idx += sprintf(string_buf + idx, " %s", "PWRKEY");
        if (causes & NWY_BOOTCAUSE_PIN_RESET)
            idx += sprintf(string_buf + idx, " %s", "PIN_RESET");
        if (causes & NWY_BOOTCAUSE_ALARM)
            idx += sprintf(string_buf + idx, " %s", "ALARM");
        if (causes & NWY_BOOTCAUSE_CHARGE)
            idx += sprintf(string_buf + idx, " %s", "CHARGE");
        if (causes & NWY_BOOTCAUSE_WDG)
            idx += sprintf(string_buf + idx, " %s", "WDG");
        if (causes & NWY_BOOTCAUSE_PIN_WAKEUP)
            idx += sprintf(string_buf + idx, " %s", "PIN_WAKEUP");
        if (causes & NWY_BOOTCAUSE_PSM_WAKEUP)
            idx += sprintf(string_buf + idx, " %s", "PSM_WAKEUP");
    }
    nwy_test_cli_echo("\r\n%s\r\n", string_buf);
}

void nwy_test_cli_get_sw_ver()
{
    char ver[64] = {0};

    nwy_dm_get_inner_version(ver, sizeof(ver));

    nwy_test_cli_echo("\r\nThe sw ver is :%s", ver);
}

void nwy_test_cli_get_hw_ver()
{
    char ver[64] = {0};

    nwy_dm_get_hw_version(ver, sizeof(ver));

    nwy_test_cli_echo("\r\nThe hw ver is :%s", ver);
}

void nwy_test_cli_get_heap_info()
{
    char heapinfo[100] = {0};

    nwy_dm_get_device_heapinfo(heapinfo);
    nwy_test_cli_echo("\r\n dev heapinfo:%s\r\n", heapinfo);
}

void nwy_test_cli_get_cpu_temp()
{
    float value = 0;
    nwy_dm_get_rftemperature(&value);
    nwy_test_cli_echo("\r\n temperature is %.2f\r\n", value);
}

void nwy_test_cli_start_timer()
{
    if (s_nwy_test_timer == NULL)
    {
        s_nwy_test_timer = nwy_timer_init(nwy_get_current_thread(), nwy_test_cli_timer_cb, NULL);
    }
    nwy_start_timer_periodic(s_nwy_test_timer, 1000);
}

void nwy_test_cli_stop_timer()
{
    bool ret = false;
    ret = nwy_stop_timer(s_nwy_test_timer);
    if (ret)
    {
        nwy_test_cli_echo("\r\nStop timer sucess");
    }
    else
    {
        nwy_test_cli_echo("\r\nStop timer fail");
    }
}

void nwy_test_cli_get_time()
{
    nwy_time_t julian_time = {0};
    char timezone = 0;
    nwy_get_time(&julian_time, &timezone);
    nwy_test_cli_echo("\r\n%d-%d-%d %d:%d:%d\r\n", julian_time.year, julian_time.mon, julian_time.day, julian_time.hour, julian_time.min, julian_time.sec);
}

void nwy_test_cli_set_time()
{
    char *sptr = NULL;
    nwy_time_t nwy_time = {2020, 3, 19, 20, 33, 30};

    sptr = nwy_test_cli_input_gets("\r\nPlease input data(2020-1-1): ");
    nwy_time.year = (atoi(strtok(sptr, "-")));
    nwy_time.mon = (char)(atoi(strtok(NULL, "-")));
    nwy_time.day = (char)(atoi(strtok(NULL, "-")));
    OSI_LOGI(0, "nwy_time: year = %d mon = %c day = %c ", nwy_time.year, nwy_time.mon, nwy_time.day);

    sptr = nwy_test_cli_input_gets("\r\nPlease input time(1:1:1): ");

    nwy_time.hour = (atoi(strtok(sptr, ":")));
    nwy_time.min = (char)(atoi(strtok(NULL, ":")));
    nwy_time.sec = (char)(atoi(strtok(NULL, ":")));
    char timezone = 0;
    nwy_set_time(&nwy_time, timezone);

    nwy_test_cli_echo("\r\nset time sucess\r\n");
    nwy_test_cli_get_time();
}

void nwy_test_cli_set_semp()
{
    s_nwy_semaphore_count = 0;
    s_nwy_cli_semaphore = nwy_semaphore_create(1, 1);
    nwy_create_thread("nwy_test_cli_semaphoreA", nwy_cli_semaphore_taskA_proc, NULL, NWY_OSI_PRIORITY_NORMAL + 1, 1024, 4);
    nwy_create_thread("nwy_test_cli_semaphoreB", nwy_cli_semaphore_taskB_proc, NULL, NWY_OSI_PRIORITY_NORMAL, 1024, 4);
}

void nwy_test_cli_send_virt_at()
{
    if (s_nwy_at_init_flag == 0)
    {
        nwy_sdk_at_parameter_init();
        nwy_cli_init_unsol_reg();
        s_nwy_at_init_flag = 1;
    }
    nwy_cli_virtual_at_test();
}

void nwy_test_cli_reg_at_fwd()
{

    nwy_set_at_forward_cb(1, "+FYTEST1", nwy_at_cmd_process_callback);
}
