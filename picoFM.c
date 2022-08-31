#include "hdr.h"
#include "libs.h"

#ifdef SET_SSD1306
	#include "ssd1306.h"
#endif
#ifdef SET_RDA
    #include "rda5807.h"
#endif
#ifdef SET_LCD_UC
	#include "fonts.h"
	#include "UC1609C.h"
#endif
//------------------------------------------------------------------------------------------
//         picotool load -v -x picoFM.uf2 -t uf2
//------------------------------------------------------------------------------------------
enum {
	cmdNone = -1,
	cmdHelp = 0,
	cmdRestart,
	cmdEpoch,
	cmdVer,
	cmdErr,
	cmdUart,
	cmdMute,
	cmdSec,
	cmdTemp
#ifdef SET_ENCODER
	,
	cmdEnc,
	cmdIncFreq,
	cmdDecFreq
#endif
#ifdef SET_RDA
	,
	cmdCfg,
	cmdScan,
	cmdVol,
	cmdBass,
	cmdBand,
	cmdFreq,
	cmdList,
	cmdRds
#endif
};

/**/
enum {
	devOk = 0,
	devMem = 1,
	devTik = 2,
	devQue = 4,
	devUart = 8,
	devI2c = 16
#ifdef SET_WITH_DMA
	,
	devDma = 32
#endif
#ifdef SET_RDA
	,dev_RDA = 64
#endif
};

#ifdef SET_WITH_DMA
	uint16_t devDMA = devDma;
#endif
/**/

//const char *ver = "Version 0.1";
//const char *ver = "Ver.0.2 15.08.22";// add queue for events
//const char *ver = "Ver.0.3 16.08.22";// add devError and function Report()
//const char *ver = "Ver.0.4 16.08.22";// add RTC support
//const char *ver = "Ver.0.4.1 17.08.22";// add RTC support done !
//const char *ver = "Ver.0.5 19.08.22";// add RDA5807 chip (first step)
//const char *ver = "Ver.0.5.1 20.08.22";// add RDA5807 chip (second step)
//const char *ver = "Ver.0.6 21.08.22";// add RDA5807 chip done (with support RDS) !
//const char *ver = "Ver.0.6.1 22.08.22";
//const char *ver = "Ver.0.7 23.08.22";// add support infrared control !!!
//const char *ver = "Ver.0.8 23.08.22";// add support jostic control !!!
//const char *ver = "Ver.0.8.1 24.08.22";
//const char *ver = "Ver.0.9 25.08.22 multicore";// support joystic control in second core now !!!
//const char *ver = "Ver.1.0 26.08.22 multicore";
//const char *ver = "Ver.2.0 27.08.22 multicore";// add display UC1609C instead of SSD1306
//const char *ver = "Ver.2.1 28.08.22 multicore";
//const char *ver = "Ver.2.2 29.08.22 multicore";
//const char *ver = "Ver.2.3 30.08.22 multicore";// add features : read boardID & read temperature sensor on chip
const char *ver = "Ver.2.4 31.08.22 multicore";// support encoder EC11 !!!



volatile static uint32_t epoch = 1661949985;//1661902365;//1661897825;//1661792625;
//1661767566;//1661726088;//1661699652;//1661684619;//1661641164;//1661614899;//1661536565;
//1661463575;//1661459555;//1661371110;//1661344350;//1661285299;//1661258255;//1661193099;//1661096209;
//1661004270;//1660945885;//1660743445;//1660736830;//1660731354;//1660684399;
//1660657998;//1660601220;//1660576465;//1660563510;//1660506862;//1660505693;//1660494699;

volatile uint16_t devError = devOk;
volatile uint16_t last_devError = devOk;
volatile uint32_t seconda = 0;
int tZone = 0;//2;
volatile uint8_t restart = 0;
//
uint8_t rxByte = 0;
uint16_t rxInd = 0;
char rxBuf[MAX_UART_BUF] = {0};
bool uart_enable = true;
//
const char *s_cmds[MAX_CMDS] = {
	"help",
	"restart",
	"epoch:",
	"ver",
	"input_err",
	"uart",
	"mute",
	"sec",
	"temp"
#ifdef SET_ENCODER
	,
	"enc",
	"encinc",
	"encdec"
#endif
#ifdef SET_RDA
	,
	"cfg",
	"scan",
	"vol:",
	"bass:",
	"band:",
	"freq:",
	"list",
	"rds"
#endif
};

char tmp[128];

queue_t evt_fifo;
typedef struct {
	int cmd;
	uint32_t attr;
} evt_t;
const int EVT_FIFO_LENGTH = 16;
bool que_start = false;

bool led = true;
const uint LED_PIN = 29;
const uint ERR_PIN = 8;//14;

const uint16_t all_devErr[MAX_ERR_CODE] = {
	devMem,
	devTik,
	devQue,
	devUart,
	devI2c
#ifdef SET_WITH_DMA
	,devDma
#endif
#ifdef SET_RDA
	,devRDA
#endif
};


#ifdef SET_RDA
	//
	float Freq = 76.0;
	float newFreq = 95.1;
	float lBand = 0.0;
	float rBand = 0.0;
	uint8_t Band = 2;// :2
	uint8_t newBand = 2;
	uint8_t Step = 0;// 100 КГц
	uint8_t newStep = 0;
	uint16_t Chan = 0;
	uint8_t RSSI = 0;
	uint8_t rdaID = 0;
	volatile uint8_t scan = 0;
	volatile uint8_t seek_up = 1;
	uint8_t Volume = 6;//8;
	uint8_t newVolume = 6;//8;
	uint8_t BassBoost = 0;
	uint8_t newBassBoost = 0;
	bool stereo = false;
	uint8_t noMute = 1;
	uint8_t dataRDS[8] = {0};
	bool syncRds = false;
	bool readyRds = false;
	//
	const char *allBands[MAX_BAND] = {
		"87-108 MHz",// (US/Europe)",
		"76-91 MHz", // (Japan)",
		"76-108 MHz",// (world wide)",
		"65-76 MHz"  // (East Europe) or 50-65MHz"
	};
	const step_t allSteps[MAX_STEP] = {
		{0.1, "100"},
		{0.2, "200"},
		{0.05, "50"},
		{0.025, "25"}
	};
	const char *noneStation = "???";
#ifdef RUS_SUPPORT
	static const rec_t list[MAX_LIST] = {
		//Band:3 65-76
		{3,  68.5, "Маяк"},// Маяк
		{3,  72.1, "Шансон"},// Шансон
		//Band:2,1 76-108, 87-108
		{2,  92.8, "Радио DFM"},//Радио DFM в Калининграде
		{2,  93.6, "Радио 7"},// Радио 7
		{2,  94.0, "Комеди Радио"},// Комеди Радио
		{2,  95.1, "Вести ФМ"},// Вести ФМ
		{2,  95.5, "Ретро ФМ"},// Ретро ФМ
		{2,  96.3, "Русское Радио"},// Русское Радио
		{2,  97.0, "Радио Вера"},// Радио Книга
		{2,  97.7, "Серебр.Дождь"},// Серебрянный Дождь
		{2,  98.5, "Радио Энергия"},// Радио Энергия
		{2,  99.5, "Радио Звезда"},// Радио Звезда
		{2, 100.1, "Авто Радио"},// АвтоРадио
		{2, 100.6, "Русский Край"},// Русский Край
		{2, 100.9, "Монте-Карло"},// Монте-Карло
		{2, 101.3, "Наше Радио"},// Наше Радио
		{2, 101.8, "Бизнес ФМ"},// Бизнес ФМ
		{2, 102.5, "Маяк"},// Маяк
		{2, 102.9, "Любимое Радио"},// Любимое Радио
		{2, 103.4, "Студия 21"},// Студия 21
		{2, 103.9, "Радио России"},// Радио России
		{2, 104.5, "Европа Плюс"},// Европа Плюс
		{2, 105.2, "Балтик Плюс"},// Балтик Плюс
		{2, 105.9, "Дорожное Радио"},// Дорожное Радио
		{2, 106.4, "Радио Максим"},// Радио Максим
		{2, 107.2, "Радио КП"}// Комсомольская Правда
	};
