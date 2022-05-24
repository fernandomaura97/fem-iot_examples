#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"
#include <string.h>
#include <stdio.h> 
#include "random.h"
#include "sys/clock.h"
#include <stdlib.h>

#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"

#include "sys/log.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG


#define NODEID_STATIC 1
#define NODEID_DHT22_1 2
#define NODEID_DHT22_2 3
#define NODEID_MGAS1 4 ///CUIDADO!!!!
#define NODEID_MGAS2 4
#define NODEID_O3_1 5
#define NODEID_O3_2 6
#define NODEID_PM10_1 7
#define NODEID_PM10_2 8


//placeholder struct for the data of the sensors: 
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
static uint16_t cb_len;
static linkaddr_t from; 

#define DEBUG 0

/* PRINTF for debug? otherwise use LOG_DBG with LOG_LEVEL_DBG and not LOG_LEVEL_INFO
#if DEBUG

#define PRINTF(...) printf(__VA_ARGS__)

#else //

#define PRINTF(...)

#endif //
*/

#define ROUTENUMBER 8 //for now, then it should be bigger


static char *rxdata;
static uint8_t bitmask;

static uint16_t lost_message_counter = 0;
static bool poll_response_received = 0; 
static linkaddr_t addr_stas[ROUTENUMBER]; //store sta's addresses in here, for routing and sending
static linkaddr_t buffer_addr; 

const linkaddr_t addr_empty = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; //placeholder address

#define SPEED_NW 1 //Speed >1 if we want faster beacons and times ( for debugging "quicker" without changing too much code)
//#define SEND_INTERVAL (220 * CLOCK_SECOND * (1/SPEED_NW))
#define BEACON_INTERVAL (60* CLOCK_SECOND * (1/SPEED_NW))
#define T_MM (10* CLOCK_SECOND  * 1/SPEED_NW)
#define T_GUARD (0.5 * CLOCK_SECOND * 1/SPEED_NW)
#define T_SLOT (1.5 * CLOCK_SECOND *  1/SPEED_NW)

#define f_BEACON 0x00
#define f_POLL 0x01
#define f_DATA 0x02
#define f_ENERGEST 0x03

//static uint8_t *buf;
uint8_t global_buffer[20];
/*---------------------------------------------------------------------------*/
PROCESS(coordinator_process, "fem-iot coordinator process");
//PROCESS(beacon_process, "beacon process");
PROCESS(serial_process, "Serial process");
//PROCESS(parser_process, "Parsing process");
PROCESS(association_process, "Association process");
PROCESS(callback_process,"Callback process");

AUTOSTART_PROCESSES(&coordinator_process, &association_process, &callback_process, &serial_process);

/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{   

    from = *src;
    cb_len = len; //save the length of the received packet
    packetbuf_copyto(&global_buffer); //copy the received packet to the buffer
    

    process_poll(&callback_process);
}

/*--------------------------------------------------------------------------------*/

