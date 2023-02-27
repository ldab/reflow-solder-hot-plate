#include <Arduino.h>


#include "gpio.h"
#include "lmt85.h"
#include "temp_control.h"
#include "webserver.h"

float temp     = 0;
uint8_t status = 0;

std::vector<float> readings;
std::vector<long> epocTime;

static void webServerConnectedCb()
{
  reflow_profile cfg = TEMP_CONTROL_DEFAULT();
  temp_control_config(&cfg);
  temp_control_start(&temp);
}

void setup(void)
{
#ifdef CALIBRATE
  // Measure GPIO in order to determine Vref to gpio 25 or 26 or 27
  adc2_vref_to_gpio(GPIO_NUM_25);
  delay(5000);
  abort();
#endif

  gpios_init();

  Serial.begin(115200);
  Serial.printf("Version: %s\n", VERSION);

  // Serial.setDebugOutput(true);

  lmt85_init();
  lmt85_start(&temp);

  webserver_start(&readings, &epocTime, &temp, &webServerConnectedCb);
}

void loop(void) {}
