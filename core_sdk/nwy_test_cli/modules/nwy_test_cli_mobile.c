#include "nwy_test_cli_utils.h"
#include "nwy_sim.h"
#include "nwy_sms.h"
#include "nwy_ussd.h"
#include "osi_log.h"
#include "stdio.h"
#include "ctype.h"
#include "nwy_osi_api.h"
#include "nwy_network.h"
#include "nwy_data.h"
#include "nwy_voice.h"
/**************************SIM*********************************/
void nwy_test_cli_get_sim_status()
{
    nwy_sim_status sim_status = NWY_SIM_STATUS_GET_ERROR;
    sim_status = nwy_sim_get_card_status();
    switch (sim_status)
    {
    case NWY_SIM_STATUS_READY:
        nwy_test_cli_echo("\r\nNWY_SIM_STATUS_READY!\r\n");
        break;
    case NWY_SIM_STATUS_NOT_INSERT:
        nwy_test_cli_echo("\r\nNWY_SIM_STATUS_NOT_INSERT!\r\n");
        break;
    case NWY_SIM_STATUS_PIN1:
        nwy_test_cli_echo("\r\nNWY_SIM_STATUS_PIN1!\r\n");
        break;
    case NWY_SIM_STATUS_PUK1:
        nwy_test_cli_echo("\r\nNWY_SIM_STATUS_PUK1!\r\n");
        break;
    case NWY_SIM_STATUS_BUSY:
        nwy_test_cli_echo("\r\nNWY_SIM_STATUS_BUSY!\r\n");
        break;
    default:
        nwy_test_cli_echo("\r\nNWY_SIM_STATUS_GET_ERROR!\r\n");
        break;
    }

    return;
}

void nwy_test_cli_verify_pin()
{
    char *sptr = NULL;
    char pin[10] = {0};
    nwy_sim_id_t simid;
    int ret = 0;

    sptr = nwy_test_cli_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);

    sptr = nwy_test_cli_input_gets("\r\nPlease input pin string: ");
    memcpy(pin, sptr, strlen(sptr));

    ret = nwy_sim_verify_pin(simid, pin);
    if (0 != ret)
        nwy_test_cli_echo("\r\nnwy verify pin fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy verify pin success!\r\n");

    return;
}

void nwy_test_cli_get_pin_mode()
{
    nwy_result_type result = NWY_RES_ERROR;
    nwy_sim_result_type sim = {"", "", "", "", NWY_SIM_AUTH_NULL};
    result = nwy_sim_get_pin_mode(&sim);
    if (NWY_RES_OK == result)
        nwy_test_cli_echo("\r\npin_mode: %d\r\n", sim.pin_mode);
    else
        nwy_test_cli_echo("\r\nget pin mode fail!\r\n");

    return;
}

void nwy_test_cli_set_pin_mode()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_change_pin()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_verify_puk()
{
    char *sptr = NULL;
    char puk[10] = {0};
    char new_pin[10] = {0};
    nwy_sim_id_t simid;
    int ret = 0;

    sptr = nwy_test_cli_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);

    sptr = nwy_test_cli_input_gets("\r\nPlease input puk string: ");
    memcpy(puk, sptr, strlen(sptr));

    sptr = nwy_test_cli_input_gets("\r\nPlease input new pin string: ");
    memcpy(new_pin, sptr, strlen(sptr));

    ret = nwy_sim_unblock(simid, puk, new_pin);
    if (0 != ret)
        nwy_test_cli_echo("\r\nnwy verify puk fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy verify puk success!\r\n");

    return;
}

void nwy_test_cli_get_imsi()
{
    nwy_result_type result = NWY_RES_ERROR;
    nwy_sim_result_type sim = {"", "", "", "", NWY_SIM_AUTH_NULL};
    result = nwy_sim_get_imsi(&sim);
    if (NWY_RES_OK == result)
        nwy_test_cli_echo("\r\nimsi: %s\r\n", sim.imsi);
    else
        nwy_test_cli_echo("\r\nget imsi fail!!\r\n");

    return;
}

void nwy_test_cli_get_iccid()
{
    nwy_result_type result = NWY_RES_ERROR;
    nwy_sim_result_type sim = {"", "", "", "", NWY_SIM_AUTH_NULL};
    result = nwy_sim_get_iccid(&sim);
    if (NWY_RES_OK == result)
    {
        nwy_sleep(1000);
        nwy_test_cli_echo("\r\niccid: %s\r\n", sim.iccid);
    }
    else
        nwy_test_cli_echo("\r\nget sim iccid fail!!\r\n");

    return;
}

void nwy_test_cli_get_msisdn()
{
    char *sptr = NULL;
    char msisdn[128] = {0};
    int ret = -1;
    nwy_sim_id_t simid;

    sptr = nwy_test_cli_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);
    ret = nwy_sim_get_msisdn(simid, msisdn, 128);
    if (0 != ret)
        nwy_test_cli_echo("\r\nget msisdn fail!!\r\n");
    else
        nwy_test_cli_echo("\r\nmsisdn: %s\r\n", msisdn);

    return;
}

void nwy_test_cli_set_msisdn()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_get_sim_slot()
{
    uint8 nSwitchSimID = 0;

    nSwitchSimID = nwy_sim_get_simid();

    nwy_test_cli_echo("\r\n simid: %d \r\n", nSwitchSimID);
    return;
}

void nwy_test_cli_set_sim_slot()
{
    char *sptr = NULL;
    uint8 nSwitchSimID = 0;
    nwy_result_type ret = NWY_RES_OK;

    sptr = nwy_test_cli_input_gets("\r\nPlease input switch simid: ");
    nSwitchSimID = atoi(sptr);

    ret = nwy_sim_set_simid(nSwitchSimID);
    if (NWY_RES_OK != ret)
        nwy_test_cli_echo("\r\nnwy set switch simid fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy set switch simid success!\r\n");

    return;
}

static int ppp_state[10] = {0};
static void nwy_test_cli_data_cb_fun(
    int hndl,
    nwy_data_call_state_t ind_state)
{
    OSI_LOGI(0, "=DATA= hndl=%d,ind_state=%d", hndl, ind_state);
    if (hndl > 0 && hndl <= 8)
    {
        ppp_state[hndl - 1] = ind_state;
        nwy_test_cli_echo("\r\nData call status update, handle_id:%d,state:%d\r\n", hndl, ind_state);
    }
}

