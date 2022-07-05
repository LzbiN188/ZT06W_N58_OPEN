#ifndef __NWY_FOTA_API_H__
#define __NWY_FOTA_API_H__

#include "fupdate_config.h"
#ifdef __cplusplus
extern "C" {
#endif
#define FOTA_FLAG_FILE_NAME "fota_flag"
#define FOTA_COOKIE_FILE_NAME        "/cache/fota_cookie"
#define APP_UPDATE_PACK_FILE_NAME    "/cache/app.bin"
#define APP_UPDATE_PACK_DIR          "/cache"


typedef struct _ota_package {
    unsigned int offset;
    unsigned int len;
    unsigned char *data;
} ota_package_t;

//common app fota
int nwy_fota_dm(ota_package_t *ota_pack);
int nwy_package_checksum(void);
int nwy_fota_ua(void);
int nwy_get_fota_result(void);

int nwy_version_core_update(bool bRst);
int nwy_fota_download_core(ota_package_t *ota_pack);

#ifdef __cplusplus
   }
#endif
#endif
