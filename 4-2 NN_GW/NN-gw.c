/*
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

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

/*#include "sys/log.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG
*/
#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6
#define NODEID_PM10_1 7
#define NODEID_PM10_2 8


//static char serbuf[300];
//static uint8_t payload[64];

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);

typedef struct sensor_data_t {
  uint8_t nodeid;
  int16_t humidity;
  int16_t temperature;
  uint16_t pm10;
  uint32_t noise;
  float o3;
  float co;
  float no2;
} sensor_data_t;
static sensor_data_t sensors;



typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision ;

void putLong(long x)
{
    if(x < 0)
    {
        putchar('-');
        x = -x;
    }
    if (x >= 10) 
    {
        putLong(x / 10);
    }
    putchar(x % 10+'0');
}

void putFloat( float f, tPrecision p )
{
    long i = (long)f ;
    putLong( i ) ;
    f = (f - i) * p ;
    i = abs((long)f) ;
    if( fabs(f) - i >= 0.5f )
    {
        i++ ;
    }
    putchar('.') ;
    putLong( i ) ;
    
}



/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  //printf("received %d bytes: \n", len);
  
  uint8_t *buf1 = (uint8_t *)malloc(sizeof(uint8_t) + 2*sizeof(float)); //allocate 9bytes (maximum payload)
  memcpy(buf1, data, len);
  
  uint32_t fbuf; //float buffer
  union {
    float float_variable;
    uint8_t temp_array[4];
      } u;
  
  union{
    uint32_t u32_var;
    uint8_t temp_array[4];
      } ua;

  union{
    int16_t u16_var;
    uint8_t temp_array[2];
      } ua2;

  


  switch(buf1[0]){
      
      
      case NODEID_MGAS1:
      case NODEID_MGAS2:
        //printf("mgas1\n");
        
        u.temp_array[0] = buf1[1];
        u.temp_array[1] = buf1[2];
        u.temp_array[2] = buf1[3];
        u.temp_array[3] = buf1[4];
        sensors.co = u.float_variable;
        //memcpy???

        u.temp_array[0] = buf1[5];
        u.temp_array[1] = buf1[6];
        u.temp_array[2] = buf1[7];
        u.temp_array[3] = buf1[8];
        sensors.no2 = u.float_variable;

        //memcpy??

        printf("{\"nodeID\": %d", buf1[0]);
        printf(",\"co\": ");
        fbuf = sensors.co * 100;
        printf("%lu.%02lu", fbuf/100, fbuf%100);
        printf(", \"no2\": ");
        fbuf = sensors.no2 * 100;
        printf("%lu.%02lu", fbuf/100, fbuf%100);
        printf("}\n");    
        break;



      case NODEID_DHT22_1:
      case NODEID_DHT22_2:
       //printf("dht22: %d\n", buf1[0]);

        //printf("buf[1] %d buf[2] %d", buf1[1], buf1[2]);
        ua2.temp_array[0] = buf1[1];
        ua2.temp_array[1] = buf1[2];
        
        memcpy(&sensors.temperature, &ua2.u16_var, sizeof(int16_t)); 
    

        ua2.temp_array[0] = buf1[3];
        ua2.temp_array[1] = buf1[4];
        memcpy(&sensors.humidity, &ua2.u16_var, sizeof(int16_t));
       
        ua.temp_array[0] = buf1[5];
        ua.temp_array[1] = buf1[6];
        ua.temp_array[2] = buf1[7];
        ua.temp_array[3] = buf1[8];
        //sensors.noise = u.u32_var;
        memcpy(&sensors.noise, &ua.u32_var, sizeof(uint32_t));

        //JSON conversion
        printf("{\"nodeID\": %d", buf1[0]);
        printf(",\"Humidity\": %d.%d", sensors.humidity/10, sensors.humidity%10);
        printf(",\"Temperature\": %d.%d", sensors.temperature/10, sensors.temperature%10);
        printf(",\"Noise\": %lu", sensors.noise);
        printf("}\n");

        //ALL GOOD!
        break;
         
      case NODEID_O3_1:
      case NODEID_O3_2:
        
        ua2.temp_array[0] = buf1[1];
        ua2.temp_array[1] = buf1[2];
        
        memcpy(&sensors.temperature, &ua2.u16_var, sizeof(int16_t)); 
    

        ua2.temp_array[0] = buf1[3];
        ua2.temp_array[1] = buf1[4];
        memcpy(&sensors.humidity, &ua2.u16_var, sizeof(int16_t));

        u.temp_array[0] = buf1[5];
        u.temp_array[1] = buf1[6];
        u.temp_array[2] = buf1[7];
        u.temp_array[3] = buf1[8];
        
        //sensors.o3 = u.float_variable;
        memcpy(&sensors.o3, &u.float_variable, sizeof(float));
        fbuf = sensors.o3 * 100;
        


        printf("{\"nodeID\": %d", buf1[0]);
        printf(",\"ppm\": ");
        printf("%lu.%02lu", fbuf/100, fbuf%100);
        //putFloat(sensors.o3, DEC3);
        printf(",\"Humidity\": %d.%d", sensors.humidity/10, sensors.humidity%10);
        printf(",\"Temperature\": %d.%d", sensors.temperature/10, sensors.temperature%10);
        printf("}\n");
        break;
      case NODEID_PM10_1:
      case NODEID_PM10_2:
        //printf("pm10: %d\n", buf1[0]);
        sensors.pm10 = (buf1[2] << 8) | buf1[1];
        printf("{\"nodeID\": %d", buf1[0]);
        printf(",\"pm10\": %d", sensors.pm10);
        printf("}\n");
        break;
        //AOK!!
      
      default:
        /*printf("unknown nodeID %d\n", buf1[0]);
        printf("BYTES copied are: ");
          for (int i = 0; i < len; i++) {
        printf("%d ", buf1[i]);
        */        
        break;
    } //switch
     
  
  /*printf("BYTES copied are: ");
  for (int i = 0; i < len; i++) {
        printf("%d ", buf1[i]);
      }
  printf("\n\n");
*/
  free(buf1);
 } //callback
 
    
 
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{

  static struct etimer et;
  PROCESS_BEGIN();
  //static my_data dht22_data; 


  nullnet_set_input_callback(input_callback);
  etimer_set(&et, CLOCK_SECOND * 10);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    /*printf("buckle up buckaroo\n");
    printf("String: %s\n", serbuf);
    printf( "struct1: nodeID: %d, humidity: %d, temperature: %d\n",
      dht22_data.old.nodeID, dht22_data.old.humidity, dht22_data.old.temperature);
    printf( "struct2: nodeID: %d, humidity: %d, temperature: %d\n", nodeid1, humidity1, temperature1);
    printf("bye bye\n");
    */
    etimer_reset(&et);
  }
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

