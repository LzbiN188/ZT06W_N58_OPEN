#ifndef __NWY_WIFI_H__
#define __NWY_WIFI_H__

#include "nwy_common.h"
#include "nwy_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NWY_WIFI_SCAN_TIMEOUT  300
#define NWY_WIFI_SCAN_AP_MAX  16

#ifdef FEATURE_NWY_OPEN_ALIOS
#define UART_PORT_ID        (2)
#define UART_DATA_MODE      (1)
#define WIFI_SCAN_TIMEOUT	(10)
#endif

typedef struct
{
#ifdef FEATURE_NWY_OPEN_ALIOS
	char ssid[33];
#endif
    uint8 mac[6];
    uint8 channel;     ///< channel id
    int  rssival;      ///< signal strength
}nwy_wifi_ap_info_t;

typedef struct
{
    int num;
    nwy_wifi_ap_info_t ap_list[NWY_WIFI_SCAN_AP_MAX];
}nwy_wifi_scan_list_t;

/**
 * @.wifi scan
 *
 * @param scan_list:get scan ap info
 * @return
        sucess:NWY_SUCESS
        fail:others
 */
int nwy_wifi_scan(nwy_wifi_scan_list_t *scan_list);

#ifdef FEATURE_NWY_OPEN_ALIOS
int nwy_wifi_scan_aliba(nwy_wifi_scan_list_t *scan_list, int portid);
#endif

#ifdef __cplusplus
   }
#endif

#endif
