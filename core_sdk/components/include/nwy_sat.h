/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====
    Copyright (c) 2018 Neoway Technologies, Inc.
    All rights reserved.
    Confidential and Proprietary - Neoway Technologies, Inc.
    Author: lipengchao
    Date: 2019/12
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __NWY_SAT_H__
#define __NWY_SAT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nwy_sat_onoff_type
{
    NWY_SAT_TYPE_OFF = 0,
    NWY_SAT_TYPE_ON = 1,
    NWY_SAT_TYPE_MAX,
}nwy_sat_onoff_type_e;

typedef enum nwy_sat_pci_type
{
    NWY_SAT_PCITYPE_HANDLE_BY_TE = 0,
    NWY_SAT_PCITYPE_HANDLE_BY_ME = 1,
    NWY_SAT_PCITYPE_NO_OTHER,
}nwy_sat_pci_type_e;


typedef struct nwy_sat_notify_info_type
{
  nwy_sat_pci_type_e pcitype;
  uint16_t nDataLen;
  char pData[540];
}nwy_sat_notify_info_type_t;




int nwy_sat_set_on_off(nwy_sat_onoff_type_e switch_type);

nwy_sat_onoff_type_e nwy_sat_get_on_off();

int nwy_sat_terminal_response(char *data);

int nwy_sat_envelope_command(char *data);

void nwy_sat_recv_data(nwy_sat_notify_info_type_t *sat_data);

int nwy_sat_set_profile(char *data);

int nwy_sat_get_profile(char *data);


#ifdef __cplusplus
   }
#endif

#endif
