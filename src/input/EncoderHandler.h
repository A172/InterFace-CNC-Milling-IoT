#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H

#include <Arduino.h>

typedef void (*EncoderTurnCallback)(int8_t direction);
typedef void (*EncoderPressCallback)();

class EncoderHandler {
  public:
    EncoderHandler(uint8_t clkPin, uint8_t dtPin, uint8_t swPin);

    void begin();
    void update();
    void setTurnCallback(EncoderTurnCallback callback);
    void setPressCallback(EncoderPressCallback callback);

  private:
    static constexpr unsigned long PRESS_DEBOUNCE_MS = 35;

    uint8_t _clkPin;
    uint8_t _dtPin;
    uint8_t _swPin;
    uint8_t _lastClkState = HIGH;
    uint8_t _lastSwitchReading = HIGH;
    uint8_t _stableSwitchState = HIGH;
    unsigned long _lastSwitchChangeAt = 0;

    EncoderTurnCallback _turnCallback = nullptr;
    EncoderPressCallback _pressCallback = nullptr;

    void processTurn();
    void processSwitch();
};

#endif
