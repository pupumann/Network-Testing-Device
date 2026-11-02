# UMBRA V4 - Cyberdeck Multi-Tool

<p align="center">
  <img src="https://raw.githubusercontent.com/pupumann/UMBRA_V4/main/1000041975.jpg" width="32%" alt="UMBRA V4 - Tampak Depan">
  <img src="https://raw.githubusercontent.com/pupumann/UMBRA_V4/main/1000041976.jpg" width="32%" alt="UMBRA V4 - Tampak Samping">
  <img src="https://raw.githubusercontent.com/pupumann/UMBRA_V4/main/1000041974.jpg" width="32%" alt="UMBRA V4 - Tampak Dalam">
</p>

**UMBRA V4** adalah perangkat *Cyberdeck* berbasis ESP32 yang dirancang untuk keperluan riset keamanan jaringan (WiFi), analisis spektrum 2.4GHz, dan uji penetrasi nirkabel. 

## 🚀 Fitur Utama
- **NRF24L01+ Dual Jammer**: Jamming pita 2.4GHz (WiFi, Bluetooth, Zigbee) dengan dua modul NRF24L01+ PA+LNA menggunakan metode *Constant Carrier* dan *Channel Sweep*.
- **WiFi Deauthentication (Deauth)**: Memutus koneksi perangkat dari Access Point (AP) menggunakan injeksi paket 802.11.
- **Deep WiFi Scanner**: Pemindaian AP per-channel yang lebih agresif dan sensitif.
- **Clock & Mission Elapsed Time (MET)**: Menggunakan NTP Server untuk sinkronisasi waktu.


## 🛠️ Hardware yang Digunakan
- **ESP32 DevKit V1** (atau yang setara)
- **TFT ST7789** (320x240) + Touchscreen XPT2046
- **2x NRF24L01 + PA + LNA** (Wajib menggunakan Kapasitor 100µF + 10µF)
- **Antena 2.4GHz** (SMA)

## 🔌 Wiring (Koneksi)

| TFT ST7789 | ESP32 | Touch XPT2046 | ESP32 | NRF #1 | ESP32 | NRF #2 | ESP32 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| VCC | 3.3V | T_CS | GPIO 15 | VCC | 3.3V | VCC | 3.3V |
| GND | GND | T_DO | GPIO 19 | GND | GND | GND | GND |
| CS | GPIO 5 | T_DIN | GPIO 23 | CE | GPIO 27 | CE | GPIO 26 |
| DC | GPIO 22 | T_CLK | GPIO 18 | CSN | GPIO 21 | CSN | GPIO 13 |
| RST | GPIO 4 | T_IRQ | (NC) | SCK | GPIO 18 | SCK | GPIO 18 |
| MOSI | GPIO 23 | | | MISO | GPIO 19 | MISO | GPIO 19 |
| MISO | GPIO 19 | | | MOSI | GPIO 23 | MOSI | GPIO 23 |
| SCK | GPIO 18 | | | IRQ | (NC) | IRQ | (NC) |
| LED | 3.3V | | | | | | |

## ⚙️ Library yang Dibutuhkan
- `TFT_eSPI` (Oleh Bodmer)
- `RF24` (Oleh nRF24)
- `WiFi` (Bawaan ESP32)
- `esp_wifi` (Bawaan ESP32)

## ⚠️ Disclaimer
Proyek ini dibuat **HANYA** untuk tujuan edukasi, riset, dan pengujian keamanan jaringan pribadi. Penggunaan alat ini untuk mengganggu jaringan publik, memutus koneksi orang lain tanpa izin, atau aktivitas ilegal lainnya adalah **tanggung jawab pengguna sepenuhnya** dan melanggar hukum yang berlaku.
