NAME := pahomqttapp

$(NAME)_SOURCES :=app_entry.c 


$(NAME)_COMPONENTS += network/netmgr \
                      middleware/common \
                      utility/cjson \
                      tools/cli

$(NAME)_SOURCES += pahomqtt_sample.c
$(NAME)_COMPONENTS += middleware/mqttclient 
# $(NAME)_SOURCES +=pahomqtt_example.c
# $(NAME)_COMPONENTS += feature.linkkit
pro := linkMQTTT

ifeq ($(pro),linkMQTT)
$(NAME)_SOURCES +=pahomqtt_example.c
$(NAME)_COMPONENTS += feature.linkkit
else ifeq ($(pro),pahoMQTT)
$(NAME)_SOURCES += pahomqtt_sample.c
$(NAME)_COMPONENTS += middleware/mqttclient 
endif



GLOBAL_DEFINES += CONFIG_AOS_CLI USE_LPTHREAD

GLOBAL_DEFINES += MQTT_TASK 
GLOBAL_INCLUDES += ./ 



