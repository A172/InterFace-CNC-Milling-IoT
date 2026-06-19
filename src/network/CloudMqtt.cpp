#include "CloudMqtt.h"

#include <WiFi.h>
#include "../config/AppConfig.h"

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
  _clientId = String(AppConfig::MQTT_CLIENT_ID) + "-" + macSuffix();
  _baseTopic = _topicPrefix;

  _mqttClient.setClient(_wifiClient);
  _mqttClient.setServer(_broker.c_str(), _port);
  _mqttClient.setCallback(mqttCallback);
  _mqttClient.setBufferSize(2048);
  _mqttClient.setSocketTimeout(AppConfig::MQTT_SOCKET_TIMEOUT_SEC);
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
    _broker = AppConfig::MQTT_BROKER;
  }

  _port = port == 0 ? AppConfig::MQTT_PORT : port;

  _topicPrefix = topicPrefix;
  _topicPrefix.trim();
  if (_topicPrefix.length() == 0) {
    _topicPrefix = AppConfig::MQTT_TOPIC_PREFIX;
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

  if (millis() - _lastReconnectAttempt < AppConfig::MQTT_RECONNECT_INTERVAL_MS) {
    return;
  }

  _lastReconnectAttempt = millis();
  connect();
}

bool CloudMqtt::isConnected() {
  return _mqttClient.connected();
}

int CloudMqtt::state() {
  _lastState = _mqttClient.state();
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
  return _broker.length() ? _broker : String(AppConfig::MQTT_BROKER);
}

uint16_t CloudMqtt::port() const {
  return _port == 0 ? AppConfig::MQTT_PORT : _port;
}

String CloudMqtt::topicPrefix() const {
  return _topicPrefix.length() ? _topicPrefix : String(AppConfig::MQTT_TOPIC_PREFIX);
}

void CloudMqtt::disconnect() {
  if (!_mqttClient.connected()) {
    return;
  }

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
  publishTime(snapshot, now);
  publishAlarm(snapshot);
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

  Serial.print("Menghubungkan MQTT ke ");
  Serial.print(_broker);
  Serial.print(":");
  Serial.println(_port);

  bool connected = false;
  if (_user.length() > 0) {
    connected = _mqttClient.connect(
      _clientId.c_str(),
      _user.c_str(),
      _password.c_str()
    );
  } else {
    connected = _mqttClient.connect(_clientId.c_str());
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
  _lastTimePublish = 0;
  _lastNetworkPayload = "";
  _lastTimePayload = "";
  _lastAlarmPayload = "";

  Serial.print("MQTT terhubung. Base topic: ");
  Serial.println(_baseTopic);
  return true;
}

bool CloudMqtt::publishJson(const String &topicSuffix, const String &payload, bool retained) {
  if (!_mqttClient.connected()) {
    return false;
  }

  String topic = _baseTopic + "/" + topicSuffix;
  return _mqttClient.publish(topic.c_str(), payload.c_str(), retained);
}

void CloudMqtt::publishNetwork(const MqttMonitoringSnapshot &snapshot, bool force) {
  String payload = String("{\"ssid\":\"") + escapeJson(snapshot.ssid) +
                   "\",\"ip\":\"" + escapeJson(snapshot.ipAddress) +
                   "\",\"mqtt\":\"" + escapeJson(snapshot.mqttLink) +
                   "\"}";

  if (!force && payload == _lastNetworkPayload) {
    return;
  }

  publishJson("network", payload, true);
  _lastNetworkPayload = payload;
  _networkPublished = true;
}

void CloudMqtt::publishPosition(const MqttMonitoringSnapshot &snapshot, unsigned long now) {
  if (now - _lastPositionPublish < AppConfig::MQTT_POSITION_INTERVAL_MS) {
    return;
  }

  String payload = String("{\"x\":") + floatJson(snapshot.posX) +
                   ",\"y\":" + floatJson(snapshot.posY) +
                   ",\"z\":" + floatJson(snapshot.posZ) +
                   ",\"unit\":\"mm\"}";

  publishJson("position", payload, true);
  _lastPositionPublish = now;
}

void CloudMqtt::publishTime(const MqttMonitoringSnapshot &snapshot, unsigned long now) {
  String payload = String("{\"sync\":\"") + escapeJson(snapshot.timeStatus) +
                   "\",\"time\":\"" + escapeJson(snapshot.time) +
                   "\",\"date\":\"" + escapeJson(snapshot.date) +
                   "\"}";

  if (payload == _lastTimePayload &&
      now - _lastTimePublish < AppConfig::MQTT_TIME_INTERVAL_MS) {
    return;
  }

  publishJson("time", payload, true);
  _lastTimePayload = payload;
  _lastTimePublish = now;
}

void CloudMqtt::publishAlarm(const MqttMonitoringSnapshot &snapshot) {
  String level = "INFO";
  String message = "OK";

  if (snapshot.marlinStatus == "NO RESP") {
    level = "ERROR";
    message = "Marlin no response";
  }

  String payload = String("{\"level\":\"") + level +
                   "\",\"message\":\"" + escapeJson(message) +
                   "\"}";

  if (payload == _lastAlarmPayload) {
    return;
  }

  publishJson("alarm", payload, true);
  _lastAlarmPayload = payload;
}

void CloudMqtt::ensureConfigured() {
  if (_broker.length() == 0 ||
      _port == 0 ||
      _topicPrefix.length() == 0) {
    configure(
      _broker.length() ? _broker : String(AppConfig::MQTT_BROKER),
      _port == 0 ? AppConfig::MQTT_PORT : _port,
      _topicPrefix.length() ? _topicPrefix : String(AppConfig::MQTT_TOPIC_PREFIX),
      _user,
      _password
    );
  }
}

bool CloudMqtt::brokerReachable() {
  WiFiClient probe;
  probe.setTimeout(AppConfig::MQTT_SOCKET_TIMEOUT_SEC);

  bool reachable = probe.connect(
    _broker.c_str(),
    _port,
    AppConfig::MQTT_CONNECT_TIMEOUT_MS
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

  Serial.print("MQTT message ignored [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
}
