#ifndef SD_CARD_READER_H
#define SD_CARD_READER_H

#include <Arduino.h>
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <vector>

class SdCardReader {
  public:
    // Membuat objek pembaca SD card dengan pin SPI yang ditentukan dari PinConfig.
    SdCardReader(uint8_t csPin, uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin);

    // Menginisialisasi bus SPI dan mount SD card.
    bool begin();

    // Mengecek apakah SD card berhasil diinisialisasi dan siap digunakan.
    bool isReady() const;

    // Menampilkan tipe, ukuran, total ruang, dan ruang terpakai SD card ke Serial Monitor.
    void printCardInfo();

    // Menampilkan daftar file/folder pada direktori tertentu.
    void listFiles(const char *dirname = "/", uint8_t levels = 0);

    // Mengambil daftar file/folder agar bisa ditampilkan di LCD/menu.
    std::vector<String> listFileNames(const char *dirname = "/", uint8_t levels = 0, size_t maxItems = 20);

    // Menjalankan rangkaian tes baca/tulis/hapus file untuk validasi SD card.
    void runDiagnostics();

    // Membuat folder baru pada SD card.
    bool createDir(const char *path);

    // Menghapus folder dari SD card.
    bool removeDir(const char *path);

    // Membaca isi file dan menampilkannya ke Serial Monitor.
    bool readFile(const char *path);

    // Menulis file baru atau menimpa file yang sudah ada.
    bool writeFile(const char *path, const char *message);

    // Menambahkan teks ke akhir file.
    bool appendFile(const char *path, const char *message);

    // Mengubah nama atau lokasi file.
    bool renameFile(const char *from, const char *to);

    // Menghapus file dari SD card.
    bool deleteFile(const char *path);

    // Menguji performa baca/tulis file pada SD card.
    void testFileIO(const char *path);

    // Mengecek apakah file tersedia pada SD card.
    bool fileExists(const char *path);

    // Memilih file G-code/job yang akan dipakai oleh sistem CNC.
    bool selectJobFile(const char *path);

    // Mengecek apakah sudah ada file job yang dipilih.
    bool hasSelectedJobFile() const;

    // Mengambil path file job yang sedang dipilih.
    const String &selectedJobFile() const;

    // Menampilkan file job yang sedang dipilih ke Serial Monitor.
    void printSelectedJobFile() const;

  // Ambil preview (beberapa baris pertama) dari sebuah file untuk ditampilkan di UI
  // Mengembalikan vector<String> berisi baris (atau potongan) file, maksimal maxLines
  std::vector<String> previewFileLines(const char *path, size_t maxLines = 5, size_t charsPerLine = 20);

  private:
    uint8_t _csPin;
    uint8_t _sckPin;
    uint8_t _misoPin;
    uint8_t _mosiPin;
    bool _ready = false;
    String _selectedJobFile;

    // Mengubah kode tipe kartu dari library SD menjadi teks.
    const char *cardTypeName(uint8_t cardType) const;

    // Fungsi internal rekursif untuk menampilkan isi direktori.
    void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

    // Fungsi internal rekursif untuk mengambil nama file/folder.
    void collectFileNames(fs::FS &fs, const char *dirname, uint8_t levels, size_t maxItems, std::vector<String> &items);
};

#endif
