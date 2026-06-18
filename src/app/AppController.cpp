#include "AppController.h"
#include <SD.h>
#include <time.h>
#include <WiFi.h>

namespace {
  // Penghubung callback dari modul input ke objek aplikasi yang sedang aktif.
  AppController *activeApp = nullptr;

  void handleButtonPressed(uint8_t buttonNumber) {
    if (activeApp != nullptr) {
      activeApp->onButtonPressed(buttonNumber);
    }
  }

  void handleEncoderTurned(int8_t direction) {
    if (activeApp != nullptr) {
      activeApp->onEncoderTurned(direction);
    }
  }

  void handleEncoderPressed() {
    if (activeApp != nullptr) {
      activeApp->onEncoderPressed();
    }
  }

  String basenameForLcd(const String &path) {
    String normalized = path;
    normalized.replace("//", "/");

    int slashIndex = normalized.lastIndexOf('/');
    if (slashIndex >= 0 && slashIndex < (int)normalized.length() - 1) {
      return normalized.substring(slashIndex + 1);
    }

    return normalized;
  }

  std::vector<String> mainMenuItems() {
    return {
      "Move & Jog",
      "Select Job",
      "Machine Control",
      "Network",
      "Settings"
    };
  }
}

// --------------------
// Penyusunan modul hardware
// --------------------

AppController::AppController()
  : _buttons(
      PinConfig::BTN1,
      PinConfig::BTN2,
      PinConfig::BTN3,
      PinConfig::BTN4,
      PinConfig::BTN5,
      PinConfig::BTN6,
      PinConfig::BTN7,
      PinConfig::BTN8
    ),
    _encoder(
      PinConfig::ENCODER_CLK,
      PinConfig::ENCODER_DT,
      PinConfig::ENCODER_SW
    ),
    _lcd(),
    _menu(_lcd),
    _sdCard(
      PinConfig::SD_CS,
      PinConfig::SD_SCK,
      PinConfig::SD_MISO,
      PinConfig::SD_MOSI
    ),
    _jogAxisIndex(0),
    _isJogAdjusting(false),
    _encoderMovedWhilePressed(false) {
}

void AppController::pushMenuState(const std::vector<String> &items, size_t selected) {
  _menuHistory.push_back({_menu.title(), items, selected});
}

bool AppController::popMenuState() {
  if (_menuHistory.empty()) return false;
  MenuState entry = _menuHistory.back();
  _menuHistory.pop_back();
  _menu.showMenu(entry.title.c_str(), entry.items, entry.selected);
  return true;
}

void AppController::showStandbyScreen() {
  _uiState = UiState::Standby;

  String jobName = computeJobName();
  String eta = computeEta();
  String time = getLocalTimeString();
  const char *jobNamePtr = jobName.length() ? jobName.c_str() : nullptr;
  const char *etaPtr = eta.length() ? eta.c_str() : nullptr;
  const char *timePtr = time.length() ? time.c_str() : nullptr;

  _menu.showStandby(
    _posX,
    _posY,
    _posZ,
    etaPtr,
    jobNamePtr,
    computeProgress(),
    timePtr
  );
}

void AppController::showMainMenuScreen(size_t selected) {
  _menuHistory.clear();
  _uiState = UiState::Menu;
  _menu.showMainMenu(mainMenuItems(), selected);
}

void AppController::showSubMenu(const char *title, const std::vector<String> &items) {
  pushMenuState(_menu.items(), _menu.selectedIndex());
  _uiState = UiState::Menu;
  _menu.showMenu(title, items, 0);
}

void AppController::showInfoScreen(
  const char *title,
  const char *message,
  UiState returnState
) {
  _infoReturnState = returnState;
  _uiState = UiState::Info;
  _lcd.showMessage(title, message);
}

void AppController::dismissInfoScreen() {
  _uiState = _infoReturnState;

  if (_uiState == UiState::Standby) {
    showStandbyScreen();
  } else if (_uiState == UiState::SetOrigin) {
    updateJogDisplay();
  } else {
    _uiState = UiState::Menu;
    _menu.redraw();
  }
}

void AppController::showActionConfirm(
  PendingAction action,
  const char *title,
  const char *message
) {
  _activeJogPin = 0;
  _pendingAction = action;
  _confirmReturnState = _uiState;
  _confirmTitle = title;
  _confirmMessage = message;
  _confirmSelectedIndex = 1;
  _uiState = UiState::ConfirmAction;
  redrawActionConfirm();
}

