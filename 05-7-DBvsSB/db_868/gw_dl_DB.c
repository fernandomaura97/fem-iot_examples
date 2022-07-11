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


#define NODEID_MGAS1 1
#define NODEID_DHT22_1 2
#define NODEID_MGAS2 3
#define NODEID_DHT22_2 4
#define NODEID_SB_1 5
#define NODEID_SB_2 6
#define NODEID_SB_3 7
#define NODEID_SB_4 8

//FOR BITMASKS
#define NODEID1 1  // non-adaptive, test
#define NODEID2 2 //DHT22
#define NODEID3 4 // PM10 
#define NODEID4 8
#define NODEID5 16
#define NODEID6 32
#define NODEID7 64
#define NODEID8 128

/*
typedef struct sensor_dataold_t {
  uint8_t nodeid;
  int16_t humidity;
  int16_t temperature;
  uint16_t pm10;
  uint32_t noise;
  float o3;
  float co;
  float no2;
} sensor_dataold_t;
static sensor_dataold_t sensors;
*/

typedef struct sensor_data_t {  
          uint8_t id1;
          uint8_t length1;
          int16_t value_temperature1;
          int16_t value_humidity1;

          uint8_t id2;
          uint8_t length2;
          int16_t value_temperature2;
          int16_t value_humidity2;
          
          } sensor_data_t;

//static struct sensor_data_t sensor_data;


#pragma pack(push,1)
    typedef struct hare_stats_t{
        uint8_t header; //header includes message type and node id
        int16_t temperature, humidity;   
        uint8_t power_tx;
        uint16_t n_beacons_received;
        uint16_t n_transmissions; 
        uint16_t permil_radio_on; // ‰ gotten through energest
        uint16_t permil_tx;
        uint16_t permil_rx;
        } hare_stats_t;
#pragma pack(pop)

#pragma pack(push,1)
    typedef struct aggregation_stats_t{
    struct hare_stats_t p1;
    struct hare_stats_t p2;
    } aggregation_msg;
#pragma pack(pop)

//static uint8_t buffer_aggregation[sizeof(aggregation_msg)]; //buffer for sending aggregated data


static uint16_t cb_len;
static linkaddr_t from; 

struct childs_polled_t {
    uint8_t nodeid1;
    uint8_t nodeid2;
};
typedef struct childs_polled_t childs_polled;

#define ROUTENUMBER 8 //for now, then it should be bigger

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

