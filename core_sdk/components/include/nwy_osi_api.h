#ifndef _NWY_OSI_API_H_
#define _NWY_OSI_API_H_

#include "nwy_common.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct osiTimer nwy_osiTimer_t;
typedef struct osiThread nwy_osiThread_t;
typedef void (*nwy_osiCallback_t)(void *ctx);

typedef struct osiMessageQueue nwy_osiMessageQueue_t;

typedef struct osiMutex nwy_osiMutex_t;
typedef struct osiSemaphore nwy_osiSemaphore_t;
typedef struct osiPipe nwy_osiPipe_t;

typedef enum
{
    NWY_OSI_SHUTDOWN_RESET = 0,               ///< normal reset
    NWY_OSI_SHUTDOWN_FORCE_DOWNLOAD = 0x5244, ///< 'RD' reset to force download mode
    NWY_OSI_SHUTDOWN_DOWNLOAD = 0x444e,       ///< 'DN' reset to download mode
    NWY_OSI_SHUTDOWN_BL_DOWNLOAD = 0x4244,    ///< 'BD' reset to bootloader download mode
    NWY_OSI_SHUTDOWN_CALIB_MODE = 0x434c,     ///< 'CL' reset to calibration mode
    NWY_OSI_SHUTDOWN_NB_CALIB_MODE = 0x4e43,  ///< 'NC' reset to NB calibration mode
    NWY_OSI_SHUTDOWN_BBAT_MODE = 0x4241,      ///< 'BA' reset to BBAT mode
    NWY_OSI_SHUTDOWN_UPGRADE = 0x4654,        ///< 'FT' reset to upgrade mode
    NWY_OSI_SHUTDOWN_POWER_OFF = 0x4f46,      ///< 'OF' power off
    NWY_OSI_SHUTDOWN_PSM_SLEEP = 0x5053,      ///< 'PS' power saving mode
} nwy_osiShutdownMode_t;
typedef enum nwy_osiThreadPriority
{
    NWY_OSI_PRIORITY_IDLE = 1, // reserved
    NWY_OSI_PRIORITY_LOW = 8,
    NWY_OSI_PRIORITY_BELOW_NORMAL = 16,
    NWY_OSI_PRIORITY_NORMAL = 24,
    NWY_OSI_PRIORITY_ABOVE_NORMAL = 32,
    NWY_OSI_PRIORITY_HIGH = 40,
    NWY_OSI_PRIORITY_REALTIME = 48,
    NWY_OSI_PRIORITY_HISR = 56, // reserved
} nwy_osiThreadPriority;


typedef struct nwy_osiEvent_t
{
    uint32 id;     ///< event identifier
    uint32 param1; ///< 1st parameter
    uint32 param2; ///< 2nd parameter
    uint32 param3; ///< 3rd parameter
} nwy_osiEvent_t;

typedef enum
{
    NWY_TIME_ZONE_DISABLE = 0,
    NWY_TIME_ZONE_ENABLE = 1
}nwy_time_zone_switch;

/* Task states returned by eTaskGetState. */
typedef enum
{
    NWY_RUNNING = 0,    /* A task is querying the state of itself, so must be running. */
    NWY_READY,          /* The task being queried is in a read or pending ready list. */
    NWY_BLOCKED,        /* The task being queried is in the Blocked state. */
    NWY_SUSPENDED,      /* The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out. */
    NWY_DELETED,        /* The task being queried has been deleted, but its TCB has not yet been freed. */
    NWY_INVALID         /* Used as an 'invalid state' value. */
} nwy_thread_state;


typedef struct nwy_thread_info_t
{
    nwy_osiThread_t *thread_handle;         /* The handle of the task to which the rest of the information in the structure relates. */
    const char *thread_name;            /* A pointer to the task's name.  This value will be invalid if the task was deleted since the structure was populated! */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    unsigned long thread_number;        /* A number unique to the task. */
    nwy_thread_state thread_current_state;      /* The state in which the task existed when the structure was populated. */
    unsigned long thread_current_priority;  /* The priority at which the task was running (may be inherited) when the structure was populated. */
    unsigned long thread_base_priority;     /* The priority to which the task will return if the task's current priority has been inherited to avoid unbounded priority inversion when obtaining a mutex.  Only valid if configUSE_MUTEXES is defined as 1 in FreeRTOSConfig.h. */
    uint32_t thread_run_time_counter;       /* The total run time allocated to the task so far, as defined by the run time stats clock.  See http://www.freertos.org/rtos-run-time-stats.html.  Only valid when configGENERATE_RUN_TIME_STATS is defined as 1 in FreeRTOSConfig.h. */
    uint32_t *thread_stack_base;        /* Points to the lowest address of the task's stack area. */
    uint16_t thread_stack_highwatermark;    /* The minimum amount of stack space that has remained for the task since the task was created.  The closer this value is to zero the closer the task has come to overflowing its stack.convert word count to byte count should multiply 4 */
} nwy_thread_info_t;

