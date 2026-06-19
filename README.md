# AYB Interface

**AYB Interface** adalah firmware antarmuka CNC milling berbasis ESP32-S3 untuk menghubungkan user, LCD 128x64, SD card, jaringan lokal, MQTT, dan controller CNC berbasis Marlin pada SKR V1.4 Turbo.

Firmware ini dikembangkan secara mandiri sebagai bagian dari proyek akhir D4 Teknik Elektronika Politeknik Negeri Malang.

- Pengembang: Alfath Yusuf Biyono
- NIM: 2141170132
- Pembaruan firmware terakhir: 20/06/2026

Identitas firmware disimpan terpusat di `src/config/AppConfig.h` melalui `FIRMWARE_NAME`, `FIRMWARE_VERSION`, dan `FIRMWARE_DESCRIPTION`. Identitas tersebut digunakan pada boot screen, Serial Monitor, dan menu About.

## Ringkasan Sistem

Alur kerja utama saat ini:

```text
Fusion 360 / CAM
      |
      v
File G-code di SD Card
      |
      v
ESP32-S3 CNC Interface
      |
      +-- LCD 128x64 + tombol + rotary encoder
      |
      +-- MQTT lokal ke Mosquitto / MQTT Explorer
      |
      +-- UART ke SKR V1.4 Turbo / Marlin
```

## Hardware

### Controller Interface

- ESP32-S3 DevKitM-1

### CNC Controller

- BTT SKR V1.4 Turbo
- Marlin Firmware

### Motion System

- DM542 Stepper Driver
- NEMA23 Stepper Motor

### User Interface

- LCD 128x64
- Rotary Encoder
- Push Button
- Passive buzzer pada GPIO 38

### Storage

- MicroSD Card

### Network

- WiFi
- MQTT Broker

## Fitur yang Sudah Ada

- Tampilan booting, standby, main menu, submenu, dan dialog konfirmasi.
- Navigasi menggunakan push button dan rotary encoder.
- Browser SD card untuk memilih file job.
- Tampilan job terpilih dan estimasi waktu di standby.
- Menu Set Origin dengan jog X/Y/Z dan konfirmasi `G92 X0 Y0 Z0`.
- Soft endstop Marlin dapat diaktifkan setelah Set Origin dengan `M211 S1`.
- Menu Machine Ctrl&Status untuk spindle, feedrate, area kerja, soft endstop, dan sensor homing.
- Menu Network untuk WiFi, MQTT, broker, AP setup, waktu, tanggal, dan reset WiFi.
- WiFiManager untuk konfigurasi WiFi dan parameter MQTT lokal.
- Sinkronisasi waktu NTP WIB dan penggunaan waktu internal ESP32 setelah sinkron.
- MQTT monitoring lokal ke Mosquitto.
- Monitoring state mesin melalui topic retained `cnc/machine`.
- Layar About bergulir dengan logo dan identitas pengembang.
- Feedback buzzer non-blocking untuk navigasi, job selesai, warning, dan alarm Marlin.

### Passive Buzzer

Passive buzzer menggunakan GPIO 38 dan PWM LEDC. Fungsinya:

- Enter dan klik rotary menghasilkan beep pendek bernada tinggi.
- Back menghasilkan beep pendek bernada lebih rendah.
- Job selesai menghasilkan pola tiga nada.
- SD card atau job yang tidak siap menghasilkan pola warning.
- Koneksi Marlin yang sebelumnya aktif lalu terputus, atau Marlin yang mengirim error, menghasilkan pola alarm berulang setiap 5 detik.
- Putaran rotary dan tombol jog tidak menghasilkan bunyi agar navigasi dan jog tetap responsif.

Pola bunyi dijalankan tanpa `delay()`, sehingga tidak menghentikan pembacaan tombol, rotary, UART Marlin, maupun MQTT. Gunakan transistor driver jika arus buzzer melebihi kemampuan aman GPIO ESP32-S3.

## User Interface LCD

UI utama menggunakan LCD 128x64 berbasis U8g2. Alur tampilan dibuat bertahap agar user dapat mengoperasikan mesin tanpa PC setelah file G-code tersedia di SD card.

### Boot Screen

Saat sistem menyala, LCD menampilkan boot screen berisi identitas interface dan area logo. Tampilan ini dipakai sebagai indikator awal bahwa ESP32, LCD, dan firmware sudah mulai berjalan.

### Standby Screen

