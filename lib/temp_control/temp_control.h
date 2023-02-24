#pragma once

#include "Arduino.h"

// With a cold heat plate, the current consumption is too high for the power
// supply, which are both rated at 3A. To reduce the current consumption, a PWM
// signal MUST be applied to the MOSFET gate during the warmup phase. The PWM
// frequency must be at least 50 kHz for the filter capacitors to work
// properly.

// I_{cold} = V_{in} / R_{cold}
// PWM_{max} = 2.75A / I_{cold} (0.25A as margin)

// The resistance multiplies by approximately 1.63 from 20°C to 180°C.

#define PWM_RESOLUTION

#define R_HEATER     2
#define PWM_MAX_COLD (pow(2, PWM_RESOLUTION) - 1) * 2.75 / (20 / R_HEATER)
#define PWM_MAX_HOT  PWM_MAX_COLD * 1.63

// https://www.mouser.dk/datasheet/2/73/TS391LT-1150364.pdf
#define TEMP_CONTROL_DEFAULT()                                                 \
  {                                                                            \
    .preheat_temp = 90, .preheat_rate = 90 / 90, .soak_temp = 130,             \
    .soak_rate = 210 / 138, .reflow_temp = 138, .reflow_time = 160,            \
  }

typedef struct {
  uint32_t preheat_temp;
  uint32_t preheat_rate;
  uint32_t soak_temp;
  uint32_t soak_rate;
  uint32_t reflow_temp;
  uint32_t reflow_time;
} reflow_profile;

void temp_control_config(reflow_profile *cfg);
void temp_control(float temp);