void AppController::redrawActionConfirm() {
  _lcd.showConfirm(
    _confirmTitle.c_str(),
    _confirmMessage.c_str(),
    _confirmSelectedIndex
  );
}

void AppController::cancelActionConfirm() {
  PendingAction cancelledAction = _pendingAction;
  _pendingAction = PendingAction::None;
  _uiState = _confirmReturnState;

  if (cancelledAction == PendingAction::SetOrigin) {
    updateJogDisplay();
  } else {
    _menu.redraw();
  }
}

void AppController::executeConfirmedAction() {
  PendingAction action = _pendingAction;
  _pendingAction = PendingAction::None;

  if (action == PendingAction::HomeAll) {
    if (AppConfig::ENABLE_HOME_CONTROL) {
      Serial1.println("G28");
      showInfoScreen("HOME ALL", "G28 dikirim");
    } else {
      showInfoScreen("UI PREVIEW", "G28 belum dikirim");
    }
    return;
  }

  if (action == PendingAction::SetOrigin) {
    confirmSetOrigin();
    return;
  }

  cancelActionConfirm();
}

bool AppController::isConfirmationState() const {
  return _uiState == UiState::ConfirmAction ||
         _uiState == UiState::ConfirmSelect;
}

void AppController::acceptConfirmation() {
  if (_uiState == UiState::ConfirmAction) {
    executeConfirmedAction();
    return;
  }

  if (_uiState == UiState::ConfirmSelect) {
    selectFileFromList();
    showStandbyScreen();
  }
}

void AppController::rejectConfirmation() {
  if (_uiState == UiState::ConfirmAction) {
    cancelActionConfirm();
    return;
  }

  if (_uiState == UiState::ConfirmSelect) {
    _uiState = UiState::FileList;
    const size_t pageSize = 5;
    size_t end = min(_fileListOffset + pageSize, _fileList.size());
    std::vector<String> page;
    for (size_t i = _fileListOffset; i < end; ++i) {
      page.push_back(_fileList[i]);
    }
    _lcd.showMenu("Files", page, _fileListSelected - _fileListOffset, 0);
  }
}

void AppController::executeSelectedConfirmation() {
  if (_confirmSelectedIndex == 0) {
    acceptConfirmation();
  } else {
    rejectConfirmation();
  }
}

void AppController::redrawCurrentConfirmation() {
  if (_uiState == UiState::ConfirmAction) {
    redrawActionConfirm();
  } else if (_uiState == UiState::ConfirmSelect) {
    _lcd.showConfirm("Confirm", "Set as job?", _confirmSelectedIndex);
  }
}


// --------------------
// Siklus utama aplikasi
// --------------------

void AppController::begin() {
  activeApp = this;
  beginSerial();
  beginDisplay();
  beginInput();
  beginStorage();

  if (AppConfig::ENABLE_NETWORK) {
    beginNetwork();
  }
}


String AppController::computeJobName() const {
  if (_selectedJobSource == JobSource::SdCard && _sdCard.hasSelectedJobFile()) {
    return _sdCard.selectedJobFile();
  }
  return String();
}

String AppController::computeEta() const {
  // Placeholder: Nantinya dihitung berdasarkan sisa baris G-code / kecepatan
  if (_selectedJobSource == JobSource::SdCard && _sdCard.hasSelectedJobFile()) {
    return String("--:--");
  }
  return String();
}

int AppController::computeProgress() const {
  // Pemilihan file belum berarti job sedang berjalan.
  return -1;
}

String AppController::getLocalTimeString() const {
  if (!_networkStarted) {
    return String();
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo, 10)) {
    // Jika NTP belum sinkron, return string kosong agar jam tidak muncul
    return String("");
  }
  char timeStr[6]; // HH:MM
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  return String(timeStr);
}

