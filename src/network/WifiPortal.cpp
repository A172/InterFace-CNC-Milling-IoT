#include "WifiPortal.h"

#include <WiFi.h>
#include <WiFiManager.h>

bool WifiPortal::begin() {
  WiFi.mode(WIFI_STA);

  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);

  Serial.println("Menghubungkan WiFi...");
  bool connected = wifiManager.autoConnect("CNC-Interface-Setup");

  if (connected) {
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
