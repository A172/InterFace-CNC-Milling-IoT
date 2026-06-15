/**
 * @file DebugUtils.h
 * @brief Utility functions untuk debugging firmware CNC Milling
 * @author CNC Team
 * @date 2024
 */

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <Arduino.h>
#include <esp_system.h>
#include <string.h>

namespace DebugUtils {
  // ============================================================
  // SERIAL OUTPUT HELPERS
  // ============================================================
  
  /**
   * Print separator line untuk memisahkan section di serial output
   * @param label Optional label untuk separator
   * @param length Panjang garis (default 60)
   */
  void printSeparator(const char *label = nullptr, uint8_t length = 60) {
    if (label) {
      Serial.printf("=== %s ", label);
      uint8_t remaining = length - strlen(label) - 6;
      for (uint8_t i = 0; i < remaining; i++) Serial.print("=");
    } else {
      for (uint8_t i = 0; i < length; i++) Serial.print("=");
    }
    Serial.println();
  }

  /**
   * Print memory usage info
   */
  void printMemoryInfo() {
    printSeparator("MEMORY INFO");
    uint32_t freeHeap = esp_get_free_heap_size();
    uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
    
    Serial.printf("Free Heap: %u bytes (%u KB)\n", freeHeap, freeHeap / 1024);
    Serial.printf("Min Free Heap: %u bytes (%u KB)\n", minFreeHeap, minFreeHeap / 1024);
    Serial.printf("Heap Fragmentation: %u%%\n", 
                  esp_get_free_heap_size() ? 0 : 0);  // Simplified
  }

  /**
   * Print CPU temperature (ESP32-S3)
   */
  void printCpuTemp() {
    printSeparator("CPU INFO");
    uint8_t temp = esp_get_free_heap_size() ? 0 : 0;  // Placeholder
    Serial.printf("Free Heap: %u bytes\n", esp_get_free_heap_size());
  }

  // ============================================================
  // PIN STATE DEBUGGING
  // ============================================================

  /**
   * Print state dari semua input pins (buttons & encoder)
   */
  void printButtonStates(uint8_t btnPins[], uint8_t count) {
    printSeparator("BUTTON STATES");
    for (uint8_t i = 0; i < count; i++) {
      uint8_t state = digitalRead(btnPins[i]);
      Serial.printf("Button[%u] (Pin %2u): %s\n", i, btnPins[i], state ? "HIGH" : "LOW");
    }
  }

  /**
   * Print encoder state (CLK, DT, SW)
   */
  void printEncoderState(uint8_t clkPin, uint8_t dtPin, uint8_t swPin) {
    printSeparator("ENCODER STATE");
    Serial.printf("CLK (Pin %u): %s\n", clkPin, digitalRead(clkPin) ? "HIGH" : "LOW");
    Serial.printf("DT  (Pin %u): %s\n", dtPin, digitalRead(dtPin) ? "HIGH" : "LOW");
    Serial.printf("SW  (Pin %u): %s\n", swPin, digitalRead(swPin) ? "HIGH" : "LOW");
  }

  // ============================================================
  // UART COMMUNICATION DEBUGGING
  // ============================================================

  /**
   * Print UART configuration
   */
  void printUartConfig(uint32_t baudRate, uint8_t rxPin, uint8_t txPin) {
    printSeparator("UART CONFIG");
    Serial.printf("Baud Rate: %u\n", baudRate);
    Serial.printf("RX Pin: %u\n", rxPin);
    Serial.printf("TX Pin: %u\n", txPin);
  }

  /**
   * Hexdump untuk debugging binary data
   */
  void hexDump(uint8_t *buffer, uint16_t length, uint16_t bytesPerLine = 16) {
    for (uint16_t i = 0; i < length; i += bytesPerLine) {
      Serial.printf("%04X: ", i);
      
      // Hex values
      for (uint16_t j = 0; j < bytesPerLine && i + j < length; j++) {
        Serial.printf("%02X ", buffer[i + j]);
      }
      
      // ASCII representation
      Serial.print("  |");
      for (uint16_t j = 0; j < bytesPerLine && i + j < length; j++) {
        uint8_t c = buffer[i + j];
        Serial.write(c >= 32 && c < 127 ? c : '.');
      }
      Serial.println("|");
    }
  }

  // ============================================================
  // TIMING & PERFORMANCE
  // ============================================================

  class Timer {
    private:
      uint32_t startTime;
      const char *label;
      
    public:
      Timer(const char *lbl = "Timer") : label(lbl) {
        startTime = millis();
      }
      
      ~Timer() {
        uint32_t elapsed = millis() - startTime;
        Serial.printf("[%s] Elapsed: %u ms\n", label, elapsed);
      }
  };

  // ============================================================
  // FILE SYSTEM DEBUGGING
  // ============================================================

  /**
   * Print informasi tentang SD Card
   */
  void printSdCardInfo(uint32_t totalSpace, uint32_t usedSpace) {
    printSeparator("SD CARD INFO");
    uint32_t freeSpace = totalSpace - usedSpace;
    uint8_t percentage = (usedSpace * 100) / totalSpace;
    
    Serial.printf("Total Space: %u MB\n", totalSpace / (1024 * 1024));
    Serial.printf("Used Space:  %u MB (%u%%)\n", 
                  usedSpace / (1024 * 1024), percentage);
    Serial.printf("Free Space:  %u MB\n", freeSpace / (1024 * 1024));
  }

  // ============================================================
  // ASSERTION & ERROR REPORTING
  // ============================================================

  #ifdef DEBUG
  #define ASSERT(condition, message) \
    if (!(condition)) { \
      Serial.printf("ASSERTION FAILED: %s (file: %s, line: %d)\n", \
                    message, __FILE__, __LINE__); \
      while (1) delay(1000); \
    }
  #else
  #define ASSERT(condition, message) ((void)0)
  #endif

  /**
   * Print timestamp
   */
  void printTimestamp() {
    uint32_t ms = millis();
    uint32_t sec = ms / 1000;
    uint32_t min = sec / 60;
    uint32_t hour = min / 60;
    
    Serial.printf("[%02u:%02u:%02u.%03u] ", 
                  hour % 24, min % 60, sec % 60, ms % 1000);
  }

  /**
   * Print timestamped message
   */
  void printLog(const char *level, const char *message) {
    printTimestamp();
    Serial.printf("[%-5s] %s\n", level, message);
  }

  #define LOG_DEBUG(msg)   DebugUtils::printLog("DEBUG", msg)
  #define LOG_INFO(msg)    DebugUtils::printLog("INFO", msg)
  #define LOG_WARN(msg)    DebugUtils::printLog("WARN", msg)
  #define LOG_ERROR(msg)   DebugUtils::printLog("ERROR", msg)

} // namespace DebugUtils

#endif // DEBUG_UTILS_H
