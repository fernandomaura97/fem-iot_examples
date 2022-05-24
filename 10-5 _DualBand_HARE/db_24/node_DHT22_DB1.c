#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include "dev/radio.h"
#include "dev/dht22.h"
#include <stdlib.h>
#include "net/packetbuf.h"

//#include "sys/energest.h"


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG

#define DEBUG 1
#define COOJA 0

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


#define NODEID1 1  // non-adaptive, test
#define NODEID2 2 //DHT22
#define NODEID3 4 // PM10 
#define NODEID4 8
#define NODEID5 16
#define NODEID6 32
#define NODEID7 64
#define NODEID8 128

#define NODEID NODEID1


#define T_MDB  (10 * CLOCK_SECOND)
#define T_SLOT  (1.5 * CLOCK_SECOND)
#define T_GUARD  (0.5 * CLOCK_SECOND)
#define T_BEACON (60 * CLOCK_SECOND)

static linkaddr_t from;
static linkaddr_t dualband24_addr = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}; //empty address

//TIMERS 
//static clock_time_t time_until_poll;
static clock_time_t time_of_beacon_rx;

//static struct etimer radiotimer;
static struct etimer next_beacon_etimer;
//static struct etimer b_timer; 
static struct etimer asotimer;
/*---------------------------------------------------------------------------*/
//FLAGS
static volatile bool is_associated;

static volatile bool amipolled_f; //set to 1 if the NODEID is the one polled
static volatile bool beaconrx_f; //set to 1 if a beacon(out of 3) has been received on this cycle

//VARIABLES
volatile static uint16_t len_msg;
volatile static uint8_t bitmask;
static volatile uint8_t i_buf;

static uint8_t nodeid;



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



uint8_t am_i_polled(uint8_t bitmask, uint8_t id)  //prints which nodes the beacon is polling, and if it's own NODEID return position of poll
{
    uint8_t masked = bitmask & id;
    if (masked & id){
        return 1;
    }
    else{
        return 0; 
    }         
}
//function to print the number of the NODEID (f.e. NODEID8 would return 8)
uint8_t get_nodeid(uint8_t id)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (id & (1 << i))
        {
            return (i+1);
        }
    }
    return 0;
}

