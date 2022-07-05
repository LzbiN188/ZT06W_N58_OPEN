#include "nwy_osi_api.h"
#include "app_task.h"


nwy_osiThread_t *myAppThread = NULL;
nwy_osiThread_t *myApp100MSThread = NULL;
nwy_osiThread_t *myAppEventThread = NULL;

int appimg_enter(void *param)
{
    myAppThread = nwy_create_thread("main", myAppRun, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 100, 16);
    myApp100MSThread = nwy_create_thread("100MS", myApp100MSRun, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 50, 16);
    myAppEventThread = nwy_create_thread("event", myAppEvent, NULL, NWY_OSI_PRIORITY_NORMAL, 1024 * 10, 16);
    return 0;
}

void appimg_exit(void)
{

}


