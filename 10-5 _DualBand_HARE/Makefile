CONTIKI_PROJECT = gw_dl_DHT22 node_DHT22 node_DHT22_staticSR

all: $(CONTIKI_PROJECT)

#MODULES += os/services/shell
MODULES += os/services/simple-energest


CONTIKI_TARGET_SOURCEFILES += dht22.c
CONTIKI=../..
MAKE_NET = MAKE_NET_NULLNET
include $(CONTIKI)/Makefile.include