typedef void (*nwy_osiPipeEventCallback_t)(void *param, unsigned event);

//timer API
nwy_osiTimer_t *nwy_timer_init(nwy_osiThread_t *thread, nwy_osiCallback_t cb, void *ctx);
void nwy_timer_deinit(nwy_osiTimer_t *timer);
bool nwy_start_timer_periodic(nwy_osiTimer_t *timer, uint32 ms);
bool nwy_start_timer(nwy_osiTimer_t *timer, uint32 ms);
bool nwy_stop_timer(nwy_osiTimer_t *timer);

// set app version , and user <at+nappcheck?> to read
bool nwy_app_version(char* ver, char* build_time);


//thread API
nwy_osiThread_t *nwy_create_thread(const char *name, nwy_osiCallback_t func, void *argument,
                             uint32 priority, uint32 stack_size,
                             uint32 event_count);

nwy_osiThread_t *nwy_create_thread_withstack(const char *name, nwy_osiCallback_t func, void *argument,
                             uint32 priority, void *stack, uint32 stack_size,
                             uint32 event_count);

nwy_osiThread_t *nwy_get_current_thread();
bool nwy_set_thread_priority(nwy_osiThread_t *thread, uint32 priority);
uint32 nwy_get_thread_priority(nwy_osiThread_t *thread);
void nwy_suspend_thread(nwy_osiThread_t *thread);
void nwy_resume_thread(nwy_osiThread_t *thread);
bool nwy_send_thead_event(nwy_osiThread_t *thread, nwy_osiEvent_t *event, uint32 timeout);
bool nwy_wait_thead_event(nwy_osiThread_t *thread, nwy_osiEvent_t *event, uint32 timeout);
void nwy_exit_thread();
uint32_t nwy_get_thread_pendingevent_cnt(nwy_osiThread_t *thread);
uint32_t nwy_get_thread_spaceevent_cnt(nwy_osiThread_t *thread);
uint32_t nwy_get_thread_info(nwy_osiThread_t *thread, nwy_thread_info_t *thread_info);
uint32_t nwy_get_thread_list(char *thread_list);
uint32_t nwy_get_thread_runtime_stats(char *stats_buf);

/* val: 0-20 release rrc seconds */
uint32_t nwy_set_rrc_release(uint32_t val);


void nwy_sleep(uint32 ms);
void nwy_usleep(uint32 us);

// get sys time from start
int64_t nwy_get_ms(void);
#ifndef NWY_TIME_STRUCT
#define NWY_TIME_STRUCT
typedef struct nwy_time_t
{
    unsigned short year; // [0,127] Year-2000, tm_year: Year-1900
    unsigned char mon;  // [1,12], tm_mon: (0-11)
    unsigned char day;  // [1,31], tm_mday: (1-31)
    unsigned char hour; // [0,23], tm_hour: (0-23)
    unsigned char min;  // [0,59], tm_min: (0-59)
    unsigned char sec;  // [0,59], tm_sec: (0-60)
    unsigned char wday; // [1,7] 1 for Monday, tm_wday: (0-6) 0 for Sunday
} nwy_time_t;
#endif
extern void nwy_set_time(nwy_time_t *julian_time, char timezone);
extern int nwy_get_time(nwy_time_t *julian_time, char *timezone);


/*********************************************************************
handle: uart or usb, or vir at handle
type:   0: AT_CMD_SET, 1: AT_CMD_TEST, 2: AT_CMD_READ; 3: AT_CMD_EXE;
**********************************************************************/
#define NWY_ATFORWARD_MAX  50
typedef void (*nwy_at_forward_process_cb)(void* handle, char* atcmd,int type, char* para0, char* para1, char* para2);
extern void nwy_set_at_forward_cb(int index,const char* name, nwy_at_forward_process_cb cb);
extern void nwy_at_forward_send(void* handle, char* buf, int len);


