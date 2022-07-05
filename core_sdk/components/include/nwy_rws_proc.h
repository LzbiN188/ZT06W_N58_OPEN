#ifndef __NWY_RWS_PROC_H__
#define __NWY_RWS_PROC_H__ 1

#ifdef __cplusplus
extern "C"
{
#endif

#define NWY_WS_DISCONNECTED      0
#define NWY_WS_CONNECTED         1

typedef struct 
{
  unsigned char *cert_data;
  unsigned int cert_len;
}nwy_rws_ssl_cert_type;

typedef struct 
{
  int ssl_version;
  int authmode;
  nwy_rws_ssl_cert_type cacert;
  nwy_rws_ssl_cert_type clientcert;
  nwy_rws_ssl_cert_type clientkey;
}nwy_rws_ssl_conf_type;

typedef struct _nwy_rws_opt
{
	int ping_interval;  /*seconds */
	int ping_timout;  /* seconds */
	int should_handshake;/* if need handshake,the value is 1,ohter 0 */
	char *origin; /* if  */
	char *key; /* if input,used the key ,othe used default value */
  char *proct;
  nwy_rws_ssl_conf_type *ssl_conf;
}nwy_rws_opt_t;


/* the data call back
* parameter:
*          @t: the handle which the nwy_ws_open return;
*	       @data: the received data
*          @len: the data length
*		   @arg: the user input parameter
*/
typedef void (*nwy_rws_data_fun_cb)(void *t, const void *data, unsigned int len, void *arg);
/* the session state call back
* parameter:
*          @t: the handle which the nwy_ws_open return;
*	       @reason: default value 0
*		   @arg: the user input parameter
*/
typedef void (*nwy_rws_close_fun_cb)(void *t,int reason, void *arg);

/* the session status call back
* parameter:
*          @t: the handle which the nwy_ws_open return;
*          @status: session status 0-disconnected 1-connected
*          @arg: the user input parameter
*/
typedef void (*nwy_rws_status_fun_cb)(void *t,int status, void *arg);

/* nwy_ws_open:open the session
*  parameter:
*            @opt: the option,please input NULL;
*            @data_cb: the data callback of the session
*            @close_cb: the closed callback of the session
*            @arg: the user data of the func data_cb and close_cb
*  return: 
			@return the handle of the session if success,NUll if failed;
*/
void *nwy_ws_open(const char *url,const nwy_rws_opt_t *opt,nwy_rws_data_fun_cb data_cb,nwy_rws_close_fun_cb close_cb, void *arg);


/* nwy_ws_open:open the session with status cb
*  parameter:
*            @opt: the option,please input NULL;
*            @data_cb: the data callback of the session
*            @st_cb: the status callback of the session
*            @arg: the user data of the func data_cb and close_cb
*  return: 
			@return the handle of the session if success,NUll if failed;
*/
void *nwy_ws_open_with_st(const char *url,const nwy_rws_opt_t *opt,nwy_rws_data_fun_cb data_cb,nwy_rws_status_fun_cb st_cb, void *arg);


/* nwy_ws_send_binary: send the binary data 
*  parameter:
*            @ws_client: the handle of the session;
*            @data: the data
*            @size: the length of the data
*  return: 
			@return the 0 if success,non 0 if failed;
*/
int nwy_ws_send_binary(void *ws_client, const void *data, unsigned int size);

/* nwy_rws_send_text: send the text data 
*  parameter:
*            @ws_client: the handle of the session;
*            @data: the data
*  return: 
			@return the 0 if success,non 0 if failed;
*/
int nwy_rws_send_text(void *ws_client, const char *data);

/* nwy_rws_close: close the session 
*  parameter:
*            @ws_client: the handle of the session;
*  return: 
			@return the 0 if success,non 0 if failed;
*/
int nwy_rws_close(void * ws_client);

#ifdef __cplusplus
}
#endif

#endif/*end __NWY_RWS_PROC_H__ */

