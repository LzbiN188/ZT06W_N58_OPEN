/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====
    Copyright (c) 2018 Neoway Technologies, Inc.
    All rights reserved.
    Confidential and Proprietary - Neoway Technologies, Inc.
    Author: lipengchao
    Date: 2019/12
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __NWY_POCDSDS_H__
#define __NWY_POCDSDS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum nwy_poc_dsds_onoff_type
{
    NWY_POC_DSDS_TYPE_OFF = 0,
    NWY_POC_DSDS_TYPE_ON = 1,
    NWY_POC_DSDS_TYPE_MAX,
}nwy_poc_dsds_onoff_type_e;

typedef enum nwy_poc_dsds_defaultcard_type
{
    NWY_POC_DSDS_DEFAULT_SIM1 = 0,
    NWY_POC_DSDS_DEFAULT_SIM2 = 1,
    NWY_POC_DSDS_DEFAULT_NONE = 255,
    NWY_POC_DSDS_DEFAULT_MAX,
}nwy_poc_dsds_defaultcard_type_e;

typedef enum nwy_poc_dsds_cdma_imsi_type
{
    NWY_POC_DSDS_CDMA_IMSI_NOT_INIT = 0,
    NWY_POC_DSDS_CDMA_IMSI_NOT_EXIST = 1,
    NWY_POC_DSDS_CDMA_IMSI_NORMAL = 2,
    NWY_POC_DSDS_CDMA_IMSI_MAX,
}nwy_poc_dsds_cdma_imsi_type_e;

typedef enum nwy_poc_dsds_auto_switch_type
{
    NWY_POC_DSDS_AUTO_SWITCH_OFF = 0,
    NWY_POC_DSDS_AUTO_SWITCH_ON = 1,
    NWY_POC_DSDS_AUTO_SWITCH_MAX,
}nwy_poc_dsds_auto_switch_type_e;

typedef enum nwy_poc_dsds_mastercard_type
{
    NWY_POC_DSDS_MASTER_SIM1 = 0,
    NWY_POC_DSDS_MASTER_SIM2 = 1,
    NWY_POC_DSDS_MASTER_DEFAULT = 3,
    NWY_POC_DSDS_MASTER_NONE = 255,
}nwy_poc_dsds_mastercard_type_e;



int nwy_poc_dsds_set_on_off(nwy_poc_dsds_onoff_type_e switch_type);

nwy_poc_dsds_onoff_type_e nwy_poc_dsds_get_on_off();

int nwy_poc_dsds_set_default_card(nwy_poc_dsds_defaultcard_type_e default_card);

nwy_poc_dsds_defaultcard_type_e nwy_poc_dsds_get_default_card();

int nwy_poc_dsds_set_auto_switch_card(nwy_poc_dsds_auto_switch_type_e auto_switch);

nwy_poc_dsds_auto_switch_type_e nwy_poc_dsds_get_auto_switch_card();

int nwy_poc_dsds_set_mastercard_card(nwy_poc_dsds_mastercard_type_e master_card);

nwy_poc_dsds_mastercard_type_e nwy_poc_dsds_get_mastercard_card();

#ifdef __cplusplus
   }
#endif

#endif
