#include "CloudMqtt.h"

#include <WiFi.h>
#include "../config/MqttConfig.h"

namespace {
  CloudMqtt *activeMqtt = nullptr;

  String escapeJson(const String &value) {
    String escaped;
    escaped.reserve(value.length());

    for (size_t i = 0; i < value.length(); i++) {
      char current = value.charAt(i);

      if (current == '"' || current == '\\') {
        escaped += '\\';
      }

      if (current == '\n') {
        escaped += "\\n";
      } else if (current == '\r') {
        escaped += "\\r";
      } else {
        escaped += current;
      }
    }

    return escaped;
  }

  String macSuffix() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();
    return mac;
  }

  String floatJson(float value) {
    return String(value, 2);
  }

  void mqttCallback(char *topic, byte *payload, unsigned int length) {
    if (activeMqtt != nullptr) {
      activeMqtt->handleMessage(topic, payload, length);
    }
  }
}

bool CloudMqtt::begin() {
  ensureConfigured();
  _clientId = String(MqttConfig::CLIENT_ID) + "-" + macSuffix();
  _baseTopic = _topicPrefix;

  _mqttClient.setClient(_wifiClient);
  _mqttClient.setServer(_broker.c_str(), _port);
  _mqttClient.setCallback(mqttCallback);
  _mqttClient.setBufferSize(2048);
  _mqttClient.setSocketTimeout(MqttConfig::SOCKET_TIMEOUT_SEC);
  activeMqtt = this;

  return connect();
}

void CloudMqtt::configure(
  const String &broker,
  uint16_t port,
  const String &topicPrefix,
  const String &user,
  const String &password
) {
  _broker = broker;
  _broker.trim();
  if (_broker.length() == 0) {
    _broker = MqttConfig::BROKER;
  }

  _port = port == 0 ? MqttConfig::PORT : port;

  _topicPrefix = topicPrefix;
  _topicPrefix.trim();
  if (_topicPrefix.length() == 0) {
    _topicPrefix = MqttConfig::TOPIC_PREFIX;
  }
  while (_topicPrefix.endsWith("/")) {
    _topicPrefix.remove(_topicPrefix.length() - 1);
  }

  _user = user;
  _user.trim();
  _password = password;
}

void CloudMqtt::update() {
  if (_mqttClient.connected()) {
    _lastState = MQTT_CONNECTED;
    _mqttClient.loop();
    return;
  }

  if (_lastState == MQTT_CONNECTED) {
    _lastState = _mqttClient.state();
  }

  if (millis() - _lastReconnectAttempt < MqttConfig::RECONNECT_INTERVAL_MS) {
    return;
  }

  _lastReconnectAttempt = millis();
  connect();
}

bool CloudMqtt::isConnected() {
  return _mqttClient.connected();
}

int CloudMqtt::state() {
  return _lastState;
}

String CloudMqtt::statusText() {
  if (_mqttClient.connected()) {
    _lastState = MQTT_CONNECTED;
    return String("CONNECTED");
  }

  if (WiFi.status() != WL_CONNECTED) {
    return String("NO WIFI");
  }

  int currentState = state();
  switch (currentState) {
    case MQTT_CONNECTION_TIMEOUT:
      return String("TIMEOUT");
    case MQTT_CONNECTION_LOST:
      return String("LOST");
    case MQTT_CONNECT_FAILED:
      return String("CONN FAIL");
    case MQTT_DISCONNECTED:
      return String("DISCONNECTED");
    case MQTT_CONNECT_BAD_PROTOCOL:
      return String("BAD PROTO");
    case MQTT_CONNECT_BAD_CLIENT_ID:
      return String("BAD ID");
    case MQTT_CONNECT_UNAVAILABLE:
      return String("BROKER OFF");
    case MQTT_CONNECT_BAD_CREDENTIALS:
      return String("BAD LOGIN");
    case MQTT_CONNECT_UNAUTHORIZED:
      return String("DENIED");
    default:
      return String("WAIT ") + String(currentState);
  }
}

String CloudMqtt::broker() const {
  return _broker.length() ? _broker : String(MqttConfig::BROKER);
}

uint16_t CloudMqtt::port() const {
  return _port == 0 ? MqttConfig::PORT : _port;
}

String CloudMqtt::topicPrefix() const {
  return _topicPrefix.length() ? _topicPrefix : String(MqttConfig::TOPIC_PREFIX);
}

void CloudMqtt::disconnect() {
  if (!_mqttClient.connected()) {
    return;
  }

  publishConnectionStatus(false, "offline");
  _mqttClient.disconnect();
  _lastState = MQTT_DISCONNECTED;
}

