#include "contiki.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <string.h>

#include "dev/leds.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "sys/log.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"

#include "net/packetbuf.h"

#define NODEID1 1
#define NODEID2 2
#define NODEID3 3
#define NODEID4 4
#define NODEID5 5
#define NODEID6 6
#define NODEID7 7
#define NODEID8 8


#define ROUTENUMBER 8 //number of routes

#define WINDOW_SIZE (5*CLOCK_SECOND)


#define LOG_MODULE "DB_24"
#define LOG_LEVEL LOG_LEVEL_DBG
#define COOJA 0 ///SET TO 0 FOR REAL TESTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1





static uint8_t global_buffer[30]; 
static volatile bool flag_rx_window  = false; 

static linkaddr_t buffer_addr; 
static linkaddr_t addr_stas[ROUTENUMBER]; //store sta's addresses in here, for routing and sending

PROCESS(dualband_24, "dualband 24");
PROCESS(sender, "sender");
PROCESS(radio_receiver, "receiver");
PROCESS(window_process, "RX window process");
PROCESS(association_process, "assoc process");
PROCESS(cooja_beacons, "cooja beacons");
AUTOSTART_PROCESSES(&dualband_24, &sender, &radio_receiver,&window_process, &association_process, &cooja_beacons);
/*---------------------------------------------------------------------------*/

static linkaddr_t from; 
const linkaddr_t addr_empty = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; //placeholder address
static uint16_t cb_len;

typedef struct data_t{
  int16_t temperature, humidity;
  uint16_t noise;
  //add more sensor data here
} data_t;
struct data_t datas; 

#pragma pack(push,1) //REMOVE PADDING
static struct hare_stats_t{
    uint8_t header; //header includes message type and node id
    
    int16_t temperature, humidity;   
    uint8_t power_tx;
    uint16_t n_beacons_received;
    uint16_t n_transmissions; 
    uint16_t permil_radio_on; // ‰ gotten through energest
    uint16_t permil_tx;
    uint16_t permil_rx;
    } hare_stats;
#pragma pack(pop)

#pragma pack(push,1)
static struct aggregation_stats_t{
  struct hare_stats_t p1;
  struct hare_stats_t p2;
 } aggregation_msg;
#pragma pack(pop)

uint8_t buffer_aggregation[sizeof(aggregation_msg)]; //buffer for sending aggregated data

char buf_in[100];
uint8_t beacon[3];
const char delimitador[2] = ",";
long int sortida[3];
char* endPtr;

static struct aggregator_flags_t
{
  bool f_m1;
  bool f_m2; 
  //TODO: add more flags if we need more messages included
}aggregator_flags;




//static bool flag = 0;

uint16_t counter_uart = 0;

unsigned int uart1_send_bytes(const unsigned char *s, unsigned int len){  
  unsigned int i = 0;

  while(s && *s != 0){
    if(i >= len) {
      break;
    }
    uart_write_byte(1, *s++);
    i++;
  }
  return i;
}

void serial_in(){ // Implementa la lògica a la cadena de caràcters que ha entrat al UART. node_db_24
    printf("yoyyoyoyoyo");
    char buffer_header[20];
    strcpy(buffer_header, buf_in);
    char *header; 
    header = strtok(buffer_header, delimitador); //agafem només "B0", "P0", etc

    if (!strncmp(header, "BO", sizeof("B0")) == 0){ // B0 indicarà al db_24 que el missatge és beacon

      char *token = strtok(buf_in, delimitador);
      int i= 0; 
      
      while(token != NULL){
        token = strtok(NULL, delimitador);
        if(i == 0){
          beacon[0] = atoi(token);
        }
        else if(i == 1){
          beacon[1] = atoi(token);
        }
        else if(i == 2){
          beacon[2] = atoi(token);
        }
        i++;
        }

        process_poll(&sender);
    
       //flag = 1;  
  
      }

      else if(!strncmp(header, "P0", sizeof("P0")) == 0){ // P0 indicarà al db_24 que el missatge és un poll

        printf("received poll message\n");
        //char *token = strtok(buf_in, delimitador);
      
      /* ------------------------------------------------- */
      //SEND "DYNAMICALLY AGGREGATED" DATA TO DB_868 THROUGH UART! 
      //
      //
      }

     //printf("Beacon: B0, %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
}

int print_uart(unsigned char c){
	buf_in[counter_uart] = c;
	counter_uart++;

	if (c == '\n'){
		printf("SERIAL DATA IN --> %s", (char *)buf_in); // SERIAL DATA IN --> B0, 0, 0, 0
		counter_uart = 0;
		serial_in();
	}
	
	return 1;
}
//TORNADA


