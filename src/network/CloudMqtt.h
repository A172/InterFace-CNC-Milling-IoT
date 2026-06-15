#ifndef CLOUD_MQTT_H
#define CLOUD_MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

class CloudMqtt {
  public:
    bool begin();
    void update();
    bool isConnected();

    void publishStatus(const String &state, const String &detail);
    void publishButtonEvent(uint8_t buttonNumber);
    void publishEncoderEvent(int8_t direction);
    void handleMessage(char *topic, byte *payload, unsigned int length);

  private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    String _clientId;
    String _baseTopic;
    unsigned long _lastReconnectAttempt = 0;

    bool connect();
    String makePayload(const String &state, const String &detail) const;
};

#endif
