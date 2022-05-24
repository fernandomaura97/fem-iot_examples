#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include "dev/radio.h"
#include <stdlib.h>
#include "net/packetbuf.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG

#define DEBUG 1

#if DEBUG

#define PRINTF(...) printf(__VA_ARGS__)

#else //

#define PRINTF(...)

#endif //

/*
#define f_BEACON 0x00
#define f_POLL 0x01
#define f_DATA 0x02
#define f_ENERGEST 0x03
*/


#define NODEID  1

#define T_MDB  (10 * CLOCK_SECOND)
#define T_SLOT  (1.5 * CLOCK_SECOND)
#define T_GUARD  (0.5 * CLOCK_SECOND)
#define T_BEACON (360 * CLOCK_SECOND)

//STATIC STRUCT FOR FLAGS?


static linkaddr_t from;
static linkaddr_t coordinator_addr = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }};
static clock_time_t time_until_poll;
static struct etimer radiotimer;
static struct etimer next_beacon_etimer;
//static struct rtimer next_beacon_rtimer;
static struct etimer btimer;    
static clock_time_t time_of_beacon_rx;
static uint16_t len_msg;
volatile static bool is_associated;

static uint8_t bitmask;
static volatile bool amipolled;
static volatile uint8_t i_buf;

const uint8_t power_levels[3] = {0x46, 0x71, 0x7F}; // 0dB, 8dB, 14dB


/*---------------------------------------------------------------------------*/
 
static struct mydatas {
        uint8_t NodeID;
        int16_t temperature;
        int16_t hum;
        float co ;
        float no2 ;
        float o3 ;
        uint32_t noise ; 
        uint16_t pm10;

    } mydata;

static bool txflag = 0;

uint8_t who_bitmask(uint8_t b) //prints which nodes the beacon is polling, and if it's own NODEID return position of poll
{
    uint8_t i2;         
    uint8_t pos = 0;
    amipolled = 0; //reset amipolled flag
    
    printf("Beacon is asking for: ");
    for (i2 = 0; i2 < 8; i2++) {
        if (bitmask & (1 << i2)) { //for each bit in bitmask
            printf("%d \t", (i2+1));
            if((i2+1) == NODEID)
            {   
                pos = i2+1;
                amipolled = 1;
            }
        }   
    }
    printf("\n");
    return pos;
}



