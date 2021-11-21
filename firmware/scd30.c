#include "main.h"

#include <string.h>

scd30_t hub_scd30 = {
  .valid = false,
  .fwver = { 0 },
  .instant_co2_ppm = 0.0,
  .instant_temperature_c = 0.0,
  .instant_humidity_rh = 0.0,
  .avg_co2_ppm = 0.0,
  .avg_temperature_c = 0.0,
  .avg_humidity_rh = 0.0
};

scd30_history_t hub_scd30_history = {
  .started = false,
  .head = 0,
  .rolled = false
};

// 2s / 5% = 40 seconds 't-factor' (not window!)
#define AVG_SMOOTH_FACTOR   (0.95)

static const uint16_t i2c_address = 0x61;
static I2CDriver *i2cp = &I2CD1;

CC_ALIGN_DATA(32) static uint8_t txbuf[128];
CC_ALIGN_DATA(32) static uint8_t rxbuf[128];

static const I2CConfig i2ccfg = {
  .timingr = STM32_TIMINGR_PRESC(0x6)
      | STM32_TIMINGR_SCLL(0xC7)
      | STM32_TIMINGR_SCLH(0xC3)
      | STM32_TIMINGR_SDADEL(0x2)
      | STM32_TIMINGR_SCLDEL(0x4),
  .cr1 = 0,
  .cr2 = 0
};

#define AMBIENT_PRESSURE_MB   1013

static uint8_t scd30_crc8(const uint8_t *data, uint8_t length)
{
  uint8_t crc = 0xFF;
  uint8_t extract;
  uint8_t sum;
  for(uint8_t i = 0; i < length; i++)
  {
    extract = *data;
    for (uint8_t tempI = 8; tempI; tempI--) 
    {
      sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum)
      {
        crc ^= 0x31;
      }
      extract >>= 1;
    }
    data++;
  }
  return crc;
}

static void scd30_init(void)
{
  i2cStart(i2cp, &i2ccfg);

  i2cAcquireBus(i2cp);

  /* Soft Reset */
  txbuf[0] = 0xD3;
  txbuf[1] = 0x04;
  cacheBufferFlush(&txbuf[0], sizeof txbuf);

  i2cMasterTransmitTimeout(i2cp, i2c_address, txbuf, 2, NULL, 0, TIME_MS2I(500));

  chThdSleepMilliseconds(50);

  /* Trigger continuous measurement */
  txbuf[0] = 0x00;
  txbuf[1] = 0x10;
  txbuf[2] = 0x00;
  txbuf[3] = 0x00;
  txbuf[4] = 0x81;
  //txbuf[2] = ((AMBIENT_PRESSURE_MB >> 8) & 0xFF);
  //txbuf[3] = (AMBIENT_PRESSURE_MB & 0xFF);
  //txbuf[4] = scd30_crc8(txbuf, 4);
  cacheBufferFlush(&txbuf[0], sizeof txbuf);

  i2cMasterTransmitTimeout(i2cp, i2c_address, txbuf, 5, NULL, 0, TIME_MS2I(500));

  chThdSleepMilliseconds(5);

  /* Read Firmware Version */
  txbuf[0] = 0xD1;
  txbuf[1] = 0x00;
  cacheBufferFlush(&txbuf[0], sizeof txbuf);

  i2cMasterTransmitTimeout(i2cp, i2c_address, txbuf, 2, NULL, 0, TIME_MS2I(500));

  chThdSleepMilliseconds(5);

  i2cMasterReceiveTimeout(i2cp, i2c_address, rxbuf, 3, TIME_MS2I(500));
  cacheBufferInvalidate(&rxbuf[0], sizeof rxbuf);

  i2cReleaseBus(i2cp);

  memcpy(hub_scd30.fwver, rxbuf, 2); // Ignoring CRC for now
}