#else
	static const rec_t list[MAX_LIST] = {
		//Band:3 65-76
		{3, 68.5, "Majak"},// Маяк
		{3, 72.1, "Shanson"},// Шансон
		//Band:2,1 76-108, 87-108
		{2, 92.8, "Radio DFM"},//Радио DFM в Калининграде
		{2, 93.6, "Radio 7"},// Радио 7
		{2, 94, "Comedy Radio"},// Комеди Радио
		{2, 95.1, "Vesti FM"},// Вести ФМ
		{2, 95.5, "Retro FM"},// Ретро ФМ
		{2, 96.3, "Russian Radio"},// Русское Радио
		{2, 97, "Radio Vera"},// Радио Книга
		{2, 97.7, "Silver Rain"},// Серебрянный Дождь
		{2, 98.5, "Radio Enegry"},// Радио Энергия
		{2, 99.5, "Radio Star"},// Радио Звезда
		{2, 100.1, "Auto Radio"},// АвтоРадио
		{2, 100.6, "Rus.Contry"},// Русский Край
		{2, 100.9, "Monte-Carlo"},// Монте-Карло
		{2, 101.3, "Our Radio"},// Наше Радио
		{2, 101.8, "Business FM"},// Бизнес ФМ
		{2, 102.5, "Majak"},// Маяк
		{2, 102.9, "Love Radio"},// Любимое Радио
		{2, 103.4, "Studio 21"},// Студия 21
		{2, 103.9, "Radio Russia"},// Радио России
		{2, 104.5, "Europe+"},// Европа Плюс
		{2, 105.2, "Baltic+"},// Балтик Плюс
		{2, 105.9, "Road Radio"},// Дорожное Радио
		{2, 106.4, "Radio Maxim"},// Радио Максим
		{2, 107.2, "Radio KP"}// Комсомольская Правда
	};
#endif
	uint16_t listSize = 0;
	//
	#ifdef SET_RDS

		#pragma pack(push,1)
		typedef struct {
			uint16_t blockA;
			uint16_t blockB;
			uint16_t blockC;
			uint16_t blockD;
		} blocks_t;
		#pragma pack(pop)

		const char *namePTy[MAX_SPTY] = {
				"No program type or undefined",
				"News",
				"Current affairs",
				"Information",
				"Sport",
				"Education",
				"Drama",
				"Culture",
				"Science",
				"Varied",
				"Pop music",
				"Rock music",
				"Easy listening",
				"Light classical",
				"Serious classical",
				"Other music",
				"Weather",
				"Finance",
				"Children’s programs",
				"Social affairs",
				"Religion",
				"Phone-in",
				"Travel",
				"Leisure",
				"Jazz music",
				"Country music",
				"National music",
				"Oldies music",
				"Folk music",
				"Documentary",
				"Alarm test",
				"Alarm"
		};
		uint16_t sID = 0; // ID радиостанции
		uint16_t MaybeThisIDIsReal = 0; // Предыдущее значение ID
		uint8_t IDRepeatCounter = 0; // Счетчик повторений ID
		uint8_t errLevelB, errLevelC, errLevelD, groupType, groupVer;
		uint8_t PTy = 255;
		bool PTy_printed = false;
		char PSName[9]; // Значение PSName
		char PSName_prev[9];
		uint8_t PSNameUpdated = 0; // Для отслеживания изменений в PSName
	#endif
#endif


#ifdef SET_IR

	#define IR_PIN 14
	#define BIT_LENGTH 32
	#define MAX_IRED_KEY 21

	enum {
		key_ch_minus = 0,
		key_ch,
		key_ch_plus,
		key_left,
		key_right,
		key_sp,
		key_minus,
		key_plus,
		key_eq,
		key_100,
		key_200,
		key_0,
		key_1,
		key_2,
		key_3,
		key_4,
		key_5,
		key_6,
		key_7,
		key_8,
		key_9
	};

	typedef struct {
		char name[8];
		uint32_t code;
	} one_key_t;

	const one_key_t keyAll[MAX_IRED_KEY] = {
			{"irCH-",   0xffa25d},
			{"irCH",    0xff629d},
			{"irCH+",   0xffe21d},
			{"irLEFT",  0xff22dd},
			{"irRIGHT", 0xFF02FD},
			{"irSP",    0xFFC23D},
			{"ir-",     0xFFE01F},
			{"ir+",     0xFFA857},
			{"irEQ",    0xFF906F},
			{"ir100+",  0xFF9867},
			{"ir200+",  0xFFB04F},
			{"ir0",     0xFF6897},
			{"ir1",     0xFF30CF},
			{"ir2",     0xFF18E7},
			{"ir3",     0xFF7A85},
			{"ir4",     0xFF10EF},
			{"ir5",     0xFF38C7},
			{"ir6",     0xFF5AA5},
			{"ir7",     0xFF42BD},
			{"ir8",     0xFF4AB5},
			{"ir9",     0xFF52AD},
	};

	uint cont_zero = 0;
	uint cont_one = 0;
	int cont_decode = BIT_LENGTH - 1;
	long decode = 0;
	bool flag_decode = 0;
	evt_t ei = {-1, 0};
#endif

#ifdef SET_JOYSTIC
	#define jKEY_PIN 15
	#define corX_PIN 26
	#define corY_PIN 27
	#define MIN_VAL 30
	#define MAX_VAL 4000
	#define MAX_ADC_BUF 8

	#pragma pack(push,1)
	typedef struct {
		uint8_t counter;
		uint16_t val[MAX_ADC_BUF];
	} adc_chan_t;
	#pragma pack(pop)

	adc_chan_t chanX;
	adc_chan_t chanY;

	//uint32_t start_jkey = 0;
	bool joy = false;
#endif

#if defined(SET_ENCODER) || defined(SET_JOYSTIC) || defined(SET_IR)
	#define LED_CMD_PIN 16
	//
	uint32_t start_jkey = 0;
	uint32_t cmd_tmr = 0;
	//
	void cmdLedOn()
	{
		//gpio_put(LED_CMD_PIN, cled);
		cmd_tmr = get_mstmr(_150ms);
	}
	//
	static inline void put_pixel(uint32_t pixel_grb)
	{
	    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
	}
#endif


#ifdef SET_LCD_UC
	FontDef_t *mfnt = NULL;
	FontDef_t *lfnt = NULL;
	FontDef_t *hfnt = NULL;
#endif


#ifdef SET_ENCODER
	#define ENC_PIN 12
	#define ENC_PIN_A 10
	#define ENC_PIN_B 11

	const int16_t MIN_ENC_VALUE = -16383;
	const int16_t MAX_ENC_VALUE = 16383;

	volatile int16_t ec_counter = 0;
	volatile int16_t ec_last_counter = 0;
	bool fix_freq = false;
#endif


//*******************************************************************************************
//*******************************************************************************************
//*******************************************************************************************

//-------------------------------------------------------------------------------------------

#if defined(SET_JOYSTIC) || defined(SET_ENCODER)
	//----------------------------------------------------------------------------------------
	void gpio_callback(uint gpio, uint32_t events)
	{
		if (!que_start) {
			ec_counter = ec_last_counter = 0;
			return;
		}

		if ((gpio == jKEY_PIN) || (gpio == ENC_PIN)) {
			if (!gpio_get(gpio)) {
				start_jkey = get_mstmr(_55ms);
			} else {
				if (start_jkey) {
					if (check_mstmr(start_jkey)) {
						start_jkey = 0;
						cmdLedOn();
						evt_t e;
						if (gpio == jKEY_PIN) {
							seek_up = 1;
							e.cmd = cmdScan;
						} else {
							e.cmd = cmdEnc;
						}
						if (!queue_try_add(&evt_fifo, &e)) devError |= devQue;
					}
				}
			}
		} else if (gpio == ENC_PIN_A) {
			if (!fix_freq) {
				if (gpio_get(ENC_PIN_B)) ec_counter++;
									else ec_counter--;
				if (ec_last_counter != ec_counter) {
					int cd = cmdNone;
					if ((ec_last_counter == MIN_ENC_VALUE) && (ec_counter == MAX_ENC_VALUE)) cd = cmdDecFreq;//dec
					else
					if ((ec_last_counter == MAX_ENC_VALUE) && (ec_counter == MIN_ENC_VALUE)) cd = cmdIncFreq;//inc
					else
					if (ec_last_counter < ec_counter) cd = cmdIncFreq;//inc
					else
					if (ec_last_counter > ec_counter) cd = cmdDecFreq;//dec
					ec_last_counter = ec_counter;
					if (cd != cmdNone)	{
						if (cd == cmdIncFreq) {
							newFreq = Freq + allSteps[Step].freq;
						} else {
							newFreq = Freq - allSteps[Step].freq;
						}
						evt_t e = {cmdFreq, 0};
						memcpy(&e.attr, &newFreq, sizeof(e.attr));
						if (!queue_try_add(&evt_fifo, &e)) devError |= devQue;
					}
				}
			}
		}
	}
#endif
	//----------------------------------------------------------------------------------------