void CloudMqtt::publishMonitoring(const MqttMonitoringSnapshot &snapshot) {
  if (!_mqttClient.connected()) {
    return;
  }

  unsigned long now = millis();

  publishNetwork(snapshot, !_networkPublished);
  publishPosition(snapshot, now);
  publishProgress(snapshot, now);
  publishTime(snapshot, now);
  publishAlarm(snapshot);
  publishError(snapshot);
}

bool CloudMqtt::connect() {
  ensureConfigured();

  if (WiFi.status() != WL_CONNECTED) {
    _lastState = MQTT_CONNECT_FAILED;
    return false;
  }

  if (!brokerReachable()) {
    return false;
  }

  String statusTopic = topicName(MqttConfig::TOPIC_STATUS);
  const char *offlinePayload = "{\"connected\":false,\"state\":\"offline\"}";

  Serial.print("Menghubungkan MQTT ke ");
  Serial.print(_broker);
  Serial.print(":");
  Serial.println(_port);

  bool connected = false;
  if (_user.length() > 0) {
    connected = _mqttClient.connect(
      _clientId.c_str(),
      _user.c_str(),
      _password.c_str(),
      statusTopic.c_str(),
      1,
      true,
      offlinePayload
    );
  } else {
    connected = _mqttClient.connect(
      _clientId.c_str(),
      statusTopic.c_str(),
      1,
      true,
      offlinePayload
    );
  }

  if (!connected) {
    _lastState = _mqttClient.state();
    Serial.print("MQTT gagal, state: ");
    Serial.println(_lastState);
    return false;
  }

  _lastState = MQTT_CONNECTED;
  _networkPublished = false;
  _lastPositionPublish = 0;
  _lastProgressPublish = 0;
  _lastTimePublish = 0;
  _lastProgressPayload = "";
  _lastNetworkPayload = "";
  _lastTimePayload = "";
  _lastAlarmPayload = "";
  _lastErrorPayload = "";

  subscribeInboundTopics();
  publishConnectionStatus(true, "online");

  Serial.print("MQTT terhubung. Base topic: ");
  Serial.println(_baseTopic);
  return true;
}

bool CloudMqtt::subscribeInboundTopics() {
  String commandTopic = topicName(MqttConfig::TOPIC_COMMAND);
  String gcodeTopic = topicName(MqttConfig::TOPIC_GCODE);
  bool commandReady = _mqttClient.subscribe(commandTopic.c_str(), 1);
  bool gcodeReady = _mqttClient.subscribe(gcodeTopic.c_str(), 1);

  Serial.print("MQTT subscribe ");
  Serial.print(commandTopic);
  Serial.println(commandReady ? " OK" : " GAGAL");
  Serial.print("MQTT subscribe ");
  Serial.print(gcodeTopic);
  Serial.println(gcodeReady ? " OK" : " GAGAL");

  return commandReady && gcodeReady;
}

String CloudMqtt::topicName(const char *suffix) const {
  return _baseTopic + "/" + suffix;
}

bool CloudMqtt::publishJson(const String &topicSuffix, const String &payload, bool retained) {
  if (!_mqttClient.connected()) {
    return false;
  }

  String topic = topicName(topicSuffix.c_str());
  return _mqttClient.publish(topic.c_str(), payload.c_str(), retained);
}

void CloudMqtt::publishConnectionStatus(bool connected, const char *state) {
  String payload = String("{\"connected\":") + (connected ? "true" : "false") +
                   ",\"state\":\"" + escapeJson(state) + "\"}";
  publishJson(MqttConfig::TOPIC_STATUS, payload, true);
}

void CloudMqtt::publishNetwork(const MqttMonitoringSnapshot &snapshot, bool force) {
  String payload = String("{\"ssid\":\"") + escapeJson(snapshot.ssid) +
                   "\",\"ip\":\"" + escapeJson(snapshot.ipAddress) +
                   "\",\"mqtt\":\"" + escapeJson(snapshot.mqttLink) +
                   "\"}";

  if (!force && payload == _lastNetworkPayload) {
    return;
  }

  publishJson(MqttConfig::TOPIC_NETWORK, payload, true);
  _lastNetworkPayload = payload;
  _networkPublished = true;
}

void CloudMqtt::publishPosition(const MqttMonitoringSnapshot &snapshot, unsigned long now) {
  if (now - _lastPositionPublish < MqttConfig::POSITION_INTERVAL_MS) {
    return;
  }

  String payload = String("{\"x\":") + floatJson(snapshot.posX) +
                   ",\"y\":" + floatJson(snapshot.posY) +
                   ",\"z\":" + floatJson(snapshot.posZ) +
                   ",\"unit\":\"mm\"}";

  publishJson(MqttConfig::TOPIC_POSITION, payload, true);
  _lastPositionPublish = now;
}

