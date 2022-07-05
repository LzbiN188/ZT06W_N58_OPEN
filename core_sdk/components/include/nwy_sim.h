/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====
    Copyright (c) 2018 Neoway Technologies, Inc.
    All rights reserved.
    Confidential and Proprietary - Neoway Technologies, Inc.
    Author: lipengchao
    Date: 2019/12
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __NWY_API_H__
#define __NWY_API_H__

#include "nwy_common.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum nwy_result
{
  NWY_RES_OK = 0,
  NWY_RES_ERROR = -1
}nwy_result_type;

typedef enum nwy_sim_status
{
  NWY_SIM_STATUS_GET_ERROR = -1,
  NWY_SIM_STATUS_READY,
  NWY_SIM_STATUS_NOT_INSERT,
  NWY_SIM_STATUS_PIN1,
  NWY_SIM_STATUS_PUK1,
  NWY_SIM_STATUS_BUSY,
}nwy_sim_status;

typedef enum
{
  NWY_SIM_AUTH_READY,                  //"READY"
  NWY_SIM_AUTH_PIN1_READY,             //"PIN1 READY"
  NWY_SIM_AUTH_SIMPIN,                 //"SIM PIN"
  NWY_SIM_AUTH_SIMPUK,                 //"SIM PUK"
  NWY_SIM_AUTH_PHONE_TO_SIMPIN,        //"PH-FSIM PIN"
  NWY_SIM_AUTH_PHONE_TO_FIRST_SIMPIN,  //"PH-FSIM PIN"
  NWY_SIM_AUTH_PHONE_TO_FIRST_SIMPUK,  //"PH-FSIM PUK"
  NWY_SIM_AUTH_SIMPIN2,                //"SIM PIN2"
  NWY_SIM_AUTH_SIMPUK2,                //"SIM PUK2"
  NWY_SIM_AUTH_NETWORKPIN,             //"PH-NET PIN"
  NWY_SIM_AUTH_NETWORKPUK,             //"PH-NET PUK"
  NWY_SIM_AUTH_NETWORK_SUBSETPIN,      //"PH-NETSUB PIN"
  NWY_SIM_AUTHNETWORK_SUBSETPUK,       //"PH-NETSUB PUK"
  NWY_SIM_AUTH_PROVIDERPIN,            //"PH-SP PIN"
  NWY_SIM_AUTH_PROVIDERPUK,            //"PH-SP PUK"
  NWY_SIM_AUTH_CORPORATEPIN,           //"PH-CORP PIN"
  NWY_SIM_AUTH_CORPORATEPUK,           //"PH-CORP PUK"
  NWY_SIM_AUTH_NOSIM,                  //"NO SIM"
  NWY_SIM_AUTH_PIN1BLOCK,              //"PIN1 BLOCK"
  NWY_SIM_AUTH_PIN2BLOCK,              //"PIN2 BLOCK"
  NWY_SIM_AUTH_PIN1_DISABLE,           //"PIN1 DISABLE"
  NWY_SIM_AUTH_SIM_PRESENT,            //"SIM PRESENT"
  NWY_SIM_AUTH_NULL,                   //"NULL"
}nwy_pin_mode_type;

typedef struct nwy_sim_result
{
  unsigned char iccid[23];
  unsigned char imsi[16];
  unsigned char lock_sim_password[12];
  unsigned char unlock_sim_password[12];
  nwy_pin_mode_type pin_mode;
  unsigned char nImei[16];
}nwy_sim_result_type;

typedef enum
{
  NWY_SIM_ID_SLOT_1,
  NWY_SIM_ID_SLOT_2,
  NWY_SIM_ID_MAX
}nwy_sim_id_t;

/* add by wangchen for sim power on/off api 20210514 begin */
typedef enum nwy_sim_power_type
{
    NWY_SIM_POWER_TYPE_ON = 0x00,
    NWY_SIM_POWER_TYPE_OFF = 0x01,
    NWY_SIM_POWER_TYPE_MAX,
}nwy_sim_power_type_e;
/* add by wangchen for sim power on/off api 20210514 begin */

nwy_sim_status nwy_sim_get_card_status();

// add by lipengchao in 2019.11.23 for iccid
nwy_result_type nwy_sim_get_iccid
(
  nwy_sim_result_type *iccid_buf
);

nwy_result_type nwy_sim_get_imsi
(
  nwy_sim_result_type *imsi_buf
);

nwy_result_type nwy_sim_enable_pin
(
  nwy_sim_result_type *sim_lock
);

nwy_result_type nwy_sim_disable_pin
(
  nwy_sim_result_type *sim_unlock
);

nwy_result_type nwy_sim_get_pin_mode
(
  nwy_sim_result_type *pin_mode
);

/* add by wangchen for get IMEI 20200228 begin */
nwy_result_type nwy_sim_get_imei
(
  nwy_sim_result_type *imei
);
/* add by wangchen for get IMEI 20200228 end */

/* add by wangchen for simcross 20200601 begin */
nwy_result_type nwy_sim_set_simid
(
  uint8 nSwitchSimID
);

uint8 nwy_sim_get_simid();
/* add by wangchen for simcross 20200601 end */

/* add by wangchen for get number 20200804 begin */
int nwy_sim_get_msisdn(nwy_sim_id_t sim_id, char* msisdn_buf, size_t buf_len);
/* add by wangchen for get number 20200804 end */

int nwy_sim_csim(nwy_sim_id_t sim_id, char *indata, int indata_len, char *outdata, int outdata_len);

int nwy_sim_verify_pin(nwy_sim_id_t sim_id, const char* pin);

int nwy_sim_unblock(nwy_sim_id_t sim_id, const char* puk, const char* new_pin);

int nwy_sim_get_lacid(int *lac, int *cid);

/*Start:add by yangmengsha for modify sim hot plug trigger mode 20201031*/
/**
 * @brief set sim detect trigger mode
 *
 * @param mode : 0--low level(insert card) 1--high level(insert card)
 * @return
 *      - NULL
 */
//int nwy_set_sim_detect_trigger_mode(bool mode);
void nwy_set_sim_detect_trigger_mode(bool mode); //update by gaohe for Eliminate the warning in 2021/11/11

/*End:add by yangmengsha for modify sim hot plug trigger mode 20201031*/

/*Start:add by yangmengsha for config sim detect enable or disable 20210112*/
/**
 * @brief set sim detect enable or disable
 *
 * @param mode : 0--disable, 1--enable
 * @return
 *      - NULL
 */
//int nwy_set_sim_detect_mode(bool enable);
void nwy_set_sim_detect_mode(bool enable);//update by gaohe for Eliminate the warning in 2021/11/11
/*End:add by yangmengsha for config sim detect enable or disable 20210112*/
/* add by wangchen for sim power on/off api 20210514 begin */
int nwy_sim_power_on_off(nwy_sim_id_t sim_id, nwy_sim_power_type_e power_type);
/* add by wangchen for sim power on/off api 20210514 end */

/* added by wangchen for VSIM 20210527 begin */
unsigned int nwy_sim_setvsim_handler(void *getSimType, void *vsimReset, void *vsimProcessApdu, nwy_sim_id_t nSimID);
/* added by wangchen for VSIM 20210527 end */

/* Begin by zhaoyating to control tripple sim 20220615*/
int nwy_get_oper_curr_sim_id(void);
int nwy_set_switch_simcross(int simId);
/* End by zhaoyating to control tripple sim 20220615*/
#ifdef __cplusplus
   }
#endif


#endif
