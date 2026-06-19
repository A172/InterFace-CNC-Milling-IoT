#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include <Arduino.h>

namespace MqttConfig {
  // Nilai default. Preferences/WiFiManager tetap dapat menggantinya saat runtime.
  constexpr bool DEFAULT_ENABLED = false;
  constexpr const char *BROKER = "192.168.250.3";
  constexpr const char *LEGACY_BROKER = "192.168.250.13";
  constexpr uint16_t PORT = 1883;
  constexpr const char *CLIENT_ID = "Interface CNC Milling";
  constexpr const char *TOPIC_PREFIX = "cnc";
  constexpr const char *USER = "";
  constexpr const char *PASSWORD = "";

  // Suffix topic. Prefix runtime ditambahkan oleh CloudMqtt.
  constexpr const char *TOPIC_STATUS = "status";
  constexpr const char *TOPIC_COMMAND = "command";
  constexpr const char *TOPIC_GCODE = "gcode";
  constexpr const char *TOPIC_PROGRESS = "progress";
  constexpr const char *TOPIC_ERROR = "error";
  constexpr const char *TOPIC_POSITION = "position";
  constexpr const char *TOPIC_NETWORK = "network";
  constexpr const char *TOPIC_TIME = "time";
  constexpr const char *TOPIC_ALARM = "alarm";

  constexpr size_t BROKER_MAX_LEN = 63;
  constexpr size_t TOPIC_PREFIX_MAX_LEN = 63;
  constexpr size_t USER_MAX_LEN = 31;
  constexpr size_t PASSWORD_MAX_LEN = 31;

  constexpr unsigned long RECONNECT_INTERVAL_MS = 10000;
  constexpr uint32_t CONNECT_TIMEOUT_MS = 250;
  constexpr uint16_t SOCKET_TIMEOUT_SEC = 1;
  constexpr unsigned long MONITOR_INTERVAL_MS = 1000;
  constexpr unsigned long POSITION_INTERVAL_MS = 1000;
  constexpr unsigned long PROGRESS_INTERVAL_MS = 1000;
  constexpr unsigned long TIME_INTERVAL_MS = 5000;
}

#endif