void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){

    
    //memcpy(dbgbuf, data, len);
    //LOG_DBG("Received %u bytes: %d %d %d %d %d\n", len, dbgbuf[0], dbgbuf[1], dbgbuf[2], dbgbuf[3], dbgbuf[4]);  
    
    from = *src;
    cb_len = len; //save the length of the received packet
    //packetbuf_copyto(&global_buffer); //copy the received packet to the buffer

    process_poll(&radio_receiver);


}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_24, ev, data){

  static struct etimer et;
  PROCESS_BEGIN();
  uart_set_input(1, print_uart);
  nullnet_set_input_callback(input_callback);
  

  while(1){  

    PROCESS_WAIT_EVENT_UNTIL( ev == PROCESS_EVENT_POLL);

    etimer_set(&et, CLOCK_SECOND * 5);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et)); //SOME delay needed for db_868 UART to be available
    LOG_INFO("sending through UART the aggregated msg\n"); //TODO: CHANGE SO IF WINDOW EXPIRES ALSO SEND THE RECEIVED DATA
    char sprinter[100];
    //sprintf for all elements in the buffer_aggregation
    sprintf(sprinter, "P0,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", buffer_aggregation[0], buffer_aggregation[1], buffer_aggregation[2], buffer_aggregation[3], buffer_aggregation[4], buffer_aggregation[5], buffer_aggregation[6], buffer_aggregation[7], buffer_aggregation[8], buffer_aggregation[9], buffer_aggregation[10], buffer_aggregation[11], buffer_aggregation[12], buffer_aggregation[13], buffer_aggregation[14], buffer_aggregation[15], buffer_aggregation[16], buffer_aggregation[17], buffer_aggregation[18], buffer_aggregation[19], buffer_aggregation[20], buffer_aggregation[21], buffer_aggregation[22], buffer_aggregation[23], buffer_aggregation[24], buffer_aggregation[25], buffer_aggregation[26], buffer_aggregation[27], buffer_aggregation[28], buffer_aggregation[29], buffer_aggregation[30], buffer_aggregation[31]);


    printf("%s", sprinter);

    uart1_send_bytes((unsigned char *)sprinter, strlen(sprinter));

    //clear both the buffer and hare_stats
    memset(buffer_aggregation, 0, sizeof(buffer_aggregation));
    memset(hare_stats, 0, sizeof(hare_stats));
    memset(aggregation_msg, 0, sizeof(aggregation_msg));


    /*------------------------------------------------------
    Here send the aggregated buffer to the db_868 through UART
    ------------------------------------------------*/

  } 
  PROCESS_END();
}


