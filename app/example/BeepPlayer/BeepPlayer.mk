NAME := BeepPlayer

$(NAME)_SOURCES :=app_entry.c \
                  driver/src/led.c \
                  driver/src/button.c \
                  driver/src/beep.c  \
                  driver/src/decode.c \
                  driver/src/player.c  \
                  driver/src/player_demo.c \
                  driver/src/key.c

$(NAME)_COMPONENTS := yloop cli

GLOBAL_INCLUDES  += ./       \
                    driver/inc