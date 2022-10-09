#include "hdr.h"



#include "rda5807.h"

#ifdef SET_RDA_DEBUG
        uint8_t buf10[64];
        uint8_t buf11[64];
#endif
#ifdef SET_PICO
	#define i2cRDA portRDA
#else
    I2C_HandleTypeDef *i2cRDA = &hi2c1;
#endif
//
Buffs_t Buffs;
BufAB_t BufAB;
tRadioState RadioState = Idle;
tRadioState RadioStateOld = Idle;
//
//==============================================================================
void rda5807_delay(uint32_t t)
{
#ifdef SET_PICO
	sleep_ms(t);
#else
	HAL_Delay(t);
#endif
}
//==============================================================================
//  Чтение ID микросхемы
//==============================================================================
uint8_t rda5807_readID()
{
uint8_t buf[2] = {0, 0};

#ifdef SET_PICO
	i2c_write_blocking(i2cRDA, RDA5807_RandAccess_Addr, buf, 1, true);
	i2c_read_blocking(i2cRDA, RDA5807_RandAccess_Addr, buf, sizeof(buf), false);
#else
   if (HAL_I2C_Mem_Read(i2cRDA, RDA5807_RandAccess_Addr, 0, I2C_MEMADD_SIZE_8BIT, buf, 2, RDA5807_TO) != HAL_OK) {
	   devError |= devRDA;
   }
#endif
#ifdef SET_RDA_DEBUG
    Report(1, "[%s] data: 0x%02x 0x%02x\r\n", __func__, buf[0], buf[1]);
#endif
    return buf[0];
}

//==============================================================================
// Инициализация rda5807
//==============================================================================
uint8_t rda5807_init(float *freq, uint8_t band, uint8_t step)
{
uint8_t buf[2] = {0, 0};
uint8_t *id = &buf[0];

#ifdef SET_PICO
	i2c_write_blocking(i2cRDA, RDA5807_RandAccess_Addr, buf, 1, true);
	if (i2c_read_blocking(i2cRDA, RDA5807_RandAccess_Addr, buf, sizeof(buf), false) <= 0) {
		goto err_out;
	}
#else
    if (HAL_I2C_Mem_Read(i2cRDA, RDA5807_RandAccess_Addr, 0, I2C_MEMADD_SIZE_8BIT, buf, 2, RDA5807_TO) != HAL_OK) {
    	goto err_out;
    }
#endif

#ifdef SET_RDA_DEBUG
    	memset(buf10, 0, sizeof(buf10));
    	memset(buf11, 0, sizeof(buf11));
	#ifdef SET_PICO
    	char stx[256] = {0};
    	//i2c_write_blocking(i2cRDA, RDA5807_SeqAccess_Addr, buf11, 2, true);
    	//i2c_read_blocking(i2cRDA, RDA5807_SeqAccess_Addr, buf11, 64, false);
    	uint16_t sz = sizeof(Buffs_t);
    	uint8_t *bf = (uint8_t *)calloc(1, sz);
    	if (bf) {
    		i2c_read_blocking(i2cRDA, RDA5807_SeqAccess_Addr, bf, sz, false);
    		for (int i = 0; i < sz>>1; i++) sprintf(stx+strlen(stx)," %02x%02x", bf[i], bf[i+1]);
    		Report(1, "[%s] %s\n", __func__, stx);
    		free(bf);
    	}
	#else
    	if (HAL_I2C_Mem_Read(i2cRDA, RDA5807_RandAccess_Addr, 0, I2C_MEMADD_SIZE_8BIT, buf11, 64, RDA5807_TO) != HAL_OK) {
    		goto err_out;
    	}
    	if (HAL_I2C_Master_Receive(i2cRDA, RDA5807_SeqAccess_Addr, buf10, 64, RDA5807_TO) != HAL_OK) {
    		goto err_out;
    	}
    	for (int i = 0; i < 64; i++) sprintf(stx+strlen(stx)," %02x", buf10[i]);
    	Report(1, "[%s] %s\n", __func__, stx);
    	stx[0] = '\0';
    	for (int i = 0; i < 64; i++) sprintf(stx+strlen(stx)," %02x", buf11[i]);
    	Report(1, "[%s] %s\n", __func__, stx);
	#endif
#endif

    rda5807_SoftReset();
    if (devError & devRDA) goto err_out;
    rda5807_delay(50);

    rda5807_SetupDefault(band, step);
    if (devError & devRDA) goto err_out;
    rda5807_delay(250);

    uint16_t fr = (uint16_t)(*freq * 10);
    rda5807_SetFreq_In100Khz(fr);
    rda5807_delay(250);
    //
    *freq = (float)rda5807_GetFreq_In100Khz();
    *freq /= 10;
#ifdef SET_RDA_DEBUG
    Report(1, "[%s] Freq: %.2f\n", __func__, *freq);//rda5807_GetFreq_In100Khz());
#endif

    //rda5807_delay(100);

    return *id;

err_out:

	devError |= devRDA;

    return *id;
}
//==============================================================================
uint8_t rda5807_rssi()
{
uint16_t word = 0;

    rda5807_read(0x0B, &word, 1);

    return (word >> 9);
}
//==============================================================================


