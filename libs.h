/*
 * libs.h
 *
 *  Created on: Aug 16, 2022
 *      Author: Salara
 */

#ifndef LIBS_H_
#define LIBS_H_


#include "hdr.h"

#define _5ms 1
#define _10ms _5ms * 2
#define _15ms _5ms * 3
#define _20ms (_5ms * 4)
#define _25ms (_5ms * 5)
#define _30ms (_10ms * 3)
#define _40ms (_10ms * 4)
#define _50ms (_10ms * 5)
#define _55ms (_50ms + _5ms)
#define _60ms (_10ms * 6)
#define _65ms (_60ms + _5ms)
#define _70ms (_10ms * 7)
#define _75ms (_70ms + _5ms)
#define _80ms (_10ms * 8)
#define _90ms (_10ms * 9)
#define _100ms (_10ms * 10)
#define _110ms (_10ms * 11)
#define _120ms (_10ms * 12)
#define _125ms ((_10ms * 12) + _5ms)
#define _130ms (_10ms * 13)
#define _140ms (_10ms * 14)
#define _150ms (_10ms * 15)
#define _155ms ((_10ms * 15) + _5ms)
#define _160ms (_10ms * 16)
#define _165ms ((_10ms * 16) + _5ms)
#define _170ms (_10ms * 17)
#define _175ms ((_10ms * 17) + _5ms)
#define _180ms (_10ms * 18)
#define _185ms ((_10ms * 18) + _5ms)
#define _190ms (_10ms * 19)
#define _195ms ((_10ms * 19) + _5ms)
#define _200ms (_10ms * 20)
#define _250ms (_10ms * 25)
#define _300ms (_10ms * 30)
#define _350ms (_10ms * 35)
#define _400ms (_10ms * 40)
#define _450ms (_10ms * 45)
#define _500ms (_10ms * 50)
#define _550ms (_10ms * 55)
#define _600ms (_10ms * 60)
#define _650ms (_10ms * 65)
#define _700ms (_10ms * 70)
#define _750ms (_10ms * 75)
#define _800ms (_10ms * 80)
#define _900ms (_10ms * 90)
#define _950ms (_10ms * 95)
#define _1s (_10ms * 100)
#define _1s25 ((_1s * 1) + _250ms)
#define _1s3 ((_1s * 1) + _300ms)
#define _1s5 (_1s + _500ms)
#define _2s (_1s * 2)
#define _2s3 ((_1s * 2) + _300ms)
#define _2s5 ((_1s * 2) + _500ms)
#define _3s (_1s * 3)
#define _4s (_1s * 4)
#define _4s3 ((_1s * 4) + _300ms)
#define _5s (_1s * 5)
#define _6s (_1s * 6)
#define _7s (_1s * 7)
#define _8s (_1s * 8)
#define _9s (_1s * 9)
#define _10s (_1s * 10)
#define _15s (_1s * 15)
#define _20s (_1s * 20)
#define _30s (_1s * 30)



#ifndef MAX_UART_BUF
    #define MAX_UART_BUF 256
#endif



#ifdef SET_WITH_DMA
	extern uint16_t devDMA;
	//extern void iniDMA();
#endif

extern void inc_msCounter();
extern uint32_t get_msCounter();
extern uint32_t get_mstmr(uint32_t hs);
extern bool check_mstmr(uint32_t hs);
extern void set_sec(uint32_t sec);
extern uint32_t get_sec();
extern void inc_sec();
extern uint32_t get_tmr(uint32_t sec);
extern bool check_tmr(uint32_t sec);
extern void errLedOn(bool on);
extern int sec2str(char *st);
extern void Report(const uint8_t addTime, const char *fmt, ...);

#ifdef SET_BLE
	#define MAX_BLE_LIST 8
	//
	#pragma pack(push,1)
	typedef struct q_rec_t {
		int8_t id;
		ble_dev_t cli;
	} q_rec_t;
	#pragma pack(pop)

	#pragma pack(push,1)
	typedef struct s_recq_t {
		uint8_t put;
		q_rec_t rec[MAX_BLE_LIST];
	} s_recq_t;
	#pragma pack(pop)
	//
	void initLIST(s_recq_t *q);
	int8_t putLIST(void *item, s_recq_t *lst);
	int8_t addLIST(void *item, s_recq_t *lst);
	int8_t findLIST(void *item, s_recq_t *lst);
	uint8_t prnLIST(s_recq_t *lst);
	uint8_t getCountLIST();
	//
	#ifdef SET_MUTEX
		void initMutex();
	#endif
	//
#endif


#endif /* LIBS_H_ */
