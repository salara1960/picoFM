#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include "pico/error.h"
#include "pico/stdlib.h"
#include "pico/types.h"
#include "pico/time.h"
#include "pico/binary_info.h"
//#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "pico/util/datetime.h"
#include "hardware/regs/addressmap.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/watchdog.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/rtc.h"
#include "hardware/adc.h"

#include "ssd1306.h"

//------------------------------------------------------------------------------

#define SET_PICO

#define SET_RDA

#define SET_IR

#define SET_JOSTIC

//#define SET_WITH_DMA

//------------------------------------------------------------------------------

#ifndef i2c_default
	#define i2c_default  i2c0
#endif

#define MAX_UART_BUF 256

#ifdef SET_RDA
	#define portRDA     i2c0

	#define MAX_LIST      26//25
	#define MAX_BAND       4
	#define MAX_STEP       4
	#define MAX_SIZE_NAME 16//31

	#define MAX_CMDS      16//15//6
#else
	#define MAX_CMDS       8
#endif
//
#ifdef SET_WITH_DMA
	#ifdef SET_RDA
		#define MAX_ERR_CODE 7
	#else
		#define MAX_ERR_CODE 6
	#endif
#else
	#ifdef SET_RDA
		#define MAX_ERR_CODE 6
	#else
		#define MAX_ERR_CODE 5
	#endif
#endif
//
#define UART_ID 	 uart0
#define BAUD_RATE 	230400 //115200
#define UART_TX_PIN      0 //12//0
#define UART_RX_PIN      1 //13//1
//
#define SET_SSD1306
#ifdef SET_SSD1306
    #define OLED_128x32
    #define OLED_ADDR _u(0x3C)
#endif

//------------------------------------------------------------------------------

extern volatile uint16_t devError;
extern const uint LED_PIN;
extern const uint ERR_PIN;
extern volatile uint32_t seconda;
extern int tZone;
extern bool uart_enable;

//------------------------------------------------------------------------------