PROCESS_THREAD(coordinator_process, ev,data)
{
    static struct etimer periodic_timer;
    static struct etimer guard_timer;
    static struct etimer mm_timer;
    static struct etimer beacon_timer;

    
    static uint8_t beaconbuf[3];

    static clock_time_t t;
    static clock_time_t dt;
    static uint8_t pollbuf[2];
    
    PROCESS_BEGIN();

    //bitmask = random_rand();
    bitmask = 0xFF;
    printf("bitmask = %d\n", bitmask);


    //What do we do with this???
    beaconbuf[0] = 0x00;
    beaconbuf[1] = bitmask;
    beaconbuf[2] = 0x00;

    nullnet_buf = (uint8_t*)&beaconbuf;
    nullnet_len = sizeof(beaconbuf);
    nullnet_set_input_callback(input_callback);

    //setup time
    etimer_set(&periodic_timer, 5*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    while(1)
    {
        LOG_DBG("Bitmask is %d\n", bitmask);
        printf("AA0\n"); //NODE-RED HEARTBEAT
        printf("LMC,%d\n",lost_message_counter);
        etimer_set(&beacon_timer, BEACON_INTERVAL); //set the timer for the next interval
        
        static uint8_t i;

        for (i= 0; i<3; i++) 
        {
            beaconbuf[1] = bitmask; 
           
            beaconbuf[0] = i; 
            nullnet_buf = (uint8_t*)&beaconbuf;
            nullnet_len = sizeof(beaconbuf);
            
            NETSTACK_NETWORK.output(NULL);
            LOG_INFO("beacon %d sent, length %d, bitmask %d\n", i,sizeof(beaconbuf), beaconbuf[1]);
            
            if(i<2){
            etimer_set(&guard_timer, T_GUARD); //set the timer for the next interval
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&guard_timer));
            }
        }
        
        etimer_set(&mm_timer, T_MM); /// T_MDB wait
        
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&mm_timer));

        t=clock_time();
        printf("seconds since boot: %lu\n", t/CLOCK_SECOND);
        
        for(i = 1; i<9; i++)
        {
           if(bitmask & 0x01){
               bitmask = bitmask >> 1;
                pollbuf[0] = 0b01100000; //poll is 3
                pollbuf[1] = i;

                nullnet_buf = (uint8_t*)&pollbuf;
                nullnet_len = sizeof(pollbuf);

                dt = clock_time() - t;
                LOG_INFO("polling node %d, dt: %lu\n", i, dt/CLOCK_SECOND);
               
               NETSTACK_NETWORK.output(NULL); //POLLING IS BROADCAST, SHOULD BE UNICAST??

                
                
                etimer_set(&periodic_timer, T_SLOT); //set the timer for the next interval
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                
                if(!poll_response_received){
                    
                    LOG_INFO("polling node %d, no response!! TRYING AGAIN \n", i);

                    // HERE, TRY AGAIN? 
                    NETSTACK_NETWORK.output(NULL);; 
                }

                etimer_set(&periodic_timer, T_GUARD);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                if(!poll_response_received){

                    if(i<3){ //if we are in the first 3 nodes, account for error ( ONLY FOR TEST WITH 3 NODES)

                        LOG_INFO("ERROR: POLL no RESPONSE!!\n");
                        lost_message_counter++;
                    }
                    else{
                        LOG_INFO("no response(EXPECTED) \n");
                    }
                   
                }
                poll_response_received = 0;


           }
              else{
                bitmask = bitmask >> 1;
                dt = clock_time() - t;
                LOG_INFO("NOT polling node %d, dt: %lu\n", i, dt/CLOCK_SECOND);
                etimer_set(&periodic_timer, T_SLOT); //set the timer for the next interval
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                
                etimer_set(&periodic_timer, T_GUARD);
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                
              }
        }
        LOG_INFO("Polling finished\n");
      
         //if we need to change bitmask for next loop, do it here      
         //also use this time for uplink and downlink extra communications

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&beacon_timer));
        
        LOG_DBG("finished loop\n");
    } //while
    
    PROCESS_END();

}

/*-----------------------------------------------------------------------------------------*/




