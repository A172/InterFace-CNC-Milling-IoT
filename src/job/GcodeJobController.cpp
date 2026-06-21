#include "GcodeJobController.h"

#include <ctype.h>
#include <math.h>

#include "../config/AppConfig.h"

namespace {
  constexpr float PI_F = 3.14159265358979323846f;

  struct GcodeWord {
    char letter;
    float value;
  };

  uint8_t parseWords(const String &command, GcodeWord *words, uint8_t capacity) {
    uint8_t count = 0;
    const int length = command.length();
    int index = 0;

    while (index < length && count < capacity) {
      char current = command.charAt(index);
      if (!isalpha(static_cast<unsigned char>(current))) {
        index++;
        continue;
      }

      char letter = static_cast<char>(toupper(static_cast<unsigned char>(current)));
      int numberStart = ++index;
      while (index < length) {
        char numberChar = command.charAt(index);
        if (!(isdigit(static_cast<unsigned char>(numberChar)) ||
              numberChar == '+' || numberChar == '-' || numberChar == '.')) {
          break;
        }
        index++;
      }

      if (numberStart == index) {
        continue;
      }

      words[count++] = {letter, command.substring(numberStart, index).toFloat()};
    }

    return count;
  }

  bool findWord(const GcodeWord *words, uint8_t count, char letter, float &value) {
    bool found = false;
    for (uint8_t i = 0; i < count; ++i) {
      if (words[i].letter == letter) {
        value = words[i].value;
        found = true;
      }
    }
    return found;
  }

  bool hasCode(const GcodeWord *words, uint8_t count, char letter, int code) {
    for (uint8_t i = 0; i < count; ++i) {
      if (words[i].letter == letter &&
          fabsf(words[i].value - static_cast<float>(code)) < 0.0001f) {
        return true;
      }
    }
    return false;
  }

  float normalizeAngle(float angle) {
    while (angle < 0.0f) angle += 2.0f * PI_F;
    while (angle >= 2.0f * PI_F) angle -= 2.0f * PI_F;
    return angle;
  }
}

bool GcodeJobController::prepare(SdCardReader &storage, const String &path) {
  reset();
  _storage = &storage;
  _filePath = path;
  _analysisFile = _storage->openFileForRead(_filePath.c_str());
  if (!_analysisFile || _analysisFile.isDirectory()) {
    fail("File G-code gagal dibuka");
    return false;
  }

  _fileSize = _analysisFile.size();
  if (_fileSize == 0) {
    fail("File G-code kosong");
    return false;
  }

  _analysisMotion = MotionContext();
  _totalEstimateSec = 0.0f;
  _state = State::Analyzing;
  return true;
}

void GcodeJobController::updateAnalysis(uint8_t maxLines) {
  if (_state != State::Analyzing || !_analysisFile) {
    return;
  }

  for (uint8_t processed = 0; processed < maxLines; ++processed) {
    String rawLine;
    size_t endPosition = 0;
    if (!readLine(_analysisFile, rawLine, endPosition)) {
      if (_state == State::Error) {
        return;
      }
      _analysisFile.close();
      _state = State::Ready;
      return;
    }

    String command = sanitizeLine(rawLine);
    if (command.length() > 0) {
      _totalEstimateSec += estimateCommandSeconds(command, _analysisMotion);
    }
  }
}

bool GcodeJobController::start(bool returnToOrigin) {
  if (_storage == nullptr ||
      (_state != State::Ready &&
       _state != State::Completed &&
       _state != State::Stopped)) {
    return false;
  }

  _jobFile = _storage->openFileForRead(_filePath.c_str());
  if (!_jobFile || _jobFile.isDirectory()) {
    fail("File job tidak dapat dibuka");
    return false;
  }

  _senderMotion = MotionContext();
  _acknowledgedBytes = 0;
  _pendingEndPosition = 0;
  _completedEstimateSec = 0.0f;
  _pendingEstimateSec = 0.0f;
  _awaitingOk = false;
  _pendingKind = PendingKind::None;
  _pauseBarrierSent = false;
  _stopBarrierSent = false;
  _completionBarrierSent = false;
  _preambleIndex = 0;
  _preambleCount = 0;
  _spindleOn = false;
  _completedEvent = false;
  _stoppedEvent = false;
  _errorEvent = false;
  _errorMessage = "";

  _preamble[_preambleCount++] = "M400";
  if (returnToOrigin) {
    _preamble[_preambleCount++] = "G90";
    _preamble[_preambleCount++] = String("G0 Z") +
      String(AppConfig::JOB_REPEAT_SAFE_Z_MM, 2) + " F" +
      String(AppConfig::JOB_TRAVEL_FEED_MM_MIN);
    _preamble[_preambleCount++] = String("G0 X0 Y0 F") +
      String(AppConfig::JOB_TRAVEL_FEED_MM_MIN);
    _preamble[_preambleCount++] = "M400";
  }

  _startReadyAt = millis() + AppConfig::JOB_START_DRAIN_MS;
  _state = State::Starting;
  return true;
}

