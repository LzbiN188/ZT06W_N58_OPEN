#ifndef __NWY_DATA_H__
#define __NWY_DATA_H__

#include "nwy_common.h"
#ifdef __cplusplus
extern "C" {
#endif
/*---------------------------Macro definition---------------------------*/

#define NWY_APN_MAX_LEN 50
#define NWY_PDP_ADDR_MAX_LEN 21
#define NWY_APN_USER_MAX_LEN 21
#define NWY_APN_PWD_MAX_LEN 20
#define NWY_DATA_CALL_MAX_NUM          7

/*-----------------------------Type Definition---------------------------*/





typedef struct
{
  int profile_idx;
  uint8 is_auto_recon;       //auto re connect enable, 0 disable, 1 enable
  uint16 auto_re_max;        //the maxinum times of the auto re_connect attempts, range[0,65535] 0: always attempts
  uint32 auto_interval_ms;   //the interval of the auto re_connect attempts, uint(ms),range[100,86400000]
}nwy_data_start_call_v02_t;

struct nwy_ip4_addr {
  uint32 addr;
};

struct nwy_ip6_addr {
  uint32 addr[4];
//#if LWIP_IPV6_SCOPES
  uint8 zone;
//#endif /* LWIP_IPV6_SCOPES */
};

typedef struct nwy_ip4_addr nwy_ip4_addr_t;
typedef struct nwy_ip6_addr nwy_ip6_addr_t;


typedef struct {
    nwy_ip4_addr_t ip_addr;
    /** Array of IPv6 addresses for this netif. */
    nwy_ip6_addr_t ip6_addr;
}nwy_data_addr_t;

typedef struct {
    nwy_data_addr_t iface_addr_s;                //Network interface address
    nwy_ip4_addr_t iface_mask;                         //Subnet mask
    nwy_ip4_addr_t gtwy_addr_s;                 //Gateway server address
    nwy_data_addr_t dnsp_addr_s;                 //Primary DNS server address
    nwy_data_addr_t dnss_addr_s;                 //Secondary DNS server address
}nwy_data_addr_t_info;

typedef enum {
    NWY_DATA_CALL_INVALID = 0x0,
    NWY_DATA_CALL_CONNECTED,                     //data call is connected
    NWY_DATA_CALL_DISCONNECTED,                  //data call is disconnected
} nwy_data_call_state_t;

typedef enum {
    NWY_DATA_PDP_TYPE_IPV4 = 1,                  //IPv4 dial
    NWY_DATA_PDP_TYPE_IPV6 = 2,                      //IPv6 dial
    NWY_DATA_PDP_TYPE_IPV4V6 = 3,                    //IPv4/v6 dial
    NWY_DATA_PDP_TYPE_PPP = 6,                       //PPP dial
} nwy_data_pdp_type_t;

typedef enum {
    NWY_DATA_PROFILE_3GPP = 0,                   //UMTS profile
    NWY_DATA_PROFILE_3GPP2                       //CDMA profile
} nwy_data_profile_type_t;

typedef enum {
    NWY_DATA_AUTH_PROTO_NONE=0,                    //none
    NWY_DATA_AUTH_PROTO_PAP,                     //PAP
    NWY_DATA_AUTH_PROTO_CHAP,                    //CHAP
} nwy_data_auth_proto_t;

typedef struct {
    nwy_data_pdp_type_t pdp_type;                //PPP,IPv4,
    char apn[NWY_APN_MAX_LEN+1];                 //Access Point Name
    nwy_data_auth_proto_t auth_proto;            //authentication protocol
    char user_name[NWY_APN_USER_MAX_LEN+1];      //user name
    char pwd[NWY_APN_PWD_MAX_LEN+1];            //password
} nwy_data_profile_info_t;

typedef void (*nwy_data_cb_func)
(   
    int hndl,
    nwy_data_call_state_t ind_state
    //void *ind_struct
);

typedef struct
{
  uint64 tx_bytes;
  uint64 rx_bytes;
  uint32 tx_packets;
  uint32 rx_packets;
}nwy_data_flowcalc_info_t;


typedef enum {
    NWY_DATA_AUTO_RECONN_DISABLE=0,           
    NWY_DATA_AUTO_RECONN_DOING,                
    NWY_DATA_AUTO_RECONN_ATTEMPT_MAX,         
} nwy_data_auto_reconn_state_e;

typedef struct {
    nwy_data_auto_reconn_state_e auto_reconn_state;
    uint16 auto_re_cnt;
} nwy_data_auto_reconn_status_t;

/*
@func 
    nwy_data_get_srv_handle
@desc 
    create a new data service handle
@param 
    cb_func: the callback function of the data service update state(such as connect, disconnect);
@return
    positive integer : data service handle id
    other : create a new data service handle fail
@other
*   
*/
int nwy_data_get_srv_handle(nwy_data_cb_func cb_func);


/*
@func 
    nwy_data_relealse_srv_handle
@desc 
    release the data service handle
@param 
    hndl [in]: Specify the data service handle id
@return
@other
*/
void nwy_data_relealse_srv_handle(int hndl);


/*
@func 
    nwy_data_start_call
@desc 
    start the data service call
@param 
    hndl [in]: Specify the data service handle id
    param: Specify the active param(such as profile id)
@return
    0: on success;
    negative integer: the error code, see nwy_error_e
@other
  Once data call activation is completed successfully, the callback function(see nwy_data_get_srv_handle) will be triggered. 
*/
int nwy_data_start_call(int hndl,nwy_data_start_call_v02_t* param);


/*
@func 
    nwy_data_stop_call
@desc 
    stop the data service call
@param 
    hndl [in]: Specify the data service handle id
@return
    0: on success;
    negative integer: the error code, see nwy_error_e
@other
  Once data call deactivation is completed successfully, the callback function(see nwy_data_get_srv_handle) will be triggered. 
*/
int nwy_data_stop_call(int hndl);


/*
@func 
    nwy_data_get_ip_addr
@desc 
    get the data call address info
@param 
    hndl[in]: Specify the data service handle id
    info_ptr[out]: the pointer of the data call address info
    len[out]: the pointer of the data call address info number
@return
    0: on success;
    negative integer: the error code, see nwy_error_e
@other
*/
int nwy_data_get_ip_addr(int hndl,nwy_data_addr_t_info* info_ptr,int *len);


/*
@func 
    nwy_data_set_profile
@desc 
    set the profile info
@param 
    profile_idx [in]: Specify the profile id
    profile_type [in]: Specify the profile type
    profile_info [in]: the pointer of the setting profile info
@return
    0: on success;
    negative integer: the error code, see nwy_error_e
@other
*/
int nwy_data_set_profile(int profile_idx,nwy_data_profile_type_t profile_type, nwy_data_profile_info_t *profile_info);


/*
@func 
    nwy_data_get_profile
@desc 
    get the profile info
@param 
    profile_idx [in]: Specify the profile id
    profile_type [in]: Specify the profile type
    profile_info [out]: the pointer of the acquired profile info
@return
    0: on success;
    negative integer: the error code, see nwy_error_e
@other
*/
int nwy_data_get_profile(int profile_idx,nwy_data_profile_type_t profile_type, nwy_data_profile_info_t *profile_info);


/*
@func 
    nwy_data_get_auto_reconnect_status
@desc 
    get the auto reconnect status info
@param 
    hndl[in]: Specify the data service handle id
    reconn_status[out]: the pointer of the data call auto reconnect status info
@return
    0: on success;
    negative integer: the error code, see nwy_error_e
@other
*/

int nwy_data_get_auto_reconnect_status(int hndl, nwy_data_auto_reconn_status_t *reconn_status);


/*
 @func: 
    nwy_ip4addr_ntoa
 @desc 
    Convert numeric IP address into decimal dotted ASCII representation.returns ptr to static buffer; not reentrant!
 @param
    addr: ip address in network order to convert
 @return
   pointer to a global static (!) buffer that holds the ASCII representation of addr
 */
char* nwy_ip4addr_ntoa(const nwy_ip4_addr_t *addr);


/*
 @func: 
    nwy_ip6addr_ntoa
 @desc 
    Convert numeric IPV6 address into decimal dotted ASCII representation.returns ptr to static buffer; not reentrant!
 @param
    addr: ip address in network order to convert
 @return
   pointer to a global static (!) buffer that holds the ASCII representation of addr
 */
char *nwy_ip6addr_ntoa(const nwy_ip6_addr_t *addr);

int nwy_cmdfunc_nftrcal(void);

int nwy_cmdfunc_nftrfit(void);


#ifdef __cplusplus
   }
#endif

#endif
