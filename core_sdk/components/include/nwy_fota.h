#ifndef __NWY_FOTA_H__
#define __NWY_FOTA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nwy_common.h"

/*----------------------------Type Definition-----------------------------*/

/*---------------------------Function Definition--------------------------*/

/**
 * write fota data to update file
 *
 *
 * \param [in] data    starting pointer of the fota data
 * \param [in] size    fota data szie

 * \return
 *      the number of bytes written on success
 */
 unsigned int nwy_fota_update(const void *data, unsigned int size);

/**
 * \brief set firmware update to ready status
 *
 * Inside this function, the update pack file will be thoroughly checked.
 *
 * When the update pack file is valid:
 *    set firmware update to ready status
 *
 * It must be called after the update pack file is written and closed. If
 * firmware update to ready status, bootloader will try to update. 
 *
 * \param [in] bRst     if true, system will be reboot for update
 *                      else , system will be update at next reboot
 * \return
 *      -  0 success
 *      - -1 fail
 */
int nwy_version_update(bool bRst);

/**
 * \brief get firmware update result after update reboot 
 *
 * \return
 *      -  0 success
 *      - -1 fail
 */
 int nwy_get_update_result(void);

#ifdef __cplusplus
   }
#endif

#endif // __NWY_FOTA_H__


