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

typedef enum {HIGH, MID, LOW} tx_power_t;
const uint8_t beacon_[3] = {0x05, 0x06, 0x07}; 
const uint8_t ass_req[3] = {0x08, 0x09, 0x0A};
const uint8_t ass_response[3] = {0x0B, 0x0C, 0x0D};

/*static const struct { 
  uint8_t MIN_TX_POWER;
  uint8_t MID_TX_POWER;
  uint8_t MAX_TX_POWER;
} tx_power_levels = {0x46, 0x71, 0x7F}; // -11dB, 8dB, 14dB
*/
const uint8_t power_levels[3] = {0x46, 0x71, 0x7F}; // -11dB, 8dB, 14dB

static linkaddr_t coordinator_addr = {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}; //placeholder address
static bool is_associated = false;
//static uint8_t tx_power = 0x7F;

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_example_process, "NullNet broadcast example");
PROCESS(association_process, "assoc xd");
AUTOSTART_PROCESSES(&nullnet_example_process, &association_process);

/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{ 

  //printf("Callback received\n");
  uint8_t rxbuf[len];
  memcpy(rxbuf, data, len);
  memcpy(&coordinator_addr, src, sizeof(linkaddr_t));

  //printf("rxbuf:  %02x %02x %02x\n", rxbuf[0], rxbuf[1], rxbuf[2]);

 if ((memcmp(rxbuf, beacon_, 3) == 0) && !is_associated)
  {
  //if(rxbuf == beacon_)
  
    printf("Assoc. Beacon received\n");
    process_poll(&association_process);
  }

  if(memcmp(ass_response, rxbuf, 3) == 0)
  {
    printf("Assoc. response received\n");
    is_associated = true;
  }
} 
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_example_process, ev, data)
{
  static struct etimer periodic_timer;
 
  PROCESS_BEGIN();
  
  
  nullnet_set_input_callback(input_callback);
  while(1) {
    etimer_set(&periodic_timer, 50*CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    printf("tick\t");
    if(is_associated )
    {
      printf("I'm associated to ");
      LOG_INFO_LLADDR(&coordinator_addr);
      LOG_INFO("\n");

    }
  }

  PROCESS_END();
}

PROCESS_THREAD(association_process, ev, data)
{ 

  static struct etimer asotimer; 
  PROCESS_BEGIN();
  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_POLL);
    printf("Association process started, sending to:  ");
    LOG_INFO_LLADDR(&coordinator_addr);
    LOG_INFO("\n");
    static uint8_t ix = 0;
    
    /*---------------------------------------------------------------------------*/
    
    while(!is_associated)
    {
      //augment TX POWER and send again. If no response, "poison"
      

      NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, power_levels[ix]);
      printf("tx power: %d\n", power_levels[ix]);

      nullnet_buf = (uint8_t *)ass_req;
      nullnet_len = sizeof(ass_req);
      NETSTACK_NETWORK.output(&coordinator_addr);

      ix++;
       if(ix >= 2)
      {
        LOG_INFO("Association process failed\n");

        //poison!!
        break; 
      }
      etimer_set(&asotimer, 5* CLOCK_SECOND);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&asotimer));

     
    } 
  } //while(1)
    
    PROCESS_END();

}