Mode standby adalah tampilan utama setelah boot. Informasi yang ditampilkan:

- Status mesin.
- Waktu lokal dari sinkronisasi NTP/internal ESP32.
- Koordinat X, Y, dan Z.
- Nama job yang sedang dipilih dari SD card.
- Estimasi waktu kerja jika tersedia.
- Status komunikasi Marlin secara ringkas.
- Indikator lengkap `WiFi` dan `MQTT` di area bawah koordinat Y.

Format indikator koneksi:

- `WiFi:V` / `MQTT:V` berarti tersambung.
- `WiFi:X` / `MQTT:X` berarti mati atau belum tersambung.
- `WiFi:?` / `MQTT:?` berarti terjadi error koneksi.

Teks `WiFi` dimulai sejajar di bawah label koordinat `Y`. Huruf `V` dipakai sebagai simbol centang yang kompatibel dengan font LCD. Saat progress job tampil, area indikator dipakai oleh progress bar.

Nilai koordinat X/Y/Z hanya dianggap valid setelah respons `M114` diterima saat CNC berstatus `CONNECTED`. Jika mesin belum tersambung, standby menampilkan `?` untuk setiap koordinat.

### Main Menu

Menu utama berisi:

- `Move & Jog`
- `Select Job`
- `Machine Ctrl&Status`
- `Network`
- `About`

Navigasi menu menggunakan rotary encoder. Klik rotary atau tombol Enter menjalankan item yang dipilih.

### Submenu Move & Jog

Submenu ini mengikuti urutan persiapan bidang kerja:

1. `Home All (G28)` untuk mencari referensi home mesin.
2. `Set Origin (G92)` untuk memosisikan spindle dan menetapkan nol material.

### Home All

`Home All (G28)` membuka dialog konfirmasi sebelum mengirim `G28`. Perintah hanya dikirim jika komunikasi CNC berstatus `CONNECTED` dan fitur Home aktif. Home All menggerakkan mesin menuju sensor home dan tidak menetapkan origin material.

### Set Origin

Menu Set Origin menampilkan koordinat X/Y/Z dan memungkinkan user mengatur posisi spindle sebelum menyimpan origin material.

Kontrol yang digunakan:

- Tombol X/Y/Z short press mengubah nilai `+/-1`.
- Tombol X/Y/Z long press mengubah nilai `+/-10`.
- Rotary encoder memilih axis.
- Putar rotary mengubah nilai axis terpilih.
- Tekan dan putar rotary mengubah nilai lebih besar.
- Tombol Enter membuka konfirmasi penyimpanan origin.

Saat konfirmasi `Yes`, firmware mengirim perintah:

```gcode
M400
G92 X0 Y0 Z0
M211 S1
```

`M211 S1` dikirim jika fitur soft endstop setelah Set Origin aktif di konfigurasi.

### Select Job

Menu Select Job langsung membuka browser SD card. Fitur yang tersedia:

- Menampilkan label SD card jika terbaca.
- Menampilkan path folder saat ini.
- Folder ditampilkan di atas file.
- Folder dan file diurutkan berdasarkan abjad.
- File yang dipilih disimpan sebagai job aktif.
- Enter atau klik rotary membuka folder atau mengonfirmasi file.
- Back kembali ke folder induk, lalu kembali ke menu utama dari root SD card.

Job aktif ditampilkan kembali di standby screen sebagai `Job: nama_file`.

### Machine Ctrl&Status

Menu ini digunakan untuk melihat dan mengatur status mesin yang berhubungan dengan Marlin. Urutan item aktual dan fungsinya:

1. `CNC`: menampilkan state komunikasi dan menjalankan pembacaan ulang status ketika dipilih.
2. `Home X/Y/Z`: membaca sensor endstop melalui `M119`.
3. `SoftEnd`: membaca status soft endstop melalui `M211`.
4. `Spindle`: membuka konfirmasi untuk mengirim `M3` atau `M5`.
5. `Feed XY`: membaca/mengubah feedrate maksimum X/Y melalui `M203`.
6. `Feed Z`: membaca/mengubah feedrate maksimum Z melalui `M203`.
7. `Area X/Y/Z`: menampilkan geometry report yang dibaca melalui `M115`.
8. `Refresh Status`: membaca ulang seluruh parameter status yang didukung.

Urutan tersebut memprioritaskan pemeriksaan komunikasi dan sensor keselamatan sebelum kontrol spindle serta parameter gerak.

