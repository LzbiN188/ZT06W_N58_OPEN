// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>

#include "iothub_client.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/platform.h"
#include "iothubtransportmqtt.h"
#include "iothub_client_options.h"
#include "iothub_client_core_common.h"
#include "xlogging.h"
#include "agenttime.h"
#include "azure_c_shared_utility/certs.h"


/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */

//static const char* connectionString = "HostName=neoway-iot.azure-devices.cn;DeviceId=NeowayTest1;SharedAccessKey=uf8NFMWSOIQQ1cmvRAttrymLMGMtUCG099Y1rnF6rw8=";
//"HostName=<host_name>;DeviceId=<device_id>;x509=true" 
static const char* connectionString = "HostName=neoway-iot.azure-devices.cn;DeviceId=nwy_hj;SharedAccessKey=25XhzrhsdbF8h1ciytPyLnVSsTsTxayZ/OpB5hCRzFM=";
static int callbackCounter;
static char msgText[1024];
static char propText[1024];
static bool g_continueRunning;
#define MESSAGE_COUNT 50
#define DOWORK_LOOP_NUM     3
#if 1
typedef struct EVENT_INSTANCE_TAG
{
    IOTHUB_MESSAGE_HANDLE messageHandle;
    size_t messageTrackingId;  // For tracking the messages within the user callback.
} EVENT_INSTANCE;

static void nwy_azure_client_status_cb(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* userContextCallback)
{
    int *c = (int *)userContextCallback;


    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        /*>.need:If the connection is setup successfully, set the state of context.*/
        LogInfo("Change to state NWY_AZURE_CLIENT_CONNECTION_OK   $^_^$   !!!");
        
    }
    else
    {
        switch (reason)
        {
        case IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
            LogError("Change to state NWY_AZURE_CLIENT_EXPIRED_SAS_TOKEN   @_@   !!!");
            break;
        case IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
            LogError("Change to state NWY_AZURE_CLIENT_DEVICE_DISABLED   @_@   !!!");
            break;
        case IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
            LogError("Change to state NWY_AZURE_CLIENT_BAD_CREDENTIAL   @_@   !!!");
            break;
        case IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED:
            LogError("Change to state NWY_AZURE_CLIENT_RETRY_EXPIRED   @_@   !!!");
            break;
        case IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
            LogError("Change to state NWY_AZURE_CLIENT_NO_NETWORK   @_@   !!!");
            break;
        case IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR:
            LogError("Change to state NWY_AZURE_CLIENT_COMMUNICATION_ERROR   @_@   !!!");
            break;
        default:
            break;
        }
    }
}

static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    size_t i = 0;
    int* counter = (int*)user_context;
    const char* buffer;
    size_t size;
    MAP_HANDLE mapProperties;
    const char* messageId;
    const char* correlationId;

    // Message properties
    if ((messageId = IoTHubMessage_GetMessageId(message)) == NULL)
    {
        messageId = "<null>";
    }

    if ((correlationId = IoTHubMessage_GetCorrelationId(message)) == NULL)
    {
        correlationId = "<null>";
    }

    // Message content
    if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        LogError("unable to retrieve the message data\r\n");
    }
    else
    {
        // If we receive the work 'quit' then we stop running
        /*if (size == (strlen("quit") * sizeof(char)) && memcmp(buffer, "quit", size) == 0)
        {
            g_continueRunning = false;
        }*/
        LogInfo("Received Message len = %lu", (int)size);
        /*char *tmp = (char *)malloc(size + 1);
        memcpy(tmp, buffer, size);
        tmp[size] = '\0';*/
        
        LogInfo("Received Message content = %s", buffer);
        //free(tmp);
        //tmp = NULL;
    }

    // Retrieve properties from the message
    mapProperties = IoTHubMessage_Properties(message);
    if (mapProperties != NULL)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
                size_t index;

                LogInfo(" Message Properties:\r\n");
                for (index = 0; index < propertyCount; index++)
                {
                    LogInfo("\tKey: %s Value: %s\r\n", keys[index], values[index]);
                }
                LogInfo("\r\n");
            }
        }
    }

    /* Some device specific action code goes here... */
    (*counter)++;
    return IOTHUBMESSAGE_ACCEPTED;
}



static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
    EVENT_INSTANCE* eventInstance = (EVENT_INSTANCE*)userContextCallback;
    size_t id = eventInstance->messageTrackingId;

    LogError(" nwy_azure Confirmation[%d] received for message tracking id = %d with result = %s\r\n", callbackCounter, (int)id, MU_ENUM_TO_STRING(IOTHUB_CLIENT_CONFIRMATION_RESULT, result));
    /* Some device specific action code goes here... */
    callbackCounter++;
    IoTHubMessage_Destroy(eventInstance->messageHandle);
}