//==============================================================================
// Процедура меняет местами байты попарно в буфере pBuff 
//==============================================================================
void rda5807_bytes_change(uint8_t *pBuff, uint8_t count)
{
    while (count > 1) {    // Если осталась хотя бы пара байт
        uint8_t Temp = *(pBuff + 1);
        *(pBuff + 1) = *pBuff;
        *pBuff = Temp;
        pBuff += 2;
        count -= 2;
    }
}
//==============================================================================


//==============================================================================
// Процедура читает из rda5807 группу регистров (кол-во RegNum) начиная с 0x0A
// Используется I2C-адрес RDA5807_SeqAccess_Addr
//==============================================================================
void rda5807_read_regfile(uint16_t *pBuff, uint8_t RegNum)
{
#ifdef SET_PICO
	//uint8_t reg = 2;
	//i2c_write_blocking(i2cRDA, RDA5807_RandAccess_Addr, &reg, 1, true);
    if (i2c_read_blocking(i2cRDA, RDA5807_SeqAccess_Addr, (uint8_t *)pBuff, RegNum << 1, false) <= 0) {
    	devError |= devRDA;
    }

#else
	if (HAL_I2C_Master_Receive(i2cRDA, (uint16_t)RDA5807_SeqAccess_Addr, (uint8_t *)pBuff, RegNum << 1, RDA5807_TO) != HAL_OK) {
        devError |= devRDA;
    }
#endif

    rda5807_bytes_change((uint8_t *)pBuff, RegNum << 1);
}
//==============================================================================


//==============================================================================
// Процедура пишет в rda5807 группу регистров (кол-во RegNum) начиная с 0x02
// Используется I2C-адрес RDA5807_SeqAccess_Addr
//==============================================================================
void rda5807_write_regfile(uint16_t *pBuff, uint8_t RegNum)
{

    rda5807_bytes_change((uint8_t *)pBuff, RegNum << 1);

#ifdef SET_PICO
    //uint8_t reg = 2;
    //i2c_write_blocking(i2cRDA, RDA5807_RandAccess_Addr, &reg, 1, true);
    if (i2c_write_blocking(i2cRDA, RDA5807_SeqAccess_Addr, (uint8_t *)pBuff, RegNum << 1, false) <= 0) {
    	devError |= devRDA;
    }
#else
    if (HAL_I2C_Master_Transmit(i2cRDA, (uint16_t)RDA5807_SeqAccess_Addr, (uint8_t *)pBuff, RegNum << 1, RDA5807_TO) != HAL_OK) {
    	devError |= devRDA;
    }
#endif

    rda5807_bytes_change((uint8_t *)pBuff, RegNum << 1);
}
//==============================================================================


//==============================================================================
// Процедура читает из rda5807 группу регистров (кол-во RegNum) начиная с RegAddr
// Используется I2C-адрес RDA5807_RandAccess_Addr (для режима совместимости с rda5800)
//==============================================================================
void rda5807_read(uint8_t RegAddr, uint16_t *pBuff, uint8_t RegNum)
{
#ifdef SET_PICO
	i2c_write_blocking(i2cRDA, RDA5807_RandAccess_Addr, &RegAddr, 1, true);
	if (i2c_read_blocking(i2cRDA, RDA5807_RandAccess_Addr, (uint8_t *)pBuff, RegNum << 1, false) <= 0) {
		devError |= devRDA;
	}

#else
    if (HAL_I2C_Mem_Read(i2cRDA, RDA5807_RandAccess_Addr,
    				     RegAddr, I2C_MEMADD_SIZE_8BIT,
						 (uint8_t *)pBuff, RegNum << 1,
						 RDA5807_TO) != HAL_OK) {
    	devError |= devRDA;
    }
#endif

    rda5807_bytes_change((uint8_t *)pBuff, RegNum << 1);
}
//==============================================================================


