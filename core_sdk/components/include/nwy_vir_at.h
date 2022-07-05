#ifndef __NWY_VIR_AT_H__
#define __NWY_VIR_AT_H__

#ifdef __cplusplus
extern "C" {
#endif
#define NWY_AT_TIMEOUT_MAX   (30)  //30S
#define NWY_AT_TIMEOUT_MIN   (5)  //5S
#define NWY_AT_TIMEOUT_DEFAULT   (30)  //5S
#define NWY_AT_UNISOLICITE_REG_MAX (32)

typedef int (*nwy_at_unsolicite_handle)(char *data_str);
typedef void (*nwy_get_original_at_rsp)(char *rsp, int len);

typedef struct nwy_at_info
{
    char at_command[1024];
    int length;
}nwy_at_info;

typedef struct nwy_at_unsolicite_table
{
    char at_prefix[32];
    nwy_at_unsolicite_handle nwy_at_unsolicite_cb;
}nwy_at_unsolicite_table_t;


typedef void (*nwy_at_rsp_report_cb)(char *resp, int len);
/**
 * @init at parameter.
 *
 * @param 
 * @return
 */

void nwy_sdk_at_parameter_init();

/**
 * @.send at command
 *
 * @param pInfo:at infor
          resp:at response
          timeout:5-30s
 * @return
        sucess:NWY_SUCESS
        fail:others
 */
int nwy_sdk_at_cmd_send(nwy_at_info *pInfo, char *resp, int timeout);

/**
 * @.register the unsolicited at process
 *
 * @param at_prefix:the unsolicited at prefix
          p_func:the process function
 * @return
        sucess:NWY_SUCESS
        fail:others
 */
int nwy_sdk_at_unsolicited_cb_reg(char *at_prefix, void *p_func);

void nwy_sdk_at_get_original_rsp_cb(nwy_get_original_at_rsp cb);


#ifdef __cplusplus
   }
#endif

#endif