uint8_t datasender( uint8_t id )  
{
    uint8_t megabuf[9];
    mydata.co = 1.23;
    mydata.no2 = 2.34;
    mydata.o3 = 3.45;
    mydata.noise = 4560;
    mydata.pm10 = 45;
    mydata.hum = 560;
    mydata.temperature = 670;

    nullnet_buf = (uint8_t *)&megabuf;
    nullnet_len = sizeof(megabuf);   
         
    /*switch(id) {
        
        case 1:
        case 3:
            printf("Node %d, multigas\n", id);

            //megabuf[0] = id;
            megabuf[0] = 0b10000000 | id;
            memcpy(&megabuf[1], &mydata.co, sizeof(mydata.co));
            memcpy(&megabuf[5], &mydata.no2, sizeof(mydata.no2));
            printf("Sending %d %d %d %d %d %d %d %d %d\n", megabuf[0], megabuf[1], megabuf[2], megabuf[3], megabuf[4], megabuf[5], megabuf[6], megabuf[7], megabuf[8]);

            //make sure it's correct data
            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(NULL); 
            break;
        case 2:
        case 4: 
            printf("Node %d\n, dht22", id);

            megabuf[0] = id;
            memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
            memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));
            memcpy(&megabuf[5], &mydata.noise, sizeof(mydata.noise));

            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(NULL); 
            break;
        case 5:
        case 6:
          printf("Node %d\n, Ozone\n", id);
          union {
            float float_variable;
            uint8_t temp_array[4];
          } u;

          u.float_variable = mydata.o3;

          megabuf[5] = u.temp_array[0];
          megabuf[6] = u.temp_array[1];
          megabuf[7] = u.temp_array[2];
          megabuf[8] = u.temp_array[3];
          megabuf[0] = id;

          memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
          memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));

          nullnet_buf = (uint8_t *)&megabuf;
          nullnet_len = sizeof(megabuf);
          NETSTACK_NETWORK.output(NULL); 
          break;
        case 7:
        case 8:
            printf("Node %d, PM10\n", id);

            megabuf[0] = id;
            memcpy(&megabuf[1], &mydata.pm10, sizeof(mydata.pm10));

            nullnet_buf = (uint8_t *)&megabuf;
            nullnet_len = sizeof(megabuf);
            NETSTACK_NETWORK.output(NULL); 
                  
            break;
        default:
            printf("?");
          
            break;
    }*/
    if(id == 1 || id ==3){
        printf("Node %d, multigas\n", id);

        //megabuf[0] = id;
        megabuf[0] = 0b10000000 | id;
        memcpy(&megabuf[1], &mydata.co, sizeof(mydata.co));
        memcpy(&megabuf[5], &mydata.no2, sizeof(mydata.no2));
        printf("Sending %d %d %d %d %d %d %d %d %d\n", megabuf[0], megabuf[1], megabuf[2], megabuf[3], megabuf[4], megabuf[5], megabuf[6], megabuf[7], megabuf[8]);

        //make sure it's correct data
        
        nullnet_buf = (uint8_t *)&megabuf;
       
        nullnet_len = sizeof(megabuf);

       
        NETSTACK_NETWORK.output(NULL); 
      
        return 1;
    }
    else if(id == 2 || id == 4){
        printf("Node %d, dht22\n", id);

        megabuf[0] = id;
        memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
        memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));
        memcpy(&megabuf[5], &mydata.noise, sizeof(mydata.noise));

        nullnet_buf = (uint8_t *)&megabuf;
        nullnet_len = sizeof(megabuf);
        NETSTACK_NETWORK.output(NULL); 
        return 1;

    }
    else if(id == 5 || id == 6){
        printf("Node %d, Ozone\n", id);
        union {
            float float_variable;
            uint8_t temp_array[4];
        } u;

        u.float_variable = mydata.o3;

        megabuf[5] = u.temp_array[0];
        megabuf[6] = u.temp_array[1];
        megabuf[7] = u.temp_array[2];
        megabuf[8] = u.temp_array[3];
        megabuf[0] = id;

        memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
        memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));

        nullnet_buf = (uint8_t *)&megabuf;
        nullnet_len = sizeof(megabuf);
        NETSTACK_NETWORK.output(NULL); 
    }
    else if(id == 7 || id == 8){
        printf("Node %d, PM10\n", id);

        megabuf[0] = id;
        memcpy(&megabuf[1], &mydata.pm10, sizeof(mydata.pm10));

        nullnet_buf = (uint8_t *)&megabuf;
        nullnet_len = sizeof(megabuf);
        NETSTACK_NETWORK.output(NULL); 
    }
    else{
        printf("?");
    }
    printf("finishhhhhh");
    return 1;

}

/*---------------------------------------------------------------------------*/

PROCESS(poll_process, "STA process");
PROCESS(associator_process,"associator process");
PROCESS(rx_process, "Radio process");

AUTOSTART_PROCESSES(&rx_process, &associator_process);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{   
  
  len_msg = len;
  //uint8_t *buf = (uint8_t *)malloc(len);
  //packetbuf_dataptr(buf, data, len); //TEST THIS
  
  process_poll(&rx_process);

}
/*
void
next_beacon_rtimer_callback(struct rtimer *timer, void *ptr)
{
    printf("radio back on from rtimer");
    NETSTACK_RADIO.on();

  // Normally avoid printing from rtimer - rather do a process poll 
}
*/


/*---------------------------------------------------------------------------*/


