/*
 * libs.c
 *
 *  Created on: Aug 16, 2022
 *      Author: Salara
 */
#include "libs.h"


volatile uint32_t ms10 = 0;

static char txBuf[MAX_UART_BUF << 2] = {0};

//------------------------------------------------------------------------------------------

#ifdef SET_WITH_DMA

int chan;

//------------------------------------------------------------------------------------------
void iniDMA()
{
	chan = dma_claim_unused_channel(true);
	if (chan == -1) {
		devError |= devDMA;
		return;
	}

	// 8 bit transfers. Both read and write address increment after each
	// transfer (each pointing to a location in src or dst respectively).
	// No DREQ is selected, so the DMA transfers as fast as it can.

	dma_channel_config conf = dma_channel_get_default_config(chan);
	channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
	//channel_config_set_read_increment(&conf, false);
	channel_config_set_write_increment(&conf, true);

	strcpy(txBuf, "UART_DMA_INIT\n");

	dma_channel_configure(
		chan,            // Channel to be configured
		&conf,           // The configuration we just created
		&uart_get_hw(UART_ID)->dr,//UART_ID->UARTDR_DATA, //UART_ID->UARTDR.DATA, //UARTDR, //dst, // The initial write address
		txBuf,//src,     // The initial read address
		strlen(txBuf),   // Number of transfers; in this case each is 1 byte.
		true      // Start immediately.
	);
	///uart_get_hw(UART_ID)->dmacr |= 2; //UART_ID->UARTDMACR_TXDMAE = UART_UARTDMACR_TXDMAE_BITS;
	    // We could choose to go and do something else whilst the DMA is doing its
	    // thing. In this case the processor has nothing else to do, so we just
	    // wait for the DMA to finish.
	dma_channel_wait_for_finish_blocking(chan);

}
//------------------------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------------------------
void set_sec(uint32_t sec)
{
    //seconda = sec;
	time_t ep = (time_t)sec;
	struct tm ts;
	gmtime_r(&ep, &ts);
	datetime_t dt = {
		.year  = ts.tm_year + 1900, // Year - 1900,
		.month = ts.tm_mon + 1 ,    // Month (0-11)
		.day   = ts.tm_mday,        // Day of the month (1-31)
		.dotw  = ts.tm_wday,        // Day of the week (0-6, Sunday = 0) // 0 is Sunday, so 5 is Friday
		.hour  = ts.tm_hour,        // Hours (0-23)
		.min   = ts.tm_min,         // Minutes (0-59)
		.sec   = ts.tm_sec          // Seconds (0-60)
	};
	rtc_set_datetime(&dt);
}
//-----------------------------------------------------------------------------
void inc_msCounter()
{
	ms10++;
}
//-----------------------------------------------------------------------------
uint32_t get_msCounter()
{
	return ms10;
}
//------------------------------------------------------------------------------------------
uint32_t get_mstmr(uint32_t hs)
{
	return (get_msCounter() + hs);
}
//------------------------------------------------------------------------------------------
bool check_mstmr(uint32_t hs)
{
	return (get_msCounter() >= hs ? true : false);
}
//------------------------------------------------------------------------------------------
uint32_t get_sec()
{
    return seconda;
}
//------------------------------------------------------------------------------------------
void inc_sec()
{
    seconda++;
}
//------------------------------------------------------------------------------------------
uint32_t get_tmr(uint32_t sec)
{
    return (get_sec() + sec);
}
//------------------------------------------------------------------------------------------
bool check_tmr(uint32_t sec)
{
    return (get_sec() >= sec ? true : false);
}
//-------------------------------------------------------------------------------------------
void errLedOn(bool on)
{
	if (on)
		gpio_put(ERR_PIN, 1);
	else
		gpio_put(ERR_PIN, 0);
}
//------------------------------------------------------------------------------------------
int sec2str(char *st)
{
datetime_t dt;

    rtc_get_datetime(&dt);

    return sprintf(st, "%02d.%02d.%02d %02d:%02d:%02d",
                dt.day, dt.month, dt.year - 2000,
                dt.hour + tZone, dt.min, dt.sec);
}
//------------------------------------------------------------------------------------------
void Report(const uint8_t addTime, const char *fmt, ...)
{
	if (!uart_enable) return;

	size_t len = MAX_UART_BUF << 2;
	char *buf = &txBuf[0];
	memset(buf, 0, sizeof(txBuf));//*buf = '\0';
	int dl = 0;
	if (addTime) {
		dl = sec2str(buf);
		strcat(buf, " | ");
		dl += 3;
	}

	va_list args;
	va_start(args, fmt);
	vsnprintf(buf + dl, len - dl, fmt, args);
	printf("%s", buf);
	va_end(args);
}
//------------------------------------------------------------------------------------------

