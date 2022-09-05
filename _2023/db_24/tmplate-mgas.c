

/*---------------------------------------------------------------------------*/

#include "contiki.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "sys/log.h"

#include "dev/i2c.h"
#include "dev/multigas.h"

#include "dev/dht22.h"
#include "dev/pm10-sensor.h"



#include "dev/adc-sensors.h"
#include "lib/sensors.h"

////////////IPV6 definitions

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO


#include "sys/log.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
#define ADC_PIN1        5
#define ADC_PIN2	      2

static struct simple_udp_connection udp_conn;


//#define SENSOR_READ_INTERVAL (10*CLOCK_SECOND)


/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer timer;
  
  
  static char str3[320];
  char pm10s[20];
  char louds[20];
  uip_ipaddr_t dest_ipaddr;
  uint8_t is_reachable, root_found;
  static uint16_t pm10_value;
  uint16_t loudness;


  PROCESS_BEGIN();
  

  etimer_set(&timer, CLOCK_SECOND * 20);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));  
  
  /* Setup a periodic timer that expires after 10 seconds. Give some time to join DODAG */
  
//Setup Multichannel gas sensor  
  i2c_master_enable();
  mgsbegin(0x04);
  mgspowerOn();
 
//Setup DHT22
//SENSORS_ACTIVATE(dht22);

//Setup PM10 GP2Y10
 
pm10.configure(SENSORS_ACTIVE, ADC_PIN2);
//Setup Noise sensor
adc_sensors.configure(ANALOG_GROVE_LOUDNESS, ADC_PIN1);

simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
   
while(1) {


  etimer_set(&timer, CLOCK_SECOND * 120);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    
  //Sample Multichannel gas sensor
    i2c_master_enable();
      
    float c__; 			
    char co_[20];
    char no2_[20];
    char fractional_part[5];

    
    c__ = measure_CO();
    /*printf("The concentration of CO is ");
    floatsensor(c__);
    printf(" ppm\n");
    */

    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(co_, sizeof(co_),"\"co\": %d.%s", (int)c__, &fractional_part[1]);
    
    c__ = measure_NO2();
    /*printf("The concentration of NO2 is ");
    floatsensor(c__);
    printf(" ppm\n");
*/
    sprintf(fractional_part, "%d", (int)((c__ - (int)c__ + 1.0) * 1000));
    snprintf(no2_, sizeof(no2_),"\"no2\": %d.%s", (int)c__, &fractional_part[1]);
    
  //Sample Loudness sensor   
  loudness = adc_sensors.value(ANALOG_GROVE_LOUDNESS);
    
  if(loudness != ADC_WRAPPER_ERROR) {
      printf("%u\n", loudness);
      snprintf(louds, sizeof(louds), "\"Noise\": %u", loudness);
  } else {
      printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
      PROCESS_EXIT();
  }
//Sample PM10

  pm10_value = pm10.value(1);

  if(pm10_value != ADC_WRAPPER_ERROR) {
    printf("PM10 value = %u ppm\n", pm10_value);
    snprintf(pm10s, sizeof(pm10s), "\"PM10\": %u", pm10_value);
} else {
    printf("Error, enable the DEBUG flag in adc-wrapper.c for info\n");
    PROCESS_EXIT();
}

//ipv6 send
    is_reachable = NETSTACK_ROUTING.node_is_reachable();
    root_found = NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr);

    if(is_reachable && root_found) {
      LOG_INFO("Sending to ");/* Send to DAG root */
      LOG_INFO("Sending request to ");
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      //snprintf(str3,sizeof(str3), "{%s,%s,%s,%s,%s,%s}", co_, no2_, temp, hums, pm10s, louds );  //prepare JSON string
	    snprintf(str3,sizeof(str3), "{\"nodeID\": \"1\",%s,%s,%s,%s}", co_, no2_, pm10s, louds );  //prepare JSON string
      
      simple_udp_sendto(&udp_conn, str3, sizeof(str3), &dest_ipaddr); 
      }    
  else {
  	    LOG_INFO("Not reachable yet\n");
    }
}
  PROCESS_END();
}