//==============================================================================
// Процедура пишет в rda5807 группу регистров (кол-во RegNum) начиная с RegAddr
// Используется I2C-адрес RDA5807_RandAccess_Addr (для режима совместимости с rda5800)
//==============================================================================
void rda5807_write(uint8_t RegAddr, uint16_t *pBuff, uint8_t RegNum)
{

    rda5807_bytes_change((uint8_t *)pBuff, RegNum << 1);

#ifdef SET_PICO
    uint8_t *buf = (uint8_t *)calloc(1, (RegNum << 1) + 1);
    if (buf) {
    	buf[0] = RegAddr;
    	memcpy(buf + 1, pBuff, RegNum << 1);
    	if (i2c_write_blocking(i2cRDA, RDA5807_RandAccess_Addr, buf, (RegNum << 1) + 1, false) <= 0) {
    		devError |= devRDA;
    	}
    	free(buf);
    }
#else
   if (HAL_I2C_Mem_Write(i2cRDA,
    					  RDA5807_RandAccess_Addr,
						  RegAddr, I2C_MEMADD_SIZE_8BIT,
						  (uint8_t *)pBuff, RegNum << 1,
						  RDA5807_TO) != HAL_OK) {
	   devError |= devRDA;
   }
#endif

    rda5807_bytes_change((uint8_t *)pBuff, RegNum << 1);
}
//==============================================================================


//==============================================================================
// Процедура делает программный сброс rda5807
//==============================================================================
void rda5807_SoftReset()
{
//tReg02h reg2;

    rda5807_read(2, (uint16_t *)&Buffs.Reg02, 1);

    Buffs.Reg02.bENABLE = 1;
    Buffs.Reg02.bSOFT_RESET = 1;
    rda5807_write(2, (uint16_t *)&Buffs.Reg02, 1);

    rda5807_delay(5);

    //Buffs.Reg02.bENABLE = 1;
    Buffs.Reg02.bSOFT_RESET = 0;
    rda5807_write(2, (uint16_t *)&Buffs.Reg02, 1);

    rda5807_delay(5);
}
//==============================================================================


