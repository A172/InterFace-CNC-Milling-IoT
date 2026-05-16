#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

namespace PinConfig {
  constexpr uint8_t BTN1 = 4;
  constexpr uint8_t BTN2 = 5;
  constexpr uint8_t BTN3 = 6;
  constexpr uint8_t BTN4 = 7;
  constexpr uint8_t BTN5 = 15;
  constexpr uint8_t BTN6 = 16;

  constexpr uint8_t ENCODER_CLK = 17;
  constexpr uint8_t ENCODER_DT = 18;
  constexpr uint8_t ENCODER_SW = 8;
}

#endif
