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
#define COOJA 0

#if DEBUG

#define PRINTF(...) printf(__VA_ARGS__)

#else //

#define PRINTF(...)

#endif //
    
void print_uart(unsigned char c)
{
	buf_in[counter_uart] = c;
	counter_uart++;
    //uart_write_byte(0,c++);
	//printf("%c",c);

    if (c == '\n')
    {
		PRINTF("[SERIAL]SERIAL DATA IN  --> %s\n", (char *)buf_in);
		counter_uart = 0;
		serial_in();
		flush_uart_in();
    }
}


unsigned int
uart1_send_bytes(const unsigned char *s, unsigned int len)
{
  unsigned int i = 0;

  while(s && *s != 0) {
    if(i >= len) {
      break;
    }
    uart_write_byte(1, *s++);
    i++;
  }
  return i;


/*---------------------------------------------------------------------------*/


static uint8_t buf_in[128]; 

/*---------------------------------------------------------------------------*/

PROCESS(poll_process, "STA process");

AUTOSTART_PROCESSES(&poll_process);
/*---------------------------------------------------------------------------*/

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{   
  memcpy(buf_in, *data, len);
  printf("Received: %s\n", buf_in);
  //uint8_t *buf = (uint8_t *)malloc(len);
  //packetbuf_dataptr(buf, data, len); //TEST THIS

  if()
  
}


/*---------------------------------------------------------------------------*/


PROCESS_THREAD(poll_process,ev,data)
{   

    nullnet_set_input_callback(input_callback);

    PROCESS_BEGIN();

    while(1)
    {



    }

    PROCESS_END();

}