void AppController::update() {
  if (_inputStarted) {
    updateInput();
  }

  updateMarlinCommunication();

  // Logika Long Press Jogging
  if (_activeJogPin != 0 && _uiState == UiState::SetOrigin) {
    if (digitalRead(_activeJogPin) == HIGH) {
      // Tekan singkat baru dijalankan saat tombol dilepas. Dengan cara ini,
      // long-press tidak didahului oleh gerakan 1 mm.
      if (!_jogLongPressHandled) {
        handleJog(_activeJogPin, AppConfig::JOG_STEP_SHORT_MM);
      }
      _activeJogPin = 0;
    } else {
      uint32_t pressDuration = millis() - _jogStartTime;
      if (!_jogLongPressHandled &&
          pressDuration >= AppConfig::BUTTON_LONG_PRESS_MS) {
        handleJog(_activeJogPin, AppConfig::JOG_STEP_LONG_MM);
        _jogLongPressHandled = true;
        _lastJogRepeatTick = millis();
      } else if (_jogLongPressHandled &&
                 millis() - _lastJogRepeatTick >= AppConfig::BUTTON_JOG_REPEAT_MS) {
        handleJog(_activeJogPin, AppConfig::JOG_STEP_LONG_MM);
        _lastJogRepeatTick = millis();
      }
    }
  }

  updateNetwork();
  if (_storageStarted) {
    updateStorageFileDisplay();
  }
}

// --------------------
// Komunikasi Marlin (Polling & Parsing)
// --------------------

void AppController::updateMarlinCommunication() {
  bool setOriginControlActive =
    _uiState == UiState::SetOrigin &&
    AppConfig::ENABLE_SET_ORIGIN_CONTROL;

  if (AppConfig::UI_DEVELOPMENT_MODE && !setOriginControlActive) {
    return;
  }

  // 1. Baca data masuk dari Marlin
  while (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      parseMarlinResponse(line);
    }
  }

  // 2. Kirim permintaan posisi (M114) setiap 500ms jika sedang standby atau jog
  if (_uiState == UiState::Standby || _uiState == UiState::SetOrigin) {
    if (millis() - _lastM114Request > 500) {
      Serial1.println("M114");
      _lastM114Request = millis();
    }
  }
}

void AppController::parseMarlinResponse(const String &line) {
  // Format standar Marlin M114: "X:0.00 Y:0.00 Z:0.00 E:0.00 Count X:0 Y:0 Z:0"
  if (line.indexOf("X:") != -1 && line.indexOf("Y:") != -1 && line.indexOf("Z:") != -1) {
    int xPos = line.indexOf("X:") + 2;
    int yPos = line.indexOf("Y:") + 2;
    int zPos = line.indexOf("Z:") + 2;
    int ePos = line.indexOf("E:");

    String xStr = line.substring(xPos, line.indexOf(' ', xPos));
    String yStr = line.substring(yPos, line.indexOf(' ', yPos));
    String zStr = (ePos != -1)
                  ? line.substring(zPos, line.indexOf(' ', zPos))
                  : line.substring(zPos);

    _posX = xStr.toFloat();
    _posY = yStr.toFloat();
    _posZ = zStr.toFloat();

    // Update tampilan jika sedang standby agar angka sinkron
    if (_uiState == UiState::Standby) {
      showStandbyScreen();
    } else if (_uiState == UiState::SetOrigin) {
      updateJogDisplay();
    }
  }
}

// --------------------
// Event dari input
// --------------------

