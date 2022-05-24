

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "stdlib.h"
#include "dev/dht22.h"
#include "dev/MQ1312.h"


#include "dev/adc-sensors.h"
#include "dev/gpio-hal.h"
#include "lib/sensors.h"

#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

////////////IPV6 definitions

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SAMPLE_RATE ( CLOCK_SECOND)
#define ADC_PIN              2



#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6
#define NODEID_PM10_1 7
#define NODEID_PM10_2 8

#define OWN_NODEID NODEID_O3_1

//static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0xb6, 0xa4 }};

typedef enum
{
    DEC1 = 10,
    DEC2 = 100,
    DEC3 = 1000,
    DEC4 = 10000,
    DEC5 = 100000,
    DEC6 = 1000000,

} tPrecision ;


static struct {
  float R0;
  uint16_t stable_time;
} calib_s; //calibration struct

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

float MQQ_getEnvCorrectRatio(int16_t t, int16_t hum) {
 	// Select the right equation based on humidity
 	// If default value, ignore correction ratio

	uint8_t temperature = t/10; 
	uint8_t humidity = hum/10;

 	if(humidity == 60 && temperature == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	else if( humidity > 75) {
    // R^2 = 0.996
   	return -0.0103 * temperature + 1.1507;
 	}
 	// For humidity > 50%, use the 60% curve
 	else if (humidity> 50) {
 		// R^2 = 0.9976
 		return -0.0119 * temperature + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
  // R^2 = 0.9986
 	else{
		 return -0.0141 * temperature + 1.5623;
	 }	
 }

float ReadRs(){

    uint16_t valueSensor= adc_zoul.value(ZOUL_SENSORS_ADC3);
    printf("Valor de sensor: %u\n", valueSensor);

    float fvaluesensor = valueSensor;
    float vRL = (fvaluesensor / 16384.0)* 5.1 ; //16384 bits adc, 2¹⁴ /5.1V 
    
    printf("Valor de VRL:");
    putFloat(vRL, DEC3);
    printf("\n");

    float rS = (5.1 / vRL - 1.0) * 1000000.0;  // 1MOhm ==  Value R_l 

    /*printf("Valor de RS - hardc:");
    putFloat(rS, DEC3);
    printf("\n"); */ 
    return rS;
    }
void calibrate(){

  //wait until adc stabilyzed, then assign lastRsvalue to R0
  float fmeasurement = ReadRs();

  calib_s.R0 = fmeasurement; 
  printf("Valor de R0:");
  putFloat(calib_s.R0, DEC3);
}


float getppm(float rs, float env_ratio){

  float valueR0 = calib_s.R0;
  float ratio = rs / valueR0 * env_ratio; //Where 0.9906 is ratio, to be calculated through getEnvCorrectRatio()
  float ppb = 9.4783 * pow(ratio, 2.3348);
  //float ppm2 = ppb / 1000.0;
/*
  printf("putfloat ppb \n");
  putFloat( ppb, DEC3 );

  printf("putfloat ppm \n");
  putFloat( ppm2, DEC3 );
*/
  return ppb;  
}

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static int16_t temperature, humidity; 
  static struct etimer timy;
  static uint8_t buf[sizeof(uint8_t) + sizeof(float) + 2*sizeof(uint16_t)];
  PROCESS_BEGIN()
  
  nullnet_buf = (uint8_t *)&buf;
  nullnet_len = sizeof(buf);
  printf("size of buffer: %d\n", sizeof(buf));

  buf[0] = OWN_NODEID;
  SENSORS_ACTIVATE(dht22);

	//MQ131_sensor.configure(S/if((uint32_t)lastRsValue != (uint32_t)value && (uint32_t)lastLastRsValue != (uint32_t)value){ ////////////PROBAR PROBAR 

	//adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC_ALL);
  adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC3);

  printf("Calibrando...\n");
  calibrate(); 
  printf("Calibrado!\n");
    
  while(1) {
  etimer_set(&timy, SAMPLE_RATE);
  
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timy));

  
  float rs = ReadRs();

  float enviratio = MQQ_getEnvCorrectRatio(temperature, humidity);
   
  float ppm = getppm(rs, enviratio);
 

  union {
        float float_variable;
        uint8_t temp_array[4];
      } u;

  u.float_variable = ppm;

  buf[5] = u.temp_array[0];
  buf[6] = u.temp_array[1];
  buf[7] = u.temp_array[2];
  buf[8] = u.temp_array[3];

  if(dht22_read_all(&temperature, &humidity) != DHT22_ERROR) {
      printf("\"Temperature\": %02d.%d, ", temperature / 10, temperature % 10);
      printf("\"Humidity\": %02d.%d ", humidity / 10, humidity % 10);

      printf("nodeid = %d\n", buf[0]);    
      buf[1] = temperature & 0xFF;
      buf[2] = (temperature >> 8) & 0xFF;
      buf[3] = humidity & 0xFF;
      buf[4] = (humidity >> 8) & 0xFF;
  }

  else{
    printf("Error reading DHT22\n");
    printf("rebooting...\n");
    //watchdog_reboot();
    //adc_zoul.configure(SENSORS_HW_INIT, ZOUL_SENSORS_ADC3); //por si acaso
    watchdog_reboot();
  }     

  nullnet_buf = (uint8_t *)&buf;
  nullnet_len = sizeof(buf);


  printf("\n nullnet buffer = %d %d %d %d %d %d %d %d %d \n ", nullnet_buf[0], nullnet_buf[1], nullnet_buf[2], nullnet_buf[3], nullnet_buf[4] , nullnet_buf[5], nullnet_buf[6], nullnet_buf[7], nullnet_buf[8]); 
  NETSTACK_NETWORK.output(NULL);    //

    
  }

  PROCESS_END();
}

