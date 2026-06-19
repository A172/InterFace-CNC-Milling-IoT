# InterFace CNC Milling IoT

Firmware antarmuka CNC milling berbasis ESP32-S3 untuk menghubungkan user, LCD 128x64, SD card, jaringan lokal, MQTT, dan controller CNC berbasis Marlin pada SKR V1.4 Turbo.

Project ini dikembangkan sebagai bagian dari proyek akhir D4 Teknik Elektronika Politeknik Negeri Malang.

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

Nilai koordinat X/Y/Z berasal dari variabel posisi internal yang diperbarui dari respons `M114` Marlin.

### Main Menu

Menu utama berisi:

- `Move & Jog`
- `Select Job`
- `Machine Ctrl&Status`
- `Network`
- `Settings`

Navigasi menu menggunakan rotary encoder. Klik rotary atau tombol Enter menjalankan item yang dipilih.

### Submenu Move & Jog

Submenu ini berisi workflow gerak manual dan Set Origin. Bagian Set Origin menjadi workflow utama untuk menentukan titik nol bidang kerja.

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

Job aktif ditampilkan kembali di standby screen sebagai `Job: nama_file`.

### Machine Ctrl&Status

Menu ini digunakan untuk melihat dan mengatur status mesin yang berhubungan dengan Marlin. Item yang tersedia meliputi:

- Status spindle.
- Status koneksi Marlin.
- Status soft endstop.
- Feedrate XY.
- Feedrate Z.
- Ukuran area kerja X/Y/Z.
- Sensor homing X/Y/Z.
- Refresh status.

Status sensor homing memakai format seperti `OPEN`, `TRIGGER`, atau `?` jika belum diketahui.

### Network

Menu Network digunakan untuk konfigurasi dan pemantauan koneksi:

- Status WiFi.
- SSID.
- IP ESP32.
- Status MQTT.
- Link MQTT.
- Broker MQTT.
- Nama AP setup WiFiManager.
- Status waktu NTP.
- Tanggal.
- Reset WiFi.

Saat WiFi setup aktif, LCD menampilkan instruksi untuk menyambungkan perangkat ke AP `CNC-Interface-Setup`.

### Dialog Konfirmasi

Dialog konfirmasi menggunakan aturan konsisten:

- Tombol Enter fisik selalu `Yes`.
- Tombol Back fisik selalu `No`.
- Putar rotary memilih `Yes` atau `No`.
- Klik rotary menjalankan pilihan yang disorot.

## MQTT Monitoring Lokal

Topic monitoring yang digunakan:

- `cnc/network`
- `cnc/time`
- `cnc/alarm`
- `cnc/position`

Payload dikirim dalam format JSON.

Contoh `cnc/position`:

```json
{
  "x": 0.0,
  "y": 0.0,
  "z": 0.0,
  "unit": "mm"
}
```

Parameter MQTT default ada di `src/config/AppConfig.h`, tetapi nilai yang sudah tersimpan di Preferences/WiFiManager akan dipakai lebih dulu. Menu Reset WiFi mengembalikan parameter MQTT ke default `AppConfig.h` sebelum portal konfigurasi berikutnya.

## Integrasi Marlin

Firmware menggunakan UART `Serial1` untuk komunikasi ke Marlin/SKR.

Status integrasi saat ini:

- ESP32 mengirim `M114` secara periodik untuk meminta posisi.
- Respons Marlin dengan format posisi `X:... Y:... Z:...` diparse ke `_posX`, `_posY`, dan `_posZ`.
- Nilai posisi tersebut dipakai oleh tampilan standby, Set Origin, dan MQTT `cnc/position`.
- Jika Marlin tidak merespons melewati timeout, MQTT `cnc/alarm` mengirim:

```json
{
  "level": "ERROR",
  "message": "Marlin no response"
}
```

- Jika Marlin kembali merespons normal, `cnc/alarm` kembali menjadi:

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
- MQTT command belum diaktifkan.
- Upload file G-code via MQTT belum diaktifkan.
- MQTT saat ini hanya untuk monitoring lokal.