void AppController::onButtonPressed(uint8_t buttonNumber) {
  Serial.print("Event tombol dikirim ke aplikasi: ");
  Serial.println(buttonNumber);

  if (_uiState == UiState::Info) {
    if (buttonNumber == (uint8_t)PinConfig::BTN7 ||
        buttonNumber == (uint8_t)PinConfig::BTN8) {
      dismissInfoScreen();
    }
    return;
  }

  // Jika sedang standby dan tombol ENTER ditekan -> masuk menu
  if (_uiState == UiState::Standby && buttonNumber == (uint8_t)PinConfig::BTN7) {
    showMainMenuScreen();
    return;
  }

    // NOTE: tombol khusus jog sebelumnya hanya diizinkan saat mode Jogging.
    // Sekarang mode "Jogging" digantikan oleh menu "Home/G92" sehingga
    // penanganan per-axis harus diimplementasikan jika diperlukan.

  // If in FileList view, BTN6 previews the currently highlighted file on LCD
  if (_uiState == UiState::FileList && buttonNumber == (uint8_t)PinConfig::BTN6) {
    // preview from selected storage source
    if (_fileListSource == JobSource::SdCard) {
      if (_fileListSelected < _fileListPaths.size()) {
        // Show a small textual preview on the LCD using SdCardReader helper
        std::vector<String> preview = _sdCard.previewFileLines(_fileListPaths[_fileListSelected].c_str(), 4, 18);
        if (preview.empty()) {
          _lcd.showMessage("Preview", "Tidak dapat membaca file");
        } else {
          _lcd.showList("Preview", preview);
        }
      }
    }
    return;
  }

  // Jika sedang di menu dan tombol BACK ditekan -> kembali ke menu sebelumnya atau Standby
  if (_uiState == UiState::Menu && buttonNumber == (uint8_t)PinConfig::BTN8) {
    if (!popMenuState()) {
      showStandbyScreen();
    }
    return;
  }

  // BACK handling for submenus is covered by Menu history/pop behavior above

  // Jika sedang melihat daftar file dan BACK ditekan -> kembali ke Menu
  if (_uiState == UiState::FileList && buttonNumber == (uint8_t)PinConfig::BTN8) {
    _uiState = UiState::Menu;
    if (!popMenuState()) {
      showMainMenuScreen(1);
    }
    return;
  }

  if (isConfirmationState()) {
    if (buttonNumber == (uint8_t)PinConfig::BTN7) {
      // Tombol ENTER fisik selalu berarti Yes.
      acceptConfirmation();
      return;
    }

    if (buttonNumber == (uint8_t)PinConfig::BTN8) {
      // Tombol BACK fisik selalu berarti No.
      rejectConfirmation();
      return;
    }

    return;
  }

  if (_uiState == UiState::SetOrigin) {
    if (buttonNumber == (uint8_t)PinConfig::BTN7) {
      showActionConfirm(
        PendingAction::SetOrigin,
        "CONFIRM ORIGIN",
        "Set posisi Origin?"
      );
      return;
    }

    if (buttonNumber == (uint8_t)PinConfig::BTN8) {
      _activeJogPin = 0;
      _isJogAdjusting = false;
      _uiState = UiState::Menu;
      _menu.redraw();
      return;
    }

    bool isJogButton = (buttonNumber == PinConfig::BTN1 || buttonNumber == PinConfig::BTN2 ||
                        buttonNumber == PinConfig::BTN3 || buttonNumber == PinConfig::BTN4 ||
                        buttonNumber == PinConfig::BTN5 || buttonNumber == PinConfig::BTN6);

    if (isJogButton) {
      _activeJogPin = buttonNumber;
      _jogStartTime = millis();
      _jogLongPressHandled = false;
      _lastJogRepeatTick = 0;
    }

    updateJogDisplay();
    return;
  }

  // If button is ENTER (BTN7) treat as select/OK
  if (buttonNumber == (uint8_t)PinConfig::BTN7) {
    processSelectAction();
    return;
  }

  // Publikasikan event ke cloud jika tersedia
  if (_networkStarted) {
    _cloud.publishButtonEvent(buttonNumber);
  }
}

void AppController::handleJog(uint8_t pin, float distance) {
  char axis = 'X';
  float direction = 1.0f;

  if (pin == PinConfig::BTN1) { axis = 'X'; direction = 1.0f; }
  else if (pin == PinConfig::BTN2) { axis = 'X'; direction = -1.0f; }
  else if (pin == PinConfig::BTN3) { axis = 'Y'; direction = 1.0f; }
  else if (pin == PinConfig::BTN4) { axis = 'Y'; direction = -1.0f; }
  else if (pin == PinConfig::BTN5) { axis = 'Z'; direction = 1.0f; }
  else if (pin == PinConfig::BTN6) { axis = 'Z'; direction = -1.0f; }
  else return;

  float moveVal = distance * direction;
  if (axis == 'X') _posX += moveVal;
  else if (axis == 'Y') _posY += moveVal;
  else if (axis == 'Z') _posZ += moveVal;

  if (AppConfig::ENABLE_SET_ORIGIN_CONTROL) {
    // Kirim G-Code ke SKR (G91 = Relative Positioning)
    Serial1.println("G91");
    Serial1.print("G1 ");
    Serial1.print(axis);
    Serial1.print(moveVal);
    Serial1.print(" F");
    Serial1.println(AppConfig::JOG_SPEED_NORMAL);
    Serial1.println("G90");
  }

  updateJogDisplay();
}

