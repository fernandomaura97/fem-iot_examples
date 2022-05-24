#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "dev/dht22.h"
#include "lib/sensors.h"
#include "sys/log.h"
#include "dev/adc-sensors.h"
#include "lib/sensors.h"
#include "dev/gpio-hal.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define ADC_PIN 2 // ADC1 utilitza el pin 5 i ADC3 utilitza el pin 2. Es pot veure en el datasheet
#define SENSOR_READ_INTERVAL (CLOCK_SECOND / 8)
#define NODEID 1
/*---------------------------------------------------------------------------*/
static struct etimer et;
uint16_t counter_uart;
char buf_out[100];
static char buf_in[100];
uint8_t beacon [3];
int16_t temperature, humidity;
uint8_t nodeid = NODEID;
uint16_t loudness;
char delimitador[4] = ",";

static char b[4];
//uint8_t sortida[3];
long int sortida[3];
char* endPtr;
int i = 0;
/*---------------------------------------------------------------------------*/
PROCESS(dual_band, "dual band");
AUTOSTART_PROCESSES(&dual_band);
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dual_band, ev, data){

  PROCESS_BEGIN();

  while(1){
  
    etimer_set(&et, CLOCK_SECOND * 60);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    uint8_t beacon[3] = {0x00, 0x01, 0x034};
    nullnet_buf = beacon;
    nullnet_len = 3;
 
    NETSTACK_NETWORK.output(NULL);
     
    leds_toggle(LEDS_RED);
    
  }
 

  PROCESS_END();
}