void GcodeJobController::update(Stream &marlin) {
  if (_state == State::Starting) {
    if (_awaitingOk || millis() < _startReadyAt) {
      return;
    }

    if (_preambleIndex < _preambleCount) {
      sendCommand(marlin, _preamble[_preambleIndex], PendingKind::Preamble);
      return;
    }

    _state = State::Running;
  }

  if (_state != State::Running &&
      _state != State::Pausing &&
      _state != State::Paused &&
      _state != State::Stopping &&
      _state != State::Completing) {
    return;
  }

  if (_awaitingOk) {
    unsigned long reference = _lastPendingResponseAt > _pendingSentAt
      ? _lastPendingResponseAt
      : _pendingSentAt;
    if (millis() - reference > AppConfig::JOB_COMMAND_TIMEOUT_MS) {
      marlin.println("M5");
      _spindleOn = false;
      fail("Timeout respons Marlin");
    }
    return;
  }

  if (_state == State::Stopping) {
    if (!_stopBarrierSent) {
      sendCommand(marlin, "M400", PendingKind::StopBarrier);
      _stopBarrierSent = true;
      return;
    }
    finishStopped(marlin);
    return;
  }

  if (_state == State::Completing) {
    if (!_completionBarrierSent) {
      sendCommand(marlin, "M400", PendingKind::CompletionBarrier);
      _completionBarrierSent = true;
      return;
    }
    finishCompleted(marlin);
    return;
  }

  if (_state == State::Pausing) {
    if (!_pauseBarrierSent) {
      sendCommand(marlin, "M400", PendingKind::PauseBarrier);
      _pauseBarrierSent = true;
      return;
    }
    _state = State::Paused;
    _lastPauseHeartbeatAt = millis();
    return;
  }

  if (_state == State::Paused) {
    if (millis() - _lastPauseHeartbeatAt >= AppConfig::MARLIN_STATUS_POLL_MS) {
      sendCommand(marlin, "M114", PendingKind::Heartbeat);
      _lastPauseHeartbeatAt = millis();
    }
    return;
  }

  String rawLine;
  size_t endPosition = 0;
  if (!readLine(_jobFile, rawLine, endPosition)) {
    if (_state != State::Error) {
      _state = State::Completing;
      _completionBarrierSent = false;
    }
    return;
  }

  String command = sanitizeLine(rawLine);
  if (command.length() == 0) {
    _acknowledgedBytes = endPosition;
    return;
  }

  _pendingEndPosition = endPosition;
  _pendingEstimateSec = estimateCommandSeconds(command, _senderMotion);
  sendCommand(marlin, command, PendingKind::Job);
}

void GcodeJobController::handleMarlinResponse(const String &line) {
  if (!_awaitingOk) {
    return;
  }

  _lastPendingResponseAt = millis();
  String normalized = line;
  normalized.trim();
  normalized.toLowerCase();

  if (normalized.startsWith("error:") ||
      normalized.startsWith("!!") ||
      normalized.startsWith("alarm:")) {
    fail(String("Marlin: ") + line);
    return;
  }

  if (!normalized.startsWith("ok")) {
    return;
  }

  _awaitingOk = false;
  if (_pendingKind == PendingKind::Preamble) {
    _preambleIndex++;
  } else if (_pendingKind == PendingKind::Job) {
    _acknowledgedBytes = _pendingEndPosition;
    _completedEstimateSec += _pendingEstimateSec;
  }
  _pendingEstimateSec = 0.0f;
  _pendingKind = PendingKind::None;
  _pendingCommand = "";
}

