/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(__MQTT_LINUX_)
#define __MQTT_LINUX_

#if defined(WIN32_DLL) || defined(WIN64_DLL)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#elif defined(LINUX_SO)
  #define DLLImport extern
  #define DLLExport  __attribute__ ((visibility ("default")))
#else
  #define DLLImport
  #define DLLExport
#endif


/*
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <pthread.h>
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>


//#include "error.h"
#include "ssl.h"
#include "net.h"
#include "x509_crt.h"
//#include "pk.h"
//#include "debug.h"
//#include "platform.h"
#include "entropy.h"
#include "ctr_drbg.h"

#define MQTT_TASK
/*Begin: Add by shuiying for fix error  by typedef  unsigned char      boolean<_com_dtypes.h> in 2020.08.17*/
typedef int boolean_t;
/*End:  Add by shuiying for fix error by typedef  unsigned char      boolean<_com_dtypes.h> in 2020.08.17*/
typedef signed int s32;

typedef struct Timer
{
#if 0
struct timeval end_time;
#else
uint32_t end_time;
#endif

} Timer;

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);
void nwy_set_ssl_hostname(int set_flag);


/*Begin: Add by yjj in 2018.05.07 for support SSL/TLS*/
//#ifdef FEAYURE_NWY_PAHO
typedef struct {
	char *pRootCALocation;                ///< Pointer to string containing the filename (including path) of the root CA file.
	char *pDeviceCertLocation;            ///< Pointer to string containing the filename (including path) of the device certificate.
	char *pDevicePrivateKeyLocation;    ///< Pointer to string containing the filename (including path) of the device private key file.
	char *pDestinationURL;                ///< Pointer to string containing the endpoint of the MQTT service.
	uint16_t DestinationPort;            ///< Integer defining the connection port of the MQTT service.
	uint32_t timeout_ms;                ///< Unsigned integer defining the TLS handshake timeout value in milliseconds.
	boolean_t ServerVerificationFlag;        ///< Boolean.  True = perform server certificate hostname validation.  False = skip validation \b NOT recommended.
	uint8_t sslversion                     //sslversion default value = 3   0 == SSL3.0 1 == TLS v1.0  2 == TLS v1.1 3 == TLS v1.2
} TLSConnectParams;
/*End: Add by yjj in 2018.05.07 for support SSL/TLS*/

typedef struct _TLSDataParams {
     mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	uint32_t flags;
	mbedtls_x509_crt cacert;
	mbedtls_x509_crt clicert;
	mbedtls_pk_context pkey;
	mbedtls_net_context server_fd;
}TLSDataParams;

typedef struct Network
{
	int my_socket;
/*Begin: Add by yjj in 2018.05.07 for support SSL/TLS*/
	//#ifdef FEAYURE_NWY_PAHO
	boolean_t is_SSL;
	TLSConnectParams tlsConnectParams;				///< TLSConnect params structure containing the common connection parameters
	TLSDataParams tlsDataParams;						///< TLSData params structure containing the connection data parameters that are specific to the library being used
/*End: Add by yjj in 2018.05.07 for support SSL/TLS*/ 

	int (*mqttread) (struct Network*, unsigned char*, int, int);
	int (*mqttwrite) (struct Network*, unsigned char*, int, int);
} Network;

int linux_read(Network*, unsigned char*, int, int);
int linux_write(Network*, unsigned char*, int, int);

DLLExport void NetworkInit(Network*);
DLLExport int NetworkConnect(Network*, char*, int);
DLLExport void NetworkDisconnect(Network*);

/*Begin: Add by yjj in 2018.05.07 for support PAHO MQTT*/
	//#ifdef FEAYURE_NWY_PAHO