AUTOSTART_PROCESSES(&coordinator_process,&association_process , &callback_process, &serial_process);

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

    static uint8_t current_pollDB = 0; 

    static struct flags_t {
        uint8_t nodeDB1;
        uint8_t nodeDB2;
        uint8_t nodeDB3;
        uint8_t nodeDB4;
        } flags;
    
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
    etimer_set(&periodic_timer, 12*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    while(1)
    {   
        //bitmask = 0xff;
        LOG_DBG("Bitmask is %d\n", bitmask);
        printf("AA0\n"); //NODE-RED HEARTBEAT
        printf("LMC,%d",lost_message_counter);
        etimer_set(&beacon_timer, BEACON_INTERVAL); //set the timer for the next interval
        
        static uint8_t i;

        for (i= 0; i<3; i++) 
        {   leds_on(LEDS_BLUE);
            beaconbuf[1] = bitmask; 
            beaconbuf[0] = i; 
            nullnet_buf = (uint8_t*)&beaconbuf;
            nullnet_len = sizeof(beaconbuf);
            
            NETSTACK_NETWORK.output(NULL);
            LOG_INFO("beacon %d sent, length %d, bitmask %d\n", i,sizeof(beaconbuf), beaconbuf[1]);
            leds_off(LEDS_BLUE);
            if(i<2){
            etimer_set(&guard_timer, T_GUARD); //set the timer for the next interval
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&guard_timer));
            
            }
        }
        leds_off(LEDS_BLUE);
        etimer_set(&mm_timer, T_MM); /// T_MDB wait
        
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&mm_timer));

        t=clock_time();
        LOG_DBG("seconds since boot: %lu\n", t/CLOCK_SECOND);
        
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
                            LOG_INFO("polling node DB1\n");
                            pollbuf[0] = 0b01100000; //poll is 3
                            pollbuf[1] = 1;
                            nullnet_buf = (uint8_t*)&pollbuf;
                            nullnet_len = sizeof(pollbuf);
                            NETSTACK_NETWORK.output(NULL); 
                        
                        }
                        else{
                            LOG_DBG("node DB1 already polled\n");
                        }
                        break;

                    case 3: 
                    case 4:
                        current_pollDB = 2;
                        if(flags.nodeDB2 == 0){
                            flags.nodeDB2 = 1;
                            LOG_INFO("polling node DB2\n");

                            pollbuf[0] = 0b01100000; //poll is 3
                            pollbuf[1] = 2;
                            nullnet_buf = (uint8_t*)&pollbuf;
                            nullnet_len = sizeof(pollbuf);
                            NETSTACK_NETWORK.output(NULL); 
                        }
                        else{
                            LOG_DBG("node DB2 already polled\n");
                        }
                        break; 
                    
                    
                    case 5:
                    case 6: 
                    case 7:
                    case 8:      
                        current_pollDB = i;
                        LOG_INFO("polling node SB%d\n",i);

                        pollbuf[0] = 0b01100000; //poll is 3
                        pollbuf[1] = current_pollDB;
                        nullnet_buf = (uint8_t*)&pollbuf;
                        nullnet_len = sizeof(pollbuf);
                        NETSTACK_NETWORK.output(NULL); 

                        break;                    
                    
                    default:
                        LOG_ERR("error in bitmask\n");
                        break;
                        
                }                                
                
                if(i ==2 || i ==4 || i == 5 || i == 6 || i == 7 || i == 8){  //DB nodes come 2 in 2, since they have 2 child nodes assigned

                    etimer_set(&periodic_timer, T_SLOT); //set the timer for the next interval
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
                    
                    if(!poll_response_received){
                        
                        LOG_DBG("polling node %d, no response!! TRYING AGAIN \n", current_pollDB);
                    
                        NETSTACK_NETWORK.output(NULL); 
                    }

                    etimer_set(&periodic_timer, T_GUARD);
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

                    if(!poll_response_received){ //we can receive a response by this time

                        if(i<5){ //if we are polling the first 4 child nodes, account for error

                            childs_polled nins_polled = get_childs_ID_m(current_pollDB, nins_polled);
                            LOG_INFO("no response\n");
                            printf(" { \"Nodeid_DB\": %d, \"nodeid_ch1\": %d, \"nodeid2_ch2\": %d, \"T1\": 0, \"H1\": 0, \"Pw_tx1\": 0,\"n_beacons1\": 0, \"n_transmissions1\": 0, \"permil_radio_on1\": 0,\"permil_tx1\": 0, \"permil_rx1\": 0, \"T2\": 0, \"H2\": 0, \"Pw_tx2\": 0, \"n_beacons2\": 0,  \"n_transmissions2\": 0, \"permil_radio_on2\": 0, \"permil_tx2\": 0, \"permil_rx2\": 0}\n" ,current_pollDB, nins_polled.nodeid1, nins_polled.nodeid2); 
                            lost_message_counter ++;
                        }
                        else{
                            LOG_INFO("no response(EXPECTED) \n");
                        }
                
                    }
                    poll_response_received = 0;
                }             
           }            
            else{
                bitmask = bitmask >> 1;
                dt = clock_time() - t;
                LOG_INFO("NOT polling child node %d, dt: %lu\n", i, dt/CLOCK_SECOND);
                etimer_set(&periodic_timer, T_SLOT+T_GUARD); //set the timer for the next interval
                PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

                if( (i==2) & (!poll_response_received)){
                            
                            LOG_INFO("no response\n");
                            printf(" { \"Nodeid_DB\": 1, \"nodeid_ch1\": 1, \"nodeid2_ch2\": 2, \"T1\": 0, \"H1\": 0, \"Pw_tx1\": 0,\"n_beacons1\": 0, \"n_transmissions1\": 0, \"permil_radio_on1\": 0,\"permil_tx1\": 0, \"permil_rx1\": 0, \"T2\": 0, \"H2\": 0, \"Pw_tx2\": 0, \"n_beacons2\": 0,  \"n_transmissions2\": 0, \"permil_radio_on2\": 0, \"permil_tx2\": 0, \"permil_rx2\": 0}\n"); 
                }

                else if((i==4) & (!poll_response_received)){
                            
                            LOG_INFO("no response\n");
                            printf(" { \"Nodeid_DB\": 2, \"nodeid_ch1\": 3, \"nodeid2_ch2\": 4, \"T1\": 0, \"H1\": 0, \"Pw_tx1\": 0,\"n_beacons1\": 0, \"n_transmissions1\": 0, \"permil_radio_on1\": 0,\"permil_tx1\": 0, \"permil_rx1\": 0, \"T2\": 0, \"H2\": 0, \"Pw_tx2\": 0, \"n_beacons2\": 0,  \"n_transmissions2\": 0, \"permil_radio_on2\": 0, \"permil_tx2\": 0, \"permil_rx2\": 0}\n"); 
                }
            }
        }
        memset(&flags, 0, sizeof(flags));

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
            //break;     

           //NEW CODE; TEST 
            buf_assoc[0] = 0b01000000;
            buf_assoc[1] = id_rx;

            nullnet_buf = (uint8_t*)&buf_assoc;
            nullnet_len = sizeof(buf_assoc);

            NETSTACK_NETWORK.output(&buffer_addr);
            printf("REASSOCIATION message sent to node %d\n", id_rx);

            //END NEW CODE
            break;

            }              
        }

        if(!oldaddr){
            
            for(i = 0; i<ROUTENUMBER; i++){

                if(linkaddr_cmp(&addr_stas[i], &addr_empty)) //if address is empty, we can use it
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
                    memset(buf_assoc, 0, sizeof(buf_assoc));
                    break;
                    //oldaddr = 1;     
                    }
            }
        }

 }//while
    
    PROCESS_END();
}


