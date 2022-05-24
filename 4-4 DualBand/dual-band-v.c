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
#include <stdio.h>
#include <stdint.h>
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

void serial_in(){ // Implementa la lògica a la cadena de caràcters que ha entrat al UART.

  if (strncmp(buf_in, "BO", 2) == 0){
    printf("SWITCH ON\n");

    leds_toggle(LEDS_GREEN);

    sprintf(buf_out, "B0, %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
    uart1_send_bytes((uint8_t *)buf_out, sizeof(buf_out)-1);
    
    leds_toggle(LEDS_RED);

    printf("SERIAL DATA OUT --> %s", (char *) buf_out);
    printf("***********************\n");
    // B0 indicarà al db_24 que el missatge és beacon
    //Extreure de cada substring (b[0], b[1], b[2]) el valor numèric. --> strtol
    printf ("%ld", strtol(beacon[0],NULL,0)); // Base detectada tercer paràmetre de la funció strtol (0)
    printf ("%ld", strtol(beacon[1],NULL,0));
    printf ("%ld", strtol(beacon[2],NULL,0));
    // Reemsamblar el beacon al db_24 i enviar-lo cap als nodes sensors

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
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dual_band, ev, data){

  char string[20];

  PROCESS_BEGIN();
  uart_set_input(1, print_uart);
  etimer_set(&et, CLOCK_SECOND * 4);
  leds_toggle(LEDS_RED);

	while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    sprintf(string, "B0, %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
    uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
    etimer_reset(&et);
    printf("string is being sent\n");
  }

  PROCESS_END();
}
