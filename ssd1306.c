#include "hdr.h"

#ifdef SET_SSD1306

uint8_t invert = OLED_CMD_DISPLAY_NORMAL;//0xA6 //OLED_CMD_DISPLAY_INVERTED - 0xA7

uint8_t font8x8[128][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0000 (nul)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0001
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0002
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0003
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0004
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0005
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0006
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0007
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0008
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0009
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+000A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+000B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+000C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+000D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+000E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+000F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0010
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0011
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0012
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0013
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0014
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0015
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0016
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0017
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0018
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0019
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+001A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+001B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+001C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+001D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+001E
    { 0x00, 0x0E, 0x11, 0x11, 0x0E, 0x00, 0x00, 0x00 },   // U+001F (gradus)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0020 (space)
    { 0x00, 0x00, 0x06, 0x5F, 0x5F, 0x06, 0x00, 0x00 },   // U+0021 (!)
    { 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00 },   // U+0022 (")
    { 0x14, 0x7F, 0x7F, 0x14, 0x7F, 0x7F, 0x14, 0x00 },   // U+0023 (#)
    { 0x24, 0x2E, 0x6B, 0x6B, 0x3A, 0x12, 0x00, 0x00 },   // U+0024 ($)
    { 0x46, 0x66, 0x30, 0x18, 0x0C, 0x66, 0x62, 0x00 },   // U+0025 (%)
    { 0x30, 0x7A, 0x4F, 0x5D, 0x37, 0x7A, 0x48, 0x00 },   // U+0026 (&)
    { 0x04, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0027 (')
    { 0x00, 0x1C, 0x3E, 0x63, 0x41, 0x00, 0x00, 0x00 },   // U+0028 (()
    { 0x00, 0x41, 0x63, 0x3E, 0x1C, 0x00, 0x00, 0x00 },   // U+0029 ())
    { 0x08, 0x2A, 0x3E, 0x1C, 0x1C, 0x3E, 0x2A, 0x08 },   // U+002A (*)
    { 0x08, 0x08, 0x3E, 0x3E, 0x08, 0x08, 0x00, 0x00 },   // U+002B (+)
    { 0x00, 0x80, 0xE0, 0x60, 0x00, 0x00, 0x00, 0x00 },   // U+002C (,)
    { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00 },   // U+002D (-)
    { 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00 },   // U+002E (.)
    { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00 },   // U+002F (/)
    { 0x3E, 0x7F, 0x71, 0x59, 0x4D, 0x7F, 0x3E, 0x00 },   // U+0030 (0)
    { 0x40, 0x42, 0x7F, 0x7F, 0x40, 0x40, 0x00, 0x00 },   // U+0031 (1)
    { 0x62, 0x73, 0x59, 0x49, 0x6F, 0x66, 0x00, 0x00 },   // U+0032 (2)
    { 0x22, 0x63, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00 },   // U+0033 (3)
    { 0x18, 0x1C, 0x16, 0x53, 0x7F, 0x7F, 0x50, 0x00 },   // U+0034 (4)
    { 0x27, 0x67, 0x45, 0x45, 0x7D, 0x39, 0x00, 0x00 },   // U+0035 (5)
    { 0x3C, 0x7E, 0x4B, 0x49, 0x79, 0x30, 0x00, 0x00 },   // U+0036 (6)
    { 0x03, 0x03, 0x71, 0x79, 0x0F, 0x07, 0x00, 0x00 },   // U+0037 (7)
    { 0x36, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00, 0x00 },   // U+0038 (8)
    { 0x06, 0x4F, 0x49, 0x69, 0x3F, 0x1E, 0x00, 0x00 },   // U+0039 (9)
    { 0x00, 0x00, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00 },   // U+003A (:)
    { 0x00, 0x80, 0xE6, 0x66, 0x00, 0x00, 0x00, 0x00 },   // U+003B (;)
    { 0x08, 0x1C, 0x36, 0x63, 0x41, 0x00, 0x00, 0x00 },   // U+003C (<)
    { 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00, 0x00 },   // U+003D (=)
    { 0x00, 0x41, 0x63, 0x36, 0x1C, 0x08, 0x00, 0x00 },   // U+003E (>)
    { 0x02, 0x03, 0x51, 0x59, 0x0F, 0x06, 0x00, 0x00 },   // U+003F (?)
    { 0x3E, 0x7F, 0x41, 0x5D, 0x5D, 0x1F, 0x1E, 0x00 },   // U+0040 (@)
    { 0x7C, 0x7E, 0x13, 0x13, 0x7E, 0x7C, 0x00, 0x00 },   // U+0041 (A)
    { 0x41, 0x7F, 0x7F, 0x49, 0x49, 0x7F, 0x36, 0x00 },   // U+0042 (B)
    { 0x1C, 0x3E, 0x63, 0x41, 0x41, 0x63, 0x22, 0x00 },   // U+0043 (C)
    { 0x41, 0x7F, 0x7F, 0x41, 0x63, 0x3E, 0x1C, 0x00 },   // U+0044 (D)
    { 0x41, 0x7F, 0x7F, 0x49, 0x5D, 0x41, 0x63, 0x00 },   // U+0045 (E)
    { 0x41, 0x7F, 0x7F, 0x49, 0x1D, 0x01, 0x03, 0x00 },   // U+0046 (F)
    { 0x1C, 0x3E, 0x63, 0x41, 0x51, 0x73, 0x72, 0x00 },   // U+0047 (G)
    { 0x7F, 0x7F, 0x08, 0x08, 0x7F, 0x7F, 0x00, 0x00 },   // U+0048 (H)
    { 0x00, 0x41, 0x7F, 0x7F, 0x41, 0x00, 0x00, 0x00 },   // U+0049 (I)
    { 0x30, 0x70, 0x40, 0x41, 0x7F, 0x3F, 0x01, 0x00 },   // U+004A (J)
    { 0x41, 0x7F, 0x7F, 0x08, 0x1C, 0x77, 0x63, 0x00 },   // U+004B (K)
    { 0x41, 0x7F, 0x7F, 0x41, 0x40, 0x60, 0x70, 0x00 },   // U+004C (L)
    { 0x7F, 0x7F, 0x0E, 0x1C, 0x0E, 0x7F, 0x7F, 0x00 },   // U+004D (M)
    { 0x7F, 0x7F, 0x06, 0x0C, 0x18, 0x7F, 0x7F, 0x00 },   // U+004E (N)
    { 0x1C, 0x3E, 0x63, 0x41, 0x63, 0x3E, 0x1C, 0x00 },   // U+004F (O)
    { 0x41, 0x7F, 0x7F, 0x49, 0x09, 0x0F, 0x06, 0x00 },   // U+0050 (P)
    { 0x1E, 0x3F, 0x21, 0x71, 0x7F, 0x5E, 0x00, 0x00 },   // U+0051 (Q)
    { 0x41, 0x7F, 0x7F, 0x09, 0x19, 0x7F, 0x66, 0x00 },   // U+0052 (R)
    { 0x26, 0x6F, 0x4D, 0x59, 0x73, 0x32, 0x00, 0x00 },   // U+0053 (S)
    { 0x03, 0x41, 0x7F, 0x7F, 0x41, 0x03, 0x00, 0x00 },   // U+0054 (T)
    { 0x7F, 0x7F, 0x40, 0x40, 0x7F, 0x7F, 0x00, 0x00 },   // U+0055 (U)
    { 0x1F, 0x3F, 0x60, 0x60, 0x3F, 0x1F, 0x00, 0x00 },   // U+0056 (V)
    { 0x7F, 0x7F, 0x30, 0x18, 0x30, 0x7F, 0x7F, 0x00 },   // U+0057 (W)
    { 0x43, 0x67, 0x3C, 0x18, 0x3C, 0x67, 0x43, 0x00 },   // U+0058 (X)
    { 0x07, 0x4F, 0x78, 0x78, 0x4F, 0x07, 0x00, 0x00 },   // U+0059 (Y)
    { 0x47, 0x63, 0x71, 0x59, 0x4D, 0x67, 0x73, 0x00 },   // U+005A (Z)
    { 0x00, 0x7F, 0x7F, 0x41, 0x41, 0x00, 0x00, 0x00 },   // U+005B ([)
    { 0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00 },   // U+005C (\)
    { 0x00, 0x41, 0x41, 0x7F, 0x7F, 0x00, 0x00, 0x00 },   // U+005D (])
    { 0x08, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x08, 0x00 },   // U+005E (^)
    { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 },   // U+005F (_)
    { 0x00, 0x00, 0x03, 0x07, 0x04, 0x00, 0x00, 0x00 },   // U+0060 (`)
    { 0x20, 0x74, 0x54, 0x54, 0x3C, 0x78, 0x40, 0x00 },   // U+0061 (a)
    { 0x41, 0x7F, 0x3F, 0x48, 0x48, 0x78, 0x30, 0x00 },   // U+0062 (b)
    { 0x38, 0x7C, 0x44, 0x44, 0x6C, 0x28, 0x00, 0x00 },   // U+0063 (c)
    { 0x30, 0x78, 0x48, 0x49, 0x3F, 0x7F, 0x40, 0x00 },   // U+0064 (d)
    { 0x38, 0x7C, 0x54, 0x54, 0x5C, 0x18, 0x00, 0x00 },   // U+0065 (e)
    { 0x48, 0x7E, 0x7F, 0x49, 0x03, 0x02, 0x00, 0x00 },   // U+0066 (f)
    { 0x98, 0xBC, 0xA4, 0xA4, 0xF8, 0x7C, 0x04, 0x00 },   // U+0067 (g)
    { 0x41, 0x7F, 0x7F, 0x08, 0x04, 0x7C, 0x78, 0x00 },   // U+0068 (h)
    { 0x00, 0x44, 0x7D, 0x7D, 0x40, 0x00, 0x00, 0x00 },   // U+0069 (i)
    { 0x60, 0xE0, 0x80, 0x80, 0xFD, 0x7D, 0x00, 0x00 },   // U+006A (j)
    { 0x41, 0x7F, 0x7F, 0x10, 0x38, 0x6C, 0x44, 0x00 },   // U+006B (k)
    { 0x00, 0x41, 0x7F, 0x7F, 0x40, 0x00, 0x00, 0x00 },   // U+006C (l)
    { 0x7C, 0x7C, 0x18, 0x38, 0x1C, 0x7C, 0x78, 0x00 },   // U+006D (m)
    { 0x7C, 0x7C, 0x04, 0x04, 0x7C, 0x78, 0x00, 0x00 },   // U+006E (n)
    { 0x38, 0x7C, 0x44, 0x44, 0x7C, 0x38, 0x00, 0x00 },   // U+006F (o)
    { 0x84, 0xFC, 0xF8, 0xA4, 0x24, 0x3C, 0x18, 0x00 },   // U+0070 (p)
    { 0x18, 0x3C, 0x24, 0xA4, 0xF8, 0xFC, 0x84, 0x00 },   // U+0071 (q)
    { 0x44, 0x7C, 0x78, 0x4C, 0x04, 0x1C, 0x18, 0x00 },   // U+0072 (r)
    { 0x48, 0x5C, 0x54, 0x54, 0x74, 0x24, 0x00, 0x00 },   // U+0073 (s)
    { 0x00, 0x04, 0x3E, 0x7F, 0x44, 0x24, 0x00, 0x00 },   // U+0074 (t)
    { 0x3C, 0x7C, 0x40, 0x40, 0x3C, 0x7C, 0x40, 0x00 },   // U+0075 (u)
    { 0x1C, 0x3C, 0x60, 0x60, 0x3C, 0x1C, 0x00, 0x00 },   // U+0076 (v)
    { 0x3C, 0x7C, 0x70, 0x38, 0x70, 0x7C, 0x3C, 0x00 },   // U+0077 (w)
    { 0x44, 0x6C, 0x38, 0x10, 0x38, 0x6C, 0x44, 0x00 },   // U+0078 (x)
    { 0x9C, 0xBC, 0xA0, 0xA0, 0xFC, 0x7C, 0x00, 0x00 },   // U+0079 (y)
    { 0x4C, 0x64, 0x74, 0x5C, 0x4C, 0x64, 0x00, 0x00 },   // U+007A (z)
    { 0x08, 0x08, 0x3E, 0x77, 0x41, 0x41, 0x00, 0x00 },   // U+007B ({)
    { 0x00, 0x00, 0x00, 0x77, 0x77, 0x00, 0x00, 0x00 },   // U+007C (|)
    { 0x41, 0x41, 0x77, 0x3E, 0x08, 0x08, 0x00, 0x00 },   // U+007D (})
    { 0x02, 0x03, 0x01, 0x03, 0x02, 0x03, 0x01, 0x00 },   // U+007E (~)
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 }    // U+007F ( )
};

//******************************************************************************************

//-----------------------------------------------------------------------------------------
void oled_send_cmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(portOLED, OLED_ADDR/* & I2C_WRITE_MODE)*/, buf, 2, false);
}
//-----------------------------------------------------------------------------------------
void oled_send_cmds(uint8_t *cmd, uint8_t len)
{
	if (!len) return;

    uint8_t *buf = calloc(1, len + 1);
    if (buf) {
    	if (len == 1) buf[0] = 0x80;
    	memcpy(&buf[1], cmd, len);
    	i2c_write_blocking(portOLED, OLED_ADDR, buf, len + 1, false);
    	free(buf);
    } else {
    	printf("[%s] Error calloc(1, %u)\n", __func__, len + 1);
    }
}
//-----------------------------------------------------------------------------------------
void ssd1306_on(bool flag)
{
uint8_t bt;

    if (flag) bt = OLED_CMD_DISPLAY_ON; else bt = OLED_CMD_DISPLAY_OFF;

    uint8_t buf[] = {
        OLED_CONTROL_BYTE_CMD_SINGLE,
        bt
    };
    i2c_write_blocking(portOLED, OLED_ADDR, buf, sizeof(buf), false);
}
//-----------------------------------------------------------------------------------------
void ssd1306_init()
{
#ifdef OLED_128x32
    uint8_t ini[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_DISPLAY_OFF,
        OLED_CMD_SET_DISPLAY_CLK_DIV,//0xd5, // SSD1306_SETDISPLAYCLOCKDIV
        0x80, // Suggested value 0x80
        OLED_CMD_SET_MUX_RATIO,//0xa8, // SSD1306_SETMULTIPLEX
        0x1f, // 1/32
        OLED_CMD_SET_DISPLAY_OFFSET,//0xd3, // SSD1306_SETDISPLAYOFFSET
        0x00, // 0 no offset
        OLED_CMD_SET_DISPLAY_START_LINE,//0x40, // SSD1306_SETSTARTLINE line #0
        OLED_CMD_SET_CHARGE_PUMP,//0x8d, // SSD1306_CHARGEPUMP
        0x14, // Charge pump on
        OLED_CMD_SET_MEMORY_ADDR_MODE,//0x20, // SSD1306_MEMORYMODE
        0x00, // 0x0 act like ks0108
        //no rotation //to 180 
        OLED_CMD_SET_SEGMENT_REMAP,//0xa0//0xa1, // SSD1306_SEGREMAP | 1
        OLED_CMD_SET_COM_SCAN_MODE,//0xc0//0xc8, // SSD1306_COMSCANDEC
        //
        OLED_CMD_SET_COM_PIN_MAP,//0xda, // SSD1306_SETCOMPINS
        0x02,
        OLED_CMD_SET_CONTRAST,//0x81, // SSD1306_SETCONTRAST
        0x2f,
        OLED_CMD_SET_PRECHARGE,//0xd9, // SSD1306_SETPRECHARGE
        0xf1,
        OLED_CMD_SET_VCOMH_DESELCT,//0xdb, // SSD1306_SETVCOMDETECT
        0x40,
        OLED_CMD_SET_DEACTIVATE_SCROLL,//0x2e, // SSD1306_DEACTIVATE_SCROLL
        OLED_CMD_DISPLAY_RAM,//0xa4, // SSD1306_DISPLAYALLON_RESUME
        invert,//0xa6  // SSD1306_NORMALDISPLAY
        OLED_CMD_DISPLAY_ON
    };
#else 
    uint8_t ini[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_DISPLAY_OFF,//0xae, // SSD1306_DISPLAYOFF
        OLED_CMD_SET_DISPLAY_CLK_DIV,//0xd5, // SSD1306_SETDISPLAYCLOCKDIV
        0x80, // Suggested value 0x80
        OLED_CMD_SET_MUX_RATIO,//0xa8, // SSD1306_SETMULTIPLEX
        0x3f, // 1/64
        OLED_CMD_SET_DISPLAY_OFFSET,//0xd3, // SSD1306_SETDISPLAYOFFSET
        0x00, // 0 no offset
        OLED_CMD_SET_DISPLAY_START_LINE,//0x40, // SSD1306_SETSTARTLINE line #0
        OLED_CMD_SET_MEMORY_ADDR_MODE,//0x20, // SSD1306_MEMORYMODE
        0x00, // 0x0 act like ks0108
        //no rotation
        OLED_CMD_SET_SEGMENT_REMAP,//0xa1, // SSD1306_SEGREMAP | 1
        OLED_CMD_SET_COM_SCAN_MODE,//0xc8, // SSD1306_COMSCANDEC
        //
        OLED_CMD_SET_COM_PIN_MAP,//0xda, // SSD1306_SETCOMPINS
        0x12,
        OLED_CMD_SET_CONTRAST,//0x81, // SSD1306_SETCONTRAST
        0xcf,
        OLED_CMD_SET_PRECHARGE,//0xd9, // SSD1306_SETPRECHARGE
        0xf1,
        OLED_CMD_SET_VCOMH_DESELCT,//0xdb, // SSD1306_SETVCOMDETECT
        0x30,
        OLED_CMD_SET_CHARGE_PUMP,//0x8d, // SSD1306_CHARGEPUMP
        0x14, // Charge pump on
        OLED_CMD_SET_DEACTIVATE_SCROLL,//0x2e, // SSD1306_DEACTIVATE_SCROLL
        OLED_CMD_DISPLAY_RAM,//0xa4, // SSD1306_DISPLAYALLON_RESUME
        invert,//0xa6  // SSD1306_NORMALDISPLAY
        OLED_CMD_DISPLAY_ON
    };
#endif

    i2c_write_blocking(portOLED, OLED_ADDR, ini, sizeof(ini), false);

}
//-----------------------------------------------------------------------------------------
void ssd1306_invert()
{
    if (invert == OLED_CMD_DISPLAY_INVERTED) invert = OLED_CMD_DISPLAY_NORMAL;
                                        else invert = OLED_CMD_DISPLAY_INVERTED;
    uint8_t buf[] = {
        OLED_CONTROL_BYTE_CMD_SINGLE,
        invert
    };                                 
    i2c_write_blocking(portOLED, OLED_ADDR, buf, sizeof(buf), false);
}
//-----------------------------------------------------------------------------------------
void ssd1306_clear()
{
uint8_t zero[129] = {0};
uint8_t bytes[] = {OLED_CONTROL_BYTE_CMD_SINGLE, 0};

    zero[0] = OLED_CONTROL_BYTE_DATA_STREAM;

    for (uint8_t i = 0; i < OLED_MAX_STR; i++) {
        bytes[1] = 0xB0 | i;
        i2c_write_blocking(portOLED, OLED_ADDR, bytes, sizeof(bytes), true);

        i2c_write_blocking(portOLED, OLED_ADDR, zero, sizeof(zero), false);
    }
}
//-----------------------------------------------------------------------------------------
void ssd1306_clear_lines(uint8_t cy, uint8_t cnt)
{
    if (cy > OLED_MAX_STR) return;

    cy--;
    uint8_t to = cy + cnt - 1;
    if (to > OLED_MAX_STR) return;

    uint8_t zero[129] = {0};
    zero[0] = OLED_CONTROL_BYTE_DATA_STREAM;    
    uint8_t bytes[] = {OLED_CONTROL_BYTE_CMD_SINGLE, 0};

    for (uint8_t i = cy; i < to; i++) {
        bytes[1] = 0xB0 | i;
        i2c_write_blocking(portOLED, OLED_ADDR, bytes, sizeof(bytes), true);

        i2c_write_blocking(portOLED, OLED_ADDR, zero, sizeof(zero), false);
    }
}
//-----------------------------------------------------------------------------------------
void ssd1306_pattern()
{
uint8_t patt[129] = {0};
uint8_t bytes[] = {OLED_CONTROL_BYTE_CMD_SINGLE, 0};
uint8_t j;

    for (j = 1; j < 129; j++) patt[j] = 0xFF >> (j % OLED_MAX_STR);
    patt[0] = OLED_CONTROL_BYTE_DATA_STREAM;

    for (uint8_t i = 0; i < OLED_MAX_STR; i++) {
        bytes[1] = 0xB0 | i;
        i2c_write_blocking(portOLED, OLED_ADDR, bytes, sizeof(bytes), true);

        i2c_write_blocking(portOLED, OLED_ADDR, patt, sizeof(patt), false);
    }
}
//-----------------------------------------------------------------------------------------
void ssd1306_contrast(uint8_t value)//0xff or 0x00
{
uint8_t bytes[] = {OLED_CONTROL_BYTE_CMD_STREAM, OLED_CMD_SET_CONTRAST, value};

    i2c_write_blocking(portOLED, OLED_ADDR, bytes, sizeof(bytes), false);

}
//-----------------------------------------------------------------------------------------
void ssd1306_shift(bool left, uint8_t line)
{
uint8_t dir;

    if (left) dir = 0x27; else dir = 0x26;

    uint8_t bytes[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_SET_DEACTIVATE_SCROLL,//0x2E,
        dir,
        0,
        0,
        7,
        0x3F,
        OLED_CMD_SET_ACTIVATE_SCROLL//0x2F
    };

    i2c_write_blocking(portOLED, OLED_ADDR, bytes, sizeof(bytes), false);
}
//-----------------------------------------------------------------------------------------
void ssd1306_scroll(bool flag)
{
uint8_t bytes[] = {
    OLED_CONTROL_BYTE_CMD_STREAM,
    0x27,//0x29// vertical and horizontal scroll (p29)    
    0,
    0,
    7,
    1,
    0x3f,
    0xa3,// set vertical scroll area (p30)
    0x20,
    0x40,
    0
};
    if (flag) bytes[sizeof(bytes - 1)] = OLED_CMD_SET_ACTIVATE_SCROLL;//0x2F;// activate scroll (p29)
         else bytes[sizeof(bytes - 1)] = OLED_CMD_SET_DEACTIVATE_SCROLL;//0x2E;// deactivate scroll (p29)

    i2c_write_blocking(portOLED, OLED_ADDR, bytes, sizeof(bytes), false);
}
//-----------------------------------------------------------------------------------------
void ssd1306_text_xy(const char *stroka, uint8_t cx, uint8_t cy, bool inv)
{

if (cy > OLED_MAX_STR) return;

uint8_t i, lin = cy - 1, col = cx - 1, len = strlen(stroka); 
uint8_t first[] = {
    OLED_CONTROL_BYTE_CMD_STREAM,
    OLED_CMD_SET_COLUMN_RANGE,
    col << 3,
    0x7f,
    OLED_CMD_SET_PAGE_RANGE,
    lin,
    7
};
uint8_t dat[] = {OLED_CONTROL_BYTE_CMD_STREAM, 0, 0x10, 0};
uint8_t cif[] = {OLED_CONTROL_BYTE_DATA_STREAM, 0, 0, 0, 0, 0, 0, 0, 0};


    i2c_write_blocking(portOLED, OLED_ADDR, first, sizeof(first), true);

    for (i = 0; i < len; i++) {
        if (stroka[i] == '\n') {
            dat[3] = 0xB0 | ++lin;
            i2c_write_blocking(portOLED, OLED_ADDR, dat, sizeof(dat), false);
        } else {
            memcpy(&cif[1], &font8x8[(uint8_t)stroka[i]][0], 8);
            if (inv) 
                for (uint8_t j = 1; j < sizeof(cif); j++) cif[j] = ~cif[j];
            i2c_write_blocking(portOLED, OLED_ADDR, cif, 8, false);
        }
    }
}
//-----------------------------------------------------------------------------------------
void ssd1306_text(const char *stroka)
{
    ssd1306_text_xy(stroka, 1, 1, false);
}
//-----------------------------------------------------------------------------------------
uint8_t ssd1306_calcx(int len)
{
uint8_t ret = 1;

    if ( (len > 0) && (len < 16) ) ret = ((16 - len) >> 1) + 1;

    return ret;
}
//-----------------------------------------------------------------------------------------
char *mkLineCenter(char *str, uint16_t width)
{
char st[32] = {0};

    memset(st, 0x20, 32);
    uint8_t slen = OLED_WIDTH / width;//16
    uint8_t k = strlen(str);
    if (k < slen) {
        uint8_t n = (slen - k) >> 1;
        memcpy(&st[n], str, k);
        st[slen] = '\0';
        strcpy(str, st);
    }

    return str;
}
//-----------------------------------------------------------------------------------------
char *mkLineWidth(char *str1, char *str2, uint16_t width)
{
char st[64] = {0};

    uint8_t slen = OLED_WIDTH / width;
    uint8_t k1 = strlen(str1);
    uint8_t k2 = strlen(str2);
    if ((k1 + k2) <= slen) {
        uint8_t k = slen - (k1 + k2);
        strcpy(st, str1);
        for (int8_t i = 0; i < k; i++) st[k1 + i] = 0x20;
        strcat(st, str2);
        strcpy(str1, st);
    }

    return str1;
}

//******************************************************************************************

#endif