int nwy_test_cli_check_data_connect()
{
    int i = 0;
    for (i = 0; i < NWY_DATA_CALL_MAX_NUM; i++)
    {
        if (ppp_state[i] == NWY_DATA_CALL_CONNECTED)
        {
            return 1;
        }
    }
    return 0;
}

/**************************DATA*********************************/
void nwy_test_cli_data_create()
{
    int hndl = 0;
    hndl = nwy_data_get_srv_handle(nwy_test_cli_data_cb_fun);
    if (hndl > 0)
    {
        nwy_test_cli_echo("\r\nCreate a Resource Handle id: %d success\r\n", hndl);
    }
    else
    {
        nwy_test_cli_echo("\r\nCreate a Resource Handle id: %d fail\r\n", hndl);
    }
    OSI_LOGI(0, "=DATA=  hndl= %d", hndl);
}

void nwy_test_cli_get_profile()
{
    int ret = NWY_GEN_E_UNKNOWN;
    char *sptr = NULL;
    nwy_data_profile_info_t profile_info;

    sptr = nwy_test_cli_input_gets("\r\nPlease input profile index: (1-7)");
    int profile_id = atoi(sptr);
    if ((profile_id <= 0) || (profile_id > 7))
    {
        nwy_test_cli_echo("\r\nInvaild profile id: %d\r\n", profile_id);
        return;
    }
    memset(&profile_info, 0, sizeof(nwy_data_profile_info_t));
    ret = nwy_data_get_profile(profile_id, NWY_DATA_PROFILE_3GPP, &profile_info);
    OSI_LOGI(0, "=DATA=  nwy_data_get_profile ret= %d", ret);
    OSI_LOGI(0, "=DATA=  profile= %d|%d", profile_info.pdp_type, profile_info.auth_proto);
    OSI_PRINTFI("=DATA=   profile= %s|%s|%s", profile_info.apn, profile_info.user_name, profile_info.pwd);
    if (ret != NWY_RES_OK)
    {
        nwy_test_cli_echo("\r\nRead profile %d info fail, result%d\r\n", profile_id, ret);
    }
    else
    {
        nwy_test_cli_echo("\r\nProfile %d info: <pdp_type>,<auth_proto>,<apn>,<user_name>,<password>\r\n%d,%d,%s,%s,%s\r\n", profile_id, profile_info.pdp_type,
                          profile_info.auth_proto, profile_info.apn, profile_info.user_name, profile_info.pwd);
    }
}

void nwy_test_cli_set_profile()
{
    int ret = NWY_GEN_E_UNKNOWN;
    char *sptr = NULL;
    nwy_data_profile_info_t profile_info;

    memset(&profile_info, 0, sizeof(nwy_data_profile_info_t));
    sptr = nwy_test_cli_input_gets("\r\nPlease input profile info: profile_id <1-7>");
    int profile_id = atoi(sptr);
    if ((profile_id <= 0) || (profile_id > 7))
    {
        nwy_test_cli_echo("\r\nInvaild profile id: %d\r\n", profile_id);
        return;
    }

    sptr = nwy_test_cli_input_gets("\r\nPlease input profile info: auth_proto <0-2>\r\n0:NONE, 1:PAP, 2:CHAP\r\n");
    int auth_proto = atoi(sptr);
    if ((auth_proto < 0) || (auth_proto > 2))
    {
        nwy_test_cli_echo("\r\nInvaild auth_proto value: %d\r\n", auth_proto);
        return;
    }
    profile_info.auth_proto = auth_proto;

    sptr = nwy_test_cli_input_gets("\r\nPlease input profile info: pdp_type <1-3,6>\r\n1:IPV4, 2:IPV6, 3:IPV4V6, 6:PPP\r\n");
    int pdp_type = atoi(sptr);
    if ((pdp_type != 1) && (pdp_type != 2) && (pdp_type != 3) && (pdp_type != 6))
    {
        nwy_test_cli_echo("\r\nInvaild pdp_type value: %d\r\n", pdp_type);
        return;
    }
    profile_info.pdp_type = pdp_type;

    sptr = nwy_test_cli_input_gets("\r\nPlease input profile info: apn (length 0-%d)\r\n", NWY_APN_MAX_LEN);
    if (strlen(sptr) > NWY_APN_MAX_LEN)
    {
        nwy_test_cli_echo("\r\nInvaild apn len\r\n");
        return;
    }
    memcpy(profile_info.apn, sptr, sizeof(profile_info.apn));

    sptr = nwy_test_cli_input_gets("\r\nPlease input profile info: user name (length 0-%d)\r\n", NWY_APN_USER_MAX_LEN);
    if (strlen(sptr) > NWY_APN_USER_MAX_LEN)
    {
        nwy_test_cli_echo("\r\nInvaild user name len\r\n");
        return;
    }
    memcpy(profile_info.user_name, sptr, sizeof(profile_info.user_name));

    sptr = nwy_test_cli_input_gets("\r\nPlease input profile info: password(length 0-%d)\r\n", NWY_APN_PWD_MAX_LEN);
    if (strlen(sptr) > NWY_APN_PWD_MAX_LEN)
    {
        nwy_test_cli_echo("\r\nInvaild password len\r\n");
        return;
    }
    memcpy(profile_info.pwd, sptr, sizeof(profile_info.pwd));

    ret = nwy_data_set_profile(profile_id, NWY_DATA_PROFILE_3GPP, &profile_info);
    OSI_LOGI(0, "=DATA=  nwy_data_set_profile ret= %d", ret);
    if (ret != NWY_RES_OK)
        nwy_test_cli_echo("\r\nSet profile %d info fail, result<%d>\r\n", profile_id, ret);
    else
        nwy_test_cli_echo("\r\nSet profile %d info success\r\n", profile_id);
}

