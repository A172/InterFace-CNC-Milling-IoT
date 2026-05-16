#include "AppController.h"

void AppController::begin() {
  bool wifiReady = _wifi.begin();

  if (wifiReady) {
    _cloud.begin();
  }
}

void AppController::update() {
  _wifi.update();

  if (_wifi.isConnected()) {
    _cloud.update();
  }

  if (millis() - _lastHeartbeat < 5000) {
    return;
  }

  _lastHeartbeat = millis();

  if (_cloud.isConnected()) {
    _cloud.publishStatus("idle", "Heartbeat");
  }
}

void AppController::onButtonPressed(uint8_t buttonNumber) {
  Serial.print("Event tombol dikirim ke aplikasi: ");
  Serial.println(buttonNumber);
  _cloud.publishButtonEvent(buttonNumber);
}

void AppController::onEncoderTurned(int8_t direction) {
  Serial.print("Event encoder dikirim ke aplikasi: ");
  Serial.println(direction > 0 ? "Z+" : "Z-");
  _cloud.publishEncoderEvent(direction);
}

void AppController::onEncoderPressed() {
  Serial.println("Encoder digunakan sebagai OK / Select");
  _cloud.publishStatus("input", "encoder_pressed");
}