void m_and_send_dht22(uint8_t id)
{
    uint8_t buf_dht22[5];
    int16_t temperature, humidity;

    SENSORS_ACTIVATE(dht22);

    if(dht22_read_all(&temperature, &humidity)!= DHT22_ERROR)
    {
        mydata.hum = humidity;
        mydata.temperature = temperature;

        LOG_DBG("Temperature: %02d.%d C\n", temperature/10, temperature%10);
        LOG_DBG("Humidity: %02d.%d \n", humidity/10, humidity%10);
    }
    else
    {
        mydata.hum = 0;
        mydata.temperature = 0;
        LOG_ERR("FAILED TO READ DHT22\n");
    }
    
    
    buf_dht22[0] = 0b10000000 | id;
    memcpy(&buf_dht22[1], &temperature, sizeof(temperature));
    memcpy(&buf_dht22[3], &humidity, sizeof(humidity));
 

    nullnet_buf = (uint8_t *)&buf_dht22;
    nullnet_len = sizeof(buf_dht22);
    NETSTACK_NETWORK.output(&dualband24_addr); 
    
    LOG_DBG("Sending DHT22 data\n");
    SENSORS_DEACTIVATE(dht22);
    return; 
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

        megabuf[0] = 0b10000000 | id;
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
        megabuf[0] = 0b10000000 | id;

        memcpy(&megabuf[1], &mydata.temperature, sizeof(mydata.temperature));
        memcpy(&megabuf[3], &mydata.hum, sizeof(mydata.hum));

        nullnet_buf = (uint8_t *)&megabuf;
        nullnet_len = sizeof(megabuf);
        NETSTACK_NETWORK.output(NULL); 
    }
    else if(id == 7 || id == 8){
        printf("Node %d, PM10\n", id);

        megabuf[0] = 0b10000000 | id;
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
//PROCESS(energest_example_process, "energest");
//AUTOSTART_PROCESSES(&rx_process, &associator_process, &poll_process, &energest_example_process);
AUTOSTART_PROCESSES(&rx_process, &associator_process, &poll_process);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{   
  linkaddr_copy(&from, src);
  len_msg = len;
  //uint8_t *buf = (uint8_t *)malloc(len);
  //packetbuf_dataptr(buf, data, len); //TEST THIS
  
  process_poll(&rx_process);

}

/*void input_callback2(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{  

  printf("HELLO CALLBACK 2 \n");
  linkaddr_copy(&from, src);
  len_msg = len;
  //uint8_t *buf = (uint8_t *)malloc(len);
  //packetbuf_dataptr(buf, data, len); //TEST THIS
  
  process_poll(&rx_process);

}
*/
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(rx_process,ev,data)
{   
    static struct etimer Beacon_no_timer;
    volatile static uint8_t* datapoint; //pointer to the packetbuf
    //static uint8_t buf[10];
    PROCESS_BEGIN();
    nullnet_set_input_callback(input_callback);
    nodeid = get_nodeid(NODEID);
    
    #if COOJA
    uint8_t seed = linkaddr_node_addr.u8[0];
    random_init(seed);
    nodeid = (random_rand() % 8) + 1;
    #endif
    printf("***NODEID***: %d\n", nodeid);

    //PROCESS_YIELD();
    while(1)
    {        
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        datapoint = packetbuf_dataptr();
        //buf[]

        uint8_t frame_header = (datapoint[0] & 0b11100000) >> 5;
        bitmask = datapoint[1];
        //switch(frame_header ) {

        if(frame_header ==0){
            printf("Beacon received from address ");
            LOG_INFO_LLADDR(&from);
            printf("\n");
                        
            if(!beaconrx_f){ //if it's the first beacon received on this cycle

                
                linkaddr_copy(&dualband24_addr, &from); //store coordinator address
                
                uint8_t Beacon_no = datapoint[0] & 0b00011111;

                if(Beacon_no == 0)
                {
                    beaconrx_f = 1;
                    etimer_set(&Beacon_no_timer, CLOCK_SECOND);
                    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
                    process_poll(&associator_process);

                }
                else if(Beacon_no == 1)
                {   
                    beaconrx_f = 1;
                    etimer_set(&Beacon_no_timer, CLOCK_SECOND/2);
                    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
                    process_poll(&associator_process);

                }
                else if(Beacon_no ==2)
                {
                    beaconrx_f = 1; //no need to wait
                    process_poll(&associator_process);
                }
                
                //we have address in &dualband24_addr, 
            }
            else{
                
                LOG_INFO("ignoring beacon\n");
            }
        }

        else if(frame_header ==1){
            LOG_INFO("Association request received ??\n"); //not supposed to be hearing these
        }

        else if(frame_header ==2){ 
            LOG_INFO("Association response received for %d\n", datapoint[1]);
        
            //if(from == dualband24_addr) {
            if(linkaddr_cmp(&from, &dualband24_addr)) { 
                LOG_INFO("Not associated, associating now\n");
                PROCESS_CONTEXT_BEGIN(&associator_process);
                is_associated = true; 
                PROCESS_CONTEXT_END(&associator_process);

                process_poll(&poll_process);
            }
            else{
                LOG_DBG("error, different adresses");
            }
        }
                
        else if(frame_header ==3) {
            if(is_associated) {
                //process_poll(&poll_process);    
                LOG_DBG("received poll message\n");
                }
            else{
                LOG_ERR("I'm not associated!!!!!\n");
            }
        }

        else if (frame_header ==4) {
            LOG_INFO("Sensor data received??\n"); //not supposed to be hearing these
        }

        else if(frame_header ==5){

            LOG_INFO("Energest data received\n");
        }

        else{
            LOG_INFO("Unknown packet received\n");
        } //switch
    } //while
    PROCESS_END();
    
}

/*---------------------------------------------------------------------------*/


PROCESS_THREAD(associator_process, ev,data){
    //static uint8_t nodeid_no;
    static uint8_t buf[2];
    PROCESS_BEGIN();
    //nullnet_set_input_callback(input_callback);
    while(1){

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL); //wait until beacon 
    
        time_of_beacon_rx = clock_time();
       

        //time_until_poll = (T_MDB + ((nodeid - 1) * (T_SLOT + T_GUARD))) - T_GUARD;
        //LOG_DBG("time until poll is %lu\n", time_until_poll/CLOCK_SECOND);

        //etimer_set(&radiotimer, time_until_poll);
        
        if(!is_associated)
        {   
            static uint8_t i_pwr = 0;            
         
            buf[0] = 0b00100000; //association request
            buf[1] = nodeid; 
            //buf[0] |= ???;
            /*---------------------------------------------------------------------------*/
            nullnet_buf = (uint8_t *)buf;
            nullnet_len = sizeof(buf);

            for (i_pwr = 0; i_pwr < 3; i_pwr++){
               

                etimer_set(&asotimer, 2* CLOCK_SECOND + (random_rand() % (CLOCK_SECOND)));   //add some jitter/randomness for the transmission   
                
                NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, power_levels[i_pwr]);
                
                
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&asotimer));
                 
                if(is_associated ==1 ){break;}

                printf("Sending assoc. Request, tx power: %02x\n", power_levels[i_pwr]);
                NETSTACK_NETWORK.output(&dualband24_addr);
                
                                
            }
            if(!is_associated) 
            {
                printf("I'm STILL not associated!!\n");
            }
        }
        else //if is associated
        {
            printf("I'm already associated\n");
            printf("bitmask: %02x, nodeid: %d\n", bitmask, nodeid);
            
            
        }






        /*---------------------------------------------------------------------------*/
        //NO NEED TO WAIT UNTIL POLL TIME
        /*---------------------------------------------------------------------------*/
        
        /*amipolled_f = am_i_polled(bitmask, nodeid);

        if( amipolled_f == 1){
            printf("I'm transmitting in the %dth slot\n", nodeid);
            
            //time_until_poll = T_MDB + ((NODEID-1) * (T_SLOT + T_GUARD)) - T_GUARD;
            //printf("radio off, time until radio on: %lu ticks, %lu seconds\n", time_until_poll ,time_until_poll/CLOCK_SECOND);              
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            //etimer_set( &radiotimer, time_until_poll);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));
            
            NETSTACK_RADIO.on();
            printf("radio back on\n");


        }
        else if (amipolled_f == 0) { //if not polled, just wait for the next beacon
            printf("Radio off until the next beacon\n");
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            etimer_set( &radiotimer, T_BEACON - 2*CLOCK_SECOND);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));

            NETSTACK_RADIO.on();

            printf("radio back on, beacon in ~2s\n");
        }   
        amipolled_f = 0; 
        beaconrx_f= 0 ; 
        */
        //CODE STARTS HERE
    }

    PROCESS_END();

}

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
    static clock_time_t time_after_poll;

    PROCESS_BEGIN();
    while(1)
    {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
       
        #if COOJA
            datasender(nodeid);
        #else
            m_and_send_dht22(nodeid);
        #endif


        printf("finished sending\n");
        NETSTACK_RADIO.off();
        RTIMER_BUSYWAIT(5);
        
        
        time_after_poll = clock_time() - time_of_beacon_rx;
        printf("setting timer for %lu seconds. Time now: %lu, Time of beacon : %lu, dt : %lu\n", T_BEACON - 10*CLOCK_SECOND - time_after_poll/CLOCK_SECOND, clock_time()/CLOCK_SECOND, time_of_beacon_rx/CLOCK_SECOND, time_after_poll/CLOCK_SECOND); // BROKEN: FIXME
        
        etimer_set(&next_beacon_etimer, T_BEACON - 3*CLOCK_SECOND - time_after_poll);
        
        PROCESS_WAIT_UNTIL(etimer_expired(&next_beacon_etimer));
        NETSTACK_RADIO.on();
        
        LOG_DBG("radio back on, beacon in ~2s!!! \n");
        PROCESS_CONTEXT_BEGIN(&rx_process);
        beaconrx_f = 0;
        PROCESS_CONTEXT_END(&rx_process);
        //TOO MUCH

        /* NETSTACK_RADIO.off();
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&next_beacon_etimer));
        NETSTACK_RADIO.on();
        printf("radio back on, beacon in ~2s \n");
        */
            

  
    }
    PROCESS_END();
}


// This Process will periodically print energest values for the last minute.


/*
PROCESS_THREAD(energest_example_process, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

  etimer_set(&periodic_timer, CLOCK_SECOND * 60);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    
     * Update all energest times. Should always be called before energest
     * times are read.
     
    energest_flush();

    printf("\nEnergest:\n");
    printf(" CPU          %4lus LPM      %4lus DEEP LPM %4lus  Total time %lus\n",
           to_seconds(energest_type_time(ENERGEST_TYPE_CPU)),
           to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
           to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
           to_seconds(ENERGEST_GET_TOTAL_TIME()));
    printf(" Radio LISTEN %4lus TRANSMIT %4lus OFF      %4lus\n",
           to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)),
           to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
           to_seconds(ENERGEST_GET_TOTAL_TIME()
                      - energest_type_time(ENERGEST_TYPE_TRANSMIT)
                      - energest_type_time(ENERGEST_TYPE_LISTEN)));
  }

  PROCESS_END();
}
*/



