#include "nwy_test_cli_adpt.h"
#include "nwy_test_cli_utils.h"
#include "osi_log.h"

extern void nwy_test_cli_menu_display();
extern void nwy_test_cli_menu_back();
extern void nwy_test_cli_menu_select(int opt);

static nwy_osiThread_t *nwy_test_cli_thread = NULL;

void nwy_test_cli_dbg(const char *func, int line, char *fmt, ...)
{
    static char buf[1024];
    va_list args;
    int len = 0;

    memset(buf, 0, sizeof(buf));

    sprintf(buf, "NWY_CLI %s[%d]:", func, line);
    len = strlen(buf);
    va_start(args, fmt);

    vsnprintf(&buf[len], sizeof(buf) - len - 1, fmt, args);
    va_end(args);

    OSI_LOGXI(OSI_LOGPAR_S, 0, "%s\n", buf);
}

int nwy_test_cli_wait_select()
{
    nwy_osiEvent_t event;

    while (1)
    {
        memset(&event, 0, sizeof(event));
        nwy_wait_thead_event(nwy_test_cli_thread, &event, 0);
        if (event.id == NWY_EXT_INPUT_RECV_MSG)
        {
            return 1;
        }
    }
}

void nwy_test_cli_select_enter()
{
    nwy_osiEvent_t event;

    memset(&event, 0, sizeof(event));
    event.id = NWY_EXT_INPUT_RECV_MSG;
    nwy_send_thead_event(nwy_test_cli_thread, &event, 0);
}

static void nwy_test_cli_main_func(void *param)
{
    char *sptr = NULL;
    nwy_usb_serial_reg_recv_cb((nwy_sio_recv_cb_t)nwy_test_cli_sio_data_proc);

    while (1)
    {
        nwy_test_cli_menu_display();
        sptr = nwy_test_cli_input_gets("\r\nPlease input option: ");
        if (sptr[0] == 'q' || sptr[0] == 'Q')
        {
            nwy_test_cli_menu_back();
        }
        else
        {
            nwy_test_cli_menu_select(atoi(sptr));
        }
    }
}

static const char APP_VERSION[65] = "NWY_APP_V1.0.1";
int appimg_enter(void *param)
{
    nwy_sleep(10 * 1000);
	char APP_BUILD_TIME[65]= {0};
    char version[70]={0};

    sprintf(version,"\"%s\"", APP_VERSION);
    sprintf(APP_BUILD_TIME,"\"%s,%s\"", __DATE__,__TIME__);
    OSI_LOGI(0, "appimg_enter ...");
    if(false == nwy_app_version(version, APP_BUILD_TIME))
    {
      OSI_LOGI(0, "app set version fail");
      return 0;
    }

    nwy_test_cli_thread = nwy_create_thread("test-cli", nwy_test_cli_main_func, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 10, 16);
    return 0;
}

void appimg_exit(void)
{
}