PROCESS_THREAD(rx_process,ev,data)
{   

    static uint8_t* datapoint;
    
    PROCESS_BEGIN();
    nullnet_set_input_callback(input_callback);

    //PROCESS_YIELD();
    while(1)
    {        
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        //uint8_t *buf = (uint8_t *)malloc(len_msg);
        //packetbuf_dataptr(buf, data, len); //TEST THIS

        datapoint = packetbuf_dataptr();
        uint8_t frame_header = (datapoint[0] & 0b11100000) >> 5;  


        switch(frame_header ) {

        case 0:
            LOG_INFO("Beacon received\n");
            if(!txflag){
                linkaddr_copy(&coordinator_addr, &from);
                process_poll(&associator_process);
            break;
            }
            else{
                LOG_INFO("ignoring beacon\n");
                break;
            }

        case 1:
            LOG_INFO("Association request received ??\n"); //not supposed to be hearing these
            break;

        case 2: 
            LOG_INFO("Association response received\n");
        
            //if(from == coordinator_addr) {
            if(linkaddr_cmp(&from, &coordinator_addr)) { 
                is_associated = 1; 
                LOG_INFO("Not associated, associating now\n");
                
                
                
            }
            else{
               LOG_DBG("error, different adresses");
            }
            
            /// ASSOC_ACK = TRUE         
            break;

        case 3:
        printf("poll request received\n");
        if(is_associated) {
            process_poll(&poll_process);    
            }
        else{
            printf("I'm not associated!!!!!\n");
        }
            //process_poll(&datasender);         TODO
            break;

        case 4: 
            LOG_INFO("Sensor data received??\n"); //not supposed to be hearing these
            break;

        case 5: 
            LOG_INFO("Energest data received\n");
            break;

        default:
            LOG_INFO("Unknown packet received\n");
            break;
        } //switch
        //free(datapoint);
    } //while
    PROCESS_END();
    
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(poll_process, ev,data){

    /*static struct mydatas {
        uint8_t NodeID;
        int16_t temperature;
        int16_t hum;
        float co ;
        float no2 ;
        float o3 ;
        uint32_t noise ; 
        uint16_t pm10;

    } mydata;
    mydata.co = 1.23;
    mydata.no2 = 2.34;
    mydata.o3 = 3.45;
    mydata.noise = 4560;
    mydata.pm10 = 45;
    mydata.hum = 560;
    mydata.temperature = 670;
    */
   static uint8_t *buffer_poll;
   clock_time_t time_after_poll;
   //static struct etimer next_beacon_etimer;

    PROCESS_BEGIN();

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

    
    buffer_poll = packetbuf_dataptr();
    

    printf("POLL frame!\n");
    printf("received poll for %d, I am node %d\n", buffer_poll[1], NODEID);
    if(buffer_poll[1] == NODEID)
        {
            //printf("I'm transmitting in the %dth slot\n", buf[1]);
            datasender(buffer_poll[1]);


            //maybe try instead of function, to do the switch case here


            printf("finished sending\n");
            NETSTACK_RADIO.off();
            printf("still here\n");
            time_after_poll = clock_time() - time_of_beacon_rx;
            etimer_set(&next_beacon_etimer, 350*CLOCK_SECOND - time_after_poll/CLOCK_SECOND);

            
            printf("setting timer for %lu seconds. Time now: %lu, Time of beacon : %lu, dt : %lu", 350*CLOCK_SECOND - time_after_poll, clock_time(), time_of_beacon_rx, time_after_poll);
            //etimer_set(&next_beacon_etimer, CLOCK_SECOND * 2);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&next_beacon_etimer));
            NETSTACK_RADIO.on();
            printf("radio back on, beacon in ~2s \n");
            
        }
        else
        {
            printf("Error: I'm awake in the %d slot and I am %d \n", buffer_poll[1], NODEID);
            NETSTACK_RADIO.off();
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&next_beacon_etimer));
            NETSTACK_RADIO.on();
            printf("radio back on, beacon in ~2s \n");
        }


    PROCESS_END();



}

