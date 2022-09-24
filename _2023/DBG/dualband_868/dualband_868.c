#include "contiki.h"
#include "sys/etimer.h"
#include "dev/uart.h"
#include "dev/serial-line.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "dev/leds.h"
#include "net/packetbuf.h"
#include "sys/log.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "random.h"
#include "dev/button-hal.h"

/*---------------------------------------------------------------------------*/



#define NODEID1 1  // non-adaptive, test
#define NODEID2 2 //DHT22
#define NODEID3 4 // PM10 
#define NODEID4 8
#define NODEID5 16
#define NODEID6 32
#define NODEID7 64
#define NODEID8 128

#define NODEID_DB NODEID1


#define T_MDB  (10 * CLOCK_SECOND)
#define T_SLOT  (1.5 * CLOCK_SECOND)
#define T_GUARD  (0.5 * CLOCK_SECOND)
#define T_BEACON (60 * CLOCK_SECOND)

#define LOG_MODULE "DB_868"
#define LOG_LEVEL LOG_LEVEL_DBG

struct childs_polled_t {
    uint8_t nodeid1;
    uint8_t nodeid2;
};
typedef struct childs_polled_t childs_polled;


uint8_t is_polled(uint8_t bitmask, uint8_t id)  //prints which nodes the beacon is polling, and if it's own NODEID return position of poll
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

#pragma pack(push,1) //REMOVE PADDING
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

static uint8_t buffer_aggregation[sizeof(aggregation_msg)]; //buffer for sending aggregated data

static uint8_t death_beacons, death_uart; 

uint16_t counter_uart;
static char buf_in[100];
//static uint8_t buffer[7];
static uint8_t global_ag_buf[sizeof(buffer_aggregation)];
const char delimitador[3] = " ";
long int sortida[3];
char* endPtr;
static uint8_t nodeid_db;
static childs_polled kids; //struct to store the childs of the node
volatile static uint8_t bitmask; 

static clock_time_t time_of_beacon_rx; // time of beacon reception


static bool beaconrx_f = 0; // flag que indica si el node ha rebut cap beacon


static struct ctimer ct; 

const uint8_t power_levels[3] = {0x46, 0x71, 0x7F}; // 0dB, 8dB, 14dB

volatile static uint16_t len_msg;
static volatile  bool is_associated = 0; // flag to check if the node is associated
static volatile bool uart_rx_flag = false; // flag to check if the node has received a messagesss
static linkaddr_t from, gw_addr; 

//static bool flag = 0;  

//function to print the number of the NODEID (f.e. NODEID8 would return 8)


/*---------------------------------------------------------------------------*/
PROCESS(dualband_868, "dualband 868");
PROCESS(radio_rx_process, "rx process");
PROCESS(associator_process, "assoc process");
PROCESS(poll_process, "poll process");
PROCESS(button_hal_example, "Button HAL Example");
PROCESS(reboot_prc,"reboot on command");
AUTOSTART_PROCESSES(&dualband_868, &radio_rx_process, &poll_process, &associator_process, &button_hal_example);
/*---------------------------------------------------------------------------*/

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

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest){
  
  
  
  linkaddr_copy(&from, src); //PREMATURO, comprobar!
  len_msg = len; 
  process_poll(&radio_rx_process);
  
  
  
}

void serial_in(){
    char delimitador[] = ",";
    char copy_buffer[100]; 
    strcpy(copy_buffer, buf_in);

    LOG_DBG("incoming string: %s\n", copy_buffer);

    char *token = strtok(copy_buffer, delimitador);
    //uint8_t buffer_aggregated[12];

    LOG_DBG("token: %s\n", token);
    if (strncmp(token, "P0", sizeof("P0")) == 0){
      int i=0;
      while(token != NULL) {
          // printf("token: %s \n", token);
          token = strtok(NULL, delimitador);
          buffer_aggregation[i] = atoi(token);
          //LOG_DBG("buffer_aggregated[%d]: %d\n ", i, buffer_aggregation[i]);
          i++;
          if(i==32){break;}

      }//while
      memcpy(global_ag_buf, buffer_aggregation, sizeof(buffer_aggregation));
      uart_rx_flag = true; 
    } //if
    else{
      LOG_DBG("unrecognised header\n");
    }
    //memcpy(global_ag_buf,)


  
}


