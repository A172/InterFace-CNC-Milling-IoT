#include "WifiPortal.h"

#include <WiFi.h>
#include <WiFiManager.h>
#include <cstring>
#include "../config/AppConfig.h"
#include "../config/MqttConfig.h"

namespace {
  void copyToBuffer(char *target, size_t targetSize, const String &value, const char *fallback) {
    String source = value;
    source.trim();

    if (source.length() == 0 && fallback != nullptr) {
      source = fallback;
    }

    source.toCharArray(target, targetSize);
  }
}

bool WifiPortal::begin(WifiPortalMqttSettings *mqttSettings) {
  WiFi.mode(WIFI_STA);

  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);

  char brokerBuffer[MqttConfig::BROKER_MAX_LEN + 1];
  char portBuffer[7];
  char topicBuffer[MqttConfig::TOPIC_PREFIX_MAX_LEN + 1];
  char userBuffer[MqttConfig::USER_MAX_LEN + 1];
  char passBuffer[MqttConfig::PASSWORD_MAX_LEN + 1];

  copyToBuffer(
    brokerBuffer,
    sizeof(brokerBuffer),
    mqttSettings != nullptr ? mqttSettings->broker : String(),
    MqttConfig::BROKER
  );
  copyToBuffer(
    portBuffer,
    sizeof(portBuffer),
    mqttSettings != nullptr ? mqttSettings->port : String(),
    String(MqttConfig::PORT).c_str()
  );
  copyToBuffer(
    topicBuffer,
    sizeof(topicBuffer),
    mqttSettings != nullptr ? mqttSettings->topicPrefix : String(),
    MqttConfig::TOPIC_PREFIX
  );
  copyToBuffer(
    userBuffer,
    sizeof(userBuffer),
    mqttSettings != nullptr ? mqttSettings->user : String(),
    MqttConfig::USER
  );
  copyToBuffer(
    passBuffer,
    sizeof(passBuffer),
    mqttSettings != nullptr ? mqttSettings->password : String(),
    MqttConfig::PASSWORD
  );

  WiFiManagerParameter brokerParam("mqtt_broker", "MQTT Broker/IP", brokerBuffer, sizeof(brokerBuffer));
  WiFiManagerParameter portParam("mqtt_port", "MQTT Port", portBuffer, sizeof(portBuffer));
  WiFiManagerParameter topicParam("mqtt_topic", "MQTT Topic Prefix", topicBuffer, sizeof(topicBuffer));
  WiFiManagerParameter userParam("mqtt_user", "MQTT User", userBuffer, sizeof(userBuffer));
  WiFiManagerParameter passParam("mqtt_pass", "MQTT Password", passBuffer, sizeof(passBuffer));

  if (mqttSettings != nullptr) {
    wifiManager.addParameter(&brokerParam);
    wifiManager.addParameter(&portParam);
    wifiManager.addParameter(&topicParam);
    wifiManager.addParameter(&userParam);
    wifiManager.addParameter(&passParam);
  }

  Serial.println("Menghubungkan WiFi...");
  bool connected = wifiManager.autoConnect(AppConfig::WIFI_PORTAL_AP_NAME);

  if (connected) {
    if (mqttSettings != nullptr) {
      mqttSettings->broker = brokerParam.getValue();
      mqttSettings->port = portParam.getValue();
      mqttSettings->topicPrefix = topicParam.getValue();
      mqttSettings->user = userParam.getValue();
      mqttSettings->password = passParam.getValue();
    }

    Serial.print("WiFi terhubung. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi belum terhubung. Restart device untuk membuka portal lagi.");
  }

  return connected;
}

void WifiPortal::update() {
  if (millis() - _lastStatusPrint < 10000) {
    return;
  }

  _lastStatusPrint = millis();

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("WiFi terputus, mencoba reconnect...");
  WiFi.reconnect();
}

bool WifiPortal::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

String WifiPortal::ipAddress() const {
  if (!isConnected()) {
    return "-";
  }

  return WiFi.localIP().toString();
}

String WifiPortal::ssid() const {
  if (!isConnected()) {
    return "-";
  }

  return WiFi.SSID();
}

const char *WifiPortal::portalApName() const {
  return AppConfig::WIFI_PORTAL_AP_NAME;
}

void WifiPortal::disconnect() {
  WiFi.disconnect(false, false);
  WiFi.mode(WIFI_OFF);
}

void WifiPortal::resetSettings() {
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
}
