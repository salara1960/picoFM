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
#include "pico/multicore.h"
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
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
// for ws2812 support
#include "hardware/clocks.h"
#include "ws2812.pio.h"
// for encoder support
//#include "hardware/timer.h"
//#include "quadrature_encoder.pio.h"
//
#include "pico/unique_id.h"



//#define SET_SSD1306


#define portI2C i2c1


#ifdef SET_SSD1306
	#define portOLED portI2C

	#include "ssd1306.h"

    #define OLED_128x32
    #define OLED_ADDR _u(0x3C)
#else
	#define portSPI spi0

	#define SET_LCD_UC
#endif

//------------------------------------------------------------------------------

#define SET_PICO

#define SET_RDA

//#define SET_IR
#define SET_JOYSTIC
//#define SET_ENCODER
//#define SET_KBD
#define SET_KBD_MUX


//#define SET_WITH_DMA
#define RUS_SUPPORT

//------------------------------------------------------------------------------

#define MAX_UART_BUF 256

#if defined(SET_RDA) || defined(SET_SSD1306)
	#define I2C_SDA_PIN 2//PICO_DEFAULT_I2C_SDA_PIN
	#define I2C_SCL_PIN 3//PICO_DEFAULT_I2C_SCL_PIN
#endif

#ifdef SET_LCD_UC
	#define LCD_DC_PIN 4
	//#define LCD_CS_PIN 5
	#define LCD_RST_PIN 5
	#define LCD_SCK_PIN  6
	#define LCD_MOSI_PIN 7
#endif

#ifdef SET_RDA

	//#define SET_RDA_FP
	#define MIN_VOLUME  0
	#define MAX_VOLUME 15

	#define portRDA  portI2C

	#define MAX_LIST      26//25
	#define MAX_BAND       4
	#define MAX_STEP       4
	#define MAX_SIZE_NAME 31//16//31

	#ifdef SET_ENCODER
		#define MAX_CMDS  21//20//17//16//15//6
	#else
		#define MAX_CMDS  18//15//6
	#endif

#else

	#ifdef SET_ENCODER
		#define MAX_CMDS   12
	#else
		#define MAX_CMDS   10
	#endif

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


//------------------------------------------------------------------------------

extern volatile uint16_t devError;
extern const uint LED_PIN;
extern const uint ERR_PIN;
extern volatile uint32_t seconda;
extern int tZone;
extern bool uart_enable;

//------------------------------------------------------------------------------

