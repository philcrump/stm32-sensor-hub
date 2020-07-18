# Sensor Hub [![Build Status](https://travis-ci.org/philcrump/stm32-sensor-hub.svg?branch=master)](https://travis-ci.org/philcrump/stm32-sensor-hub)

This project exposes the status of connected sensors on a HTTP Webpage & API, using the ChibiOS (20.3.x) and lwIP (2.1.x) software stacks.

The hardware components are:

* ST STM32F429ZI ARM Cortex-M4 Development Board - [Product Link](https://www.st.com/en/evaluation-tools/nucleo-f429zi.html)

<p float="center">
  <img src="/images/web-screenshot.png" width="49%" />
</p>

## API

`/api/status`
* `{"contacts": [{"name": "CH1", "value": 0}], "environmentals": [{"name": "CH2-FAKE", "temperature": 17.60, "humidity": 65.50}]}`

## Software Compilation

This toolchain is designed for Linux.

You must have `arm-none-eabi-gcc` from [GNU ARM embedded toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) in your path.

Running `firmware/build -M` will pack the web files and then compile all source files in parallel. The final linker stage can take a few seconds. Running without the `-M` flag will remove parallel compilation and may help errors to be more readable.

## Flashing

`firmware/flash` will flash a compiled firmware image. This first looks for a connected BlackMagic JTAG probe, and if unsuccessful will drop back to `st-flash` from [st-utils](https://github.com/stlink-org/stlink) to flash over the ST-Link USB.

`firmware/build -MF` will build, and if successful also attempt to flash.

## Hardware

### Dry Contacts

Currently only a proof-of-concept dry-contact input is configured on pin PC7 (ZIO header pin D21) with internal pull-up to wet a demonstration input contact between PC7 and board GND.

For proper use the Dry Contacts inputs should be used with an opto-isolater to protect the inputs from static discharge and inter-equipment voltage potentials. An isolated DC-DC should also be used for each channel to generate the wetting voltage. Example components would be:

* Lite-On LTV-357T 3750VDC Isolation Optoisolator - [Datasheet](https://optoelectronics.liteon.com/upload/download/DS70-2001-012/LTV-357T%20series%20201606.pdf)
* Traco Power TBA 1-0512 5V in, 12V out, 1W, 1000VDC Isolation DC-DC Converter - [Product Page](https://www.tracopower.com/int/model/tba-1-0512)

Additional input pins must be configured in `firmware/cfg/board.h`, and must then be added to the contacts array at the top of `firmware/contacts.c`.

### Environmental Sensors

For the environmental sensors no interface is currently configured, however ChibiOS libraries include a driver for the ST HTS221 Temperature & Humidity Sensor which is commonly available on breakout boards. [Sensor Datasheet](https://www.st.com/resource/en/datasheet/hts221.pdf), [ChibiOS Documentation](http://chibiforge.org/doc/20.3/ex/group___h_t_s221.html).

Another approach would be to implement a RS-422 multidrop bus, pairing each sensor with an inexpensive microcontroller and PHY to facilitate polling communication, then allowing far greater distances (1km+) between the Hub board and the Sensors.

# Copyright

GPLv3 licensed. © Phil Crump - phil@philcrump.co.uk

Derivations from other license-compatible works are acknowledged in the source.