typedef enum {
	/** Returned when the Network physical layer is connected */
			NETWORK_PHYSICAL_LAYER_CONNECTED = 6,
	/** Returned when the Network is manually disconnected */
			NETWORK_MANUALLY_DISCONNECTED = 5,
	/** Returned when the Network is disconnected and the reconnect attempt is in progress */
			NETWORK_ATTEMPTING_RECONNECT = 4,
	/** Return value of yield function to indicate auto-reconnect was successful */
			NETWORK_RECONNECTED = 3,
	/** Returned when a read attempt is made on the TLS buffer and it is empty */
			MQTT_NOTHING_TO_READ = 2,
	/** Returned when a connection request is successful and packet response is connection accepted */
			MQTT_CONNACK_CONNECTION_ACCEPTED = 1,
	/** Success return value - no error occurred */
			IoT_SUCCESS = 0,
	/** A generic error. Not enough information for a specific error code */
			IoT_FAILURE = -1,
	/** A required parameter was passed as null */
             PAHO_NULL_VALUE_ERROR = -2,
	/** The TCP socket could not be established */
			TCP_CONNECTION_ERROR = -3,
	/** The TLS handshake failed */
			SSL_CONNECTION_ERROR = -4,
	/** Error associated with setting up the parameters of a Socket */
			TCP_SETUP_ERROR = -5,
	/** A timeout occurred while waiting for the TLS handshake to complete. */
			NETWORK_SSL_CONNECT_TIMEOUT_ERROR = -6,
	/** A Generic write error based on the platform used */
			NETWORK_SSL_WRITE_ERROR = -7,
	/** SSL initialization error at the TLS layer */
			NETWORK_SSL_INIT_ERROR = -8,
	/** An error occurred when loading the certificates.  The certificates could not be located or are incorrectly formatted. */
			NETWORK_SSL_CERT_ERROR = -9,
	/** SSL Write times out */
			NETWORK_SSL_WRITE_TIMEOUT_ERROR = -10,
	/** SSL Read times out */
			NETWORK_SSL_READ_TIMEOUT_ERROR = -11,
	/** A Generic error based on the platform used */
			NETWORK_SSL_READ_ERROR = -12,
	/** Returned when the Network is disconnected and reconnect is either disabled or physical layer is disconnected */
			NETWORK_DISCONNECTED_ERROR = -13,
	/** Returned when the Network is disconnected and the reconnect attempt has timed out */
              PAHO_NETWORK_RECONNECT_TIMED_OUT_ERROR = -14,
	/** Returned when the Network is already connected and a connection attempt is made */
			NETWORK_ALREADY_CONNECTED_ERROR = -15,
	/** Network layer Error Codes */
	/** Network layer Random number generator seeding failed */
			NETWORK_MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED = -16,
	/** A generic error code for Network layer errors */
			NETWORK_SSL_UNKNOWN_ERROR = -17,
	/** Returned when the physical layer is disconnected */
			NETWORK_PHYSICAL_LAYER_DISCONNECTED = -18,
	/** Returned when the root certificate is invalid */
			NETWORK_X509_ROOT_CRT_PARSE_ERROR = -19,
	/** Returned when the device certificate is invalid */
			NETWORK_X509_DEVICE_CRT_PARSE_ERROR = -20,
	/** Returned when the private key failed to parse */
			NETWORK_PK_PRIVATE_KEY_PARSE_ERROR = -21,
	/** Returned when the network layer failed to open a socket */
			NETWORK_ERR_NET_SOCKET_FAILED = -22,
	/** Returned when the server is unknown */
			NETWORK_ERR_NET_UNKNOWN_HOST = -23,
	/** Returned when connect request failed */
			NETWORK_ERR_NET_CONNECT_FAILED = -24,
	/** Returned when there is nothing to read in the TLS read buffer */
			NETWORK_SSL_NOTHING_TO_READ = -25,
	/** A connection could not be established. */
			MQTT_CONNECTION_ERROR = -26,
	/** A timeout occurred while waiting for the TLS handshake to complete */
			MQTT_CONNECT_TIMEOUT_ERROR = -27,
	/** A timeout occurred while waiting for the TLS request complete */
			MQTT_REQUEST_TIMEOUT_ERROR = -28,
	/** The current client state does not match the expected value */
			MQTT_UNEXPECTED_CLIENT_STATE_ERROR = -29,
	/** The client state is not idle when request is being made */
			MQTT_CLIENT_NOT_IDLE_ERROR = -30,
	/** The MQTT RX buffer received corrupt or unexpected message  */
			MQTT_RX_MESSAGE_PACKET_TYPE_INVALID_ERROR = -31,
	/** The MQTT RX buffer received a bigger message. The message will be dropped  */
			MQTT_RX_BUFFER_TOO_SHORT_ERROR = -32,
	/** The MQTT TX buffer is too short for the outgoing message. Request will fail  */
			MQTT_TX_BUFFER_TOO_SHORT_ERROR = -33,
	/** The client is subscribed to the maximum possible number of subscriptions  */
			MQTT_MAX_SUBSCRIPTIONS_REACHED_ERROR = -34,
	/** Failed to decode the remaining packet length on incoming packet */
			MQTT_DECODE_REMAINING_LENGTH_ERROR = -35,
	/** Connect request failed with the server returning an unknown error */
            PAHO_MQTT_CONNACK_UNKNOWN_ERROR = -36,
	/** Connect request failed with the server returning an unacceptable protocol version error */
			MQTT_CONNACK_UNACCEPTABLE_PROTOCOL_VERSION_ERROR = -37,
              PAHO_MQTT_CONNACK_IDENTIFIER_REJECTED_ERROR = -38,
          /** Connect request failed with the server returning an unavailable error */
              PAHO_MQTT_CONNACK_SERVER_UNAVAILABLE_ERROR = -39,
          /** Connect request failed with the server returning a bad userdata error */
              PAHO_MQTT_CONNACK_BAD_USERDATA_ERROR = -40,
          /** Connect request failed with the server failing to authenticate the request */
              PAHO_MQTT_CONNACK_NOT_AUTHORIZED_ERROR = -41,
	/** An error occurred while parsing the JSON string.  Usually malformed JSON. */
			JSON_PARSE_ERROR = -42,
	/** Shadow: The response Ack table is currently full waiting for previously published updates */
			SHADOW_WAIT_FOR_PUBLISH = -43,
	/** Any time an snprintf writes more than size value, this error will be returned */
			SHADOW_JSON_BUFFER_TRUNCATED = -44,
	/** Any time an snprintf encounters an encoding error or not enough space in the given buffer */
			SHADOW_JSON_ERROR = -45,
	/** Mutex initialization failed */
			MUTEX_INIT_ERROR = -46,
	/** Mutex lock request failed */
			MUTEX_LOCK_ERROR = -47,
	/** Mutex unlock request failed */
			MUTEX_UNLOCK_ERROR = -48,
	/** Mutex destroy failed */
			MUTEX_DESTROY_ERROR = -49,
	/** Input argument exceeded the allowed maximum size */
			MAX_SIZE_ERROR = -50,
	/** Some limit has been exceeded, e.g. the maximum number of subscriptions has been reached */
			LIMIT_EXCEEDED_ERROR = -51,
	/** Invalid input topic type */
			INVALID_TOPIC_TYPE_ERROR = -52
} IoT_Error_t;
/*End: Add by yjj in 2018.05.07 for support paho mqtt*/ 


