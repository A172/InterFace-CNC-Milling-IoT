# Project Status

## Ringkasan Status Project Saat Ini

Firmware **AYB Interface** pada ESP32-S3 DevKitM-1 sudah memiliki UI LCD, input tombol/rotary, passive buzzer, SD card browser, konfigurasi jaringan lewat WiFiManager, MQTT monitoring lokal, sinkronisasi waktu NTP, dan integrasi awal Marlin pada BTT SKR V1.4 Turbo untuk membaca posisi melalui `M114`.

## Fitur yang Sudah Selesai

- LCD menu system untuk booting, standby, menu utama, submenu, info, dan konfirmasi.
- Input push button dan rotary encoder.
- Passive buzzer GPIO 38 dengan PWM LEDC dan pola bunyi non-blocking.
- Feedback bunyi untuk Enter, Back, klik rotary, warning, job selesai, dan alarm timeout Marlin.
- Identitas `AYB Interface` digunakan secara terpusat oleh boot screen, Serial Monitor, menu About, dan dokumentasi.
- Layar About menampilkan logo, versi, Alfath Yusuf Biyono, NIM 2141170132, dan tanggal pembaruan dengan scrolling non-blocking.
- Toggle `CNC_ENABLE_MARLIN_CONNECTION` mengendalikan komunikasi UART/polling Marlin selama hardware CNC belum tersambung.
- Standby menampilkan status `CNC:OFF/WAIT/DISC/OK/LOST/ERR`.
- Perintah mesin diblokir sampai Marlin berstatus `CONNECTED`.
- Standby screen menampilkan waktu, koordinat X/Y/Z, job aktif, estimasi, dan status Marlin.
- Koordinat standby menggunakan `?` sampai posisi nyata diterima dari `M114`.
- Standby screen menampilkan `WiFi` dan `MQTT` secara lengkap di bawah koordinat Y dengan status `V`, `X`, atau `?`.
- Dialog konfirmasi memiliki aturan tetap: Enter = Yes, Back = No, rotary memilih dan mengeksekusi pilihan.
- SD card browser dengan pemilihan file job.
- Select Job langsung membuka SD card, menampilkan label/path, dan mengurutkan folder sebelum file.
- Menu Set Origin dengan jog X/Y/Z, `G92 X0 Y0 Z0`, dan opsi soft endstop `M211 S1`.
- Menu Machine Ctrl&Status untuk spindle, feedrate, area kerja, soft endstop, dan sensor homing.
- Urutan Machine Ctrl&Status memprioritaskan status CNC, sensor home, dan soft endstop sebelum spindle serta parameter gerak.
- Menu Network untuk WiFi, MQTT, broker, AP setup, waktu, tanggal, dan reset WiFi.
- Reset WiFi langsung membuka kembali portal WiFiManager tanpa toggle OFF/ON manual.
- WiFiManager untuk WiFi dan parameter MQTT.
- MQTT lokal memakai PubSubClient dengan konfigurasi terpisah di `src/config/MqttConfig.h`.
- Publish topic `cnc/status`, `cnc/progress`, `cnc/error`, dan `cnc/position`.
- Publish retained `cnc/machine` berisi state CNC, spindle, soft endstop, feedrate, dan sensor home.
- `cnc/position` membawa field `valid`, `source`, serta display `?` ketika posisi belum tersedia.
- Jalur simulasi posisi MQTT tersedia tetapi default dinonaktifkan agar tidak menghasilkan data dummy permanen.
- Subscribe topic `cnc/command` dan `cnc/gcode` dalam mode receive-only ke Serial Monitor.
- Topic monitoring lama `cnc/network`, `cnc/time`, dan `cnc/alarm` tetap dipertahankan.
- Sinkronisasi waktu NTP WIB.
- Polling Marlin `M114` secara periodik.
- Parsing posisi Marlin X/Y/Z dari respons `M114`.
- Alarm MQTT/buzzer aktif saat koneksi Marlin yang sebelumnya berhasil menjadi `LOST` atau Marlin mengirim `ERROR`.
- Dokumentasi hardware mencakup ESP32-S3 DevKitM-1, BTT SKR V1.4 Turbo, Marlin, DM542, NEMA23, LCD, rotary encoder, push button, MicroSD, WiFi, dan MQTT broker.
- `ARCHITECTURE.md` sudah diperbarui dengan diagram ASCII dan pembagian modul firmware.
- README menjelaskan fungsi menu utama, submenu, setiap item Machine Ctrl&Status, Network, dan Refresh Status berdasarkan source aktual.

