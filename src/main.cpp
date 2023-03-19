#include <Arduino.h>


#include "gpio.h"
#include "lmt85.h"
#include "temp_control.h"
#include "webserver.h"

float temp     = 0;
uint8_t status = 0;

std::vector<float> readings;
std::vector<long> epocTime;

static void webServerConnectedCb(/*void *cfg*/)
{
  set_led(RED, SLOW);
  reflow_profile cfg = TEMP_CONTROL_DEFAULT();
  temp_control_config(&cfg);

  // reflow_profile *_cfg = (reflow_profile *)cfg;
  // temp_control_config(_cfg);
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
  log_i("APP version: %s", VERSION);

  // Serial.setDebugOutput(true);

  lmt85_init();
  lmt85_start(&temp);

  webserver_start(&readings, &epocTime, &temp, &webServerConnectedCb);
}

void loop(void) {}
