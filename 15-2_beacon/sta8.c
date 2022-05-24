#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include "dev/radio.h"

#include "net/packetbuf.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define f_BEACON 0x00
#define f_POLL 0x01
#define f_DATA 0x02
#define f_ENERGEST 0x03

#define NODEID  8

#define T_MDB  (10 * CLOCK_SECOND)
#define T_SLOT  (1.5 * CLOCK_SECOND)
#define T_GUARD  (0.5 * CLOCK_SECOND)
#define T_BEACON (360 * CLOCK_SECOND)

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



void datasender( uint8_t id )  
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
    switch(id) {
        case 1:
        case 3:
            printf("Node %d, multigas\n", id);

            megabuf[0] = id;
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
    }
}





/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)

static linkaddr_t coordinator_addr ; 
/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
PROCESS(parser_process, "Parsing process");
AUTOSTART_PROCESSES(&nullnet_example_process, &parser_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("Callback received rx: %d\n", *(uint8_t *)data);
  coordinator_addr = *src;                                //do we use this for anything?????
  process_poll(&parser_process);  
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
   static struct etimer periodic_timer;
  
  //static unsigned count = 0;
  //static radio_value_t txpower;

  PROCESS_BEGIN();
  nullnet_set_input_callback(input_callback);
  while(1) {
    etimer_set(&periodic_timer, CLOCK_SECOND);
    PROCESS_YIELD();
   
  }

  PROCESS_END();
}

PROCESS_THREAD(parser_process, ev, data)
{
    static clock_time_t time_until_poll;
    static struct etimer radiotimer;
    static struct etimer next_beacon_etimer;
    static struct etimer btimer; 
    static uint8_t* buf;

    


    PROCESS_BEGIN();
    //nullnet_set_input_callback(input_callback);
    printf("my node id %d\n", NODEID);
    

    while(1) {

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

    
    //?????????????????????? ZONE
    
    buf = packetbuf_dataptr();	
    //uint8_t* sensor_reading = (uint8_t*)buf;


    printf("PROCESS:\tReceived msg from ");
    LOG_INFO_LLADDR(&coordinator_addr);
    LOG_INFO_(" address \n");

    static uint8_t frame;
    static uint8_t B_f;
    //static uint8_t B_timer;
    static uint8_t B_n;

    frame = buf[0];
    //uint8_t* data = buf + 1;
    B_f = (frame & 0b11000000) >> 6;
    
    //B_timer = frame & 0x3f;
    B_n = (frame & 0b00110000) >> 4;
    printf("B_n %d B_f %d \n", B_f, B_n);

    //printf("frame: %d\t", B_f);
    // printf("Beacon_timer: %d\n", B_timer);

    //TODO: do something with the timer, synchronize with the coordinator (r = 0.1s, 0.5s, 1s...? )


    if(B_f== f_BEACON){
        printf("BEACON frame! n: %d, bitmask: %d \n", B_n, buf[1]);

       
        if(B_n==0){
            etimer_set(&btimer, CLOCK_SECOND);
        }
        else if(B_n==1){
            etimer_set(&btimer, CLOCK_SECOND*0.5);
        }
        uint8_t bitmask = buf[1];
        static uint8_t i_buf = 0;
        
        int i;

        if(txflag==0){ //only print once
            printf("beacon is asking for ");
            for (i = 0; i < 8; i++) {
                if (bitmask & (1 << i)) {
                    printf("%d \t", (i+1));
                    if((i+1) == NODEID)
                    {   
                        i_buf = i;
                        txflag = 1;
                    }
                }          
              
            } 
        }
        if(B_n != 2) 
        {
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&btimer));

        }
        //PROCESS_WAIT_UNTIL( B_n == 2 || timer_expired(&btimer));
       
        clock_time_t bufvar = 357*CLOCK_SECOND;
        printf("setting timer for %lu ticks, %lu seconds (+3) until beacon\n", bufvar, (bufvar/CLOCK_SECOND));

        etimer_set(&next_beacon_etimer, (357*CLOCK_SECOND)); //use rtimer maybe?

        if(txflag) {
            printf("I'm transmitting in the %dth slot\n", (i_buf+1));
            
            time_until_poll = T_MDB + ((NODEID-1) * (T_SLOT + T_GUARD)) - T_GUARD;
            printf("radio off, time until radio on: %lu ticks, %lu seconds\n", time_until_poll ,time_until_poll/CLOCK_SECOND);              
            //NETSTACK_RADIO.off();
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            etimer_set( &radiotimer, time_until_poll);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));
            
            NETSTACK_RADIO.on();
            
            
            //NETSTACK_MAC.on(); //test this
            
            //NETSTACK_RADIO.on();
            printf("radio back on\n");
            txflag = 0;

        }
        else{
            printf("Radio off until the next beacon\n");
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            etimer_set( &radiotimer, T_BEACON - 2*CLOCK_SECOND);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));

            NETSTACK_RADIO.on();

            printf("radio back on, beacon in ~2s\n");
        }    
    }
    if( B_f == f_POLL)
    {
        printf("POLL frame!\n");
        printf("received poll for %d, I am node %d\n", buf[1], NODEID);
        if(buf[1] == NODEID)
        {
            //printf("I'm transmitting in the %dth slot\n", buf[1]);
            datasender(buf[1]);
            NETSTACK_RADIO.off();
            RTIMER_BUSYWAIT(5);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&next_beacon_etimer));
            NETSTACK_RADIO.on();
            printf("radio back on, beacon in ~2s \n");
        }
        else
        {
            printf("Error: I'm awake in the %d slot and I am %d \n", buf[1], NODEID);
        }
             
        //Here, depending on our nodeid we will send the requested data to the coord 

    }
    else if (B_f == f_DATA){
        printf("Data frame\n");
        //STAs do nothing here
    }
        
    else if (B_f ==f_ENERGEST){
        printf("Energest frame\n");
        
        //instead of sending data, send energest metrics
    }


    else{
        printf("Unknown frame\n");
        printf("B_f  %02x , B_n %02x, buf[0,1,2]: %02x %02x %02x\n", B_f, B_n, buf[0], buf[1], buf[2] );
    } 
    
    }


    PROCESS_END();

}




      