#ifdef SET_JOYSTIC
	uint8_t adcAddVal(adc_chan_t *chan, uint16_t val)
	{
	int8_t i;

		if (!chan->counter) {
			for (i = 0; i < MAX_ADC_BUF; i++) chan->val[i] = val;
			chan->counter = MAX_ADC_BUF;
		} else {
			for (i = MAX_ADC_BUF - 2; i >= 0; i--) chan->val[i + 1] = chan->val[i];
			chan->val[0] = val;
			if (chan->counter < MAX_ADC_BUF) chan->counter++;
		}
	    return chan->counter;
	}
	//----------------------------------------------------------------------------------------
	void joystik_task()// Loop for check event from jostic
	{
		joy = true;
		uint32_t sumAdc = 0;
		adc_select_input(0);
		uint32_t valX = adc_read();
		uint32_t last_valX = valX;
		adc_select_input(1);
		uint32_t valY = adc_read();
		uint32_t last_valY = valY;
		evt_t ev;
		uint8_t yes = 0;

		Report(1, "Start '%s' function on Core1\n", __func__);

		uint32_t jtmr = get_mstmr(_10ms);

		while (!restart) {

			if (check_mstmr(jtmr)) {

				jtmr = get_mstmr(_20ms);

				adc_select_input(0);
				valX = adc_read();
				if (adcAddVal(&chanX, valX) == MAX_ADC_BUF) {//в окне накоплено MAX_ADC_BUF выборок -> фильтрация !
					sumAdc = 0;
					for (int8_t j = 0; j < MAX_ADC_BUF; j++) sumAdc += chanX.val[j];
					valX = sumAdc / MAX_ADC_BUF;
					if ((valX < MIN_VAL) || (valX > MAX_VAL)) {
						cmdLedOn();
						//
						if (valX < MIN_VAL) seek_up = 1;
						else
						if (valX > MAX_VAL) seek_up = 0;
						//
						ev.cmd = cmdList;
						ev.attr = seek_up;
						if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
						jtmr = get_mstmr(_120ms);
					}
				}
				//
				adc_select_input(1);
				valY = adc_read();
				if (adcAddVal(&chanY, valY) == MAX_ADC_BUF) {//в окне накоплено MAX_ADC_BUF выборок -> фильтрация !
					sumAdc = 0;
					for (int8_t j = 0; j < MAX_ADC_BUF; j++) sumAdc += chanY.val[j];
					valY = sumAdc / MAX_ADC_BUF;
					if ((valY < MIN_VAL) || (valY > MAX_VAL)) {
						yes = 0;
						if (valY < MIN_VAL) {
							if (Volume < 15) {
								newVolume = Volume + 1;
								yes = 1;
							}
						} else if (valY > MAX_VAL) {
							if (Volume) {
								newVolume = Volume - 1;
								yes = 1;
							}
						}
						if (yes) {
							cmdLedOn();
							ev.cmd = cmdVol;
							ev.attr = newVolume;
							if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
							jtmr = get_mstmr(_120ms);
						}
					}
				}
			}
		}//while(1)

		Report(1, "Closed '%s' function.\n", __func__);
		joy = false;
	}
	//----------------------------------------------------------------------------------------
#endif
//-------------------------------------------------------------------------------------------

#ifdef SET_RDA
//------------------------------------------------------------------------------------------
void showCfg()
{
	char *st = (char *)calloc(1, MAX_UART_BUF << 2);
	if (st) {
		for (int i = 0; i < MAX_LIST; i++) {
			sprintf(st+strlen(st), "%u:%.1f:%.*s\r\n", list[i].band, list[i].freq, strlen(list[i].name), list[i].name);
		}
		Report(0, "%s", st);
		free(st);
	}
}
//-------------------------------------------------------------------------------------------
const char *nameStation(float fr, int8_t *ind)
{
int8_t ik = -1;
//uint32_t fin = fr * 10;
//uint32_t fcon;

	for (int8_t i = 0; i < MAX_LIST; i++) {
		//fcon = list[i].freq * 10;
		//if (fcon == fin) {
		if (list[i].freq == fr) {
			ik = i;
			break;
		}
	}
	if (ind) *ind = ik;

	if (ik != -1) return list[ik].name;
			 else return noneStation;
}
//-------------------------------------------------------------------------------------------
float getNextList(float fr, uint8_t up, uint8_t *band)
{
float ret = fr;
int8_t ik = -1;

	for (int8_t i = 0; i < MAX_LIST; i++) {
		if (list[i].freq == fr) {
			ik = i;
			break;
		}
	}
	if (ik != -1) {
		if (up) {
			if (++ik == MAX_LIST) ik = 0;
		} else {
			if (ik != 0) ik--; else ik = MAX_LIST - 1;
		}
	} else {
		if (up) {// seek_up
			for (int8_t i = ik; i < MAX_LIST; i++) {
				if (list[i].freq > fr) {
					ik = i;
					break;
				}
			}
		} else {// seek_down
			for (int8_t i = ik; i <= 0; i--) {
				if (list[i].freq < fr) {
					ik = i;
					break;
				}
			}
		}
		if (ik == -1) ik = 0;
	}
	ret = list[ik].freq;
	*band = list[ik].band;
	//Report(1, "[%s] up=%u ik=%d, fr=%.1f ret=%.1f band=%u\r\n", __func__, up, ik, fr, ret, *band);

	return ret;
}
//-------------------------------------------------------------------------------------------
#ifdef SET_RDS
//-------------------------------------------------------------------------------------------
void MJDDecode(unsigned long MJD, uint16_t *y, uint8_t *m, uint8_t *d)
{
unsigned long L = 2400000 + MJD + 68570;
unsigned long N = (L * 4) / 146097;

	L -= ((146097.0 * N + 3) / 4);
	uint16_t year = 4000 * (L + 1) / 1461001;
	L -= (1461 * year / 4 + 31);
	uint8_t month = 80.0 * L / 2447.0;
	uint8_t day = L - 2447 * month / 80;
	L = month / 11;
	month = month + 2 - 12 * L;
	year = 100 * (N - 49) + year + L;
	*y = year;
	*m = month;
	*d = day;
}
//-------------------------------------------------------------------------------------------
void rds_init()
{
//
	sID = 0;
	MaybeThisIDIsReal = 0;
	IDRepeatCounter = 0;
	errLevelB = 0, errLevelC = 0, errLevelD = 0, groupType = 0, groupVer = 0;
	PTy = 255;
	PTy_printed = false;
	memset(PSName, 0, sizeof(PSName)); // Значение PSName
	memset(PSName_prev, 0, sizeof(PSName));
	PSNameUpdated = 0;
//
}
//-------------------------------------------------------------------------------------------
#endif

#endif