`Refresh Status` terlebih dahulu menandai feedrate, area kerja, soft endstop, dan sensor home sebagai belum diketahui. Jika CNC berstatus `CONNECTED`, interface kemudian meminta:

- `M203` untuk feedrate maksimum X/Y/Z.
- `M115` untuk firmware/geometry report.
- `M119` untuk sensor home/endstop.
- `M211` untuk soft endstop.

Refresh tidak menjalankan homing, tidak menggerakkan axis, tidak mengubah origin, dan tidak me-restart Marlin. Posisi `M114` tetap diperbarui oleh polling periodik terpisah. Jika CNC belum terhubung, nilai yang belum diketahui tetap tampil `?`.

Perubahan nilai `Area X/Y/Z` saat ini masih hanya mengubah nilai lokal pada interface; belum ada command Marlin yang menerapkan perubahan area kerja tersebut.

Status sensor homing memakai format seperti `OPEN`, `TRIGGER`, atau `?` jika belum diketahui.

### Network

Menu Network digunakan untuk konfigurasi dan pemantauan koneksi:

- `WiFi`: mengaktifkan atau menonaktifkan WiFi melalui konfirmasi.
- `SSID`: menampilkan nama jaringan yang terhubung.
- `IP`: menampilkan alamat IP ESP32.
- `MQTT`: mengaktifkan atau menonaktifkan MQTT; WiFi harus aktif.
- `Link`: menampilkan kondisi koneksi MQTT dan broker aktif.
- `Broker`: menampilkan alamat broker dan port.
- `AP`: menampilkan nama access point konfigurasi WiFiManager.
- `Time`: meminta sinkronisasi NTP jika WiFi terhubung.
- `Date`: menampilkan tanggal internal ESP32.
- `Reset WiFi`: menghapus credential lama dan membuka kembali portal konfigurasi setelah konfirmasi.

Saat WiFi setup aktif, LCD menampilkan instruksi untuk menyambungkan perangkat ke AP `CNC-Interface-Setup`.

Saat `Reset WiFi` dikonfirmasi, credential lama dihapus dan portal `CNC-Interface-Setup` langsung dibuka kembali. User tidak perlu mematikan dan menyalakan toggle WiFi secara manual.

### About

Menu About menampilkan logo firmware di tengah atas serta informasi yang bergulir otomatis di bawahnya:

- Nama firmware `AYB Interface`.
- Versi firmware.
- Nama pembuat `Alfath Yusuf Biyono`.
- NIM `2141170132`.
- Tanggal pembaruan firmware terakhir.

Layar About dapat ditutup menggunakan tombol Enter, tombol Back, atau klik rotary encoder. Animasi scrolling menggunakan `millis()` dan tidak menghentikan loop utama.

### Dialog Konfirmasi

Dialog konfirmasi menggunakan aturan konsisten:

- Tombol Enter fisik selalu `Yes`.
- Tombol Back fisik selalu `No`.
- Putar rotary memilih `Yes` atau `No`.
- Klik rotary menjalankan pilihan yang disorot.

## MQTT Lokal

Konfigurasi default broker dan topic berada di `src/config/MqttConfig.h`. Nilai broker yang sudah disimpan melalui Preferences/WiFiManager tetap dapat menggantikan nilai default saat runtime.

Default broker lokal saat ini adalah `192.168.250.3:1883`, sesuai IPv4 laptop saat pengujian. Sebaiknya laptop diberi DHCP reservation/static IP atau broker diperbarui melalui WiFiManager jika alamat laptop berubah.

Topic utama yang digunakan:

- `cnc/status` - publish status koneksi ESP32 ke broker.
- `cnc/command` - subscribe command sederhana dan tampilkan payload di Serial Monitor.
- `cnc/gcode` - subscribe baris G-code dan tampilkan payload di Serial Monitor.
- `cnc/progress` - publish progress job; bernilai `null` dan `idle` jika job belum berjalan.
- `cnc/error` - publish status error aktif.
- `cnc/position` - publish posisi X/Y/Z dari Marlin.
- `cnc/machine` - publish state CNC, spindle, soft endstop, feedrate, dan sensor home.

Topic monitoring lama tetap dipertahankan agar dashboard yang sudah ada tidak rusak:

- `cnc/network`
- `cnc/time`
- `cnc/alarm`

Payload dikirim dalam format JSON.

