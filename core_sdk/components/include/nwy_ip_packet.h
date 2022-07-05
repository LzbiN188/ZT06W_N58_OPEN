#ifndef __NWY_IP_PACKET_H__
#define __NWY_IP_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nwy_get_ip_packet)(uint8_t *data,int len);

void nwy_get_recv_ip_packet_reg(nwy_get_ip_packet cb);
void nwy_get_send_ip_packet_reg(nwy_get_ip_packet cb);


#ifdef __cplusplus
}
#endif

#endif/*__NWY_ADC_H__*/
