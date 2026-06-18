#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>

class TimeManager {
  public:
    void begin();
    void update(bool wifiConnected);
    void requestSync();

    bool isSynced() const;
    const char *statusText() const;
    String timeString() const;
    String dateString() const;

  private:
    bool _wifiWasConnected = false;
    bool _syncRequested = false;
    bool _syncInProgress = false;
    bool _synced = false;
    unsigned long _syncStartedAt = 0;

    void startSync();
    void finishSync(bool success);
    bool readLocalTime(struct tm &timeinfo, uint32_t timeoutMs = 1) const;
};

#endif
