# Project Development Rules
## Cara Kerja

Sebelum mengubah source code:

1. Baca AGENTS.md
2. Baca PROJECT_STATUS.md
3. Baca README.md
4. Analisis struktur project
5. Identifikasi file yang akan diubah
6. Jelaskan rencana perubahan sebelum melakukan modifikasi besar

Setiap perubahan signifikan pada project WAJIB melakukan review dan pembaruan dokumentasi.

Sebelum menyelesaikan tugas:

1. Perbarui README.md jika terdapat perubahan fitur, arsitektur, hardware, software, atau workflow.

2. Perbarui PROJECT_STATUS.md dengan format:

## Ringkasan Status Project Saat Ini

## Fitur yang Sudah Selesai

## Fitur yang Sedang Dikerjakan

## Daftar Bug yang Masih Ada

## TODO Prioritas Berikutnya

3. Perbarui CHANGELOG.md dengan perubahan yang dilakukan pada sesi tersebut.

4. Jangan menganggap dokumentasi selesai sebelum ketiga file tersebut diperbarui.

README.md harus selalu merepresentasikan kondisi project saat ini berdasarkan source code sebagai sumber kebenaran utama.

Jika menemukan README tidak sesuai source code, README harus diperbaiki.

## Aturan Pengembangan MQTT

1. Konfigurasi default broker, port, client ID, topic, dan timeout MQTT disimpan di `src/config/MqttConfig.h`.

2. `cnc/command` dan `cnc/gcode` bersifat receive-only sampai terdapat validasi payload, whitelist command, interlock keselamatan, dan konfirmasi user.

3. Jangan meneruskan payload MQTT langsung ke UART Marlin atau menulisnya ke SD card tanpa requirement eksplisit dan review keselamatan.

4. Perubahan MQTT tidak boleh mengganggu update loop LCD, SD card, button, rotary encoder, atau komunikasi UART Marlin.

5. Topic monitoring yang sudah digunakan dashboard tidak boleh dihapus tanpa migrasi dan pembaruan dokumentasi.

## Aturan Hardware

1. Jangan mengubah pin mapping pada src/config/PinConfig.h tanpa instruksi eksplisit.

2. Jika diperlukan perubahan pin, jelaskan alasan teknis dan dampaknya terhadap PCB serta wiring.

## Aturan Refactor

1. Jangan melakukan refactor besar tanpa instruksi eksplisit.

2. Prioritaskan stabilitas dibanding perubahan arsitektur.

3. Jangan memindahkan file atau class hanya demi kerapian jika tidak memberikan manfaat nyata.

## Aturan Build

Setelah mengubah source code:

1. Jalankan build.
2. Pastikan build sukses.
3. Laporkan error jika build gagal.
4. Jangan menganggap tugas selesai jika build belum diverifikasi.

## Aturan Data Monitoring

1. Jangan membuat data dummy permanen.

2. Jika data nyata belum tersedia:
   - gunakan placeholder sementara
   - beri komentar TODO

3. Setelah sumber data tersedia, gantikan placeholder dengan data nyata.

## Prioritas Project

Urutan prioritas:

1. Stabilitas firmware
2. Keselamatan sistem CNC
3. Komunikasi Marlin
4. SD Card
5. MQTT Monitoring
6. Dashboard
7. MQTT Command
8. Upload G-code melalui MQTT

Jangan mengorbankan stabilitas demi fitur baru.
Setiap selesai tugas, update PROJECT_STATUS.md dengan format:

## Ringkasan Status Project Saat Ini

## Fitur yang Sudah Selesai

## Fitur yang Sedang Dikerjakan

## Daftar Bug yang Masih Ada

## TODO Prioritas Berikutnya

## Aturan Keselamatan Marlin

1. Jangan mengirim command gerakan (G0, G1, G28, G92, M3, M5, dll) tanpa instruksi eksplisit.

2. Jangan mengubah workflow homing, origin, soft endstop, atau spindle tanpa persetujuan user.

3. Jangan mengubah konfigurasi keselamatan CNC hanya untuk membuat fitur baru bekerja.

4. Keselamatan mesin lebih penting daripada otomatisasi.

## Aturan Komunikasi Marlin

1. Jangan mengubah baudrate, pin UART, atau protokol komunikasi tanpa instruksi eksplisit.

2. Jangan menghapus timeout, retry, atau mekanisme deteksi error komunikasi.

3. Jika komunikasi gagal, prioritaskan logging dan diagnosis sebelum mengubah arsitektur.

## Source of Truth

Urutan sumber kebenaran project:

1. Source code
2. PinConfig.h
3. AppConfig.h
4. PROJECT_STATUS.md
5. README.md

Jika dokumentasi berbeda dengan source code, source code dianggap benar dan dokumentasi harus diperbarui.

## Format Laporan Setelah Tugas

Setelah menyelesaikan tugas, laporkan:

1. File yang diubah
2. Alasan perubahan
3. Risiko perubahan
4. Hasil build
5. Cara pengujian
6. Dokumentasi yang diperbarui

## Aturan Pengujian

1. Jangan menganggap fitur berfungsi hanya karena build berhasil.

2. Jika perubahan menyentuh:
   - MQTT
   - UART Marlin
   - SD Card
   - LCD
   - Input Button
   - Rotary Encoder

   maka sertakan langkah pengujian yang harus dilakukan user.

3. Bedakan dengan jelas:
   - Build Success
   - Logic Verified
   - Hardware Tested

4. Jika hardware tidak tersedia, nyatakan asumsi dan batasan pengujian.

## Aturan Dependency

1. Jangan menambahkan library baru jika fungsi yang dibutuhkan sudah dapat dibuat menggunakan library yang ada.

2. Jika membutuhkan library baru:
   - jelaskan alasan teknis
   - jelaskan manfaat
   - jelaskan dampak terhadap ukuran firmware

3. Hindari dependency yang tidak aktif dipelihara.

## Aturan Resource ESP32

1. Prioritaskan stabilitas dan penggunaan memori yang efisien.

2. Hindari alokasi String besar secara berulang pada loop utama.

3. Hindari blocking delay yang panjang.

4. Perubahan tidak boleh menyebabkan:
   - watchdog reset
   - reboot acak
   - fragmentasi memori berlebihan

5. Jika perubahan berpotensi meningkatkan penggunaan RAM atau Flash secara signifikan, laporkan dampaknya.git