#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

namespace PinConfig {
  // ========== LCD 128x64 ST7920 (SPI Mode) ==========
  constexpr uint8_t LCD_RS = 39;   // Reset pin
  constexpr uint8_t LCD_RW = 40;   // Read/Write (RW)
  constexpr uint8_t LCD_E = 41;    // Enable (E)
  constexpr uint8_t LCD_RST = 42;  // Reset (RST)
  // CATATAN: SC1/PSB harus terhubung ke GND untuk mode SPI
  // Power: BLA/VCC ke 5V, BLK/VSS ke GND

  // ========== SD Card (SPI Mode) ==========
  constexpr uint8_t SD_SCK = 12;   // Clock
  constexpr uint8_t SD_MISO = 14;  // Master In, Slave Out
  constexpr uint8_t SD_MOSI = 13;  // Master Out, Slave In
  constexpr uint8_t SD_CS = 11;    // Chip Select

  // ========== UART ke Motherboard CNC (SKR V1.4 Turbo) ==========
  constexpr uint8_t CNC_UART_RX = 43;  // Receive dari SKR
  constexpr uint8_t CNC_UART_TX = 44;  // Transmit ke SKR
  // CATATAN: Perhatikan tegangan (logic level converter jika diperlukan)

  // ========== USB (OTG Mode - opsional) ==========
  constexpr uint8_t USB_D_PLUS = 20;   // D+ (USB)
  constexpr uint8_t USB_D_MINUS = 19;  // D- (USB)
  // CATATAN: Power USB 5V masuk ke VIN ESP32-S3

  // ========== Push Button (Jog & Navigasi) ==========
  // Catatan: Semua tombol aktif LOW (terpencet = LOW, lepas = HIGH)
  constexpr uint8_t BTN_X_PLUS = 16;   // Jog X+
  constexpr uint8_t BTN_X_MINUS = 17;  // Jog X-
  constexpr uint8_t BTN_Y_PLUS = 8;    // Jog Y+
  constexpr uint8_t BTN_Y_MINUS = 5;   // Jog Y-
  constexpr uint8_t BTN_Z_PLUS = 18;   // Jog Z+
  constexpr uint8_t BTN_Z_MINUS = 6;   // Jog Z-
  constexpr uint8_t BTN_ENTER = 15;    // Confirm/Select
  constexpr uint8_t BTN_BACK = 7;      // Cancel/Back

  // Alias berurutan agar handler tombol tetap sederhana.
  constexpr uint8_t BTN1 = BTN_X_PLUS;
  constexpr uint8_t BTN2 = BTN_X_MINUS;
  constexpr uint8_t BTN3 = BTN_Y_PLUS;
  constexpr uint8_t BTN4 = BTN_Y_MINUS;
  constexpr uint8_t BTN5 = BTN_Z_PLUS;
  constexpr uint8_t BTN6 = BTN_Z_MINUS;
  constexpr uint8_t BTN7 = BTN_ENTER;
  constexpr uint8_t BTN8 = BTN_BACK;

  // ========== Rotary Encoder (Menu Navigation) ==========
  constexpr uint8_t ENCODER_CLK = 9;   // Clock (A)
  constexpr uint8_t ENCODER_DT = 3;    // Data (B)
  constexpr uint8_t ENCODER_SW = 10;   // Switch (tekan)
  // CATATAN: COM encoder terhubung ke GND
}

#endif
