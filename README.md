# ESP32-CYD Multi-Function Display

example project for ESP32-CYD (Cheap Yellow Display) featuring:
- **Market Dashboard**: Real-time BTC (Binance) & NASDAQ (Yahoo).
- **Stock Monitor**: Taiwan Stock Index (mock/yahoo).
- **Weather Station**: OpenWeatherMap.
- **Photo Album**: Slide show from SD card.
- **Settings**: WiFi Manager with on-screen keyboard.

## Hardware Required
- ESP32-CYD (ESP32-2432S028)
- MicroSD Card (formatted FAT32) with `.jpg` images in root.

## Setup
1. **API Keys**: Edit `src/Config.h` for OpenWeatherMap API Key.
2. **WiFi**: No need to edit code! Use the Settings page (Page 5) on the device to scan and connect.
3. **Libraries**: PlatformIO will automatically install `TFT_eSPI`, `TFT_Touch`, `ArduinoJson`, `TJpg_Decoder`.
4. **Upload**: Connect via USB and upload using PlatformIO.

## Features
- **Top Bar**: Touch the top blue bar to switch pages (1-5).
- **Page 1 (Market)**: Shows NASDAQ and BTC prices.
- **Page 2 (Stock)**: Shows TWSE Index.
- **Page 3 (Weather)**: Current weather for configured city.
- **Page 4 (Album)**: Slideshow of images from SD card.
- **Page 5 (Settings)**: WiFi Scanner & Password Input.

## Notes
- **APIs**: The project uses public APIs (Yahoo Finance, Binance). Use responsibly. 
- **SD Card**: Ensure images are < 320x240 or they will be cropped/scaled. 
