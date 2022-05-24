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

#define ROUTENUMBER 8
static linkaddr_t addr_stas[ROUTENUMBER]; //store sta's addresses in here, for routing and sending
static linkaddr_t buffer_addr; 

const linkaddr_t addr_empty = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}; //placeholder address

const uint8_t beacon_[3] = {0x05, 0x06, 0x07}; 
const uint8_t ass_req[3] = {0x08, 0x09, 0x0A};
const uint8_t ass_response[3] = {0x0B, 0x0C, 0x0D};

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
PROCESS(associator_process, "assoc xd");


AUTOSTART_PROCESSES(&nullnet_example_process, &associator_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  printf("Callback received\n");
 
  
  //int pos = 0;

  linkaddr_copy	(&buffer_addr, src ); //copy the address of the packet received into the buffer


  
        
  
  /*if(!oldaddr){
    printf("New address: ");
    LOG_INFO_LLADDR(src);
    printf("\n");
    addr_stas[pos] = *src;
  }
  */
  if((memcmp(data, ass_req, 3) == 0))
  {
    printf("Assoc. request received");
    
    process_poll(&associator_process);
    //nullnet_set_input_callback(NULL);

  }
 } //callback

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
  const uint8_t nbuf[3] = {0x05, 0x06, 0x07};

  
 
  PROCESS_BEGIN();
 
  nullnet_set_input_callback(input_callback);
  nullnet_buf = (uint8_t *)nbuf;
  nullnet_len = sizeof(nbuf);

  while(1) {
    etimer_set(&periodic_timer, 15* CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    LOG_INFO("Sending BEACON\n");


    nullnet_buf = (uint8_t *)nbuf;
    nullnet_len = sizeof(nbuf);
    NETSTACK_NETWORK.output(NULL);

    printf("addresses: \n");
    int i_ad;
    for (i_ad= 0; i_ad<ROUTENUMBER; i_ad++){
      LOG_INFO_LLADDR(&addr_stas[i_ad]);
      LOG_INFO("\n");
    }
   
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(associator_process, ev,data)
{
PROCESS_BEGIN()
while(1)
{
  PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
  uint8_t i;
  uint8_t oldaddr = 0; 
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
          printf("Address");
          LOG_INFO_LLADDR(&buffer_addr);
          printf(" added at pos %d\n", i);
          break;
          //oldaddr = 1;     
        }
    }
}






  //nullnet_set_input_callback(NULL);
  LOG_INFO("Associating nowwwwww\n");
  uint8_t ibuf[3] = {0x0B, 0x0C, 0x0D};
  nullnet_buf = (uint8_t*)ibuf;
  nullnet_len = 3;
  NETSTACK_NETWORK.output(&buffer_addr);
  //nullnet_set_input_callback(input_callback);
  //LOG_INFO("Associating done\n");
}
  PROCESS_END();
}