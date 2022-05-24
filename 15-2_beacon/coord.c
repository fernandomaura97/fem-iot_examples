#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"
#include <string.h>
#include <stdio.h> 
#include "random.h"
#include "sys/clock.h"
#include <stdlib.h>


#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6
#define NODEID_PM10_1 7
#define NODEID_PM10_2 8


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

//static linkaddr_t addr_stas[8]; //store sta's addresses in here, for routing and sending


/*
#define SEND_INTERVAL (10 * CLOCK_SECOND)
#define BEACON_INTERVAL (20* CLOCK_SECOND)
#define T_MM (5* CLOCK_SECOND)
#define T_GUARD (1* CLOCK_SECOND)
#define T_SLOT (10* CLOCK_SECOND)*/


/* Configuration */
#define SPEED_NW 1 //Speed >1 if we want faster beacons and times ( for debugging "quicker" without changing too much code)

#define SEND_INTERVAL (220 * CLOCK_SECOND * (1/SPEED_NW))
#define BEACON_INTERVAL (240* CLOCK_SECOND * (1/SPEED_NW))
#define T_MM (30* CLOCK_SECOND  * 1/SPEED_NW)
#define T_GUARD (1* CLOCK_SECOND * 1/SPEED_NW)
#define T_SLOT (10* CLOCK_SECOND *  1/SPEED_NW)

//struct sensor types



/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
//PROCESS(beacon_process, "beacon process");
//PROCESS(parser_process, "Parsing process");

AUTOSTART_PROCESSES(&nullnet_example_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("Callback \t received rx: %d\n", *(uint8_t *)data); 
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
static uint8_t sensor_type_byte;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  static struct etimer send_timer2;
  //static struct etimer send_timer3;
  //static unsigned count = 0;
  //static uint16_t sensor_nodes; 
  
  union {
    uint8_t bytes[2];
    uint16_t value;
  } sensor_type;
  
  PROCESS_BEGIN();

  //random_init(0);
 
  sensor_type.value = random_rand(); //Create random sensor type
  

  //create random beacon type choosing between 4 possible options
  uint8_t beacon_type = random_rand() % 4;
  printf("beacon type: %d\n", beacon_type);

  uint8_t cycle_time = random_rand() % 10;
  printf("cycle time: %d\n", cycle_time);

  uint8_t frame = beacon_type << 6 | cycle_time;
  printf("frame: %d\n", frame);
 
  printf("R0: %d\t, R1: %d\n", sensor_type.bytes[0], sensor_type.bytes[1]);
 
    //Get random uint8_t
  memcpy(&sensor_type_byte, &sensor_type.bytes[0], sizeof(sensor_type.bytes[0]));
  printf("Bit mask: %d\n", sensor_type_byte);

  nullnet_buf = (uint8_t *)&sensor_type_byte;
  nullnet_len = sizeof(sensor_type_byte);
  nullnet_set_input_callback(input_callback); //Comment if we don't need a rcv callback



  
  while(1) {

    etimer_set(&periodic_timer, BEACON_INTERVAL);  //Set timer for beacon interval
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    /*--------------------------------------------------------------*/
    //Send 3 beacons
    static uint8_t i;
    for (i=0; i<3; i++) { 
      etimer_set(&send_timer, T_GUARD); //Time between beacons
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

      LOG_INFO("Beacon %d sent, data %d\n", (i+1), sensor_type_byte);
      nullnet_buf = (uint8_t *)&sensor_type_byte;
      nullnet_len = sizeof(sensor_type_byte);
      NETSTACK_NETWORK.output(NULL); 
      
    }
    
    /*--------------------------------------------------------------*/
   // process_poll(&beacon_process);


    printf("waiting for measurements\n");


    etimer_set (&send_timer2, T_MM); //Set timer for measurement interval
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer2));

    //Now, poll every node in sensor_type_byte bitmask in order to receive a measurement


    static clock_time_t t;
    static clock_time_t dt;
    t = clock_time();
    printf("seconds since boot: %lu\n", t/CLOCK_SECOND);
    static uint8_t i2;
    for (i2=1; i2<9; i2++) {
      if (sensor_type_byte & 0x01) {

        sensor_type_byte = sensor_type_byte >> 1;
        //printf("Polling node %d\n", i);


        nullnet_buf = (uint8_t *)&i2;
        nullnet_len = sizeof(i2);
        dt = clock_time()-t;
        printf("Polling node %d, dt: %lu\n", i2, dt/CLOCK_SECOND);
        NETSTACK_NETWORK.output(NULL);
       
        etimer_set(&periodic_timer, T_SLOT); 
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
       
        etimer_set(&periodic_timer, T_GUARD); //Time between polls
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

      }
      else{
        sensor_type_byte = sensor_type_byte >> 1;
        dt = clock_time()-t;
        printf("Not Polling node %d, dt: %lu\n", i2, dt/CLOCK_SECOND);


        etimer_set(&periodic_timer, T_SLOT); 
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
        etimer_set(&periodic_timer, T_GUARD); //Time between polls
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

      }
    }

    printf("All finished\n\n\n\n");

     sensor_type.value = random_rand(); //Create random sensor type
  //printf ("%d\n", sensor_type.value);
    printf("new R0: %d\t, R1: %d\n", sensor_type.bytes[0], sensor_type.bytes[1]);
 
     //Get random uint8_t
    memcpy(&sensor_type_byte, &sensor_type.bytes[0], sizeof(sensor_type.bytes[0]));
    printf("new Bit mask: %d\n", sensor_type_byte);

    //Choose between 3 random values for count
    /*
    unsigned r = random_rand() % 5;
    count = r;
    LOG_INFO("Sending %u to ", count);
    LOG_INFO_LLADDR(NULL);
    LOG_INFO_("\n");
    
    memcpy(nullnet_buf, &count, sizeof(count));
    nullnet_len = sizeof(count);

    NETSTACK_NETWORK.output(NULL);
    etimer_reset(&periodic_timer);
    */
  }

  PROCESS_END();
}
/*
PROCESS_THREAD(beacon_process, ev, data)
{   
  
  static struct etimer send_timer;
    
    PROCESS_BEGIN();  
    while(1){
       PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

       static uint8_t i;
       for (i=0; i<3; i++) { 
        etimer_set(&send_timer, T_GUARD); //Time between beacons
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

        LOG_INFO("Beacon %d sent, data %d\n", (i+1), sensor_type_byte);
        nullnet_buf = (uint8_t *)&sensor_type_byte;
        nullnet_len = sizeof(sensor_type_byte);
        NETSTACK_NETWORK.output(NULL); 
        PROCESS_EXIT();
        }
             
      
    }
   

   

    PROCESS_END();


}
*/


