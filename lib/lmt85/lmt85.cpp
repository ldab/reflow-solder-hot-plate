#include "lmt85.h"

void lmt85_init()
{
  /*
  Measurable input voltage range
  ADC_ATTEN_DB_0      0 mV ~ 950 mV
  ADC_ATTEN_DB_2_5    0 mV ~ 1250 mV
  ADC_ATTEN_DB_6      0 mV ~ 1750 mV
  ADC_ATTEN_DB_11     0 mV ~ 3100 mV
  */
  adcAttachPin(TEMP_ANALOG);
  analogReadResolution(12); // 0-4095
  analogSetAttenuation(ADC_11db);
}

float lmt85_temp()
{
  uint32_t adcMv = analogReadMilliVolts(TEMP_ANALOG);
  float temp     = 0;
  for (size_t i = 0; i < 10; i++) {
    // equation (2) product datasheet
    temp +=
        (8.194 - sqrt(pow(-8.194, 2) + 4 * 0.00262 * (1324 - (float)adcMv))) /
            (2 * -0.00262) + 30;
  }
  temp = temp / 10;
  log_d("adc: %dmV temp: %f", adcMv, temp);
  return temp;
}

static void TaskTemp(void *pvParameters)
{
  float *temp = (float *)pvParameters;
  lmt85_temp(); // the first reading is weird, ignore it

  for (;;) // A Task shall never return or exit.
  {
    *temp = lmt85_temp();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
  vTaskDelete(NULL);
}

void lmt85_start(float *temp)
{
  xTaskCreate(TaskTemp, "TaskTemp", 4096, temp, 1, NULL);
}