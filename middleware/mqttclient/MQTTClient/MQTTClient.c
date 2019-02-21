/*******************************************************************************
 * Copyright (c) 2014, 2017 IBM Corp.
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
 *   Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *   Ian Craggs - fix for #96 - check rem_len in readPacket
 *   Ian Craggs - add ability to set message handler separately #6
 *******************************************************************************/
#include "MQTTClient.h"

#include <stdio.h>
#include <string.h>


#define mqtt_debug(format,...)                                               \
do {                                                                   \
        printf("[debug]: %d %s() | " format "\n", __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                             \
    } while (0);

#define mqtt_err(format,...)                                               \
do {                                                                   \
        printf("[err]: %d %s() | " format "\n", __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                             \
    } while (0);

#define mqtt_info(format,...)                                              \
  do {                                                                   \
        printf("[info]: %d %s() | " format "\n", __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                             \
    } while (0);

#define mqtt_warning(format,...)                                         \
  do {                                                                   \
        printf("[warning]: %d %s() | " format "\n", __LINE__, __FUNCTION__, \
               ##__VA_ARGS__);                                             \
    } while (0);


static int keepalive_sub(MQTTClient *);
static mqtt_state_t get_client_state(MQTTClient *);
static void set_client_state(MQTTClient *,mqtt_state_t);
static int check_state_normal(MQTTClient *);


static void NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessage) {
    md->topicName = aTopicName;
    md->message = aMessage;
}


static int getNextPacketId(MQTTClient *c) {
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}


static int sendPacket(MQTTClient* c, int length, Timer* timer)
{
    int rc = FAILURE;
    int sent = 0;
    unsigned int left_t = 0;
    
    if(!c ) {
        return rc;
    }

    while (sent < length && !TimerIsExpired(timer)){
        // left_t = time_left(timer);
        // left_t = (left_t == 0) ? 1:left_t;
        rc = c->ipstack->mqttwrite(c->ipstack, &c->buf[sent], length, TimerLeftMS(timer));
        if (rc < 0){// there was an error writing the data
            break;
        }  
            
        sent += rc;
    }
    if (sent == length)
    {
    //    TimerCountdown(&c->last_sent, c->keepAliveInterval); // record the fact that we have successfully sent the packet
        rc = SUCCESS;
    }
    else
        rc = FAILURE;
    return rc;
}


void MQTTClientInit(MQTTClient* pClient, Network* network, unsigned int command_timeout_ms,
		unsigned char* sendbuf, size_t sendbuf_size, unsigned char* readbuf, size_t readbuf_size)
{
    int i;
    pClient->ipstack = network;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        pClient->messageHandlers[i].topicFilter = 0;
    pClient->command_timeout_ms = command_timeout_ms;
    pClient->buf = sendbuf;
    pClient->buf_size = sendbuf_size;
    pClient->readbuf = readbuf;
    pClient->readbuf_size = readbuf_size;
    pClient->isconnected = 0;
    pClient->cleansession = 0;
    pClient->ping_outstanding = 0;
    pClient->defaultMessageHandler = NULL;
	  pClient->next_packetid = 1;
    TimerInit(&pClient->last_sent);
    TimerInit(&pClient->last_received);
#if defined(MQTT_TASK)
	  MutexInit(&pClient->mutex);
      MutexInit(pClient->lock_generic);
#endif
}


static int decodePacket(MQTTClient* c, int* value, int timeout)
{
    unsigned char i;
    int multiplier = 1;
    int len = 0;
    const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

    *value = 0;
    do
    {
        int rc = MQTTPACKET_READ_ERROR;

        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
        {
            rc = MQTTPACKET_READ_ERROR; /* bad data */
            goto exit;
        }
        rc = c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
        if (rc != 1)
            goto exit;
        *value += (i & 127) * multiplier;
        multiplier *= 128;
    } while ((i & 128) != 0);
exit:
    return len;
}


static int readPacket(MQTTClient* c, Timer* timer)
{
    MQTTHeader header = {0};
    int len = 0;
    int rem_len = 0;

    /* 1. read the header byte.  This has the packet type in it */
    int rc = c->ipstack->mqttread(c->ipstack, c->readbuf, 1, TimerLeftMS(timer));
    if (rc != 1)
        goto exit;

    len = 1;
    /* 2. read the remaining length.  This is variable in itself */
    decodePacket(c, &rem_len, TimerLeftMS(timer));
    len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

    if (rem_len > (c->readbuf_size - len))
    {
        rc = BUFFER_OVERFLOW;
        goto exit;
    }

    /* 3. read the rest of the buffer using a callback to supply the rest of the data */
    if (rem_len > 0 && (rc = c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, TimerLeftMS(timer)) != rem_len)) {
        rc = 0;
        goto exit;
    }

    header.byte = c->readbuf[0];
    rc = header.bits.type;
    if (c->keepAliveInterval > 0)
        TimerCountdown(&c->last_received, c->keepAliveInterval); // record the fact that we have successfully received a packet
exit:
    return rc;
}


// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}


int deliverMessage(MQTTClient* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].fp != NULL)
            {
                MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].fp(&md);
                rc = SUCCESS;
            }
        }
    }

    if (rc == FAILURE && c->defaultMessageHandler != NULL)
    {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(&md);
        rc = SUCCESS;
    }

    return rc;
}

