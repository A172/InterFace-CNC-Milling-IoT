#include <Arduino.h>
#include "app/AppController.h"

// ============================================================
// KONFIGURASI FITUR FIRMWARE
// ============================================================
// Cara pakai:
// - Hilangkan tanda // untuk mengaktifkan fitur.
// - Tambahkan tanda // untuk menonaktifkan fitur.
//
// Contoh:
// #define CNC_ENABLE_NETWORK    -> WiFi/IoT aktif
// // #define CNC_ENABLE_NETWORK -> WiFi/IoT nonaktif

// Aktifkan pembacaan push button dan rotary encoder.
#define CNC_ENABLE_INPUT

// Aktifkan modul SD card.
#define CNC_ENABLE_SD_CARD

// Aktifkan hanya saat SD card memang sedang diuji. Tes ini menulis file.
// #define CNC_RUN_SD_CARD_DIAGNOSTICS

// Tampilkan boot screen saja untuk pengujian LCD.
// #define CNC_BOOT_SCREEN_ONLY



// Aktifkan koneksi WiFi dan MQTT IoT.
// #define CNC_ENABLE_NETWORK


AppController app;

void setup() {
  app.beginSerial();

#if defined(CNC_BOOT_SCREEN_ONLY)
  app.beginDisplay(true);
  Serial.println("Boot screen only mode aktif");
  return;
#else
  app.beginDisplay();
#endif

#if defined(CNC_ENABLE_INPUT)
  app.beginInput();
#else
  Serial.println("Input tombol dan encoder nonatifkan");
#endif

#if defined(CNC_ENABLE_SD_CARD)
  app.beginStorage();

  #if defined(CNC_RUN_SD_CARD_DIAGNOSTICS)
    app.runSdCardDiagnostics();
  #endif

  // Contoh memilih file G-code dari SD card.
  // Uncomment jika ingin menguji pemilihan file job.
  // app.selectSdCardJobFile("/contoh.nc");
  // app.printSelectedJobFile();

#else
  Serial.println("Storage SD nonaktifkan");
#endif


#if defined(CNC_ENABLE_NETWORK)
  app.beginNetwork();
#else
  Serial.println("Network nonaktifkan");
#endif
}

void loop() {
  app.update();
}
