#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include "random.h"
#include "dev/radio.h"
#include <stdlib.h>
#include "net/packetbuf.h"

#include "dev/uart.h"
#include "dev/serial-line.h"

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
    



/*---------------------------------------------------------------------------*/


static uint8_t buf_in[128]; 



void flush_uart_in ()
{
	memset(buf_in, 0, sizeof(buf_in));
	buf_in[0] = '\0';
}

void flush_uart_out ()
{
	memset(buf_out, 0, sizeof(buf_out));
	buf_out[0] = '\0';
}

/*---------------------------------------------------------------------------*/

PROCESS(poll_process, "STA process");

AUTOSTART_PROCESSES(&poll_process);
/*---------------------------------------------------------------------------*/



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
		serial_in(&buf_in);
		flush_uart_in();
    }
}


void serial_in(unsigned char *buf)
{
    
    if (strncmp(buf_in, "set", 3) == 0)
    {
        PRINTF("[SERIAL]SETTING VALUE\n");
        int value = atoi(buf_in + 4);
        PRINTF("[SERIAL]VALUE: %d\n", value);
        set_value(value);
    }
    else if (strncmp(buf_in, "get", 3) == 0)
    {
        PRINTF("[SERIAL]GETTING VALUE\n");
        int value = get_value();
        PRINTF("[SERIAL]VALUE: %d\n", value);
        sprintf(buf_out, "%d", value);
        PRINTF("[SERIAL]BUF_OUT: %s\n", buf_out);
        uart1_send_bytes(buf_out, strlen(buf_out));
    }
    else if (strncmp(buf_in, "reset", 5) == 0)
    {
        PRINTF("[SERIAL]RESETTING VALUE\n");
        reset_value();
    }
    else if (strncmp(buf_in, "set_random", 10) == 0)
    {
        PRINTF("[SERIAL]SETTING RANDOM VALUE\n");
        int value = random_rand();
        PRINTF("[SERIAL]VALUE: %d\n", value);
        set_value(value);
    }
    else if (strncmp(buf_in, "get_random", 10) == 0)
    {
        PRINTF("[SERIAL]GETTING RANDOM VALUE\n");
        int value = random_rand();
        PRINTF("[SERIAL]VALUE: %d\n", value);
        sprintf(buf_out, "%d", value);
        PRINTF("[SERIAL]BUF_OUT: %s\n", buf_out);
        uart1_send_bytes(buf_out, strlen(buf_out));
    }
    else if (strncmp(buf_in, "set_random_range", 16) == 0)
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
    static uint8_t *rxdata;
    static uint16_t count = 0; 
    static uint8_t buf[128];
    

    PROCESS_BEGIN();
    nullnet_set_input_callback(input_callback);
    uart_set_input(1, serial_line_input_byte)


    while(1)
    {



    }

    PROCESS_END();

}



