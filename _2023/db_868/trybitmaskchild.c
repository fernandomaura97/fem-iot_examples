#include "contiki.h"

#include <stdio.h> /* For printf() */
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"



#define NODEID1 1  // non-adaptive, test
#define NODEID2 2 //DHT22
#define NODEID3 4 // PM10 
#define NODEID4 8
#define NODEID5 16
#define NODEID6 32
#define NODEID7 64
#define NODEID8 128

#define T_SLOT (CLOCK_SECOND)
#define T_GUARD (0.5*CLOCK_SECOND)
struct childs_polled_t {
    uint8_t nodeid1;
    uint8_t nodeid2;
};
typedef struct childs_polled_t childs_polled;

childs_polled get_childs_ID_m(uint8_t nodeid_t, childs_polled childs)
{
    
    //printf("nodeid_t: %d\n", nodeid_t);
    switch (nodeid_t)
    {
        case 1:
            childs.nodeid1 = 1;
            childs.nodeid2 = 2;
            break;
        case 2:
            childs.nodeid1 = 3;
            childs.nodeid2 = 4;
            break;
        case 3:
            childs.nodeid1 = 5;
            childs.nodeid2 = 6;
            break;
        case 4:
            childs.nodeid1 = 7;
            childs.nodeid2 = 8;
            break;
        default:
            break;  
    }
    return childs;
}
childs_polled get_childs_ID(uint8_t nodeid_t, childs_polled childs)
{
    
    //printf("nodeid_t: %d\n", nodeid_t);
    switch (nodeid_t)
    {
        case NODEID1:
            childs.nodeid1 = NODEID1;
            childs.nodeid2 = NODEID2;
            break;
        case NODEID2:
            childs.nodeid1 = NODEID3;
            childs.nodeid2 = NODEID4;
            break;
        case NODEID3:
            childs.nodeid1 = NODEID5;
            childs.nodeid2 = NODEID6;
            break;
        case NODEID4:
            childs.nodeid1 = NODEID7;
            childs.nodeid2 = NODEID8;
            break;
        default:
            break;  
    }
    return childs;
}

childs_polled are_childs_polled( childs_polled childs, uint8_t bitmask_in)
{
    uint8_t masked1 = bitmask_in & childs.nodeid1;
    uint8_t masked2 = bitmask_in & childs.nodeid2;
    
    if(masked1 & childs.nodeid1) {
            childs.nodeid1 = 1;
        }
    else{
            childs.nodeid1 = 0;
        }
    
    if (masked2 & childs.nodeid2) {
            childs.nodeid2 = 1;
        }
    else{
            childs.nodeid2 = 0;
        }
    return childs;
}
/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  static struct etimer timer;
  static uint8_t bitmask; 
  uint8_t pollbuf[2];
  static bool poll_response_received = false;
  static uint8_t i; 
  static uint8_t current_pollDB = 0; 
  static struct flags_t {
   uint8_t nodeDB1;
    uint8_t nodeDB2;
    uint8_t nodeDB3;
    uint8_t nodeDB4;
    } flags;

  PROCESS_BEGIN();

  /* Setup a periodic timer that expires after 10 seconds. */
  

  while(1) {
    //get_childs_ID()
    printf("starting test\n\n");
    bitmask = 0b00011001; 

    


    for(i = 1; i<9; i++)
    {   
        if(bitmask & 0x01){

            bitmask = bitmask >> 1;
           
            switch(i){

                case 1: 
                case 2: 
                    if(flags.nodeDB1 == 0){
                        current_pollDB = 1;
                        flags.nodeDB1 = 1;
                        printf("polling node DB1\n");
                        pollbuf[0] = 0b01100000; //poll is 3
                        pollbuf[1] = 1;
                        nullnet_buf = (uint8_t*)&pollbuf;
                        nullnet_len = sizeof(pollbuf);
                        NETSTACK_NETWORK.output(NULL); 
                    
                    }
                    else{
                        printf("node DB1 already polled\n");
                    }
                    break;

                case 3: 
                case 4:
                    current_pollDB = 2;
                    if(flags.nodeDB2 == 0){
                        flags.nodeDB2 = 1;
                        printf("polling node DB2\n");

                        pollbuf[0] = 0b01100000; //poll is 3
                        pollbuf[1] = 2;
                        nullnet_buf = (uint8_t*)&pollbuf;
                        nullnet_len = sizeof(pollbuf);
                        NETSTACK_NETWORK.output(NULL); 
                    }
                    else{
                        printf("node DB2 already polled\n");
                    }
                    break; 
                case 5:
                case 6:
                    current_pollDB = 3;
                    if(flags.nodeDB3 == 0){
                        flags.nodeDB3 = 1;
                        printf("polling node DB3\n");
                        pollbuf[0] = 0b01100000; //poll is 3
                        pollbuf[1] = 3;
                        nullnet_buf = (uint8_t*)&pollbuf;
                        nullnet_len = sizeof(pollbuf);
                        NETSTACK_NETWORK.output(NULL); 
                    }   
                    else{
                        printf("node DB3 already polled\n");
                    }
                    break;

                case 7:
                case 8:
                    current_pollDB = 4;
                    if(flags.nodeDB4 == 0){
                        flags.nodeDB4 = 1;
                        printf("polling node DB4\n");
                        
                        pollbuf[0] = 0b01100000; //poll is 3
                        pollbuf[1] = 4;
                        nullnet_buf = (uint8_t*)&pollbuf;
                        nullnet_len = sizeof(pollbuf);
                        NETSTACK_NETWORK.output(NULL); 
                    }
                    else{
                        printf("node DB4 already polled\n");
                    }
                    break;
            }

            etimer_set(&timer, 1 *CLOCK_SECOND); //set the timer for the next interval
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));  




            if(!poll_response_received){

                if(i<5){ //if we are in the first 3 nodes, account for error ( ONLY FOR TEST WITH 3 NODES)
                    childs_polled nins_polled = get_childs_ID_m(current_pollDB, nins_polled);
                    printf("no response\n");
                    printf(" { \"Nodeid_DB\": %d, \"nodeid_ch1\": %d, \"nodeid2_ch2\": %d, \"T1\": 0, \"H1\": 0, \"Pw_tx1\": 0,\"n_beacons1\": 0, \"n_transmissions1\": 0, \"permil_radio_on1\": 0,\"permil_tx1\": 0, \"permil_rx1\": 0, \"T2\": 0, \"H2\": 0, \"Pw_tx2\": 0, \"n_beacons2\": 0,  \"n_transmissions2\": 0, \"permil_radio_on2\": 0, \"permil_tx2\": 0, \"permil_rx2\": 0}\n" ,current_pollDB, nins_polled.nodeid1, nins_polled.nodeid2); 
                }
                else{
                    printf("no response(EXPECTED) \n");
                }
                   
                }
                poll_response_received = 0;         
        }
    
        else{
            bitmask = bitmask >> 1;
            etimer_set(&timer, 1*CLOCK_SECOND); //set the timer for the next interval
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
        }
     } 
    flags.nodeDB1 = 0;
    flags.nodeDB2 = 0;
    flags.nodeDB3 = 0;
    flags.nodeDB4 = 0;
    printf("test finished\n\n");
    }

  PROCESS_END();
}
