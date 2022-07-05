#ifndef __NWY_COMMON_H__
#define __NWY_COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "_com_dtypes.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NWY_APP_EVENT_ID_BASE 2100
#define NWY_APP_EVENT_ID_END  2200
//#include "osi_log.h"

typedef enum
{
    NWY_SUCESS = 0,

    NWY_GEN_E_UNKNOWN           = -1,
    NWY_GEN_E_INVALID_PARA      = -2,
    NWY_GEN_E_SET_FAILED = -3,
    NWY_GEN_E_GET_FAILED = -4,
    NWY_GEN_E_SIM_NOT_READY = -5,
    NWY_GEN_E_SIM_NOT_REG = -6,
    NWY_GEN_E_ATT_NOT_ACT = -7,
    NWY_GEN_E_GPRS_NOT_ACT = -8,
    NWY_GEN_E_AREADY_CONNECT = -9,

    NWY_AT_MSG_QUEUE_NULL            = -100,
    NWY_AT_GET_RESP_TIMEOUT          = -101,
    NWY_AT_RESP_TIMEOUT_PARA_ERR     = -102,
    NWY_AT_MALLOC_ERR                = -103,
    NWY_AT_MSG_PUT_ERR               = -104,

    NWY_WIFI_OPEN_FAILED        = -200,
    NWY_WIFI_SCAN_FAILED        = -201,
    NWY_WIFI_SCAN_TIMEOUT        = -202
}nwy_error_e;

#ifdef __cplusplus
   }
#endif

#endif
