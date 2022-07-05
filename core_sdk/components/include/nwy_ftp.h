#ifndef __NWY_FTP_H__
#define __NWY_FTP_H__

#include "nwy_common.h"
#include "nwy_osi_api.h"
#include "nwy_http.h"

#define FTP_STRING_SIZE 256
#ifdef __cplusplus
extern "C" {
#endif

#define NWY_OPEN_FTPS_TYPE_EXPLICIT          1
#define NWY_OPEN_FTPS_TYPE_IMPLICIT          2

typedef enum
{
    NWY_FTP_EVENT_DNS_ERR,
    NWY_FTP_EVENT_OPEN_FAIL,
    NWY_FTP_EVENT_CLOSED,
    NWY_FTP_EVENT_LOGOUT,
    NWY_FTP_EVENT_DATA_RECVED,
    NWY_FTP_EVENT_DATA_GET,            //5
    NWY_FTP_EVENT_DATA_OPEND,
    NWY_FTP_EVENT_DATA_CLOSED, 
    NWY_FTP_EVENT_LOGIN,
    NWY_FTP_EVENT_SIZE,             //9
    NWY_FTP_EVENT_SIZE_ZERO,
    NWY_FTP_EVENT_PASS_ERROR,
    NWY_FTP_EVENT_FILE_NOT_FOUND,
    NWY_FTP_EVENT_FILE_SIZE_ERROR,
    NWY_FTP_EVENT_FILE_DELE_SUCCESS,    
    NWY_FTP_EVENT_DATA_PUT_FINISHED,
    NWY_FTP_EVENT_SSL_ERR,
    NWY_FTP_EVENT_DATA_OPEN_FAIL,
    NWY_FTP_EVENT_DATA_SETUP_ERR,
    NWY_FTP_EVENT_MK_DIR_OK,
    NWY_FTP_EVENT_MK_DIR_ERR,
    NWY_FTP_EVENT_RM_DIR_OK,
    NWY_FTP_EVENT_RM_DIR_ERR,
    NWY_FTP_EVENT_UNKOWN,
}nwy_ftp_event_e;


typedef struct nwy_ftp_result
{
  uint32 event;                     /* event id */
  uint32 data_len;                  /* data length */
  const byte* data;                 /* data */
}nwy_ftp_result_t;


typedef void (*resultcb)(nwy_ftp_result_t *event);  //notify events

typedef struct nwy_ftp_login_t
{
    uint16_t channel;                   /* channle (1-7) */
    uint8_t host[FTP_STRING_SIZE];      /* remote server hostname */
    uint16_t port;                      /* remote server port */
    uint8_t username[FTP_STRING_SIZE];  /* login username */
    uint8_t passwd[FTP_STRING_SIZE];    /* login passwd */
    uint8_t mode;                       /* 1 is active 0 is passv */
    uint16 timeout;                     /* timeout */
}nwy_ftp_login_t;


/*
@func 
    nwy_ftp_login
@desc 
    ftp login
@param 
    nwy_ftp_login_t *ftp_param [in]: Login parameter structure 
    resultcb cb [in]: Callback notification function
@return
    0: on success;
    -1: on failed 
@other
  Once ftp login is completed successfully, the callback function cb will be triggered. 
  NWY_FTP_EVENT_LOGIN event will report
*/

int nwy_ftp_login(nwy_ftp_login_t *ftp_param, resultcb cb);

/*
@func 
    nwy_ftp_ssl_login
@desc 
    ftps(ftp over ssl) login
@param 
    nwy_ftp_login_t *ftp_param [in]: Login parameter structure 
    nwy_app_ssl_conf_t *ssl_cfg [in]: SSL config
    int ftps_type[in]: ftps type(explicit/implicit)
    resultcb cb [in]: Callback notification function
@return
    0: on success;
    -1: on failed 
@other
  Once ftp login is completed successfully, the callback function cb will be triggered. 
  NWY_FTP_EVENT_LOGIN event will report
*/
int nwy_ftp_ssl_login(nwy_ftp_login_t *ftp_param, nwy_app_ssl_conf_t *ssl_cfg, int ftps_type, resultcb cb);

/*
@func 
    nwy_ftp_get
@desc 
    get file from remote server
@param 
    const char* filename [in]: The file name to get 
    uint8_t type [in]: File transfer mode 1£ºASCII 2£ºBinary
    int offset [in]: the offset of the file content to be fetched
    int len [in]: the len of the file content to be fetched
@return
    0: on success;
    -1: on failed 
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_DATA_RECVED :recv data 
  NWY_FTP_EVENT_DATA_GET :get file
  NWY_FTP_EVENT_DATA_CLOSED : data path disconnected
*/

int nwy_ftp_get(const char* filename, uint8_t type, int offset, int len);

/*
@func 
    nwy_ftp_put
@desc 
    put file from remote server
@param 
    const char* filename [in]: The file name to get 
    uint8_t type [in]: File transfer mode 1£ºASCII 2£ºBinary
    uint8_t mode [in]: Operating mode 
           1£ºSTOR mode Create a file on the server to write data, if the file exists, overwrite the source file
           2: APPE mode Create a file on the server to write data, if the file exists, append to the end of the file
           3: DELE delete a file
    const char *data [in]:Data content to be uploaded
    int lne [in]: the length of the data
@return
    0: on success;
    -1: on failed 
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_DATA_PUT_FINISHED: the data have be uploaded success.
  
*/

int nwy_ftp_put(const char* filename, uint8_t type ,uint8_t mode, const char *data, int len);

/*
@func 
    nwy_ftp_filesize
@desc 
    get file size
@param 
    const char* filename [in]: The file name to getsize
    uint16 tout [in]: timeout
    
@return
    0: on success;
    -1: on failed 
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_SIZE notify the file length
  
*/

int nwy_ftp_filesize(const char* filename, uint16 tout);

/*
@func 
    nwy_ftp_logout
@desc 
    ftp logout
@param 
    uint16 tout [in]: timeout
    
@return
    0: on success;
    -1: on failed 
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_LOGOUT notify success logout
  
*/

int nwy_ftp_logout(uint16 tout);

/*
@func 
    nwy_ftp_mk_dir
@desc 
    make new dir in ftp server
@param 
    const char* dir [in]: dir name
    
@return
    0: on success;
    -1: on failed 
@other
  Once action is completed successfully, the callback function cb will be triggered. The NWY_FTP_EVENT_MK_DIR_OK
  or NWY_FTP_EVENT_MK_DIR_ERR will be report to notify ok or err.
  
*/
int nwy_ftp_mk_dir(const char* dir);

/*
@func 
    nwy_ftp_rm_dir
@desc 
    remove dir in ftp server
@param 
    const char* dir [in]: dir name
    
@return
    0: on success;
    -1: on failed 
@other
  Once action is completed successfully, the callback function cb will be triggered. The NWY_FTP_EVENT_RM_DIR_OK
  or NWY_FTP_EVENT_RM_DIR_ERR will be report to notify ok or err.
  
*/
int nwy_ftp_rm_dir(const char* dir);


/*
@func
    nwy_multiplex_ftp_login
@desc
    ftp login
@param
    nwy_ftp_login_t *ftp_param [in]: Login parameter structure
    resultcb cb [in]: Callback notification function
@return
    0: on success;
    -1: on failed 
@other
  Once ftp login is completed successfully, the callback function cb will be triggered.
  NWY_FTP_EVENT_LOGIN event will report
*/

int nwy_multiplex_ftp_login(nwy_ftp_login_t *ftp_param, resultcb cb);

/*
@func
    nwy_ftp_get
@desc
    get file from remote server
@param
    uint16_t channel: multiplex index (1-7)
    const char* filename [in]: The file name to get
    uint8_t type [in]: File transfer mode 1£ºASCII 2£ºBinary
    int offset [in]: the offset of the file content to be fetched
    int len [in]: the len of the file content to be fetched
@return
    0: on success;
    -1: on failed
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_DATA_RECVED :recv data 
  NWY_FTP_EVENT_DATA_GET :get file
  NWY_FTP_EVENT_DATA_CLOSED : data path disconnected
*/

int nwy_multiplex_ftp_get(uint16_t channel, const char* filename, uint8_t type, int offset, int len);

/*
@func
    nwy_multiplex_ftp_put
@desc
    put file from remote server
@param
    uint16_t channel: multiplex index (1-7)
    const char* filename [in]: The file name to get
    uint8_t type [in]: File transfer mode 1£ºASCII 2£ºBinary
    uint8_t mode [in]: Operating mode
           1£ºSTOR mode Create a file on the server to write data, if the file exists, overwrite the source file
           2: APPE mode Create a file on the server to write data, if the file exists, append to the end of the file
           3: DELE delete a file
    const char *data [in]:Data content to be uploaded
    int lne [in]: the length of the data
@return
    0: on success;
    -1: on failed 
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_DATA_PUT_FINISHED: the data have be uploaded success.
  
*/

int nwy_multiplex_ftp_put(uint16_t channel, const char* filename, uint8_t type ,uint8_t mode, const char *data, int len);

/*
@func
    nwy_multiplex_ftp_filesize
@desc
    get file size
@param
    uint16_t channel: multiplex index (1-7)
    const char* filename [in]: The file name to getsize
    uint16 tout [in]: timeout
    
@return
    0: on success;
    -1: on failed
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_SIZE notify the file length
  
*/

int nwy_multiplex_ftp_filesize(uint16_t channel, const char* filename, uint16 tout);

/*
@func
    nwy_multiplex_ftp_logout
@desc
    ftp logout
@param
    uint16_t channel: multiplex index (1-7)
    uint16 tout [in]: timeout
    
@return
    0: on success;
    -1: on failed
@other
  Once action is completed successfully, the callback function cb will be triggered. some event will be report
  NWY_FTP_EVENT_LOGOUT notify success logout
  
*/

int nwy_multiplex_ftp_logout(uint16_t channel, uint16 tout);

#ifdef __cplusplus
}
#endif
#endif //__NWY_FTP_H__ endif
