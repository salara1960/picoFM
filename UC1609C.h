#ifndef _UC1609C_H
#define _UC1609C_H

#include "hdr.h"

#ifdef SET_LCD_UC


#include "fonts.h"

//#include "stdlib.h"
//#include "string.h"
//#include "stdio.h"


//	A	 ->   anode led ( +3.3V or PWM )  |	 K	 ->   katode led ( GND )

#ifdef FOR_STM32

	#define 	UC1609C_SPI_HAL 		hspi1
		
	//=== указываем порты ( если в кубе назвали их DC RES CS то тогда нечего указывать не нужно )
	#if defined (DC_GPIO_Port)
	#else
		#define DC_GPIO_Port	GPIOC
		#define DC_Pin			GPIO_PIN_5
	#endif
			
	#if defined (RST_GPIO_Port)
	#else
		#define RST_GPIO_Port   GPIOB
		#define RST_Pin			GPIO_PIN_14
	#endif
			
	#define CS_PORT
	#ifdef CS_PORT
		#if defined (CS_GPIO_Port)
		#else
			#define CS_GPIO_Port    GPIOB
			#define CS_Pin			GPIO_PIN_12
		#endif
	#endif

	#ifdef UC1609C_SPI_HAL
		extern SPI_HandleTypeDef UC1609C_SPI_HAL;
	#endif

	#define CS_SELECT() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET )
	#define CS_UnSELECT() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET )
	#define DC_SELECT() HAL_GPIO_WritePin( DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET )
	#define DC_UnSELECT() HAL_GPIO_WritePin( DC_GPIO_Port, DC_Pin, GPIO_PIN_SET )

#else
	#define CS_SELECT() {}//gpio_put(LCD_CS_PIN, 0);
	#define CS_UnSELECT() {}//gpio_put(LCD_CS_PIN, 1);
	#define DC_SELECT() gpio_put(LCD_DC_PIN, 0);
	#define DC_UnSELECT() gpio_put(LCD_DC_PIN, 1);

	#define errMEM 1 //devMem
#endif

//----------------------------------------------------------------------------------------
			
// def: 192 x 64
#define UC1609C_width	192
#define	UC1609C_height	64
#define UC1609C_WIDTH	UC1609C_width
#define UC1609C_HEIGHT	UC1609C_height


#define ABS(x)   ((x) > 0 ? (x) : -(x))


// UC1609C Read registers ---------------------------------------------
#define UC1609C_GET_STATUS 					0x01 	// Not used v1.0
// UC1609C Write registers --------------------------------------------
#define UC1609C_SYSTEM_RESET 				0xE2 	// Not used v1.0
#define UC1609C_POWER_CONTROL 				0x2F 
#define UC1609C_PC_SET 						0x06 	// PC[2:0] 110 Internal V LCD (7x charge pump) + 10b: 1.4mA

#define UC1609C_ADDRESS_CONTROL 			0x88 	// set RAM address control
#define UC1609_ADDRESS_SET 					0x02 	// FOR UC1609  Set AC [2:0] Program registers  for RAM address control.
#define UC1609C_ADDRESS_SET 				0x01 	// FOR UC1609C  Set AC [2:0] Program registers  for RAM address control.

#define UC1609C_SET_PAGEADD 				0xB0 	// Page address Set PA[3:0]
#define UC1609C_SET_COLADD_LSB 				0x00 	// Column Address Set CA [3:0]
#define UC1609C_SET_COLADD_MSB 				0x10 	// Column Address Set CA [7:4]

#define UC1609C_TEMP_COMP_REG 				0x27 	// Temperature Compensation Register
#define UC1609C_TEMP_COMP_SET 				0x00 	// TC[1:0] = 00b= -0.00%/ C

#define UC1609C_FRAMERATE_REG 				0xA0 	// Frame rate
#define UC1609C_FRAMERATE_SET 				0x01  	// Set Frame Rate LC [4:3] 01b: 95 fps

#define UC1609C_BIAS_RATIO 					0xE8 	// Bias Ratio. The ratio between V-LCD and V-D .
#define UC1609C_BIAS_RATIO_SET 				0x03 	//  Set BR[1:0] = 11 (set to 9 default)

#define UC1609C_GN_PM 						0x81 	// Set V BIAS Potentiometer to fine tune V-D and V-LCD  (double-byte command)
#define UC1609C_DEFAULT_GN_PM 				0x49 	// default only used if user does not specify Vbias

#define UC1609C_LCD_CONTROL 				0xC0 	// Rotate map control
#define UC1609C_DISPLAY_ON 					0xAE 	// enables display
#define UC1609C_ALL_PIXEL_ON 				0xA4 	// sets on all Pixels on
#define UC1609C_INVERSE_DISPLAY 			0xA6 	// inverts display
#define UC1609C_SCROLL 						0x40 	// scrolls , Set the scroll line number. 0-64
// Rotate -------------------------------------------------------------
#define UC1609C_ROTATION_FLIP_TWO 			0x06
#define UC1609C_ROTATION_NORMAL 			0x04
#define UC1609C_ROTATION_FLIP_ONE 			0x02
#define UC1609C_ROTATION_FLIP_THREE 		0x00

//---------------------------------------------------------------------

// Отображение определения цветов пикселей
// (1): белый на синем, FG = белый BG = синий    // ERM19264SBS-4 ЖК-дисплей белый на синем
// (2): черный на белом, FG = черный BG = белый  // ERM19264FS-4 ЖК-дисплей, черный на белом
// (3): белое на черном, FG = белое, BG = черное // ERM19264DNS-4 ЖК-дисплей Белый на черном
#define FOREGROUND 0
#define BACKGROUND 1
#define INVERSE	   2

//----------------------------------------------------------------------------------

void UC1609C_init( void );
void UC1609C_contrast (uint8_t bits);//значение от 0....255 ( по умолчанию 30 )
void UC1609C_enable (uint8_t bits);//1 - ON    0 - OFF
void UC1609C_scroll (uint8_t bits);
void UC1609C_rotate(uint8_t rotatevalue);
void UC1609C_invertDisplay (uint8_t bits);
void UC1609C_allPixelsOn(uint8_t bits);
void UC1609C_fillScreen(uint8_t dataPattern);
void UC1609C_update( void );
void UC1609C_drawPixel( int16_t x, int16_t y, uint8_t colour );
void UC1609C_clearBuffer( void );
void UC1609C_bitmap(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t* data, uint8_t colour);
void UC1609C_clearDisplay( void );			
void UC1609C_Print(int16_t x, int16_t y, char* str, FontDef_t* Font, uint8_t multiplier, uint8_t color);
void UC1609C_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c);
void UC1609C_DrawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t c);
void UC1609C_DrawFilledRectangle(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t c);
void UC1609C_DrawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t color);
void UC1609C_DrawFilledTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint8_t color);
void UC1609C_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t c);
void UC1609C_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint8_t c);

int16_t caclX(char *str, uint16_t width);
char *mkLineCenter(char *str, uint16_t width);
char *mkLineWidth(char *str1, char *str2, uint16_t width);
void showLine(char *msg, uint16_t lin, FontDef_t *fnt, bool update, uint8_t back);
void clrLines(uint16_t lin, uint8_t cnt, uint8_t update, uint8_t fh, uint8_t inv);

#endif  // SET_LCD_UC
	

#endif	//	_UC1609C_H