//-------------------------------------------------------------------------------------------
static char *errName(uint16_t err)
{
	switch (err) {
		case devMem:// = 1,
			return "devMem";
		case devTik:// = 2,
			return "devTik";
		case devQue:// = 4,
			return "devQue";
		case devUart://= 8,
			return "devUart";
		case devI2c:// = 0x10,
			return "devI2c";
#ifdef SET_WITH_DMA
		case devDma:// = 0x20,
			return "devDma";
#endif
#ifdef SET_RDA
		case devRDA:// = 0x40
			return "devRDA";
#endif
	}

	return "???";
}
//------------------------------------------------------------------------------------------
bool repeating_timer_callback(struct repeating_timer *t)
{
	inc_msCounter();

	if (!que_start) return true;

#if defined(SET_ENCODER) || defined(SET_JOYSTIC) || defined(SET_IR)
	if (cmd_tmr) {
		put_pixel(rand() % 10 ? 0 : 0xffff0000);//0xffffffff);//LED_CMD_PIN active
	}
#endif

#ifdef SET_ENCODER
	if (!(get_msCounter() % _100ms)) {
		/*new_value = quadrature_encoder_get_count(pioe, sme);
		if (new_value != old_value) {
			delta = new_value - old_value;
			old_value = new_value;
			//if (delta != old_delta) {
			//old_delta = delta;
			evt_t ev = {
				.cmd = cmdEnc,
				.attr = delta
			};
			if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
		}*/
		/*evt_t ev = {
			.cmd = cmdEnc,
			.attr = ec_a_cnt
		};
		if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;*/
	}
#endif

	if (!(get_msCounter() % _1s)) {// 1 seconda
		seconda++;
		evt_t evt = {
			.cmd = cmdSec,
			.attr = seconda
		};
		if (!queue_try_add(&evt_fifo, &evt)) devError |= devQue;

		led = !led;
		gpio_put(LED_PIN, led);
	}

    return true;
}
//------------------------------------------------------------------------------------------
void uart_rx_callback()
{
    if (uart_is_readable(UART_ID)) {
    	rxByte = uart_getc(UART_ID);
    	//
    	rxBuf[rxInd++] = (char)rxByte;
        if (rxByte == 0x0a) {//end of line
        	rxBuf[--rxInd] = '\0';
        	//
        	int i;
        	evt_t evt = {cmdNone, 0};
        	if (strlen(rxBuf) > 2) {
        		for (i = 0; i < MAX_CMDS; i++) {
        			if (!strncmp(rxBuf, s_cmds[i], strlen(s_cmds[i]))) {
        				char *uk = rxBuf + strlen(s_cmds[i]);
        				evt.cmd = -1;
        				evt.attr = 0;
        				switch (i) {
        					case cmdHelp:    //"help"
        					case cmdVer:     //"ver"
        					case cmdRestart: //"restart" -> restart = 1;
        					case cmdUart:    //"uart" on/off
        					case cmdMute:    //"mute"
        					case cmdTemp:    //"temp"
        						evt.cmd = i;
        					break;
#ifdef SET_RDA
        					case cmdCfg:     //"cfg"
        						evt.cmd = i;
        					break;
        					case cmdBand:    //"band:2"
        						if (strlen(uk) >= 1) {
        							newBand = atol(uk);
        							evt.cmd = i;
        						}
        					break;
        					case cmdBass:
        						if (strlen(uk) >= 1) {
        							newBassBoost = (uint8_t)atol(uk);
        							evt.cmd = i;
        						}
        					break;
        					case cmdScan:    //"scan"
        					case cmdList:    //"list"
        						seek_up = 1;
        						evt.cmd = i;
        						char *uki = strchr(uk, ':');
        						if (uki) {
        							if ((*(char *)(uki + 1) == '0') || strstr(uki + 1, "down")) seek_up = 0;
        						}
        					break;
        					case cmdFreq:    //"freq:95.1"
        						if (strlen(uk) >= 2) {
        							newFreq = (float)atof(uk);
        							if (newFreq != Freq) {
        								evt.cmd = i;
        							}
        						}
        					break;
        					case cmdVol:     //"vol:0..15"
        						if (strlen(uk) >= 1) {
        							uint8_t nv = Volume;
        							if (strstr(uk, "up")) {
        								nv++;
        							} else if (strstr(uk, "down")) {
        								nv--;
        							} else {
        								nv = (uint8_t)atol(uk);
        							}
        							if ((nv >= 0) && (nv <= 15)) {
        								newVolume = nv;
        								evt.cmd = i;
        							}
        						}
        					break;
	#ifdef SET_RDS
							case cmdRds:     //"rds"
								evt.cmd = i;
							break;
	#endif
#endif
        					case cmdEpoch:   //"epoch:1657191323"
        						if (strlen(uk) >= 10) {
        							char *uki = strchr(uk, ':');
        							if (uki) {
        								tZone = (uint8_t)atol(uki + 1);
        								*uki = '\0';
        							} else {
        								tZone = 0;
        							}
        							epoch = (uint32_t)atol(uk);
        							set_sec(epoch);
        							evt.cmd = i;
        							evt.attr = epoch;
        						}
        					break;
        				}
        				break;
        			}
        		}
        		if (evt.cmd == cmdNone) evt.cmd = cmdErr;
        		if (!queue_try_add(&evt_fifo, &evt)) devError |= devQue;
        	}
        	//
        	rxInd = 0;
        	*rxBuf = '\0';
        }
        //
    } else {
    	devError |= devUart;
    }


}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

#ifdef SET_IR

//------------------------------------------------------------------------------------------
bool read_pulse_cb(struct repeating_timer *t)
{
    if (gpio_get(IR_PIN)) {
        if (!flag_decode && cont_zero > 15 && cont_decode == BIT_LENGTH - 1) flag_decode = 1;
        cont_zero = 0;
        if (cont_one <= 21) cont_one++;
    } else {
        if (flag_decode){
            if (cont_one == 1) {
                cont_decode--;
                //printf("0");
            } else if(cont_one >= 2 && cont_decode != BIT_LENGTH - 1) {
                if (!decode) cont_decode = 23;
                if (cont_decode < 24) decode += pow(2, cont_decode);
                //printf("1");
                cont_decode--;
            }
        }
        cont_zero++;
        cont_one = 0;
    }
    if (cont_decode < 0 || cont_one > 20 || cont_zero > 20) {
        flag_decode = 0;
        cont_decode = BIT_LENGTH - 1;
        if (decode) {
        	ei.cmd = decode;
        	if (!queue_try_add(&evt_fifo, &ei)) devError |= devQue;
        }
        decode = 0;
        cont_zero = cont_one = 0;
    }

    return true;
}
//------------------------------------------------------------------------------------------
int ir_parse(uint32_t value, int *idx)
{
int ird = cmdNone;

int8_t kid = -1;

	for (int8_t i = 0; i < MAX_IRED_KEY; i++) {
		if (value == keyAll[i].code) {
			kid = i;
			cmdLedOn();
			break;
		}
	}
	*idx = kid;
	if (kid == -1) {
		cmd_tmr = 0;
		return ird;
	}

	switch (kid) {
		case key_ch:
			ird = cmdRestart;
		break;
		case key_ch_plus:
			seek_up = 1;
			ird = cmdScan;
		break;
		case key_ch_minus:
			seek_up = 0;
			ird = cmdScan;
		break;
		case key_minus:
			if (Volume) {
				newVolume = Volume - 1;
				ird = cmdVol;
			}
		break;
		case key_plus:
			if (Volume < 15) {
				newVolume = Volume + 1;
				ird = cmdVol;
			}
		break;
		case key_left:
			seek_up = 0;
			ird = cmdList;
		break;
		case key_right:
			seek_up = 1;
			ird = cmdList;
		break;
		case key_eq:// enable/disable print via uart
			ird = cmdMute;
		break;
		case key_sp:
			newBassBoost = (BassBoost + 1)  & 1;
			ird = cmdBass;
		break;
		case key_100://bandUp();
			if (Band < MAX_BAND) {
				newBand = Band + 1;
				ird = cmdBand;
			}
		break;
		case key_200://bandDown();
			if (Band) {
				newBand = Band - 1;
				ird = cmdBand;
			}
		break;
		case key_0:
		case key_1:
		case key_2:
		case key_3:
		case key_4:
		case key_5:
		case key_6:
		case key_7:
		case key_8:
		case key_9:
			newFreq = list[kid - key_0 + 2].freq;//for band=2 only !!!
			ird = cmdFreq;
		break;
	}

	return ird;
}
#endif
//------------------------------------------------------------------------------------------
float read_onboard_temperature(const char unit)
{
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C') {
        return tempC;
    } else if (unit == 'F') {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}
