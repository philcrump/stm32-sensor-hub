# Sensor Hub [![Build Status](https://travis-ci.org/philcrump/stm32-sensor-hub.svg?branch=master)](https://travis-ci.org/philcrump/stm32-sensor-hub)

This firmware is written for the ST NUCLEO-STM32F429ZI Development Board.

The firmware exposes a web server on port 80, through which the status of the sensors can be monitored.

This project is built on ChibiOS 20.3.x with LwIP 2.1.x

<p float="center">
  <img src="/images/web-screenshot.png" width="49%" />
</p>

## API

`/api/status`
* `{"contacts": [{"name": "CH1", "value": 0}], "environmentals": [{"name": "CH2-FAKE", "temperature": 17.60, "humidity": 65.50}]}`

## TODO

