#include "hdr.h"
#include "sleep.h"
#include "libs.h"
#include "rda5807.h"
#include "fonts.h"
#include "UC1609C.h"

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
	cmdClr,
	cmdUart,
	cmdMute,
	cmdSec,
	cmdTemp
#ifdef SET_ENCODER
	,
	cmdEnc,
	cmdInc,
	cmdDec
#endif
	,
	cmdCfg,
	cmdScan,
	cmdVol,
	cmdBass,
	cmdBand,
	cmdFreq,
	cmdList,
	cmdRds,
	cmdShowFreq,
	cmdRead,
	cmdReadCont,
	cmdNext,
	cmdCheck,
	cmdErase
};

enum {
	devOk = 0,
	devMem = 1,
	devTik = 2,
	devQue = 4,
	devUart = 8,
	devI2c = 16,
	devDma = 32,
	dev_RDA = 64
};

#ifdef SET_WITH_DMA
	uint16_t devDMA = devDma;

	int dma_chan;

#endif


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
//const char *ver = "Ver.2.4 31.08.22 multicore";// support encoder EC11 !!!
//const char *ver = "Ver.2.4.1 31.08.22 multicore";// set system clock to 48MHz !
//const char *ver = "Ver.2.4.2 01.09.22 multicore";
//const char *ver = "Ver.2.4.3 02.09.22 multicore";//minor changes for keys pressed
//const char *ver = "Ver.2.5 02.09.22 mux_kbd";//add mux_kbd with 6 button
//const char *ver = "Ver.2.5.1 03.09.22 encoder";
//const char *ver = "Ver.2.6 03.09.22 encoder";// new encoder mode support !!!
//const char *ver = "Ver.2.7 04.09.22 encoder";// remove ssd1306 support and add sleep mode
//const char *ver = "Ver.2.8 05.09.22 encoder";// add contrast mode to control menu
//const char *ver = "Ver.2.9 07.09.22 encoder";// add dma for read flash
//const char *ver = "Ver.2.9.1 08.09.22 encoder";
//const char *ver = "Ver.3.0 08.09.22 enc&flash";// save/restore radio_list in rda_sector of flash-memory
const char *ver = "Ver.3.1 09.09.22";




volatile static uint32_t epoch = 1662723599;//1662671765;//1662670195;//1662659160;//1662643850;//1662589615;
//1662572765;//1662373645;//1662368495;//1662331845;//1662327755;//1662295275;//1662288820;
//1662251055;//1662246985;//1662209185;//1662156375;//1662151345;//1662114275;//1662038845;
//1661990305;//1661949985;//1661902365;//1661897825;//1661792625;
//1661767566;//1661726088;//1661699652;//1661684619;//1661641164;//1661614899;//1661536565;
//1661463575;//1661459555;//1661371110;//1661344350;//1661285299;//1661258255;//1661193099;//1661096209;
//1661004270;//1660945885;//1660743445;//1660736830;//1660731354;//1660684399;
//1660657998;//1660601220;//1660576465;//1660563510;//1660506862;//1660505693;//1660494699;

const char *s_cmds[MAX_CMDS] = {
	"help",
	"restart",
	"epoch:",
	"ver",
	"input_err",
	"clr",
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
	,
	"cfg",
	"scan",
	"vol:",
	"bass:",
	"band:",
	"freq:",
	"list",
	"rds",
	"showfreq",
	"read",
	"readcont",
	"next",
	"check",
	"erase"
};

const uint16_t all_devErr[MAX_ERR_CODE] = {
	devMem,
	devTik,
	devQue,
	devUart,
	devI2c,
	devDma,
	dev_RDA
};


volatile uint16_t devError = devOk;
volatile uint16_t last_devError = devOk;
volatile uint32_t seconda = 0;
int tZone = 0;//2;
volatile uint8_t restart = 0;
uint8_t sleepON = false;
//
uint8_t rxByte = 0;
uint16_t rxInd = 0;
char rxBuf[MAX_UART_BUF] = {0};
bool uart_enable = true;
//
queue_t evt_fifo;
typedef struct {
	int cmd;
	uint32_t attr;
} evt_t;
const int EVT_FIFO_LENGTH = 32;
bool que_start = false;

char tmp[128];

bool led = true;
const uint LED_PIN = 29;
const uint ERR_PIN = 8;//14;
const uint LCD_HIDE_PIN = 12;
const uint LED_CMD_PIN = 14;//16
uint32_t start_jkey = 0;
uint32_t cmd_tmr = 0;

//   for RDA support
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
uint8_t Volume = 3;//6;//8;
uint8_t newVolume = 3;//6;//8;
uint8_t BassBoost = 0;
uint8_t newBassBoost = 0;
bool stereo = false;
uint8_t noMute = 1;
uint8_t dataRDS[8] = {0};
bool syncRds = false;
bool readyRds = false;
//

//---   for Menu   ---
enum {
	iNone = -1,
	iExit,
	iList,
	iScan,
	iFreq,
	iVolume,
	iBass,
	iContrast,
	iRestart,
	iSleep
};
enum {
	line1 = 0,
	line2,
	line3,
	line4,
	line5
};
FontDef_t *mfnt = NULL;
FontDef_t *lfnt = NULL;
FontDef_t *hfnt = NULL;
uint8_t lines[5] = {0};
bool menuAct = false;
int8_t encMode = iNone;
int8_t indMenu = -1;
const char *allMenu[MAX_MENU] = {
	"  Exit  ",
	"  List  ",
	"  Scan  ",
	"  Freq  ",
	" Volume ",
	"  Bass  ",
	"Contrast",
	" Restart",
	"  Sleep "
};

