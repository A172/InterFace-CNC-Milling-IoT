#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>

class WifiPortal {
  public:
    bool begin();
    void update();
    bool isConnected() const;
    String ipAddress() const;

  private:
    unsigned long _lastStatusPrint = 0;
};

#endif