void AppController::updateJogDisplay() {
  if (_uiState != UiState::SetOrigin) return;

  char top[32], mid[32], bot[32];
  char axis = _jogAxisIndex == 0 ? 'X' : (_jogAxisIndex == 1 ? 'Y' : 'Z');
  snprintf(
    top,
    sizeof(top),
    _isJogAdjusting ? "ORIGIN ADJUST %c" : "ORIGIN SELECT %c",
    axis
  );

  // Tanda kursor: > untuk seleksi, * untuk mode ubah nilai
  char xMark = (_jogAxisIndex == 0) ? (_isJogAdjusting ? '*' : '>') : ' ';
  char yMark = (_jogAxisIndex == 1) ? (_isJogAdjusting ? '*' : '>') : ' ';
  char zMark = (_jogAxisIndex == 2) ? (_isJogAdjusting ? '*' : '>') : ' ';

  snprintf(mid, sizeof(mid), "%cX:%7.2f %cY:%7.2f", xMark, _posX, yMark, _posY);
  snprintf(bot, sizeof(bot), "%cZ:%7.2f", zMark, _posZ);

  _lcd.showStandbyLines(
    top,
    mid,
    bot,
    nullptr,
    "ENTER=SET BACK=EXIT",
    -1,
    nullptr
  );
}

void AppController::enterSetOrigin() {
  _uiState = UiState::SetOrigin;
  _jogAxisIndex = 0;
  _isJogAdjusting = false;
  _encoderMovedWhilePressed = false;
  _activeJogPin = 0;
  _lastM114Request = 0;
  updateJogDisplay();
}

void AppController::confirmSetOrigin() {
  _activeJogPin = 0;
  _isJogAdjusting = false;

  if (AppConfig::ENABLE_SET_ORIGIN_CONTROL) {
    Serial1.println("M400");
    Serial1.println("G92 X0 Y0 Z0");
  }

  _posX = 0.0f;
  _posY = 0.0f;
  _posZ = 0.0f;
  showInfoScreen(
    "SET ORIGIN",
    AppConfig::ENABLE_SET_ORIGIN_CONTROL ? "Origin disimpan" : "Simulasi disimpan"
  );
}