void nwy_test_cli_data_start()
{
    int ret = NWY_GEN_E_UNKNOWN;
    char *sptr = NULL;
    nwy_data_start_call_v02_t param_t;
    memset(&param_t, 0, sizeof(nwy_data_start_call_v02_t));

    sptr = nwy_test_cli_input_gets("\r\nPlease select resource handle id <1-%d>", NWY_DATA_CALL_MAX_NUM);
    int hndl = atoi(sptr);
    if ((hndl <= 0) || (hndl > NWY_DATA_CALL_MAX_NUM))
    {
        nwy_test_cli_echo("\r\nInvaild resource handle id: %d\r\n", hndl);
        return;
    }

    sptr = nwy_test_cli_input_gets("\r\nPlease select profile: profile_id <1-7>");
    int profile_id = atoi(sptr);
    if ((profile_id <= 0) || (profile_id > 7))
    {
        nwy_test_cli_echo("\r\nInvaild profile id: %d\r\n", profile_id);
        return;
    }
    param_t.profile_idx = profile_id;

    /* Begin: Add by YJJ for support auto re_connect in 2020.05.20*/
    sptr = nwy_test_cli_input_gets("\r\nPlease set auto_connect: 0 Disable, 1 Enable");
    int is_auto_recon = atoi(sptr);
    if ((is_auto_recon != 0) && (is_auto_recon != 1))
    {
        nwy_test_cli_echo("\r\nInvaild auto_connect: %d\r\n", is_auto_recon);
        return;
    }
    param_t.is_auto_recon = is_auto_recon;

    if (is_auto_recon == 1)
    {
        sptr = nwy_test_cli_input_gets("\r\nPlease set auto_connect maximum times: 0 Always Re_connect, [1-65535] maximum_times");
        int auto_re_max = atoi(sptr);
        if ((auto_re_max < 0) || (auto_re_max > 65535))
        {
            nwy_test_cli_echo("\r\nInvaild auto_connect maximum times : %d\r\n", auto_re_max);
            return;
        }
        param_t.auto_re_max = auto_re_max;

        sptr = nwy_test_cli_input_gets("\r\nPlease set auto_connect interval (ms): [100,86400000]");
        int auto_interval_ms = atoi(sptr);
        if ((auto_interval_ms < 100) || (auto_interval_ms > 86400000))
        {
            nwy_test_cli_echo("\r\nInvaild auto_connect interval: %d\r\n", auto_interval_ms);
            return;
        }
        param_t.auto_interval_ms = auto_interval_ms;
    }

    /* End: Add by YJJ for auto re_connect in 2020.05.20*/

    ret = nwy_data_start_call(hndl, &param_t);
    OSI_LOGI(0, "=DATA=  nwy_data_start_call ret= %d", ret);
    if (ret != NWY_RES_OK)
        nwy_test_cli_echo("\r\nStart data call fail, result<%d>\r\n", ret);
    else
        nwy_test_cli_echo("\r\nStart data call ...\r\n");
}

void nwy_test_cli_data_info()
{
    char *sptr = NULL;
    int ret = NWY_GEN_E_UNKNOWN;
    int len = 0;
    nwy_data_addr_t_info addr_info;

    sptr = nwy_test_cli_input_gets("\r\nPlease select resource handle id <1-%d>", NWY_DATA_CALL_MAX_NUM);
    int hndl = atoi(sptr);
    if ((hndl <= 0) || (hndl > NWY_DATA_CALL_MAX_NUM))
    {
        nwy_test_cli_echo("\r\nInvaild resource handle id: %d\r\n", hndl);
        return;
    }

    memset(&addr_info, 0, sizeof(nwy_data_addr_t_info));
    OSI_LOGI(0, "=DATA=  addr_info size= %d", sizeof(nwy_data_addr_t_info));
    ret = nwy_data_get_ip_addr(hndl, &addr_info, &len);
    OSI_LOGI(0, "=DATA=  nwy_data_get_ip_addr = %d|len%d", ret, len);
    if (ret != NWY_RES_OK)
    {
        nwy_test_cli_echo("\r\nGet data info fail, result<%d>\r\n", ret);
        return;
    }
    nwy_test_cli_echo("\r\nGet data info success\r\nIface address: %s,%s\r\n",
                      nwy_ip4addr_ntoa(&addr_info.iface_addr_s.ip_addr),
                      nwy_ip6addr_ntoa(&addr_info.iface_addr_s.ip6_addr));
    nwy_test_cli_echo("Dnsp address: %s,%s\r\n",
                      nwy_ip4addr_ntoa(&addr_info.dnsp_addr_s.ip_addr),
                      nwy_ip6addr_ntoa(&addr_info.dnsp_addr_s.ip6_addr));
    nwy_test_cli_echo("Dnss address: %s,%s\r\n",
                      nwy_ip4addr_ntoa(&addr_info.dnss_addr_s.ip_addr),
                      nwy_ip6addr_ntoa(&addr_info.dnss_addr_s.ip6_addr));
}

void nwy_test_cli_data_stop()
{
    char *sptr = NULL;
    int ret = NWY_GEN_E_UNKNOWN;

    sptr = nwy_test_cli_input_gets("\r\nPlease select resource handle id <1-%d>", NWY_DATA_CALL_MAX_NUM);
    int hndl = atoi(sptr);
    if ((hndl <= 0) || (hndl > NWY_DATA_CALL_MAX_NUM))
    {
        nwy_test_cli_echo("\r\nInvaild resource handle id: %d\r\n", hndl);
        return;
    }
    ret = nwy_data_stop_call(hndl);
    OSI_LOGI(0, "=DATA=  nwy_data_stop_call ret= %d", ret);
    if (ret != NWY_RES_OK)
        nwy_test_cli_echo("\r\nStop data call fail, result<%d>\r\n", ret);
    else
        nwy_test_cli_echo("\r\nStop data call ...\r\n");
}

void nwy_test_cli_data_release()
{
    char *sptr = NULL;

    sptr = nwy_test_cli_input_gets("\r\nPlease select resource handle id <1-%d>", NWY_DATA_CALL_MAX_NUM);
    int hndl = atoi(sptr);
    if ((hndl < 0) || (hndl > NWY_DATA_CALL_MAX_NUM))
    {
        nwy_test_cli_echo("\r\nInvaild resource handle id: %d\r\n", hndl);
        return;
    }

    nwy_data_relealse_srv_handle(hndl);
    OSI_LOGI(0, "=DATA=  nwy_data_relealse_srv_handle hndl= %d", hndl);
    nwy_test_cli_echo("\r\nRelease resource handle id %d\r\n", hndl);
}

