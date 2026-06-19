#ifndef LCD_HANDLER_H
#define LCD_HANDLER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <vector>

class LcdHandler {
  public:
    void begin();
    void clear();
    void print(uint8_t x, uint8_t y, const char *text);
    void drawText(uint8_t x, uint8_t y, const char *text, bool clearFirst = false);
    void refresh();
    void showMessage(const char *title, const char *message);
    void showCenteredMessage(const char *title, const char *message);
    void showWifiSetup(const char *apName);
    void showBootSplash(
      const char *title,
      const char *subtitle,
      const char *footer,
      uint16_t durationMs = 1200
    );
    void showList(const char *title, const std::vector<String> &items, size_t offset = 0);
  // Show three-line standby layout: top, middle, bottom
  void showStandbyLines(const char *top, const char *middle, const char *bottom, const char *eta = nullptr, const char *jobName = nullptr, int progress = -1, const char *timeStr = nullptr, const char *networkStatus = nullptr);
  // Show a menu with a selected index (draws a '>' marker on selected row)
  void showMenu(const char *title, const std::vector<String> &items, size_t selected, size_t offset = 0);
  // Show a confirmation dialog with two choices (Yes/No). selectedIndex is 0 for Yes, 1 for No
  void showConfirm(const char *title, const char *message, size_t selectedIndex = 0);

  private:
    U8G2_ST7920_128X64_F_SW_SPI *_lcd = nullptr;

    String fitText(const String &text, uint8_t maxChars) const;
};

#endif
