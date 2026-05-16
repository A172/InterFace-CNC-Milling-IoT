#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

typedef void (*ButtonEventCallback)(uint8_t buttonNumber);

class ButtonHandler {
  public:
    ButtonHandler(int b1, int b2, int b3, int b4, int b5, int b6);

    void begin();
    void update();   // dipanggil di loop()
    void setCallback(ButtonEventCallback callback);

  private:
    static constexpr uint8_t BUTTON_COUNT = 6;
    static constexpr unsigned long DEBOUNCE_MS = 35;

    uint8_t _pins[BUTTON_COUNT];
    uint8_t _lastReading[BUTTON_COUNT];
    uint8_t _stableState[BUTTON_COUNT];
    unsigned long _lastChangeAt[BUTTON_COUNT];
    ButtonEventCallback _callback = nullptr;

    void processButton(uint8_t index);
};

#endif