void AppController::processSelectAction() {
  if (_uiState == UiState::Standby) {
    showMainMenuScreen();
    return;
  }

  // Klik rotary mengikuti pilihan Yes/No yang sedang disorot.
  if (isConfirmationState()) {
    executeSelectedConfirmation();
    return;
  }

  if (_uiState == UiState::Info) {
    dismissInfoScreen();
    return;
  }

  if (_uiState == UiState::Menu) {
    size_t sel = _menu.selectedIndex();
    const std::vector<String> &items = _menu.items();
    if (sel >= items.size()) {
      return;
    }

    String choice = items[sel];

    if (choice == "Move & Jog") {
      showSubMenu(
        "MOVE & JOG",
        {"Home All (G28)", "Set Origin (G92)"}
      );
      return;
    }

    if (choice == "Select Job") {
      showSubMenu(
        "SELECT JOB",
        {"Browse SD Card", "Selected File", "Job Status"}
      );
      return;
    }

    if (choice == "Machine Control") {
      showSubMenu(
        "MACHINE CONTROL",
        {"Spindle ON (M3)", "Spindle OFF (M5)", "Fan ON", "Fan OFF"}
      );
      return;
    }

    if (choice == "Network") {
      showSubMenu(
        "NETWORK",
        {"Connection Status", "IP Address", "Reset WiFi"}
      );
      return;
    }

    if (choice == "Settings") {
      showSubMenu(
        "SETTINGS",
        {"UI Information", "Jog Step", "About Firmware"}
      );
      return;
    }

    if (choice == "Home All (G28)") {
      showActionConfirm(
        PendingAction::HomeAll,
        "CONFIRM HOME",
        "Jalankan Home All?"
      );
      return;
    }

    if (choice == "Set Origin (G92)") {
      enterSetOrigin();
      return;
    }

    if (choice == "Browse SD Card") {
      if (AppConfig::UI_DEVELOPMENT_MODE) {
        showInfoScreen("UI PREVIEW", "File browser tahap 2");
        return;
      }

      if (!_sdCard.isReady()) {
        showInfoScreen("SD CARD", "Belum siap");
        return;
      }

      pushMenuState(items, sel);
      _fileListPaths = _sdCard.listFileNames("/", 0, 50);
      _fileList.clear();
      for (const String &path : _fileListPaths) {
        _fileList.push_back(basenameForLcd(path));
      }
      _fileListSelected = 0;
      _fileListOffset = 0;
      _fileListSource = JobSource::SdCard;
      _uiState = UiState::FileList;

      size_t end = min((size_t)5, _fileList.size());
      std::vector<String> page(_fileList.begin(), _fileList.begin() + end);
      _lcd.showMenu("FILES", page, 0);
      return;
    }

    if (choice == "Selected File") {
      String selected = computeJobName();
      showInfoScreen(
        "SELECTED FILE",
        selected.length() ? selected.c_str() : "Belum ada file"
      );
      return;
    }

    if (choice == "Job Status") {
      showInfoScreen("JOB STATUS", "Belum ada job aktif");
      return;
    }

    if (choice == "Spindle ON (M3)") {
      if (AppConfig::UI_DEVELOPMENT_MODE) {
        showInfoScreen("UI PREVIEW", "M3 belum dikirim");
      } else {
        Serial1.println("M3");
        showInfoScreen("SPINDLE", "M3 dikirim");
      }
      return;
    }

    if (choice == "Spindle OFF (M5)") {
      if (AppConfig::UI_DEVELOPMENT_MODE) {
        showInfoScreen("UI PREVIEW", "M5 belum dikirim");
      } else {
        Serial1.println("M5");
        showInfoScreen("SPINDLE", "M5 dikirim");
      }
      return;
    }

    if (choice == "Fan ON" || choice == "Fan OFF") {
      showInfoScreen("UI PREVIEW", "Kontrol fan tahap 2");
      return;
    }

    if (choice == "Connection Status") {
      showInfoScreen(
        "NETWORK STATUS",
        _wifi.isConnected() ? "WiFi connected" : "WiFi disconnected"
      );
      return;
    }

    if (choice == "IP Address") {
      String ip = _wifi.isConnected() ? WiFi.localIP().toString() : "Belum terhubung";
      showInfoScreen("IP ADDRESS", ip.c_str());
      return;
    }

    if (choice == "Reset WiFi") {
      showInfoScreen("UI PREVIEW", "Reset WiFi tahap 2");
      return;
    }

    if (choice == "UI Information") {
      showInfoScreen("UI MODE", "Navigasi aktif");
      return;
    }

    if (choice == "Jog Step") {
      showInfoScreen("JOG STEP", "Default 1.00 mm");
      return;
    }

    if (choice == "About Firmware") {
      showInfoScreen("CNC INTERFACE", "ESP32-S3 + Marlin");
      return;
    }

    showInfoScreen("UI PREVIEW", "Belum diimplementasi");
    return;
  }

  // If currently browsing file list and ENTER pressed, show confirm dialog
  if (_uiState == UiState::FileList) {
    if (_fileList.empty() || _fileListSelected >= _fileListPaths.size()) {
      _lcd.showMessage("Files", "Tidak ada file");
      return;
    }

    _confirmSelectedIndex = 0;
    _uiState = UiState::ConfirmSelect;
    _lcd.showConfirm("Confirm", "Set as job?", _confirmSelectedIndex);
    return;
  }

  // Klik encoder memilih mode axis. Tekan-putar tidak mengubah mode.
  if (_uiState == UiState::SetOrigin) {
    if (_encoderMovedWhilePressed) {
      _encoderMovedWhilePressed = false;
      return;
    }

    _isJogAdjusting = !_isJogAdjusting;
    updateJogDisplay();
    return;
  }
}

