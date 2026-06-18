#ifndef APP_CONTROLLER_H
#define APP_CONTROLLER_H

#include <Arduino.h>
#include <vector>
#include <utility>
#include <WString.h>

// Source of job files
enum class JobSource {
  None,
  SdCard
};

#include "../config/AppConfig.h"
#include "../config/PinConfig.h"
#include "../display/LcdHandler.h"
#include "../input/ButtonHandler.h"
#include "../input/EncoderHandler.h"
#include "../network/CloudMqtt.h"
#include "../network/WifiPortal.h"
#include "../storage/SdCardReader.h"
#include "../display/Menu.h"

class AppController {
  public:
    AppController();

    // Siklus utama aplikasi.
    void begin();
    void update();

    // Tahap inisialisasi yang bisa dipanggil dari main.cpp.
    void beginSerial();
    void beginDisplay(bool holdBootScreen = false);
    void beginInput();
    void beginStorage();
    void beginNetwork();

  // Fungsi pengujian hardware.
  void runSdCardDiagnostics();

    // Fungsi pemilihan file job dari media penyimpanan.
  bool selectSdCardJobFile(const char *path);
    void printSelectedJobFile();
  // Internal helper to finalize selection from file list (shows messages)
  bool selectFileFromList();

    // Fungsi update yang bisa dipanggil dari main.cpp.
    void updateInput();
    void updateNetwork();
    void updateStorageFileDisplay();

    // Penangan event dari input.
    void onButtonPressed(uint8_t buttonNumber);
    void onEncoderTurned(int8_t direction);
    void onEncoderPressed();
  // Helper for unified "select/enter" action (used by encoder press and BTN7)
  void processSelectAction();

  private:
    // Modul hardware lokal.
    ButtonHandler _buttons;
    EncoderHandler _encoder;
    LcdHandler _lcd;
    Menu _menu;
    SdCardReader _sdCard;
  // USB support removed

    // UI state
    enum class UiState {
      Boot,
      Standby,
      Menu,
      Info,
      FileList,
      ConfirmSelect,
      ConfirmAction,
      SetOrigin
    };
    UiState _uiState = UiState::Boot;

    // Dummy posisi mesin (akan digantikan oleh pembacaan aktual dari MCU/driver)
    float _posX = 0.0f;
    float _posY = 0.0f;
    float _posZ = 0.0f;
    // Selected job source
    JobSource _selectedJobSource = JobSource::None;
  // File list state
  // _fileList contains display-friendly names (basenames) shown on LCD
  std::vector<String> _fileList;
  // _fileListPaths stores the full paths required for selecting a job
  std::vector<String> _fileListPaths;
  size_t _fileListSelected = 0;
  size_t _fileListOffset = 0; // starting index shown on screen
    JobSource _fileListSource = JobSource::None;
  // Confirm dialog state: 0 = Yes, 1 = No
  size_t _confirmSelectedIndex = 0;

  enum class PendingAction {
    None,
    HomeAll,
    SetOrigin
  };
  PendingAction _pendingAction = PendingAction::None;
  UiState _confirmReturnState = UiState::Menu;
  UiState _infoReturnState = UiState::Menu;
  String _confirmTitle;
  String _confirmMessage;

  struct MenuState {
    String title;
    std::vector<String> items;
    size_t selected;
  };

  // Menu history stack: stores previous menu title, items, and selected index.
  std::vector<MenuState> _menuHistory;

  // Helper to push/pop menus
  void pushMenuState(const std::vector<String> &items, size_t selected);
  bool popMenuState();
  void showStandbyScreen();
  void showMainMenuScreen(size_t selected = 0);
  void showSubMenu(const char *title, const std::vector<String> &items);
  void showCurrentFileListPage();
  void showInfoScreen(
    const char *title,
    const char *message,
    UiState returnState = UiState::Menu
  );
  void dismissInfoScreen();
  void showActionConfirm(PendingAction action, const char *title, const char *message);
  void redrawActionConfirm();
  void cancelActionConfirm();
  void executeConfirmedAction();
  bool isConfirmationState() const;
  void acceptConfirmation();
  void rejectConfirmation();
  void executeSelectedConfirmation();
  void redrawCurrentConfirmation();

    // Modul jaringan dan cloud.
    WifiPortal _wifi;
    CloudMqtt _cloud;

    // Status runtime aplikasi.
    unsigned long _lastHeartbeat = 0;
    unsigned long _lastStorageDisplay = 0;
    bool _showSdFilesOnLcd = true;
    bool _inputStarted = false;
    bool _storageStarted = false;
    bool _networkStarted = false;
  // Helper to compute ETA string for selected job (mm:ss or "--:--")
  String computeEta() const;
  // Return selected job filename or empty string if none
  String computeJobName() const;
  // Return progress percent (0-100) or -1 if no active job
  int computeProgress() const;
  // Return current local time as HH:MM or nullptr/empty if not available
  String getLocalTimeString() const;

  private:
    unsigned long _lastM114Request = 0;
    void updateMarlinCommunication();
    void parseMarlinResponse(const String &line);

  private:
    uint8_t _jogAxisIndex = 0;      // 0:X, 1:Y, 2:Z
    bool _isJogAdjusting = false;   // Toggle antara pilih axis atau gerakkan nilai
    bool _encoderMovedWhilePressed = false;

  private:
    uint8_t _activeJogPin = 0;
    uint32_t _jogStartTime = 0;
    bool _jogLongPressHandled = false;
    uint32_t _lastJogRepeatTick = 0;

    void enterSetOrigin();
    void confirmSetOrigin(UiState returnState);
    void handleJog(uint8_t pin, float distance);
    void updateJogDisplay();
};

#endif