void CloudMqtt::publishProgress(const MqttMonitoringSnapshot &snapshot, unsigned long now) {
  String payload;
  if (snapshot.progress < 0) {
    payload = "{\"progress\":null,\"state\":\"idle\"}";
  } else {
    int progress = constrain(snapshot.progress, 0, 100);
    payload = String("{\"progress\":") + String(progress) +
              ",\"state\":\"running\"}";
  }

  if (payload == _lastProgressPayload &&
      now - _lastProgressPublish < MqttConfig::PROGRESS_INTERVAL_MS) {
    return;
  }

  publishJson(MqttConfig::TOPIC_PROGRESS, payload, true);
  _lastProgressPayload = payload;
  _lastProgressPublish = now;
}

void CloudMqtt::publishTime(const MqttMonitoringSnapshot &snapshot, unsigned long now) {
  String payload = String("{\"sync\":\"") + escapeJson(snapshot.timeStatus) +
                   "\",\"time\":\"" + escapeJson(snapshot.time) +
                   "\",\"date\":\"" + escapeJson(snapshot.date) +
                   "\"}";

  if (payload == _lastTimePayload &&
      now - _lastTimePublish < MqttConfig::TIME_INTERVAL_MS) {
    return;
  }

  publishJson(MqttConfig::TOPIC_TIME, payload, true);
  _lastTimePayload = payload;
  _lastTimePublish = now;
}

void CloudMqtt::publishAlarm(const MqttMonitoringSnapshot &snapshot) {
  String level = "INFO";
  String message = "OK";

  if (snapshot.marlinStatus == "LOST") {
    level = "ERROR";
    message = "Marlin connection lost";
  } else if (snapshot.marlinStatus == "ERROR") {
    level = "ERROR";
    message = "Marlin error";
  } else if (snapshot.marlinStatus == "DISCONNECTED") {
    message = "Marlin not connected";
  } else if (snapshot.marlinStatus == "WAITING") {
    message = "Waiting for Marlin";
  } else if (snapshot.marlinStatus == "OFF") {
    message = "Marlin connection disabled";
  }

  String payload = String("{\"level\":\"") + level +
                   "\",\"message\":\"" + escapeJson(message) +
                   "\"}";

  if (payload == _lastAlarmPayload) {
    return;
  }

  publishJson(MqttConfig::TOPIC_ALARM, payload, true);
  _lastAlarmPayload = payload;
}

void CloudMqtt::publishError(const MqttMonitoringSnapshot &snapshot) {
  bool connectionLost = snapshot.marlinStatus == "LOST";
  bool marlinError = snapshot.marlinStatus == "ERROR";
  bool active = connectionLost || marlinError;
  const char *message = connectionLost
    ? "Marlin connection lost"
    : (marlinError ? "Marlin error" : "");
  String payload = String("{\"active\":") + (active ? "true" : "false") +
                   ",\"message\":\"" +
                   message + "\"}";

  if (payload == _lastErrorPayload) {
    return;
  }

  publishJson(MqttConfig::TOPIC_ERROR, payload, true);
  _lastErrorPayload = payload;
}

void CloudMqtt::ensureConfigured() {
  if (_broker.length() == 0 ||
      _port == 0 ||
      _topicPrefix.length() == 0) {
    configure(
      _broker.length() ? _broker : String(MqttConfig::BROKER),
      _port == 0 ? MqttConfig::PORT : _port,
      _topicPrefix.length() ? _topicPrefix : String(MqttConfig::TOPIC_PREFIX),
      _user,
      _password
    );
  }
}

bool CloudMqtt::brokerReachable() {
  WiFiClient probe;
  probe.setTimeout(MqttConfig::SOCKET_TIMEOUT_SEC);

  bool reachable = probe.connect(
    _broker.c_str(),
    _port,
    MqttConfig::CONNECT_TIMEOUT_MS
  );
  probe.stop();

  if (!reachable) {
    _lastState = MQTT_CONNECTION_TIMEOUT;
    Serial.print("MQTT broker tidak terjangkau: ");
    Serial.print(_broker);
    Serial.print(":");
    Serial.println(_port);
  }

  return reachable;
}

void CloudMqtt::handleMessage(char *topic, byte *payload, unsigned int length) {
  String message;
  message.reserve(length + 1);

  for (unsigned int i = 0; i < length; i++) {
    message += static_cast<char>(payload[i]);
  }

  String incomingTopic(topic);
  if (incomingTopic == topicName(MqttConfig::TOPIC_COMMAND)) {
    Serial.print("[MQTT][COMMAND] ");
    Serial.println(message);
    return;
  }

  if (incomingTopic == topicName(MqttConfig::TOPIC_GCODE)) {
    Serial.print("[MQTT][GCODE] ");
    Serial.println(message);
    return;
  }

  Serial.print("[MQTT][IGNORED] ");
  Serial.print(incomingTopic);
  Serial.print(" = ");
  Serial.println(message);
}
