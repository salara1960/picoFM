FM radio based on Raspberry Pi Pico + RDA5807 + UC1609C + NS8002 + TL1838 + KY-023 + EC11 encoder

#################################################
#
#            picoFM - FM Radio
#
#################################################


## Состав рабочего оборудования:

```
* RDA5807, RF receiver FM
* Raspberry Pi Pico (Waveshare-Zero board)
* UC1609C LCD display 192x64
* PAM8003 chip - 5W Class-D stereo audio amplifier
* speaker 8 Om 3 W
* KY-023 - joystick, control device
* TL1838 Infrared Receiver
* EC11 - rotary encoder
```


# Средства разработки:

```
* pico-sdk - sdk development applications for Raspberry Pi Pico
  (https://github.com/raspberrypi/pico-sdk, https://raspberrypi.github.io/pico-sdk-doxygen/index.html)
* Eclipse IDE for Embedded C/C++ - IDE software development environment
  (https://www.eclipse.org/)
* pico-project-generator-master - python script for create project for Raspberry Pi Pico
```


# Функционал:
* Устройство предназначено для прослушивания радиостанций ФМ диапазона (65 - 108 Мгц),
  а также приема и отображения информации RDS от радиостанций FM диапазона.
* ПО построено с использованием программных средств SDK 'pico-sdk'.
  События обслуживаются в основном цикле программы. Формируются события, как правило, в callBack-функциях
  по завершении прерываний от используемых модулей микроконтроллера.
* Устройство инициализирует некоторые интерфейсы микроконтроллера :
  - ADC : Chan0(GP26) и Chan1(GP27) аналогово-цифровой преобразователь (измеряет напряжение на аналоговых выходах джойстика).
  - GPIO : подключены два светодиода : GP29 - секундный тик, GP8 - индикатор ошибки на устройстве,
           GP4,GP5,GP6,GP7 (DC,RST,SCK,MOSI) - пины интерфейса SPI0, GP14 - пин приема данных от
           инфракрасного датчика TL1838, GP15 - пин обслуживает кнопку джойстка в режиме прерывания,
           GP10,GP11,GP13 - обслуживают энкодер EC11 в режиме прерывания.
  - I2C1 : режим мастера с частотой 400Кгц (шина ослуживает чипы rda5807).
  - SPI0 : режим мастера с частотой 12Мгц (шина ослуживает LCD display UC1609C).
  - USART1 : параметры порта 230400 8N1 - порт для логов и передачи команд устройству.
  - Core1 задействовано для обслуживания джойстика KY-023.
* Прием данных по последовательному порту (USART1) выполняется в callback-функции обработчика прерывания.

После подачи питания или нажатия на кнопку 'Reset' начинается выполнение загрузчика.
При успешном запуске в порту USART1 появятся следующие сообщения :


```
31.08.22 23:58:27 | Start picoRadio app Ver.2.4.1 31.08.22 multicore (BoardID:0xE66138935F374B29 temp:33.69 deg.C)
31.08.22 23:58:27 | All clocks are set to:
        pll_sys : 0 kHz
        pll_usb : 48000 kHz
        rosc    : 5731 kHz
        clk_sys : 48000 kHz
        clk_peri: 48000 kHz
        clk_usb : 48000 kHz
        clk_adc : 48000 kHz
        clk_rtc : 47 kHz
31.08.22 23:58:27 | Create queue for 16 events OK
31.08.22 23:58:27 | Start timer with 5 ms period.
31.08.22 23:58:29 | RDA5807 cID:0x58
31.08.22 23:58:29 | Start 'joystik_task' function on Core1
31.08.22 23:58:29 | [que:3] RDS monitoring start
31.08.22 23:58:29 | [que:2] set new Freq to 95.100 МГц 'Вести ФМ' Chan:191 Volume:6 idx=5
31.08.22 23:58:29 | [que:1] Freq change deny
```

* Через USART1 можно отправлять команды на устройство, например :