PROCESS_THREAD(association_process,ev,data){

    uint8_t buf_assoc[2];
    static uint8_t id_rx;
    uint8_t i;
    uint8_t oldaddr = 0; 
    
    PROCESS_BEGIN();
    
    while(1){
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

        LOG_DBG("rx messg from nodeid: %d\n", global_buffer[1]);
        
        id_rx = global_buffer[1];


        for (i= 0; i<ROUTENUMBER; i++){
            //if (addr_stas[i] == *buffer_addr){
            if (linkaddr_cmp(&addr_stas[i], &buffer_addr)){ //if they are the same then
            oldaddr = 1;
            printf("Address already found at pos %d\n", i);
            break;     

            }   //if we have this address in our list, we don't need to add it again                
        }

        if(!oldaddr){
            
            for(i = 0; i<ROUTENUMBER; i++){

                if(linkaddr_cmp(&addr_stas[i], &addr_empty))
                    {
                    linkaddr_copy(&addr_stas[i], &buffer_addr); //if we don't have it, add it
                    printf("Address ");
                    LOG_INFO_LLADDR(&buffer_addr);
                    printf(" added at pos %d\n", i);

                    buf_assoc[0] = 0b01000000;
                    buf_assoc[1] = id_rx;

                    nullnet_buf = (uint8_t*)&buf_assoc;
                    nullnet_len = sizeof(buf_assoc);

                    NETSTACK_NETWORK.output(&buffer_addr);
                    printf("Association message sent to node %d\n", id_rx);
                    break;
                    //oldaddr = 1;     
                    }
            }
        }

 }//while
    
    PROCESS_END();
}

PROCESS_THREAD(callback_process,ev,data){

     uint32_t fbuf; //float buffer
    union {
        float float_variable;
        uint8_t temp_array[4];
        } u;
  
  /*  union{
        uint32_t u32_var;
        uint8_t temp_array[4];
        } ua;
*/
    union{
        int16_t u16_var;
        uint8_t temp_array[2];
        } ua2;
    uint8_t *buf;
    uint8_t frame; 
    PROCESS_BEGIN();


    while(1) {      
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        //buf = (uint8_t *)malloc(cb_len);
        buf = packetbuf_dataptr();
        frame = (buf[0] & 0b11100000)>>5;
        
        if (frame == 0){
            LOG_DBG("Beacon received ??\n");
        }
        else if( frame ==1){
            LOG_DBG("Association request received\n");
            linkaddr_copy(&buffer_addr, &from );
            process_poll(&association_process);     
        }
        else if( frame ==2){
            LOG_DBG("Association response received ??\n");
        }
        else if( frame ==3){
            LOG_DBG("Poll request received ?? \n");
        }
        else if( frame ==4){
            LOG_DBG("Sensor Data received\n");
            //process_poll(&parser_process);
                    poll_response_received = 1; //we received a poll response
                    printf("PARSING\n");
                    //switch(buf[0] & 31){
                    switch(buf[0] & 0b00011111) //last 5 bits of the first byte is for NodeID?
                    {   

                                               
                        //case NODEID_MGAS1:
                        case NODEID_MGAS2:
                                    
                        u.temp_array[0] = buf[1];
                        u.temp_array[1] = buf[2];
                        u.temp_array[2] = buf[3];
                        u.temp_array[3] = buf[4];
                        sensors.co = u.float_variable;
                    

                        u.temp_array[0] = buf[5];
                        u.temp_array[1] = buf[6];
                        u.temp_array[2] = buf[7];
                        u.temp_array[3] = buf[8];
                        sensors.no2 = u.float_variable;


                        printf("{\"nodeID\": %d", buf[0] & 0b00011111);
                        printf(",\"co\": ");
                        fbuf = sensors.co * 100;
                        printf("%lu.%02lu", fbuf/100, fbuf%100);
                        printf(", \"no2\": ");
                        fbuf = sensors.no2 * 100;
                        printf("%lu.%02lu", fbuf/100, fbuf%100);
                        printf("}\n");    
                        break;


                    case NODEID_STATIC:
                    case NODEID_DHT22_1:
                    case NODEID_DHT22_2:
                    

                        ua2.temp_array[0] = buf[1];
                        ua2.temp_array[1] = buf[2];
                        
                        memcpy(&sensors.temperature, &ua2.u16_var, sizeof(int16_t)); 
                    

                        ua2.temp_array[0] = buf[3];
                        ua2.temp_array[1] = buf[4];
                        memcpy(&sensors.humidity, &ua2.u16_var, sizeof(int16_t));

                    
                        printf("{\"nodeID\": %d", buf[0] & 0b00011111);
                        printf(",\"Humidity\": %d.%d",  sensors.humidity/10, sensors.humidity%10);
                        printf(",\"Temperature\": %d.%d""}\n",  sensors.temperature/10, sensors.temperature%10);

                        if((buf[0] & 0b00011111)== NODEID_DHT22_2){


                            
                        }
                    
                        break;
                        
                    case NODEID_O3_1:
                    case NODEID_O3_2:
                        
                        ua2.temp_array[0] = buf[1];
                        ua2.temp_array[1] = buf[2];
                        
                        memcpy(&sensors.temperature, &ua2.u16_var, sizeof(int16_t)); 
                    

                        ua2.temp_array[0] = buf[3];
                        ua2.temp_array[1] = buf[4];
                        memcpy(&sensors.humidity, &ua2.u16_var, sizeof(int16_t));

                        u.temp_array[0] = buf[5];
                        u.temp_array[1] = buf[6];
                        u.temp_array[2] = buf[7];
                        u.temp_array[3] = buf[8];
                        
                    
                        memcpy(&sensors.o3, &u.float_variable, sizeof(float));
                        fbuf = sensors.o3 * 100;
                        


                        printf("{\"nodeID\": %d", buf[0]&0b00011111);
                        printf(",\"ppm\": ");
                        printf("%lu.%02lu", fbuf/100, fbuf%100);
                    
                        printf(",\"Humidity\": %d.%d", sensors.humidity/10, sensors.humidity%10);
                        printf(",\"Temperature\": %d.%d", sensors.temperature/10, sensors.temperature%10);
                        printf("}\n");
                        break;
                    case NODEID_PM10_1:
                    case NODEID_PM10_2:
                    
                        sensors.pm10 = (buf[2] << 8) | buf[1];
                        printf("{\"nodeID\": %d", buf[0]&0b00011111);
                        printf(",\"pm10\": %d", sensors.pm10);
                        printf("}\n");
                        break;
                        //AOK!!
                    
                    default:
                        /*printf("unknown nodeID %d\n", buf[0]);
                        printf("BYTES copied are: ");
                        for (int i = 0; i < len; i++) {
                        printf("%d ", buf[i]);
                        */        
                        break;
                    } //switch

        }
        else if( frame ==5){
            LOG_DBG("Energest Data received\n");
        }
        else{
            LOG_DBG("Unknown packet received\n");
        }
        
        //free(buf);
    
    
    } //while      
    PROCESS_END();
}