static bool scd30_co2_measurement(float *co2_ppm, float *temperature_c, float *humidity_rh)
{
  msg_t msg;

  i2cAcquireBus(i2cp);

  /* Get Data Ready */
  txbuf[0] = 0x02;
  txbuf[1] = 0x02;
  cacheBufferFlush(&txbuf[0], sizeof txbuf);

  msg = i2cMasterTransmitTimeout(i2cp, i2c_address, txbuf, 2, NULL, 0, TIME_MS2I(500));

  chThdSleepMilliseconds(5);

  i2cMasterReceiveTimeout(i2cp, i2c_address, rxbuf, 3, TIME_MS2I(500));
  cacheBufferInvalidate(&rxbuf[0], sizeof rxbuf);

  if(msg != MSG_OK)
  {
    i2cReleaseBus(i2cp);
    return false;
  }

  if(((rxbuf[0] << 8) | rxbuf[1]) == 0x0000)
  {
    /* Not ready */
    i2cReleaseBus(i2cp);
    return false;
  }

  chThdSleepMilliseconds(5);

  /* Read Measurement */
  txbuf[0] = 0x03;
  txbuf[1] = 0x00;
  cacheBufferFlush(&txbuf[0], sizeof txbuf);

  msg = i2cMasterTransmitTimeout(i2cp, i2c_address, txbuf, 2, NULL, 0, TIME_MS2I(500));

  chThdSleepMilliseconds(5);

  i2cMasterReceiveTimeout(i2cp, i2c_address, rxbuf, 19, TIME_MS2I(500));
  cacheBufferInvalidate(&rxbuf[0], sizeof rxbuf);

  i2cReleaseBus(i2cp);

  if(msg != MSG_OK)
  {
    return false;
  }

  uint32_t temp_u32;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

  if(co2_ppm != NULL)
  {
    temp_u32 = (rxbuf[0] << 24) | (rxbuf[1] << 16) | (rxbuf[3] << 8) | rxbuf[4];
    *co2_ppm = *(float *)&temp_u32;
  }

  if(temperature_c != NULL)
  {
    temp_u32 = (rxbuf[6] << 24) | (rxbuf[7] << 16) | (rxbuf[9] << 8) | rxbuf[10];
    *temperature_c = *(float *)&temp_u32;
  }

  if(humidity_rh != NULL)
  {
    temp_u32 = (rxbuf[12] << 24) | (rxbuf[13] << 16) | (rxbuf[15] << 8) | rxbuf[16];
    *humidity_rh = *(float *)&temp_u32;
  }

#pragma GCC diagnostic pop

  return true;
}

static virtual_timer_t scd_history_vt;
static void scd_history_cb(struct ch_virtual_timer *_vt, void *arg)
{
  (void)_vt;
  (void)arg;

  if(hub_scd30.valid == true)
  {
    if(hub_scd30_history.copy_lock == true)
    {
      /* Currently being copied, likely by web, we reschedule ISR */
      chSysLockFromISR();
      chVTSetI(&scd_history_vt, TIME_MS2I(100), scd_history_cb, NULL);
      chSysUnlockFromISR();
      return;
    }

    if(hub_scd30_history.started)
    {
      hub_scd30_history.head++;
      if(hub_scd30_history.head >= SCD30_HISTORY_ROW_COUNT)
      {
        hub_scd30_history.head = 0;
        hub_scd30_history.rolled = true;
      }
    }

    hub_scd30_history.rows[hub_scd30_history.head].co2_ppm = hub_scd30.avg_co2_ppm;
    hub_scd30_history.rows[hub_scd30_history.head].temperature_c = hub_scd30.avg_temperature_c;
    hub_scd30_history.rows[hub_scd30_history.head].humidity_rh = hub_scd30.avg_humidity_rh;

    hub_scd30_history.started = true;
  }

  /* Run timer again */
  chSysLockFromISR();
  chVTSetI(&scd_history_vt, TIME_S2I(30), scd_history_cb, NULL);
  chSysUnlockFromISR();
}

THD_FUNCTION(scd30_service_thread, arg)
{
  (void)arg;
  float co2_ppm, temperature_c, humidity_rh;

  scd30_init();

  chThdSleepMilliseconds(1000);

  chVTObjectInit(&scd_history_vt);
  chVTSet(&scd_history_vt, TIME_S2I(30), scd_history_cb, NULL);

  while(1)
  {
    if(scd30_co2_measurement(&co2_ppm, &temperature_c, &humidity_rh)
      && co2_ppm > 50.0) // Validation - first valid measurement is zero?
    {
      /* Valid reading */
      hub_scd30.instant_co2_ppm = co2_ppm;
      hub_scd30.instant_temperature_c = temperature_c;
      hub_scd30.instant_humidity_rh = humidity_rh;

      /* Update or initialise average */
      if(hub_scd30.valid)
      {
        hub_scd30.avg_co2_ppm =
          ((1.0-AVG_SMOOTH_FACTOR) * hub_scd30.instant_co2_ppm)
          + (AVG_SMOOTH_FACTOR * hub_scd30.avg_co2_ppm);
        hub_scd30.avg_temperature_c =
          ((1.0-AVG_SMOOTH_FACTOR) * hub_scd30.instant_temperature_c)
          + (AVG_SMOOTH_FACTOR * hub_scd30.avg_temperature_c);
        hub_scd30.avg_humidity_rh =
          ((1.0-AVG_SMOOTH_FACTOR) * hub_scd30.instant_humidity_rh)
          + (AVG_SMOOTH_FACTOR * hub_scd30.avg_humidity_rh);
      }
      else
      {
        hub_scd30.avg_co2_ppm = hub_scd30.instant_co2_ppm;
        hub_scd30.avg_temperature_c = hub_scd30.instant_temperature_c;
        hub_scd30.avg_humidity_rh = hub_scd30.instant_humidity_rh;
      }
      hub_scd30.valid = true;
    }

    chThdSleepMilliseconds(100);
  }
}