bool GcodeJobController::requestPause() {
  if (_state != State::Running) {
    return false;
  }
  _state = State::Pausing;
  _pauseBarrierSent = false;
  return true;
}

bool GcodeJobController::resume() {
  if (_state != State::Paused) {
    return false;
  }
  _state = State::Running;
  return true;
}

bool GcodeJobController::requestStop() {
  if (!isActive()) {
    return false;
  }
  _state = State::Stopping;
  _stopBarrierSent = false;
  return true;
}

void GcodeJobController::reset() {
  closeFiles();
  _storage = nullptr;
  _filePath = "";
  _errorMessage = "";
  _state = State::Idle;
  _fileSize = 0;
  _acknowledgedBytes = 0;
  _totalEstimateSec = 0.0f;
  _completedEstimateSec = 0.0f;
  _pendingEstimateSec = 0.0f;
  _awaitingOk = false;
  _pendingKind = PendingKind::None;
  _pauseBarrierSent = false;
  _stopBarrierSent = false;
  _completionBarrierSent = false;
  _spindleOn = false;
  _completedEvent = false;
  _stoppedEvent = false;
  _errorEvent = false;
}

GcodeJobController::State GcodeJobController::state() const {
  return _state;
}

const char *GcodeJobController::stateText() const {
  switch (_state) {
    case State::Idle: return "IDLE";
    case State::Analyzing: return "ANALYZING";
    case State::Ready: return "READY";
    case State::Starting: return "STARTING";
    case State::Running: return "RUNNING";
    case State::Pausing: return "PAUSING";
    case State::Paused: return "PAUSED";
    case State::Stopping: return "STOPPING";
    case State::Completing: return "COMPLETING";
    case State::Completed: return "COMPLETED";
    case State::Stopped: return "STOPPED";
    case State::Error: return "ERROR";
  }
  return "UNKNOWN";
}

const String &GcodeJobController::filePath() const {
  return _filePath;
}

const String &GcodeJobController::errorMessage() const {
  return _errorMessage;
}

bool GcodeJobController::estimateReady() const {
  return _state != State::Idle &&
         _state != State::Analyzing &&
         _state != State::Error;
}

bool GcodeJobController::ownsMarlinStream() const {
  return _state == State::Starting ||
         _state == State::Running ||
         _state == State::Pausing ||
         _state == State::Paused ||
         _state == State::Stopping ||
         _state == State::Completing;
}

bool GcodeJobController::isActive() const {
  return ownsMarlinStream();
}

bool GcodeJobController::spindleOn() const {
  return _spindleOn;
}

int GcodeJobController::progressPercent() const {
  if (_state == State::Idle || _state == State::Analyzing || _state == State::Ready) {
    return 0;
  }
  if (_state == State::Completed) {
    return 100;
  }
  if (_fileSize == 0) {
    return 0;
  }
  return constrain(static_cast<int>((_acknowledgedBytes * 100ULL) / _fileSize), 0, 100);
}

uint32_t GcodeJobController::totalEstimateSeconds() const {
  return static_cast<uint32_t>(ceilf(_totalEstimateSec));
}

uint32_t GcodeJobController::remainingEstimateSeconds() const {
  if (_state == State::Completed) {
    return 0;
  }

  float completed = _completedEstimateSec;
  if (_awaitingOk && _pendingKind == PendingKind::Job && _pendingEstimateSec > 0.0f) {
    float elapsed = (millis() - _pendingSentAt) / 1000.0f;
    completed += min(elapsed, _pendingEstimateSec);
  }

  float remaining = _totalEstimateSec - completed;
  return remaining > 0.0f ? static_cast<uint32_t>(ceilf(remaining)) : 0;
}

bool GcodeJobController::consumeCompletedEvent() {
  bool value = _completedEvent;
  _completedEvent = false;
  return value;
}

bool GcodeJobController::consumeStoppedEvent() {
  bool value = _stoppedEvent;
  _stoppedEvent = false;
  return value;
}

bool GcodeJobController::consumeErrorEvent() {
  bool value = _errorEvent;
  _errorEvent = false;
  return value;
}