PROCESS_THREAD(callback_process,ev,data){

       
    static uint8_t frame; 
    static uint8_t *buf; 
    static aggregation_msg ag_msg; 
    static hare_stats_t hare_stats_msg;
    PROCESS_BEGIN();


    while(1) {      

        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
        

        //buf = (uint8_t*)malloc(cb_len);
        buf = packetbuf_dataptr(); //THIS NEEDS TO BE TESTED

        //LOG_DBG("buf %d %d %d\n", buf[0], buf[1], buf[2]);
        frame = (buf[0] & 0b11100000)>>5;
        uint8_t frame2 = (buf[0] & 0b00011111);
        
        

        if (frame == 0){
            LOG_DBG("Beacon received ??\n %d %d %d \n", buf[0], buf[1], buf[2]);
            LOG_INFO_LLADDR(&from);
            LOG_DBG("\n");
        }
        else if( frame ==1){
            LOG_DBG("Association request received (NOT ASSOCIATING )\n");
            linkaddr_copy(&buffer_addr, &from );
            process_poll(&association_process);     
        }
        else if( frame ==2){
            LOG_DBG("Association response received (!?)\n");
        }
        else if( frame ==3){
            LOG_DBG("Poll request received (!?) \n");
        }
        else if( frame ==4){
            LOG_DBG("Sensor Data received\n");
            //process_poll(&parser_process);
               
            poll_response_received = 1; //we received a poll response
            //switch(buf[0] & 31){
            //LOG_DBG("header == %d\n" , buf[0]&0b00011111);
            //LOG_DBG("MSG RX: %d %d %d %d %d %d %d %d\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]); DEPRECATED
            switch(frame2) //last 5 bits of the first byte is for NodeID?
            {
                case NODEID_MGAS1:  //1
                case NODEID_DHT22_1:    //2
                case NODEID_MGAS2:
                case NODEID_DHT22_2:    //4
                
                            
                if(cb_len == sizeof(aggregation_msg)){  //IF AGGREGATION MESSSAGE RECEIVED 
                    
                    
                    memcpy(&ag_msg, buf, sizeof(aggregation_msg));

                    uint8_t nodeid_DB = frame2;

                    childs_polled kids_polled;
                    kids_polled = get_childs_ID(nodeid_DB, kids_polled);               
                    //JSON parser: 
                    printf(" { \"Nodeid_DB\": %d, \"nodeid_ch1\": %d, \"nodeid2_ch2\": %d, \"T1\": %d.%d, \"H1\": %d.%d, \"Pw_tx1\": %d, \"n_beacons1\": %d, \"n_transmissions1\": %d, \"permil_radio_on1\": %d,\"permil_tx1\": %d, \"permil_rx1\": %d, \"T2\": %d.%d, \"H2\": %d.%d, \"Pw_tx2\": %d, \"n_beacons2\": %d,  \"n_transmissions2\": %d, \"permil_radio_on2\": %d, \"permil_tx2\": %d, \"permil_rx2\": %d}\n",frame2, get_nodeid(kids_polled.nodeid1), get_nodeid(kids_polled.nodeid2),ag_msg.p1.temperature/10, ag_msg.p1.temperature%10, ag_msg.p1.humidity/10 ,ag_msg.p1.humidity%10, ag_msg.p1.power_tx, ag_msg.p1.n_beacons_received, ag_msg.p1.n_transmissions, ag_msg.p1.permil_radio_on, ag_msg.p1.permil_tx, ag_msg.p1.permil_rx, ag_msg.p2.temperature/10, ag_msg.p2.temperature%10, ag_msg.p2.humidity/10, ag_msg.p2.humidity%10, ag_msg.p2.power_tx, ag_msg.p2.n_beacons_received, ag_msg.p2.n_transmissions, ag_msg.p2.permil_radio_on, ag_msg.p2.permil_tx, ag_msg.p2.permil_rx);
                    memset(&ag_msg, 0, sizeof(aggregation_msg));
                    memset(&buf , 0, sizeof(aggregation_msg));
                    memset(&global_buffer, 0, sizeof(global_buffer));    
                }
                else{
                    LOG_DBG("ERROR: wrong size of aggregation message\n");
                }      
                break;
                
            case NODEID_SB_1:
            case NODEID_SB_2:
            case NODEID_SB_3:
            case NODEID_SB_4:
                
                
                if(cb_len == sizeof(hare_stats_t)){
                
                    
                    memcpy(&hare_stats_msg, buf, sizeof(hare_stats_t));
                    uint8_t h_nodeid = (hare_stats_msg.header & 0b00011111);
                    printf(" { \"Nodeid_SB\": %d, \"Temp\": %d.%d, \"Hum\": %d.%d , \"Pw_tx1\": %d, \"n_beacons\": %d, \"n_transmissions\": %d, \"permil_radio_on\": %d, \"permil_tx\": %d, \"permil_rx\": %d}\n" ,h_nodeid, hare_stats_msg.temperature/10, hare_stats_msg.temperature%10,  hare_stats_msg.humidity/10,   hare_stats_msg.humidity%10,  hare_stats_msg.power_tx,  hare_stats_msg.n_beacons_received, hare_stats_msg.n_transmissions, hare_stats_msg.permil_radio_on, hare_stats_msg.permil_tx, hare_stats_msg.permil_rx);             
                    
                    
                   
                    memset(&hare_stats_msg, 0, sizeof(hare_stats_t));
                    memset(&buf , 0, sizeof(hare_stats_t));
                    memset(&global_buffer, 0, sizeof(global_buffer));
                }
                else{
                    LOG_DBG("ERROR: wrong size of hare stats message\n");
                }
            
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
      LOG_DBG("Data received over UART: %s\n", rxdata);    //Mostrem el missatge rebut.
      leds_toggle(LEDS_RED); 

      char buffer_header[30];
      strcpy(buffer_header, rxdata);
      LOG_DBG("buffer header: %s\n", buffer_header);
      char *header; 
      header = strtok(buffer_header, ",");
      LOG_DBG("header: %s\n", header);



    if (strcmp(header, "BM") == 0) { //If we received a new BM from the GW
        
        
        char *token = strtok(NULL, ",");
        uint8_t buf_bitmask = 0;

        if(token != NULL) { //only once
            
            LOG_DBG("token: %s\n", token);
            buf_bitmask = atoi(token);

            LOG_DBG("bitmask value (serial): %d\n", buf_bitmask);
            //bitmask = buf_bitmask; //store received bitmask for next cycle
            memcpy(&bitmask, &buf_bitmask, sizeof(uint8_t));

        }   
        else{
            LOG_ERR("Error parsing bitmask\n");
        }
    }    
   }
    
  }

  PROCESS_END();
}
