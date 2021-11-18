#include "main.h"

#include "chprintf.h"

#define ENVIRONMENTALS_COUNT    1

environmental_t environmentals_array[ENVIRONMENTALS_COUNT] = {
  {
    .name = "CH2-FAKE",
    .temperature = 0.0,
    .humidity = 0.0,
    /* TODO: Environmental Sensor Drivers Config */
  }
};
environmentals_t hub_environmentals = {
  .environmental = environmentals_array,
  .count = ENVIRONMENTALS_COUNT
};

static float sensor_vbat_voltage = 0.0;
static float sensor_cpu_temperature = 0.0;

static uint16_t ts_cal1; // DEV board: 0x2FDC
static uint16_t ts_cal2; // DEV board: 0x3FDB

const GPTConfig sensors_gptcfg = {
  .frequency    =  20000U,
  .callback     =  NULL,
  .cr2          =  TIM_CR2_MMS_1,   /* MMS = 010 = TRGO on Update Event.    */
  .dier         =  0U
};

#define SENSOR_ADC_CHANNELS   2
#define SENSOR_ADC_SAMPLES    32

/* ADC3 uses BDMA so needs to be in this RAM slot */
static adcsample_t sensors_samples[SENSOR_ADC_CHANNELS * SENSOR_ADC_SAMPLES] CC_SECTION(".ram4_clear");

void sensor_adccallback(ADCDriver *adcp)
{
  int32_t sensor_vbat_voltage_result = 0;
  int32_t sensor_cpu_temperature_result = 0;

  cacheBufferInvalidate(sensors_samples, sizeof(sensors_samples));

  if(adcIsBufferComplete(adcp))
  {
    /* Second half of buffer ready */
    for(int32_t i=(SENSOR_ADC_SAMPLES/4); i<(SENSOR_ADC_SAMPLES/2); i++)
    {
      sensor_vbat_voltage_result += sensors_samples[(i*2)];
    }
    for(int32_t i=(SENSOR_ADC_SAMPLES/4); i<(SENSOR_ADC_SAMPLES/2); i++)
    {
      sensor_cpu_temperature_result += sensors_samples[(i*2)+1];
    }
  }
  else
  {
    /* First half of buffer ready */
    for(int32_t i=0; i<(SENSOR_ADC_SAMPLES/4); i++)
    {
      sensor_vbat_voltage_result += sensors_samples[(i*2)];
    }
    for(int32_t i=0; i<(SENSOR_ADC_SAMPLES/4); i++)
    {
      sensor_cpu_temperature_result += sensors_samples[(i*2)+1];
    }
  }
  sensor_vbat_voltage_result /= (SENSOR_ADC_SAMPLES / 4);
  sensor_cpu_temperature_result /= (SENSOR_ADC_SAMPLES / 4);

  sensor_vbat_voltage = 3.3 * ((float)(sensor_vbat_voltage_result * 4) / 0xFFFF);
  sensor_cpu_temperature = ((110.0-30.0) / (float)(ts_cal2 - ts_cal1)) * (float)(sensor_cpu_temperature_result - ts_cal1) + 30.0;
}

static const ADCConversionGroup sensors_adcgrpcfg = {
  .circular     = true,
  .num_channels = SENSOR_ADC_CHANNELS,
  .end_cb       = sensor_adccallback,
  .error_cb     = NULL,
  .cfgr         = ADC_CFGR_EXTEN_RISING | ADC_CFGR_EXTSEL_SRC(12),
  .cfgr2        = 0U,
  .ccr          = 0U,
  .pcsel        = ADC_SELMASK_IN17 | ADC_SELMASK_IN18,
  .ltr1         = 0x00000000U,
  .htr1         = 0x03FFFFFFU,
  .ltr2         = 0x00000000U,
  .htr2         = 0x03FFFFFFU,
  .ltr3         = 0x00000000U,
  .htr3         = 0x03FFFFFFU,
  .smpr         = {
    0U,
    ADC_SMPR2_SMP_AN17(ADC_SMPR_SMP_810P5)
    | ADC_SMPR2_SMP_AN18(ADC_SMPR_SMP_810P5)
  },
  .sqr          = {
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN17)
    | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN18),
    0U,
    0U,
    0U
  }
};

THD_FUNCTION(environmentals_service_thread, arg)
{
  (void)arg;

  uint32_t i;
  
  /* Extract Temperature Sensor calibration coeffs */
  ts_cal1 = *((uint16_t *)((uint32_t)0x1FF1E820));
  ts_cal2 = *((uint16_t *)((uint32_t)0x1FF1E840));
  
  /* Enable temperature sensor */
  adcStart(&ADCD3, NULL);
  adcSTM32EnableTS(&ADCD3);
  adcSTM32EnableVBAT(&ADCD3);

  /* Enable GPT to trigger ADC */
  gptStart(&GPTD4, &sensors_gptcfg);

  adcStartConversion(&ADCD3, &sensors_adcgrpcfg, sensors_samples, SENSOR_ADC_SAMPLES);
  gptStartContinuous(&GPTD4, 500); // 2Hz sample rate

  while(1)
  {
    for(i = 0; i < hub_environmentals.count; i++)
    {
      /* TODO: Environmental Sensor Drivers */
      hub_environmentals.environmental[i].temperature = sensor_cpu_temperature;

      //hub_environmentals.environmental[i].temperature = Temperature_readCPU();

      hub_environmentals.environmental[i].humidity = 60.0 + (hwrandom(200)/10.0);
    }

    chThdSleepMilliseconds(100);

    //debugPrintf("[Sensor] VBAT: %.3f\r\n", sensor_vbat_voltage);
  }

  gptStopTimer(&GPTD4);
  adcStopConversion(&ADCD3);

  /* Disable temperature sensor */
  adcSTM32DisableTS(&ADCD3);
  adcSTM32DisableVBAT(&ADCD3);
  adcStop(&ADCD3);
}