# InfoPaper
 Demonstration of GDEP0565D90 ePaper using ESP32.
- Based on example code provided by SeeedStudio
- Implemented basic SPI based driver using ESP-IDF

# How to use demo applicaton?
- First change the WiFi SSID & Password in esp_wifi_manager.h
- Compile and flash the code
- Download the companion app source code - [InfoPublisher](https://github.com/yashp96/InfoPublisher)

# Use Offline (without WiFi/Mqtt/CompanionApp)
if you don't wan't the hassle of installing Qt6 for companion app, just modify the code as per needs or you can just uncomment the macro `#define SAMPLE_CODE` in `main.c`.
