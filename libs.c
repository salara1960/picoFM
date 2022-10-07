/*
 * libs.c
 *
 *  Created on: Aug 16, 2022
 *      Author: Salara
 */
#include "libs.h"

volatile uint32_t ms10 = 0;

static char txBuf[MAX_UART_BUF << 2] = {0};

#ifdef SET_BLE
	#ifdef SET_MUTEX
		uint32_t owner_out = 1000;
		mutex_t itMutex;
	#endif
	uint8_t ble_cli_total = 0;
	uint8_t ble_cli_len = 0;
#endif
//------------------------------------------------------------------------------------------
/*
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
*/
//------------------------------------------------------------------------------------------
//   Функция устанавливает дату и время в модуле RTC по значению Unix Timestamp
//
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
//-----------------------------------------------------------------------------------------
//              Функции для использования таймера с перидом в 5 ms
//-----------------------------------------------------------------------------------------
void inc_msCounter()
{
	ms10++;
}
//-----------------------------------------------------------------------------------------
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
//   Функция зажигает/тушит светодиод индикации ошибки
//
void errLedOn(bool on)
{
#ifndef SET_BLE
 	if (on)
		gpio_put(ERR_PIN, 1);
	else
		gpio_put(ERR_PIN, 0);
#endif
}
//------------------------------------------------------------------------------------------
//   Функция формирует символьную сторку из данных, прочитанных из модуля RTC
//
int sec2str(char *st)
{
datetime_t dt;

    rtc_get_datetime(&dt);

    return sprintf(st, "%02d.%02d.%02d %02d:%02d:%02d",
                dt.day, dt.month, dt.year - 2000,
                dt.hour + tZone, dt.min, dt.sec);
}
//------------------------------------------------------------------------------------------
//   Функция формирует и выводит на печать символьную строку (stdout - uart0)
//
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
	len = strlen(buf);
/*#ifdef SET_WITH_DMA
	int chan = dma_claim_unused_channel(true);
	if (chan == -1) {
		devError |= devDMA;
		return;
	}
	dma_channel_config conf = dma_channel_get_default_config(chan);
	channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
	channel_config_set_read_increment(&conf, true);
	channel_config_set_write_increment(&conf, false);
	dma_channel_configure(
		chan,           // Channel to be configured
		&conf,          // The configuration we just created
		&uart_get_hw(UART_ID)->dr, //dst      //UART_ID->UARTDR_DATA, //UART_ID->UARTDR.DATA, //UARTDR, //dst, // The initial write address
		buf, //src,     // The initial read address
		len, // Number of transfers; in this case each is 1 byte.
		false //true // Start immediately.
	);
	dma_channel_start(chan);
	dma_channel_wait_for_finish_blocking(chan);

#else*/
	printf("%s", buf);
	//uart_puts(UART_ID, buf);
	//uart_write_blocking(UART_ID, buf, strlen(buf));
//#endif

	va_end(args);
}
//------------------------------------------------------------------------------------------
#ifdef SET_BLE
	//-----------------------------------------------------------------------------
	void initMutex()
	{
	#ifdef SET_MUTEX
		mutex_init(&itMutex);
	#endif
	}
	//-----------------------------------------------------------------------------
	void initLIST(s_recq_t *lst)
	{
		ble_cli_total = 0;
		ble_cli_len = sizeof(ble_dev_t);
		initMutex();
		lst->put = 0;
		for (uint8_t i = 0; i < MAX_BLE_LIST; i++) {
			lst->rec[i].id = i;
			lst->rec[i].cli = NULL;
		}
	}
	//-----------------------------------------------------------------------------
	int8_t putLIST(void *item, s_recq_t *lst)
	{
	int8_t ret = -1;
		if (ble_cli_total < MAX_BLE_LIST) {
	#ifdef SET_MUTEX
			if (mutex_try_enter(&itMutex, &owner_out)) {
	#endif
				if (lst->rec[lst->put].cli == NULL) {
					lst->rec[lst->put].cli = item;
					ret = lst->rec[lst->put].id;
					lst->put++;
					ble_cli_total++;
				}
	#ifdef SET_MUTEX
				mutex_exit(&itMutex);
			}
	#endif
		}
		return ret;
	}
	//-----------------------------------------------------------------------------
	int8_t findLIST(void *item, s_recq_t *lst)
	{
	int8_t ret = -1;

		if (!ble_cli_total) return ret;

	#ifdef SET_MUTEX
		if (mutex_try_enter(&itMutex, &owner_out)) {
	#endif
			ble_dev_t *it = (ble_dev_t *)item;
			for (uint8_t i = 0; i < MAX_BLE_LIST; i++) {
				if (!strcmp(it->mac, lst->rec[i].cli->mac)) {
					ret = lst->rec[i].id;
					memcpy((uint8_t *)item, (uint8_t *)lst->rec[i].cli, ble_cli_len);
					break;
				}
			}
	#ifdef SET_MUTEX
			mutex_exit(&itMutex);
		}
	#endif

		return ret;
	}
	//-----------------------------------------------------------------------------
	int8_t addLIST(void *item, s_recq_t *lst)
	{
	int8_t nrec = -1;

		uint8_t *rc = (uint8_t *)calloc(1, ble_cli_len);
		if (rc) {
			memcpy(rc, (uint8_t *)item, ble_cli_len);
			if ((nrec = putLIST((void *)rc, lst)) >= 0) {
				Report(1, "[%s] Put record to queue OK (id=%d total=%u)\n", __func__, nrec, ble_cli_total);
			} else {
				Report(1, "[%s] Put record to queue error (total=%u)\n", __func__, ble_cli_total);
				free(rc);
			}
		} else {
			devError |= devMem;
		}

		return nrec;
	}
	//
	//-----------------------------------------------------------------------------
	uint8_t prnLIST(s_recq_t *lst)
	{
		if (ble_cli_total) {
			size_t len = ((ble_cli_len + 16) * ble_cli_total) + 64;
			char *st = (char *)calloc(1, len);
			if (st) {
				sprintf(st, "Total audio bluetooth client items in list = %u:\n", ble_cli_total);
				for (uint8_t i = 0; i < ble_cli_total; i++) {
					sprintf(st+strlen(st), "\t[%u] mac:%s name:%s\n", i, lst->rec[i].cli->mac, lst->rec[i].cli->name);
				}
				Report(1, "[%s] %s", __func__, st);
				free(st);
			} else {
				devError |= devMem;
			}
		}
		return ble_cli_total;
	}
	//
#endif
//------------------------------------------------------------------------------------------