PROCESS_THREAD(serial_process, ev, data)
{
  
  PROCESS_BEGIN();
  uart_set_input(0, serial_line_input_byte);
  leds_toggle(LEDS_GREEN);

  while(1) {
    PROCESS_YIELD();                           // Surt del procés temporalment, fins que arribi un event/poll
    
    if(ev == serial_line_event_message) {      // Si l'event indica que ha arribat un missatge UART
      leds_toggle(LEDS_RED);
      rxdata = data;                           // Guardem el missatge a rxdata
      printf("Data received over UART: %s\n", rxdata);    //Mostrem el missatge rebut.
      leds_toggle(LEDS_RED); 

      char buffer_header[30];
      strcpy(buffer_header, rxdata);
      printf("buffer header: %s\n", buffer_header);
      char *header; 
      header = strtok(buffer_header, ",");
      printf("header: %s\n", header);



    if (strcmp(header, "BM") == 0) { //If we received a new BM from the GW
        
        
        char *token = strtok(NULL, ",");
        uint8_t buf_bitmask = 0;

        if(token != NULL) { //only once
            
            LOG_DBG("token: %s\n", token);
            buf_bitmask = atoi(token);

            LOG_DBG("bitmask value (serial): %d\n", buf_bitmask);
            bitmask = buf_bitmask; //store received bitmask for next cycle
            

        }   
        else{
            LOG_ERR("Error parsing bitmask\n");
        }
    }    
   }
    
  }

  PROCESS_END();
}