/**************************NW*********************************/
static int scan_finish = 0;
static nwy_nw_net_scan_list_t nw_scan_list;
#define TBL_SIZE(tbl) (sizeof(tbl) / sizeof(tbl[0]))

static const byte cmcc_plmn[][6] =
    {
        "46000",
        "46002",
        "46004",
        "46007",
        "46008",
        "46013",
        "46020",
        "45412",
        "45413",
};
static const byte unicom_plmn[][6] =
    {
        "46001",
        "46006",
        "46009",
        "46010",
};
static const byte ct_plmn[][6] =
    {
        "46003",
        "46005",
        "46011",
        "46012",
        "46059",
        "45502",
        "45507",
};

void nwy_test_cli_nw_get_mode()
{
    /*int network_mode = 0; */
    nwy_nw_mode_type_t network_mode = 0; /*update by gaohe for Eliminate the warning in 2021/12/1*/
    if (NWY_SUCCESS == nwy_nw_get_network_mode(&network_mode))
    {
        if (network_mode == NWY_NW_MODE_GSM)
            nwy_test_cli_echo("\r\nCurrent Network mode: GSM (%d)\r\n", network_mode);
        if (network_mode == NWY_NW_MODE_LTE)
            nwy_test_cli_echo("\r\nCurrent Network mode: LTE (%d)\r\n", network_mode);
    }
    else
    {
        nwy_test_cli_echo("\r\nGet Network mode Failed!!!\r\n");
    }
}

void nwy_test_cli_nw_set_mode()
{
    char *sub_opt = NULL;
    int mode = 0;
    sub_opt = nwy_test_cli_input_gets("\r\nPlease select network mode (0 - AUTO,2 - GSM,4 - LTE):");
    mode = atoi(sub_opt);
    if ((mode == 0) || (mode == 2) || (mode == 4))
    {
        nwy_nw_set_network_mode(mode);
        nwy_test_cli_echo("\r\nSet network mode over\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\nInput Wrong network mode!!!\r\n");
    }
}

void nwy_test_cli_nw_get_fplmn()
{
    nwy_nw_fplmn_list_t fplmn_list = {0};
    int i = 0;
    memset(&fplmn_list, 0x00, sizeof(fplmn_list));
    if (NWY_SUCCESS == nwy_nw_get_forbidden_plmn(&fplmn_list))
    {
        nwy_test_cli_echo("\r\nGet FPLMN list %d", fplmn_list.num);
        for (i = 0; i < fplmn_list.num; i++)
        {
            nwy_test_cli_echo("\r\n%s - %s", fplmn_list.fplmn[i].mcc, fplmn_list.fplmn[i].mnc);
        }
        nwy_test_cli_echo("\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\nGet FPLMN List Failed!!!\r\n");
    }
}

/*
@func
    nwy_network_util_get_oper_type
@desc
    get current operator type from plmn string
@param
    plmn_buf: input plmn string
@return
    0 : CMCC
    1 : CU
    2 : CT
    3 : OTHER
@other
*/
static int nwy_network_util_get_oper_type(char *plmn_buf)
{
    int i;
    if (plmn_buf == NULL)
        return 3;
    for (i = 0; i < TBL_SIZE(cmcc_plmn); i++)
    {
        if (strncmp(plmn_buf, (char *)cmcc_plmn[i], strlen((char *)cmcc_plmn[i])) == 0)
        {
            return 0;
        }
    }

    for (i = 0; i < TBL_SIZE(unicom_plmn); i++)
    {
        if (strncmp(plmn_buf, (char *)unicom_plmn[i], strlen((char *)unicom_plmn[i])) == 0)
        {
            return 1;
        }
    }

    for (i = 0; i < TBL_SIZE(ct_plmn); i++)
    {
        if (strncmp(plmn_buf, (char *)ct_plmn[i], strlen((char *)ct_plmn[i])) == 0)
        {
            return 2;
        }
    }
    return 3;
}

/*
@func
    nwy_network_test_scan_cb
@desc
    network manual scan callback func
    call by nwy_nw_manual_network_scan()
@param
    net_list: output network scan result point
@return
    NONE
@other
*/
static void nwy_network_test_scan_cb(
    nwy_nw_net_scan_list_t *net_list)
{
    int i;
    if (net_list == NULL)
        return;
    if (net_list->result != 1)
        nwy_test_cli_echo("\r\nManual network Scan failed\r\n");
    else
    {
        nwy_test_cli_echo("\r\nManual Scan Result: %d", net_list->num);
        for (i = 0; i < net_list->num; i++)
        {
            nwy_test_cli_echo("\r\nIndex: %d\r\n", i);
            nwy_test_cli_echo("MCC-MNC:%s-%s\r\n", net_list->net_list[i].net_name.mcc, net_list->net_list[i].net_name.mnc);
            nwy_test_cli_echo("Long EONS:%s\r\n", net_list->net_list[i].net_name.long_eons);
            nwy_test_cli_echo("Short EONS:%s\r\n", net_list->net_list[i].net_name.short_eons);
            nwy_test_cli_echo("Net Status:%s\r\n", scan_net_status[net_list->net_list[i].net_status]);
            nwy_test_cli_echo("Net Rat:%s\r\n", scan_net_rat[net_list->net_list[i].net_rat]);
        }
    }
    nwy_test_cli_echo("\r\nStart Test Select Network...\r\n");
    memset(&nw_scan_list, 0x00, sizeof(nw_scan_list));
    memcpy(&nw_scan_list, net_list, sizeof(nwy_nw_net_scan_list_t));
    scan_finish = 1;
}

