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
char buf_in[100];
uint8_t beacon [3];
int16_t temperature, humidity;
uint8_t nodeid = NODEID;
uint16_t loudness;
char delimitador[4] = ",";
char* b[4];
uint8_t sortida[3];
char* endPtr;
int i = 0;
/*---------------------------------------------------------------------------*/
PROCESS(dual_band, "dual band");
AUTOSTART_PROCESSES(&dual_band);
/*---------------------------------------------------------------------------*/

void funcio_sensors(){
  SENSORS_ACTIVATE(dht22);
  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN);

  if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR){
    printf("{\"nodeID\": %u", nodeid);

    printf(",\"Temperature\": %02d.%02d", temperature / 10, temperature % 10);
    printf(", \"Humidity\": %02d.%02d", humidity / 10, humidity % 10);
  }
  else{
    printf("Failed to read the sensor\n");
  }

  loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);

  if(loudness != ADC_WRAPPER_ERROR){
    printf(", \"Noise\": %u}", loudness);
  }
  else{
    printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
  }
  printf("\n");
}

unsigned int uart1_send_bytes(const unsigned char *s, unsigned int len){  
  unsigned int i = 0;

  while(s && *s != 0){
    if(i >= len) {
      break;
    }
    uart_write_byte(1, *s++);
    i++;
  }
  return i;
}

void serial_in(){ // Implementa la lògica a la cadena de caràcters que ha entrat al UART. node_db_24

  if (strncmp(buf_in, "BO", 2) == 0){ // B0 indicarà al db_24 que el missatge és beacon

    leds_toggle(LEDS_GREEN);

    sprintf(buf_out, "B0, %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
    uart1_send_bytes((uint8_t *)buf_out, sizeof(buf_out)-1);
    
    leds_toggle(LEDS_RED);

    printf("SERIAL DATA OUT --> %s", (char *) buf_out);
    printf("***********************\n");
    
    //Extreure de cada substring (b[0], b[1], b[2]) el valor numèric
    char* token = strtok(buffer, delimitador);
    
    if(token != NULL){
        while(token != NULL){
            b[i] = token;
            i++;
            token = strtok(NULL, delimitador);
        }
    }
    for (int i = 0; i<=4; i++){
        //Substring del string
        printf("b[i]: %s, ", b[i]);
        // Reemsamblar el beacon al db_24 i enviar-lo cap als nodes sensors
        sortida[i] = strtol(b[i], &endPtr, 16);
        printf("La variable uint8_t és: %ld\n", sortida[i]);
    }
    // NO TINC MOLT CLAR COM ENVIAR-LO ALS NODES SENSORS:/
    funcio_sensors(); // Mesura sensors
    
}

int print_uart(unsigned char c){
	buf_in[counter_uart] = c;
	counter_uart++;

	if (c == '\n'){
		printf("SERIAL DATA IN --> %s\n", (char *)buf_in); // SERIAL DATA IN --> B0, 0, 0, 0
		counter_uart = 0;
		serial_in();
	}
	
	return 1;
}

void nullnet_set_input_callback	(nullnet_input_callback 	callback){
char string[20];

  sprintf(string, "B0, %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
  uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dual_band, ev, data){

  PROCESS_BEGIN();
  uart_set_input(1, print_uart);
  nullnet_set_input_callback(&callback_nullnet);
  etimer_set(&et, CLOCK_SECOND * 4);
  leds_toggle(LEDS_RED);

  PROCESS_END();
}
