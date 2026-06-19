#ifndef BUZZER_HANDLER_H
#define BUZZER_HANDLER_H

#include <Arduino.h>

class BuzzerHandler {
  public:
    enum class Cue : uint8_t {
      Select,
      Back,
      Success,
      Warning,
      JobFinished,
      Alarm
    };

    BuzzerHandler(uint8_t pin, uint8_t pwmChannel, unsigned long alarmRepeatMs);

    void begin();
    void update();
    void play(Cue cue);
    void setAlarmActive(bool active);
    void stop();

  private:
    struct ToneStep {
      uint16_t frequency;
      uint16_t durationMs;
    };

    static const ToneStep SELECT_STEPS[];
    static const ToneStep BACK_STEPS[];
    static const ToneStep SUCCESS_STEPS[];
    static const ToneStep WARNING_STEPS[];
    static const ToneStep JOB_FINISHED_STEPS[];
    static const ToneStep ALARM_STEPS[];

    uint8_t _pin;
    uint8_t _pwmChannel;
    unsigned long _alarmRepeatMs;
    bool _started = false;
    bool _alarmActive = false;
    Cue _activeCue = Cue::Select;
    const ToneStep *_activeSteps = nullptr;
    uint8_t _activeStepCount = 0;
    uint8_t _activeStepIndex = 0;
    uint8_t _activePriority = 0;
    unsigned long _stepStartedAt = 0;
    unsigned long _lastAlarmPatternAt = 0;

    void startPattern(Cue cue, const ToneStep *steps, uint8_t stepCount, uint8_t priority);
    void startCurrentStep();
    uint8_t priorityFor(Cue cue) const;
};

#endif
