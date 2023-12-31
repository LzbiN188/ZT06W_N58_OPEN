#include "app_kernal.h"
#include "stdlib.h"
#include "stdio.h"

static uint8_t timer_list[TIMER_MAX];
static appTimer *timer_head = NULL;
static uint32_t systick = 0;

//此函数务必被调用，内核系统的时基
void kernalTickInc(void)
{
    systick++;
}
uint32_t kernalGetTick(void)
{
    return systick;
}

void kernalTimerInit(void)
{
    memset(timer_list, 0, TIMER_MAX);
}

/************************Kernal**********************************/
static uint8_t Create_Timer(uint32_t time, uint8_t timer_id, void (*fun)(void), uint8_t repeat)
{
    appTimer *next;
    if (timer_head == NULL)
    {
        timer_head = malloc(sizeof(appTimer));
        if (timer_head == NULL)
        {
            return 0;
        }
        memset(timer_head, 0, sizeof(appTimer));
        timer_head->timer_id = timer_id;
        timer_head->start_tick = kernalGetTick();
        timer_head->stop_tick = timer_head->start_tick + time;
        timer_head->repeat = repeat;
        timer_head->repeattime = time;
        timer_head->fun = fun;
        timer_head->next = NULL;
        return 1;
    }
    next = timer_head;
    do
    {
        if (next->next == NULL)
        {
            next->next = malloc(sizeof(appTimer));
            if (next->next == NULL)
            {
                return 0;
            }
            memset(next->next, 0, sizeof(appTimer));
            next = next->next;
            next->fun = fun;
            next->next = NULL;
            next->start_tick = kernalGetTick();
            next->stop_tick = next->start_tick + time;
            next->repeat = repeat;
            next->repeattime = time;
            next->timer_id = timer_id;
            break;
        }
        else
        {
            next = next->next;
        }
    }
    while (next != NULL);

    return 1;
}

/**************************************************
@bref		创建定时器
@param
	time	轮询实际
	fun		回调函数
	repeat	是否重复
@return

@note
**************************************************/

int8_t startTimer(uint32_t time, void (*fun)(void), uint8_t repeat)
{
    int i = 0;
    for (i = 0; i < TIMER_MAX; i++)
    {
        if (timer_list[i] == 0)
        {
            if (Create_Timer(time, i, fun, repeat) == 1)
            {
                timer_list[i] = 1;
                LogPrintf(DEBUG_ALL, "startTimer==>%d success", i);
                return i;
            }
            else
            {
                break;
            }
        }
    }
    LogPrintf(DEBUG_ALL, "startTimer==>start Error!(%d)", i);
    return -1;

}
/**************************************************
@bref		停止定时器，不能在定时器回调函数中调用此函数
@param
	timer_id	定时器id
@return

@note
**************************************************/

void stopTimer(uint8_t timer_id)
{
    appTimer *next, *pre;
    next = timer_head;
    pre = next;
    while (next != NULL)
    {
        if (next->timer_id == timer_id)
        {
            LogPrintf(DEBUG_ALL, "stop cycle task %d", next->timer_id);
            if (next == timer_head)
            {
                next = next->next;
                timer_list[timer_head->timer_id] = 0;
                free(timer_head);
                timer_head = next;
                break;
            }
            timer_list[next->timer_id] = 0;
            pre->next = next->next;
            free(next);
            next = pre;
            break;
        }
        pre = next;
        next = next->next;
    }
}

/**************************************************
@bref		取消定时器
@param
	timer_id	定时器id
@return

@note
**************************************************/

void cancelTimer(uint8_t timer_id)
{
    appTimer *next;
    next = timer_head;
    while (next != NULL)
    {
        if (next->timer_id == timer_id)
        {
            LogPrintf(DEBUG_ALL, "cancelTimer %d", next->timer_id);
            next->funStop=1;
			next->suspend=0;
			next->repeat=0;
			next->stop_tick=0;
            break;
        }
        next = next->next;
    }
}

/**************************************************
@bref		定时器停止重复，并执行最后一次
@param
	timer_id
@return

@note
**************************************************/

void stopTimerRepeat(uint8_t timer_id)
{
    appTimer *next;
    next = timer_head;
    while (next != NULL)
    {
        if (next->timer_id == timer_id)
        {
            next->repeat = 0;
        }
        next = next->next;
    }
}
/**************************************************
@bref		定时器核心执行程序
@param
@return

@note
**************************************************/

void kernalRun(void)
{
    appTimer *next, *pre, *cur;
    char debug[100];
    next = timer_head;
    pre = next;
    while (next != NULL)
    {
        if (next->suspend == 0 && next->stop_tick <= kernalGetTick())
        {
            if (next->funStop == 0)
            {
                next->fun();
            }
            if (next->repeat == 0)
            {
                if (next == timer_head)
                {
                    timer_list[next->timer_id] = 0;
                    sprintf(debug, "destroy task %d", next->timer_id);
                    LogMessage(DEBUG_ALL, debug);
                    next = next->next;
                    free(timer_head);
                    timer_head = next;
                    continue;
                }
                cur = next;
                timer_list[cur->timer_id] = 0;
                sprintf(debug, "destroy task %d", cur->timer_id);
                LogMessage(DEBUG_ALL, debug);
                pre->next = cur->next;
                next = pre;
                free(cur);
            }
            else
            {
                next->stop_tick = kernalGetTick() + next->repeattime;
            }
        }
        pre = next;
        next = next->next;
    }
    kernalTickInc();
}

//挂起任务
void systemTaskSuspend(uint8_t taskid)
{
    appTimer *next;
    char debug[50];
    next = timer_head;
    while (next != NULL)
    {
        if (next->timer_id == taskid)
        {
            next->suspend = 1;
            sprintf(debug, "systemTaskSuspend:%d", taskid);
            LogMessage(DEBUG_ALL, debug);
            return ;
        }
        next = next->next;
    }
}
//恢复任务
void systemTaskResume(uint8_t taskid)
{
    appTimer *next;
    char debug[50];
    next = timer_head;
    while (next != NULL)
    {
        if (next->timer_id == taskid)
        {
            next->suspend = 0;
            next->stop_tick = 0; //立即执行该任务
            sprintf(debug, "systemTaskResume:%d\n", taskid);
            LogMessage(DEBUG_ALL, debug);
            return;
        }
        next = next->next;
    }
}
//创建任务
int8_t createSystemTask(void(*fun)(void), uint32_t cycletime)
{
    return startTimer(cycletime, fun, 1);
}