/* send keepalive packet */
static int MQTTKeepalive(MQTTClient *pClient)
{
    int len = 0;
    int rc  =0 ;
    Timer timer;

    if(!pClient){
        return FAILURE;
    }
    TimerInit(&timer);
    TimerCountdownMS(&timer,1000);
    len = MQTTSerialize_pingreq(pClient->buf, pClient->buf_size);
    if (len > 0 && (rc = sendPacket(pClient, len, &timer)) == SUCCESS) // send the ping packet
                pClient->ping_outstanding = 1;

}
int keepalive(MQTTClient* pClient)
{
    int rc = SUCCESS;
    if(!pClient){
        return;
    }
    
    /* Periodic sending ping packet to detect whether the network is connetced */
    keepalive_sub(pClient);

    mqtt_state_t currentState = get_client_state(pClient);
    do{
        if(STATE_DISCONNECTED_RECONNECTING == currentState){
            MutexLock(pClient->lock_generic);
            pClient->ping_mark = 0;
            MutexUnlock(pClient->lock_generic);

            rc = handle_reconnect(pClient);
            if(SUCCESS != rc){
                mqtt_debug("reconnect network fail, rc = %d",rc);
            }else{
                mqtt_info("network is reconnected!");
                //reconnect_callback(pClient);
            }

            break;
        }

        if(STATE_DISCONNECTED == currentState){
            mqtt_err("network is disconnected");
        }

    }while(0);
    if (TimerIsExpired(&pClient->last_sent) || TimerIsExpired(&pClient->last_received))
    {
        if (pClient->ping_outstanding)
            rc = FAILURE; /* PINGRESP not received in keepalive interval */
        else
        {
            Timer timer;
            TimerInit(&timer);
            TimerCountdownMS(&timer, 1000);
            int len = MQTTSerialize_pingreq(pClient->buf, pClient->buf_size);
            if (len > 0 && (rc = sendPacket(pClient, len, &timer)) == SUCCESS) // send the ping packet
                pClient->ping_outstanding = 1;
        }
    }

exit:
    return rc;
}