int print_uart(unsigned char c){

	buf_in[counter_uart] = c;
	counter_uart++;

	if (c == '\n'){
		//printf("SERIAL DATA IN --> %s\n", (char *)buf_in);
		counter_uart = 0;
		serial_in();
	}
	
	return 1;
}



void
ctimer_callback(void *ptr)
{

  if(!beaconrx_f){

    death_beacons +=1;
  }
  else{death_beacons = 0;}
  if(!uart_rx_flag)
  {
    death_uart +=1;
  }
  else{death_uart = 0;}

  if(death_beacons >=3 || death_uart>=10)
  {
      printf("Reboot by fail to connect: cause %d %d", death_beacons,death_uart );
      process_start(&reboot_prc, "reboot");

  }
   beaconrx_f = 0;
  //NETSTACK_RADIO.on();
  
  LOG_DBG("CTimer callback called, turning radio ON\n");
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(dualband_868, ev, data){
 
  PROCESS_BEGIN();
  nullnet_set_input_callback(input_callback);
  nodeid_db = get_nodeid(NODEID_DB);
  kids = get_childs_ID(NODEID_DB, kids);
  printf("Nodeid: %d\n", nodeid_db);
  printf("Childs: %d %d\n", get_nodeid(kids.nodeid1), get_nodeid(kids.nodeid2));
  uart_set_input(1, print_uart);
  while(1){
        
      
      PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL); //Fins que no arribi un missatge UART, espera
/*
      nullnet_buf = (uint8_t *) &global_ag_buf;
      nullnet_len = sizeof(global_ag_buf);

      LOG_DBG("Sending aggregated data: %d %d %d %d %d %d %d %d %d %d %d %d\n", global_ag_buf[0], global_ag_buf[1], global_ag_buf[2], global_ag_buf[3], global_ag_buf[4], global_ag_buf[5], global_ag_buf[6], global_ag_buf[7], global_ag_buf[8], global_ag_buf[9], global_ag_buf[10], global_ag_buf[11]);
      NETSTACK_NETWORK.output(NULL);*/
      //printf("Data sensors surt: %d %02d.%02d %02d.%02d %u\n", buffer[0], datas.temperature / 10, datas.temperature%10, datas.humidity / 10, datas.humidity % 10, datas.noise);

      }
  PROCESS_END();
}


PROCESS_THREAD(radio_rx_process, ev, data){

static struct etimer beacon_no_timer;
volatile static uint8_t* datapoint;
PROCESS_BEGIN();

while(1){

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL); //Fins que no arribi un missatge Ràdio, espera
  datapoint = packetbuf_dataptr();


  uint8_t frame_header = (datapoint[0] & 0b11100000) >> 5; 
  
  
  if(frame_header == 0){
    //printf("beacon\n");
    
    linkaddr_copy(&gw_addr, &from);
    bitmask = datapoint[1];
                
    uint8_t Beacon_no = datapoint[0] & 0b00011111;

    if(beaconrx_f == 0){

    
     
      beaconrx_f = 1;
      ctimer_set(&ct, T_BEACON - 3*CLOCK_SECOND, ctimer_callback, NULL);
      if(Beacon_no == 0){
        
        
        etimer_set(&beacon_no_timer, CLOCK_SECOND);
        LOG_DBG("Beacon 0 received\n");

        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&beacon_no_timer));
        process_poll(&associator_process);
      }
      else if(Beacon_no == 1)
      {   
       
        LOG_DBG("Beacon 1 received\n");
        etimer_set(&beacon_no_timer, CLOCK_SECOND/2 );
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&beacon_no_timer));
        process_poll(&associator_process);
      }
      else if(Beacon_no ==2)
      {   
        LOG_DBG("Beacon 2 received\n");
        process_poll(&associator_process); 
      }
      // TODO: find a METHOD to wait for the 1.5 seconds after first beacon, or time depending on the message received first    
      else{
          
          LOG_INFO("ignoring beacon\n");
      }

     

      //printf("Beacon received: %d %d %d\n", datapoint[0], datapoint[1], datapoint[2]);
      
      char string[20];

      sprintf(string, "B0, %d, %d, %d\n", datapoint[0], datapoint[1], datapoint[2]);
      
      uart1_send_bytes((uint8_t *)string, sizeof(string) - 1);
      printf("beacon sent UART:\t");
      printf("%s", string);
      } //if_beacon_flag
      
  } //if_frame_header==0
  else if(frame_header == 1){
    LOG_DBG("association request received ??\n");

  }

  else if(frame_header == 2){
    LOG_DBG("association response received\n");
    
    if(linkaddr_cmp(&from, &gw_addr)) { 

          LOG_INFO("Not associated, associating now\n");
          //PROCESS_CONTEXT_BEGIN(&associator_process);
          is_associated = true; 
          //PROCESS_CONTEXT_END(&associator_process);
        }
        else{
            LOG_DBG("error, different adresses");
        }
    
  }

  else if(frame_header == 3){

    LOG_DBG("Poll message \n");
    if(is_associated){
      process_poll(&poll_process);
    }
    else{
      printf("Not associated, ignoring poll\n");

      /*
      REASSOCIATION COULD BE DONE HERE
      */



    }

  }

  else{
    LOG_DBG("unknown frame header\n");
  }
}
PROCESS_END();

}


