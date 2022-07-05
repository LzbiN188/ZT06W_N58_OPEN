/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====
    Copyright (c) 2018 Neoway Technologies, Inc.
    All rights reserved.
    Confidential and Proprietary - Neoway Technologies, Inc.
    Author: lipengchao
    Date: 2019/12
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __NWY_SMS_H__
#define __NWY_SMS_H__

//#include "osi_api.h"
#ifdef __cplusplus
extern "C" {
#endif

#define NWY_SMS_MAX_PDU_LENGTH 500
#define NWY_SMS_BODY_BUFF_LEN 176
#define NWY_SMS_MAX_ADDR_LENGTH 21
#define NWY_SMS_MAX_MO_MSG_LENGTH 5*160

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef signed char int8_t;


typedef enum nwy_ret_type
{
  NWY_SMS_SUCCESS = 0,
  NWY_SMS_ERROR = -1
}nwy_result_t;

typedef struct nwy_sms_result
{
  char sca[64];
}nwy_sms_result_type;


typedef enum nwy_sms_storage_type
{
    NWY_SMS_STORAGE_TYPE_NONE = 0x00,
	NWY_SMS_STORAGE_TYPE_ME = 0x01,
	NWY_SMS_STORAGE_TYPE_SM = 0x02,
	NWY_SMS_STORAGE_TYPE_MAX,
}nwy_sms_storage_type_e;

typedef enum nwy_sms_mag_dcs_type
{
  NWY_SMS_MSG_GSM7_DCS = 0,
  NWY_SMS_MSG_8BIT_DCS = 1,
  NWY_SMS_MSG_UCS2_DCS = 2,
  NWY_SMS_MSG_UNKONWN_DCS,
}nwy_sms_mag_dcs_type_e;

typedef struct nwy_sms_info_type
{
  char phone_num[NWY_SMS_MAX_ADDR_LENGTH + 1];
  unsigned int msg_context_len;
  uint8_t msg_contxet[NWY_SMS_MAX_MO_MSG_LENGTH + 1];
  nwy_sms_mag_dcs_type_e msg_format;
}nwy_sms_info_type_t;

typedef struct NWY_SMS_TIME_STAMP
{
    uint16_t uYear;
    uint8_t uMonth;
    uint8_t uDay;
    uint8_t uHour;
    uint8_t uMinute;
    uint8_t uSecond;
    int8_t iZone;
} NWY_SMS_TIME_STAMP_t;

typedef struct nwy_sms_recv_info_type
{
  char oa[NWY_SMS_MAX_ADDR_LENGTH + 1];
  uint8_t oa_size;
  NWY_SMS_TIME_STAMP_t scts;
  nwy_sms_mag_dcs_type_e dcs;
  uint16_t nStorageId;
  uint16_t nIndex;
  uint16_t nDataLen;
  char pData[NWY_SMS_MAX_PDU_LENGTH];
  uint8_t cnmi_mt;
}nwy_sms_recv_info_type_t;

/*add by gaohe for read cmgl list begin*/
typedef enum nwy_sms_msg_status
{
  NWY_SMS_STORED_CMGL_STATUS_UNREAD = 1,
  NWY_SMS_STORED_CMGL_STATUS_READ,
  NWY_SMS_STORED_CMGL_STATUS_UNSENT,
  NWY_SMS_STORED_CMGL_STATUS_SENT,
  NWY_SMS_STORED_CMGL_STATUS_ALL
}nwy_sms_msg_status_e;

typedef struct nwy_sms_msg_list_index
{
  int index;
  nwy_sms_msg_status_e type;
}nwy_sms_msg_list_index_t;

typedef struct nwy_sms_msg_list
{
  int count;
  nwy_sms_msg_list_index_t sms_info[500];
}nwy_sms_msg_list_t;
/*add by gaohe for read cmgl list end*/

/* add by wangchen for del msg by type 20200925 begin */
typedef enum nwy_sms_msg_dflag_type
{
  NWY_SMS_MSG_DFLAG_NONE = 0,
  NWY_SMS_MSG_DFLAG_READ = 1,
  NWY_SMS_MSG_DFLAG_READ_SEND = 2,
  NWY_SMS_MSG_DFLAG_READ_SEND_UNSEND = 3,
  NWY_SMS_MSG_DFLAG_ALL = 4,
  NWY_SMS_MSG_DFLAG_MAX,
}nwy_sms_msg_dflag_e;
/* add by wangchen for del msg by type 20200925 end */


/******************************************************************************************
Function            :   nwy_sms_get_sca
Description         :   get sca address number
Data Accessed       :
Data Updated        :
Input               :   nwy_sms_result_type *sca
Output              :   nwy_sms_result_type *sca
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_get_sca(nwy_sms_result_type *sca);

/******************************************************************************************
Function            :   nwy_sms_set_sca
Description         :   set sca address number
Data Accessed       :
Data Updated        :
Input               :   char *sca unsigned tosca
Output              :
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_set_sca(char *sca, unsigned tosca);

/******************************************************************************************
Function            :   nwy_init_sms_option
Description         :   init sms
Data Accessed       :
Data Updated        :
Input               :   
Output              :
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_init_sms_option();

/******************************************************************************************
Function            :   nwy_sms_set_storage
Description         :   set sms storage(ME\SM)
Data Accessed       :
Data Updated        :
Input               :   nwy_sms_storage_type_e sms_storage
Output              :
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_set_storage(nwy_sms_storage_type_e sms_storage);

/******************************************************************************************
Function            :   nwy_sms_set_storage
Description         :   get sms storage(ME\SM)
Data Accessed       :
Data Updated        :
Input               :   
Output              :
Return              :   nwy_sms_storage_type_e sms_storage
*******************************************************************************************/
nwy_sms_storage_type_e nwy_sms_get_storage();

/******************************************************************************************
Function            :   nwy_sms_recv_message
Description         :   get one new sms
Data Accessed       :
Data Updated        :
Input               :   nwy_sms_recv_info_type_t *sms_data
Output              :   nwy_sms_recv_info_type_t *sms_data
Return              :   
*******************************************************************************************/
void nwy_sms_recv_message(nwy_sms_recv_info_type_t *sms_data);

/******************************************************************************************
Function            :   nwy_sms_send_pdu_message
Description         :   send PDU sms 
Data Accessed       :
Data Updated        :
Input               :   char *data,     PDU data hex format
                        int data_len    PDU hex data len
Output              :   
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_send_pdu_message(char *data, int data_len);

/******************************************************************************************
Function            :   nwy_sms_send_message
Description         :   send one sms
Data Accessed       :
Data Updated        :
Input               :   nwy_sms_info_type_t *p_sms_data
Output              :   
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_send_message(nwy_sms_info_type_t *p_sms_data);

/******************************************************************************************
Function            :   nwy_sms_delete_message
Description         :   delete one sms
Data Accessed       :
Data Updated        :
Input               :   uint16_t  nindex  nwy_sms_storage_type_e nStorage
Output              :   
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_delete_message(uint16_t  nindex, nwy_sms_storage_type_e nStorage);

/******************************************************************************************
Function            :   nwy_sms_read_message
Description         :   read one sms by index
Data Accessed       :
Data Updated        :
Input               :   unsigned nindex
Output              :   nwy_sms_recv_info_type_t *sms_data
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_read_message(unsigned nindex, nwy_sms_recv_info_type_t *sms_data);

/******************************************************************************************
Function            :   nwy_set_report_option
Description         :   set sms report mode(CMT/CMTI),ref AT+CNMI
Data Accessed       :
Data Updated        :
Input               :   uint8_t mode uint8_t mt uint8_t bm uint8_t ds uint8_t bfr
Output              :   
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_set_report_option(uint8_t mode, uint8_t mt, uint8_t bm, uint8_t ds, uint8_t bfr);

/* add by wangchen for del msg by type 20200925 begin */
/******************************************************************************************
Function            :   nwy_sms_delete_message_by_type
Description         :   del sms by type
Data Accessed       :
Data Updated        :
Input               :   nwy_sms_msg_dflag_e delflag nwy_sms_storage_type_e nStorage
Output              :   
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_delete_message_by_type(nwy_sms_msg_dflag_e delflag, nwy_sms_storage_type_e nStorage);
/* add by wangchen for del msg by type 20200925 end */

/* add by gaohe for read sms list 20210719 begin */
/******************************************************************************************
Function            :   nwy_sms_read_message_list
Description         :   get sms index and type list
Data Accessed       :
Data Updated        :
Input               :   nwy_sms_msg_list_t struct point
Output              :   
Return              :   nwy_result_t
*******************************************************************************************/
nwy_result_t nwy_sms_read_message_list(nwy_sms_msg_list_t *sms_info);
/* add by gaohe for read sms list 20210719 end */

#ifdef __cplusplus
   }
#endif

#endif