PROCESS_THREAD(associator_process, ev,data){
    static struct etimer asotimer;
    //static struct rtimer next_beacon_rtimer;
    
     
    static uint8_t ix;
    static uint8_t *buf;
    static uint8_t B_n;
    PROCESS_BEGIN();

    
    
    while(1){

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL); //wait until beacon 

        buf = (uint8_t *)malloc(2); //2 BYTE
        buf = packetbuf_dataptr();
        
        printf("buf[0], buf[1] buf[2] :%02x %02x %02x ", buf[0], buf[1], buf[2]);
        
        B_n = (buf[0] & 0b00011111);
        printf("B_n = %d\n", B_n);
        //bitmask = buf[1];


        if(B_n==0 && txflag == 0){ // if it's beacon 0
            etimer_set(&btimer, CLOCK_SECOND);
            i_buf = who_bitmask(buf[1]);
            txflag = 1;            
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&btimer));
            }
        else if(B_n==1 && txflag == 0){ // if it's beacon 1
            etimer_set(&btimer, CLOCK_SECOND*0.5);
            i_buf = who_bitmask(buf[1]);
            txflag = 1;
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&btimer));
            }
        else if(B_n==2 && txflag == 0){ // if it's beacon 2
            
            i_buf = who_bitmask(buf[1]);
            txflag = 1;
        }            
        printf("I_buf == %d\n", i_buf);
        clock_time_t bufvar = 357*CLOCK_SECOND;
        printf("setting timer for %lu ticks, %lu seconds (+3) until beacon\n", bufvar, (bufvar/CLOCK_SECOND));
        
        //rtimer_set(&next_beacon_rtimer, RTIMER_NOW() + 357*RTIMER_SECOND , 0, &next_beacon_rtimer_callback, NULL); //we can pass an argument here, but we don't need it

        /*---------------------------------------------------------------------------*/                                                                         //TESTS
        PROCESS_CONTEXT_BEGIN(&poll_process);
        //etimer_set(&next_beacon_etimer, (357*CLOCK_SECOND)); //use rtimer maybe?
        time_of_beacon_rx = clock_time();
        

        
      
        //etimer_set(&next_beacon_etimer, (357*CLOCK_SECOND)); //use rtimer maybe?
        PROCESS_CONTEXT_END(&poll_process);
        /*---------------------------------------------------------------------------*/                                                                         //TESTS

        etimer_set(&btimer, CLOCK_SECOND); //add some guard time, if not it will do the association during the beacons
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&btimer));

        time_until_poll = (T_MDB + ((NODEID-1) * (T_SLOT + T_GUARD)) - 1.5*CLOCK_SECOND);
        printf("radio off, time until radio on: %lu ticks, %lu seconds\n", time_until_poll ,time_until_poll/CLOCK_SECOND);
        etimer_set(&radiotimer, time_until_poll);
        
        while(!is_associated)
            {
            //augment TX POWER and send again. If no response, "poison"
            etimer_set(&asotimer, 5* CLOCK_SECOND);

            NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, power_levels[ix]);
            
            printf("tx power: %02x\n", power_levels[ix]);

            /*---------------------------------------------------------------------------*/
            //WHAT TO SEND IN HERE? association request: 0x0

            buf[0] = 0b00100000; //association request
            buf[1] = NODEID; 
            //buf[0] |= ???;
            /*---------------------------------------------------------------------------*/
            nullnet_buf = (uint8_t *)buf;
            nullnet_len = sizeof(*buf);
            NETSTACK_NETWORK.output(&coordinator_addr);
            
            ix++;
            if(ix >= 3)
            {
                LOG_INFO("Association process failed\n");
                //POISON!
                /*---------------------------------------------------------------------------*/
                break; 
            }

            
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&asotimer));
   
            } //while
            //uint8_t *buf = (uint8_t *)malloc(len_msg);

        if( amipolled == 1   ) {
            printf("I'm transmitting in the %dth slot\n", (i_buf));
            
            //time_until_poll = T_MDB + ((NODEID-1) * (T_SLOT + T_GUARD)) - T_GUARD;
            //printf("radio off, time until radio on: %lu ticks, %lu seconds\n", time_until_poll ,time_until_poll/CLOCK_SECOND);              
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            //etimer_set( &radiotimer, time_until_poll);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));
            
            NETSTACK_RADIO.on();
            printf("radio back on\n");
            txflag = 0;

        }
        else if (amipolled == 0) { //if not polled, just wait for the next beacon
            printf("Radio off until the next beacon  11\n");
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            etimer_set( &radiotimer, T_BEACON - 2*CLOCK_SECOND);           //PROBABLY THIS IS WRONG!!! FIX FIX FIX
            //instead of using 2*clock_second maybe get the system time at the beginning of the beacon and subtract that from the current time
            //*******************************************************************************************************/
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));

            NETSTACK_RADIO.on();

            printf("radio back on, beacon in ~2s\n");
        }        
        
        //CODE STARTS HERE
    }

    PROCESS_END();

}