PROCESS_THREAD(associator_process, ev, data){


  //static struct etimer next_beacon_etimer;
  static clock_time_t time_to_wait; 
  static uint8_t buf[2];
  static struct etimer asotimer; 
  static struct etimer radiotimer; 
  static bool amipolled_f = 0;
  static uint8_t i_pwr;
  PROCESS_BEGIN();

  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL); //Espera fins que arribi un missatge d'associació
    
   
    
    //etimer_set(&next_beacon_etimer, (T_BEACON - 3*CLOCK_SECOND));

    
    
    time_of_beacon_rx = RTIMER_NOW();

    time_to_wait = (T_MDB  + ((nodeid_db -1) * (T_SLOT + T_GUARD)))- T_GUARD;

    printf("time to poll is  %lu\n", time_to_wait/CLOCK_SECOND);
    etimer_set(&radiotimer, time_to_wait);


    if(!is_associated){
      LOG_INFO("associating\n");
      i_pwr= 0;            
         
      buf[0] = 0b00100000; //association request
      buf[1] = nodeid_db; 

      nullnet_buf = (uint8_t *)buf;
      nullnet_len = sizeof(buf);

      for (i_pwr = 0; i_pwr < 3; i_pwr++){
               

        etimer_set(&asotimer, 2* CLOCK_SECOND + (random_rand() % (CLOCK_SECOND)));   //add some jitter/randomness for the transmission   
        
        NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, power_levels[i_pwr]);
        
        
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&asotimer));
        
        if(is_associated == 1 ){
       
        break;}
                                   
        

        printf("Sending assoc. Request, tx power: %02x\n", power_levels[i_pwr]);
     
        //LOG_INFO_LLADDR(&gw_addr);
        NETSTACK_NETWORK.output(&gw_addr);  

            
      } 
     
      if(!is_associated) 
      {
          printf("I'm STILL not associated!!\n");

          /*

          TODO: Try to associate again here


          */
      }

      else //if is associated
        {
            printf("I'm already associated\n");
            printf("bitmask: %02x, nodeid_db: %d\n", bitmask, nodeid_db);
        }
      } //if !is_associated


      /*----------------------------------------------------------------------------------------*/
      //SECOND PART, WAITING UNTIL POLL TIME

      static childs_polled children;
      
      children = are_childs_polled(kids,bitmask);
      LOG_DBG("is_polled: child1 %d child2 %d\n", children.nodeid1, children.nodeid2);
      amipolled_f = children.nodeid1 || children.nodeid2;

      if( amipolled_f == 1){
          printf("I'm transmitting in the %dth slot\n", nodeid_db);
          
          
          //NETSTACK_RADIO.off();
          
          //etimer_set( &radiotimer, time_until_poll);
          PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));
          
          NETSTACK_RADIO.on();
          printf("radio back on\n");
      }
      else if (amipolled_f == 0) { //if not polled, just wait for the next beacon
          printf("Radio off until the next beacon\n");
          //NETSTACK_RADIO.off();
          etimer_set( &radiotimer, T_BEACON - 2*CLOCK_SECOND);
          PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&radiotimer));

          //NETSTACK_RADIO.on();
          printf("radio back on, beacon in ~2s\n");
      }   
      amipolled_f = 0; 

      
      
  }
  PROCESS_END();
}