//------------------------------------------------------------------------------------------
int main() {

    stdio_init_all();

    //------------------------- GPIO init --------------------------------
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(ERR_PIN);
    gpio_set_dir(ERR_PIN, GPIO_OUT);

#if defined(SET_JOYSTIC) || defined(SET_ENCODER)
    gpio_init(jKEY_PIN);
    gpio_set_dir(jKEY_PIN, GPIO_IN);
    gpio_pull_up(jKEY_PIN);
    gpio_set_irq_enabled_with_callback(jKEY_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_init(ENC_PIN);
    gpio_set_dir(ENC_PIN, GPIO_IN);
    gpio_pull_up(ENC_PIN);
    gpio_set_irq_enabled_with_callback(ENC_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
#endif


    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    float temperature = read_onboard_temperature('C');


#if defined(SET_ENCODER) || defined(SET_JOYSTIC) || defined(SET_IR)
    PIO pio = pio0;
    int sm = 0;
    ws2812_program_init(pio, sm, pio_add_program(pio, &ws2812_program), LED_CMD_PIN, 800000, true);
#endif

#ifdef SET_IR
    gpio_init(IR_PIN);
    gpio_set_dir(IR_PIN, GPIO_IN);
    gpio_pull_up(IR_PIN);
    struct repeating_timer read_timer;
    add_repeating_timer_us(-526, read_pulse_cb, NULL, &read_timer);//-526
#endif
    //--------------------------------------------------------------------

    int idx = -1;

    for (int8_t i = 0; i < 4; i++) {
    	errLedOn(true);
    	sleep_ms(128);
    	errLedOn(false);
    	sleep_ms(128);
    }

    //-------------------------- uart0 config ----------------------------
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, uart_rx_callback);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);
    //--------------------------------------------------------------------

#ifdef SET_WITH_DMA
    iniDMA();
#endif


    //--------------------- i2c master config ----------------------------
    bi_decl(bi_2pins_with_func(I2C_SDA_PIN, I2C_SCL_PIN, GPIO_FUNC_I2C));
    bi_decl(bi_program_description("picoFM Radio"));
    i2c_init(portRDA, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    //------------------- Initialize RTC module --------------------------
    rtc_init();
    sleep_ms(250);
    set_sec(epoch + 1);
    sleep_ms(250);
    //--------------------------------------------------------------------

    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    sprintf(tmp, "0x");
    for (int8_t i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
    	sprintf(tmp+strlen(tmp), "%02X", board_id.id[i]);
    }
    //
    Report(0,"\n");
    Report(1, "Start picoRadio app %s (BoardID:%s temp:%.02f C)\n", ver, tmp, temperature);//uart_puts(UART_ID, "Hello, UART!\n");

    //----------------- Initialize queue for events ----------------------
    queue_init(&evt_fifo, sizeof(evt_t), EVT_FIFO_LENGTH);
    Report(1, "Create queue for %d events OK\n", EVT_FIFO_LENGTH);
    que_start = true;

    //---------------- Setup and start periodic timer --------------------
    struct repeating_timer timer;
    int32_t period = 5;//10;
    if (add_repeating_timer_ms(period * -1, repeating_timer_callback, NULL, &timer)) {
    	Report(1, "Start timer with %lu ms period.\n", period);
    } else {
    	devError |= devTik;
    }
    //--------------------------------------------------------------------

#ifdef SET_ENCODER
    	gpio_init(ENC_PIN_A);
        gpio_set_dir(ENC_PIN_A, GPIO_IN);
        gpio_pull_up(ENC_PIN_A);
        gpio_set_irq_enabled_with_callback(ENC_PIN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        gpio_init(ENC_PIN_B);
        gpio_set_dir(ENC_PIN_B, GPIO_IN);
        gpio_pull_up(ENC_PIN_B);

        bool prnFixFreq = false;
#endif


    char stz[64];
    char stx[64];
#ifdef SET_SSD1306
    //-------------- Initialize ssd1306 display -------------------
    ssd1306_init();
    ssd1306_on(true);
    ssd1306_pattern();
    ssd1306_clear();
    ssd1306_contrast(0xff);//0xff or 0x00
    strncpy(stx, ver, OLED_WIDTH/FONT_WIDTH - 1);
    mkLineCenter(stx, FONT_WIDTH);
    ssd1306_text_xy(stx, 1, 4, false);
    //-------------------------------------------------------------
#else

	#ifdef SET_LCD_UC

    	spi_init(portSPI, 12000 * 1000);//set SCK to 12Mhz !
    	gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
        gpio_set_function(LCD_SCK_PIN, GPIO_FUNC_SPI);
        bi_decl(bi_2pins_with_func(LCD_MOSI_PIN, LCD_SCK_PIN, GPIO_FUNC_SPI));
        gpio_init(LCD_DC_PIN);
        gpio_set_dir(LCD_DC_PIN, GPIO_OUT);
        gpio_put(LCD_DC_PIN, 0);//for send command
        gpio_init(LCD_RST_PIN);
        gpio_set_dir(LCD_RST_PIN, GPIO_OUT);
        gpio_put(LCD_RST_PIN, 1);//no reset

		#ifdef FONT_6x8
  			mfnt = &Font_6x8;
		#endif
		#ifdef FONT_7x9
  			lfnt = &Font_7x9;
		#endif
		#ifdef FONT_11x18
  			hfnt = &Font_11x18;
		#endif
    	//
    	UC1609C_init();
    	UC1609C_enable(1);
    	UC1609C_contrast(30);
    	UC1609C_clearDisplay();
    	//
    	int dl = sprintf(tmp, "%s", ver);
    	if (dl > (UC1609C_WIDTH / mfnt->FontWidth - 1)) {
    		dl = UC1609C_WIDTH / mfnt->FontWidth - 1;
    		tmp[dl] = '\0';
    	}
    	uint16_t x = ((UC1609C_WIDTH - (mfnt->FontWidth * dl)) >> 1);
    	UC1609C_Print(x, UC1609C_HEIGHT - mfnt->FontHeight, tmp, mfnt, 0, FOREGROUND);
    	//
    	UC1609C_DrawFilledRectangle(0, 0, UC1609C_WIDTH - 1, hfnt->FontHeight - 1, FOREGROUND);
    	UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    	UC1609C_update();
    	//
    	uint8_t line1 = hfnt->FontHeight + 1;
    	uint8_t line2 = line1 + lfnt->FontHeight;
    	uint8_t line3 = line2 + lfnt->FontHeight;
    	uint8_t line4 = line3 + lfnt->FontHeight;
    	uint8_t line5 = line4 + lfnt->FontHeight + 1;
    	//
    	bool flag_ver = false;
    	uint8_t tmr_ver = 0;
	#endif
#endif
    evt_t ev;
    int evt;
    int queCnt = 0;
    int next_evt = cmdNone;

#ifdef SET_RDA
    rda5807_delay(500);
    char st[64];
    char sta[64];
    char stb[64];
    char stn[64];
    rdaID = rda5807_init(&Freq, Band, Step);
    rda5807_delay(500);

    rda5807_SetVolume(Volume);
    rda5807_SetBassBoost(BassBoost);
  	rda5807_Get_ChanRssiFlag(&Chan, &RSSI, &stereo);
    if (rdaID == 0x58) {
    	sprintf(st, "RDA5807 cID:0x%x", rdaID);
    } else {
    	sprintf(st, "Unknown cID:0x%x", rdaID);
    }
    Report(1, "%s\n", st);
#ifdef SET_SSD1306
    mkLineCenter(st, FONT_WIDTH);
    ssd1306_text_xy(st, 1, 4, false);
#endif
    rda5807_delay(100);

	#ifdef SET_RDS
    	blocks_t *blk = NULL;
    	const uint64_t rdsWait = _30ms;
    	bool rdsFlag = false;
    	uint64_t rdsTime = 0;
    	//
    	ev.cmd = cmdRds;
    	ev.attr = 0;
    	if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
	#endif

    if (rdaID == RDA5807_CHIP) {
    	ev.cmd = cmdFreq;
    	ev.attr = 0;
    	if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    }
#endif

#ifdef SET_JOYSTIC
    multicore_launch_core1(joystik_task);
#endif


    uint32_t attr = 0;

    while (!restart) {

    	queCnt = queue_get_level(&evt_fifo);
    	if (queCnt) {
    		if (queue_try_remove(&evt_fifo, &ev)) {
    			idx = -1;
    			evt = ev.cmd;
    			attr = ev.attr;
    			if ((evt > cmdNone) && (evt < cmdSec)) {
    				Report(1, "[que:%d] cmd:%d attr:%lu\n", queCnt, ev.cmd, ev.attr);
#ifdef SET_SSD1306
    				sprintf(stz, "cmd:'%s'", s_cmds[evt]);
    				mkLineCenter(stz, FONT_WIDTH);
    				ssd1306_clear_lines(4, 1);
    				ssd1306_text_xy(stz, 1, 4, false);
#endif
#ifdef SET_LCD_UC
    				sprintf(stz, "cmd : %s", s_cmds[evt]);
    				mkLineCenter(stz, mfnt->FontWidth);
    				UC1609C_Print(1, line5, stz, mfnt, 0, FOREGROUND);
    				UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    				UC1609C_update();
    				flag_ver = true;
    				tmr_ver = 10;
#endif
    			} else {
#ifdef SET_IR
    				if (evt > cmdRds) {
    					evt = ir_parse(ev.cmd, &idx);
    					if (evt == -1) {
    						sprintf(tmp, "CODE:0x%X", ev.cmd);
    					} else {
    						sprintf(tmp, "irKEY: %s", keyAll[idx].name);
    					}
    					Report(1, "[que:%u] %s\r\n", queCnt, tmp);
    				}
#endif
    			}
    			switch (evt) {
    				case cmdTemp:
    					adc_select_input(4);
    					temperature = read_onboard_temperature('C');
    					Report(1, "[que:%u] onchip temperature:%.02f C\n", queCnt, temperature);
#ifdef SET_LCD_UC
    					sprintf(stz, "Onchip temperature:%.02f C", temperature);
    					mkLineCenter(stz, mfnt->FontWidth);
    					UC1609C_Print(1, line5, stz, mfnt, 0, FOREGROUND);
    					UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    					UC1609C_update();
    					flag_ver = true;
    					tmr_ver = 10;
#endif
    				break;
#ifdef SET_ENCODER
    				case cmdEnc:
    					if (fix_freq) fix_freq = false; else fix_freq = true;
    					ec_counter = ec_last_counter = 0;
    					if (fix_freq) {
    						strcpy(stz, "Freq change deny");
    					} else {
    						strcpy(stz, "Freq change allow");
    					}
    					Report(1, "[que:%u] %s\n", queCnt, stz);
	#ifdef SET_LCD_UC
    					mkLineCenter(stz, mfnt->FontWidth);
    					UC1609C_Print(1, line5, stz, mfnt, 0, FOREGROUND);
    					UC1609C_update();
    					flag_ver = true;
    					tmr_ver = 10;
	#endif
    				break;
    				case cmdIncFreq:
    				case cmdDecFreq:
    					newFreq = Freq;
    					if (evt == cmdIncFreq) {
    						Report(1, "[que:%u] incFreq %.3f + step %s МГц = %.3f\n", queCnt, newFreq, allSteps[Step].name, newFreq + allSteps[Step].freq);
    						newFreq += allSteps[Step].freq;
    					} else {
    						Report(1, "[que:%u] decFreq %.3f - step %s МГц = %.3f\n", queCnt, newFreq, allSteps[Step].name, newFreq - allSteps[Step].freq);
    						newFreq -= allSteps[Step].freq;
    					}
    					ev.cmd = cmdFreq;
    					if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    				break;
#endif
    				case cmdErr:
    					Report(1, "Error input\n");
    				break;
    				case cmdUart:
    					sprintf(tmp, "[que:%u] ", queCnt);
    					if (!uart_enable)
    						strcat(tmp, "Enable");
    					else
    						strcat(tmp, "Disable");
    					Report(1, "%s uart\n", tmp);
    					if (uart_enable) uart_enable = false; else uart_enable = true;
    				break;
#ifdef SET_RDA
    				case cmdCfg:
    					showCfg();
    				break;
    				case cmdMute:
    					noMute = (~noMute) & 1;
    					rda5807_Set_Mute(noMute);
	#ifdef SET_SSD1306
    					if (noMute)
    						sprintf(st, "Bas:%u Vol:%u", BassBoost, Volume);
    					else
    						sprintf(st, "Bas:%u Vol:%u M", BassBoost, Volume);
    					mkLineCenter(st, FONT_WIDTH);
    					ssd1306_clear_lines(4, 1);
    					ssd1306_text_xy(st, 1, 4, false);
	#endif
	#ifdef SET_LCD_UC
    					sprintf(sta, "BASS:%u", BassBoost);
    					sprintf(stb, "VOLUME:%u", Volume);
    					if (!noMute) strcat(stb, " Mute");
    					mkLineWidth(sta, stb, lfnt->FontWidth);
    					showLine(sta, line2, lfnt, true);
	#endif
    					Report(1, "[que:%u] set Mute to %u\r\n", queCnt, (~noMute) & 1);
    				break;
    				case cmdList:
    					next_evt = cmdFreq;
    					newFreq = getNextList(Freq, seek_up, &newBand);
    					if (newBand == Band) {
    						Report(1, "Band = newBand = %u -> goto set newFreq to %.3f (up = %u)\n", newBand, newFreq, seek_up);
    						ev.cmd = cmdFreq;
    					} else {
    						Report(1, "Band = %u -> goto set newBand to %u (newFreq to %.3f up = %u)\n", Band, newBand, newFreq, seek_up);
    						ev.cmd = cmdBand;
    					}
    					if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    				break;
    				case cmdBand:
    					if (newBand != Band) {
    						Band = newBand;
    						if (!rda5807_Set_Band(Band)) {
	#ifdef SET_SSD1306
    							sprintf(stb, "FM Band:%s", allBands[Band]);
    							mkLineCenter(stb, FONT_WIDTH);
    							ssd1306_clear_lines(4, 1);
    							ssd1306_text_xy(stb, 1, 4, false);
	#endif
	#ifdef SET_LCD_UC
    							sprintf(stb, "FM Band:%s", allBands[Band]);
    							mkLineCenter(stb, lfnt->FontWidth);
    							showLine(stb, line4, lfnt, true);
	#endif
    							Report(1, "[que:%u] set new band=%u '%s'\n", queCnt, Band, allBands[Band]);
    							if (next_evt == evt) {
    								if ((Freq < lBand) || (Freq > rBand)) {
    									newFreq = lBand;
    									ev.cmd = cmdFreq;
    									if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    								}
    							} else {
    								next_evt = evt;
    								ev.cmd = cmdFreq;
    								if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    							}
    						}
    					} else {
    						Report(1, "[que:%u] Band already set to %u '%s'\n", queCnt, Band, allBands[Band]);
    					}
    				break;
    				case cmdScan:
    					if (!scan) {
    						scan = 1;
    						rda5807_StartSeek(seek_up);
    					}
    				break;
    				case cmdBass:
    					if (newBassBoost != BassBoost) {
    						BassBoost = newBassBoost;
    						rda5807_SetBassBoost(BassBoost);
	#ifdef SET_SSD1306
    						if (noMute)
    							sprintf(st, "Bas:%u Vol:%u", BassBoost, Volume);
    						else
    							sprintf(st, "Bas:%u Vol:%u M", BassBoost, Volume);
    						mkLineCenter(st, FONT_WIDTH);
    						ssd1306_clear_lines(4, 1);
    						ssd1306_text_xy(st, 1, 4, false);
	#endif
	#ifdef SET_LCD_UC
    						sprintf(sta, "BASS:%u", BassBoost);
    						sprintf(stb, "VOLUME:%u", Volume);
    						if (!noMute) strcat(stb, " Mute");
    						mkLineWidth(sta, stb, lfnt->FontWidth);
    						showLine(sta, line2, lfnt, true);
	#endif
    						Report(1, "[que:%u] set new BassBoost to %u\n", queCnt, BassBoost);
    					} else {
    						Report(1, "[que:%u] BassBoost already set to %u\n", queCnt, BassBoost);
    					}
    					break;
    				case cmdVol:
    					if (newVolume != Volume) {
    						Volume = newVolume;
    						rda5807_SetVolume(Volume);
	#ifdef SET_SSD1306
    						if (noMute)
    							sprintf(st, "Bas:%u Vol:%u", BassBoost, Volume);
    						else
    							sprintf(st, "Bas:%u Vol:%u M", BassBoost, Volume);
    						mkLineCenter(st, FONT_WIDTH);
    						ssd1306_clear_lines(4, 1);
    						ssd1306_text_xy(st, 1, 4, false);
	#endif
	#ifdef SET_LCD_UC
    						sprintf(sta, "BASS:%u", BassBoost);
    						sprintf(stb, "VOLUME:%u", Volume);
    						if (!noMute) strcat(stb, " Mute");
    						mkLineWidth(sta, stb, lfnt->FontWidth);
    						showLine(sta, line2, lfnt, true);
	#endif
    						Report(1, "[que:%u] set new Volume to %u\n", queCnt, Volume);
    					} else {
    						Report(1, "[que:%u] Volume already set to %u\n", queCnt, Volume);
    					}
    				break;
    				case cmdFreq:
    					if ((newFreq >= lBand) && (newFreq <= rBand)) {
    						if (newFreq != Freq) {
    							Freq = newFreq;
    							uint16_t fr = (uint16_t)(Freq * 10);
    							rda5807_SetFreq_In100Khz(fr);
    							rda5807_Get_ChanRssiFlag(&Chan, &RSSI, &stereo);
    							int8_t idx = -1;
	#ifdef SET_SSD1306
    							if (stereo)
    								sprintf(st, "S:%u F:%.2f S", RSSI, Freq);
    							else
    								sprintf(st, "S:%u F:%.2f", RSSI, Freq);
    							mkLineCenter(st, FONT_WIDTH);
    							ssd1306_clear_lines(2, 3);
    							ssd1306_text_xy(st, 1, 2, false);
    							sprintf(sta, "%s", nameStation(Freq, &idx));
    							mkLineCenter(sta, FONT_WIDTH);
    							ssd1306_text_xy(sta, 1, 3, false);
								if (noMute)
    								sprintf(st, "Bas:%u Vol:%u", BassBoost, Volume);
    							else
    								sprintf(st, "Bas:%u Vol:%u M", BassBoost, Volume);
    							mkLineCenter(st, FONT_WIDTH);
    							ssd1306_text_xy(st, 1, 4, false);
	#endif
	#ifdef SET_LCD_UC
    							sprintf(sta, "RSSI:%u", RSSI);
    							stb[0] = '\0';
    							if (stereo) strcpy(stb, "Stereo ");
    							sprintf(stb+strlen(stb), "FREQ:%.2f", Freq);
    							mkLineWidth(sta, stb, lfnt->FontWidth);
    							showLine(sta, line1, lfnt, false);
    							//
    							sprintf(sta, "BASS:%u", BassBoost);
    							sprintf(stb, "VOLUME:%u", Volume);
    							if (!noMute) strcat(stb, " Mute");
    							mkLineWidth(sta, stb, lfnt->FontWidth);
    							showLine(sta, line2, lfnt, false);
    							//
    							int dlm = sprintf(stn, "%s", nameStation(Freq, &idx));
    							UC1609C_DrawFilledRectangle(1, line3, UC1609C_WIDTH - 4, lfnt->FontHeight - 1, BACKGROUND);
    							UC1609C_Print(caclX(stn, lfnt->FontWidth), line3, stn, lfnt, 0, FOREGROUND);
    							//
    							UC1609C_DrawFilledRectangle(2, line4, UC1609C_WIDTH - 4, lfnt->FontHeight - 2, BACKGROUND);
    							UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    							UC1609C_update();
	#endif
    							Report(1, "[que:%u] set new Freq to %.3f МГц '%s' Chan:%u Volume:%u idx=%d\n",
    									  queCnt, Freq, nameStation(Freq, NULL), Chan, Volume, idx);
    #ifdef SET_RDS
    							if (rdsFlag) {
    								rds_init();
    								rdsTime = get_mstmr(rdsWait);
    							}
	#endif
    						}
    					}
    				break;
	#ifdef SET_RDS
    				case cmdRds:
    					rds_init();
    					if (!rdsFlag) {
    						Report(1, "[que:%u] RDS monitoring start\r\n", queCnt);
    						rdsFlag = true;
    						rdsTime = get_mstmr(rdsWait);
    					} else {
    						Report(1, "[que:%u] RDS monitoring stop\r\n", queCnt);
    						rdsFlag = false;
    						rdsTime = 0;
    					}
    				break;
	#endif
#endif
    				case cmdSec:
    				{
#ifdef SET_SSD1306
    					sec2str(stx);
    					strcat(stx, " ");
    					mkLineCenter(stx, FONT_WIDTH);
    					ssd1306_text_xy(stx, 1, 1, true);
#endif
#ifdef SET_LCD_UC
    					dl = sec2str(stx);
    					if (dl > (UC1609C_WIDTH / hfnt->FontWidth)) {
    						dl = UC1609C_WIDTH / hfnt->FontWidth;
    						stx[dl] = '\0';
    					}
    					uint16_t x = ((UC1609C_WIDTH - (hfnt->FontWidth * dl)) >> 1);
    					UC1609C_Print(x, 0, stx, hfnt, 0, BACKGROUND);
    					//
    					if (flag_ver) {
    						tmr_ver--;
    						if (!tmr_ver) {
    							flag_ver = false;
    							dl = sprintf(tmp, "%s", ver);
    					    	if (dl > (UC1609C_WIDTH / mfnt->FontWidth - 1)) {
    					    		dl = UC1609C_WIDTH / mfnt->FontWidth - 1;
    					    		tmp[dl] = '\0';
    					    	}
    					    	x = ((UC1609C_WIDTH - (mfnt->FontWidth * dl)) >> 1);
    					    	UC1609C_Print(x, UC1609C_HEIGHT - mfnt->FontHeight, tmp, mfnt, 0, FOREGROUND);
    						}
    					}
    					UC1609C_update();
#endif
#ifdef SET_RDA
    					if (!scan) {
    						uint16_t sig = rda5807_rssi();
    						if (sig != RSSI) {
    							RSSI = sig;
    							stereo = rda5807_Get_StereoMonoFlag();
	#ifdef SET_SSD1306
    							if (stereo)
    								sprintf(st, "S:%u F:%.2f S", RSSI, Freq);
    							else
    								sprintf(st, "S:%u F:%.2f", RSSI, Freq);
    							mkLineCenter(st, FONT_WIDTH);
    							ssd1306_clear_lines(2, 1);
    							ssd1306_text_xy(st, 1, 2, false);
	#endif
	#ifdef SET_LCD_UC
    							sprintf(sta, "RSSI:%u", RSSI);
    							stb[0] = '\0';
    							if (stereo) strcpy(stb, "Stereo ");
    							sprintf(stb+strlen(stb), "FREQ:%.2f", Freq);
    							mkLineWidth(sta, stb, lfnt->FontWidth);
    							showLine(sta, line1, lfnt, true);
	#endif
    						}
    					} else {
    						if (rda5807_Get_SeekTuneReadyFlag()) {
    							scan = 0;
    							Freq = (float)rda5807_GetFreq_In100Khz();
    							Freq /= 10;
    							rda5807_Get_ChanRssiFlag(&Chan, &RSSI, &stereo);
    							int8_t idx = -1;
	#ifdef SET_SSD1306
    							if (stereo)
    								sprintf(st, "S:%u F:%.2f S", RSSI, Freq);
    							else
    								sprintf(st, "S:%u F:%.2f", RSSI, Freq);
    							mkLineCenter(st, FONT_WIDTH);
    							ssd1306_clear_lines(2, 3);
    							ssd1306_text_xy(st, 1, 2, false);
    							sprintf(sta, "%s", nameStation(Freq, &idx));
    							mkLineCenter(sta, FONT_WIDTH);
    							ssd1306_text_xy(sta, 1, 3, false);
    							if (noMute)
    								sprintf(st, "Bas:%u Vol:%u", BassBoost, Volume);
    							else
    								sprintf(st, "Bas:%u Vol:%u M", BassBoost, Volume);
    							mkLineCenter(st, FONT_WIDTH);
    							ssd1306_text_xy(st, 1, 4, false);
	#endif
    #ifdef SET_LCD_UC
    							sprintf(sta, "RSSI:%u", RSSI);
    							stb[0] = '\0';
    							if (stereo) strcpy(stb, "Stereo ");
    							sprintf(stb+strlen(stb), " FREQ:%.2f", Freq);
    							mkLineWidth(sta, stb, lfnt->FontWidth);
    							showLine(sta, line1, lfnt, false);
    							//
    							sprintf(sta, "BASS:%u", BassBoost);
    							sprintf(stb, "VOLUME:%u", Volume);
    							if (!noMute) strcat(stb, "_Mute");
    							mkLineWidth(sta, stb, lfnt->FontWidth);
    							showLine(sta, line2, lfnt, false);
    							//
    							int dlm = sprintf(stn, "%s", nameStation(Freq, &idx));
    							UC1609C_DrawFilledRectangle(1, line3, UC1609C_WIDTH - 4, lfnt->FontHeight - 1, BACKGROUND);
    							UC1609C_Print(caclX(stn, lfnt->FontWidth), line3, stn, lfnt, 0, FOREGROUND);
    							//
    							UC1609C_DrawFilledRectangle(2, line4, UC1609C_WIDTH - 4, lfnt->FontHeight - 2, BACKGROUND);
    							UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    							UC1609C_update();
	#endif
    							Report(1, "[que:%u] set new Freq to %.3f МГц %s Chan:%u Volume:%u idx=%d\n",
    									  queCnt, Freq, nameStation(Freq, NULL), Chan, Volume, idx);
    							//
    					#ifdef SET_RDS
    							if (rdsFlag) {
    								rds_init();
    								rdsTime = get_mstmr(rdsWait);
    							}
    					#endif
    						}
    					}
#endif
    					if (!prnFixFreq) {
    						prnFixFreq = true;
    						ev.cmd = cmdEnc;
    						if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    					}
    				}
    				break;
    				case cmdHelp:
    					tmp[0] = '\0';
    					for (int8_t i = 0; i < MAX_CMDS; i++) {
    						sprintf(tmp+strlen(tmp), "\t%s\n", s_cmds[i]);
    					}
    					Report(0, "%s", tmp);
    				break;
    				case cmdRestart:
    					restart = 1;
    				break;
    				case cmdEpoch:
    				break;
    				case cmdVer:
    					Report(1, "%s\n", ver);
    				break;
    			}
    		}
    	}
//
#ifdef SET_RDS
    	if (rdsFlag && rdsTime) {
    		if (check_mstmr(rdsTime)) {
    			rdsTime = get_mstmr(rdsWait);
    			if (!readyRds && rda5807_Get_RDSReady()) {
    				memset(dataRDS, 0, sizeof(dataRDS));
    				blk = (blocks_t *)&dataRDS;
    				blk->blockA = rda5807_Get_reg(RDA5807M_REG_BLOCK_A);
    				//Report(1, "[RDS] %04X %04X %04X %04X\r\n", blk->blockA, blk->blockB, blk->blockC, blk->blockD);
    				//

    				// Сравним содержимое блока A (ID станции) с предыдущим значением
    				if (blk->blockA == MaybeThisIDIsReal) {
    					if (IDRepeatCounter < REPEATS_TO_BE_REAL_ID) {
    						IDRepeatCounter++; // Значения совпадают, отразим это в счетчике
    						if (IDRepeatCounter == REPEATS_TO_BE_REAL_ID) sID = MaybeThisIDIsReal;// Определились с ID станции
    				    }
    				} else {
    					IDRepeatCounter = 0; // Значения не совпадают, считаем заново
    					MaybeThisIDIsReal = blk->blockA;
    				}
    				//
    				if (!sID || (blk->blockA != sID)) {//Пока не определимся с ID, разбирать RDS не будем
    												   //ID не совпадает. Пропустим эту RDS группу
    					continue;
    				}
    				// ID станции не скачет, вероятность корректности группы в целом выше
    				//Report(1, "[RDS] ID:0x%X\r\n", sID);
    				//
    				errLevelB = rda5807_Get_reg(0x0B) & RDA5807M_BLERB_MASK;
    				if (errLevelB < 3) {
    					// Блок B корректный, можем определить тип и версию группы
    					blk->blockB = rda5807_Get_reg(RDA5807M_REG_BLOCK_B);
    					if (!PTy_printed) { // Но сначала считаем PTy
    						if (PTy == (blk->blockB & RDS_ALL_PTY_MASK) >> RDS_ALL_PTY_SHIFT) {
    							Report(1, "[RDS] PlayType: %s\r\n", namePTy[PTy]);
    							PTy_printed = true;
    						} else {
    							PTy = (blk->blockB & RDS_ALL_PTY_MASK) >> RDS_ALL_PTY_SHIFT;
    				        }
    				    }
    					groupType = (blk->blockB & RDS_ALL_GROUPTYPE_MASK) >> RDS_ALL_GROUPTYPE_SHIFT;
    					groupVer = (blk->blockB & RDS_ALL_GROUPVER) > 0;
    					uint16_t reg10 = rda5807_Get_reg(RDA5807M_REG_BLER_CD);//getRegister(RDA5807M_REG_BLER_CD);
    					errLevelC = (reg10 & RDA5807M_BLERC_MASK) >> RDA5807M_BLERC_SHIFT;
    					errLevelD = (reg10 & RDA5807M_BLERD_MASK) >> RDA5807M_BLERD_SHIFT;
    					// ************* 0A, 0B - PSName, PTY ************
    					if ((groupType == 0) && (errLevelD < 3)) {
    						// Сравним новые символы PSName со старыми:
    						blk->blockD = rda5807_Get_reg(RDA5807M_REG_BLOCK_D);
    						char c = blk->blockD >> 8; // новый символ
    						uint8_t i = (blk->blockB & RDS_GROUP0_C1C0_MASK) << 1; // его позиция в PSName
    						if (PSName[i] != c) { // символы различаются
    							PSNameUpdated &= ~(1 << i);//!(1 << i); // сбросим флаг в PSNameUpdated
    							PSName[i] = c;
    						} else {// символы совпадают, установим флаг в PSNameUpdated:
    							PSNameUpdated |= 1 << i;
    				        }
    						// Аналогично для второго символа
    						c = blk->blockD & 255;
    						i++;
    						if (PSName[i] != c) {
    							PSNameUpdated &= ~(1 << i);//!(1 << i);
    							PSName[i] = c;
    				        } else {
    				        	PSNameUpdated |= 1 << i;
    				        }
    						// Когда все 8 флагов в PSNameUpdated установлены, считаем что PSName получено полностью
    						if (PSNameUpdated == 255) {
    							// Дополнительное сравнение с предыдущим значением
    							if (strcmp(PSName, PSName_prev) != 0) {
    								Report(1, "[RDS] Station: %s\r\n", PSName);
    								strcpy(PSName_prev, PSName);
    								int lens = strlen(PSName);
    								if (lens > 15) lens = 15;
    								sprintf(st, "RDS : %.*s", lens, PSName);
	#ifdef SET_SSD1306
    								ssd1306_clear_lines(4, 1);
    								mkLineCenter(st, 7);
    								ssd1306_text_xy(st, 1, 4, false);
	#endif
	#ifdef SET_LCD_UC
    								mkLineCenter(st, lfnt->FontWidth);
    								UC1609C_Print(1, line4, st, lfnt, 0, FOREGROUND);
    								UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    								UC1609C_update();
	#endif
    				        	}
    				        }
    				    } // PSName, PTy end
    					// ******** 4A - Clock time and date ********
    					if ((groupType == 4) && (groupVer == 0) && (errLevelC < 3) && (errLevelD < 3)) {
    						blk->blockC = rda5807_Get_reg(RDA5807M_REG_BLOCK_C);
    						blk->blockD = rda5807_Get_reg(RDA5807M_REG_BLOCK_D);
    						uint16_t year;
    						uint8_t month, day;
    						unsigned long MJD = (blk->blockB & RDS_GROUP4A_MJD15_16_MASK);
    						MJD = (MJD << 15) | (blk->blockC >> RDS_GROUP4A_MJD0_14_SHIFT);
    						Report(1, "[RDS] Date: ");
    						if ((MJD < 58844) || (MJD > 62497)) {
    							Report(0, "decode error\r\n");
    				        } else {
    				        	MJDDecode(MJD, &year, &month, &day);
    				        	if ((day <= 31) && (month <= 12)) {
    				        		Report(0, "%02u.%02u.%04u\r\n", day, month, year);
    				            } else {
    				            	Report(0, "decode error (%02u.%02u.%04u)\r\n", day, month, year);
    				            }
    				        }
    						uint8_t hours = (blk->blockC & RDS_GROUP4A_HOURS4_MASK) << 4;
    						hours |= (blk->blockD & RDS_GROUP4A_HOURS0_3_MASK) >> RDS_GROUP4A_HOURS0_3_SHIFT;
    						uint8_t minutes = (blk->blockD & RDS_GROUP4A_MINUTES_MASK) >> RDS_GROUP4A_MINUTES_SHIFT;
    						if ((hours > 23) || (minutes > 59))
    							Report(1, "[RDS] Time: decode error (%02u:%02u)\r\n", hours, minutes);
    				        else {
    				        	long timeInMinutes = hours * 60 + minutes;
    				        	//uint8_t LTO = blk->blockD & RDS_GROUP4A_LTO_MASK;
    				        	if (blk->blockD & RDS_GROUP4A_LTO_SIGN_MASK) {
    				        		timeInMinutes -= (blk->blockD & RDS_GROUP4A_LTO_MASK) * 30;
    				        		if (timeInMinutes < 0) timeInMinutes += 60 * 24;
    				            } else {
    				            	timeInMinutes += (blk->blockD & RDS_GROUP4A_LTO_MASK) * 30;
    				            	if (timeInMinutes > 60 * 24) timeInMinutes -= 60 * 24;
    				            }
    				        	hours = timeInMinutes / 60;
    				        	minutes = timeInMinutes % 60;
    				        	Report(1, "[RDS] Time: %02u:%02u\r\n", hours, minutes);
    				        }
    				    }
    				}
    			}
    			readyRds = rda5807_Get_RDSReady();
    		}
    	}
#endif
    	//
#if defined(SET_ENCODER) || defined(SET_JOYSTIC) || defined(SET_IR)
    	if (cmd_tmr) {
    		if (check_mstmr(cmd_tmr)) {
    			cmd_tmr = 0;
    			//gpio_put(LED_CMD_PIN, 0);//LED_CMD_PIN OFF
    			for (int8_t i = 0; i < 25; i++) put_pixel(0);
    		}
    	}
#endif
    	//
    	if (devError) {
    		errLedOn(true);
    		sleep_ms(50);
    		errLedOn(false);
    		if (last_devError != devError) {
    			last_devError = devError;
    			//
    			tmp[0] = '\0';
    			uint16_t er = 0;
    			for (int8_t i = 0; i < MAX_ERR_CODE; i++) {
    				er = devError & all_devErr[i];
    				if (er) sprintf(tmp+strlen(tmp), " '%s'", errName(er));
    			}
    			Report(1, "Error 0x%04X %s\r\n", devError, tmp);
    			//
    		}
    	}
    	//
    }//while(!restart)

    //remove queue
    queue_free(&evt_fifo);
    Report(1, "Queue released\n");

    //stop and cancel timer
    bool cancelled = cancel_repeating_timer(&timer);
    Report(1, "Timer cancelled... %d\n", cancelled);

    sleep_ms(500);

#ifdef SET_SSD1306
    //display off
    ssd1306_on(false);
#endif
#ifdef SET_LCD_UC
    UC1609C_enable(0);//OFF
#endif

#ifdef SET_JOYSTIC
    //wait joystic_task closed.....
    uint8_t sch = 255;
    while (joy && sch) {
    	sleep_ms(1);
    };
    sleep_ms(100);
#endif

    //restart
    watchdog_reboot(0, SRAM_END, 0);

    return 0;
}