bool GcodeJobController::readLine(File &file, String &line, size_t &endPosition) {
  line = "";
  bool consumed = false;
  bool overflow = false;

  while (file.available()) {
    int raw = file.read();
    if (raw < 0) {
      break;
    }
    consumed = true;
    char current = static_cast<char>(raw);
    if (current == '\n') {
      break;
    }
    if (current == '\r') {
      continue;
    }
    if (line.length() < AppConfig::JOB_MAX_LINE_LENGTH) {
      line += current;
    } else {
      overflow = true;
    }
  }

  endPosition = file.position();
  if (overflow) {
    fail("Baris G-code terlalu panjang");
    return false;
  }
  return consumed;
}

String GcodeJobController::sanitizeLine(const String &line) const {
  String result = line;
  int semicolon = result.indexOf(';');
  if (semicolon >= 0) {
    result.remove(semicolon);
  }

  int checksum = result.indexOf('*');
  if (checksum >= 0) {
    result.remove(checksum);
  }

  int openComment = result.indexOf('(');
  while (openComment >= 0) {
    int closeComment = result.indexOf(')', openComment + 1);
    if (closeComment < 0) {
      result.remove(openComment);
      break;
    }
    result.remove(openComment, closeComment - openComment + 1);
    openComment = result.indexOf('(');
  }

  result.trim();
  if (result.length() > 1 &&
      (result.charAt(0) == 'N' || result.charAt(0) == 'n')) {
    int index = 1;
    while (index < result.length() && isdigit(static_cast<unsigned char>(result.charAt(index)))) {
      index++;
    }
    if (index > 1) {
      result.remove(0, index);
      result.trim();
    }
  }
  if (result == "%") {
    return String();
  }
  return result;
}

