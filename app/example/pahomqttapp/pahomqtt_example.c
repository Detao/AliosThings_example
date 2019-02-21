/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "app_entry.h"

#define PRODUCT_KEY             "a1QvH99Xn2Z"
#define PRODUCT_SECRET          "sTg5O2JTktfULCGr"
#define DEVICE_NAME             "73RAl8thmfbtNsyiPOXN"
#define DEVICE_SECRET           "nXmZeUw9yhctt54OFGA6p5mcw5QqXavT"
#define HOST_NAME   "183.230.40.39"
#define DEVICE_ID   "515068087"
#define PRO_ID      "206452"
#define AUTH_INFO  "led1"

char __product_key[PRODUCT_KEY_LEN + 1];
char __device_name[DEVICE_NAME_LEN + 1];
char __device_secret[DEVICE_SECRET_LEN + 1];

/* These are pre-defined topics */
#define TOPIC_POST            "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_SET               "/sys//"PRODUCT_KEY"/"DEVICE_NAME"/thing/service/property/set"
#define TOPIC_DATA              "/"PRODUCT_KEY"/"DEVICE_NAME"/data"


/* These are pre-defined topics format*/
#define TOPIC_UPDATE_FMT            "/%s/%s/update"
#define TOPIC_ERROR_FMT             "/%s/%s/update/error"
#define TOPIC_GET_FMT               "/%s/%s/get"
#define TOPIC_DATA_FMT              "/%s/%s/data"

#define MQTT_MSGLEN             (1024)

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, #__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static int      user_argc;
static char   **user_argv;

