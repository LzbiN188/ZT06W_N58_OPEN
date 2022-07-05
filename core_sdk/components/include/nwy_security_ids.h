#ifndef __NWY_RHINO_IDS_H__
#define __NWY_RHINO_IDS_H__

#ifdef __cplusplus
extern "C" {
#endif



typedef void (*nwy_get_warning_packet)(uint8_t *data, int len);

 /**
 * @.config the ids debug level
 *
 * @param
        lvl:debug level
        0 close, 1 output usb open_con
 * @return
        void
 */
void nwy_ids_config_debuglvl(uint8_t lvl);

 /**
  * @.regist function to process invade info
  *
  * @param
         process function
  * @return
         void
  */
void nwy_get_warning_packet_reg(nwy_get_warning_packet cb);

/**
 * @.init sdk
 *
 * @param
        void
 * @return
        sucess:NWY_SUCESS
        fail  :NWY_GEN_E_UNKNOWN
 */
int nwy_ids_init();

#ifdef __cplusplus
}
#endif


#endif
