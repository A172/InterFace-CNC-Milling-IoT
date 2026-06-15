#include "LcdHandler.h"
#include <vector>

#include "../config/PinConfig.h"
#include "../assets/Symbols.h"

namespace {
  constexpr uint8_t LCD_MAX_CHARS = 21;
  constexpr uint8_t LCD_ROW_HEIGHT = 10;

  // Boot splash layout
  constexpr uint8_t SPLASH_LOGO_W = Symbols::BootLogo::WIDTH;   // 40
  constexpr uint8_t SPLASH_LOGO_H = Symbols::BootLogo::HEIGHT;  // 40
  constexpr uint8_t SPLASH_LOGO_X = 3;
  constexpr uint8_t SPLASH_LOGO_Y = 4;
}


void LcdHandler::begin() {
  if (_lcd == nullptr) {
    // Mapping yang terdeteksi bekerja: gunakan urutan (E, RW, RS)
    _lcd = new U8G2_ST7920_128X64_F_SW_SPI(
      U8G2_R0,
      PinConfig::LCD_E,
      PinConfig::LCD_RW,
      PinConfig::LCD_RS,
      PinConfig::LCD_RST
    );
  }

  Serial.println("LcdHandler: begin()");
  // Pastikan pin-pin yang dipakai diset ke OUTPUT untuk diagnosa.
  // Pada beberapa varian ESP32, beberapa GPIO adalah input-only -> jika pin
  // tersebut dipakai maka layar akan tetap blank.
  pinMode(PinConfig::LCD_E, OUTPUT);
  pinMode(PinConfig::LCD_RS, OUTPUT);
  pinMode(PinConfig::LCD_RW, OUTPUT);
  pinMode(PinConfig::LCD_RST, OUTPUT);

  // Toggle singkat untuk memastikan pin dapat di-drive (terlihat di scope/LED)
  digitalWrite(PinConfig::LCD_RST, LOW);
  delay(10);
  digitalWrite(PinConfig::LCD_RST, HIGH);
  delay(20);

  _lcd->begin();
  Serial.println("LcdHandler: u8g2 begin() completed");

  // Coba toggle sinyal E agar bisa dideteksi alat ukur/LED; ini membantu
  // mengetahui apakah pin yang dipilih benar (jika tidak ada toggle, besar
  // kemungkinan pin input-only atau wiring salah).
  digitalWrite(PinConfig::LCD_E, LOW);
  delay(5);
  digitalWrite(PinConfig::LCD_E, HIGH);
  delay(5);
  _lcd->setFont(u8g2_font_6x10_tf);
  clear();
}

void LcdHandler::clear() {
  if (_lcd == nullptr) {
    return;
  }

  _lcd->clearBuffer();
  _lcd->sendBuffer();
}

void LcdHandler::drawText(uint8_t x, uint8_t y, const char *text, bool clearFirst) {
  if (_lcd == nullptr) {
    return;
  }

  if (clearFirst) {
    _lcd->clearBuffer();
  }
  _lcd->setFont(u8g2_font_6x10_tf);
  _lcd->drawStr(x, y, text);
}

void LcdHandler::refresh() {
  if (_lcd != nullptr) {
    _lcd->sendBuffer();
  }
}

void LcdHandler::showMessage(const char *title, const char *message) {
  if (_lcd == nullptr) {
    return;
  }

  _lcd->clearBuffer();
  _lcd->setFont(u8g2_font_6x10_tf);
  _lcd->drawStr(0, 10, fitText(title, LCD_MAX_CHARS).c_str());
  _lcd->drawHLine(0, 13, 128);
  _lcd->drawStr(0, 30, fitText(message, LCD_MAX_CHARS).c_str());
  _lcd->sendBuffer();
}

void LcdHandler::showStandbyLines(const char *top, const char *middle, const char *bottom, const char *eta, const char *jobName, int progress, const char *timeStr) {
  if (_lcd == nullptr) return;

  _lcd->clearBuffer();
  _lcd->setFont(u8g2_font_6x10_tf);

  // Baris 1: Status & Jam
  _lcd->drawStr(0, 10, fitText(top, 15).c_str());
  if (timeStr != nullptr) {
    int16_t timeX = 128 - (_lcd->getStrWidth(timeStr));
    _lcd->drawStr(timeX, 10, timeStr);
  }
  _lcd->drawHLine(0, 12, 128);

  // Baris 2: Koordinat X & Y
  _lcd->drawStr(0, 26, middle);

  // Baris 3: Koordinat Z & Progress Bar
  _lcd->drawStr(0, 40, bottom);

  if (progress >= 0) {
    // Gambar Bar Progres di sisi kanan koordinat Z
    uint8_t pct = (progress > 100) ? 100 : (progress < 0 ? 0 : progress);
    _lcd->drawFrame(65, 34, 35, 6);             // Bingkai bar (lebar 35px agar muat)
    _lcd->drawBox(67, 36, (pct * 31) / 100, 2); // Isi bar

    // Teks Persenan di pojok kanan
    char pStr[8];
    snprintf(pStr, sizeof(pStr), "%d%%", pct);
    _lcd->drawStr(104, 40, pStr);
  }

  // Baris 4: Status Aktivitas (Nama File / ETA / Zzz)
  if (jobName != nullptr && strlen(jobName) > 0) {
    // Menampilkan Nama File di Baris 4
    _lcd->drawStr(0, 58, fitText(jobName, LCD_MAX_CHARS).c_str());
  } else if (eta != nullptr && strlen(eta) > 0) {
    _lcd->drawStr(0, 58, "ETA:");
    _lcd->drawStr(30, 58, eta);
  } else {
    // Posisi Zzz tetap sesuai perbaikan manual Anda
    _lcd->setFont(u8g2_font_ncenB14_tr);
    _lcd->drawStr(50, 58, "Zzz");
    _lcd->setFont(u8g2_font_6x10_tf);
  }

  _lcd->sendBuffer();
}