static int keepalive_sub(MQTTClient *pClient)
{
    int rc = SUCCESS;

    if(NULL == pClient){
        return FAILURE;
    }

    /* if in disabled state, without having to send ping packets */
    if(check_state_normal(pClient)){
        return SUCCESS;
    }
    
    /* if there is no ping_timer timeout, then return success */
    if(!time_is_expired(&pClient->next_ping_time)){
        return SUCCESS;
    }

    /* update to next time sending MQTT keep-alive */
    TimerCountdownMS(&pClient->next_ping_time,pClient->keepAliveInterval * 1000);
    rc = MQTTKeepalive(pClient);
    if(SUCCESS != rc){
        if(0){
            set_client_state(pClient,STATE_DISCONNECTED);
        }
        mqtt_err("ping outstanding is error,result = %d ",rc);
        return rc;
    }

    mqtt_info("send MQTT ping...");

    MutexLock(pClient->lock_generic);
    pClient->ping_mark = 1;
    pClient->keepalive_probes++;
    MutexUnlock(pClient->lock_generic);

    return SUCCESS;
}
void MQTTCleanSession(MQTTClient* c)
{
    int i = 0;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = NULL;
}


void MQTTCloseSession(MQTTClient* c)
{
    c->ping_outstanding = 0;
    c->isconnected = 0;
    if (c->cleansession)
        MQTTCleanSession(c);
}


int cycle(MQTTClient* c, Timer* timer)
{
    int len = 0,
        rc = SUCCESS;

    int packet_type = readPacket(c, timer);     /* read the socket, see what work is due */

    switch (packet_type)
    {
        default:
            /* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
            rc = packet_type;
            goto exit;
        case 0: /* timed out reading packet */
            break;
        case CONNACK:{
            mqtt_debug("CONNACK");
            break;
        }
        case PUBACK:{
            mqtt_debug("PUBACK");
            break;
        }
        case SUBACK:{
            mqtt_debug("SUBACK");
            break;
        }
        case UNSUBACK:
            break;
        case PUBLISH:
        {
            MQTTString topicName;
            MQTTMessage msg;
            int intQoS;
            msg.payloadlen = 0; /* this is a size_t, but deserialize publish sets this as int */
            if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
               (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                goto exit;
            msg.qos = (enum QoS)intQoS;
            deliverMessage(c, &topicName, &msg);
            if (msg.qos != QOS0)
            {
                if (msg.qos == QOS1)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                else if (msg.qos == QOS2)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                if (len <= 0)
                    rc = FAILURE;
                else
                    rc = sendPacket(c, len, timer);
                if (rc == FAILURE)
                    goto exit; // there was a problem
            }
            break;
        }
        case PUBREC:
        case PUBREL:
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
            else if ((len = MQTTSerialize_ack(c->buf, c->buf_size,
                (packet_type == PUBREC) ? PUBREL : PUBCOMP, 0, mypacketid)) <= 0)
                rc = FAILURE;
            else if ((rc = sendPacket(c, len, timer)) != SUCCESS) // send the PUBREL packet
                rc = FAILURE; // there was a problem
            if (rc == FAILURE)
                goto exit; // there was a problem
            break;
        }

        case PUBCOMP:
            break;
        case PINGRESP:{
            mqtt_debug("PINRESP");
            c->ping_outstanding = 0;
            break;
             
        }
          
    }
   
    // if (keepalive(c) != SUCCESS) {
    //     //check only keepalive FAILURE status so that previous FAILURE status can be considered as FAULT
    //     rc = FAILURE;
    // }

exit:
    if (rc == SUCCESS)
        rc = packet_type;
    else if (c->isconnected)
        MQTTCloseSession(c);
    return rc;
}


int MQTTYield(void *handle, int timeout_ms)
{
    int rc = SUCCESS;
    Timer time;
    MQTTClient *pClient = (MQTTClient *)handle;

   
    if(!handle){
        mqtt_err("Invalid argument,handle is null");
        return -1;
    }

    if(timeout_ms < 0){
        mqtt_err("Invalid argument,timeout_ms = %d",timeout_ms);
        return -1;
    }

    if(timeout_ms == 0){
        timeout_ms = 10;
    }

    TimerInit(&time);
    TimerCountdownMS(&time, timeout_ms);

	do{
        if(SUCCESS != rc){
            unsigned int left_t = TimerLeftMS(&time);
            mqtt_info("error occur ");
            if(left_t < 20){
                HAL_SleepMs(left_t);
            }else{
                HAL_SleepMs(20);
            }
        }
        keepalive(pClient);

        if (cycle(pClient, &time) < 0)
        {
            rc = FAILURE;
            break;
        }
  	} while (!TimerIsExpired(&time));

    return rc;
}

