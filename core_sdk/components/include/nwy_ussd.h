/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====
    Copyright (c) 2018 Neoway Technologies, Inc.
    All rights reserved.
    Confidential and Proprietary - Neoway Technologies, Inc.
    Author: wangchen
    Date: 2019/12
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __NWY_USSD_H__
#define __NWY_USSD_H__

#include "nwy_common.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum nwy_ussd_result_type
{
  NWY_USSD_SUCCESS = 0,
  NWY_USSD_ERROR = -1
}nwy_ussd_result_t;

typedef struct _nwy_ussd_info_type
{
    uint8_t aucDestUsd[2 * 160 + 2];
    uint8_t nStingSize;
    uint8_t nOption;
    uint8_t nDcs;
} nwy_ussd_info_t;


int nwy_ussd_send_msg(uint8 simid, char* ussd_string, uint8 dcs, nwy_ussd_info_t* ussd_info);


#ifdef __cplusplus
   }
#endif

#endif
