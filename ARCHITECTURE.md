# Architecture

Dokumen ini menjelaskan arsitektur hardware dan software firmware ESP32-S3 CNC Interface.

## Hardware Architecture

```text
                      +---------------------------+
                      | ESP32-S3 DevKitM-1        |
                      | AYB Interface Firmware    |
                      +-------------+-------------+
                                    |
          +-------------------------+-------------------------+
          |                         |                         |
          v                         v                         v
   +--------------+          +--------------+          +---------------+
   | User         |          | Storage      |          | Network       |
   | Interface    |          | MicroSD Card |          | WiFi + MQTT   |
   +------+-------+          +--------------+          +-------+-------+
          |                                                   |
          |                                                   v
          |                                           +---------------+
          |                                           | Mosquitto /   |
          |                                           | MQTT Explorer |
          |                                           +---------------+
          |
          v
   +----------------+
   | LCD 128x64     |
   | Rotary Encoder |
   | Push Button    |
   | Passive Buzzer |
   +----------------+

                                    |
                                    | UART Serial1
                                    v
                      +---------------------------+
                      | BTT SKR V1.4 Turbo        |
                      | Marlin Firmware           |
                      +-------------+-------------+
                                    |
                                    v
                      +---------------------------+
                      | DM542 Stepper Driver      |
                      +-------------+-------------+
                                    |
                                    v
                      +---------------------------+
                      | NEMA23 Stepper Motor      |
                      +---------------------------+
```

## Software Modules

```text
src/
  app/
    AppController.h/.cpp     -> orkestrator utama UI, input, output, storage, network, dan Marlin
  assets/
    Symbols.h/.cpp           -> aset bitmap/simbol LCD
  config/
    AppConfig.h              -> konfigurasi fitur, Marlin, UI, dan parameter mesin
    MqttConfig.h             -> broker, autentikasi, topic, dan interval MQTT
    PinConfig.h              -> mapping pin hardware
  display/
    LcdHandler.h/.cpp        -> render LCD 128x64
    Menu.h/.cpp              -> wrapper tampilan menu/standby
  input/
    ButtonHandler.h/.cpp     -> pembacaan push button
    EncoderHandler.h/.cpp    -> pembacaan rotary encoder
  network/
    WifiPortal.h/.cpp        -> WiFiManager dan parameter MQTT
    CloudMqtt.h/.cpp         -> MQTT monitoring lokal
  output/
    BuzzerHandler.h/.cpp     -> pola PWM buzzer non-blocking dan prioritas alarm
  storage/
    SdCardReader.h/.cpp      -> pembacaan SD card dan file job
  time/
    TimeManager.h/.cpp       -> sinkronisasi NTP dan waktu lokal WIB
```

## Main Runtime Flow

```text
main.cpp::setup()
   |
   +-- app.beginSerial()
   +-- app.beginDisplay()
   +-- app.beginInput() [jika toggle aktif]
   +-- app.beginBuzzer() [jika toggle aktif]
   +-- app.beginMarlinConnection() [jika toggle aktif]
   +-- app.beginStorage() [jika toggle aktif]
   +-- app.beginNetwork() [jika toggle aktif]

loop()
   |
   v
AppController::update()
   |
   +-- updateInput()
   +-- updateAboutScreen()
   +-- BuzzerHandler::update()
   +-- updateMarlinCommunication()
   +-- updateNetwork()
   +-- updateStorageFileDisplay()
```

## Marlin Integration

Komunikasi Marlin dikendalikan oleh `CNC_ENABLE_MARLIN_CONNECTION` di `main.cpp`. `AppController` membedakan kondisi `OFF`, `WAITING`, `DISCONNECTED`, `CONNECTED`, `LOST`, dan `ERROR`. Hanya status `CONNECTED` yang mengizinkan pengiriman perintah mesin; `LOST` dan `ERROR` mengaktifkan alarm.

ESP32-S3 berkomunikasi dengan SKR V1.4 Turbo melalui UART `Serial1`.

Fungsi utama:

- Mengirim `M114` secara periodik.
- Parse respons posisi `X`, `Y`, dan `Z`.
- Menggunakan posisi Marlin untuk standby screen, Set Origin, dan MQTT `cnc/position`.
- Menandai posisi tidak valid sampai respons `M114` diterima dan menampilkan `?` pada LCD/MQTT.
- Mengirim alarm MQTT jika koneksi yang sebelumnya aktif menjadi `LOST` atau Marlin mengirim `ERROR`.

## MQTT Lokal

Topic publish utama:

- `cnc/status`
- `cnc/progress`
- `cnc/error`
- `cnc/position`
- `cnc/machine`

Topic subscribe receive-only:

- `cnc/command`
- `cnc/gcode`

Topic monitoring kompatibilitas:

- `cnc/network`
- `cnc/time`
- `cnc/alarm`

Payload command dan G-code hanya dicetak ke Serial Monitor. Eksekusi ke UART Marlin dan upload file belum diaktifkan.

## UI Workflow

UI LCD dibagi menjadi:

- Boot screen
- Standby screen
- Main menu
- Select Job
- Set Origin
- Machine Ctrl&Status
- Network
- About dengan logo tetap dan teks identitas bergulir
- Dialog konfirmasi

Semua alur UI dikoordinasikan oleh `AppController`.
