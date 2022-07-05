#ifndef __NWY_DM_H__
#define __NWY_DM_H__
#include "nwy_common.h"
#ifdef __cplusplus
extern "C" {
#endif
int nwy_dm_get_dev_model(char *model_buf, int buf_len);
int nwy_dm_get_inner_version(char *version_buf, int buf_len);
int nwy_dm_get_open_sdk_version(char *version_buf, int buf_len);
void nwy_dm_get_hw_version(char *hw_ver, int buf_len);
/*
@func
    nwy_dm_get_device_version
@param
    char* dev_verl: device version
    char* lin_ver:lin version
@return
    void
@other
*/
void nwy_dm_get_device_version(char* dev_ver, char* lin_ver);

int nwy_dm_get_device_heapinfo(char* heapinfo);

int nwy_dm_get_rftemperature(float *outvalue);
int nwy_dm_get_nftrannt(void);

#define NWY_DM_SN_LENGTH_MAX 64
/**
 * @brief get device serial number.
 *
 * @param index : 0~1 for double sim
 * @param sn  : sn get buffer, 64 btyes recommend
 * @return
 *      - sn real length
 */
int nwy_dm_get_sn(int index, char *sn);

#ifdef __cplusplus
   }
#endif

#endif
