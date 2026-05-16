#include "EncoderHandler.h"

EncoderHandler::EncoderHandler(uint8_t clkPin, uint8_t dtPin, uint8_t swPin)
  : _clkPin(clkPin), _dtPin(dtPin), _swPin(swPin) {
}

void EncoderHandler::begin() {
  pinMode(_clkPin, INPUT_PULLUP);
  pinMode(_dtPin, INPUT_PULLUP);
  pinMode(_swPin, INPUT_PULLUP);

  _lastClkState = digitalRead(_clkPin);
  _lastSwitchReading = digitalRead(_swPin);
  _stableSwitchState = _lastSwitchReading;
}

void EncoderHandler::update() {
  processTurn();
  processSwitch();
}

void EncoderHandler::setTurnCallback(EncoderTurnCallback callback) {
  _turnCallback = callback;
}

void EncoderHandler::setPressCallback(EncoderPressCallback callback) {
  _pressCallback = callback;
}

void EncoderHandler::processTurn() {
  uint8_t clkState = digitalRead(_clkPin);

  if (clkState == _lastClkState) {
    return;
  }

  if (clkState == LOW) {
    int8_t direction = (digitalRead(_dtPin) == HIGH) ? 1 : -1;

    Serial.print("Encoder: ");
    Serial.println(direction > 0 ? "CW" : "CCW");

    if (_turnCallback != nullptr) {
      _turnCallback(direction);
    }
  }

  _lastClkState = clkState;
}

void EncoderHandler::processSwitch() {
  uint8_t reading = digitalRead(_swPin);

  if (reading != _lastSwitchReading) {
    _lastSwitchChangeAt = millis();
    _lastSwitchReading = reading;
  }

  if ((millis() - _lastSwitchChangeAt) < PRESS_DEBOUNCE_MS) {
    return;
  }

  if (reading == _stableSwitchState) {
    return;
  }

  _stableSwitchState = reading;

  if (_stableSwitchState == LOW) {
    Serial.println("Encoder switch ditekan");

    if (_pressCallback != nullptr) {
      _pressCallback();
    }
  }
}
