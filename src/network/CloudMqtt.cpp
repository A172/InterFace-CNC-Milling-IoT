#include "CloudMqtt.h"

#include <WiFi.h>

namespace {
  constexpr const char *MQTT_HOST = "broker.hivemq.com";
  constexpr uint16_t MQTT_PORT = 1883;
  constexpr const char *PROJECT_TOPIC_PREFIX = "skripsi/cnc-interface";

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

  void mqttCallback(char *topic, byte *payload, unsigned int length) {
    if (activeMqtt != nullptr) {
      activeMqtt->handleMessage(topic, payload, length);
    }
  }
}

bool CloudMqtt::begin() {
  _clientId = "cnc-interface-" + macSuffix();
  _baseTopic = String(PROJECT_TOPIC_PREFIX) + "/" + macSuffix();

  _mqttClient.setClient(_wifiClient);
  _mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  _mqttClient.setCallback(mqttCallback);
  _mqttClient.setBufferSize(2048);
  activeMqtt = this;

  return connect();
}

void CloudMqtt::update() {
  if (_mqttClient.connected()) {
    _mqttClient.loop();
    return;
  }

  if (millis() - _lastReconnectAttempt < 5000) {
    return;
  }

  _lastReconnectAttempt = millis();
  connect();
}

bool CloudMqtt::isConnected() const {
  return _mqttClient.connected();
}

void CloudMqtt::publishStatus(const String &state, const String &detail) {
  if (!_mqttClient.connected()) {
    return;
  }

  String topic = _baseTopic + "/status";
  String payload = makePayload(state, detail);
  _mqttClient.publish(topic.c_str(), payload.c_str(), false);
}

void CloudMqtt::publishButtonEvent(uint8_t buttonNumber) {
  if (!_mqttClient.connected()) {
    return;
  }

  String topic = _baseTopic + "/event/button";
  String payload = String("{\"button\":") + String(buttonNumber) +
                   ",\"uptime_ms\":" + String(millis()) + "}";
  _mqttClient.publish(topic.c_str(), payload.c_str(), false);
}

void CloudMqtt::publishEncoderEvent(int8_t direction) {
  if (!_mqttClient.connected()) {
    return;
  }

  String topic = _baseTopic + "/event/encoder";
  String label = direction > 0 ? "Z+" : "Z-";
  String payload = String("{\"direction\":") + String(direction) +
                   ",\"label\":\"" + label +
                   "\",\"uptime_ms\":" + String(millis()) + "}";
  _mqttClient.publish(topic.c_str(), payload.c_str(), false);
}

bool CloudMqtt::connect() {
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }

  String willTopic = _baseTopic + "/availability";

  Serial.print("Menghubungkan MQTT ke ");
  Serial.print(MQTT_HOST);
  Serial.print(":");
  Serial.println(MQTT_PORT);

  bool connected = _mqttClient.connect(
    _clientId.c_str(),
    willTopic.c_str(),
    1,
    true,
    "offline"
  );

  if (!connected) {
    Serial.print("MQTT gagal, state: ");
    Serial.println(_mqttClient.state());
    return false;
  }

  _mqttClient.publish(willTopic.c_str(), "online", true);

  String commandTopic = _baseTopic + "/cmd/#";
  _mqttClient.subscribe(commandTopic.c_str());

  String uploadTopic = _baseTopic + "/upload/#";
  _mqttClient.subscribe(uploadTopic.c_str());

  publishStatus("idle", "Device online");

  Serial.print("MQTT terhubung. Base topic: ");
  Serial.println(_baseTopic);
  return true;
}

void CloudMqtt::handleMessage(char *topic, byte *payload, unsigned int length) {
  String message;
  message.reserve(length + 1);

  for (unsigned int i = 0; i < length; i++) {
    message += static_cast<char>(payload[i]);
  }

  Serial.print("MQTT command [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  publishStatus("command_received", message);
}

String CloudMqtt::makePayload(const String &state, const String &detail) const {
  return String("{\"state\":\"") + escapeJson(state) +
         "\",\"detail\":\"" + escapeJson(detail) +
         "\",\"uptime_ms\":" + String(millis()) + "}";
}