//message queue API
nwy_osiMessageQueue_t *nwy_create_msg_Que(uint32 msg_count, uint32 msg_size);
void nwy_delete_msg_que(nwy_osiMessageQueue_t *mq);
bool nwy_put_msg_que(nwy_osiMessageQueue_t *mq, const void *msg, uint32 timeout);
bool nwy_get_msg_que(nwy_osiMessageQueue_t *mq, const void *msg, uint32 timeout);
uint32_t nwy_get_queue_pendingevent_cnt(nwy_osiMessageQueue_t *mq);
uint32_t nwy_get_queue_spaceevent_cnt(nwy_osiMessageQueue_t *mq);

//mutex API
nwy_osiMutex_t *nwy_create_mutex();
bool nwy_lock_mutex(nwy_osiMutex_t *mutex, uint32 timeout);
void nwy_unlock_mutex(nwy_osiMutex_t *mutex);
void nwy_delete_mutex(nwy_osiMutex_t *mutex);

//Semaphore API
nwy_osiSemaphore_t *nwy_semaphore_create(uint32 max_count, uint32 init_count);
bool nwy_semaphore_acquire(nwy_osiSemaphore_t *sem, uint32 timeout);
void nwy_semahpore_release(nwy_osiSemaphore_t *sem);
void nwy_semahpore_delete(nwy_osiSemaphore_t *sem);

//pipe
nwy_osiPipe_t *nwy_osiPipe_create(unsigned size);
void nwy_osiPipe_delete(nwy_osiPipe_t *pipe);
int nwy_osiPipe_read(nwy_osiPipe_t *pipe, void *buf, unsigned size);
int nwy_osiPipe_write(nwy_osiPipe_t *pipe, void *buf, unsigned size);
int nwy_osiPipe_set_write_cb(nwy_osiPipe_t *pipe, unsigned mask, nwy_osiPipeEventCallback_t cb, void *ctx);
int nwy_osiPipe_set_read_cb(nwy_osiPipe_t *pipe, unsigned mask, nwy_osiPipeEventCallback_t cb, void *ctx);

/**
 * \brief monoclinic system time in microsecond
 *
 * The monoclinic system time in unit of microsecond.
 *
 * nwy_get_up_time_us_int64 is highly recommended, 
 * because nwy_get_up_time_us will overflow after 71 minutes.
 *
 * \return      32 bits monoclinic system time in microseconds
 */
int nwy_get_up_time_us(void);
/**
 * \brief monoclinic system time in microsecond
 *
 * The monoclinic system time in unit of microsecond.
 *
 * \return      64 bits monoclinic system time in microseconds
 */
int64 nwy_get_up_time_us_int64(void);

int nwy_set_time_zone_switch(nwy_time_zone_switch status);

int nwy_get_time_zone_switch(nwy_time_zone_switch *status);
void nwy_open_log(int tag, const char *format, ...);



/**
 * @brief set soft dog check time, default check time :1000ms
 *
 * @param check_time_ms: default 100ms;
 */
void nwy_dog_check_time( int check_time_ms);

/**
 * @brief kick dog ...
 */
void nwy_dog_kick(void);

/**
 * @brief register thread to soft dog
 *
 * @param thread: thread handle
 * @param feed_ms: feed soft dog time, must be multiple of check_time_ms
 * @return
 *      - SUCCESS    1
 *      - ERROR      0
 */
int nwy_dog_register(nwy_osiThread_t* thread, int feed_ms);


/**
 * @brief deregister thread from soft dog list.
 *
 * @param thread: thread handle
 * @return
 *      - SUCCESS    1
 *      - ERROR      0
 */
int nwy_dog_deregister( nwy_osiThread_t* thread );


/**
 * @brief set data to nv
 *
 * @param id: 0 or 1
 * @param data: 8Byte
 * @return
 *      - SUCCESS    0
 *      - ERROR      -1
 */
int nwy_set_text_nv(int id, unsigned char* data);

/**
 * @brief get data from nv
 *
 * @param id: 0 or 1
 * @param data: 8Byte
 * @return
 *      - SUCCESS    0
 *      - ERROR      -1
 */
int nwy_get_text_nv(int id, unsigned char* data);

#ifdef __cplusplus
   }
#endif


#endif