//==============================================================================
// Процедура производит начальную настройку rda5807
//==============================================================================
void rda5807_SetupDefault(uint8_t band, uint8_t step)
{
    // Регистр 0x02
	Buffs.Reg02.bENABLE = 1;
	Buffs.Reg02.bSOFT_RESET = 0;
	Buffs.Reg02.bNEW_METHOD = 1;
	Buffs.Reg02.bRDS_EN = 1;
	Buffs.Reg02.bCLK_MODE = 0;        // 32.768
	Buffs.Reg02.bSKMODE = 0;
	Buffs.Reg02.bSEEK = 0;
	Buffs.Reg02.bSEEKUP = 1;
	Buffs.Reg02.bRCLK_DirectInput = 0;
	Buffs.Reg02.bRCLK_NonCalibMode = 0;
	Buffs.Reg02.bBASS = 0;
	Buffs.Reg02.bMONO = 0;
	Buffs.Reg02.bDMUTE = 1;
	Buffs.Reg02.bDHIZ = 1;
	// Регистр 0x03
	Buffs.Reg03.bSPACE = step;//0;   // Шаг настройки - 0 = 100 КГц
	Buffs.Reg03.bBAND = band;//2;//0;    // Диапазон 2 - 76–108 MHz
	Buffs.Reg03.bTUNE = 1;
	Buffs.Reg03.bDIRECT_MODE = 0;
	Buffs.Reg03.bCHAN = 0;
	// Регистр 0x04
#ifdef SET_RDA_FP
	Buffs.Reg04.bGPIO1 = 0;//    // 0-1bits General Purpose I/O 1.
								 //00 = High impedance.
								 //01 = Reserved
								 //10 = Low
								 //11 = High
	Buffs.Reg04.bGPIO2 = 0;//    // 2-3bits General Purpose I/O 2.
								 //00 = High impedance.
								 //01 = Interrupt (INT)
								 //10 = Low.
								 //11 = High
	Buffs.Reg04.bGPIO3 = 0;//    // 4-5bits General Purpose I/O 3.
								 //00 = High impedance.
								 //01 = Mono/Stereo indicator (ST)
								 //10 = Low.
								 //11 = High
	Buffs.Reg04.bI2S_ENABLED = 0;// 6bit I2S bus enable. If 0, disabled. If 1, enabled.
	Buffs.Reg04.bRSVD1 = 0;      // 7bit Reserved
	Buffs.Reg04.bAFCD = 0;       // 8bit AFC disable (0 = afc work; 1 = afc disabled)
	Buffs.Reg04.bSOFTMUTE_EN = 1;// 9bit 1 = softmute enable
	Buffs.Reg04.bRSVD2 = 0;      // 10bit Reserved
	Buffs.Reg04.bDE = 0;         // 11bit De-emphasis (0 = 75 µs; 1 = 50 µs)
	Buffs.Reg04.bRSVD3 = 0;      // 12-13bits Reserved
	Buffs.Reg04.bSTCIEN = 0;     // FOR RDA5807FP : Seek/Tune Complete Interrupt Enable.0
    							 //0 = Disable Interrupt
    							 //1 = Enable Interrupt
    							 //Setting STCIEN = 1 will generate a low pulse on
    							 //GPIO2 when the interrupt occurs.
	Buffs.Reg04.bRSVD3 = 0;      // 15bit Reserved

#else
	Buffs.Reg04.bRSVD1 = 0;
	Buffs.Reg04.bAFCD = 0;
	Buffs.Reg04.bSOFTMUTE_EN = 1;
	Buffs.Reg04.bRSVD2 = 0;
	Buffs.Reg04.bDE = 0;
	Buffs.Reg04.bRSVD3 = 0;
#endif
	// Регистр 0x05
	Buffs.Reg05.bVOLUME = 0;
	Buffs.Reg05.bANT_GAIN = 0;
	Buffs.Reg05.bANT_TYPE = ANT_TYPE_Both;//ANT_TYPE_Both;//ANT_TYPE_External;//ANT_TYPE_Headphones;
	Buffs.Reg05.bSEEKTH = 6;//8;
	Buffs.Reg05.bRSVD3 = 0;
	Buffs.Reg05.bINT_MODE = 1;
	// Регистр 0x06
	Buffs.Reg06.bRSVD1 = 0;
	Buffs.Reg06.bOPEN_MODE = 0;
	Buffs.Reg06.bRSVD2 = 0;
	// Регистр 0x07
	Buffs.Reg07.bFREQ_MODE = 0;
	Buffs.Reg07.bSOFTBLEND_EN = 1;
	Buffs.Reg07.bSEEK_TH_OLD = 0;
	Buffs.Reg07.bRSVD1 = 0;
	Buffs.Reg07.b65M_50M_MODE = 1;
	Buffs.Reg07.bTH_SOFRBLEND = 16;
	Buffs.Reg07.bRSVD2 = 0;

	// Пишем регистры функцией записи регистрового файла
	rda5807_write_regfile((uint16_t *)&(Buffs.Reg02), 6);
}
//==============================================================================


//==============================================================================
// Процедура устанавливает уровень громкости (0..16) выхода rda5807. При Value=0 включает MUTE
//==============================================================================
void rda5807_SetVolume(uint8_t Value)
{
//tReg02h Reg2;
//tReg05h Reg5;
uint8_t Mute = Value ? 0 : 1;

    if (Value > 16) Value = 16;

    Value--;    // Значение для поля Volume на 1 меньше, чем входной параметр функции

    if (!Mute) {
        // Читаем регистр 0x05
        rda5807_read(5, (uint16_t *)&Buffs.Reg05, 1);
        // Меняем значение поля VOLUME
        Buffs.Reg05.bVOLUME = Value;
        // Пишем регистр 0x05
        rda5807_write(5, (uint16_t *)&Buffs.Reg05, 1);
    }

    // Читаем регистр 0x02
    rda5807_read(2, (uint16_t *)&Buffs.Reg02, 1);
    // Меняем значение поля VOLUME
    Buffs.Reg02.bDMUTE = Mute ? 0 : 1;
    // Пишем регистр 0x02
    rda5807_write(2, (uint16_t *)&Buffs.Reg02, 1);
}
//==============================================================================


