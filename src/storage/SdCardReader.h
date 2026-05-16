#ifndef SD_CARD_READER_H
#define SD_CARD_READER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SDcardReaderHandler {
  public:
    SDcardReaderHandler(uint8_t csPin);

    bool begin();
    void listFiles();
    void readFile(const char *filename);

  private:
    uint8_t _csPin;
};

#endif
