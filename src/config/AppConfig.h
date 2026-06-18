#ifndef APP_CONFIG_H
#define APP_CONFIG_H

namespace AppConfig {
  // Fokus pengembangan saat ini: validasi tampilan dan navigasi.
  // Saat aktif, perintah G-code tidak dikirim ke controller CNC.
  constexpr bool UI_DEVELOPMENT_MODE = true;

  // Set Origin tetap diizinkan menggerakkan mesin saat mode UI aktif.
  // Ubah ke false jika hanya ingin menguji tampilan tanpa gerakan hardware.
  constexpr bool ENABLE_SET_ORIGIN_CONTROL = true;
  constexpr bool ENABLE_HOME_CONTROL = true;
  constexpr bool ENABLE_SPINDLE_CONTROL = true;
  constexpr bool ENABLE_MACHINE_FEEDRATE_CONTROL = true;

  // ========== SERIAL & DEBUG ==========
  constexpr unsigned long SERIAL_BAUD_RATE = 115200;

  // ========== LCD BOOT SCREEN ==========
  constexpr const char *BOOT_TITLE = "AYB Interface";
  constexpr const char *BOOT_SUBTITLE = "Version 1.0";
  constexpr const char *BOOT_FOOTER = "Initializing";
  constexpr unsigned int BOOT_SPLASH_MS = 2500;

  // ========== NETWORK (WiFi & MQTT) ==========
  constexpr bool ENABLE_NETWORK = false;  // Set true untuk aktifkan WiFi/MQTT
  // WiFi Configuration
  constexpr const char *WIFI_SSID = "";          // Isi sesuai jaringan Anda
  constexpr const char *WIFI_PASSWORD = "";     // Isi sesuai password WiFi
  constexpr unsigned long WIFI_TIMEOUT_MS = 20000;   // Timeout koneksi WiFi
  
  // MQTT Configuration
  constexpr const char *MQTT_BROKER = "broker.mqtt-dashboard.com";
  constexpr unsigned int MQTT_PORT = 1883;
  constexpr const char *MQTT_CLIENT_ID = "cnc-milling-esp32";
  constexpr const char *MQTT_USER = "";
  constexpr const char *MQTT_PASS = "";
  constexpr unsigned long MQTT_RECONNECT_INTERVAL_MS = 5000;

  // ========== UART COMMUNICATION (ke SKR V1.4) ==========
  // RX pin: 43, TX pin: 44
  constexpr unsigned long BTT_UART_BAUD = 115200;
  constexpr unsigned long BTT_UART_TIMEOUT_MS = 1000;
  
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
  // Safety move saat mengulang job: Z naik dulu, baru X/Y kembali ke origin kerja.
  constexpr bool ENABLE_JOB_REPEAT_RETURN = true;
  constexpr float JOB_REPEAT_SAFE_Z_MM = 10.0f;
  constexpr unsigned int JOB_TRAVEL_FEED_MM_MIN = 600;
  
  // ========== UI/UX ==========
  constexpr unsigned long BUTTON_DEBOUNCE_MS = 20;     // Debounce button
  constexpr unsigned long BUTTON_LONG_PRESS_MS = 1000; // Long press threshold
  constexpr unsigned long BUTTON_JOG_REPEAT_MS = 500;  // Repeat setiap tombol ditahan
  constexpr unsigned long UI_UPDATE_INTERVAL_MS = 100; // Refresh UI rate
  constexpr unsigned long CONFIRM_RESULT_MESSAGE_MS = 1500; // Lama pesan hasil konfirmasi tampil
}

#endif