//==============================================================================
// Процедура включает/выключает BassBoost
//==============================================================================
void rda5807_SetBassBoost(uint8_t Value)
{
//tReg02h Reg2;

    // Читаем регистр 0x02
    rda5807_read(2, (uint16_t *)&Buffs.Reg02, 1);
    // Меняем значение поля BASS
    Buffs.Reg02.bBASS = (Value) ? 1 : 0;
    // Пишем регистр 0x02
    rda5807_write(2, (uint16_t *)&Buffs.Reg02, 1);
}
//==============================================================================


//==============================================================================
// Процедура устанавливает текущую частоту Freq100kHz и стартует перенастройку rda5807 на эту частоту.
// Окончание процесса можно установки можно проконтроллировать по обнулению бита STR в регистре 0x0A (функцией rda5807_Get_SeekTuneReadyFlag)
//==============================================================================
void rda5807_SetFreq_In100Khz(uint16_t Freq100kHz)
{
//tReg03h Reg3;
uint16_t l = 870, r = 1080;

	rda5807_read(3, (uint16_t *)&Buffs.Reg03, 1);
    // Проверка входного параметра для диапазона Buffs.Reg03.bBAND
	switch (Buffs.Reg03.bBAND) {    // Диапазон 87–108 MHz
		//case 0:// 00 = 87–108 MHz (US/Europe)
		//break;
		case 1:// 01 = 76–91 MHz (Japan)
			l = 760;
			r = 910;
		break;
		case 2:// 10 = 76–108 MHz (world wide)
			l = 760;
		break;
		case 3:// 11 = 65 –76 MHz (East Europe) or 50-65MHz
			l = 650;
			r = 760;
		break;
	}
	lBand = ((float)l) / 10;
	rBand = ((float)r) / 10;

    if (Freq100kHz < l)  Freq100kHz = l;
    if (Freq100kHz > r) Freq100kHz = r;

    // Вычитаем начало диапазона (left)
    Freq100kHz -= l;

    // Читаем регистр 0x03
    //rda5807_read(3, (uint16_t *)&Buffs.Reg03, 1);
    // Меняем значение поля CHAN
    Buffs.Reg03.bCHAN = Freq100kHz;
    // Выставляем флаг начала перенастройки на канал
    Buffs.Reg03.bTUNE = 1;
    Buffs.Reg03.bSPACE = Step;
    // Пишем регистр 0x03
    rda5807_write(3, (uint16_t *)&Buffs.Reg03, 1);

    rda5807_delay(100);
}
//==============================================================================


//==============================================================================
// Функция читает текущую частоту, на которую настроен rda5807
//==============================================================================
uint16_t rda5807_GetFreq_In100Khz()
{
	tReg0Ah reg;
    rda5807_read(0x0A, (uint16_t *)&reg, 1);
    uint16_t Freq100kHz = reg.bREADCHAN;
    if (Freq100kHz == 319) return 0;

    // Прибавляем начало диапазона (87 МГц)
    uint16_t left = 870;
    //tReg03h reg3;
    rda5807_read(3, (uint16_t *)&Buffs.Reg03, 1);
    switch (Buffs.Reg03.bBAND) {    // Диапазон 87–108 MHz
    	case 1:// 01 = 76–91 MHz (Japan)
    		left = 760;
    	break;
    	case 2:// 10 = 76–108 MHz (world wide)
    		left = 760;
    	break;
    	case 3:// 11 = 65 –76 MHz (East Europe) or 50-65MHz
    		left = 650;
    	break;
    }
    Freq100kHz += left;

    return Freq100kHz;
}
//==============================================================================


//==============================================================================
// Процедура стартует поиск радиостанции вверх/вниз
//==============================================================================
void rda5807_StartSeek(uint8_t Up)
{
//tReg02h Reg2;

    // Читаем регистр 0x02
    rda5807_read(2, (uint16_t *)&Buffs.Reg02, 1);

    Buffs.Reg02.bSKMODE = 1;          // 07 Seek Mode (0 = wrap at the upper or lower band limit and continue seeking; 1 = stop seeking at the upper or lower band limit)
    Buffs.Reg02.bSEEK = 1;            // 08 Seek (0 = Disable stop seek; 1 = Enable)
    Buffs.Reg02.bSEEKUP = Up ? 1 : 0; // 09 Seek Up (0 = Seek down; 1 = Seek up)

    // Пишем регистр 0x02
    rda5807_write(2, (uint16_t *)&Buffs.Reg02, 1);
}
//==============================================================================