/**
 * @brief Mutex Type
 *
 * definition of the Mutex	 struct. Platform specific
 *
 */

typedef void* Mutex;
/**
 * @brief Initialize the provided mutex
 *
 * Call this function to initialize the mutex
 *
 * @param IoT_Mutex_t - pointer to the mutex to be initialized
 * @return IoT_Error_t - error code indicating result of operation
 */
extern IoT_Error_t MutexInit(Mutex *);

/**
 * @brief Lock the provided mutex
 *
 * Call this function to lock the mutex before performing a state change
 * This is a blocking call.
 *
 * @param IoT_Mutex_t - pointer to the mutex to be locked
 * @return IoT_Error_t - error code indicating result of operation
 */
extern IoT_Error_t MutexLock(Mutex *);

/**
 * @brief Lock the provided mutex
 *
 * Call this function to lock the mutex before performing a state change.
 * This is not a blocking call.
 *
 * @param IoT_Mutex_t - pointer to the mutex to be locked
 * @return IoT_Error_t - error code indicating result of operation
 */
extern IoT_Error_t Mutex_Trylock(Mutex *);

/**
 * @brief Unlock the provided mutex
 *
 * Call this function to unlock the mutex before performing a state change
 *
 * @param IoT_Mutex_t - pointer to the mutex to be unlocked
 * @return IoT_Error_t - error code indicating result of operation
 */
extern IoT_Error_t MutexUnlock(Mutex *);

/**
 * @brief Destroy the provided mutex
 *
 * Call this function to destroy the mutex
 *
 * @param IoT_Mutex_t - pointer to the mutex to be destroyed
 * @return IoT_Error_t - error code indicating result of operation
 */
extern IoT_Error_t MutexDestroy(Mutex *);
//extern void nwy_Printf( const char *fmt, ...);

#endif
