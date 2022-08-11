#include "nwy_osi_api.h"
#include "app_task.h"

#define APP_MAIN_STACK_SIZE			1024*40
#define APP_100MS_STACK_SIZE		1024*20
#define APP_EVENT_STACK_SIZE		1024*10

static uint8_t mainStack[APP_MAIN_STACK_SIZE];
static uint8_t msStack[APP_100MS_STACK_SIZE];
static uint8_t eventStack[APP_EVENT_STACK_SIZE];

nwy_osiThread_t *myAppThread = NULL;
nwy_osiThread_t *myApp100MSThread = NULL;
nwy_osiThread_t *myAppEventThread = NULL;



int appimg_enter(void *param)
{
    myAppThread = nwy_create_thread_withstack("main", myAppRun, NULL, NWY_OSI_PRIORITY_NORMAL,mainStack,APP_MAIN_STACK_SIZE, 16);
    myApp100MSThread = nwy_create_thread_withstack("100MS", myApp100MSRun, NULL, NWY_OSI_PRIORITY_NORMAL,msStack,APP_100MS_STACK_SIZE, 16);
    myAppEventThread = nwy_create_thread_withstack("event", myAppEvent, NULL, NWY_OSI_PRIORITY_NORMAL, eventStack,APP_EVENT_STACK_SIZE, 16);
    return 0;
}

void appimg_exit(void)
{

}


