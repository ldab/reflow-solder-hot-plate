#include "temp_control.h"
#include "driver/ledc.h"

typedef enum { IDLE = 0, PREHEAT, SOAK, REFLOW, COOLING } state;
reflow_profile profile;

static float compute(float setpoint, float input)
{
  static float last_input = 0, output_sum = 0;
  float error  = setpoint - input;
  float dInput = (input - last_input);

  output_sum += (KI * error);
  output_sum -= KP * dInput;

  last_input      = input;

  float output_pc = output_sum - KD * dInput;
  log_d("output_pc %.1f", output_pc);
  log_v("t: %.1f, st: %.1f", input, setpoint);
  if (output_pc < 0)
    output_pc = 0;
  return output_pc;
}

void temp_control_config(reflow_profile *cfg)
{
  profile.preheat_temp = cfg->preheat_temp;
  profile.preheat_rate = cfg->preheat_rate;
  profile.soak_temp    = cfg->soak_temp;
  profile.soak_rate    = cfg->soak_rate;
  profile.reflow_temp  = cfg->reflow_temp;
  profile.reflow_time  = cfg->reflow_time;

  pinMode(FET_GATE, OUTPUT);

  ledcSetup(3, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(FET_GATE, 3);
  ledcWrite(3, 255);
}

static void TaskControl(void *pvParameters)
{
  uint8_t state       = PREHEAT;
  uint32_t start_time = millis();
  uint32_t duty       = 0;

  for (;;) // A Task shall never return or exit.
  {
    float temp                    = *((float *)pvParameters);
    static float current_setpoint = temp;
    switch (state) {
    case PREHEAT:
      if (temp > profile.preheat_temp) {
        state = SOAK;
      } else {
        current_setpoint += profile.preheat_rate;
        duty = compute(current_setpoint, temp);
        duty = LIMIT(duty, PWM_MAX_COLD);
        ledcWrite(3, duty);
      }
      break;
    case SOAK:
      if (temp > profile.soak_temp) {
        state      = REFLOW;
        start_time = millis();
      } else {
        current_setpoint += profile.soak_rate;
        duty = compute(current_setpoint, temp);
        duty = LIMIT(duty, PWM_MAX_HOT);
        ledcWrite(3, duty);
      }
      break;
    case REFLOW:
      if ((millis() - start_time) > (profile.reflow_time * 1000)) {
        log_d("%d %d", millis() - start_time, profile.reflow_time * 1000);
        state = COOLING;
      } else {
        current_setpoint = profile.reflow_temp;
        duty             = compute(current_setpoint, temp);
        duty             = LIMIT(duty, PWM_MAX_HOT);
        ledcWrite(3, duty);
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