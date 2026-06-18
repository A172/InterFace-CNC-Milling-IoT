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

  // ========== SERIAL & DEBUG ==========
  constexpr unsigned long SERIAL_BAUD_RATE = 115200;

  // ========== LCD BOOT SCREEN ==========
  constexpr const char *BOOT_TITLE = "AYB Interface";
  constexpr const char *BOOT_SUBTITLE = "Version 1.0";
  constexpr const char *BOOT_FOOTER = "Initializing";
  constexpr unsigned int BOOT_SPLASH_MS = 2500;

  // ========== STORAGE (SD Card) ==========
  constexpr unsigned long STORAGE_DISPLAY_INTERVAL_MS = 3000;  // Refresh list setiap 3 detik

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
  
  // ========== UI/UX ==========
  constexpr unsigned long BUTTON_DEBOUNCE_MS = 20;     // Debounce button
  constexpr unsigned long BUTTON_LONG_PRESS_MS = 1000; // Long press threshold
  constexpr unsigned long BUTTON_JOG_REPEAT_MS = 500;  // Repeat setiap tombol ditahan
  constexpr unsigned long UI_UPDATE_INTERVAL_MS = 100; // Refresh UI rate
}

#endif
