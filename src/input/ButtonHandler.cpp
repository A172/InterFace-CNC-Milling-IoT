#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(int b1, int b2, int b3, int b4, int b5, int b6) {
  _pins[0] = b1;
  _pins[1] = b2;
  _pins[2] = b3;
  _pins[3] = b4;
  _pins[4] = b5;
  _pins[5] = b6;

  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    _lastReading[i] = HIGH;
    _stableState[i] = HIGH;
    _lastChangeAt[i] = 0;
  }
}

void ButtonHandler::begin() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(_pins[i], INPUT_PULLUP);
  }
}

void ButtonHandler::update() {
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    processButton(i);
  }
}

void ButtonHandler::setCallback(ButtonEventCallback callback) {
  _callback = callback;
}

void ButtonHandler::processButton(uint8_t index) {
  uint8_t reading = digitalRead(_pins[index]);

  if (reading != _lastReading[index]) {
    _lastChangeAt[index] = millis();
    _lastReading[index] = reading;
  }

  if ((millis() - _lastChangeAt[index]) < DEBOUNCE_MS) {
    return;
  }

  if (reading == _stableState[index]) {
    return;
  }

  _stableState[index] = reading;

  if (_stableState[index] == LOW) {
    uint8_t buttonNumber = index + 1;

    Serial.print("Button ");
    Serial.print(buttonNumber);
    Serial.println(" ditekan");

    if (_callback != nullptr) {
      _callback(buttonNumber);
    }
  }
}
