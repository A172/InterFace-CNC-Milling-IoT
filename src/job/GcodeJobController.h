#ifndef GCODE_JOB_CONTROLLER_H
#define GCODE_JOB_CONTROLLER_H

#include <Arduino.h>
#include <FS.h>

#include "../storage/SdCardReader.h"

class GcodeJobController {
  public:
    enum class State {
      Idle,
      Analyzing,
      Ready,
      Starting,
      Running,
      Pausing,
      Paused,
      Stopping,
      Completing,
      Completed,
      Stopped,
      Error
    };

    bool prepare(SdCardReader &storage, const String &path);
    void updateAnalysis(uint8_t maxLines);
    bool start(bool returnToOrigin = false);
    void update(Stream &marlin);
    void handleMarlinResponse(const String &line);

    bool requestPause();
    bool resume();
    bool requestStop();
    void reset();

    State state() const;
    const char *stateText() const;
    const String &filePath() const;
    const String &errorMessage() const;
    bool estimateReady() const;
    bool ownsMarlinStream() const;
    bool isActive() const;
    bool spindleOn() const;
    int progressPercent() const;
    uint32_t totalEstimateSeconds() const;
    uint32_t remainingEstimateSeconds() const;

    bool consumeCompletedEvent();
    bool consumeStoppedEvent();
    bool consumeErrorEvent();

  private:
    enum class PendingKind {
      None,
      Preamble,
      Job,
      Heartbeat,
      PauseBarrier,
      StopBarrier,
      CompletionBarrier
    };

    struct MotionContext {
      float x = 0.0f;
      float y = 0.0f;
      float z = 0.0f;
      float feedMmMin = 600.0f;
      float unitScale = 1.0f;
      bool absolute = true;
      uint8_t plane = 17;
      int8_t motionMode = 1;
    };

    SdCardReader *_storage = nullptr;
    File _analysisFile;
    File _jobFile;
    String _filePath;
    String _errorMessage;
    String _pendingCommand;
    String _preamble[5];
    State _state = State::Idle;
    MotionContext _analysisMotion;
    MotionContext _senderMotion;
    size_t _fileSize = 0;
    size_t _acknowledgedBytes = 0;
    size_t _pendingEndPosition = 0;
    float _totalEstimateSec = 0.0f;
    float _completedEstimateSec = 0.0f;
    float _pendingEstimateSec = 0.0f;
    uint8_t _preambleCount = 0;
    uint8_t _preambleIndex = 0;
    unsigned long _startReadyAt = 0;
    unsigned long _pendingSentAt = 0;
    unsigned long _lastPendingResponseAt = 0;
    unsigned long _lastPauseHeartbeatAt = 0;
    bool _awaitingOk = false;
    PendingKind _pendingKind = PendingKind::None;
    bool _pauseBarrierSent = false;
    bool _stopBarrierSent = false;
    bool _completionBarrierSent = false;
    bool _spindleOn = false;
    bool _completedEvent = false;
    bool _stoppedEvent = false;
    bool _errorEvent = false;

    bool readLine(File &file, String &line, size_t &endPosition);
    String sanitizeLine(const String &line) const;
    float estimateCommandSeconds(const String &command, MotionContext &motion) const;
    void sendCommand(Stream &marlin, const String &command, PendingKind kind);
    void finishCompleted(Stream &marlin);
    void finishStopped(Stream &marlin);
    void fail(const String &message);
    void closeFiles();
};

#endif
