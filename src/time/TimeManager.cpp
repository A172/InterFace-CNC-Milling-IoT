#include "TimeManager.h"

#include "../config/AppConfig.h"

void TimeManager::begin() {
  _wifiWasConnected = false;
  _syncRequested = false;
  _syncInProgress = false;
  _synced = false;
  _syncStartedAt = 0;
}

void TimeManager::update(bool wifiConnected) {
  if (wifiConnected && !_wifiWasConnected) {
    Serial.println("[TIME] WiFi connected");
    requestSync();
  }

  _wifiWasConnected = wifiConnected;

  if (!wifiConnected) {
    if (_syncInProgress) {
      finishSync(false);
    }
    return;
  }

  if (_syncRequested && !_syncInProgress) {
    startSync();
  }

  if (!_syncInProgress) {
    return;
  }

  struct tm timeinfo;
  if (readLocalTime(timeinfo, 1)) {
    finishSync(true);
    return;
  }

  if (millis() - _syncStartedAt >= AppConfig::NTP_SYNC_TIMEOUT_MS) {
    finishSync(false);
  }
}

void TimeManager::requestSync() {
  _syncRequested = true;
}

bool TimeManager::isSynced() const {
  return _synced;
}

const char *TimeManager::statusText() const {
  return _synced ? "TIME_SYNCED" : "TIME_NOT_SYNCED";
}

String TimeManager::timeString() const {
  struct tm timeinfo;
  if (!readLocalTime(timeinfo, 1)) {
    return String();
  }

  char buffer[9];
  snprintf(
    buffer,
    sizeof(buffer),
    "%02d:%02d:%02d",
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec
  );
  return String(buffer);
}

String TimeManager::dateString() const {
  struct tm timeinfo;
  if (!readLocalTime(timeinfo, 1)) {
    return String("--/--/----");
  }

  char buffer[11];
  snprintf(
    buffer,
    sizeof(buffer),
    "%02d/%02d/%04d",
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1,
    timeinfo.tm_year + 1900
  );
  return String(buffer);
}

void TimeManager::startSync() {
  Serial.println("[TIME] NTP sync started");
  configTime(
    AppConfig::WIB_GMT_OFFSET_SEC,
    AppConfig::WIB_DAYLIGHT_OFFSET_SEC,
    AppConfig::NTP_SERVER_1,
    AppConfig::NTP_SERVER_2
  );

  _syncRequested = false;
  _syncInProgress = true;
  _syncStartedAt = millis();
}

void TimeManager::finishSync(bool success) {
  _syncInProgress = false;
  _syncRequested = false;

  if (success) {
    _synced = true;
    Serial.println("[TIME] NTP sync success");
    return;
  }

  Serial.println("[TIME] NTP sync failed, using internal time");
}

bool TimeManager::readLocalTime(struct tm &timeinfo, uint32_t timeoutMs) const {
  return getLocalTime(&timeinfo, timeoutMs);
}