void nwy_azure_test(void)
{

    IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

    EVENT_INSTANCE messages[MESSAGE_COUNT];

    g_continueRunning = true;
    srand((unsigned int)get_time(NULL));
    double avgWindSpeed = 10.0;
    double minTemperature = 20.0;
    double minHumidity = 60.0;

    callbackCounter = 0;
    int receiveContext = 0;
    int statusContext = 0;
    LogInfo("nwy_azure iothub_client_sample_mqtt_esp8266_run enter");

        if ((iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol)) == NULL)
        {
            (void)LogError("ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            bool traceOn = true;
            int timeout = 20000;
            //IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &traceOn);
            //IoTHubClient_LL_SetOption(iotHubClientHandle, OPTION_CONNECTION_TIMEOUT, &timeout);
           if (IoTHubClient_LL_SetOption(iotHubClientHandle, "TrustedCerts", azure_certificates)!= IOTHUB_CLIENT_OK)
            {
                LogError("ERROR: IoTHubClient_LL_SetOption..........FAILED!");
                return ;
            }

            if (IoTHubClient_LL_SetConnectionStatusCallback(iotHubClientHandle, nwy_azure_client_status_cb, &statusContext) != IOTHUB_CLIENT_OK)
            {
                LogError("ERROR: IoTHubClient_LL_SetConnectionStatusCallback..........FAILED!");
                return ;
            }
            LogInfo("IoTHubClient_LL_SetMessageCallback();");

            /* Setting Message call back, so we can receive Commands. */
            if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, &receiveContext) != IOTHUB_CLIENT_OK)
            {
                (void)LogInfo("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
            }
            else
            {
                (void)LogInfo("IoTHubClient_LL_SetMessageCallback...successful.\r\n");

                /* Now that we are ready to receive commands, let's send some messages */
                int iterator = 0;
                double temperature = 0;
                double humidity = 0;
                do
                {
                    LogInfo("iterator: [%d], callbackCounter: [%d]. \r\n", iterator, callbackCounter);

                    if (iterator < MESSAGE_COUNT && (iterator <= callbackCounter))
                    {
                    
                        temperature = minTemperature + (rand() % 10);
                        humidity = minHumidity +  (rand() % 20);
                        sprintf_s(msgText, sizeof(msgText), "{\"deviceId\":\"myFirstDevice\",\"windSpeed\":%.2f,\"temperature\":%.2f,\"humidity\":%.2f}", avgWindSpeed + (rand() % 4 + 2), temperature, humidity);

                        if ((messages[iterator].messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)msgText, strlen(msgText))) == NULL)
                        {
                            LogInfo("ERROR: iotHubMessageHandle is NULL!\r\n");
                        }
                        else
                        {
                            messages[iterator].messageTrackingId = iterator;
                            MAP_HANDLE propMap = IoTHubMessage_Properties(messages[iterator].messageHandle);
                            (void)sprintf_s(propText, sizeof(propText), temperature > 28 ? "true" : "false");
                            if (Map_AddOrUpdate(propMap, "temperatureAlert", propText) != MAP_OK)
                            {
                                LogInfo("ERROR: Map_AddOrUpdate Failed!\r\n");
                            }

                            if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messages[iterator].messageHandle, SendConfirmationCallback, &messages[iterator]) != IOTHUB_CLIENT_OK)
                            {
                                LogInfo("ERROR: IoTHubClient_LL_SendEventAsync..........FAILED!\r\n");
                            }
                            else
                            {
                                LogInfo("IoTHubClient_LL_SendEventAsync accepted message [%d] for transmission to IoT Hub.\r\n", (int)iterator);
                            }
                        }
                        
                        iterator++;
                    }
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(1000);

                    if (callbackCounter >= MESSAGE_COUNT)
                    {
                        LogInfo("exit\n");
                        break;
                    }
                } while (g_continueRunning);

                (void)LogInfo("iothub_client_sample_mqtt has gotten quit message, call DoWork %d more time to complete final sending...\r\n", DOWORK_LOOP_NUM);

            }
            
            // Loop while we wait for messages to drain off.
                size_t index = 0;
                for (index = 0; index < DOWORK_LOOP_NUM; index++)
                {
                    IoTHubClient_LL_DoWork(iotHubClientHandle);
                    ThreadAPI_Sleep(1000);
                }
                IoTHubClient_LL_Destroy(iotHubClientHandle);
        }
    
}
#endif



