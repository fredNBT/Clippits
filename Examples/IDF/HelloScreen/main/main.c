#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <ILI9488.h>

// SPI Pins for ILI9488
#define TFT_CS   5
#define TFT_RST  18
#define TFT_DC   19
#define TFT_MOSI 23
#define TFT_SCK  18

// Create Display Object
ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);

extern "C" void app_main() {
    // Initialize Arduino Core
    initArduino();
    
    Serial.begin(115200);
    tft.begin(); 
    tft.fillScreen(ILI9488_BLACK);
    tft.setTextColor(ILI9488_WHITE);
    tft.setTextSize(2);
    tft.setCursor(50, 100);
    tft.print("Hello ESP-IDF + Arduino!");
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
