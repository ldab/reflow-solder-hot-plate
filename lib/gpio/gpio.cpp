#include "gpio.h"

void TaskLed(void *pvParameters)
{
  (void)pvParameters;

  for (;;) // A Task shall never return or exit.
  {
    switch (led.colour) {
    case RED:
      digitalWrite(LED_R, !digitalRead(LED_R));
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, HIGH);
      break;
    case GREEN:
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_G, !digitalRead(LED_G));
      digitalWrite(LED_B, HIGH);
      break;
    case BLUE:
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, !digitalRead(LED_B));
      break;
    case CYAN:
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_G, !digitalRead(LED_G));
      digitalWrite(LED_B, !digitalRead(LED_B));
      break;
    case PURPLE:
      digitalWrite(LED_R, !digitalRead(LED_R));
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, !digitalRead(LED_B));
      break;
    case YELLOW:
      digitalWrite(LED_R, !digitalRead(LED_R));
      digitalWrite(LED_G, !digitalRead(LED_G));
      digitalWrite(LED_B, HIGH);
      break;
    case WHITE:
      digitalWrite(LED_R, !digitalRead(LED_R));
      digitalWrite(LED_G, !digitalRead(LED_G));
      digitalWrite(LED_B, !digitalRead(LED_B));
      break;
    default:
      digitalWrite(LED_R, HIGH);
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_B, HIGH);
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(1000 / led.rate));
  }
  vTaskDelete(NULL);
}

void set_led(colour_t colour, uint32_t rate)
{
  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, HIGH);
  digitalWrite(LED_B, HIGH);
  led.colour = colour;
  led.rate   = rate;
}

void gpios_init(void)
{
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  set_led(WHITE, SLOW);

  xTaskCreate(TaskLed, "TaskLed", 1024, NULL, 0, NULL);
}