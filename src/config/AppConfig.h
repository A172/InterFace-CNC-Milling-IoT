#ifndef APP_CONFIG_H
#define APP_CONFIG_H

namespace AppConfig {
  // ========== IDENTITAS FIRMWARE ==========
  constexpr const char *FIRMWARE_NAME = "AYB Interface";
  constexpr const char *FIRMWARE_VERSION = "1.0";
  constexpr const char *FIRMWARE_DESCRIPTION = "ESP32-S3 CNC Interface";
  constexpr const char *FIRMWARE_AUTHOR = "Alfath Yusuf Biyono";
  constexpr const char *FIRMWARE_AUTHOR_ID = "2141170132";
  constexpr const char *FIRMWARE_LAST_UPDATED = "21/06/2026";

  // Fokus pengembangan saat ini: validasi tampilan dan navigasi.
  // Saat aktif, perintah G-code tidak dikirim ke controller CNC.
  constexpr bool UI_DEVELOPMENT_MODE = true;

  // Set Origin tetap diizinkan menggerakkan mesin saat mode UI aktif.
  // Ubah ke false jika hanya ingin menguji tampilan tanpa gerakan hardware.
  constexpr bool ENABLE_SET_ORIGIN_CONTROL = true;
  // Setelah G92 disimpan, aktifkan soft endstop agar job berjalan dalam batas mesin.
  constexpr bool ENABLE_SOFT_ENDSTOP_ON_SET_ORIGIN = true;
  constexpr bool ENABLE_HOME_CONTROL = true;
  constexpr bool ENABLE_SPINDLE_CONTROL = true;
  constexpr bool ENABLE_MACHINE_FEEDRATE_CONTROL = true;

  // ========== SERIAL & DEBUG ==========
  constexpr unsigned long SERIAL_BAUD_RATE = 115200;

  // ========== LCD BOOT SCREEN ==========
  constexpr const char *BOOT_TITLE = FIRMWARE_NAME;
  constexpr const char *BOOT_FOOTER = "Initializing";
  constexpr unsigned int BOOT_SPLASH_MS = 2500;

  // ========== NETWORK (WiFi) ==========
  // Nilai awal saat flash masih kosong. Setelah itu user bisa ubah dari menu Network.
  constexpr bool DEFAULT_WIFI_ENABLED = false;
  // WiFi Configuration
  constexpr const char *WIFI_PORTAL_AP_NAME = "CNC-Interface-Setup";
  constexpr const char *WIFI_SSID = "";          // Isi sesuai jaringan Anda
  constexpr const char *WIFI_PASSWORD = "";     // Isi sesuai password WiFi
  constexpr unsigned long WIFI_TIMEOUT_MS = 20000;   // Timeout koneksi WiFi

  // ========== TIME (NTP / RTC internal ESP32) ==========
  constexpr long WIB_GMT_OFFSET_SEC = 7L * 60L * 60L;
  constexpr int WIB_DAYLIGHT_OFFSET_SEC = 0;
  constexpr const char *NTP_SERVER_1 = "pool.ntp.org";
  constexpr const char *NTP_SERVER_2 = "time.google.com";
  constexpr unsigned long NTP_SYNC_TIMEOUT_MS = 5000;

  // ========== UART COMMUNICATION (ke SKR V1.4) ==========
  // RX pin: 43, TX pin: 44
  constexpr unsigned long BTT_UART_BAUD = 115200;
  constexpr unsigned long BTT_UART_TIMEOUT_MS = 1000;
  // Query status Marlin bersifat non-gerak; dipakai untuk indikator koneksi.
  constexpr bool ENABLE_MARLIN_STATUS_MONITOR = true;
  constexpr unsigned long MARLIN_STATUS_POLL_MS = 1000;
  constexpr unsigned long MARLIN_RESPONSE_TIMEOUT_MS = 2500;
  constexpr unsigned long MARLIN_STATUS_SCREEN_REFRESH_MS = 1000;
  
