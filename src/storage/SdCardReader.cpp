#include "SdCardReader.h"

SdCardReader::SdCardReader(uint8_t csPin, uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin)
  : _csPin(csPin), _sckPin(sckPin), _misoPin(misoPin), _mosiPin(mosiPin) {
}

bool SdCardReader::begin() {
  SPI.begin(_sckPin, _misoPin, _mosiPin, _csPin);

  if (!SD.begin(_csPin)) {
    Serial.println("SD Card gagal diinisialisasi");
    _ready = false;
    return false;
  }

  if (SD.cardType() == CARD_NONE) {
    Serial.println("SD Card tidak terdeteksi");
    _ready = false;
    return false;
  }

  _ready = true;
  Serial.println("SD Card siap digunakan");
  return true;
}

bool SdCardReader::isReady() const {
  return _ready;
}

void SdCardReader::printCardInfo() {
  if (!_ready) {
    Serial.println("SD Card belum siap");
    return;
  }

  Serial.print("SD Card Type: ");
  Serial.println(cardTypeName(SD.cardType()));
  Serial.printf("SD Card Size: %lluMB\n", SD.cardSize() / (1024 * 1024));
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void SdCardReader::listFiles(const char *dirname, uint8_t levels) {
  if (!_ready) {
    Serial.println("SD Card belum siap");
    return;
  }

  listDir(SD, dirname, levels);
}

std::vector<String> SdCardReader::listFileNames(const char *dirname, uint8_t levels, size_t maxItems) {
  std::vector<String> items;

  if (!_ready) {
    Serial.println("SD Card belum siap");
    return items;
  }

  collectFileNames(SD, dirname, levels, maxItems, items);
  return items;
}

void SdCardReader::runDiagnostics() {
  if (!_ready) {
    Serial.println("Tes SD Card dibatalkan karena kartu belum siap");
    return;
  }

  printCardInfo();
  listFiles("/", 0);

  createDir("/cnc_test");
  listFiles("/", 0);
  removeDir("/cnc_test");
  listFiles("/", 1);

  writeFile("/hello.txt", "Hello ");
  appendFile("/hello.txt", "World!\n");
  readFile("/hello.txt");

  deleteFile("/foo.txt");
  renameFile("/hello.txt", "/foo.txt");
  readFile("/foo.txt");

  testFileIO("/test.txt");
  printCardInfo();
}

bool SdCardReader::createDir(const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  bool success = SD.mkdir(path);
  Serial.println(success ? "Dir created" : "mkdir failed");
  return success;
}

bool SdCardReader::removeDir(const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  bool success = SD.rmdir(path);
  Serial.println(success ? "Dir removed" : "rmdir failed");
  return success;
}

bool SdCardReader::readFile(const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = SD.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }

  Serial.println();
  file.close();
  return true;
}

bool SdCardReader::writeFile(const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }

  bool success = file.print(message);
  Serial.println(success ? "File written" : "Write failed");
  file.close();
  return success;
}

bool SdCardReader::appendFile(const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = SD.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return false;
  }

  bool success = file.print(message);
  Serial.println(success ? "Message appended" : "Append failed");
  file.close();
  return success;
}

bool SdCardReader::renameFile(const char *from, const char *to) {
  Serial.printf("Renaming file %s to %s\n", from, to);
  bool success = SD.rename(from, to);
  Serial.println(success ? "File renamed" : "Rename failed");
  return success;
}

bool SdCardReader::deleteFile(const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  bool success = SD.remove(path);
  Serial.println(success ? "File deleted" : "Delete failed");
  return success;
}

void SdCardReader::testFileIO(const char *path) {
  File file = SD.open(path);
  static uint8_t buffer[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t elapsed = 0;

  if (file) {
    len = file.size();
    size_t fileLength = len;
    start = millis();

    while (len) {
      size_t toRead = len > sizeof(buffer) ? sizeof(buffer) : len;
      file.read(buffer, toRead);
      len -= toRead;
    }

    elapsed = millis() - start;
    Serial.printf("%zu bytes read for %lu ms\n", fileLength, elapsed);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }

  file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  start = millis();
  for (size_t i = 0; i < 2048; i++) {
    file.write(buffer, sizeof(buffer));
  }

  elapsed = millis() - start;
  Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, elapsed);
  file.close();
}

bool SdCardReader::fileExists(const char *path) {
  if (!_ready) {
    Serial.println("SD Card belum siap");
    return false;
  }

  bool exists = SD.exists(path);
  Serial.printf("File %s: %s\n", path, exists ? "tersedia" : "tidak ditemukan");
  return exists;
}

bool SdCardReader::selectJobFile(const char *path) {
  if (!fileExists(path)) {
    Serial.println("File job gagal dipilih");
    return false;
  }

  _selectedJobFile = path;
  Serial.print("File job SD dipilih: ");
  Serial.println(_selectedJobFile);
  return true;
}

bool SdCardReader::hasSelectedJobFile() const {
  return _selectedJobFile.length() > 0;
}

const String &SdCardReader::selectedJobFile() const {
  return _selectedJobFile;
}

void SdCardReader::printSelectedJobFile() const {
  if (!hasSelectedJobFile()) {
    Serial.println("Belum ada file job SD yang dipilih");
    return;
  }

  Serial.print("File job SD aktif: ");
  Serial.println(_selectedJobFile);
}

std::vector<String> SdCardReader::previewFileLines(const char *path, size_t maxLines, size_t charsPerLine) {
  std::vector<String> result;

  if (!_ready) {
    Serial.println("SD Card belum siap (preview)");
    return result;
  }

  File file = SD.open(path);
  if (!file) {
    Serial.printf("Gagal membuka file untuk preview: %s\n", path);
    return result;
  }

  String line;
  size_t linesRead = 0;

  while (file.available() && linesRead < maxLines) {
    char c = file.read();
    if (c == '\n' || line.length() >= (int)charsPerLine) {
      // push the current line (trim if necessary)
      if (line.length() > (int)charsPerLine) line = line.substring(0, charsPerLine);
      result.push_back(line);
      line = "";
      linesRead++;
      // if line ended with newline, continue; else, the next char is part of next segment
    } else if (c != '\r') {
      line += c;
    }
  }

  // push remaining partial line if any and we haven't reached maxLines
  if (linesRead < maxLines && line.length() > 0) {
    if (line.length() > (int)charsPerLine) line = line.substring(0, charsPerLine);
    result.push_back(line);
  }

  file.close();
  return result;
}

const char *SdCardReader::cardTypeName(uint8_t cardType) const {
  switch (cardType) {
    case CARD_MMC:
      return "MMC";
    case CARD_SD:
      return "SDSC";
    case CARD_SDHC:
      return "SDHC";
    default:
      return "UNKNOWN";
  }
}

void SdCardReader::listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    root.close();
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());

      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }

    file = root.openNextFile();
  }

  root.close();
}

void SdCardReader::collectFileNames(fs::FS &fs, const char *dirname, uint8_t levels, size_t maxItems, std::vector<String> &items) {
  if (items.size() >= maxItems) {
    return;
  }

  File root = fs.open(dirname);
  if (!root || !root.isDirectory()) {
    if (root) {
      root.close();
    }
    return;
  }

  File file = root.openNextFile();
  while (file && items.size() < maxItems) {
    String name = file.name();

    if (file.isDirectory()) {
      items.push_back("[D] " + name);

      if (levels > 0 && items.size() < maxItems) {
        collectFileNames(fs, file.path(), levels - 1, maxItems, items);
      }
    } else {
      items.push_back(name);
    }

    file.close();
    file = root.openNextFile();
  }

  root.close();
}