void AppController::onEncoderTurned(int8_t direction) {
  Serial.print("Event encoder dikirim ke aplikasi: ");
  Serial.println(direction > 0 ? "Z+" : "Z-");

  if (_uiState == UiState::Info) {
    return;
  }

  if (isConfirmationState()) {
    _confirmSelectedIndex = (_confirmSelectedIndex == 0) ? 1 : 0;
    redrawCurrentConfirmation();
    return;
  }

  // Logika Set Origin untuk encoder.
  if (_uiState == UiState::SetOrigin) {
    bool swPressed = (digitalRead(PinConfig::ENCODER_SW) == LOW);

    if (!_isJogAdjusting && !swPressed) {
      // Mode Pilih Sumbu (ketika tombol tidak ditekan)
      if (direction > 0) _jogAxisIndex = (_jogAxisIndex + 1) % 3;
      else _jogAxisIndex = (_jogAxisIndex == 0) ? 2 : _jogAxisIndex - 1;
    } else {
      // Mode Gerakkan Sumbu (baik mode sticky atau mode tekan-putar)
      if (swPressed) _encoderMovedWhilePressed = true;

      float distance = swPressed
        ? AppConfig::JOG_STEP_LONG_MM
        : AppConfig::JOG_STEP_SHORT_MM;
      uint8_t targetPin = 0;
      if (_jogAxisIndex == 0) targetPin = (direction > 0) ? PinConfig::BTN1 : PinConfig::BTN2;
      else if (_jogAxisIndex == 1) targetPin = (direction > 0) ? PinConfig::BTN3 : PinConfig::BTN4;
      else if (_jogAxisIndex == 2) targetPin = (direction > 0) ? PinConfig::BTN5 : PinConfig::BTN6;

      if (targetPin != 0) handleJog(targetPin, distance);
    }
    updateJogDisplay();
    return;
  }

  // Jika sedang di menu, navigasi pilihan
  if (_uiState == UiState::Menu) {
    if (direction > 0) _menu.selectNext();
    else _menu.selectPrev();
    return;
  }

  // Jika sedang di daftar file, navigasi file list
  if (_uiState == UiState::FileList) {
    const size_t pageSize = 5;
    if (direction > 0) {
      // move down
      if (_fileListSelected + 1 < _fileList.size()) {
        _fileListSelected++;
        if (_fileListSelected >= _fileListOffset + pageSize) {
          _fileListOffset += pageSize;
        }
      }
    } else {
      // move up
      if (_fileListSelected > 0) {
        _fileListSelected--;
        if (_fileListSelected < _fileListOffset) {
          if (_fileListOffset >= pageSize) _fileListOffset -= pageSize;
          else _fileListOffset = 0;
        }
      }
    }

    // draw current page
    size_t end = _fileListOffset + pageSize;
    if (end > _fileList.size()) end = _fileList.size();
    std::vector<String> page;
    for (size_t i = _fileListOffset; i < end; ++i) page.push_back(_fileList[i]);
    _lcd.showMenu("Files", page, _fileListSelected - _fileListOffset, 0);
    return;
  }

  if (_networkStarted) {
    _cloud.publishEncoderEvent(direction);
  }
}

void AppController::onEncoderPressed() {
  Serial.println("Encoder digunakan sebagai OK / Select");

  // Publish event to cloud if available
  if (_networkStarted) {
    _cloud.publishStatus("input", "encoder_pressed");
  }

  // Perform unified select action (same as BTN7)
  processSelectAction();
}

// --------------------
// File list selection handling
// --------------------
  // When user presses ENTER while in FileList state, select the highlighted
  // file and set it as the job source. This function uses helper method
  // selectSdCardJobFile to register the selected path.
// Comments are intentionally included for thesis documentation.

// Note: _fileList contains display names while _fileListPaths has full paths.
// Selecting a file changes internal state _selectedJobSource and returns
// user feedback on the LCD.
void AppController::selectFileFromList() {
  if (_fileListSelected >= _fileListPaths.size()) {
    _lcd.showMessage("File", "Invalid selection");
    return;
  }

  const String &path = _fileListPaths[_fileListSelected];

  if (_fileListSource == JobSource::SdCard) {
    // Attempt to select SD job file; shows confirmation on success/fail
    if (selectSdCardJobFile(path.c_str())) {
      _lcd.showMessage("SD Card", "File dipilih");
      _selectedJobSource = JobSource::SdCard;
    } else {
      _lcd.showMessage("SD Card", "Pilih gagal");
    }
  }
}

// Implementasi helper untuk memilih file job dari SD card
bool AppController::selectSdCardJobFile(const char *path) {
  if (!_sdCard.isReady()) {
    _lcd.showMessage("SD Card", "Belum siap");
    return false;
  }

  bool ok = _sdCard.selectJobFile(path);
  if (ok) {
    _lcd.showMessage("SD Card", "File dipilih");
    _selectedJobSource = JobSource::SdCard;
  } else {
    _lcd.showMessage("SD Card", "Pilih gagal");
  }
  return ok;
}

void AppController::printSelectedJobFile() {
  if (_selectedJobSource == JobSource::SdCard && _sdCard.hasSelectedJobFile()) {
    _sdCard.printSelectedJobFile();
  } else {
    _lcd.showMessage("Job", "Tidak ada file");
  }
}