void LcdHandler::showBootSplash(
  const char *title,
  const char *subtitle,
  const char *footer,
  uint16_t durationMs
) {
  if (_lcd == nullptr) {
    return;
  }

  _lcd->clearBuffer();
  _lcd->setFont(u8g2_font_6x10_tf);

  // Logo area (40x40). Placeholder dulu jika USE_CUSTOM_LOGO = false.
  if (Symbols::BootLogo::USE_CUSTOM_LOGO && Symbols::BootLogo::LOGO_BITMAP != nullptr) {
    _lcd->drawXBMP(SPLASH_LOGO_X, SPLASH_LOGO_Y, SPLASH_LOGO_W, SPLASH_LOGO_H, Symbols::BootLogo::LOGO_BITMAP);
  } else {
    // Kotak placeholder
    _lcd->drawFrame(SPLASH_LOGO_X, SPLASH_LOGO_Y, SPLASH_LOGO_W, SPLASH_LOGO_H);
    // Label teks di dalam area
    _lcd->drawStr(SPLASH_LOGO_X + 7, SPLASH_LOGO_Y + 24, "LOGO");
  }

  _lcd->drawStr(49, 16, fitText(title, 13).c_str());
  _lcd->drawStr(49, 29, fitText(subtitle, 13).c_str());
  _lcd->drawStr(49, 42, fitText(footer, 13).c_str());
  _lcd->drawHLine(0, 49, 128);
  _lcd->drawStr(31, 62, "Starting system");

  _lcd->sendBuffer();

  if (durationMs > 0) {
    delay(durationMs);
  }

  // Jangan clear otomatis di sini.
  // Biarkan caller (AppController / update loop) yang memutuskan kapan tampilan
  // berikutnya digambar, agar tidak menyebabkan layar tetap blank.
}



void LcdHandler::showList(const char *title, const std::vector<String> &items, size_t offset) {
  if (_lcd == nullptr) {
    return;
  }

  _lcd->clearBuffer();
  _lcd->setFont(u8g2_font_6x10_tf);
  _lcd->drawStr(0, 9, fitText(title, LCD_MAX_CHARS).c_str());
  _lcd->drawHLine(0, 12, 128);

  if (items.empty()) {
    _lcd->drawStr(0, 31, "Tidak ada file");
    _lcd->sendBuffer();
    return;
  }

  for (uint8_t row = 0; row < 5; row++) {
    size_t itemIndex = offset + row;
    if (itemIndex >= items.size()) {
      break;
    }

    _lcd->drawStr(0, 22 + (row * LCD_ROW_HEIGHT), fitText(items[itemIndex], LCD_MAX_CHARS).c_str());
  }

  _lcd->sendBuffer();
}

void LcdHandler::showMenu(const char *title, const std::vector<String> &items, size_t selected, size_t offset) {
  if (_lcd == nullptr) {
    return;
  }

  _lcd->clearBuffer();
  _lcd->setFont(u8g2_font_6x10_tf);
  _lcd->drawStr(0, 9, fitText(title, LCD_MAX_CHARS).c_str());
  _lcd->drawHLine(0, 12, 128);

  for (uint8_t row = 0; row < 5; row++) {
    size_t itemIndex = offset + row;
    if (itemIndex >= items.size()) {
      break;
    }

    if (itemIndex == selected) {
      // Inverse highlight: draw filled box then render text in draw color 0
      uint8_t baselineY = 22 + (row * LCD_ROW_HEIGHT);
      int boxTop = (int)baselineY - 8; // adjust so box covers text height
      if (boxTop < 0) boxTop = 0;
      _lcd->drawBox(0, boxTop, 128, LCD_ROW_HEIGHT);
      // Draw text in color 0 to appear as inverted
      _lcd->setDrawColor(0);
      _lcd->drawStr(2, baselineY, fitText(items[itemIndex], LCD_MAX_CHARS - 1).c_str());
      // Restore draw color
      _lcd->setDrawColor(1);
    } else {
      _lcd->drawStr(10, 22 + (row * LCD_ROW_HEIGHT), fitText(items[itemIndex], LCD_MAX_CHARS - 1).c_str());
    }
  }

  _lcd->sendBuffer();
}

void LcdHandler::showConfirm(const char *title, const char *message, size_t selectedIndex) {
  if (_lcd == nullptr) return;

  _lcd->clearBuffer();
  _lcd->setFont(u8g2_font_6x10_tf);
  _lcd->drawStr(0, 10, fitText(title, LCD_MAX_CHARS).c_str());
  _lcd->drawHLine(0, 13, 128);

  _lcd->drawStr(0, 30, fitText(message, LCD_MAX_CHARS).c_str());

  // Draw choices
  const char *yes = "Yes";
  const char *no = "No";
  // left choice
  if (selectedIndex == 0) {
    _lcd->drawBox(0, 45, 60, 14);
    _lcd->setDrawColor(0);
    _lcd->drawStr(8, 56, yes);
    _lcd->setDrawColor(1);
    _lcd->drawStr(70, 56, no);
  } else {
    _lcd->drawStr(8, 56, yes);
    _lcd->drawBox(60, 45, 68, 14);
    _lcd->setDrawColor(0);
    _lcd->drawStr(76, 56, no);
    _lcd->setDrawColor(1);
  }

  _lcd->sendBuffer();
}

String LcdHandler::fitText(const String &text, uint8_t maxChars) const {
  if (text.length() <= maxChars) {
    return text;
  }

  if (maxChars <= 1) {
    return text.substring(0, maxChars);
  }

  return text.substring(0, maxChars - 1) + "~";
}