PROCESS_THREAD(poll_process,ev,data)
{
  static uint8_t *buffer_poll;
  //static clock_time_t time_after_poll; 
 // static struct etimer next_beacon_etimer;


  PROCESS_BEGIN();
  

  /*
  //Routine to be executed when we get a poll for the sensors: 

  //IF associated, data UART in is ok, send to gateway in the correct slot. 

  */
 while(1){
  

  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);


  buffer_poll = packetbuf_dataptr();

  //time_after_poll = RTIMER_NOW();
  
  if(buffer_poll[1] == nodeid_db) //if the poll is for me
  {



    if(uart_rx_flag == true){

      global_ag_buf[0] = 0b10000000 | nodeid_db;
      nullnet_buf = (uint8_t *) &global_ag_buf;
      nullnet_len = sizeof(global_ag_buf);

      LOG_DBG("Sending aggregated data: %d %d %d %d %d %d %d %d %d %d %d %d etc etc etc\n", global_ag_buf[0], global_ag_buf[1], global_ag_buf[2], global_ag_buf[3], global_ag_buf[4], global_ag_buf[5], global_ag_buf[6], global_ag_buf[7], global_ag_buf[8], global_ag_buf[9], global_ag_buf[10], global_ag_buf[11]);
      NETSTACK_NETWORK.output(NULL);

      uart_rx_flag = false;
    }
    else{
      LOG_DBG("Didn't receive a UART packet!!!\n");
    }
    
    //if(data_uart_in_ok())
    //{
      //send_data_to_gateway();
  //}




    printf("finished sending data\n");
    


    //NETSTACK_RADIO.off();
    //RTIMER_BUSYWAIT(5);
    //LOG_DBG("Radio off\n");
  }
  else{
    printf("not my poll, %d\n", buffer_poll[1]);
  }

 }
   


  PROCESS_END();
}




/*---------------------------------------------------------------------------*/
PROCESS_THREAD(button_hal_example, ev, data)
{
  button_hal_button_t *btn;
  static uint8_t bool_f;
   
  PROCESS_BEGIN();

  btn = button_hal_get_by_index(0);


  if(btn) {
    printf("%s on pin %u with ID=0, Logic=%s, Pull=%s\n",
           BUTTON_HAL_GET_DESCRIPTION(btn), btn->pin,
           btn->negative_logic ? "Negative" : "Positive",
           btn->pull == GPIO_HAL_PIN_CFG_PULL_UP ? "Pull Up" : "Pull Down");
  }
  while(1) {
    PROCESS_YIELD();
    if(ev == button_hal_press_event) {
      btn = (button_hal_button_t *)data;
      printf("Press event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));

      if(btn == button_hal_get_by_id(BUTTON_HAL_ID_BUTTON_ZERO)) {
        //printf("This was button 0, on pin %u\n", btn->pin);
      }
    } else if(ev == button_hal_release_event) {
      btn = (button_hal_button_t *)data;
      printf("Release event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));
      if(bool_f==1)
      { printf("INITIATING REBOOT");
        process_start(&reboot_prc,"AAA");}

    } else if(ev == button_hal_periodic_event) {
      btn = (button_hal_button_t *)data;
      //printf("Periodic event, %u seconds (%s)\n", btn->press_duration_seconds,
            // BUTTON_HAL_GET_DESCRIPTION(btn));

      if(btn->press_duration_seconds > 3) {
        printf("%s pressed for more than 3 secs.\n",
               BUTTON_HAL_GET_DESCRIPTION(btn));
        bool_f = 1; 
      }
    }
  }

  PROCESS_END();
}


PROCESS_THREAD(reboot_prc, ev, data)
{
  struct etimer rst_3;
  char str_rbt[20];
  PROCESS_BEGIN();
  while(1) {
    
    //PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);

    sprintf(str_rbt,"RST,\n");
    uart1_send_bytes((uint8_t *)str_rbt, sizeof(str_rbt) - 1);
    
    printf("%s", str_rbt);
    etimer_set(&rst_3, 3*CLOCK_SECOND);

    PROCESS_WAIT_UNTIL(etimer_expired(&rst_3));
    watchdog_reboot();
  }

  PROCESS_END();
}