float GcodeJobController::estimateCommandSeconds(
  const String &command,
  MotionContext &motion
) const {
  GcodeWord words[24];
  uint8_t count = parseWords(command, words, 24);
  if (count == 0) {
    return 0.0f;
  }

  if (hasCode(words, count, 'G', 20)) motion.unitScale = 25.4f;
  if (hasCode(words, count, 'G', 21)) motion.unitScale = 1.0f;
  if (hasCode(words, count, 'G', 90)) motion.absolute = true;
  if (hasCode(words, count, 'G', 91)) motion.absolute = false;
  if (hasCode(words, count, 'G', 17)) motion.plane = 17;
  if (hasCode(words, count, 'G', 18)) motion.plane = 18;
  if (hasCode(words, count, 'G', 19)) motion.plane = 19;

  float feed = 0.0f;
  if (findWord(words, count, 'F', feed) && feed > 0.0f) {
    motion.feedMmMin = feed * motion.unitScale;
  }

  if (hasCode(words, count, 'G', 4)) {
    float dwell = 0.0f;
    float value = 0.0f;
    if (findWord(words, count, 'P', value)) dwell += value / 1000.0f;
    if (findWord(words, count, 'S', value)) dwell += value;
    return max(dwell, 0.0f);
  }

  float xValue = 0.0f;
  float yValue = 0.0f;
  float zValue = 0.0f;
  bool hasX = findWord(words, count, 'X', xValue);
  bool hasY = findWord(words, count, 'Y', yValue);
  bool hasZ = findWord(words, count, 'Z', zValue);

  if (hasCode(words, count, 'G', 92)) {
    if (hasX) motion.x = xValue * motion.unitScale;
    if (hasY) motion.y = yValue * motion.unitScale;
    if (hasZ) motion.z = zValue * motion.unitScale;
    return 0.0f;
  }

  for (uint8_t i = 0; i < count; ++i) {
    if (words[i].letter != 'G') continue;
    int code = lroundf(words[i].value);
    if (fabsf(words[i].value - static_cast<float>(code)) < 0.0001f &&
        code >= 0 && code <= 3) {
      motion.motionMode = static_cast<int8_t>(code);
    }
  }

  float iValue = 0.0f;
  float jValue = 0.0f;
  float radiusValue = 0.0f;
  bool hasI = findWord(words, count, 'I', iValue);
  bool hasJ = findWord(words, count, 'J', jValue);
  bool hasR = findWord(words, count, 'R', radiusValue);
  bool hasMotionData = hasX || hasY || hasZ ||
    ((motion.motionMode == 2 || motion.motionMode == 3) && (hasI || hasJ || hasR));
  if (!hasMotionData) {
    return 0.0f;
  }

  float targetX = hasX
    ? (motion.absolute ? xValue * motion.unitScale : motion.x + xValue * motion.unitScale)
    : motion.x;
  float targetY = hasY
    ? (motion.absolute ? yValue * motion.unitScale : motion.y + yValue * motion.unitScale)
    : motion.y;
  float targetZ = hasZ
    ? (motion.absolute ? zValue * motion.unitScale : motion.z + zValue * motion.unitScale)
    : motion.z;

  float dx = targetX - motion.x;
  float dy = targetY - motion.y;
  float dz = targetZ - motion.z;
  float distance = sqrtf(dx * dx + dy * dy + dz * dz);

  if ((motion.motionMode == 2 || motion.motionMode == 3) && motion.plane == 17) {
    float planarLength = sqrtf(dx * dx + dy * dy);
    if (hasI || hasJ) {
      float offsetI = hasI ? iValue * motion.unitScale : 0.0f;
      float offsetJ = hasJ ? jValue * motion.unitScale : 0.0f;
      float centerX = motion.x + offsetI;
      float centerY = motion.y + offsetJ;
      float radius = sqrtf(offsetI * offsetI + offsetJ * offsetJ);
      if (radius > 0.0f) {
        float startAngle = atan2f(motion.y - centerY, motion.x - centerX);
        float endAngle = atan2f(targetY - centerY, targetX - centerX);
        float sweep = motion.motionMode == 2
          ? normalizeAngle(startAngle - endAngle)
          : normalizeAngle(endAngle - startAngle);
        if (sweep < 0.0001f && planarLength < 0.0001f) {
          sweep = 2.0f * PI_F;
        }
        planarLength = radius * sweep;
      }
    } else if (hasR) {
      float radius = fabsf(radiusValue * motion.unitScale);
      if (radius > 0.0f && planarLength <= 2.0f * radius) {
        float minorSweep = 2.0f * asinf(planarLength / (2.0f * radius));
        float sweep = radiusValue < 0.0f ? 2.0f * PI_F - minorSweep : minorSweep;
        planarLength = radius * sweep;
      }
    }
    distance = sqrtf(planarLength * planarLength + dz * dz);
  }

  motion.x = targetX;
  motion.y = targetY;
  motion.z = targetZ;

  float effectiveFeed = motion.motionMode == 0
    ? static_cast<float>(AppConfig::JOB_TRAVEL_FEED_MM_MIN)
    : motion.feedMmMin;
  if (effectiveFeed <= 0.0f) {
    return 0.0f;
  }
  return distance * 60.0f / effectiveFeed;
}

void GcodeJobController::sendCommand(
  Stream &marlin,
  const String &command,
  PendingKind kind
) {
  marlin.println(command);
  _pendingCommand = command;
  _pendingKind = kind;
  _awaitingOk = true;
  _pendingSentAt = millis();
  _lastPendingResponseAt = _pendingSentAt;

  GcodeWord words[24];
  uint8_t wordCount = parseWords(command, words, 24);
  if (hasCode(words, wordCount, 'M', 3) || hasCode(words, wordCount, 'M', 4)) {
    _spindleOn = true;
  } else if (hasCode(words, wordCount, 'M', 5)) {
    _spindleOn = false;
  }
}

void GcodeJobController::finishCompleted(Stream &marlin) {
  marlin.println("M5");
  _spindleOn = false;
  _acknowledgedBytes = _fileSize;
  closeFiles();
  _state = State::Completed;
  _completedEvent = true;
}

void GcodeJobController::finishStopped(Stream &marlin) {
  marlin.println("M5");
  _spindleOn = false;
  closeFiles();
  _state = State::Stopped;
  _stoppedEvent = true;
}

void GcodeJobController::fail(const String &message) {
  closeFiles();
  _state = State::Error;
  _errorMessage = message;
  _awaitingOk = false;
  _pendingCommand = "";
  _spindleOn = false;
  _errorEvent = true;
}

void GcodeJobController::closeFiles() {
  if (_analysisFile) _analysisFile.close();
  if (_jobFile) _jobFile.close();
}