/*
@func
    nwy_network_test_nw_select
@desc
    network manual select function.
@param
    NONE
@return
    NONE
@other
    User can only choose insert SIM/USIM operator's network,
    and if choose not support network will cause UE crash.
*/
void nwy_network_test_nw_select()
{
    nwy_sim_result_type imsi = {0};
    char plmn_str[10] = "";
    int i = 0;
    int opertor_type = 3;
    char *sub_opt = NULL;
    int id = 0;
    nwy_nw_net_select_param_t param = {0};
    // 1. Get current sim imsi for operator type
    if (nwy_sim_get_imsi(&imsi) == NWY_RES_OK)
    {
        nwy_sleep(1000); //wait for imsi result
        //1.1 Phase imsi to Operator Type.
        opertor_type = nwy_network_util_get_oper_type((char *)imsi.imsi);
        nwy_test_cli_echo("\r\nOperator type: %d(0-CMCC,1-CU,2-CT,3-OTHER),Current netwrok list: ", opertor_type);
        //2. Get scan result's operator type and compare with current operator type.
        for (i = 0; i < nw_scan_list.num; i++)
        {
            memset(plmn_str, 0x00, sizeof(plmn_str));
            sprintf(plmn_str, "%s%s", nw_scan_list.net_list[i].net_name.mcc,
                    nw_scan_list.net_list[i].net_name.mnc);
            //2.1 only show user available select netwrok list.
            if (nwy_network_util_get_oper_type(plmn_str) == opertor_type)
            {
                nwy_test_cli_echo("\r\nIndex:%d Rat:%d ", i, nw_scan_list.net_list[i].net_rat);
                nwy_test_cli_echo("MCC-MNC:%s-%s", nw_scan_list.net_list[i].net_name.mcc, nw_scan_list.net_list[i].net_name.mnc);
            }
        }
        sub_opt = nwy_test_cli_input_gets("\r\nPlease select index at before table:");
        id = atoi(sub_opt);
        memcpy(param.mcc, nw_scan_list.net_list[id].net_name.mcc, sizeof(param.mcc));
        memcpy(param.mnc, nw_scan_list.net_list[id].net_name.mnc, sizeof(param.mnc));
        param.net_rat = nw_scan_list.net_list[id].net_rat;
        //3. start manual select network.
        nwy_nw_manual_network_select(&param);
        nwy_test_cli_echo("\r\nSelect index %d network over!!!", id);
    }
    else
    {
        nwy_test_cli_echo("\r\nGet Current operator type FAILED!!!");
    }
    return;
}

void nwy_test_cli_nw_manual_scan()
{
    int count = 0;
    if (NWY_SUCCESS == nwy_nw_manual_network_scan(nwy_network_test_scan_cb))
    {
        nwy_test_cli_echo("\r\nWaiting Scan result...");
        scan_finish = 0;
        while (1)
        {
            if (scan_finish == 1 || count == 180) //limit 3 minute
                break;
            nwy_sleep(1000);
            count++;
        }
        if (nw_scan_list.result == true)
        {
            nwy_network_test_nw_select();
        }
        else
            nwy_test_cli_echo("\r\nScan network Failed!!!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\nScan network Failed!!!\r\n");
    }
}

void nwy_test_cli_nw_band_lock()
{
    char *sub_opt = NULL;
    int act = 0;
    char set_band[64] = {0};

    //1. Get rat mode
    sub_opt = nwy_test_cli_input_gets("\r\nPlease set rat mode(2-GSM,4-LTE): ");
    act = atoi(sub_opt);

    //2. Get bandlock mask
    sub_opt = nwy_test_cli_input_gets("\r\nPlease set bandlock mask(limit 256bits): ");
    memset(set_band, 0, sizeof(set_band));
    memcpy(set_band, sub_opt, strlen((char *)sub_opt));

    //3. Set BADNLOCK
    nwy_nw_band_lock(act, set_band);
}

void nwy_test_cli_nw_freq_lock()
{
    char *sub_opt = NULL;
    int freq_num = 0;
    uint32_t nfreq[9] = {0};
    int i = 0;
    //get num of freq
    sub_opt = nwy_test_cli_input_gets("\r\nPlease enter freq count(max,9):");
    freq_num = atoi(sub_opt);
    if (freq_num > 9 || freq_num < 0)
    {
        nwy_test_cli_echo("\r\ninput number of freq is error!\r\n");
        return;
    }

    for (i = 0; i < freq_num; i++)
    {
        /*memset(sub_opt, 0, sizeof(sub_opt));*/
        memset(sub_opt, 0, strlen(sub_opt)); /*update by gaohe for Eliminate the warning in 2021/12/1*/
        sub_opt = nwy_test_cli_input_gets("\r\nPlease enter freq number(max,65535): ");
        if (65535 > atoi(sub_opt))
            nfreq[i] = atoi(sub_opt);
        else
            nfreq[i] = 65535;
    }
    if (NWY_SUCCESS == nwy_nw_freq_lock(nfreq,freq_num))
        nwy_test_cli_echo("\r\nlock freq success\r\n");
    else
        nwy_test_cli_echo("\r\nlock freq fail\r\n");
}

void nwy_test_cli_nw_get_ims_st()
{
    /*int ims_enable = 0;*/
    uint8_t ims_enable = 0; /*update by gaohe for Eliminate the warning in 2021/12/1*/
    nwy_nw_get_IMS_state(&ims_enable);
    nwy_test_cli_echo("\r\nIMS state %d", ims_enable);
}

void nwy_test_cli_nw_set_ims_st()
{
    char *sub_opt = NULL;
    sub_opt = nwy_test_cli_input_gets("\r\nPlease set IMS state(0-off,1-on):");
    //1. before set IMS open  need set LTE only mode first.
    if (atoi(sub_opt) == 1)
    {
        nwy_nw_set_network_mode(4);
        nwy_sleep(2000);
    }
    //2. set IMS state
    uint8_t sub_switch = atoi(sub_opt);
    if (NWY_SUCCESS == nwy_nw_set_IMS_state(sub_switch)) /*update by gaohe for Eliminate the warning in 2021/12/1*/
    {
        nwy_test_cli_echo("\r\nIMS SET Success!!!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\nIMS SET Failed!!!\r\n");
    }
}

void nwy_test_cli_nw_get_def_pdn()
{
    char *apn = nwy_nw_get_default_pdn_apn();
    if (apn == NULL)
        nwy_test_cli_echo("\r\nGet Default Pdn Info Failed!!\r\n");
    else
        nwy_test_cli_echo("\r\nDefault Pdn Apn:%s\r\n", apn);
}

