# Project Status

## Ringkasan Status Project Saat Ini

Firmware ESP32-S3 sudah memiliki UI LCD, input tombol/rotary, SD card browser, konfigurasi jaringan lewat WiFiManager, MQTT monitoring lokal, sinkronisasi waktu NTP, dan integrasi awal Marlin untuk membaca posisi melalui `M114`.

## Fitur yang Sudah Selesai

- LCD menu system untuk booting, standby, menu utama, submenu, info, dan konfirmasi.
- Input push button dan rotary encoder.
- Standby screen menampilkan waktu, koordinat X/Y/Z, job aktif, estimasi, dan status Marlin.
- Dialog konfirmasi memiliki aturan tetap: Enter = Yes, Back = No, rotary memilih dan mengeksekusi pilihan.
- SD card browser dengan pemilihan file job.
- Select Job langsung membuka SD card, menampilkan label/path, dan mengurutkan folder sebelum file.
- Menu Set Origin dengan jog X/Y/Z, `G92 X0 Y0 Z0`, dan opsi soft endstop `M211 S1`.
- Menu Machine Ctrl&Status untuk spindle, feedrate, area kerja, soft endstop, dan sensor homing.
- Menu Network untuk WiFi, MQTT, broker, AP setup, waktu, tanggal, dan reset WiFi.
- WiFiManager untuk WiFi dan parameter MQTT.
- MQTT monitoring lokal dengan topic `cnc/network`, `cnc/time`, `cnc/alarm`, dan `cnc/position`.
- Sinkronisasi waktu NTP WIB.
- Polling Marlin `M114` secara periodik.
- Parsing posisi Marlin X/Y/Z dari respons `M114`.
- Alarm MQTT `ERROR` saat Marlin tidak merespons dan kembali `INFO` saat normal.

## Fitur yang Sedang Dikerjakan

- Validasi runtime komunikasi Marlin pada hardware SKR V1.4 Turbo.
- Penyempurnaan machine status berbasis data nyata firmware Marlin.
- Pemanfaatan posisi Marlin untuk workflow job yang lebih lengkap.

## Daftar Bug yang Masih Ada

- Full G-code sender/job controller belum tersedia.
- Progress job masih belum berasal dari eksekusi G-code nyata.
- Upload G-code lewat MQTT belum tersedia.
- MQTT command sengaja belum diaktifkan.
- Validasi sensor homing dan parameter mesin masih perlu diuji di hardware.

## TODO Prioritas Berikutnya

- Uji `M114` langsung dengan SKR/Marlin dan pastikan `cnc/position` berubah sesuai gerakan mesin.
- Uji timeout Marlin dan verifikasi `cnc/alarm` berubah ke `ERROR`.
- Uji alarm kembali ke `INFO` setelah Marlin merespons lagi.
- Lanjutkan desain G-code sender/job controller dari file SD card.
- Tentukan format progress job dan estimasi waktu berbasis eksekusi nyata.
- Jaga README, PROJECT_STATUS, dan CHANGELOG tetap sinkron setiap ada perubahan fitur/workflow.