int MQTTIsConnected(MQTTClient* client)
{
  return client->isconnected;
}

void MQTTRun(void* parm)
{
	Timer timer;
	MQTTClient* c = (MQTTClient*)parm;

	TimerInit(&timer);

	while (1)
	{
#if defined(MQTT_TASK)
		MutexLock(&c->mutex);
#endif
		TimerCountdownMS(&timer, 500); /* Don't wait too long if no traffic is incoming */
		cycle(c, &timer);
        aos_msleep(200);
#if defined(MQTT_TASK)
		MutexUnlock(&c->mutex);
#endif
	}
}


#if defined(MQTT_TASK)
int MQTTStartTask(MQTTClient* client)
{
	//return ThreadStart(&client->thread, &MQTTRun, client);
}
#endif


int waitfor(MQTTClient* c, int packet_type, Timer* timer)
{
    int rc = FAILURE;

    do
    {
        if (TimerIsExpired(timer))
            break; // we timed out
        rc = cycle(c, timer);
    }
    while (rc != packet_type && rc >= 0);

    return rc;
}




int MQTTConnectWithResults(MQTTClient* c, MQTTPacket_connectData* options, MQTTConnackData* data)
{
    Timer connect_timer;
    int rc = FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;

#if defined(MQTT_TASK)
	  MutexLock(&c->mutex);
#endif
	  if (c->isconnected) /* don't send connect packet again if we are already connected */
		  goto exit;

    TimerInit(&connect_timer);
    TimerCountdownMS(&connect_timer, c->command_timeout_ms);

    if (options == 0)
        options = &default_options; /* set default options if none were supplied */

    c->keepAliveInterval = options->keepAliveInterval;
    c->cleansession = options->cleansession;
    TimerCountdown(&c->last_received, c->keepAliveInterval);
    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &connect_timer)) != SUCCESS)  // send the connect packet
        goto exit; // there was a problem

    // this will be a blocking call, wait for the connack
    if (waitfor(c, CONNACK, &connect_timer) == CONNACK)
    {
        data->rc = 0;
        data->sessionPresent = 0;
        if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc, c->readbuf, c->readbuf_size) == 1)
            rc = data->rc;
        else
            rc = FAILURE;
    }
    else
        rc = FAILURE;

exit:
    if (rc == SUCCESS)
    {
        c->isconnected = 1;
        c->ping_outstanding = 0;
    }

#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif

    return rc;
}


int MQTTConnect(MQTTClient* c, MQTTPacket_connectData* options)
{
    MQTTConnackData data;
    return MQTTConnectWithResults(c, options, &data);
}


int MQTTSetMessageHandler(MQTTClient* c, const char* topicFilter, messageHandler messageHandler)
{
    int rc = FAILURE;
    int i = -1;

    /* first check for an existing matching slot */
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != NULL && strcmp(c->messageHandlers[i].topicFilter, topicFilter) == 0)
        {
            if (messageHandler == NULL) /* remove existing */
            {
                c->messageHandlers[i].topicFilter = NULL;
                c->messageHandlers[i].fp = NULL;
            }
            rc = SUCCESS; /* return i when adding new subscription */
            break;
        }
    }
    /* if no existing, look for empty slot (unless we are removing) */
    if (messageHandler != NULL) {
        if (rc == FAILURE)
        {
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if (c->messageHandlers[i].topicFilter == NULL)
                {
                    rc = SUCCESS;
                    break;
                }
            }
        }
        if (i < MAX_MESSAGE_HANDLERS)
        {
            c->messageHandlers[i].topicFilter = topicFilter;
            c->messageHandlers[i].fp = messageHandler;
        }
    }
    return rc;
}


