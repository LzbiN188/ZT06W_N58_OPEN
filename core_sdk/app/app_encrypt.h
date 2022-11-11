/*
 * app_encrypt.h
 *
 *  Created on: May 8, 2022
 *      Author: idea
 */

#ifndef APP_INCLUDE_APP_ENCRYPT_H_
#define APP_INCLUDE_APP_ENCRYPT_H_

#include "nwy_osi_api.h"


uint8_t encryptStr(uint8_t seed,uint8_t * data ,uint8_t len,uint8_t * encData,uint8_t * encLen);
uint8_t dencryptStr(uint8_t * data ,uint8_t len,uint8_t * encData,uint8_t * encLen);

void encryptTest(uint8_t *mac);
#endif /* APP_INCLUDE_APP_ENCRYPT_H_ */