// --------------------
// Tahap inisialisasi
// --------------------

void AppController::beginSerial() {
  Serial.begin(AppConfig::SERIAL_BAUD_RATE);
  delay(200);
  // Initialize secondary UART (Serial1) to communicate with BTT via pins defined in AppConfig
  // Use pin definitions from PinConfig for Serial1 (UART to CNC controller)
  Serial1.begin(AppConfig::BTT_UART_BAUD, SERIAL_8N1, PinConfig::CNC_UART_RX, PinConfig::CNC_UART_TX);
  Serial.println("AppController: Serial1 initialized for BTT");
}

void AppController::beginDisplay(bool holdBootScreen) {
  Serial.println("AppController: beginDisplay()");
  _lcd.begin();
  Serial.println("AppController: LcdHandler.begin() returned");
  _lcd.showBootSplash(
    AppConfig::BOOT_TITLE,
    AppConfig::BOOT_SUBTITLE,
    AppConfig::BOOT_FOOTER,
    AppConfig::BOOT_SPLASH_MS
  );
  Serial.println("AppController: showBootSplash() called");

  if (holdBootScreen) {
    _uiState = UiState::Boot;
    return;
  }

  showStandbyScreen();
}



void AppController::beginInput() {
  activeApp = this;
  _buttons.begin();
  _buttons.setCallback(handleButtonPressed);

  _encoder.begin();
  _encoder.setTurnCallback(handleEncoderTurned);
  _encoder.setPressCallback(handleEncoderPressed);
  _inputStarted = true;
}

void AppController::beginStorage() {
  // Inisialisasi modul storage tapi jangan langsung menimpa layar boot/standby.
  // updateStorageFileDisplay() akan dijalankan oleh loop ketika UI berada
  // pada mode Standby.
  _storageStarted = _sdCard.begin();
  // Do not show SD files automatically on standby; user must enter SD Card menu
  _showSdFilesOnLcd = false;
}



void AppController::beginNetwork() {
  bool wifiReady = _wifi.begin();

  if (wifiReady) {
    // Initialize cloud MQTT client
    _networkStarted = _cloud.begin();
    // Configure NTP time so getLocalTime() can work for display. Use pool.ntp.org
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    // Try to obtain time briefly; do not block long
    struct tm timeinfo;
    unsigned long start = millis();
    while (millis() - start < 3000) {
      if (getLocalTime(&timeinfo)) {
        // time acquired
        break;
      }
      delay(200);
    }
  } else {
    _networkStarted = false;
  }
}

void AppController::runSdCardDiagnostics() {
  if (_sdCard.isReady()) {
    _sdCard.runDiagnostics();
  } else {
    _lcd.showMessage("SD Card", "Belum siap");
  }
}

void AppController::updateInput() {
  _buttons.update();
  _encoder.update();
}

void AppController::updateNetwork() {
  if (!_networkStarted) {
    return;
  }

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

void AppController::updateStorageFileDisplay() {
  // Only refresh file listing while user is actively in FileList view.
  if (_uiState != UiState::FileList) return;

  if (_lastStorageDisplay != 0 && millis() - _lastStorageDisplay < AppConfig::STORAGE_DISPLAY_INTERVAL_MS) {
    return;
  }

  _lastStorageDisplay = millis();

  if (!_sdCard.isReady()) {
    _lcd.showMessage("SD Card", "Belum siap");
    return;
  }

  // Refresh file list and redraw current page
  std::vector<String> full = _sdCard.listFileNames("/", 0, 50);
  _fileListPaths = full;
  _fileList.clear();
  for (const String &p : full) _fileList.push_back(basenameForLcd(p));

  if (_fileList.empty()) {
    _fileListSelected = 0;
    _fileListOffset = 0;
    _lcd.showMessage("Files", "Tidak ada file");
    return;
  }

  if (_fileListSelected >= _fileList.size()) {
    _fileListSelected = _fileList.size() - 1;
  }

  const size_t pageSize = 5;
  _fileListOffset = (_fileListSelected / pageSize) * pageSize;
  size_t end = _fileListOffset + pageSize;
  if (end > _fileList.size()) end = _fileList.size();
  std::vector<String> page;
  for (size_t i = _fileListOffset; i < end; ++i) page.push_back(_fileList[i]);
  _lcd.showMenu("Files", page, _fileListSelected - _fileListOffset, 0);
}
