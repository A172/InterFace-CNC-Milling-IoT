#include <Arduino.h>
#include "app/AppController.h"
#include "config/PinConfig.h"
#include "input/ButtonHandler.h"
#include "input/EncoderHandler.h"

AppController app;

ButtonHandler buttons(
  PinConfig::BTN1,
  PinConfig::BTN2,
  PinConfig::BTN3,
  PinConfig::BTN4,
  PinConfig::BTN5,
  PinConfig::BTN6
);

EncoderHandler encoder(
  PinConfig::ENCODER_CLK,
  PinConfig::ENCODER_DT,
  PinConfig::ENCODER_SW
);

void handleButtonPressed(uint8_t buttonNumber) {
  app.onButtonPressed(buttonNumber);
}

void handleEncoderTurned(int8_t direction) {
  app.onEncoderTurned(direction);
}

void handleEncoderPressed() {
  app.onEncoderPressed();
}

void setup() {
  Serial.begin(115200);
  buttons.begin();
  buttons.setCallback(handleButtonPressed);
  encoder.begin();
  encoder.setTurnCallback(handleEncoderTurned);
  encoder.setPressCallback(handleEncoderPressed);
  app.begin();
}

void loop() {
  buttons.update();
  encoder.update();
  app.update();
}
