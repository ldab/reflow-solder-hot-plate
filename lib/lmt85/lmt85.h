#pragma once

#include "Arduino.h"

#define TEMP_ANALOG 10

void lmt85_init(void);
float lmt85_temp(void);
void lmt85_start(float *temp);