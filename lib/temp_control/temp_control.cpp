#include "temp_control.h"

typedef enum { IDLE = 0, PREHEAT, SOAK, REFLOW, COOLING } state;
reflow_profile profile;

static float compute(float setpoint, float input)
{
  static uint32_t lastTime = 0;
  static float errSum = 0, lastErr = 0;

  /*How long since we last calculated*/
  uint32_t now        = millis();
  uint32_t timeChange = now - lastTime;

  /*Compute all the working error variables*/
  float error         = setpoint - input;
  errSum += (error * timeChange);
  double dErr       = (error - lastErr) / timeChange;

  /*Remember some variables for next time*/
  lastErr           = error;
  lastTime          = now;

  float output_pc   = KP * error + KI * errSum + KD * dErr;
  float output_bits = output_pc * (pow(2, PWM_RESOLUTION) - 1);
  log_d("output_pc %.1f", output_pc);
  return output_bits;
}

void temp_control_config(reflow_profile *cfg)
{
  profile.preheat_temp = cfg->preheat_temp;
  profile.preheat_rate = cfg->preheat_rate;
  profile.soak_temp    = cfg->soak_temp;
  profile.soak_rate    = cfg->soak_rate;
  profile.soak_temp    = cfg->soak_temp;
  profile.soak_rate    = cfg->soak_rate;

  analogWriteFrequency(PWM_FREQUENCY);
  analogWriteResolution(PWM_RESOLUTION); // LOG(80000000/Freq, 2) = 10.6
}

static void TaskControl(void *pvParameters)
{
  uint8_t state          = PREHEAT;
  uint32_t start_time    = millis();
  uint32_t duty          = 0;
  float temp             = *((float *)pvParameters);
  float current_setpoint = 20;

  for (;;) // A Task shall never return or exit.
  {
    switch (state) {
    case PREHEAT:
      if (temp > profile.preheat_temp) {
        state = SOAK;
      } else {
        current_setpoint += profile.preheat_rate;
        duty = compute(current_setpoint, temp);
        analogWrite(FET_GATE, LIMIT(duty, PWM_MAX_COLD));
      }
      break;
    case SOAK:
      if (temp > profile.soak_temp) {
        state      = REFLOW;
        start_time = millis();
      } else {
        current_setpoint += profile.soak_rate;
        duty = compute(current_setpoint, temp);
        analogWrite(FET_GATE, LIMIT(duty, PWM_MAX_HOT));
      }
      break;
    case REFLOW:
      if ((millis() - start_time) > (profile.reflow_time * 1000)) {
        state = COOLING;
      } else {
        current_setpoint = profile.reflow_temp;
        duty    = compute(current_setpoint, temp);
        analogWrite(FET_GATE, LIMIT(duty, PWM_MAX_HOT));
      }
      break;
    case COOLING:
    default:
      break;
    }

    log_d("state: %d setpoint: %.1f duty: %d", state, current_setpoint, duty);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  vTaskDelete(NULL);
}

void temp_control_start(float *temp)
{
  xTaskCreate(TaskControl, "TaskControl", 4096, temp, 5, NULL);
}