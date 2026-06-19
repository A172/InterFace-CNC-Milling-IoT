#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <vector>
#include "LcdHandler.h"

class Menu {
public:
  Menu(LcdHandler &lcd);

  // Tampilkan standby screen (logo sudah ditampilkan di boot splash).
  // posisiX/Y disediakan untuk fleksibilitas layout.
  void showStandby(float x = 0.0, float y = 0.0, float z = 0.0, const char *eta = nullptr, const char *jobName = nullptr, int progress = -1, const char *timeStr = nullptr, const char *machineStatus = nullptr, const char *networkStatus = nullptr, bool positionValid = false);

  // Tampilkan main menu dengan daftar pilihan.
  void showMainMenu(const std::vector<String> &items, size_t selected);
  void showMenu(const char *title, const std::vector<String> &items, size_t selected);
  void redraw();
  void selectNext();
  void selectPrev();
  size_t selectedIndex() const;
  const std::vector<String> &items() const;
  const String &title() const;

private:
  LcdHandler &_lcd;
  String _title = "Menu";
  std::vector<String> _items;
  size_t _selected = 0;
};

#endif
