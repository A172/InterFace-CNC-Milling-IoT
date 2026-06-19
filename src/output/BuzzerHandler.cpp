#include "BuzzerHandler.h"

const BuzzerHandler::ToneStep BuzzerHandler::SELECT_STEPS[] = {
  {2400, 35}
};

const BuzzerHandler::ToneStep BuzzerHandler::BACK_STEPS[] = {
  {1400, 60}
};

const BuzzerHandler::ToneStep BuzzerHandler::SUCCESS_STEPS[] = {
  {1800, 70}, {0, 45}, {2400, 90}
};

const BuzzerHandler::ToneStep BuzzerHandler::WARNING_STEPS[] = {
  {1000, 180}, {0, 100}, {1000, 180}
};

const BuzzerHandler::ToneStep BuzzerHandler::JOB_FINISHED_STEPS[] = {
  {1800, 100}, {0, 80}, {2200, 100}, {0, 80}, {2800, 150}
};

const BuzzerHandler::ToneStep BuzzerHandler::ALARM_STEPS[] = {
  {700, 250}, {0, 120}, {700, 250}, {0, 120}, {700, 250}
};

BuzzerHandler::BuzzerHandler(
  uint8_t pin,
  uint8_t pwmChannel,
  unsigned long alarmRepeatMs
) : _pin(pin),
    _pwmChannel(pwmChannel),
    _alarmRepeatMs(alarmRepeatMs) {
}

void BuzzerHandler::begin() {
  if (_started) {
    return;
  }

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  ledcSetup(_pwmChannel, 2000, 8);
  ledcAttachPin(_pin, _pwmChannel);
  ledcWriteTone(_pwmChannel, 0);
  _started = true;
}

void BuzzerHandler::update() {
  if (!_started) {
    return;
  }

  unsigned long now = millis();
  if (_activeSteps != nullptr) {
    const ToneStep &step = _activeSteps[_activeStepIndex];
    if (now - _stepStartedAt >= step.durationMs) {
      _activeStepIndex++;
      if (_activeStepIndex >= _activeStepCount) {
        ledcWriteTone(_pwmChannel, 0);
        if (_activeCue == Cue::Alarm) {
          _lastAlarmPatternAt = now;
        }
        _activeSteps = nullptr;
        _activeStepCount = 0;
        _activeStepIndex = 0;
        _activePriority = 0;
      } else {
        startCurrentStep();
      }
    }
  }

  if (_alarmActive &&
      _activeSteps == nullptr &&
      (now - _lastAlarmPatternAt >= _alarmRepeatMs)) {
    play(Cue::Alarm);
  }
}

void BuzzerHandler::play(Cue cue) {
  if (!_started) {
    return;
  }

  uint8_t priority = priorityFor(cue);
  if (_activeSteps != nullptr && priority < _activePriority) {
    return;
  }

  switch (cue) {
    case Cue::Select:
      startPattern(cue, SELECT_STEPS, 1, priority);
      break;
    case Cue::Back:
      startPattern(cue, BACK_STEPS, 1, priority);
      break;
    case Cue::Success:
      startPattern(cue, SUCCESS_STEPS, 3, priority);
      break;
    case Cue::Warning:
      startPattern(cue, WARNING_STEPS, 3, priority);
      break;
    case Cue::JobFinished:
      startPattern(cue, JOB_FINISHED_STEPS, 5, priority);
      break;
    case Cue::Alarm:
      startPattern(cue, ALARM_STEPS, 5, priority);
      break;
  }
}

void BuzzerHandler::setAlarmActive(bool active) {
  if (!_started || active == _alarmActive) {
    return;
  }

  _alarmActive = active;
  if (active) {
    _lastAlarmPatternAt = millis() - _alarmRepeatMs;
    play(Cue::Alarm);
  } else if (_activeSteps != nullptr && _activeCue == Cue::Alarm) {
    stop();
  }
}

void BuzzerHandler::stop() {
  if (!_started) {
    return;
  }

  ledcWriteTone(_pwmChannel, 0);
  _activeSteps = nullptr;
  _activeStepCount = 0;
  _activeStepIndex = 0;
  _activePriority = 0;
}

void BuzzerHandler::startPattern(
  Cue cue,
  const ToneStep *steps,
  uint8_t stepCount,
  uint8_t priority
) {
  _activeCue = cue;
  _activeSteps = steps;
  _activeStepCount = stepCount;
  _activeStepIndex = 0;
  _activePriority = priority;
  startCurrentStep();
}

void BuzzerHandler::startCurrentStep() {
  const ToneStep &step = _activeSteps[_activeStepIndex];
  ledcWriteTone(_pwmChannel, step.frequency);
  _stepStartedAt = millis();
}

uint8_t BuzzerHandler::priorityFor(Cue cue) const {
  switch (cue) {
    case Cue::Select:
    case Cue::Back:
      return 1;
    case Cue::Success:
      return 2;
    case Cue::JobFinished:
      return 3;
    case Cue::Warning:
      return 4;
    case Cue::Alarm:
      return 5;
  }

  return 0;
}
