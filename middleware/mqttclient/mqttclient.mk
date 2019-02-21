NAME := mqttclient

$(NAME)_SOURCES := MQTTPacket/MQTTConnectClient.c \
                   MQTTPacket/MQTTConnectServer.c \
				   MQTTPacket/MQTTDeserializePublish.c \
				   MQTTPacket/MQTTFormat.c \
				   MQTTPacket/MQTTPacket.c \
				   MQTTPacket/MQTTSerializePublish.c \
				   MQTTPacket/MQTTSubscribeClient.c \
				   MQTTPacket/MQTTSubscribeServer.c \
				   MQTTPacket/MQTTUnsubscribeClient.c \
				   MQTTPacket/MQTTUnsubscribeServer.c 

$(NAME)_SOURCES += MQTTClient/MQTTClient.c\
                   hal/HAL_Rahio.c


GLOBAL_INCLUDES += MQTTClient \
                   MQTTPacket \
				   hal