void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_UNDEF:
            EXAMPLE_TRACE("undefined event occur.");
            break;

        case IOTX_MQTT_EVENT_DISCONNECT:
            EXAMPLE_TRACE("MQTT disconnect.");
            break;

        case IOTX_MQTT_EVENT_RECONNECT:
            EXAMPLE_TRACE("MQTT reconnect.");
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
            EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
            EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
            EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
            EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
            EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
            EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_NACK:
            EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;

        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                          topic_info->topic_len,
                          topic_info->ptopic,
                          topic_info->payload_len,
                          topic_info->payload);
            break;

        case IOTX_MQTT_EVENT_BUFFER_OVERFLOW:
            EXAMPLE_TRACE("buffer overflow, %s", msg->msg);
            break;

        default:
            EXAMPLE_TRACE("Should NOT arrive here.");
            break;
    }
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    /* print topic name and topic message */
    EXAMPLE_TRACE("----");
    EXAMPLE_TRACE("packetId: %d", ptopic_info->packet_id);
    EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    EXAMPLE_TRACE("----");
}
static void get_dev_info(iotx_conn_info_t* pconn_info)
{
    int len =strlen(HOST_NAME);
    pconn_info->port = 6002;
    strncpy(pconn_info->host_name , HOST_NAME,strlen(HOST_NAME));
    strncpy(pconn_info->client_id , DEVICE_ID,strlen(DEVICE_ID));
    strncpy(pconn_info->username , PRO_ID,strlen(PRO_ID));
    strncpy(pconn_info->password , AUTH_INFO,strlen(AUTH_INFO));
    pconn_info->pub_key = NULL ; 
}
int mqtt_client(void)
{
    int rc = 0, msg_len, cnt = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[128];
    char *msg_buf = NULL, *msg_readbuf = NULL;

    if (NULL == (msg_buf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MQTT_MSGLEN))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }
   // get_dev_info(&pconn_info);
   HAL_GetProductKey(__product_key);
    HAL_GetDeviceName(__device_name);
    HAL_GetDeviceSecret(__device_secret);

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(__product_key, __device_name, __device_secret, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }
    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port =6002;//pconn_info->port;
    mqtt_params.host ="183.230.40.39"; //pconn_info->host_name;
    mqtt_params.client_id = "515068087";//pconn_info->client_id;
    mqtt_params.username ="206452"; //pconn_info->username;
    mqtt_params.password = "led1";//pconn_info->password;
    mqtt_params.pub_key = NULL;//pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MQTT_MSGLEN;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;


    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Initialize topic information */
    // memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    // strcpy(msg_pub, "update: hello! start!");

    // topic_msg.qos = IOTX_MQTT_QOS1;
    // topic_msg.retain = 0;
    // topic_msg.dup = 0;
    // topic_msg.payload = (void *)msg_pub;
    // topic_msg.payload_len = strlen(msg_pub);

    // rc = IOT_MQTT_Publish(pclient, TOPIC_POST, &topic_msg);
    // if (rc < 0) {
    //     IOT_MQTT_Destroy(&pclient);
    //     EXAMPLE_TRACE("error occur when publish");
    //     rc = -1;
    //     goto do_exit;
    // }

    // EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", TOPIC_UPDATE, topic_msg.payload, rc);

    // /* Subscribe the specific topic */
    // rc = IOT_MQTT_Subscribe(pclient, TOPIC_SET, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    // if (rc < 0) {
    //     IOT_MQTT_Destroy(&pclient);
    //     EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
    //     rc = -1;
    //     goto do_exit;
    // }

    // IOT_MQTT_Yield(pclient, 200);

    // HAL_SleepMs(2000);

    /* Initialize topic information */
    // memset(msg_pub, 0x0, 128);
    // strcpy(msg_pub, "data: hello! start!");
    // memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    // topic_msg.qos = IOTX_MQTT_QOS1;
    // topic_msg.retain = 0;
    // topic_msg.dup = 0;
    // topic_msg.payload = (void *)msg_pub;
    // topic_msg.payload_len = strlen(msg_pub);

    // rc = IOT_MQTT_Publish(pclient, TOPIC_DATA, &topic_msg);
    // EXAMPLE_TRACE("\n publish message: \n topic: %s\n payload: \%s\n rc = %d", TOPIC_DATA, topic_msg.payload, rc);

    // IOT_MQTT_Yield(pclient, 200);

    do {
        /* Generate topic message */
        cnt++;
        msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"id\":\"1\",\"version\":\"0.1\",\"params\":{\"temperature\":%d },\"method\":\"thing.event.property.post\"}", cnt);
        if (msg_len < 0) {
            EXAMPLE_TRACE("Error occur! Exit program");
            rc = -1;
            break;
        }

        topic_msg.payload = (void *)msg_pub;
        topic_msg.payload_len = msg_len;

        rc = IOT_MQTT_Publish(pclient, TOPIC_POST, &topic_msg);
        if (rc < 0) {
            EXAMPLE_TRACE("error occur when publish");
            // rc = -1;
            // break;
        }
        EXAMPLE_TRACE("packet-id=%u, publish topic msg=%s", (uint32_t)rc, msg_pub);

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        /* infinite loop if running with 'loop' argument */
        if (user_argc >= 2 && !strcmp("loop", user_argv[1])) {
            HAL_SleepMs(2000);
            cnt = 0;
        }

    } while (cnt < 1);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Unsubscribe(pclient, TOPIC_DATA);

    IOT_MQTT_Yield(pclient, 200);

    IOT_MQTT_Destroy(&pclient);

do_exit:
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    return rc;
}

int linkkit_main(void *paras)
{
    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    user_argc = 0;
    user_argv = NULL;

    if (paras != NULL) {
        app_main_paras_t *p = (app_main_paras_t *)paras;
        user_argc = p->argc;
        user_argv = p->argv;
    }

    HAL_SetProductKey(DEVICE_ID);
    HAL_SetDeviceName(PRO_ID);
    HAL_SetDeviceSecret(AUTH_INFO);
#if defined(SUPPORT_ITLS)
    HAL_SetProductSecret(PRODUCT_SECRET);
#endif
    /* Choose Login Server */
    int domain_type = IOTX_CLOUD_DOMAIN_SH;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&domain_type);

    /* Choose Login  Method */
    int dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    mqtt_client();
    IOT_DumpMemoryStats(IOT_LOG_DEBUG);
    IOT_CloseLog();

    EXAMPLE_TRACE("out of sample!");

    return 0;
}
