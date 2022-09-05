#include "contiki.h"
#include "sys/etimer.h"
#include "dev/uart.h"
#include "dev/dht22.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include "dev/gpio-hal.h"
#include "dev/adc-sensors.h"
#include "lib/sensors.h"
#include "dev/serial-line.h"
#include "dev/leds.h"
#include "sys/log.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/*---------------------------------------------------------------------------*/
#define ADC_PIN 2 // ADC1 utilitza el pin 5 i ADC3 utilitza el pin 2. Es pot veure en el datasheet
#define SENSOR_READ_INTERVAL (CLOCK_SECOND / 8)
/*---------------------------------------------------------------------------*/

//static struct etimer et;
//static bool flag = 0;
static uint8_t buffer[7]; // 1 byte capçalera i 2 bytes per variable


typedef struct data_t{
  int16_t temperature, humidity;
  uint16_t noise;
} data_t;

static struct data_t datas;

/*---------------------------------------------------------------------------*/

void mesures_sensors(){

  int16_t temperature, humidity;
  uint16_t loudness;

  SENSORS_ACTIVATE(dht22);

  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN);
  if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR){
    
    //Emmagatzemem a l'estructura anomenada data
    datas.temperature = temperature;
    datas.humidity = humidity;

    printf("{\"Temperature\": %02d.%02d", temperature / 10, temperature % 10);
    printf(", \"Humidity\": %02d.%02d", humidity / 10, humidity % 10);
  }
  else{

    //Emmagatzemem a l'estructura anomenada data
    datas.temperature = 0;
    datas.humidity = 0;
    printf("Failed to read the sensor\n");
  }

  loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);
  if(loudness != ADC_WRAPPER_ERROR){

    //Emmagatzemem a l'estructura anomenada data
    datas.noise = loudness;
    printf(", \"Noise\": %u}\n", loudness);
  }
  else{
    datas.noise = 0;
    printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
  }
  printf("\n");

  //Afegir capçalera. Per exemple: buffer[0] = id;
  buffer[0] = 0;
  memcpy(&buffer[1], &datas.temperature, sizeof(datas.temperature));
  memcpy(&buffer[3], &datas.humidity, sizeof(datas.humidity));
  memcpy(&buffer[5], &datas.noise, sizeof(datas.noise));

}

/*---------------------------------------------------------------------------*/
PROCESS(sensors_db_24, "sensors db 24");
AUTOSTART_PROCESSES(&sensors_db_24);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  uint8_t* bytebuf;
  bytebuf = malloc(len);
  memcpy(bytebuf, data, len);

  printf("Data beacon: B0 %d %d %d\n", bytebuf[0], bytebuf[1], bytebuf[2]);

  mesures_sensors();
  process_poll(&sensors_db_24);

  free(bytebuf);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sensors_db_24, ev, data){


  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);
  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

    nullnet_buf = (uint8_t*)&buffer;
    nullnet_len = sizeof(buffer);
    NETSTACK_NETWORK.output(NULL);
    printf("sensor data out\n");
  }
  
  PROCESS_END();
}
