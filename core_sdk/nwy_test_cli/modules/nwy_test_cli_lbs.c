#include "nwy_test_cli_utils.h"
#include "nwy_loc.h"
#include "osi_log.h"
#include "stdio.h"
#include "ctype.h"
#include "nwy_osi_api.h"
#include "nwy_data.h"

#define NWY_AGPS_URL "www.gnss-aide.com"
#define NWY_AGPS_PORT 2621
#define NWY_AGPS_USER "freetrial"
#define NWY_AGPS_PASS "123456"

extern int nwy_test_cli_check_data_connect();
extern void nwy_test_cli_send_virt_at();

extern void nwy_pdp_set_status(nwy_pdp_status_type status);
/**************************GNSS*********************************/
void nwy_test_cli_gnss_open()
{
    int ret = 0;
    ret = nwy_loc_start_navigation();

    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc open success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc open error!\r\n");
}

void nwy_test_cli_gnss_set_position_md()
{
    char *opt = NULL;
    uint8_t mode = 0;
    int ret = 0;

    opt = nwy_test_cli_input_gets("\r\nPlease input loc mode(1-GPS 3-GPS+BD 5-GPS+GLO 7-GPS+BD+GLO):");
    mode = atoi(opt);
    nwy_test_cli_echo("\r\nTest mode = %d!\r\n", mode);

    nwy_loc_start_navigation();

    ret = nwy_loc_set_position_mode(mode);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc set position success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc set position error!\r\n");
}

void nwy_test_cli_gnss_set_updata_rate()
{
    char *opt = NULL;
    int frequency = 0;
    int ret = 0;

    opt = nwy_test_cli_input_gets("\r\nPlease input loc frequency(1HZ-1000 2HZ-500):");
    frequency = atoi(opt);

    ret = nwy_loc_nmea_format_mode(2, frequency);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc set frequency success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc set frequency error!\r\n");
}

void nwy_test_cli_gnss_set_output_fmt()
{
    char *opt = NULL;
    int opt_format = 0;
    int ret = 0;

    opt = nwy_test_cli_input_gets("\r\nPlease input loc opt_format(128-255):");
    opt_format = atoi(opt);

    ret = nwy_loc_nmea_format_mode(3, opt_format);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc set opt_format success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc set opt_format error!\r\n");
}

void nwy_test_cli_gnss_set_startup_fmt()
{
    char *opt = NULL;
    int startup_mode = 0;
    int ret = 0;

    opt = nwy_test_cli_input_gets("\r\nPlease input loc startup_mode(0-HOT 1-WARM 2-COLD 4-FAC:");
    startup_mode = atoi(opt);

    ret = nwy_loc_set_startup_mode(startup_mode);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc set startup_mode success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc set startup_mode error!\r\n");
}

void nwy_test_cli_gnss_get_nmea()
{
    char nmea_data[2048] = {0};

    nwy_loc_get_nmea_data(nmea_data);

    nwy_test_cli_echo("\r\n nmea data: %s\r\n", nmea_data);
}

void nwy_test_cli_gnss_dialing()
{
    nwy_test_cli_echo("\r\nOption not Supported!\r\n");
}

void nwy_test_cli_gnss_set_server()
{
    char *opt = NULL;
    char AGPS_URL[128] = {0};
    char AGPS_USER[128] = {0};
    char AGPS_PASS[128] = {0};
    int port = 0;
    int ret = 0;

    opt = nwy_test_cli_input_gets("\r\nPlease input agps server url:");
    memcpy(AGPS_URL,opt,strlen(opt));
	
    opt = nwy_test_cli_input_gets("\r\nPlease input agps server port:");
    port = atoi(opt);
	
    opt = nwy_test_cli_input_gets("\r\nPlease input agps server user:");
    memcpy(AGPS_USER,opt,strlen(opt));
	
    opt = nwy_test_cli_input_gets("\r\nPlease input agps server pass:");
    memcpy(AGPS_PASS,opt,strlen(opt));

    ret = nwy_loc_set_server(AGPS_URL, port, AGPS_USER, AGPS_PASS);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc set server success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc set server error!\r\n");
}

