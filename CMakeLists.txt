# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/alarm/PICO/pi-pico/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picoFM C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(picoFM
        picoFM.c
        libs.c
        rda5807.c
        fonts.c
        UC1609C.c
        sleep.c)


pico_set_program_name(picoFM "picoFM")
pico_set_program_version(picoFM "2.3")

pico_enable_stdio_uart(picoFM 1)
pico_enable_stdio_usb(picoFM 1)


# Add any user requested libraries
target_link_libraries(picoFM
        pico_stdlib
        pico_multicore
        pico_unique_id
        pico_sync
        hardware_gpio
        hardware_i2c
        hardware_dma
        hardware_watchdog
        hardware_uart
        hardware_rtc
        hardware_irq
        hardware_adc
        hardware_spi
        hardware_pll
        hardware_clocks
        hardware_rosc
        hardware_flash
        hardware_sync
        )

#        //hardware_pio


pico_add_extra_outputs(picoFM)