//==============================================================================
// Функция возвращает состояние бита STR (SeekTuneReadyFlag)
// SeekTuneReadyFlag=1 пока идёт процесс настройки на частоту или поиск радиостанции.
//==============================================================================
uint8_t rda5807_Get_SeekTuneReadyFlag()
{
tReg0Ah Reg0A;

    rda5807_read(0x0A, (uint16_t *)&Reg0A, 1);

    return Reg0A.bSTC;
}
//==============================================================================
//==============================================================================
bool rda5807_Get_StereoMonoFlag()
{
tReg0Ah Reg0A;

    // Читаем регистр 0x0A
    rda5807_read(0x0A, (uint16_t *)&Reg0A, 1);

    return (bool)Reg0A.bST;     // Stereo Indicator (0 = Mono; 1 = Stereo)
}
//==============================================================================
uint16_t rda5807_Get_Channel()
{
tReg0Ah Reg0A;

    // Читаем регистр 0x0A
    rda5807_read(0x0A, (uint16_t *)&Reg0A, 1);

    return Reg0A.bREADCHAN;    // 0-9 Read Channel.
}
//==============================================================================
void rda5807_Get_ChanRssiFlag(uint16_t *chan, uint8_t *rssi, bool *sm)
{
	// Читаем регистры 0x0A, 0x0B
    rda5807_read(0x0A, (uint16_t *)&BufAB.Reg0A, 2);

    *chan = (uint16_t)BufAB.Reg0A.bREADCHAN;    // 0-9 Read Channel.

    *rssi = (uint8_t)BufAB.Reg0B.bRSSI;

    *sm = (bool)BufAB.Reg0A.bST;     // Stereo Indicator (0 = Mono; 1 = Stereo)
}
//==============================================================================
uint8_t rda5807_Set_Band(uint8_t band)
{
	if (band > 3) return 1;

	//tReg03h Reg3;
    rda5807_read(3, (uint16_t *)&Buffs.Reg03, 1);
    Buffs.Reg03.bBAND = band;

    uint16_t l = 870, r = 1080;
    switch (Buffs.Reg03.bBAND) {    // Диапазон 87–108 MHz
    	//case 0:// 00 = 87–108 MHz (US/Europe)
    	//break;
    	case 1:// 01 = 76–91 MHz (Japan)
    		l = 760;
    		r = 910;
    	break;
    	case 2:// 10 = 76–108 MHz (world wide)
    		l = 760;
    	break;
    	case 3:// 11 = 65 –76 MHz (East Europe) or 50-65MHz
    		l = 650;
    		r = 760;
    	break;
    }
    lBand = ((float)l) / 10;
    rBand = ((float)r) / 10;

    rda5807_write(3, (uint16_t *)&Buffs.Reg03, 1);

    rda5807_delay(100);

    return 0;
}
//==============================================================================
void rda5807_Set_Mute(uint8_t mute)
{
	rda5807_read(2, (uint16_t *)&Buffs.Reg02, 1);

	Buffs.Reg02.bDMUTE = mute & 1;

	rda5807_write(2, (uint16_t *)&Buffs.Reg02, 1);
}
//==============================================================================
bool rda5807_Get_RDSReady()
{
	tReg0Ah Reg0A;
	rda5807_read(0x0A, (uint16_t *)&Reg0A, 1);

	if (Reg0A.bRDSS && Reg0A.bRDSR)
		return true;
	else
		return false;
}
//==============================================================================
uint16_t rda5807_Get_reg(uint8_t reg)
{
uint16_t ret = 0;

	rda5807_read(reg, &ret, 1);

	return ret;
}
//==============================================================================
void rda5807_Set_Step(uint8_t step)
{
	rda5807_read(3, (uint16_t *)&Buffs.Reg03, 1);

	Buffs.Reg03.bSPACE = step & 3;

	rda5807_write(3, (uint16_t *)&Buffs.Reg03, 1);
//==============================================================================
}