int MQTTSubscribeWithResults(MQTTClient* c, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler, MQTTSubackData* data)
{
    int rc = FAILURE;
    Timer timer;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;

#if defined(MQTT_TASK)
	  MutexLock(&c->mutex);
#endif
	  if (!c->isconnected)
		    goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit;             // there was a problem

    if (waitfor(c, SUBACK, &timer) == SUBACK)      // wait for suback
    {
        int count = 0;
        unsigned short mypacketid;
        data->grantedQoS = QOS0;
        if (MQTTDeserialize_suback(&mypacketid, 1, &count, (int*)&data->grantedQoS, c->readbuf, c->readbuf_size) == 1)
        {
            if (data->grantedQoS != 0x80)
                rc = MQTTSetMessageHandler(c, topicFilter, messageHandler);
        }
    }
    else
        rc = FAILURE;

exit:
    if (rc == FAILURE)
        MQTTCloseSession(c);
#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTSubscribe(MQTTClient* c, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler)
{
    MQTTSubackData data;
    return MQTTSubscribeWithResults(c, topicFilter, qos, messageHandler, &data);
}


int MQTTUnsubscribe(MQTTClient* c, const char* topicFilter)
{
    int rc = FAILURE;
    Timer timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

#if defined(MQTT_TASK)
	  MutexLock(&c->mutex);
#endif
	  if (!c->isconnected)
		  goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

    if (waitfor(c, UNSUBACK, &timer) == UNSUBACK)
    {
        unsigned short mypacketid;  // should be the same as the packetid above
        if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
        {
            /* remove the subscription message handler associated with this topic, if there is one */
            MQTTSetMessageHandler(c, topicFilter, NULL);
        }
    }
    else
        rc = FAILURE;

exit:
    if (rc == FAILURE)
        MQTTCloseSession(c);
#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTPublish(MQTTClient* c, const char* topicName, MQTTMessage* message)
{
    int rc = FAILURE;
    Timer timer;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

#if defined(MQTT_TASK)
	  MutexLock(&c->mutex);
#endif
	  if (!c->isconnected)
		    goto exit;

    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

    if (message->qos == QOS1 || message->qos == QOS2)
        message->id = getNextPacketId(c);

    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len, &timer)) != SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

    if (message->qos == QOS1)
    {
        if (waitfor(c, PUBACK, &timer) == PUBACK)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }
    else if (message->qos == QOS2)
    {
        if (waitfor(c, PUBCOMP, &timer) == PUBCOMP)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = FAILURE;
        }
        else
            rc = FAILURE;
    }

exit:
    if (rc == FAILURE)
        MQTTCloseSession(c);
#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}


int MQTTDisconnect(MQTTClient* c)
{
    int rc = FAILURE;
    Timer timer;     // we might wait for incomplete incoming publishes to complete
    int len = 0;

#if defined(MQTT_TASK)
	MutexLock(&c->mutex);
#endif
    TimerInit(&timer);
    TimerCountdownMS(&timer, c->command_timeout_ms);

	  len = MQTTSerialize_disconnect(c->buf, c->buf_size);
    if (len > 0)
        rc = sendPacket(c, len, &timer);            // send the disconnect packet
    MQTTCloseSession(c);

#if defined(MQTT_TASK)
	  MutexUnlock(&c->mutex);
#endif
    return rc;
}

/* get state of MQTT client */
static mqtt_state_t get_client_state(MQTTClient* pClient)
{
    mqtt_state_t state;
    MutexLock(pClient->lock_generic);
    state = pClient->client_state;
    MutexUnlock(pClient->lock_generic);
    return state;
}

/* set state of MQTT client */
static void set_client_state(MQTTClient *pClient, mqtt_state_t newState)
{
    MutexLock(pClient->lock_generic);
    pClient->client_state = newState;
    MutexUnlock(pClient->lock_generic);
}

/* check MQTT client is in normal state /
/* 0 ， in abmormal state; 1,in normal state */
static int check_state_normal(MQTTClient *c)
{
    if(!c){
        return 0;
    }

    if(get_client_state(c) == STATE_CONNECTED){
        return 1;
    }

    return 0;
}