# InterFace CNC Milling IoT

Firmware ESP32-S3 untuk Modul Antarmuka Pengendali CNC Mandiri. Sistem dirancang agar
mesin CNC dapat dikendalikan melalui antarmuka lokal berupa tombol, rotary encoder, LCD,
dan SD card, serta dapat terhubung ke internet untuk monitoring dan pengiriman file G-code.

## Kepemilikan Project

Project ini merupakan karya original yang dikembangkan secara mandiri oleh penulis sebagai
bagian dari penelitian skripsi. Seluruh rancangan firmware, struktur modul, integrasi input
lokal, SD card, LCD, WiFi, dan komunikasi IoT disusun untuk mendukung implementasi Modul
Antarmuka Pengendali CNC Mandiri.

## Tujuan

Project ini dikembangkan untuk kebutuhan skripsi dengan fokus pada:

- Antarmuka mandiri untuk pengendali CNC.
- Pembacaan input lokal melalui push button dan rotary encoder.
- Penyimpanan dan pemilihan file G-code melalui SD card.
- Koneksi WiFi menggunakan WiFiManager.
- Komunikasi IoT internet menggunakan MQTT.
- Rencana upload file G-code dari dashboard/cloud ke device.

## Hardware Utama

- ESP32-S3 DevKitM-1
- LCD grafis berbasis U8g2
- 6 push button
- Rotary encoder dengan push switch
- Modul SD card
- Tombol emergency stop fisik terpisah

## Input Lokal

Mapping awal:

```text
BTN1: Up / X+
BTN2: Down / X-
BTN3: Left / Back / Y-
BTN4: Right / Select / Y+
BTN5: OK / Start / Resume
BTN6: Cancel / Pause / Stop
Encoder CW : Z+ saat mode jog Z
Encoder CCW: Z- saat mode jog Z
Encoder SW : OK / Select
```

Pin dapat disesuaikan di `src/config/PinConfig.h`.

## Struktur Folder

```text
src/
  app/        Koordinator utama aplikasi firmware
  assets/     Simbol dan aset kecil untuk tampilan
  config/     Konfigurasi pin dan konstanta hardware
  display/    LCD dan menu lokal
  input/      Push button dan rotary encoder
  network/    WiFi portal dan MQTT cloud client
  storage/    SD card dan penyimpanan file G-code

docs/
  Rencana arsitektur, menu, UI, dan komunikasi IoT
```

## Alur IoT

1. Device membuka portal WiFi `CNC-Interface-Setup` saat belum memiliki kredensial.
2. Setelah terhubung ke internet, device terkoneksi ke MQTT broker.
3. Device mengirim status periodik dan event input.
4. Dashboard/cloud dapat mengirim command dan file G-code.
5. File G-code disimpan ke SD card sebelum dieksekusi.

## Catatan Keselamatan

- Emergency stop wajib berupa tombol fisik dan tidak bergantung pada internet.
- Perintah jarak jauh wajib divalidasi berdasarkan state mesin.
- File G-code dari IoT tidak boleh langsung dieksekusi sebelum lengkap dan tervalidasi.
- Untuk versi final, gunakan MQTT broker private dengan autentikasi.

## Build

Project menggunakan PlatformIO.

```powershell
pio run
```

Jika command `pio` belum tersedia, aktifkan PlatformIO CLI dari VS Code atau tambahkan
PlatformIO ke PATH sistem.