void nwy_test_cli_nw_set_def_pdn()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_nw_get_radio_st()
{
    int nFM = 0;
    if (nwy_nw_get_radio_st(&nFM) != 0)
    {
        nwy_test_cli_echo("\r\nget radio state error!\r\n");
        return;
    }

    nwy_test_cli_echo("\r\nradio state:%d\r\n", nFM);
}

void nwy_test_cli_nw_set_radio_st()
{
    char *sub_opt = NULL;

    sub_opt = nwy_test_cli_input_gets("\r\nPlease set radio state(0-off,1-on):");
    int fun = atoi(sub_opt);
    if (nwy_nw_set_radio_st(fun) == 0)
        nwy_test_cli_echo("\r\nSet radio state OK!\r\n");
    else
        nwy_test_cli_echo("\r\nSet radio state error!\r\n");
}

void nwy_test_cli_nw_get_radio_sign()
{
    /*int csq_val = 0;*/
    uint8_t csq_val = 0; /*update by gaohe for Eliminate the warning in 2021/12/1*/
    nwy_nw_get_signal_csq(&csq_val);
    nwy_test_cli_echo("\r\nCSQ is %d \r\n", csq_val);
}

void nwy_test_cli_nw_cs_st()
{
    /*uint8 status = 0;*/
    int status = 0; /*update by gaohe for Eliminate the warning in 2021/12/1*/

    if (nwy_nw_get_cs_st(&status) == 0)
    {
        switch (status)
        {
        case 0:
            nwy_test_cli_echo("\r\n CS STATE: %d (not register)\r\n", status);
            break;
        case 1:
            nwy_test_cli_echo("\r\n CS STATE: %d (register,homework)\r\n", status);
            break;
        case 2:
            nwy_test_cli_echo("\r\n CS STATE: %d (searching)\r\n", status);
            break;
        case 3:
            nwy_test_cli_echo("\r\n CS STATE: %d (register denied)\r\n", status);
            break;
        case 4:
            nwy_test_cli_echo("\r\n CS STATE: %d (unknow)\r\n", status);
            break;
        case 5:
            nwy_test_cli_echo("\r\n CS STATE: %d (register roaming)\r\n", status);
            break;
        case 6:
            nwy_test_cli_echo("\r\n CS STATE: %d (register for \"SMS only\",home network)\r\n", status);
            break;
        case 7:
            nwy_test_cli_echo("\r\n CS STATE: %d (register for \"SMS only\",roaming)\r\n", status);
            break;
        default:
            nwy_test_cli_echo("\r\n CS STATE: %d (unknow state)\r\n", status);
            break;
        }
    }
    else
        nwy_test_cli_echo("\r\n CS STATE ERROR!\r\n");
    return;
}

void nwy_test_cli_nw_ps_st()
{
    /*uint8 status = 0;*/
    int status = 0; /*update by gaohe for Eliminate the warning in 2021/12/1*/

    if (nwy_nw_get_ps_st(&status) == 0)
    {
        switch (status)
        {
        case 0:
            nwy_test_cli_echo("\r\n PS STATE: %d (not register)\r\n", status);
            break;
        case 1:
            nwy_test_cli_echo("\r\n PS STATE: %d (register,homework)\r\n", status);
            break;
        case 2:
            nwy_test_cli_echo("\r\n PS STATE: %d (searching)\r\n", status);
            break;
        case 3:
            nwy_test_cli_echo("\r\n PS STATE: %d (register denied)\r\n", status);
            break;
        case 4:
            nwy_test_cli_echo("\r\n PS STATE: %d (unknow)\r\n", status);
            break;
        case 5:
            nwy_test_cli_echo("\r\n PS STATE: %d (register roaming)\r\n", status);
            break;
        case 6:
            nwy_test_cli_echo("\r\n PS STATE: %d (register for \"SMS only\",home network)\r\n", status);
            break;
        case 7:
            nwy_test_cli_echo("\r\n PS STATE: %d (register for \"SMS only\",roaming)\r\n", status);
            break;
        default:
            nwy_test_cli_echo("\r\n PS STATE: %d (unknow state)\r\n", status);
            break;
        }
    }
    else
        nwy_test_cli_echo("\r\n PS STATE ERROR!\r\n");
    return;
}

void nwy_test_cli_nw_lte_st()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_nw_operator_info()
{
    nwy_nw_regs_info_type_t reg_info = {0};
    nwy_nw_operator_name_t opt_name = {0};
    /*int csq_val = 0;*/
    /*update by gaohe for Eliminate the warning in 2021/12/1*/
    uint8_t csq_val = 0;
    int lac = 0;
    int cid = 0;

    memset(&reg_info, 0x00, sizeof(reg_info));
    if (NWY_RES_OK == nwy_nw_get_register_info(&reg_info))
    {
        if (reg_info.data_regs_valid == 1)
        {
            nwy_test_cli_echo("Network Data Reg state: %d\r\n"
                              "Network Data Roam state: %d\r\n"
                              "Network Data Radio Tech: %d\r\n",
                              reg_info.data_regs.regs_state,
                              reg_info.data_regs.roam_state,
                              reg_info.data_regs.radio_tech);
        }
        if (reg_info.voice_regs_valid == 1)
        {
            nwy_test_cli_echo("Network Voice Reg state: %d\r\n"
                              "Network Voice Roam state: %d\r\n"
                              "Network Voice Radio Tech: %d\r\n",
                              reg_info.voice_regs.regs_state,
                              reg_info.voice_regs.roam_state,
                              reg_info.voice_regs.radio_tech);
        }

        // Get operator name
        if (reg_info.voice_regs.regs_state != NWY_NW_SERVICE_NONE)
        {
            memset(&opt_name, 0x00, sizeof(opt_name));
            if (NWY_SUCCESS == nwy_nw_get_operator_name(&opt_name))
            {
                nwy_test_cli_echo("Long EONS: %s\r\n"
                                  "Short EONS: %s\r\n"
                                  "MCC and MNC: %s %s\r\n"
                                  "SPN: %s\r\n",
                                  (char *)opt_name.long_eons,
                                  (char *)opt_name.short_eons,
                                  opt_name.mcc,
                                  opt_name.mnc,
                                  opt_name.spn);
            }
        }

        //Get CSQ
        nwy_nw_get_signal_csq(&csq_val);
        nwy_test_cli_echo("\r\nCSQ is %d \r\n", csq_val);
        //Get Lac and CELL ID
        nwy_sim_get_lacid(&lac, &cid);
        nwy_test_cli_echo("\r\nLAC: %X, CELL_ID: %X \r\n", lac, cid);
    }
    else
    {
        nwy_test_cli_echo("\r\nGet Register Information Failed!!!\r\n");
    }
}