Contoh `cnc/status` saat terhubung:

```json
{
  "connected": true,
  "state": "online"
}
```

Contoh `cnc/position`:

```json
{
  "x": 0.0,
  "y": 0.0,
  "z": 0.0,
  "display": {"x": "0.00", "y": "0.00", "z": "0.00"},
  "unit": "mm",
  "valid": true,
  "source": "marlin"
}
```

Saat Marlin belum memberikan posisi nyata:

```json
{
  "x": null,
  "y": null,
  "z": null,
  "display": {"x": "?", "y": "?", "z": "?"},
  "unit": "mm",
  "valid": false,
  "source": "unavailable"
}
```

Contoh `cnc/machine` ketika komunikasi CNC dinonaktifkan:

```json
{
  "state": "OFF",
  "connected": false,
  "spindle": "?",
  "soft_endstop": "?",
  "feed_xy": null,
  "feed_z": null,
  "x_home": "?",
  "y_home": "?",
  "z_home": "?"
}
```

Jalur simulasi posisi MQTT sudah disiapkan melalui `MqttConfig::ENABLE_POSITION_SIMULATION`, tetapi default tetap `false` agar dashboard tidak menerima data palsu. Aktifkan hanya untuk pengujian terkontrol tanpa Marlin.

`cnc/command` dan `cnc/gcode` pada tahap ini bersifat receive-only. Payload hanya dicetak ke Serial Monitor dengan prefix `[MQTT][COMMAND]` atau `[MQTT][GCODE]`. Firmware tidak mengeksekusi payload dan tidak meneruskannya ke UART Marlin.

Menu Reset WiFi mengembalikan parameter MQTT ke default `MqttConfig.h` sebelum portal konfigurasi berikutnya.

## Integrasi Marlin

Firmware menggunakan UART `Serial1` untuk komunikasi ke Marlin/SKR.

Komunikasi CNC dikendalikan dari `src/main.cpp`:

```cpp
// #define CNC_ENABLE_MARLIN_CONNECTION
```

Selama SKR V1.4 Turbo belum tersambung, biarkan toggle tersebut dikomentari. Standby akan menampilkan `CNC:OFF`, polling `M114` tidak dijalankan, perintah mesin tidak dikirim, dan alarm buzzer Marlin tidak aktif.

Status integrasi saat ini:

- Saat komunikasi diaktifkan, ESP32 mengirim `M114` secara periodik untuk meminta posisi.
- Respons Marlin dengan format posisi `X:... Y:... Z:...` diparse ke `_posX`, `_posY`, dan `_posZ`.
- Nilai posisi tersebut dipakai oleh tampilan standby, Set Origin, dan MQTT `cnc/position`.
- Perintah gerak, spindle, feedrate, dan Set Origin hanya dikirim saat status Marlin `CONNECTED`.

Status ringkas pada standby:

- `CNC:OFF`: komunikasi dinonaktifkan.
- `CNC:WAIT`: menunggu respons awal.
- `CNC:DISC`: belum pernah terhubung setelah timeout.
- `CNC:OK`: Marlin merespons normal.
- `CNC:LOST`: koneksi yang sebelumnya aktif terputus.
- `CNC:ERR`: Marlin mengirim error.

Menu Machine Ctrl&Status menampilkan `CNC: OFF`, `CNC: WAITING`, `CNC: DISCONNECTED`, `CNC: CONNECTED`, `CNC: LOST`, atau `CNC: ERROR`.

Jika koneksi yang sebelumnya aktif terputus, MQTT `cnc/alarm` mengirim:

```json
{
  "level": "ERROR",
  "message": "Marlin connection lost"
}
```

- Status `DISCONNECTED` sebelum koneksi pertama tidak dianggap alarm. Jika Marlin kembali merespons normal, `cnc/alarm` kembali menjadi:

```json
{
  "level": "INFO",
  "message": "OK"
}
```

## Build

Environment PlatformIO:

```text
esp32-s3-devkitm-1
```

Perintah build:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run
```

## Batasan Saat Ini

- Job controller / G-code sender penuh belum selesai.
- MQTT command sudah dapat diterima dan dicatat di Serial, tetapi belum dieksekusi.
- Baris G-code MQTT sudah dapat diterima dan dicatat di Serial, tetapi belum dikirim ke Marlin atau disimpan ke SD card.
- Upload file G-code lengkap via MQTT belum tersedia.
