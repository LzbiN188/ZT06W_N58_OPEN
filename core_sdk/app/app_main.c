#include "nwy_osi_api.h"
#include "app_task.h"
#include "app_sys.h"

nwy_osiThread_t *myAppThread = NULL;
nwy_osiThread_t *myAppEventThread = NULL;
nwy_osiThread_t *myAppBleThread = NULL;
nwy_osiThread_t *myAppPost = NULL;



int appimg_enter(void *param)
{
    myAppThread = nwy_create_thread("main", myAppRun, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 100, 16);
    myAppEventThread = nwy_create_thread("event", myAppEvent, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 20, 16);
    myAppBleThread = nwy_create_thread("ble", myAppRunIn100Ms, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 20, 16);
    myAppPost = nwy_create_thread("post", myAppCusPost, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 20, 16);
    return 0;
}

void appimg_exit(void)
{
	appSystemReset();
}


