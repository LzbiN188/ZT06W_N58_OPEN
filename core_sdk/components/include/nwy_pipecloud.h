#ifndef _NWY_PIPECLOUD_H_
#define _NWY_PIPECLOUD_H_
#ifdef FEATURE_NEOWAY_PIPECLOUD
#include "nwy_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#define NWY_BUFF_LEN        (64)
typedef enum
{
    NWY_PIPE_SWITCH_DISABLE = 0,
    NWY_PIPE_SWITCH_ENABLE  = 1,
}nwy_pipe_status_e;

typedef enum
{
    NWY_PIPE_SEVICE_TYPE_MONE = 0,
    NWY_PIPE_SEVICE_TYPE_NANWANG = 1,
    NWY_PIPE_SERVICE_TYPE_NEOWAY = 2,
    NWY_PIPE_SERVICE_TYPE_MAX
}nwy_pipe_service_type_e;

typedef struct
{
    nwy_pipe_status_e status;
    nwy_pipe_service_type_e type;
}nwy_pipe_switch_st;

typedef struct
{
    uint16 server_port;
    char server_url[NWY_BUFF_LEN];
}nwy_pipe_server_para_st;

int nwy_pipecloude_svr_cfg_set(nwy_pipe_server_para_st *svr_info);
int nwy_pipecloude_svr_cfg_get(nwy_pipe_server_para_st *svr_info);
int nwy_pipecloude_switch_set(nwy_pipe_switch_st *info);
int nwy_pipecloude_switch_get(nwy_pipe_switch_st *info);


#ifdef __cplusplus
}
#endif
#endif
#endif


