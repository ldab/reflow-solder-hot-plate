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
  analogReadResolution(12); // 0-4096
  analogSetPinAttenuation(ADC_ATTEN_DB_6);
  adcAttachPin(TEMP_ANALOG);
}

float lmt85_temp()
{
  uint32_t adcMv = analogReadMilliVolts(TEMP_ANALOG);
	// equation (2) product datasheet
  float temp = (8.194 - sqrt( pow(-8.194, 2) + 4 * 0.00262 * (1324 - adcMv))) / (2 * -0.00262) + 30;
	return temp;
}