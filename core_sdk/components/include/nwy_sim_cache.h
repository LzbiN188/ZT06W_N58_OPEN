#ifndef _NWY_SIM_CACHE_H_
#define _NWY_SIM_CACHE_H_
#include "_com_dtypes.h"

//#ifndef NWY_RES_OK
//#define NWY_RES_OK 0
//#endif

#ifndef NWY_SUCCESS
#define NWY_SUCCESS 0
#endif

#ifndef NWY_ERROR
#define NWY_ERROR  -1
#endif

#define NWY_SIM_CACHE_TIMER_DESC_MAX 128
#define NWY_SIM_CACHE_TIMER_MAX 8
#define NWY_NW_SCAN_LIST_MAX    10
#define NWY_NW_MNC_MCC_LEN      3
#define NWY_NW_EONS_MAX_LEN     128
#define NWY_NW_FPLMN_LIST_MAX   64
#define NWY_NW_SPN_LEN_MAX      17

#define SIM_FILE_STRUCTURE_BINARY 0
#define SIM_FILE_STRUCTURE_LINEAR 1
#define SIM_FILE_STRUCTURE_CYCLIC 3
#define SIM_FILE_TYPE_EF 4
#define USIM_FILE_STRUCTURE_BINARY 1
#define USIM_FILE_STRUCTURE_LINEAR 2
#define USIM_FILE_STRUCTURE_CYCLIC 6

#define NWY_CRAD_READ_BINARY 176
#define CRSM_READ_BINARY 176
#define CRSM_READ_RECORD 178
#define CRSM_GET_RESPONSE 192
#define CRSM_UPDATE_BINARY 214
#define CRSM_UPDATE_RECORD 220
#define CRSM_STATUS 242

#define NWY_SIM_CACHE_STATE_CHECK(state, sub) ((0 != sub ) && (( state & sub ) == sub))

typedef enum
{
  NWY_SIM_CACHE_TIMER_ID_1 = 0, /* Main State timer*/
  NWY_SIM_CACHE_TIMER_ID_2, /* SIM cache refresh timer */
  NWY_SIM_CACHE_TIMER_ID_3,
  NWY_SIM_CACHE_TIMER_ID_4,
  NWY_SIM_CACHE_TIMER_ID_5,
  NWY_SIM_CACHE_TIMER_ID_6,
  NWY_SIM_CACHE_TIMER_ID_7,
  NWY_SIM_CACHE_TIMER_ID_8,
  NWY_SIM_CACHE_TIMER_ID_MAX = NWY_SIM_CACHE_TIMER_ID_8
}nwy_sim_cache_timer_id_e;

typedef enum{
  NWY_SIM_CACHE_STATE_INVALID = 0x00,
  NWY_SIM_CACHE_REFRESH_STATE = 0x01,
  NWY_SIM_CACHE_RESERVER_STATE = 0x02,
  NWY_SIM_CACHE_STATE_MAX 
}nwy_sim_cache_state_e;

typedef enum
{
    NWY_SIM_CACHE_MIN = -1,
    NWY_FPLMN_READ,
    NWY_FPLMN_WRITE,
    NWY_SIM_CACHE_MAX,
}nwy_sim_cache_option_e;

typedef struct sys_plmn_id_s
{
  uint8                                identity[3];
    /**< Public Land Mobile Network ID. */
} sys_plmn_id_s_type;

typedef struct
{
    int num;
    sys_plmn_id_s_type fplmn[NWY_NW_FPLMN_LIST_MAX];
}nwy_sim_fplmn_list_t;

struct nwySimCacheCmd_t;
typedef void (*nwySimCacheAsyncDestroy_t)(struct nwySimCacheCmd_t *cmd);

typedef struct nwySimCacheCmd_t{
    uint32  option; // read/write card file option;read FPLMN.eg.
    uint32  data_len;
    void   *data;
    void    *async_ctx;
    nwySimCacheAsyncDestroy_t async_ctx_destroy;
}nwy_sim_cache_cmd_s;

typedef struct
{
    uint32 iCommand;
    uint32 iP1;
    uint32 iP2;
    uint32 iP3;
    uint16 nFileIDs;
} sim_cache_async_ctx_t;

void nwy_sim_global_cache_init(void);

void sys_plmn_get_mcc_mnc_str
(
  sys_plmn_id_s_type  plmn,
  char*             mcc_ptr,
  char*             mnc_ptr
);
boolean nwy_sim_cached_read_fplmn_list
(
    nwy_sim_fplmn_list_t * fplmn_list
);
boolean nwy_sim_cached_read_spn
(
  char *spn
);
/* added by wangchen for write FPLMN 20201105 begin */
int nwy_sim_read_fplmn_list();
/* added by wangchen for write FPLMN 20201105 end */

#endif