## Fitur yang Sedang Dikerjakan

- Validasi runtime komunikasi Marlin pada hardware SKR V1.4 Turbo.
- Validasi payload `cnc/machine` dan transisi validitas `cnc/position` melalui MQTT Explorer.
- Validasi animasi serta tombol keluar layar About pada LCD/input hardware.
- Validasi transisi status `WAIT`, `DISCONNECTED`, `CONNECTED`, `LOST`, dan `ERROR` pada hardware nyata.
- Penyempurnaan machine status berbasis data nyata firmware Marlin.
- Pemanfaatan posisi Marlin untuk workflow job yang lebih lengkap.
- Perancangan validasi dan izin eksekusi command/G-code MQTT yang aman.

## Daftar Bug yang Masih Ada

- Broker Mosquitto memakai IP laptop; koneksi akan gagal jika DHCP mengubah alamat laptop dan konfigurasi belum diperbarui.
- Selama portal WiFiManager aktif, loop UI menunggu proses konfigurasi atau timeout portal.
- Full G-code sender/job controller belum tersedia.
- Progress job masih belum berasal dari eksekusi G-code nyata.
- Payload `cnc/command` baru dicetak ke Serial dan belum dieksekusi.
- Payload `cnc/gcode` baru dicetak ke Serial dan belum dikirim ke Marlin atau SD card.
- Upload file G-code lengkap lewat MQTT belum tersedia.
- Validasi sensor homing dan parameter mesin masih perlu diuji di hardware.
- Level suara, rangkaian driver, dan pola passive buzzer belum divalidasi pada hardware nyata.
- Detail wiring final antar ESP32-S3, SKR, driver DM542, sensor, dan aktuator masih perlu divalidasi terhadap rangkaian nyata.
- Edit nilai Area X/Y/Z pada Machine Ctrl&Status masih lokal dan belum diterapkan ke Marlin.

## TODO Prioritas Berikutnya

- Uji `M114` langsung dengan SKR/Marlin dan pastikan `cnc/position` berubah sesuai gerakan mesin.
- Verifikasi `cnc/position` menampilkan `valid:false`, nilai `null`, dan display `?` sebelum Marlin tersambung.
- Verifikasi `cnc/machine` mengikuti state CNC dan data status mesin.
- Uji Enter, Back, dan klik rotary untuk keluar dari layar About.
- Aktifkan `CNC_ENABLE_MARLIN_CONNECTION` setelah SKR tersambung, lalu uji seluruh transisi status CNC.
- Uji koneksi yang terputus setelah pernah aktif dan verifikasi `cnc/alarm` berubah ke `ERROR`.
- Uji alarm kembali ke `INFO` setelah Marlin merespons lagi.
- Uji passive buzzer GPIO 38 dan pastikan rangkaian driver sesuai arus buzzer.
- Lanjutkan desain G-code sender/job controller dari file SD card.
- Tentukan format progress job dan estimasi waktu berbasis eksekusi nyata.
- Jaga README, PROJECT_STATUS, dan CHANGELOG tetap sinkron setiap ada perubahan fitur/workflow.
- Lengkapi dokumentasi wiring/pinout jika hardware final sudah dikunci.
- Tentukan whitelist command MQTT dan workflow konfirmasi sebelum eksekusi mesin diaktifkan.
- Rancang protokol upload G-code bertahap dengan validasi ukuran, urutan, checksum, dan penyimpanan SD card.
- Tetapkan DHCP reservation/static IP untuk PC Mosquitto agar alamat broker stabil.
- Tentukan command dan workflow aman jika Area X/Y/Z nantinya perlu diterapkan ke firmware Marlin.
