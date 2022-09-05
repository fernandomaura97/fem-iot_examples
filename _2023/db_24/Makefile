CONTIKI_PROJECT = dualband_24 sensors_db_24 node

all: $(CONTIKI_PROJECT)

#MODULES += os/services/shell

CONTIKI_TARGET_SOURCEFILES += dht22.c adc-sensors.c

CONTIKI = ../../..
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include



