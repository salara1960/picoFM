
FM radio based on Raspberry Pi Pico + RDA5807 + SSD1306 + NS8002 + TL1838 + KY-023

#################################################
#
#            picoFM - FM Radio
#
#################################################


## Состав рабочего оборудования:

```
* RDA5807, RF receiver FM
* Raspberry Pi Pico (Waveshare board)
* ssd1306 OLED display 128x32
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
* Устройство предназначено для прослушивания радио станций ФМ диапазона (65 - 108 Мгц)
* ПО построено с использованием программных средств SDK 'pico-sdk'.
  События обслуживаются в основном цикле программы. Формируются события в callBack-функциях
  по завершении прерываний от используемых модулей микроконтроллера.
* Устройство инициализирует некоторые интерфейсы микроконтроллера :
  - ADC : Chan0(GP26) и Chan1(GP27) аналогово-цифровой преобразователь (измеряет напряжение на аналоговых выходах джойстика).
  - GPIO : подключены два сетодиода : GP29 - секундный тик, GP8 - индикатор ошибки на устройстве,
           GP7 - светодиод индикации появления очередного события в очереди, GP14 - пин приема данных от
           инфракрасного датчика TL1838, GP15 - пин обслуживает кнопку джойстка в режиме прерывания.
  - I2C0 : режим мастера с частотой 400Кгц (шина ослуживает чипы rda5807, ssd1306).
  - USART1 : параметры порта 230400 8N1 - порт для логов и передачи команд устройству.
* Прием данных по последовательному порту (USART1) выполняется в callback-функции обработчика прерывания.

После подачи питания или нажатия на кнопку 'Reset' начинается выполнение загрузчика.
При успешном запуске в порту USART1 появятся следующие сообщения :


```
24.08.22 12:32:32 | Start picoRadio application Ver.0.8.1 24.08.22
24.08.22 12:32:32 | Create queue for 16 events OK
24.08.22 12:32:32 | Start timer with 10 ms period.
24.08.22 12:32:33 | RDA5807 cID:0x58
24.08.22 12:32:34 | [que:2] RDS monitoring start
24.08.22 12:32:34 | [que:1] set new Freq to 95.100 МГц 'VestiFM' (Chan:191) with volume 6
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

restart
24.08.22 14:34:38 | [que:1] cmd:1 attr:0
24.08.22 14:34:38 | Queue released
24.08.22 14:34:38 | Timer cancelled... 1

24.08.22 12:32:32 | Start picoRadio application Ver.0.8.1 24.08.22
24.08.22 12:32:32 | Create queue for 16 events OK
24.08.22 12:32:32 | Start timer with 10 ms period.
24.08.22 12:32:33 | RDA5807 cID:0x58
24.08.22 12:32:34 | [que:2] RDS monitoring start
24.08.22 12:32:34 | [que:1] set new Freq to 95.100 МГц 'VestiFM' (Chan:191) with volume 6
```

