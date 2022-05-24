#include "contiki.h"
#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"

#include "random.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/

static struct etimer et;
uint8_t beacon [3];
typedef struct data_t{
  int16_t temperature, humidity;
  uint16_t noise;
} data_t; 

struct data_t datas; 
static clock_time_t now, last;
/*---------------------------------------------------------------------------*/

PROCESS(beacons, "beacons");
AUTOSTART_PROCESSES(&beacons);



/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){

  uint8_t* bytebuf;
  last = RTIMER_NOW() - now;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);
  memcpy(&datas.temperature, &bytebuf[1], 2);
  memcpy(&datas.humidity, &bytebuf[3], 2);
  memcpy(&datas.noise, &bytebuf[5], 2);

  printf("{\"Temperature\": %02d.%02d", datas.temperature / 10, datas.temperature % 10);
  printf(", \"Humidity\": %02d.%02d", datas.humidity / 10, datas.humidity % 10);
  printf(", \"Noise\": %u}\n\n", datas.noise);
  
  printf("Time elapsed: %lu ticks, %lu seconds, %lu ms\n", last, last / CLOCK_SECOND, last / CLOCK_SECOND * 1000);

  free(bytebuf);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(beacons, ev, data){

  PROCESS_BEGIN();

  uint8_t seed = linkaddr_node_addr.u8[0];
  random_init(seed);
  
  while(1){

    etimer_set(&et, CLOCK_SECOND * 20);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    uint8_t beacon[3] = {0x00, 0x01, 0x34};

    beacon[1] = random_rand() % 255;
    beacon[2] = random_rand() % 255;

    nullnet_buf = beacon;
    nullnet_len = 3;

    now = RTIMER_NOW();
    NETSTACK_NETWORK.output(NULL);

    nullnet_set_input_callback(input_callback);
    
    printf("sent %d %d %d\n", beacon[0], beacon[1], beacon[2]);
  }

  
  
  PROCESS_END();
}


PROCESS_THREAD(rx_process,ev,data)
{
static struct etimer beacon_no_timerM 
volatile static uint8_t * datapoint;
PROCESS_BEGIN();

while(1){

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);




}



PROCESS_END();



}