/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====
    Copyright (c) 2018 Neoway Technologies, Inc.
    All rights reserved.
    Confidential and Proprietary - Neoway Technologies, Inc.
    Author: shuiying
    Date: 2020/9
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __NWY_MQTT_H__
#define __NWY_MQTT_H__

#include "nwy_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define NWY_CERT_DIR "_nwy_cert_"
#define PAHO_HOST_ADDRESS_LEN    (255)
#define PAHO_CLIENT_ID_LEN       (256)
#define PAHO_USER_NAME_LEN       (512)
#define PAHO_PASSWORD_LEN        (256)    

#define PAHO_TOPIC_LEN_MAX 128
#ifdef FEATURE_NWY_N58_OPEN_NIPPON
#define PAHO_WILLMSG_LEN_MAX        (1024 * 65)
#define PAHO_MSG_LEN_MAX    (1024 * 65)
#else
#define PAHO_WILLMSG_LEN_MAX        (1024)   
#define PAHO_MSG_LEN_MAX   (512)
#endif
#define PAHO_PLAYOAD_LEN_MAX   (PAHO_MSG_LEN_MAX+256)

#define AESKEY_STR_LEN      (32)
#define AESKEY_HEX_LEN      (128/8)
#define PAHO_MULTILINE_WRITE_TIMEOUT  30000
#define PAHO_CERT_FILEPATH 100

typedef unsigned short		uint16;
typedef unsigned long int	uint32;

typedef enum
{
  PAHO_AUTH_NONE = 0,
  PAHO_AUTH_ENABLE,
  PAHO_AUTH_MAX
}paho_ssl_auth_mode;

typedef enum
{
  PAHO_SSL_NONE = 0,
  PAHO_SSL_ENABLE,
  PAHO_SSL_MAX
}paho_ssl_mode;

typedef struct 
{
  char cert_name[PAHO_CERT_FILEPATH+1];
}paho_ssl_cert_type;

typedef struct 
{
  paho_ssl_cert_type ssl_version;
  paho_ssl_auth_mode authmode;
  paho_ssl_mode sslmode;
  //int ciphersuite;   //reverse
  paho_ssl_cert_type cacert;
  paho_ssl_cert_type clientcert;
  paho_ssl_cert_type clientkey;
}paho_ssl_conf_type;



typedef struct
{
  unsigned char uBusy;//cmd is busy
  char clientID[PAHO_CLIENT_ID_LEN+1];
  char username[PAHO_USER_NAME_LEN+1];
  char password[PAHO_PASSWORD_LEN+1] ;

  paho_ssl_conf_type paho_ssl_tcp_conf;

  unsigned char willretained;
  unsigned char willqos;
  char willtopic[PAHO_TOPIC_LEN_MAX+1];
  char willmessage[PAHO_WILLMSG_LEN_MAX+1];
  int willmessage_len;

  uint16       port;
  char            host_name[PAHO_HOST_ADDRESS_LEN + 1]; 
  unsigned char keepalive;
  unsigned char cleanflag;
  int conntimeout;
  int timeout;
  
  char topic[PAHO_TOPIC_LEN_MAX+1];
  unsigned char qos;
  unsigned char retained;
  char message[PAHO_MSG_LEN_MAX+1];
  int message_len;

}nwy_paho_mqtt_at_param_type;

#ifdef __cplusplus
   }
#endif

#endif