//---------------

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
static rec_t list[MAX_LIST];
#ifdef RUS_SUPPORT
	static const rec_t def_list[MAX_LIST] = {
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
		{2,  98.1, "Радио Пи ФМ"},// Пи ФМ
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
	static const rec_t def_list[MAX_LIST] = {
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
		{2, 98.1, "Radio PI FM"},// Пи ФМ
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

	bool joy = false;
#endif


#ifdef SET_ENCODER
	#define ENC_PIN_A  9
	#define ENC_PIN_B 10
	#define ENC_PIN   11

	const int16_t MIN_ENC_VALUE = -16383;
	const int16_t MAX_ENC_VALUE = 16383;

	volatile int16_t ec_counter = 0;
	volatile int16_t ec_last_counter = 0;
	bool fix_freq = false;
	uint32_t ec_tmr = 0;
#endif

#ifdef SET_KBD_MUX
	#define MUX_S0_PIN   9
	#define MUX_S1_PIN  10
	#define MUX_S2_PIN  11
	#define MUX_KEY_PIN 12
	#define TOTAL_MUX    8
	enum {//GP9,GP10,GP11
		listMinus = 0,
		listPlus,
		scanMinus,
		scanPlus,
		volMinus,
		volPlus,
		muteKey,
		rstKey,
		chanNone = 255
	};
	volatile uint8_t muxNum = chanNone;
#endif


#ifdef SET_FLASH
	const uint8_t *flash_addr = (const uint8_t *)(XIP_BASE);
	const uint32_t flash_size = 2 * 1024 * 1024;
	const uint32_t flash_sectors = flash_size / FLASH_SECTOR_SIZE;
	const uint32_t rda_sector = flash_sectors - 1;
	//
	int adr_sector = 0, offset_sector = 0;
	unsigned char fs_work[FLASH_SECTOR_SIZE] = {0};
	char flash_buf[FLASH_SECTOR_SIZE] = {0};
	int list_sector = FLASH_PAGE_SIZE << 1;
	uint32_t fadr;// = XIP_BASE + (adr_sector * FLASH_SECTOR_SIZE) + offset_sector;
	const uint32_t flash_step = 16;
	#ifdef SET_WITH_DMA
		dma_channel_config read_conf;
		bool irq_dma_init = false;
		bool dma_chan_init = false;
	#endif

	//uint32_t irq_status = 0;
	//uint spinlock_num = 1;
	//spin_lock_t *flash_spinlock = NULL;
#endif


//*******************************************************************************************
//*******************************************************************************************
//*******************************************************************************************

//-------------------------------------------------------------------------------------------
//   Функция зажигает светодиод нового события (от JOYSTIC, ENCODER, KBD_MUX)
//   Светодиод будет потушен через 150 ms
//
void cmdLedOn()
{
	gpio_put(LED_CMD_PIN, 1);
	cmd_tmr = get_mstmr(_150ms);
}
//-------------------------------------------------------------------------------------------

#if defined(SET_JOYSTIC) || defined(SET_ENCODER)
	//----------------------------------------------------------------------------------------
	//   CallBack-функция вызывается по завершению прерывания от определенных пинов
	//   Обслуживает кнопки джойстика и энкодера, помещая соответствующее сообщение в очередь
	//
	void gpio_callback(uint gpio, uint32_t events)
	{
		if (!que_start) {
#ifdef SET_ENCODER
			ec_counter = ec_last_counter = 0;
#endif
			return;
		}
#ifdef SET_KBD_MUX
		uint8_t chan = muxNum;
#endif

		if ((gpio == jKEY_PIN)
#ifdef SET_KBD_MUX
						 || (gpio == MUX_KEY_PIN)
#endif
#ifdef SET_ENCODER
						 	 || (gpio == ENC_PIN)
#endif
							) {
			if (!gpio_get(gpio)) {
				//
				if (check_mstmr(start_jkey)) {
					cmdLedOn();
					evt_t e = {cmdNone, 0};
					switch (gpio) {
						case jKEY_PIN:// нажата кнопка джойстика
							seek_up = 1;
							e.cmd = cmdScan;
						break;
#ifdef SET_ENCODER
						case ENC_PIN:// нажата кнопка энкодера
							e.cmd = cmdEnc;
							if (sleepON) for (int i = 0; i < 1000; i++) {}
						break;
#endif
#ifdef SET_KBD_MUX
						case MUX_KEY_PIN:// нажата кнопка на клавиатуре
						{
							switch (chan) {
								case listMinus:
									seek_up = 0;
									e.cmd = cmdList;
								break;
								case listPlus:
									seek_up = 1;
									e.cmd = cmdList;
								break;
								case scanMinus:
									seek_up = 0;
									e.cmd = cmdScan;
								break;
								case scanPlus:
									seek_up = 1;
									e.cmd = cmdScan;
								break;
								case volMinus:
									newVolume = Volume - 1;
									if (newVolume <= MAX_VOLUME) {
										e.cmd = cmdVol;
										e.attr = newVolume;
									}
								break;
								case volPlus:
									newVolume = Volume + 1;
									if (newVolume <= MAX_VOLUME) {
										e.cmd = cmdVol;
										e.attr = newVolume;
									}
								break;
								case muteKey:
									e.cmd = cmdMute;
								break;
								case rstKey:
									e.cmd = cmdRestart;
								break;
							}
						}
						break;
#endif
					}
					if (e.cmd != cmdNone)
						if (!queue_try_add(&evt_fifo, &e)) devError |= devQue;
					start_jkey = get_mstmr(_100ms);//_40ms
				}
				//
			} else {
				start_jkey = get_mstmr(_75ms);//_40ms
			}
		}
#ifdef SET_ENCODER
		else if (gpio == ENC_PIN_A) {// блок обслудивания данных от энкодера (каналы А и В)
			if (sleepON) return;

			if (check_mstmr(ec_tmr)) {
				if (gpio_get(ENC_PIN_B)) ec_counter++;
									else ec_counter--;
				if (ec_last_counter != ec_counter) {
					int cd = cmdNone;
					if ((ec_last_counter == MIN_ENC_VALUE) && (ec_counter == MAX_ENC_VALUE)) cd = cmdDec;
					else
					if ((ec_last_counter == MAX_ENC_VALUE) && (ec_counter == MIN_ENC_VALUE)) cd = cmdInc;
					else
					if (ec_last_counter < ec_counter) cd = cmdInc;
					else
					if (ec_last_counter > ec_counter) cd = cmdDec;
					ec_last_counter = ec_counter;
					if (cd != cmdNone)	{
						evt_t e = {cd, 0};
						if (!queue_try_add(&evt_fifo, &e)) devError |= devQue;
						ec_tmr = get_mstmr(_250ms);
					}
				}
			}
		}
#endif
	}
#endif
	//----------------------------------------------------------------------------------------
#ifdef SET_JOYSTIC
	//   Функция добавляет очередной замер в буффер для последующей фильтрации
	//   (поиск среднего в скользящем окне размерностью MAX_ADC_BUF замеров)
	//
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
	//   Функция обслуживает джойстик - формирует соответствующие события в очереди
	//   Выполняется вторым ядром rp pico
	//
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

				jtmr = get_mstmr(_25ms);

				if (sleepON) continue;

				adc_select_input(0);
				valX = adc_read();
				if (adcAddVal(&chanX, valX) == MAX_ADC_BUF) {//в окне накоплено MAX_ADC_BUF выборок -> фильтрация !
					sumAdc = 0;
					for (int8_t j = 0; j < MAX_ADC_BUF; j++) sumAdc += chanX.val[j];
					valX = sumAdc / MAX_ADC_BUF;
					if ((valX < MIN_VAL) || (valX > MAX_VAL)) {
						//
						if (valX < MIN_VAL) seek_up = 1;
						else
						if (valX > MAX_VAL) seek_up = 0;
						//
						cmdLedOn();
						ev.cmd = cmdList;
						ev.attr = seek_up;
						if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
						jtmr = get_mstmr(_175ms);
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
							jtmr = get_mstmr(_175ms);
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

//--------------------------------------------------------------------------------------------
//   Функция выводит на печать список радио станций из листа (лист для Калининграда)
//
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
//   Функция возврвщает текстовое название радио станции и её индекс в листе
//   по заданной частоте, есди такая существует в листе
//
const char *nameStation(float fr, int8_t *ind)
{
int8_t ik = -1;
uint32_t fin = fr * 10;
uint32_t fcon;

	for (int8_t i = 0; i < MAX_LIST; i++) {
		fcon = list[i].freq * 10;
		if (fcon == fin) {
			ik = i;
			break;
		}
	}
	if (ind) *ind = ik;

	if (ik != -1) return list[ik].name;
			 else return noneStation;
}
//-------------------------------------------------------------------------------------------
//   Функция возвращает следубщую частоту, относительно текущей, радио станции из лист
//   согласно направлению поиска - вперёд или назад по списку листа
//
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

	return ret;
}
//-------------------------------------------------------------------------------------------
#ifdef SET_RDS
//-------------------------------------------------------------------------------------------
//   Функция декодирует значения год, месяц и день из данных RDS
//
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
//   Функция инициализирует служебные переменные для чтения и декодирования данных RDS
//
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

//-------------------------------------------------------------------------------------------
//   Функция возвращает текстовое название ошибки (от какого модуля произошла ошибка)
//
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
		case devRDA:// = 0x40
			return "devRDA";
	}

	return "???";
}
//------------------------------------------------------------------------------------------
//   CallBack-функция вызывается по завершению прерывания от таймера
//   Функция ведёт внутренние часы (с перидом в 5 ms), а также обслуживает кнопки клавиатуры
//
bool repeating_timer_callback(struct repeating_timer *t)
{
	inc_msCounter();

	if (!que_start) return true;

#ifdef SET_KBD_MUX
	if (!(get_msCounter() % _50ms)) {
		muxNum++;
		if (muxNum >= TOTAL_MUX) muxNum = 0;
		uint32_t reg = 1ul << MUX_S0_PIN;
		for (uint8_t i = 0; i < 3; i++) {
			if ((muxNum>>i) & 1) gpio_set_mask(reg);
							else gpio_clr_mask(reg);
			reg <<= 1;
		}
	}
#endif

	if (!(get_msCounter() % _1s)) {// 1 seconda
		seconda++;
		if (!sleepON) {
			evt_t evt = {
				.cmd = cmdSec,
				.attr = seconda
			};
			if (!queue_try_add(&evt_fifo, &evt)) devError |= devQue;

			led = !led;
#ifdef SET_TIK_LED
			gpio_put(LED_PIN, led);
#endif
		} else {
#ifdef SET_TIK_LED
			gpio_put(LED_PIN, 0);
#endif
		}
	}

    return true;
}
//------------------------------------------------------------------------------------------
//   CallBack-функция вызывается по приёму символа с интерфейса uart0
//   Функция принимает команду и помещает в очередь соответствующее событие
//
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
        					case cmdClr:     //"clr"
        					case cmdTemp:    //"temp"
        					case cmdShowFreq://"showfreq"
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
        							nv &= 0xf;
        							newVolume = nv;
        							evt.cmd = i;
        							evt.attr = newVolume;
        						}
        					break;
