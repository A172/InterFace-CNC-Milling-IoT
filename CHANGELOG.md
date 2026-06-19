# Changelog

## 2026-06-19

- Menjaga konfigurasi MQTT yang sudah bekerja tanpa mengubah topic monitoring.
- Mengaktifkan polling `M114` periodik untuk monitoring posisi Marlin.
- Memastikan posisi X/Y/Z dari respons Marlin dipakai oleh snapshot MQTT `cnc/position`.
- Mengubah alarm MQTT agar fokus pada status respons Marlin:
  - `ERROR / Marlin no response` saat Marlin timeout.
  - `INFO / OK` saat Marlin kembali normal.
- Menambahkan dokumentasi status project sesuai aturan `AGENTS.md`.
- Memperluas README dengan dokumentasi UI LCD: boot, standby, main menu, submenu, Select Job, Set Origin, Machine Ctrl&Status, Network, dan dialog konfirmasi.
- Memperbarui PROJECT_STATUS agar mencatat workflow UI yang sudah selesai.

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
