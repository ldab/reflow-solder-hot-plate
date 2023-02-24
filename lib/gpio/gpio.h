#pragma once

#include "Arduino.h"

#define BUTTON             0

#define TEMP_ANALOG        10
#define FET_GATE           14

#define LED_R              5
#define LED_G              2
#define LED_B              8

typedef enum { RED, GREEN, YELLOW, BLUE, CYAN, WHITE, PURPLE } colour_t;

typedef enum { SLOW = 2, MEDIUM = 4, FAST = 6 } rate_t;

static struct {
  colour_t colour;
  uint32_t rate;
} led;

void gpios_init(void);
void set_led(colour_t colour, uint32_t rate = 2);