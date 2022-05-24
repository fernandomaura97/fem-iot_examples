

/*---------------------------------------------------------------------------*/

#include "contiki.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "sys/log.h"

#include "dev/i2c.h"
#include "dev/multigas.h"

#include "dev/adc-sensors.h"
#include "lib/sensors.h"

#include "net/nullnet/nullnet.h"
#include "net/netstack.h"
#include <math.h>
#include <stdlib.h>

#define SAMPLE_RATE 150
#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6

#define OWN_NODEID 1

//static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa4 }};

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO




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
    putchar('\n') ;
}




//static uint32_t avg_loud;
//static int im = 0;
static float c__;


//#define SENSOR_READ_INTERVAL (10*CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
 //int16_t temperature, humidity;
  static struct etimer timer;
  static uint8_t buf[(sizeof(uint8_t)+2*sizeof(float))]; // NODEID + CO + NO2
 
  PROCESS_BEGIN();
  
  //nullnet_buf = (uint8_t *)&buf; //init nullnet_buf to point to the buffer
  //nullnet_len = sizeof(buf); //length of the buffer
  etimer_set(&timer, CLOCK_SECOND * 10);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));  
  buf[0] = OWN_NODEID; //won't change, no need to reassign every time
  
  /* Setup a periodic timer that expires after 10 seconds. Give some time to join DODAG */
  
//Setup Multichannel gas sensor  
  i2c_master_enable();
  mgsbegin(0x04);
  mgspowerOn();


  nullnet_buf = (uint8_t *)&buf; //init nullnet_buf to point to the buffer
  nullnet_len = sizeof(buf); //length of the buffer
 
//Setup DHT22
//SENSORS_ACTIVATE(dht22);

//Setup PM10 GP2Y10
 
  //pm10.configure(SENSORS_ACTIVE, ADC_PIN2);
//Setup Noise sensor
  //adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1);


   
while(1) {
  etimer_set(&timer, CLOCK_SECOND * SAMPLE_RATE);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));


   /*avg_loud = 0; 
   
    for(im = 0; im < 100; im++)
    { 
      etimer_set(&timer, CLOCK_SECOND * 0.1);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
      avg_loud += (adc_sensors.value(ANALOG_GROVE_LOUDNESS));
    }

    printf(" avg_loud %lu\n", avg_loud/100);
    snprintf(louds, sizeof(louds), "\"Noise\": %lu", avg_loud/100);
  */
    
  //Sample Multichannel gas sensor
    i2c_master_enable();
         
    c__ = measure_CO();

    union {
        float float_variable;
        uint8_t temp_array[4];
      } u;

      u.float_variable = c__;
      printf("CO: ");
      putFloat(c__, DEC3);
      printf("\n");
      //printf("co: float bytes = %02x %02x %02x %02x\n", u.temp_array[0], u.temp_array[1], u.temp_array[2], u.temp_array[3]);
      printf("\nCO bytes in decimal: %d %d %d %d\n", u.temp_array[0], u.temp_array[1], u.temp_array[2], u.temp_array[3]);

      memcpy(buf +1, u.temp_array, sizeof(float));

     
    c__ = measure_NO2();
     
      printf("CO: ");
      putFloat(c__, DEC3);
      printf("\n");
    union {
        float float_variable;
        uint8_t temp_array[4];
      } us;

    us.float_variable = c__;
    //putFloat(fvar, DEC3);
    //printf("no2:float bytes = %02x %02x %02x %02x\n", us.temp_array[0], us.temp_array[1], us.temp_array[2], us.temp_array[3]);
    printf("\n NO2 bytes in decimal: %d %d %d %d\n", us.temp_array[0], us.temp_array[1], us.temp_array[2], us.temp_array[3]);

    //memcpy(buf+5, us.temp_array, 4);
    buf[5] = us.temp_array[0];
    buf[6] = us.temp_array[1];
    buf[7] = us.temp_array[2];
    buf[8] = us.temp_array[3];

   
    nullnet_buf = (uint8_t *)&buf; //assign pointer to the buffer
    nullnet_len = sizeof(buf); //length of the buffer
    NETSTACK_NETWORK.output(NULL); //send the buffer
    
    printf("\nnullnet buffer = %d %d %d %d %d %d %d %d %d \n ", nullnet_buf[0], nullnet_buf[1], nullnet_buf[2], nullnet_buf[3], nullnet_buf[4] , nullnet_buf[5], nullnet_buf[6], nullnet_buf[7], nullnet_buf[8]);  
        


       
    
      
      
  }    
  PROCESS_END();
}