PROCESS_THREAD(radio_receiver, ev, data){
uint8_t* bytebuf; 
uint8_t header_rx_msg;
uint8_t frame_header; 

PROCESS_BEGIN();
PROCESS_YIELD();
while (1){

  PROCESS_WAIT_EVENT_UNTIL(PROCESS_EVENT_POLL);

  LOG_DBG("Receiving RADIO msg...\n");  


  
  bytebuf = packetbuf_dataptr();
  //printf("length: %d , \n", cb_len);

 
  frame_header = (bytebuf[0]&0b11100000) >>5;
  
  header_rx_msg = ( bytebuf[0] & 0b00011111);
  uint8_t len_little = (uint8_t)cb_len + 1; // +1 for the header //¿¿¿¿???
  
  printf("header: %d, header_rx_msg: %d, len_little: %d\n", frame_header, header_rx_msg, len_little);
    
  switch(frame_header){
    case 0: 
      LOG_DBG("RX: Beacon ???? \n");
      break;
    case 1:
      LOG_DBG("RX: Association request\n");

      linkaddr_copy(&buffer_addr, &from );
      process_poll(&association_process);     
      break;
    case 2: 
      LOG_DBG("RX: Association response\n");
      break;
    case 3: 
      LOG_DBG("RX: Poll ?????? \n");
      break;
    case 4: 
      LOG_DBG("RX: Sensor Data\n");

      if(cb_len ==sizeof(hare_stats))
              {
              memcpy(&hare_stats, bytebuf, cb_len);
              LOG_DBG("Received %u bytes: n_beacons: %d n_tx %d permil_radio %d permil_tx %d permil_rx %d\n", cb_len, hare_stats.n_beacons_received, hare_stats.n_transmissions, hare_stats.permil_radio_on, hare_stats.permil_tx, hare_stats.permil_rx);
                      
                /*TODO:
                -test 
                -aggregate to bigger struct
                -activate flag
            */
              }

      switch(header_rx_msg) {
        //let's assume all incoming traffic is DHT22: temp and humidity.
        //    len_little is size of the "submessage", will be useful later if buffer is to be allocated dynamically!

          case NODEID1:  //TO BE TESTED!!!!**
          case NODEID3:
          case NODEID5:
          case NODEID7:
    
            //memcpy(&ha, &hare_stats, sizeof(hare_stats));
            aggregation_msg.p1 = hare_stats;
            aggregator_flags.f_m1 = true;              
            break;
            
          case NODEID2:   //TO BE TESTED!!!!**
          case NODEID4:
          case NODEID6:
          case NODEID8: 
            
            aggregation_msg.p2 = hare_stats; 
            aggregator_flags.f_m2 = true;
            break; 
          
          default:
            LOG_DBG("NODEID: %d???\n", bytebuf[0]& 0b00011111); 
            break; 
        }
         LOG_DBG("Added %d bytes of data from node %d to buffer\n", cb_len, header_rx_msg);
    
        if(aggregator_flags.f_m1 || aggregator_flags.f_m2)
        {
          aggregator_flags.f_m1 = false;
          aggregator_flags.f_m2 = false; 
          
          printf("data inside struct 1 is: %d %d %d %d %d %d %d %d %d\n", aggregation_msg.p1.header,  aggregation_msg.p1.temperature,  aggregation_msg.p1.humidity,  aggregation_msg.p1.power_tx  , aggregation_msg.p1.n_beacons_received, aggregation_msg.p1.n_transmissions, aggregation_msg.p1.permil_radio_on, aggregation_msg.p1.permil_tx, aggregation_msg.p1.permil_rx);
          printf("data inside struct 2 is  %d %d %d %d %d %d %d %d %d\n", aggregation_msg.p2.header,  aggregation_msg.p2.temperature,  aggregation_msg.p2.humidity,  aggregation_msg.p2.power_tx,  aggregation_msg.p2.n_beacons_received, aggregation_msg.p2.n_transmissions, aggregation_msg.p2.permil_radio_on, aggregation_msg.p2.permil_tx, aggregation_msg.p2.permil_rx);

          memcpy(&buffer_aggregation[0], &aggregation_msg, sizeof(aggregation_msg));
        /*
          printf("Buffer of aggregated message is: ");
          for (int i = 0; i < sizeof(buffer_aggregation); i++)
          {
          printf("%d ", buffer_aggregation[i]);
          }
          printf("\n");
        */
          
          process_poll(&dualband_24);
        }
        break;
  
    default: 
      LOG_DBG("RX: Unknown\n");
      break;
    }

}

  //if(flag_rx_window == false){
  //process_poll( &window_process);
  //THIS WILL OPEN THE WINDOW AS LONG AS A MESSAGE IS RECEIVED, WATCH OUT!!!!!!!!!!!!!!!!
  
  //window process will be started when the first packet is received
  //then it will wait for a window of time (WINDOW_SIZE) and it will set the flag back to 0
  

  //else if(flag_rx_window ==true)
  //{ 
    //while

  //TODO: AGGREGATE MESSAGES INTO A DYNAMIC SIZE MESSAGE
PROCESS_END();

}

PROCESS_THREAD(sender, ev,data){
  PROCESS_BEGIN();
  PROCESS_YIELD();
  while(1){

    PROCESS_WAIT_EVENT_UNTIL(PROCESS_EVENT_POLL);

    nullnet_buf = (uint8_t* )beacon;
    nullnet_len = sizeof(beacon);

    printf("broadcasting BEACON: %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
    NETSTACK_NETWORK.output(NULL);
 }
  PROCESS_END();
}


PROCESS_THREAD(window_process, ev, data){

  static struct etimer window_timer;

  PROCESS_BEGIN();

  while(1)
  {

    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    LOG_DBG("setting window of rx timer for %d seconds", WINDOW_SIZE/CLOCK_SECOND);
    flag_rx_window = true; 
    etimer_set(&window_timer, WINDOW_SIZE);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&window_timer));
    flag_rx_window = false;
    LOG_DBG("window of rx timer expired");
  }


  PROCESS_END();

}

PROCESS_THREAD(association_process, ev, data)
{
  uint8_t buf_assoc[2];
  static uint8_t id_rx;
  uint8_t i;
  uint8_t oldaddr = 0; 
  
  PROCESS_BEGIN();
  
  while(1){

      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

      LOG_DBG("rx messg from nodeid: %d\n", global_buffer[1]);
      packetbuf_copyto(&global_buffer);
      
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

PROCESS_THREAD(cooja_beacons, ev,data){
#if COOJA
static struct etimer cooja_timer;

  PROCESS_BEGIN();
  
    etimer_set(&cooja_timer, 10*CLOCK_SECOND);

    while(1){

      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&cooja_timer));
      
      beacon[0] = 0x00;
      beacon[1] = 0xff;
      beacon[2] = 0x00;
    
      nullnet_buf = (uint8_t*)beacon;
      nullnet_len = sizeof(beacon);

      NETSTACK_NETWORK.output(NULL);
      printf("broadcasting BEACON: %d, %d, %d\n", beacon[0], beacon[1], beacon[2]);
      etimer_set(&cooja_timer, 60*CLOCK_SECOND);
    }

  
  #else
  PROCESS_BEGIN();
  #endif
  PROCESS_END();
}