#ifdef SET_RDS
							case cmdRds:     //"rds"
								evt.cmd = i;
							break;
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
#ifdef SET_FLASH
        					case cmdRead:// "read:511"
        					case cmdCheck:// "check:511"
        					case cmdErase:// "erase:511"
        						if (*uk == ':') {
        							uk++;
        							if (strlen(uk) > 0) {
        								int sek = atoi(uk);
        								if ( ((sek >= 0) && (sek < flash_sectors)) || (sek == -1) ) {
        									adr_sector = sek;
        									offset_sector = 0;
        									fadr = XIP_BASE + (adr_sector * FLASH_SECTOR_SIZE) + offset_sector;
        									evt.cmd = i;
        									evt.attr = fadr;
        								}
        							}
        						}
        					break;
        					case cmdNext:// "next"
        						offset_sector += list_sector;
        						offset_sector &= FLASH_SECTOR_SIZE - 1;
        						fadr += list_sector;
        						evt.cmd = i;
        						evt.attr = fadr;
        					break;
#endif
        				}
        				break;
        			}
        		}
        		if (!sleepON) {
        			if (evt.cmd == cmdNone) evt.cmd = cmdErr;
        			if (!queue_try_add(&evt_fifo, &evt)) devError |= devQue;
        		}
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
//   Функция обновляет экранное меню на дисплее UC1609C
//
void refreshMenu()
{
uint8_t inv = FOREGROUND;
char st[64];
int16_t tp = MAX_MENU / 4;
if (MAX_MENU % 4) tp++;
int16_t cx = (UC1609C_WIDTH - ((strlen(allMenu[0]) * lfnt->FontWidth) * tp)) >> 1;
uint8_t lin = 255;
int16_t lone = strlen(allMenu[0]);

	for (int8_t l = 0; l < MAX_MENU; l++) {
		if (indMenu == l) inv = BACKGROUND; else inv = FOREGROUND;
		sprintf(st, "%s", allMenu[l]);
		if ((!(l % 4)) && l) {
			cx += (lone * lfnt->FontWidth) + 1;
			lin = 0;
		} else {
			lin++;
		}
		showLineMenu(cx, st, lines[lin], lfnt, false, inv);
	}
	UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
	UC1609C_update();
}
//------------------------------------------------------------------------------
//   Функция возвращает значение температуры с внутреннего датчика в градусах
//   Цельсия или Фаренгейта или значение -1 при ошибке задания входного параметра
//
float read_onboard_temperature(const char unit)
{
    // 12-bit conversion, assume max value == ADC_VREF == 3.3V
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
//------------------------------------------------------------------------------
//   Функция печатает значения частот, установленных в системе
//
void show_clocks()
{
	uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
	uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
	uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
	uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
	uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
	uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
	uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
	uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
	Report(1, "All clocks are set to:\n"
			  "\tpll_sys : %d kHz\n"
			  "\tpll_usb : %d kHz\n"
			  "\trosc    : %d kHz\n"
			  "\tclk_sys : %d kHz\n"
			  "\tclk_peri: %d kHz\n"
			  "\tclk_usb : %d kHz\n"
			  "\tclk_adc : %d kHz\n"
			  "\tclk_rtc : %d kHz\n",
			  f_pll_sys, f_pll_usb, f_rosc, f_clk_sys, f_clk_peri, f_clk_usb, f_clk_adc, f_clk_rtc);
}
//------------------------------------------------------------------------------
#ifdef SET_FLASH

	#ifdef SET_WITH_DMA
	//------------------------------------------------------------------------------
	//   Функция инициализирует канал DMA для чтения данных flash-памяти
	//
	bool chan_init()
	{
		dma_chan = dma_claim_unused_channel(true);
		if (dma_chan == -1) {
			devError |= devDma;
			return false;
		}
		read_conf = dma_channel_get_default_config(dma_chan);
		channel_config_set_transfer_data_size(&read_conf, DMA_SIZE_8);
		channel_config_set_read_increment(&read_conf, true);
		channel_config_set_write_increment(&read_conf, true);

		return true;
	}
	//------------------------------------------------------------------------------
	//   CallBack-функция, вызывается по завершению пересылки данных
	//   по каналу dma_chan. Помещает в очередь соответствующее сообщение.
	//
	void dma_callback()
	{
		// Clear the interrupt request.
		dma_hw->ints0 = 1u << dma_chan;
		//
		fadr = XIP_BASE + (adr_sector * FLASH_SECTOR_SIZE) + offset_sector;
		evt_t evt = {cmdReadCont, fadr};
		if (!queue_try_add(&evt_fifo, &evt)) devError |= devQue;
	}
	//------------------------------------------------------------------------------

	#endif

//------------------------------------------------------------------------------
//   Функция обеспечивает пересылку данных из flash-памяти в заданный буфер
//   с помощью канала dma_chan. Завершается пересылка прерыванием DMA_IRQ0.
//
void Flash_ReadSector(uint8_t *buf, uint32_t sector, uint32_t offset, uint32_t len)
{
	if ((sector >= flash_sectors) || (offset >= FLASH_SECTOR_SIZE)) return;
	const uint8_t *adr = flash_addr + (sector * FLASH_SECTOR_SIZE) + offset;
	if (len > (FLASH_SECTOR_SIZE - offset)) len = FLASH_SECTOR_SIZE;

#ifdef SET_WITH_DMA

	dma_channel_configure(
		dma_chan,   // Channel to be configured
		&read_conf, // The configuration we just created
		buf,        // dst
		adr,        // src
		len,        // Number of transfers; in this case each is 1 byte.
		false       //true - Start immediately.
	);
	if (!irq_dma_init) {
		irq_dma_init = true;
		dma_channel_set_irq0_enabled(dma_chan, true);
		irq_set_exclusive_handler(DMA_IRQ_0, dma_callback);
		irq_set_enabled(DMA_IRQ_0, true);
	}
	dma_channel_start(dma_chan);

#else

	memcpy(buf, adr, len);

#endif

}
//------------------------------------------------------------------------------
void Flash_EraseSector(uint32_t sector)
{
	Report(1, "[%s] erase sector %lu ...", __func__, sector);

	spin_lock_t *slk = spin_lock_init(1);
	uint32_t status = spin_lock_blocking(slk);
		flash_range_erase(sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
	spin_unlock_unsafe(slk);
	restore_interrupts(status);

	Report(0, " done.\n");
}
//------------------------------------------------------------------------------
//   Функция записывает данные во flash-память по заданным параметрам
//
void Flash_WriteSector(void *buf, uint32_t sector, uint32_t offset, uint32_t len)
{
	if ((sector >= flash_sectors) ||
			(offset >= FLASH_SECTOR_SIZE) ||
				(len > FLASH_SECTOR_SIZE)) return;

	uint32_t ofs_adr = sector * FLASH_SECTOR_SIZE;

	memset(fs_work, 0xff, sizeof(fs_work));
	memcpy(fs_work, (uint8_t *)buf, len);
	size_t sz = len / FLASH_PAGE_SIZE;
	if (len % FLASH_PAGE_SIZE) sz++;
	sz *= FLASH_PAGE_SIZE;
	ofs_adr += offset;

	Report(1, "[%s] write %lu/%lu bytes to sector %lu (adr:0x%X)...", __func__, len, sz, sector, ofs_adr);

	spin_lock_t *slk = spin_lock_init(1);
	uint32_t status = spin_lock_blocking(slk);
		flash_range_program(ofs_adr, fs_work, sz);
	spin_unlock_unsafe(slk);
	restore_interrupts(status);

	Report(0, " done.\n");
}
//------------------------------------------------------------------------------
void readList()
{
	memcpy(&list[0].band, flash_addr + (rda_sector * FLASH_SECTOR_SIZE), sizeof(rec_t) * MAX_LIST);
}
//------------------------------------------------------------------------------
//   Функция возвращает признак "свободен/занят" для указанного сектора flash-памяти
//
bool isSectorEmpty(uint32_t sector)
{
uint8_t *adr = (uint8_t *)(XIP_BASE + (sector * FLASH_SECTOR_SIZE));

	for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
		if (*adr++ != 0xFF) return false;
	}
	return true;
}
//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------

//**************************************************************************************************
//                                       MAIN
//**************************************************************************************************
int main() {

	set_sys_clock_48mhz();

    stdio_init_all();

    //------------------------- GPIO init --------------------------------------
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(ERR_PIN);
    gpio_set_dir(ERR_PIN, GPIO_OUT);

    gpio_init(LED_CMD_PIN);//GP9
    gpio_set_dir(LED_CMD_PIN, GPIO_OUT);
    gpio_pull_up(LED_CMD_PIN);

#ifdef SET_JOYSTIC
    gpio_init(jKEY_PIN);
    gpio_set_dir(jKEY_PIN, GPIO_IN);
    gpio_pull_up(jKEY_PIN);
    gpio_set_irq_enabled_with_callback(jKEY_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
#endif
#ifdef SET_ENCODER
    gpio_init(ENC_PIN);
    gpio_set_dir(ENC_PIN, GPIO_IN);
    gpio_pull_up(ENC_PIN);
    gpio_set_irq_enabled_with_callback(ENC_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
#endif

#ifdef SET_KBD_MUX
    gpio_init(MUX_S0_PIN);//GP9
    gpio_set_dir(MUX_S0_PIN, GPIO_OUT);
    gpio_pull_down(MUX_S0_PIN);
    //
    gpio_init(MUX_S1_PIN);//GP10
    gpio_set_dir(MUX_S1_PIN, GPIO_OUT);
    gpio_pull_down(MUX_S1_PIN);
    //
    gpio_init(MUX_S2_PIN);//GP11
    gpio_set_dir(MUX_S2_PIN, GPIO_OUT);
    gpio_pull_down(MUX_S2_PIN);
    //
    gpio_init(MUX_KEY_PIN);//12
    gpio_set_dir(MUX_KEY_PIN, GPIO_IN);
    gpio_pull_up(MUX_KEY_PIN);
    gpio_set_irq_enabled_with_callback(MUX_KEY_PIN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
#endif

    //---   Init and read data from internal temperature sensor   ---

    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
    float temperature = read_onboard_temperature('C');

    //--------------------------------------------------------------------

    int idx = -1;

    for (int8_t i = 0; i < 4; i++) {
    	errLedOn(true);
    	sleep_ms(100);
    	errLedOn(false);
    	sleep_ms(100);
    }

    //-------------------------- uart0 config ----------------------------

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, uart_rx_callback);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);

    //------------------- Initialize RTC module --------------------------

    rtc_init();
    sleep_ms(150);
    set_sec(epoch + 1);
    sleep_ms(150);

    //--------------------------------------------------------------------

//#ifdef SET_WITH_DMA
//    iniDMA();
//#endif

    //--------------------- i2c master config ----------------------------

    bi_decl(bi_2pins_with_func(I2C_SDA_PIN, I2C_SCL_PIN, GPIO_FUNC_I2C));
    bi_decl(bi_program_description("picoFM Radio"));
    i2c_init(portRDA, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    //------------------------ read board_id -----------------------------

    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    pico_get_unique_board_id_string(tmp, (sizeof(pico_unique_board_id_t) << 1) + 1);

    //--------------------------------------------------------------------

    Report(0,"\n");
    Report(1, "Start picoRadio app %s\n\tBoardID:%s\n\tTemp:%.02f deg.C\n", ver, tmp, temperature);//uart_puts(UART_ID, "Hello, UART!\n");

    //--------------------------------------------------------------------

    show_clocks();

    //----------------- Initialize queue for events ----------------------

    queue_init(&evt_fifo, sizeof(evt_t), EVT_FIFO_LENGTH);
    Report(1, "Create queue for %d events OK\n", EVT_FIFO_LENGTH);
    que_start = true;

    //---------------- Setup and start periodic timer --------------------

    struct repeating_timer timer;
    int32_t period = 5;// 5 ms
    if (add_repeating_timer_ms(period * -1, repeating_timer_callback, NULL, &timer)) {
    	Report(1, "Start timer with %lu ms period.\n", period);
    } else {
    	devError |= devTik;
    }

    //--------------------------------------------------------------------------

    memset(&list[0].band, 0, sizeof(rec_t) * MAX_LIST);

#ifdef SET_FLASH
    //
    //flash_spinlock = spin_lock_init(spinlock_num);
    //
    // Запись данных листа радио станций во flash-память по адресу сектора rda_sector
    //  !!!!! в SDK НЕТ упоминания, что при стирании/записи данных во флэш НУЖНО
    //        ОБЯЗАТЕЛЬНО ЗАПРЕТИТЬ все прерывания, а после операции их РАЗРЕШИТЬ !!!!!
    if (isSectorEmpty(rda_sector)) {
    	Flash_WriteSector((void *)&def_list[0].band, rda_sector, 0, sizeof(rec_t) * MAX_LIST);
    }
    readList();
    //
    //
    //
#else
    memcpy(&list[0].band, &def_list[0].band, sizeof(rec_t) * MAX_LIST);
#endif

    //--------------------------------------------------------------------------


#ifdef SET_ENCODER
    //   инициализация пинов энкодера
    gpio_init(ENC_PIN_A);
    gpio_set_dir(ENC_PIN_A, GPIO_IN);
    gpio_pull_up(ENC_PIN_A);
    gpio_set_irq_enabled_with_callback(ENC_PIN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_init(ENC_PIN_B);
    gpio_set_dir(ENC_PIN_B, GPIO_IN);
    gpio_pull_up(ENC_PIN_B);

    ec_tmr = get_mstmr(_500ms);
#endif

    //   Init SPI0 module (for support display UC1609C)
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
    gpio_init(LCD_HIDE_PIN);
    gpio_set_dir(LCD_HIDE_PIN, GPIO_OUT);
    gpio_put(LCD_HIDE_PIN, 0);//for show/hide display ; 0-show, 1-hide

    //  init font's pointer
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
    UC1609C_enable(1);//display ON
    uint8_t contrast = 40;
    UC1609C_contrast(contrast);
    UC1609C_clearDisplay();
    //
    int dl = sprintf(tmp, "%s", ver);
    mkLineCenter(tmp, mfnt->FontWidth);
    uint16_t x = 1;
    UC1609C_Print(x, UC1609C_HEIGHT - mfnt->FontHeight, tmp, mfnt, 0, FOREGROUND);
    //
    UC1609C_DrawFilledRectangle(0, 0, UC1609C_WIDTH - 1, hfnt->FontHeight - 1, FOREGROUND);
    UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    UC1609C_update();
    //
    uint8_t lfr = lfnt->FontHeight;
    for (int8_t i = 0; i < 5; i++) {
    	lines[i] = hfnt->FontHeight + 1 + (i * lfr);
    }
    lines[line5]++;
    //
    bool flag_ver = false;
    uint8_t tmr_ver = 0;

    evt_t ev;
    int evt;
    int queCnt = 0;
    int next_evt = cmdNone;
    char stz[64];
    char stx[64];
    char st[64];
    char sta[64];
    char stb[64];
    char stn[64];

    //   init rda5807m RF chip
    rda5807_delay(250);
    rdaID = rda5807_init(&Freq, Band, Step);
    rda5807_delay(250);

    rda5807_SetVolume(Volume);
    rda5807_SetBassBoost(BassBoost);
  	rda5807_Get_ChanRssiFlag(&Chan, &RSSI, &stereo);
    if (rdaID == 0x58) {
    	sprintf(st, "RDA5807 cID:0x%x", rdaID);
    } else {
    	sprintf(st, "Unknown cID:0x%x", rdaID);
    }
    Report(1, "%s\n", st);
    rda5807_delay(50);

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

    if (rdaID == RDA5807_CHIP) {// если успешно опознан чип rda5807m устанвливаем новую частоту из newFreq
    	ev.cmd = cmdFreq;
    	ev.attr = 0;
    	if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    }


#ifdef SET_JOYSTIC

    //   Стартуем в ядре № 1 функцию, обслуживающую джойстик
    multicore_launch_core1(joystik_task);
    //

#endif


    uint32_t sleep_tmr = 0;

    uint32_t attr = 0;

    while (!restart) {// main loop

    	queCnt = queue_get_level(&evt_fifo);//читаем количество сообщение в очереди

    	if (queCnt) {
    		if (queue_try_remove(&evt_fifo, &ev)) {// читаем очередное сообщение из очереди
    			idx = -1;
    			evt = ev.cmd;
    			attr = ev.attr;
    			if ((evt > cmdNone) && (evt < cmdSec)) {
    				Report(1, "[que:%d] cmd:%d attr:%lu\n", queCnt, ev.cmd, ev.attr);
    				sprintf(stz, "cmd : %s", s_cmds[evt]);
    				mkLineCenter(stz, mfnt->FontWidth);
    				UC1609C_Print(1, lines[line5], stz, mfnt, 0, FOREGROUND);
    				UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    				UC1609C_update();
    				flag_ver = true;
    				tmr_ver = 10;
    			}
    			switch (evt) {
    				case cmdTemp:
    					adc_select_input(4);
    					temperature = read_onboard_temperature('C');
    					Report(1, "[que:%u] onchip temperature:%.02f deg.C\n", queCnt, temperature);
    					sprintf(stz, "Onchip temperature:%.02f deg.C", temperature);
    					mkLineCenter(stz, mfnt->FontWidth);
    					UC1609C_Print(1, lines[line5], stz, mfnt, 0, FOREGROUND);
    					UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    					UC1609C_update();
    					flag_ver = true;
    					tmr_ver = 10;
    				break;
#ifdef SET_ENCODER
    				case cmdEnc:
    				{
    					if (sleepON) {//exit from sleep mode
    						UC1609C_enable(sleepON);//1 - ON    0 - OFF
    						gpio_put(LCD_HIDE_PIN, 0);//ON
    						sleepON = false;
    						indMenu = iExit;
    						Report(1, "Exit from sleep mode !\n");
    					}
    					bool snd = false;
    					switch (indMenu) {
    						case iNone://start menu
    							menuAct = true;
    							indMenu = 0;
    							clrLines(lines[line1], 4, 1, lfnt->FontHeight, BACKGROUND);//FOREGROUND);
    							refreshMenu();
    						break;
    						case iExit://exit from menu
    							menuAct = false;
    							clrLines(lines[line1], 4, 1, lfnt->FontHeight, BACKGROUND);
    							indMenu = -1;
    							ev.cmd = cmdShowFreq;
    							snd = true;
    							//
    							tmr_ver = 1;
    							flag_ver = true;
    						break;
    						case iList://set encMode to List
    							indMenu = iExit;
    							encMode = iList;
    							ev.cmd = cmdEnc;
    							snd = true;
    						break;
    						case iScan://set encMode to Scan
    							indMenu = iExit;
    							encMode = iScan;
    							ev.cmd = cmdEnc;
    							snd = true;
    						break;
    						case iFreq://set encMode to Scan
    							indMenu = iExit;
    							encMode = iFreq;
    							ev.cmd = cmdEnc;
    							snd = true;
    						break;
    						case iVolume://set encMode to Volume
    							indMenu = iExit;
    							encMode = iVolume;
    							ev.cmd = cmdEnc;
    							snd = true;
    						break;
    						case iBass://set encMode to Bass
    							indMenu = iExit;
    							encMode = iBass;
    							ev.cmd = cmdEnc;
    							snd = true;
    						break;
    						case iRestart://goto reload software
    							indMenu = iExit;
    							encMode = iRestart;
    							ev.cmd = cmdRestart;
    							snd = true;
    						break;
    						case iSleep://goto sleep mode
    							indMenu = iExit;
    							encMode = iSleep;
    							ev.cmd = cmdEnc;
    							snd = true;
    							sleep_tmr = 2;
    						break;
    						case iContrast:
    							indMenu = iExit;
    							encMode = iContrast;
    							ev.cmd = cmdEnc;
    							snd = true;
    						break;
    					}
    					if (snd) {
    						ev.attr = 0;
    						if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    					}
    				}
    				break;
    				case cmdInc:
    				{
    					bool ref = false;
    					if (menuAct) {
    						ref = true;
    						if (indMenu >= (MAX_MENU - 1)) {
    							indMenu = 0;
    						} else {
    							indMenu++;
    						}
    					} else {
    						bool snd = false;
    						switch (encMode) {
    							case iList:
    								seek_up = 1;
    								ev.cmd = cmdList;
    								snd = true;
    							break;
    							case iScan:
    								seek_up = 1;
    								ev.cmd = cmdScan;
    								snd = true;
    							break;
    							case iFreq:
    								ev.cmd = cmdFreq;
    								snd = true;
    								newFreq = Freq + allSteps[Step].freq;
    							break;
    							case iVolume:
    								newVolume = Volume + 1;
    								if (newVolume > 15) newVolume = 1;
    								ev.cmd = cmdVol;
    								snd = true;
    							break;
    							case iBass:
    								if (!BassBoost) {
    									newBassBoost = BassBoost + 1;
    									ev.cmd = cmdBass;
    									snd = true;
    								}
    							break;
    							case iContrast:
    								contrast++;
    								UC1609C_contrast(contrast);
    								sprintf(stz, "contrast:%u", contrast);
    								mkLineCenter(stz, lfnt->FontWidth);
    								UC1609C_Print(1, lines[line4], stz, lfnt, 0, FOREGROUND);
    								UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    								UC1609C_update();
    							break;
    						}
    						if (snd) {
    							ev.attr = 0;
    							if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    						}
    					}
    					if (ref) refreshMenu();
    				}
    				break;
    				case cmdDec:
    				{
    					bool ref = false;
    					if (menuAct) {
    						ref = true;
    						if (!indMenu) {
    							indMenu = MAX_MENU - 1;
    						} else {
    							indMenu--;
    						}
    					} else {
    						bool snd = false;
    						switch (encMode) {
    							case iList:
    								seek_up = 0;
    								ev.cmd = cmdList;
    								snd = true;
    							break;
    							case iScan:
    								seek_up = 0;
    								ev.cmd = cmdScan;
    								snd = true;
    							break;
    							case iFreq:
    								ev.cmd = cmdFreq;
    								snd = true;
    								newFreq = Freq - allSteps[Step].freq;
    							break;
    							case iVolume:
    								newVolume = (Volume - 1) & 0xf;
    								if (!newVolume) newVolume = 1;
    								ev.cmd = cmdVol;
    								snd = true;
    							break;
    							case iBass:
    								if (BassBoost) {
    									newBassBoost = BassBoost - 1;
    									ev.cmd = cmdBass;
    									snd = true;
    								}
    							break;
    							case iContrast:
    								contrast--;
    								UC1609C_contrast(contrast);
    								sprintf(stz, "contrast:%u", contrast);
    								mkLineCenter(stz, lfnt->FontWidth);
    								UC1609C_Print(1, lines[line4], stz, lfnt, 0, FOREGROUND);
    								UC1609C_DrawRectangle(0, lfnt->FontHeight, UC1609C_WIDTH - 1, UC1609C_HEIGHT - (lfnt->FontHeight << 1) - 1, 0);
    								UC1609C_update();
    							break;
    						}
    						if (snd) {
    							ev.attr = 0;
    							if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    						}
    					}
    					if (ref) refreshMenu();
    				}
    				break;
#endif
    				case cmdErr:
    					Report(1, "Error input\n");
    				break;
    				case cmdUart:
    					sprintf(tmp, "[que:%u] ", queCnt);
    					if (!uart_enable) strcat(tmp, "Enable");
    					             else strcat(tmp, "Disable");
    					Report(1, "%s uart\n", tmp);
    					if (uart_enable) uart_enable = false; else uart_enable = true;
    				break;
    				case cmdClr:// clear devError and ERR_LED OFF
    					devError = 0;
    					errLedOn(false);
    				break;
    				case cmdCfg:
    					showCfg();
    				break;
    				case cmdMute:
    					noMute = (~noMute) & 1;
    					rda5807_Set_Mute(noMute);
    					sprintf(sta, "BASS:%u", BassBoost);
    					sprintf(stb, "VOLUME:%u", Volume);
    					if (!noMute) strcat(stb, " Mute");
    					mkLineWidth(sta, stb, lfnt->FontWidth);
    					showLine(sta, lines[line2], lfnt, true, FOREGROUND);
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
    							sprintf(stb, "FM Band:%s", allBands[Band]);
    							mkLineCenter(stb, lfnt->FontWidth);
    							showLine(stb, lines[line4], lfnt, true, FOREGROUND);
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
    						sprintf(sta, "BASS:%u", BassBoost);
    						sprintf(stb, "VOLUME:%u", Volume);
    						if (!noMute) strcat(stb, " Mute");
    						mkLineWidth(sta, stb, lfnt->FontWidth);
    						showLine(sta, lines[line2], lfnt, true, FOREGROUND);
    						Report(1, "[que:%u] set new BassBoost to %u\n", queCnt, BassBoost);
    					} else {
    						Report(1, "[que:%u] BassBoost already set to %u\n", queCnt, BassBoost);
    					}
    					break;
    				case cmdVol:
    					Volume = newVolume;
    					rda5807_SetVolume(Volume);
    					sprintf(sta, "BASS:%u", BassBoost);
    					sprintf(stb, "VOLUME:%u", Volume);
    					if (!noMute) strcat(stb, " Mute");
    					mkLineWidth(sta, stb, lfnt->FontWidth);
    					showLine(sta, lines[line2], lfnt, true, FOREGROUND);
    					Report(1, "[que:%u] set new Volume to %u\n", queCnt, Volume);
    				break;
    				case cmdFreq:
    					if ((newFreq >= lBand) && (newFreq <= rBand)) {
    						Freq = newFreq;
    						uint16_t fr = (uint16_t)(Freq * 10);
    						rda5807_SetFreq_In100Khz(fr);
    						rda5807_Get_ChanRssiFlag(&Chan, &RSSI, &stereo);
    						ev.cmd = cmdShowFreq;
    						if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
        					Report(1, "[que:%u] set new Freq to %.1f МГц '%s' Chan:%u Volume:%u\n",
        								  queCnt, Freq, nameStation(Freq, NULL), Chan, Volume);

#ifdef SET_RDS
    						if (rdsFlag) {
    							rds_init();
    							rdsTime = get_mstmr(rdsWait);
    						}
#endif
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
    				case cmdSec:
    				{
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
    							if (encMode == iNone) {
    								dl = sprintf(tmp, "%s", ver);
    							} else {
    								dl = sprintf(tmp, "encMode :%s", allMenu[encMode]);
    							}
    							mkLineCenter(tmp, mfnt->FontWidth);
    							UC1609C_Print(1, UC1609C_HEIGHT - mfnt->FontHeight, tmp, mfnt, 0, FOREGROUND);
    						}
    					}
    					UC1609C_update();
    					//
    					if (sleep_tmr) {
    						sleep_tmr--;
    						if (!sleep_tmr) {
    							UC1609C_enable(sleepON);// Off display //1 - ON    0 - OFF
    							gpio_put(LCD_HIDE_PIN, 1);
    							Report(1, "Goto sleep mode. To wakeUp - press ENC_PIN key !\n");
    							sleepON = true;
    							sleep_goto_dormant_until_pin(ENC_PIN, true, true);
    						}
    					}
    					//
    					if (indMenu >= 0) break;
    					//
    					if (!scan) {
    						uint16_t sig = rda5807_rssi();
    						if (sig != RSSI) {
    							RSSI = sig;
    							stereo = rda5807_Get_StereoMonoFlag();
    							sprintf(sta, "RSSI:%u", RSSI);
    							stb[0] = '\0';
    							if (stereo) strcpy(stb, "Stereo ");
    							sprintf(stb+strlen(stb), "FREQ:%.1f", Freq);
    							mkLineWidth(sta, stb, lfnt->FontWidth);
    							showLine(sta, lines[line1], lfnt, true, FOREGROUND);
    						}
    					} else {
    						if (rda5807_Get_SeekTuneReadyFlag()) {
    							scan = 0;
    							Freq = (float)rda5807_GetFreq_In100Khz();
    							Freq /= 10;
    							rda5807_Get_ChanRssiFlag(&Chan, &RSSI, &stereo);
    							ev.cmd = cmdShowFreq;
    							if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
    							Report(1, "[que:%u] set new Freq to %.1f МГц '%s' Chan:%u Volume:%u\n",
    									  queCnt, Freq, nameStation(Freq, NULL), Chan, Volume);
    							//
#ifdef SET_RDS
    							if (rdsFlag) {
    								rds_init();
    								rdsTime = get_mstmr(rdsWait);
    							}
#endif
    						}
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
    				case cmdShowFreq:
    				{
    					sprintf(sta, "RSSI:%u", RSSI);
    					stb[0] = '\0';
    					if (stereo) strcpy(stb, "Stereo ");
    					sprintf(stb+strlen(stb), "FREQ:%.1f", Freq);
    					mkLineWidth(sta, stb, lfnt->FontWidth);
    					showLine(sta, lines[line1], lfnt, false, FOREGROUND);
    					//
    					sprintf(sta, "BASS:%u", BassBoost);
    					sprintf(stb, "VOLUME:%u", Volume);
    					if (!noMute) strcat(stb, " Mute");
    					mkLineWidth(sta, stb, lfnt->FontWidth);
    					showLine(sta, lines[line2], lfnt, false, FOREGROUND);
    					//
    					int8_t idx = -1;
    					int dlm = sprintf(stn, "%s", nameStation(Freq, &idx));
    					clrLines(lines[line3], 2, 0, lfnt->FontHeight, BACKGROUND);
    					UC1609C_Print(caclX(stn, lfnt->FontWidth), lines[line3], stn, lfnt, 0, FOREGROUND);
    					//
    					UC1609C_DrawLine(0, lines[line4] + lfnt->FontHeight - 1, UC1609C_WIDTH - 1, lines[line4] + lfnt->FontHeight - 1, 0);
    					UC1609C_update();
    				}
    				break;
#ifdef SET_FLASH
    				case cmdCheck:
    					if (isSectorEmpty(adr_sector))
    						Report(1, "[que:%u] Sector %lu is empty\n", queCnt, adr_sector);
    					else
    						Report(1, "[que:%u] Sector %lu Not empty\n", queCnt, adr_sector);
    				break;
    				case cmdErase:
    					if (!isSectorEmpty(adr_sector)) {
    						Flash_EraseSector(adr_sector);
    					} else {
    						Report(1, "[que:%u] Sector %lu already empty\n", queCnt, adr_sector);
    					}
    				break;
    				case cmdRead:
    				case cmdNext:
	#ifdef SET_WITH_DMA
    					if (!dma_chan_init) dma_chan_init = chan_init();
    					if (dma_chan_init) {
	#endif
    						Flash_ReadSector(fs_work, adr_sector, offset_sector, list_sector);
	#ifndef SET_WITH_DMA
    						ev.cmd = cmdReadCont;
    						ev.attr = attr;
    						if (!queue_try_add(&evt_fifo, &ev)) devError |= devQue;
	#else
    					}
	#endif
    				break;
    				case cmdReadCont:
    				{
    					char bs[64];
    					char sym;
    					uint32_t ad = attr;
    					Report(0, "Read sector:%d offset:%d len:%u fadr:0x%X\r\n",
    							adr_sector, offset_sector, list_sector, ad);
    					for (uint32_t i = 0; i < list_sector; i++) {
    						if (!(i % flash_step)) {
    							Report(0, "%08X", ad);
    							ad += flash_step;
    						}
    						if ((i % flash_step) == (flash_step - 1)) {
    							bs[0] = '\0';
    							for (int8_t j = 0; j < flash_step; j++) {
    								sym = (char)fs_work[i - flash_step + 1 + j];
    								if ((sym >= 0x20) && (sym < 0x7f))
    									sprintf(bs+strlen(bs), "%c", sym);
    								else
    									sprintf(bs+strlen(bs), ".");
    							}
    							Report(0, " %02X %s\n", fs_work[i], bs);
    						} else {
    							Report(0, " %02X", fs_work[i]);
    						}
    					}
    				}
    				break;
#endif
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
    								if (!menuAct) {
    									mkLineCenter(st, lfnt->FontWidth);
    									UC1609C_Print(1, lines[line4], st, lfnt, 0, FOREGROUND);
    									UC1609C_DrawLine(0, lines[line4] + lfnt->FontHeight - 1, UC1609C_WIDTH - 1, lines[line4] + lfnt->FontHeight - 1, 0);
    									UC1609C_update();
    								}
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
    	if (cmd_tmr) {
    		if (check_mstmr(cmd_tmr)) {
    			cmd_tmr = 0;
    			gpio_put(LED_CMD_PIN, 0);//LED_CMD_PIN OFF
    		}
    	}
    	//
    	if (devError) {
    		errLedOn(true);
    		sleep_ms(25);
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

    UC1609C_enable(0);//OFF

#ifdef SET_JOYSTIC
    //wait joystic_task closed.....
    uint8_t sch = 255;
    while (joy && sch) {
    	sleep_ms(1);
    };
    sleep_ms(10);
#endif



    //restart
    watchdog_reboot(0, SRAM_END, 0);

    return 0;
}
