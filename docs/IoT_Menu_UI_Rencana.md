# Rencana IoT, Menu, dan UI Firmware CNC

## Arah Sistem

Firmware ini diarahkan menjadi Modul Antarmuka Pengendali CNC Mandiri berbasis ESP32-S3.
Modul tetap bisa dipakai lokal melalui tombol, encoder, LCD, dan SD card, tetapi juga dapat
terhubung ke internet melalui WiFi untuk monitoring, pengiriman file G-code, dan kendali
jarak jauh yang dibatasi oleh prosedur keselamatan mesin.

## Alur Koneksi Internet

1. Saat pertama menyala, device membuka portal WiFi bernama `CNC-Interface-Setup`.
2. User memilih SSID dan mengisi password melalui portal WiFiManager.
3. Setelah internet aktif, device terhubung ke broker MQTT.
4. Device mengirim heartbeat dan status mesin secara periodik.
5. Aplikasi/dashboard cloud dapat mengirim file G-code dan perintah melalui topic MQTT.
6. File G-code yang diterima disimpan ke SD card, lalu dapat dipilih untuk dieksekusi.

## Topic MQTT Awal

Base topic:

```text
skripsi/cnc-interface/<mac-address-device>
```

Publish dari device:

```text
skripsi/cnc-interface/<mac>/availability
skripsi/cnc-interface/<mac>/status
skripsi/cnc-interface/<mac>/event/button
skripsi/cnc-interface/<mac>/event/encoder
skripsi/cnc-interface/<mac>/upload/ack
skripsi/cnc-interface/<mac>/upload/progress
```

Subscribe oleh device:

```text
skripsi/cnc-interface/<mac>/cmd/#
skripsi/cnc-interface/<mac>/upload/#
```

Contoh command yang disarankan nanti:

```text
cmd/job/start
cmd/job/pause
cmd/job/resume
cmd/job/stop
cmd/jog/x
cmd/jog/y
cmd/jog/z
cmd/spindle
cmd/feedrate
```

## Rencana Upload File G-code dari Internet

Pengiriman file G-code sebaiknya tidak dikirim sebagai satu payload besar. MQTT pada
mikrokontroler lebih aman memakai sistem potongan data atau chunk.

Alur upload:

```text
1. Dashboard mengirim upload/start
2. Device membuat file sementara di SD card
3. Dashboard mengirim upload/chunk berurutan
4. Device menulis setiap chunk ke SD card dan mengirim ACK
5. Dashboard mengirim upload/finish
6. Device validasi ukuran/checksum sederhana
7. File masuk daftar pekerjaan CNC
```

Topic yang disarankan:

```text
upload/start   -> metadata file
upload/chunk   -> potongan isi G-code
upload/finish  -> penutup upload
upload/cancel  -> batalkan upload
```

Contoh payload `upload/start`:

```json
{
  "filename": "part_a.nc",
  "size": 18420,
  "checksum": "optional"
}
```

Contoh payload `upload/chunk`:

```json
{
  "index": 0,
  "data": "G21\nG90\nG0 X0 Y0\n"
}
```

Catatan penting:

- File dari internet wajib disimpan dulu, bukan langsung dieksekusi saat diterima.
- Eksekusi baru boleh dilakukan setelah file lengkap dan user memberi perintah `start`.
- Untuk G-code besar, ukuran chunk disarankan 512 sampai 1024 byte.
- Pada versi final, dashboard perlu menunggu ACK setiap chunk agar upload tidak rusak.

## Struktur Menu LCD

Menu utama:

```text
1. Status Mesin
2. File SD Card
3. Kontrol Manual
4. Job Control
5. IoT / Network
6. Settings
```

### 1. Status Mesin

Tampilan ringkas untuk layar utama.

```text
IDLE / RUN / HOLD / ALARM
X: 000.000  Y: 000.000
Z: 000.000  F: 100%
WiFi: OK    IoT: OK
```

Data yang ideal ditampilkan:

- Mode mesin: idle, run, hold, alarm
- Posisi X, Y, Z
- Feedrate override
- Status WiFi dan MQTT
- Progress job jika sedang berjalan

### 2. File SD Card

Fungsi:

- List file `.gcode`, `.nc`, atau `.tap`
- Pilih file
- Preview nama dan ukuran file
- Start job dari file yang dipilih
- Tampilkan file yang diterima dari IoT

Tampilan:

```text
> part_a.nc
  test_cut.gcode
  facing.tap
OK: Select  Back: Menu
```

### 3. Kontrol Manual

Fungsi:

- Jog X, Y, Z
- Pilih step jog: 0.1 mm, 1 mm, 10 mm
- Home axis
- Set zero/work origin
- Rotary encoder digunakan untuk Z+ dan Z- saat mode jog Z aktif

Tampilan:

```text
Jog: X+  Step: 1.0mm
X: 000.000
Y: 000.000
Z: 000.000
```

Alternatif tampilan saat jog Z:

```text
Jog Axis: Z
Rotate CW : Z+
Rotate CCW: Z-
Step: 1.0mm
```

### 4. Job Control

Fungsi:

- Start
- Pause
- Resume
- Stop
- Emergency stop tetap disarankan berupa tombol fisik terpisah

Tampilan:

```text
RUNNING part_a.nc
Line: 120 / 840
Progress: 14%
Pause  Stop  Info
```

### 5. IoT / Network

Fungsi:

- Lihat IP address
- Lihat status MQTT
- Lihat device ID / base topic
- Lihat status upload G-code
- Reset WiFi credential
- Mode pairing ulang

Tampilan:

```text
WiFi: Connected
IP: 192.168.1.20
MQTT: Online
Device: A1B2C3D4E5F6
```

Tampilan upload:

```text
UPLOAD part_a.nc
Rx: 12.4 KB / 18.0 KB
Status: Receiving
Cancel
```

### 6. Settings

Fungsi:

- Brightness LCD jika didukung
- Baudrate komunikasi CNC
- Satuan mm/inch
- Default jog step
- MQTT server jika ingin dibuat dapat diubah dari menu

## Saran Mapping Tombol

Jika memakai 6 tombol:

```text
BTN1: Up / X+
BTN2: Down / X-
BTN3: Left / Back / Y-
BTN4: Right / Select / Y+
BTN5: OK / Start / Resume
BTN6: Cancel / Pause / Stop
```

Jika nanti encoder aktif:

```text
Rotate menu mode : Navigasi menu / ubah nilai
Rotate jog Z mode: Z+ / Z-
Press            : OK / Select
BTN khusus: Back
```

Emergency stop:

```text
E-STOP: tombol fisik terpisah, tidak masuk ke mapping menu
```

## Catatan Keamanan

- Perintah dari internet harus divalidasi berdasarkan state mesin.
- Command `start` hanya boleh diterima jika mesin idle dan file sudah dipilih.
- Command jog jarak jauh sebaiknya dinonaktifkan saat spindle aktif atau job sedang berjalan.
- Emergency stop harus tetap hardware dan tidak bergantung pada internet.
- File G-code dari internet wajib melalui validasi nama file, ukuran, dan kelengkapan chunk.
- Untuk versi final, hindari broker publik. Gunakan broker private dengan username, password, dan TLS jika memungkinkan.