void nwy_cipgsmloc_cb(nwy_log_cipgsmloc_result_t *text)
{
    nwy_log_cipgsmloc_result_t *param = (nwy_log_cipgsmloc_result_t *)text;

    if (NULL == param)
        return;

    nwy_test_cli_echo("\r\n cipgsmloc info\r\n");
    if (0 == param->result)
    {
        nwy_test_cli_echo("lat %lf \r\n", param->info.data.lat);
        nwy_test_cli_echo("lng %lf \r\n", param->info.data.lng);
        nwy_test_cli_echo("accuracy %lf \r\n", param->info.data.alt);
    }
    else
    {
        nwy_test_cli_echo(" %s\r\n", param->info.errmsg);
    }

    return;
}

void nwy_wifigsmloc_cb(nwy_log_cipgsmloc_result_t *text)
{
    nwy_log_cipgsmloc_result_t *param = (nwy_log_cipgsmloc_result_t *)text;
    if (NULL == param)
        return;

    if (0 == param->result)
    {
        nwy_test_cli_echo("lat %lf \r\n", param->info.data.lat);
        nwy_test_cli_echo("lng %lf \r\n", param->info.data.lng);
        nwy_test_cli_echo("accuracy %lf \r\n", param->info.data.alt);
    }
    else
        nwy_test_cli_echo(" %s\r\n", param->info.errmsg);

    return;
}

void nwy_test_cli_gnss_open_base()
{
    char *opt = NULL;
    int value = 0;
    int ret = 0;

    opt = nwy_test_cli_input_gets("\r\nPlease open lbs(1-open,0-close):");
    value = atoi(opt);

    if (1 == nwy_test_cli_check_data_connect())
    {
        nwy_pdp_set_status(NWY_PDP_CONNECTED);
    }
    else
    {
        nwy_test_cli_echo("\r\n need module dail at+xiic = 1");
        nwy_test_cli_send_virt_at();
    }

    ret = nwy_loc_cipgsmloc_open(value, nwy_cipgsmloc_cb);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc open lbs success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc open lbs error!\r\n");
}

void nwy_test_cli_wifi_open_base()
{
    char *opt = NULL;
    int value = 0;
    int ret = 0;

    opt = nwy_test_cli_input_gets("\r\nPlease open wifi lbs(1-open,0-close):");
    value = atoi(opt);

    ret = nwy_loc_wifigsmloc_open(value, nwy_wifigsmloc_cb);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest open wifi loc success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest open wifi loc error!\r\n");
}

void nwy_test_cli_gnss_open_assisted()
{
    char *opt = NULL;
    int value = 0;
    int ret = 0;

    if (1 == nwy_test_cli_check_data_connect())
    {
        nwy_pdp_set_status(NWY_PDP_CONNECTED);
    }
    else
    {
        nwy_test_cli_echo("\r\n need module dail at+xiic = 1");
        nwy_test_cli_send_virt_at();
    }

    nwy_loc_cipgsmloc_open(1, nwy_cipgsmloc_cb);

    opt = nwy_test_cli_input_gets("\r\nPlease open A-GPS(1-open,0-close):");
    value = atoi(opt);

    ret = nwy_loc_agps_open(value);
    if (ret > 0)
        nwy_test_cli_echo("\r\nTest open A-GPS success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest open A-GPS error!\r\n");
}

void nwy_test_cli_gnss_close()
{
    int ret = 0;
    ret = nwy_loc_stop_navigation();

    if (ret > 0)
        nwy_test_cli_echo("\r\nTest loc close success!\r\n");
    else
        nwy_test_cli_echo("\r\nTest loc close error!\r\n");
}
/*----------------------------------------*/