void nwy_test_cli_nw_get_ehplmn()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_nw_get_signal_rssi()
{
    uint8_t rssi = 0;
    if (NWY_SUCCESS == nwy_nw_get_signal_rssi(&rssi))
        nwy_test_cli_echo("\r\nRSSI: %d\r\n", rssi);
    else
        nwy_test_cli_echo("Get RSSI Failed!");
}

void nwy_test_cli_nw_get_netmsg()
{
    nwy_serving_cell_info *pNetmsg = (nwy_serving_cell_info *)malloc(sizeof(nwy_serving_cell_info));
    memset(pNetmsg, 0x00, sizeof(nwy_serving_cell_info));
    if (NWY_SUCCESS == nwy_nw_get_netmsg(pNetmsg))
    {
        int i = 0;
        while (i <= 10)
        {
            if (pNetmsg->is_over == 1)
                break;
            nwy_sleep(200);
            i++;
        }
        if (i > 10)
        {
            nwy_test_cli_echo("\r\nGet NETMSG Timeout!!!\r\n");
            free(pNetmsg);
            pNetmsg = NULL;
            return;
        }
        if (pNetmsg->curr_rat == 4)
            nwy_test_cli_echo("\r\nLTE NETMSG: CELL_ID=%X, PCI=%d, RSRQ=%d, RSRP=%d, SINR=%d, dlBler=%d, ulBler=%d\r\n", pNetmsg->cellId,
                              pNetmsg->pcid, pNetmsg->rsrq, pNetmsg->rsrp, pNetmsg->SINR, pNetmsg->dlBler, pNetmsg->ulBler);
        else if (pNetmsg->curr_rat == 2)
            nwy_test_cli_echo("\r\nGSM NETMSG: CELL_ID=%X, PCI=%d, TxPwr=%d, RxPwr=%d, Bler=%d\r\n", pNetmsg->cellId, pNetmsg->pcid,
                              pNetmsg->txPower, pNetmsg->rxPower, pNetmsg->ulBler);
        else
            nwy_test_cli_echo("\r\nGet NETMSG Failed!!!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\nGet NETMSG Failed!!!\r\n");
    }
    free(pNetmsg);
    pNetmsg = NULL;
}

/**************************VOICE*********************************/
extern void nwy_test_cli_send_virt_at();

void nwy_test_cli_voice_call_start()
{
    char *sptr = NULL;
    int sim_id = 0x00;
    char phone_num[32] = {0};
    int ret = 0;
    sptr = nwy_test_cli_input_gets("\r\nPlease input dest phone number: ");

    memcpy(phone_num, sptr, strlen(sptr));
    if (strlen(phone_num) > 128)
    {
        return;
    }
    nwy_test_cli_echo("\r\n input dest phone number:%s\r\n", phone_num);

    ret = nwy_voice_call_start(sim_id, phone_num);
    if (ret == 0)
    {
        nwy_test_cli_echo("\r\n nwy call success!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy call fail!\r\n");
    }
}

void nwy_test_cli_voice_call_end()
{
    /*char* sptr = NULL;*/ /*update by gaohe for Eliminate the warning in 2021/12/1*/
    int sim_id = 0x00;
    int query_cnt = 0;

    while (query_cnt < 20)
    {
        nwy_voice_call_end(sim_id);
        query_cnt++;
        nwy_sleep(20);
    }
}

void nwy_test_cli_voice_auto_answ()
{
    /*char* sptr = NULL;
  int sim_id = 0x00;*/
    /*update by gaohe for Eliminate the warning in 2021/12/1*/
    int query_cnt = 0;

    while (query_cnt < 50)
    {
        nwy_voice_call_autoanswver();
        query_cnt++;
        nwy_sleep(20);
    }
}

void nwy_test_cli_voice_volte_set()
{
    char *sptr = NULL;
    int sim_id = 0x00;
    char phone_num[32] = {0};
    int setvolte = 0;
    int ret = 0;

    sptr = nwy_test_cli_input_gets("\r\n Please set volte 0-close 1-open:");
    setvolte = atoi(sptr);

    nwy_test_cli_echo("\r\n set volte value :%d", setvolte);

    ret = nwy_voice_setvolte(sim_id, setvolte);

    if (ret == 0)
    {
        nwy_test_cli_echo("\r\n nwy set volte success!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy set volte fail!\r\n");
    }

    sptr = nwy_test_cli_input_gets("\r\nPlease input dest phone number: ");

    memcpy(phone_num, sptr, strlen(sptr));
    if (strlen(phone_num) > 128)
    {
        return;
    }
    nwy_test_cli_echo("\r\n input dest phone number:%s\r\n", phone_num);

    ret = nwy_voice_call_start(sim_id, phone_num);
    if (ret == 0)
    {
        nwy_test_cli_echo("\r\n nwy volte call success!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\n nwy volte call fail!\r\n");
    }
}

void nwy_test_cli_voice_caller_id()
{
    nwy_test_cli_echo("\r\n please open clip at+clip = 1 \r\n");

    nwy_test_cli_send_virt_at();
}

void nwy_test_cli_voice_call_hold()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_voice_call_unhold()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

/**************************SMS*********************************/
void nwy_test_cli_sms_send()
{
    char *sptr = NULL;
    nwy_sms_info_type_t sms_data = {0};
    nwy_result_t ret = NWY_SMS_SUCCESS;

    sptr = nwy_test_cli_input_gets("\r\nPlease input dest phone number: ");
    memcpy(sms_data.phone_num, sptr, strlen(sptr));

    sptr = nwy_test_cli_input_gets("\r\nPlease input msg len: ");
    sms_data.msg_context_len = atoi(sptr);

    sptr = nwy_test_cli_input_gets("\r\nPlease input msg data: ");
    memcpy(sms_data.msg_contxet, sptr, strlen(sptr));

    sptr = nwy_test_cli_input_gets("\r\nPlease input msg format(0:GSM7 2:UNICODE): ");
    sms_data.msg_format = atoi(sptr);

    ret = nwy_sms_send_message(&sms_data);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy send sms fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy send sms success!\r\n");

    return;
}

