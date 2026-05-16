#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <Arduino.h>

#include "../network/CloudMqtt.h"
#include "../network/WifiPortal.h"

class AppController {
  public:
    void begin();
    void update();
    void onButtonPressed(uint8_t buttonNumber);
    void onEncoderTurned(int8_t direction);
    void onEncoderPressed();

  private:
    WifiPortal _wifi;
    CloudMqtt _cloud;
    unsigned long _lastHeartbeat = 0;
};

#endif
