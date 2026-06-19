# Changelog

## 2026-06-20

- Menetapkan nama firmware `AYB Interface` secara terpusat di `AppConfig.h`.
- Menggunakan identitas firmware pada boot screen, Serial Monitor, menu About, dan dokumentasi.
- Menambahkan toggle `CNC_ENABLE_MARLIN_CONNECTION` di `main.cpp`; default dinonaktifkan selama SKR belum tersambung.
- Mengganti status standby ambigu `M:--/OK/NO` menjadi `CNC:OFF/WAIT/DISC/OK/LOST/ERR`.
- Menambahkan state komunikasi Marlin `OFF`, `WAITING`, `DISCONNECTED`, `CONNECTED`, `LOST`, dan `ERROR`.
- Memblokir pengiriman perintah mesin sampai Marlin berstatus `CONNECTED`.
- Mengubah alarm buzzer dan MQTT agar hanya aktif untuk koneksi yang terputus setelah pernah tersambung atau respons error Marlin.

## 2026-06-19

- Menambahkan passive buzzer pada GPIO 38 menggunakan PWM LEDC.
- Menambahkan `BuzzerHandler` non-blocking dengan prioritas pola bunyi.
- Menambahkan feedback untuk Enter, Back, klik rotary, warning SD/job, dan job selesai.
- Menambahkan alarm buzzer berulang setiap 5 detik saat Marlin tidak merespons.
- Memperbaiki awal perhitungan timeout Marlin agar kondisi belum pernah merespons dapat berubah dari `WAIT` menjadi `NO RESP`.
- Menambahkan indikator status WiFi dan MQTT pada area kosong standby screen.
- Menggunakan `V` untuk tersambung, `X` untuk mati/belum tersambung, dan `?` untuk error.
- Mengubah label ringkas `W`/`M` menjadi `WiFi`/`MQTT` dan menyejajarkan `WiFi` tepat di bawah koordinat Y.
- Memperbaiki Reset WiFi agar portal WiFiManager langsung dibuka tanpa toggle OFF/ON manual.
- Memperbaiki default broker dari `192.168.250.13` ke IPv4 laptop aktif `192.168.250.3`.
- Menambahkan migrasi otomatis Preferences dari alamat broker lama.
- Memperbaiki status koneksi agar kegagalan TCP probe tetap tampil `TIMEOUT`, bukan tertimpa `DISCONNECTED`.
- Memindahkan konfigurasi broker/topic MQTT dari `AppConfig.h` ke `src/config/MqttConfig.h`.
- Menambahkan publish retained `cnc/status` dengan Last Will offline.
- Menambahkan subscribe `cnc/command` dan `cnc/gcode` menggunakan PubSubClient.
- Menampilkan payload command dan G-code di Serial Monitor tanpa mengeksekusi atau meneruskan ke UART.
- Menambahkan publish `cnc/progress` dan `cnc/error` tanpa menghapus topic monitoring lama.
- Menjaga konfigurasi MQTT yang sudah bekerja tanpa mengubah topic monitoring.
- Mengaktifkan polling `M114` periodik untuk monitoring posisi Marlin.
- Memastikan posisi X/Y/Z dari respons Marlin dipakai oleh snapshot MQTT `cnc/position`.
- Mengubah alarm MQTT agar fokus pada status respons Marlin:
  - `ERROR / Marlin no response` saat Marlin timeout.
  - `INFO / OK` saat Marlin kembali normal.
- Menambahkan dokumentasi status project sesuai aturan `AGENTS.md`.
- Memperluas README dengan dokumentasi UI LCD: boot, standby, main menu, submenu, Select Job, Set Origin, Machine Ctrl&Status, Network, dan dialog konfirmasi.
- Memperbarui PROJECT_STATUS agar mencatat workflow UI yang sudah selesai.
- Menambahkan bagian Hardware ke README.
- Memperbaiki ARCHITECTURE.md dengan diagram ASCII yang valid dan sinkron dengan hardware/software project.

## v0.4.0

- Menambahkan MQTT monitoring lokal.
- Menambahkan dukungan Mosquitto broker lokal.
- Menambahkan monitoring lewat MQTT Explorer.
- Menambahkan sinkronisasi waktu NTP WIB.

## v0.3.0

- Menambahkan SD card browser.
- Menambahkan navigasi folder.

## v0.2.0

- Menambahkan LCD menu system.
- Menambahkan rotary encoder.
- Menambahkan push button input.

## v0.1.0

- Initial project structure.
