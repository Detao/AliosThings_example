/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>




#include "MQTTClient.h"
#include "HAL_Rahio.h"


void messageArrived(MessageData* data)
{
	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
}

int linkkit_main(void *paras)
{
	/* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
	MQTTClient client;
	Network network;
	unsigned char sendbuf[1024], readbuf[1024];
	int rc = 0, 
		count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

	NetworkInit(&network);
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char* address = "183.230.40.39";
	if ((rc = NetworkConnect(&network, address, 6002)) != 0)
		printf("Return code from network connect is %d\n", rc);

#if defined(MQTT_TASK)
	 MQTTStartTask(&client);	
#endif

	connectData.MQTTVersion = 4;
	connectData.clientID.cstring = "515068087";
    connectData.username.cstring ="206452"; //pconn_info->username;
    connectData.password.cstring = "led1";//pconn_info->password;

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");

	// if ((rc = MQTTSubscribe(&client, "FreeRTOS/sample/#", 2, messageArrived)) != 0)
	// 	printf("Return code from MQTT subscribe is %d\n", rc);

	while (++count)
	{
		MQTTMessage message;
		char payload[30];
        int length;
		message.qos = 1;
		message.retained = 0;
		message.payload = payload;
		
		sprintf(&(payload[3]), "{\"datastreams\": [{\"id\":\"Temperature\",\"datapoints\":[{\"value\":%d}]}]}",count);
		length = strlen(&(payload[3]));
		payload[0] = 0x01;
		payload[1] = (length & 0xff00) >> 8;
		payload[2] = length & 0xff;
		message.payloadlen = length + 3;

		if ((rc = MQTTPublish(&client, "$dp", &message)) != 0){
             printf("Return code from MQTT publish is %d\n", rc);
			 break;
		}else{
			printf("Return code from MQTT publish is %d\n", rc);
		}
	    		
#if !defined(MQTT_TASK)
		if ((rc = MQTTYield(&client, 1000)) != 0)
			printf("Return code from yield is %d\n", rc);
#endif
	}

	/* do not return */
}



/*-----------------------------------------------------------*/


