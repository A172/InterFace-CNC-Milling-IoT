#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <Arduino.h>

namespace Symbols {
  namespace BootLogo {
    // Area logo booting pada LCD: 40 x 40 piksel.
    // Untuk memakai logo sendiri:
    // 1. Konversi gambar hitam-putih ke format XBM/U8g2.
    // 2. Ganti isi LOGO_BITMAP di Symbols.cpp.
    // 3. Ubah USE_CUSTOM_LOGO menjadi true.
    constexpr bool USE_CUSTOM_LOGO = true;
    constexpr uint8_t WIDTH = 40;
    constexpr uint8_t HEIGHT = 40;

    extern const uint8_t LOGO_BITMAP[] PROGMEM;
  }
}

#endif
