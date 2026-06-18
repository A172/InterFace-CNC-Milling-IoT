#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>

struct WifiPortalMqttSettings {
  String broker;
  String port;
  String topicPrefix;
  String user;
  String password;
};

class WifiPortal {
  public:
    bool begin(WifiPortalMqttSettings *mqttSettings = nullptr);
    void update();
    bool isConnected() const;
    String ipAddress() const;
    String ssid() const;
    const char *portalApName() const;
    void disconnect();
    void resetSettings();

  private:
    unsigned long _lastStatusPrint = 0;
};

#endif
