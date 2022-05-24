/*
 * Copyright (c) 2016, Zolertia <http://www.zolertia.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */
/**
 * \addtogroup zoul-examples
 * @{
 *
 * \defgroup zoul-dht22-test DHT22 temperature and humidity sensor test
 *
 * Demonstrates the use of the DHT22 digital temperature and humidity sensor
 * @{
 *
 * \file
 *         A quick program for testing the DHT22 temperature and humidity sensor
 * \author
 *         Antonio Lignan <alinan@zolertia.com>
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/dht22.h"
#include <stdlib.h>
#include "dbg.h"
#include <math.h>
#include "dev/adc-sensors.h"
#include "lib/sensors.h"

#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "sys/log.h"
#include "stdlib.h"

#define SAMPLE_RATE 140

#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6
#define NODEID_PM10_1 7
#define NODEID_PM10_2 8


#define OWN_NODEID NODEID_DHT22_2

//dht22 uses pin 5, same as noise sensor.
#define ADC_PIN1 2 //5V adc, need to multiply results by 5/3 to get true value of voltage, still 
//we lose resolution. Better to use ADC2, although it has no phidget connector.




#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


//static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa4 }};


/*---------------------------------------------------------------------------*/
PROCESS(remote_dht22_process, "DHT22 test");
AUTOSTART_PROCESSES(&remote_dht22_process);
/*---------------------------------------------------------------------------*/
static struct etimer et;


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(remote_dht22_process, ev, data)
{ 
  static int16_t temperature, humidity;
  static uint8_t buf2[(sizeof(uint8_t)+2*sizeof(int16_t)) + sizeof(uint32_t)]; // NODEID + TEMPERATURE + HUMIDITY + avg_noise
  static uint32_t avg_noise; 
  

  PROCESS_BEGIN();
   
  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1);
  

  nullnet_buf = (uint8_t *)&buf2; //initialize the pointer to the buffer
  nullnet_len = sizeof(buf2); //initialize the length of the buffer

  printf("sizeof(buffer) = %d\n", sizeof(buf2));

  //etimer_set(&et, CLOCK_SECOND * 5);
  //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  buf2[0] = OWN_NODEID; //won't change, no need to reassign every time



  adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1);

  SENSORS_ACTIVATE(dht22);//conve
//static dest_ipaddr_t dest_ipaddr = {{0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0xb6, 0xa8}};

  while(1) {
    etimer_set(&et, SAMPLE_RATE*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));


    avg_noise = 0;
    static int i;
    for(i = 0; i<100; i++){
      etimer_set(&et, CLOCK_SECOND/10);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      avg_noise += adc_sensors.value(ANALOG_GROVE_LOUDNESS) * 5/3; //multiply by 5/3 to get "true" value
      printf("avg_noise = %lu\n", avg_noise);
    }
    printf("avg noise = %lu\n", avg_noise/100);
    avg_noise = avg_noise/100;
    
    
    union {
        uint32_t float_variable;
        uint8_t temp_array[sizeof(uint32_t)];
      } us;


    us.float_variable = avg_noise;
    buf2[5] = us.temp_array[0];
    buf2[6] = us.temp_array[1];
    buf2[7] = us.temp_array[2];
    buf2[8] = us.temp_array[3];




    if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      printf("\"Temperature\": %02d.%d, ", temperature / 10, temperature % 10);
      printf("\"Humidity\": %02d.%d ", humidity / 10, humidity % 10);

      //printf("nodeid = %d\n", buf2[0]);
    
      buf2[1] = temperature & 0xFF;
      buf2[2] = (temperature >> 8) & 0xFF;
      buf2[3] = humidity & 0xFF;
      buf2[4] = (humidity >> 8) & 0xFF;
      
      printf("\nbuf2 = %d %d %d %d %d %d %d %d %d\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4] ,buf2[5], buf2[6], buf2[7], buf2[8]);
      
    } else {
      printf("Failed to read the sensor\n");
      //adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1); //Just in case, adc's get stuck otherwise
      watchdog_reboot();
    }
    
     nullnet_buf = (uint8_t *)&buf2; //assign pointer to the buffer
     nullnet_len = sizeof(buf2); //length of the buffer

     printf("\nnullnet buffer = %d %d %d %d %d %d %d %d %d\n ", nullnet_buf[0], nullnet_buf[1], nullnet_buf[2], nullnet_buf[3], nullnet_buf[4], nullnet_buf[5], nullnet_buf[6], nullnet_buf[7], nullnet_buf[8]);
     NETSTACK_NETWORK.output(NULL); //send the buffer
    
  }
  PROCESS_END();

}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

