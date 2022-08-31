M radio based on Raspberry Pi Pico + RDA5807 + UC1609C + NS8002 + TL1838 + KY-023 + EC11 encoder

#################################################
#
#            picoFM - FM Radio
#
#################################################


## Состав рабочего оборудования:

```
* RDA5807, RF receiver FM
* Raspberry Pi Pico (Waveshare board)
* UC1609C LCD display 192x64
* NS8002 chip - audio power amplifier
* speaker 8 Om 3 W
* KY-023 - joystick, control device
* TL1838 Infrared Receiver
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
* Устройство предназначено для прослушивания радио станций ФМ диапазона (65 - 108 Мгц),
  а также приема и отображения информации RDS от радио станций FM диапазона.
* ПО построено с использованием программных средств SDK 'pico-sdk'.
  События обслуживаются в основном цикле программы. Формируются события в callBack-функциях
  по завершении прерываний от используемых модулей микроконтроллера.
* Устройство инициализирует некоторые интерфейсы микроконтроллера :
  - ADC : Chan0(GP26) и Chan1(GP27) аналогово-цифровой преобразователь (измеряет напряжение на аналоговых выходах джойстика).
  - GPIO : подключены три светодиода : GP29 - секундный тик, GP8 - индикатор ошибки на устройстве,
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
28.08.22 15:14:14 | Start picoRadio application Ver.2.1 28.08.22 multicore
28.08.22 15:14:14 | Create queue for 16 events OK
28.08.22 15:14:14 | Start timer with 10 ms period.
28.08.22 15:14:15 | RDA5807 cID:0x58
28.08.22 15:14:16 | Start 'joystik_task' function on Core1
28.08.22 15:14:16 | [que:2] RDS monitoring start
28.08.22 15:14:16 | [que:1] set new Freq to 95.100 МГц 'VestiFM' Chan:191 Volume:6
```

* Через USART1 можно отправлять команды на устройство, например :

```
ver
24.08.22 12:33:11 | [que:1] cmd:3 attr:0
24.08.22 12:33:11 | Ver.0.8.1 24.08.22

help
24.08.22 12:33:44 | [que:1] cmd:0 attr:0
        help
        restart
        epoch:
        ver
        input_err
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

epoch:1661351278
24.08.22 12:34:32 | [que:1] cmd:2 attr:1661351278

mute
24.08.22 14:28:36 | [que:1] cmd:6 attr:0
24.08.22 14:28:36 | [que:1] set Mute to 1
mute
24.08.22 14:28:45 | [que:1] cmd:6 attr:0
24.08.22 14:28:45 | [que:1] set Mute to 0

scan
24.08.22 14:29:26 | [que:1] set new Freq to 95.500 МГц RetroFM (Chan:195) with volume 6

list
24.08.22 14:30:15 | Band = newBand = 2 -> goto set newFreq to 96.300 (up = 1)
24.08.22 14:30:15 | [que:1] set new Freq to 96.300 МГц 'RusRadio' (Chan:203) with volume 6

vol:up
24.08.22 14:31:11 | [que:1] set new Volume to 7
vol:down
24.08.22 14:31:17 | [que:1] set new Volume to 6

freq:105.9
24.08.22 14:32:07 | [que:1] set new Freq to 105.900 МГц 'RoadRadio' (Chan:299) with volume 6

rds
24.08.22 14:32:36 | [que:1] RDS monitoring stop
rds
24.08.22 14:32:39 | [que:1] RDS monitoring start

bass:1
24.08.22 14:33:47 | [que:1] set new BassBoost to 1
bass:0
24.08.22 14:33:53 | [que:1] set new BassBoost to 0

cfg
3:68.5:Majak
3:72.1:Shanson
2:92.8:RadioDFM
2:93.6:Radio7
2:94.0:ComedyRadio
2:95.1:VestiFM
2:95.5:RetroFM
2:96.3:RusRadio
2:97.0:RadioVera
2:97.7:SilverRain
2:98.5:RadioEnegry
2:99.5:RadioStar
2:100.1:AutoRadio
2:100.6:RusContry
2:100.9:MonteCarlo
2:101.3:OurRadio
2:101.8:BusinessFM
2:102.5:Majak
2:102.9:LoveRadio
2:103.4:Studio21
2:103.9:RadioRussian
2:104.5:Europe+
2:105.2:Baltic+
2:105.9:RoadRadio
2:106.4:RadioMaxim
2:107.2:RadioKP

restart
28.08.22 15:32:24 | [que:1] cmd:1 attr:0
28.08.22 15:32:24 | Queue released
28.08.22 15:32:24 | Queue released
28.08.22 15:32:24 | Timer cancelled... 1

28.08.22 15:14:14 | Start picoRadio application Ver.2.1 28.08.22 multicore
28.08.22 15:14:14 | Create queue for 16 events OK
28.08.22 15:14:14 | Start timer with 10 ms period.
28.08.22 15:14:15 | RDA5807 cID:0x58
28.08.22 15:14:16 | Start 'joystik_task' function on Core1
28.08.22 15:14:16 | [que:2] RDS monitoring start
28.08.22 15:14:16 | [que:1] set new Freq to 95.100 МГц 'VestiFM' Chan:191 Volume:6
```
