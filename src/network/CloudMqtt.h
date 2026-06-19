#ifndef CLOUD_MQTT_H
#define CLOUD_MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

struct MqttMonitoringSnapshot {
  String marlinStatus;
  String timeStatus;
  String time;
  String date;
  String ssid;
  String ipAddress;
  String mqttLink;
  String homeX;
  String homeY;
  String homeZ;
  String softEndstop;
  String positionSource = "unavailable";
  float posX = 0.0f;
  float posY = 0.0f;
  float posZ = 0.0f;
  unsigned int feedrateXY = 0;
  unsigned int feedrateZ = 0;
  int progress = -1;
  bool sdReady = false;
  bool spindleOn = false;
  bool feedrateValid = false;
  bool positionValid = false;
};

class CloudMqtt {
  public:
    bool begin();
    void configure(
      const String &broker,
      uint16_t port,
      const String &topicPrefix,
      const String &user,
      const String &password
    );
    void update();
    bool isConnected();
    int state();
    String statusText();
    void disconnect();
    String broker() const;
    uint16_t port() const;
    String topicPrefix() const;

    void publishMonitoring(const MqttMonitoringSnapshot &snapshot);
    void handleMessage(char *topic, byte *payload, unsigned int length);

  private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    String _clientId;
    String _baseTopic;
    String _broker;
    String _topicPrefix;
    String _user;
    String _password;
    uint16_t _port = 0;
    int _lastState = MQTT_DISCONNECTED;
    unsigned long _lastReconnectAttempt = 0;
    unsigned long _lastPositionPublish = 0;
    unsigned long _lastMachinePublish = 0;
    unsigned long _lastProgressPublish = 0;
    unsigned long _lastTimePublish = 0;
    bool _networkPublished = false;
    String _lastProgressPayload;
    String _lastMachinePayload;
    String _lastNetworkPayload;
    String _lastTimePayload;
    String _lastAlarmPayload;
    String _lastErrorPayload;

    void ensureConfigured();
    bool brokerReachable();
    bool connect();
    bool subscribeInboundTopics();
    String topicName(const char *suffix) const;
    bool publishJson(const String &topicSuffix, const String &payload, bool retained = false);
    void publishConnectionStatus(bool connected, const char *state);
    void publishNetwork(const MqttMonitoringSnapshot &snapshot, bool force = false);
    void publishPosition(const MqttMonitoringSnapshot &snapshot, unsigned long now);
    void publishMachine(const MqttMonitoringSnapshot &snapshot, unsigned long now);
    void publishProgress(const MqttMonitoringSnapshot &snapshot, unsigned long now);
    void publishTime(const MqttMonitoringSnapshot &snapshot, unsigned long now);
    void publishAlarm(const MqttMonitoringSnapshot &snapshot);
    void publishError(const MqttMonitoringSnapshot &snapshot);
};

#endif