  // ========== JOG CONTROL ==========
  // Kecepatan jog dalam mm/min (atau sesuai unit mesin Anda)
  constexpr unsigned int JOG_SPEED_FAST = 100;   // Kecepatan cepat
  constexpr unsigned int JOG_SPEED_NORMAL = 50;  // Kecepatan normal
  constexpr unsigned int JOG_SPEED_SLOW = 10;    // Kecepatan lambat
  constexpr float JOG_STEP_SHORT_MM = 1.0f;
  constexpr float JOG_STEP_LONG_MM = 10.0f;

  // ========== MACHINE CONTROL & STATUS ==========
  constexpr unsigned int MACHINE_FEEDRATE_STEP_MM_S = 10;
  constexpr unsigned int MACHINE_FEEDRATE_FAST_STEP_MM_S = 100;
  constexpr unsigned int MACHINE_FEEDRATE_MIN_MM_S = 1;
  constexpr unsigned int MACHINE_FEEDRATE_MAX_MM_S = 9999;
  constexpr unsigned int MACHINE_WORK_AREA_STEP_MM = 10;
  constexpr unsigned int MACHINE_WORK_AREA_FAST_STEP_MM = 100;
  constexpr unsigned int MACHINE_WORK_AREA_MIN_MM = 1;
  constexpr unsigned int MACHINE_WORK_AREA_MAX_MM = 9999;
  constexpr unsigned long MACHINE_STATUS_POLL_MS = 1000;

  // ========== JOB CONTROL ==========
  // Tetap membutuhkan CNC_ENABLE_MARLIN_CONNECTION dan preflight yang valid.
  constexpr bool ENABLE_JOB_CONTROL = true;
  // Safety move saat mengulang job: Z naik dulu, baru X/Y kembali ke origin kerja.
  constexpr bool ENABLE_JOB_REPEAT_RETURN = true;
  constexpr float JOB_REPEAT_SAFE_Z_MM = 10.0f;
  constexpr unsigned int JOB_TRAVEL_FEED_MM_MIN = 600;
  constexpr uint8_t JOB_ANALYSIS_LINES_PER_LOOP = 12;
  constexpr size_t JOB_MAX_LINE_LENGTH = 192;
  constexpr unsigned long JOB_START_DRAIN_MS = 500;
  constexpr unsigned long JOB_COMMAND_TIMEOUT_MS = 120000;
  constexpr unsigned long JOB_UI_REFRESH_MS = 500;
  constexpr unsigned long JOB_START_HOLD_MS = 1000;
  constexpr unsigned long JOB_STOP_HOLD_MS = 1500;
  
  // ========== UI/UX ==========
  constexpr unsigned long BUTTON_DEBOUNCE_MS = 20;     // Debounce button
  constexpr unsigned long BUTTON_LONG_PRESS_MS = 1000; // Long press threshold
  constexpr unsigned long BUTTON_JOG_REPEAT_MS = 500;  // Repeat setiap tombol ditahan
  constexpr unsigned long UI_UPDATE_INTERVAL_MS = 100; // Refresh UI rate
  constexpr unsigned long CONFIRM_RESULT_MESSAGE_MS = 1500; // Lama pesan hasil konfirmasi tampil
  constexpr unsigned long ABOUT_PAGE_INTERVAL_MS = 3000;
  constexpr uint8_t ABOUT_PAGE_COUNT = 3;

  // ========== PASSIVE BUZZER ==========
  constexpr bool ENABLE_BUZZER_KEY_FEEDBACK = true;
  constexpr bool ENABLE_BUZZER_NOTIFICATION = true;
  constexpr bool ENABLE_BUZZER_MARLIN_ALARM = true;
  constexpr uint8_t BUZZER_PWM_CHANNEL = 0;
  constexpr unsigned long BUZZER_ALARM_REPEAT_MS = 5000;
}

#endif
