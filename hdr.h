#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
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
//#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/unique_id.h"
#include "pico/mutex.h"


//------------------------------------------------------------------------------


#define portI2C i2c1

#define portSPI spi0

#define SET_PICO

//#define SET_JOYSTIC
#define SET_ENCODER
#ifdef SET_ENCODER
	//#define SET_ENC4
#endif
//#define SET_KBD_MUX

#ifndef SET_JOYSTIC
	#define SET_MINI_DEV
#endif

#define SET_WITH_DMA
#define RUS_SUPPORT

#define SET_TIK_LED


#define SET_FLASH


#define SET_BLE
#ifdef SET_BLE
	#define SET_MUTEX
#endif

//------------------------------------------------------------------------------

#define MAX_UART_BUF 256


#define I2C_SDA_PIN 2//PICO_DEFAULT_I2C_SDA_PIN
#define I2C_SCL_PIN 3//PICO_DEFAULT_I2C_SCL_PIN

#define LCD_DC_PIN 4
#define LCD_RST_PIN 5
#define LCD_SCK_PIN  6
#define LCD_MOSI_PIN 7

//#define SET_RDA_FP
#define MIN_VOLUME  0
#define MAX_VOLUME 15

#define portRDA  portI2C

#define MAX_LIST      27//26//25
#define MAX_BAND       4
#define MAX_STEP       4
#define MAX_SIZE_NAME 31//16//31
#define MAX_MENU      12//11//10//9//8//7//6

#ifdef SET_ENCODER
	#define MAX_CMDS  30//29//28//27//26//25//24//22//21//20//17//16//15//6
#else
	#define MAX_CMDS  22//21//20//19//18//15//6
#endif

//
#define MAX_ERR_CODE 8//7
//
#define UART_ID 	 uart0
#define BAUD_RATE 	230400 //115200
#define UART_TX_PIN      0 //12//0
#define UART_RX_PIN      1 //13//1
#ifdef SET_BLE
	#define UART_BLE 	    uart1
	#define BAUD_RATE_BLE   9600
	#define UART_TX_BLE_PIN 8
	#define UART_RX_BLE_PIN 9
#endif
//


//------------------------------------------------------------------------------

extern volatile uint16_t devError;
extern const uint LED_PIN;
extern const uint ERR_PIN;
extern volatile uint32_t seconda;
extern int tZone;
extern bool uart_enable;

//------------------------------------------------------------------------------

#pragma once

enum {
	devOk = 0,
	devMem = 1,
	devTik = 2,
	devQue = 4,
	devUart = 8,
	devI2c = 16,
	devDma = 32,
	dev_RDA = 64,
	devList = 128
};

#pragma pack(push,1)
typedef struct {
	char mac[16];
	char name[20];
} ble_dev_t;
#pragma pack(pop)


