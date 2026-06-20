# Changelog

## 2026-06-20

- Mengganti scrolling piksel About menjadi tiga halaman yang berganti setiap 3 detik agar mudah dibaca.
- Mengubah indikator standby menjadi label lengkap `WiFi:[OK]/WiFi:[X]` dan `MQTT:[OK]/MQTT:[X]`.
- Menempatkan indikator WiFi/MQTT pada satu baris khusus dan mempertahankan font koordinat `6x10`.
- Mengurutkan Machine Ctrl&Status menjadi CNC, Home X/Y/Z, SoftEnd, Spindle, Feed XY/Z, Area X/Y/Z, lalu Refresh Status.
- Menyesuaikan indeks handler agar navigasi dan aksi setiap item tetap benar setelah perubahan urutan.
- Melengkapi README dengan fungsi seluruh menu/submenu yang sudah tersedia.
- Menjelaskan bahwa Refresh Status membaca ulang `M203`, `M115`, `M119`, dan `M211` tanpa menggerakkan mesin.
- Mendokumentasikan bahwa edit Area X/Y/Z masih lokal dan belum diterapkan ke Marlin.
- Menambahkan topic retained `cnc/machine` untuk state CNC, spindle, soft endstop, feedrate, dan sensor home.
- Menandai detail mesin sebagai `?` atau `null` saat CNC belum `CONNECTED` agar status lama tidak dianggap data nyata.
- Menambahkan validitas dan sumber data pada `cnc/position`.
- Menampilkan posisi tidak tersedia sebagai nilai `null` dengan display `?`, bukan angka nol palsu.
- Menyiapkan simulasi posisi MQTT dengan default nonaktif.
- Mengganti menu `Settings` menjadi `About` pada urutan terakhir workflow menu utama.
- Menambahkan layar About dengan logo, nama firmware, versi, Alfath Yusuf Biyono, NIM 2141170132, dan tanggal update.
- Menambahkan scrolling About non-blocking serta keluar melalui Enter, Back, atau klik rotary.
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
