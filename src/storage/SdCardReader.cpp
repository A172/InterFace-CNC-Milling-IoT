#include "SdCardReader.h"

SDcardReaderHandler::SDcardReaderHandler(uint8_t csPin) {
  _csPin = csPin;
}

bool SDcardReaderHandler::begin() {
  if (!SD.begin(_csPin)) {
    Serial.println("SD Card gagal diinisialisasi");
    return false;
  }

  Serial.println("SD Card siap digunakan");
  return true;
}

void SDcardReaderHandler::listFiles() {

  File root = SD.open("/");

  if (!root) {
    Serial.println("Gagal membuka root directory");
    return;
  }

  File file = root.openNextFile();

  while (file) {

    Serial.print("File: ");
    Serial.print(file.name());
    Serial.print("  Size: ");
    Serial.println(file.size());

    file = root.openNextFile();
  }

  root.close();
}

void SDcardReaderHandler::readFile(const char *filename) {

  File file = SD.open(filename);

  if (!file) {
    Serial.println("File tidak ditemukan");
    return;
  }

  Serial.print("Membaca file: ");
  Serial.println(filename);

  while (file.available()) {
    Serial.write(file.read());
  }

  Serial.println();
  file.close();
}
