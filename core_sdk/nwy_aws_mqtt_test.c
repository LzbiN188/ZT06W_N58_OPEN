/*
 *   Copyright (c) 2014 - 2019 Oleh Kulykov <info@resident.name>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include "nwy_osi_api.h"
#include "nwy_common.h"
#include "osi_log.h"

#include "aws_iot_mqtt_client.h"
#include "aws_iot_log.h"
#include "aws_iot_mqtt_client_interface.h"

#define NWY_AWS_MQTT_MAX_CERT_SIZE  (4096)
#define NWY_AWS_MQTT_MAX_CERT_NAME_LEN (64)
#define NWY_AWS_MQTT_MAX_KEY_NAME_LEN (64)
#define NWY_EXT_SIO_RX_MAX          (2 * 1024)


#define NWY_CA_NAME "/nwy_cert/rootca.pem"
#define NWY_CLEINT_NAME "/nwy_cert/client_cert.cert.pem"
#define NWY_CLIENT_PRIVATE_NAME "/nwy_cert/clinet_key.private.key"

typedef struct{
    uint8_t ca_cert_name[NWY_AWS_MQTT_MAX_CERT_NAME_LEN + 32];
    uint8_t client_cert_name[NWY_AWS_MQTT_MAX_CERT_NAME_LEN + 32];
    uint8_t client_key_name[NWY_AWS_MQTT_MAX_KEY_NAME_LEN + 32];
    uint32_t ca_cert_len;
    uint32_t client_cert_len;
    uint32_t client_key_len;
}nwy_aws_mqtt_cert_t;


AWS_IoT_Client   g_aws_mqtt_Client = { 0 };
static nwy_osiThread_t * g_nwy_aws_mqtt_task_id = NULL;
static uint8_t                      g_nwy_mqtt_state = 0;

nwy_aws_mqtt_cert_t g_nwy_aws_mqtt_cert;
extern  char nwy_ext_sio_recv_buff[NWY_EXT_SIO_RX_MAX + 1];

static nwy_aws_mqtt_menu()
{
    nwy_ext_echo("\r\nPlease select an option to data test from the items listed below. \r\n");
    nwy_ext_echo("1. aws_mqtt connect\r\n");
    nwy_ext_echo("2. aws_mqtt disconnect \r\n");
    nwy_ext_echo("3. aws_mqtt publlish\r\n");
    nwy_ext_echo("4. aws_mqtt subscribe\r\n");
    nwy_ext_echo("5. Exit tcpfunction\r\n");
}

static void nwy_aws_mqtt_disconnect_cb(AWS_IoT_Client *pClient, void *data)
{
    IOT_WARN("MQTT Disconnect");

}

static void nwy_aws_mqtt_cycle(void)
{
    OSI_LOGI(0,"AWS nwy_awsmqtt_cycle");

    for (;;)
    {
        IoT_Error_t rc        = SUCCESS;
        while (g_nwy_mqtt_state && ((rc >= 0) || (rc == MQTT_CLIENT_NOT_IDLE_ERROR)))
        {
            rc = aws_iot_mqtt_yield(&g_aws_mqtt_Client, 100);
            if(NETWORK_ATTEMPTING_RECONNECT == rc) {
               // If the client is attempting to reconnect we will skip the rest of the loop.
                   continue;
              }
            osiThreadSleep(300);
        }
        OSI_LOGI(0, "Out AWS cycle");
        g_nwy_mqtt_state = 0;
        rc = 0;
        osiThreadSuspend(g_nwy_aws_mqtt_task_id);
    }

}

static nwy_osiThread_t * nwy_aws_mqtt_yeild_task(void)
{
  if (g_nwy_aws_mqtt_task_id == NULL)
  {
    nwy_osiThread_t * task_id = nwy_create_thread ("nwy_aws_yeild_task",nwy_aws_mqtt_cycle,
                   NULL, NWY_OSI_PRIORITY_NORMAL,1024*15,4);
    g_nwy_aws_mqtt_task_id = task_id;
  }
  else
  {
    nwy_resume_thread(g_nwy_aws_mqtt_task_id);
  }

  return g_nwy_aws_mqtt_task_id;
}

void nwy_aws_mqtt_connect()
{
    IoT_Error_t ret  = FAILURE;
    char url[128] = {0};
    int autoconnect = 0;
    char* sptr = nwy_ext_sio_recv_buff;
    IoT_Client_Init_Params     nwy_mqttinit_params = IoT_Client_Init_Params_initializer;
    IoT_Client_Connect_Params  nwy_connect_params  = IoT_Client_Connect_Params_initializer;

    ret = aws_iot_mqtt_is_client_connected(&g_aws_mqtt_Client);
    if(ret == 1)
    {
        nwy_ext_echo("\r\n aws_mqtt has already connect");
        return;
    }

    nwy_mqttinit_params.pRootCALocation= NWY_CLEINT_NAME;
    nwy_mqttinit_params.pDeviceCertLocation = NWY_CLEINT_NAME;
    nwy_mqttinit_params.pDevicePrivateKeyLocation = NWY_CLIENT_PRIVATE_NAME;


    nwy_ext_input_gets("\r\nPlease host url: ");
    memcpy(url, sptr, strlen(sptr));


    nwy_ext_input_gets("\r\nPlease url port: ");

    nwy_mqttinit_params.pHostURL = url;
    nwy_mqttinit_params.port = atoi(sptr);
    nwy_ext_input_gets("\r\nPlease auto connect(0-1): ");
    autoconnect = atoi(sptr);
    nwy_mqttinit_params.enableAutoReconnect = autoconnect;


    nwy_mqttinit_params.mqttPacketTimeout_ms = 5000;
    nwy_mqttinit_params.mqttCommandTimeout_ms = 20000;
    nwy_mqttinit_params.tlsHandshakeTimeout_ms = 20000;

    nwy_mqttinit_params.disconnectHandler = nwy_aws_mqtt_disconnect_cb;
    nwy_mqttinit_params.disconnectHandlerData = NULL;
    nwy_mqttinit_params.isSSLHostnameVerify = 1;
    nwy_aws_log_print("aws aws_iot_mqtt_init isSSLHostnameVerify = %d", nwy_mqttinit_params.isSSLHostnameVerify);
    ret = aws_iot_mqtt_init(&g_aws_mqtt_Client, &nwy_mqttinit_params);
    if (SUCCESS != ret)
    {
        nwy_ext_echo("\r\n aws_mqtt init fail");
        return;
    }

    nwy_connect_params.keepAliveIntervalInSec = 60;
    nwy_connect_params.isCleanSession = 1;
    nwy_connect_params.MQTTVersion = 4;
    nwy_connect_params.isWillMsgPresent = false;

    OSI_LOGI(0, "aws aws_iot_mqtt_connect");

    ret = aws_iot_mqtt_connect(&g_aws_mqtt_Client, &nwy_connect_params);
    if (SUCCESS != ret)
    {
        nwy_ext_echo("\r\n aws_mqtt connect fail");
        return;
    }
    else
    {

         if(true == autoconnect)
         {
           ret = aws_iot_mqtt_autoreconnect_set_status(&g_aws_mqtt_Client, true);
           if(SUCCESS != ret)
           {
               nwy_ext_echo("Unable to set Auto Reconnect to true - %d", ret);
               return;
           }
         }

         OSI_LOGI(0, "aws nwy_awsmqtt_send_recv_loop");
         g_nwy_mqtt_state = 1;
         nwy_aws_mqtt_yeild_task();
         nwy_ext_echo("\r\n aws mqtt connect success");
    }
}
int nwy_aws_mqtt_disconnect(void)
{
    IoT_Error_t rc  = FAILURE;
    bool ret = true;
    ret = aws_iot_mqtt_is_client_connected(&g_aws_mqtt_Client);
    if(!ret)
    {
        return ret;
    }
    rc = aws_iot_mqtt_autoreconnect_set_status(&g_aws_mqtt_Client, false);
    if(SUCCESS != rc)
    {
      IOT_ERROR("Unable to set Auto Reconnect to false - %d", rc);
      return rc;
    }
    IOT_ERROR("AWS AA100");
    rc = aws_iot_mqtt_disconnect(&g_aws_mqtt_Client);
    if (SUCCESS != rc)
    {
       g_aws_mqtt_Client.clientStatus.clientState = CLIENT_STATE_DISCONNECTED_ERROR;
       g_aws_mqtt_Client.networkStack.disconnect(&(g_aws_mqtt_Client.networkStack));
       g_aws_mqtt_Client.networkStack.destroy(&(g_aws_mqtt_Client.networkStack));
       IOT_ERROR("error aws_iot_mqtt_disconnect rc:%d", rc);

       return rc;
    }
    if(NULL != g_aws_mqtt_Client.clientData.disconnectHandler)
    {
       g_aws_mqtt_Client.clientData.disconnectHandler(&g_aws_mqtt_Client,
                         g_aws_mqtt_Client.clientData.disconnectHandlerData);
    }

    g_nwy_mqtt_state = 0;
    if (rc != SUCCESS) {
        nwy_ext_echo("\r\n aws mqtt disconnect error %d", rc);
    } else {
        nwy_ext_echo("\r\n aws mqtt disconnect success");
    }
    return rc;
}

int nwy_aws_mqtt_publish(void)
{
    char* sptr = nwy_ext_sio_recv_buff;
    char topicname[32] = {0};
    IoT_Error_t rc = FAILURE;
    IoT_Publish_Message_Params paramsQOS0;

    memset(&paramsQOS0, 0, sizeof(paramsQOS0));


    nwy_ext_input_gets("\r\nPlease input topicname: ");
    memcpy(topicname, sptr, strlen(sptr));


    nwy_ext_input_gets("\r\nPlease input payload len: ");
    paramsQOS0.payloadLen = atoi(sptr);

    paramsQOS0.payload = (char *)malloc(paramsQOS0.payloadLen + 1);
    memset(paramsQOS0.payload, 0, (paramsQOS0.payloadLen + 1));

    nwy_ext_input_gets("\r\nPlease input payload info: ");
    memcpy(paramsQOS0.payload, sptr, (paramsQOS0.payloadLen));

    paramsQOS0.qos = QOS0;
    paramsQOS0.isRetained = 0;

    rc = aws_iot_mqtt_publish(&g_aws_mqtt_Client, topicname, strlen(topicname), &paramsQOS0);
    if (rc != SUCCESS) {
        nwy_ext_echo("\r\n aws mqtt publish error %d", rc);
    } else {
        nwy_ext_echo("\r\n aws mqtt publish success");
    }
    return rc;
}

void nwy_aws_mqtt_subscribe_cb(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    nwy_ext_echo("\r\nSubscribe callback");
    nwy_ext_echo("\r\n%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *) params->payload);
}

int nwy_aws_mqtt_subscribe(void)
{
    char* sptr = nwy_ext_sio_recv_buff;
    char topicname[32] = {0};
    IoT_Error_t rc = FAILURE;

    nwy_ext_input_gets("\r\nPlease input topicname: ");
    memcpy(topicname, sptr, strlen(sptr));

    rc = aws_iot_mqtt_subscribe(&g_aws_mqtt_Client, topicname, strlen(topicname), QOS0, nwy_aws_mqtt_subscribe_cb, NULL);
    if (rc != SUCCESS) {
        nwy_ext_echo("\r\n aws mqtt subscribe error %d", rc);
    } else {
        nwy_ext_echo("\r\n aws mqtt subscribe success");
    }
    return rc;
}

int nwy_aws_mqtt_test()
{
    char* sptr = nwy_ext_sio_recv_buff;
    while (1)
    {
        nwy_aws_mqtt_menu();
        nwy_ext_input_gets("\r\nPlease input option: ");
        switch(atoi(sptr)) {
            case 1:
                nwy_aws_mqtt_connect();
                break;
            case 2:
                nwy_aws_mqtt_disconnect();
                break;
            case 3:
                nwy_aws_mqtt_publish();
                break;
            case 4:
                nwy_aws_mqtt_subscribe();
                break;
            case 5:
                return;
        }
    }
}