```
ver
1.09.22 00:01:48 | [que:1] cmd:3 attr:0
01.09.22 00:01:48 | Ver.2.4.1 31.08.22 multicore

help
01.09.22 00:02:21 | [que:1] cmd:0 attr:0
        help
        restart
        epoch:
        ver
        input_err
        clr
        uart
        mute
        sec
        temp
        enc
        encinc
        encdec
        cfg
        scan
        vol:
        bass:
        band:
        freq:
        list
        rds

epoch:1662032931
01.09.22 11:48:52 | [que:1] cmd:2 attr:1662032931

mute
01.09.22 11:49:29 | [que:1] cmd:7 attr:0
01.09.22 11:49:29 | [que:1] set Mute to 1
mute
01.09.22 11:49:31 | [que:1] cmd:7 attr:0
01.09.22 11:49:31 | [que:1] set Mute to 0

scan
01.09.22 11:50:35 | [que:1] set new Freq to 95.500 МГц Ретро ФМ Chan:195 Volume:6 idx=6

list
01.09.22 11:51:15 | Band = newBand = 2 -> goto set newFreq to 95.500 (up = 1)
01.09.22 11:51:15 | [que:1] set new Freq to 95.500 МГц 'Ретро ФМ' Chan:195 Volume:6 idx=6

vol:up
01.09.22 11:51:48 | [que:1] set new Volume to 7
vol:down
01.09.22 11:51:53 | [que:1] set new Volume to 6

freq:105.9
01.09.22 11:52:23 | [que:1] set new Freq to 105.900 МГц 'Дорожное Радио' Chan:299 Volume:6 idx=23

rds
01.09.22 11:52:47 | [que:1] RDS monitoring stop
rds
01.09.22 11:52:50 | [que:1] RDS monitoring start

bass:1
01.09.22 11:53:17 | [que:1] set new BassBoost to 1
bass:0
01.09.22 11:53:21 | [que:1] set new BassBoost to 0

cfg
3:68.5:Маяк
3:72.1:Шансон
2:92.8:Радио DFM
2:93.6:Радио 7
2:94.0:Комеди Радио
2:95.1:Вести ФМ
2:95.5:Ретро ФМ
2:96.3:Русское Радио
2:97.0:Радио Вера
2:97.7:Серебр.Дождь
2:98.5:Радио Энергия
2:99.5:Радио Звезда
2:100.1:Авто Радио
2:100.6:Русский Край
2:100.9:Монте-Карло
2:101.3:Наше Радио
2:101.8:Бизнес ФМ
2:102.5:Маяк
2:102.9:Ўбимое Радио
2:103.4:Студия 21
2:103.9:Радио России
2:104.5:Европа Плюс
2:105.2:Балтик Плюс
2:105.9:Дорожное Радио
2:106.4:Радио Максим
2:107.2:Радио КП

temp
01.09.22 11:54:29 | [que:1] onchip temperature:30.88 deg.C

01.09.22 11:55:13 | Band = newBand = 2 -> goto set newFreq to 95.100 (up = 0)
01.09.22 11:55:13 | [que:1] set new Freq to 95.100 МГц 'Вести ФМ' Chan:191 Volume:5 idx=5

restart
01.09.22 11:55:52 | [que:1] cmd:1 attr:0
01.09.22 11:55:52 | Queue released
001.09.22 11:55:52 | Timer cancelled... 1

31.08.22 23:58:27 | Start picoRadio app Ver.2.4.1 31.08.22 multicore (BoardID:0xE66138935F374B29 temp:33.69 deg.C)
31.08.22 23:58:27 | All clocks are set to:
        pll_sys : 0 kHz
        pll_usb : 48000 kHz
        rosc    : 5726 kHz
        clk_sys : 48000 kHz
        clk_peri: 48000 kHz
        clk_usb : 48000 kHz
        clk_adc : 48000 kHz
        clk_rtc : 47 kHz
31.08.22 23:58:27 | Create queue for 16 events OK
31.08.22 23:58:27 | Start timer with 5 ms period.
31.08.22 23:58:29 | RDA5807 cID:0x58
31.08.22 23:58:29 | Start 'joystik_task' function on Core1
31.08.22 23:58:29 | [que:3] RDS monitoring start
31.08.22 23:58:29 | [que:2] set new Freq to 95.100 МГц 'Вести ФМ' Chan:191 Volume:6 idx=5
31.08.22 23:58:29 | [que:1] Freq change deny
```