void nwy_test_cli_sms_del()
{
    char *sptr = NULL;
    uint16_t nindex;
    nwy_sms_storage_type_e nStorage;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    sptr = nwy_test_cli_input_gets("\r\nPlease input sms index: ");
    nindex = atoi(sptr);

    sptr = nwy_test_cli_input_gets("\r\nPlease input sms storage: ");
    nStorage = atoi(sptr);

    ret = nwy_sms_delete_message(nindex, nStorage);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy del sms fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy del sms success!\r\n");

    return;
}

void nwy_test_cli_sms_get_sca()
{
    nwy_sms_result_type sca = {0};
    nwy_result_t ret = NWY_SMS_SUCCESS;

    ret = nwy_sms_get_sca(&sca);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy get sms sca fail!\r\n");
    else
        nwy_test_cli_echo("\r\nsca: %s \r\n", sca.sca);

    return;
}

void nwy_test_cli_sms_set_sca()
{
    char *sptr = NULL;
    char sca[21] = {0};
    unsigned tosca;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    sptr = nwy_test_cli_input_gets("\r\nPlease input sca number: ");
    memcpy(sca, sptr, strlen(sptr));

    sptr = nwy_test_cli_input_gets("\r\nPlease input sca type: ");
    tosca = atoi(sptr);

    ret = nwy_sms_set_sca(sca, tosca);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy set SMS sca fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy set SMS sca success!\r\n");

    return;
}

void nwy_test_cli_sms_set_storage()
{
    char *sptr = NULL;
    nwy_sms_storage_type_e sms_storage;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    sptr = nwy_test_cli_input_gets("\r\nPlease input sms storage(1-ME\2-SM): ");
    sms_storage = atoi(sptr);

    ret = nwy_sms_set_storage(sms_storage);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy set sms storage fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy set sms storage success!\r\n");

    return;
}

void nwy_test_cli_sms_get_storage()
{
    nwy_sms_storage_type_e sms_storage;
    sms_storage = nwy_sms_get_storage();
    nwy_test_cli_echo("\r\nsms_storage: %d", sms_storage);

    return;
}

void nwy_test_cli_sms_set_report_md()
{
    char *sptr = NULL;
    uint8_t mode = 0;
    uint8_t mt = 0;
    uint8_t bm = 0;
    uint8_t ds = 0;
    uint8_t bfr = 0;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    sptr = nwy_test_cli_input_gets("\r\nPlease input sms mode: ");
    mode = atoi(sptr);

    sptr = nwy_test_cli_input_gets("\r\nPlease input sms mt: ");
    mt = atoi(sptr);

    ret = nwy_set_report_option(mode, mt, bm, ds, bfr);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy set sms report mode fail!\r\n");
    else
        nwy_test_cli_echo("\r\nSet sms report mode success!\r\n");

    return;
}

void nwy_test_cli_sms_read()
{
    char *sptr = NULL;
    nwy_sms_recv_info_type_t sms_data = {0};
    unsigned index = 0;
    nwy_result_t ret = NWY_SMS_SUCCESS;

    sptr = nwy_test_cli_input_gets("\r\nPlease input sms index: ");
    index = atoi(sptr);

    ret = nwy_sms_read_message(index, &sms_data);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy read sms fail!\r\n");
    else
    {
        OSI_LOGI(0, "=SMS= finish to read sms\n");
        nwy_sleep(2000); //wait for sms_data
        nwy_test_cli_echo("\r\n read one sms oa:%s\r\nmsg_context:%s", sms_data.oa, sms_data.pData);
    }

    OSI_LOGI(0, "=SMS= end to test read sms\n");

    return;
}
static void nwy_HexStrToByte(const char *source, int sourceLen, unsigned char *dest)
{
    short i;
    unsigned char highByte = 0, lowByte = 0;

    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper((unsigned char)source[i]);
        lowByte = toupper((unsigned char)source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return;
}

void nwy_test_cli_sms_pdu_send()
{
    int ret = 0;
    char *sptr = NULL;
    char pbuf[1024] = {0};
    sptr = nwy_test_cli_input_gets("\r\nPlease input sms pdu data: ");
    nwy_HexStrToByte(sptr, strlen(sptr), (unsigned char *)pbuf); /*update by gaohe for Eliminate the warning in 2021/12/1*/
    ret = nwy_sms_send_pdu_message(pbuf, strlen(sptr) / 2);
    if (NWY_SMS_SUCCESS != ret)
        nwy_test_cli_echo("\r\nnwy send pdu sms fail!\r\n");
    else
        nwy_test_cli_echo("\r\nnwy send pdu sms success!\r\n");

    return;
}

void nwy_test_cli_ussd_send()
{
    char *sptr = NULL;
    uint8 dcs;
    uint8 simid;
    char ussd_string[160] = {0};
    nwy_ussd_info_t ussd_info = {0};
    nwy_ussd_result_t ret = NWY_USSD_SUCCESS;

    memset(ussd_string, 0, strlen(ussd_string));

    sptr = nwy_test_cli_input_gets("\r\nPlease input dcs: ");
    dcs = atoi(sptr);

    sptr = nwy_test_cli_input_gets("\r\nPlease input simid: ");
    simid = atoi(sptr);

    sptr = nwy_test_cli_input_gets("\r\nPlease input ussd string: ");
    memcpy(ussd_string, sptr, strlen(sptr));

    ret = nwy_ussd_send_msg(simid, ussd_string, dcs, &ussd_info);
    if (NWY_USSD_SUCCESS != ret)
    {
        nwy_test_cli_echo("\r\nnwy send ussd fail!\r\n");
    }
    else
    {
        nwy_test_cli_echo("\r\n ussd info Option: %d", ussd_info.nOption);
        nwy_test_cli_echo("\r\n ussd info Dcs: %d", ussd_info.nDcs);
        nwy_test_cli_echo("\r\n ussd info usd: %s", ussd_info.aucDestUsd);
    }

    return;
}
