/***************************
 * Modified from HelloWorld example of Arduino_GFX
 * run on Xiao ESP32C3 + 480x320 ILI9488 SPI TFT
 #define TFT_BL 32 // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH // Level to turn ON back-light (HIGH or LOW)
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS 15 // Chip select control pin
#define TFT_DC 2 // Data Command control pin
#define TFT_RST 4 // Reset pin (could connect to RST pin)s
 **************************/
#include <Arduino_GFX_Library.h>

#define Stepper_DIR 12
#define Stepper_STEP 14
#define speed 100

#define ILI9488_DRIVER // WARNING: Do not connect ILI9488 display SDO to MISO if other devices share the SPI bus (TFT SDO does NOT tristate when CS is high)
#define TFT_BL 32 // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH // Level to turn ON back-light (HIGH or LOW)
#define TFT_MISO -1
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS 5 // Chip select control pin
#define TFT_DC 4 // Data Command control pin
#define TFT_RST 2 // Reset pin (could connect to RST pin)
#define TOUCH_CS 21 // Chip select pin (T_CS) of touch screen
#define LOAD_GLCD // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2 // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4 // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6 // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7 // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8 // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
#define SMOOTH_FONT
//define SPI_FREQUENCY 27000000
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 16000000
//#define SPI_READ_FREQUENCY 20000000
#define SPI_TOUCH_FREQUENCY 2500000

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);

/* More display class: https://github.com/moononournation/Arduino_GFX/wiki/Display-Class */
//Arduino_GFX gfx = new Arduino_ILI9488_18bit(bus, TFT_RST, 3 / rotation /, false  IPS );

Arduino_GFX *gfx = new Arduino_ILI9488_18bit(bus, TFT_RST, 3 /* rotation */, false /* IPS */);


/***************************
 * End of Arduino_GFX setting
 **************************/

void setup(void)
{
    gfx->begin();
    gfx->fillScreen(BLACK);

#ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2, 2, 2);

    gfx->setCursor(10, 10);
    gfx->println("XIAO ESP32C3 (in Arduino framework)");

    gfx->setCursor(10, 30);
    gfx->println("+ ILI9488 SPI TFT");

    gfx->setCursor(10, 50);
    gfx->println("using Arduino_GFX Library");

    int w = gfx->width();
    int h = gfx->height();

    gfx->setCursor(10, 70);
    gfx->printf("%i x %d", w, h);
    gfx->drawRect(0, 0, w, h, WHITE);

    delay(3000);

    for(int i=0; i<w; i++){
      int d = (int)(255 * i/w);
      gfx->drawLine(i, 0, i, w, RGB565(d, 0, 0));
      delay(10);
    }
    for(int i=0; i<w; i++){
      int d = (int)(255 * i/w);
      gfx->drawLine(w-i, 0, w-i, w, RGB565(0, d, 0));
      delay(10);
    }
    for(int i=0; i<w; i++){
      int d = (int)(255 * i/w);
      gfx->drawLine(i, 0, i, w, RGB565(0, 0, d));
      delay(10);
    }

      pinMode(Stepper_STEP, OUTPUT);  // Set the pin as an output
  pinMode(Stepper_DIR, OUTPUT);  // Set the pin as an output
  digitalWrite(Stepper_DIR, LOW); 

  Serial.begin(115200);
  Serial.println("starting");

}

void loop()
{
  
    for (int i = 0; i < 10000; i++) { // Toggle the pin 10 times
    digitalWrite(Stepper_STEP, HIGH); // Turn the pin on
    delayMicroseconds(speed);                     // Wait for 500 milliseconds
    digitalWrite(Stepper_STEP, LOW);  // Turn the pin off
    delayMicroseconds(speed);  
    }
    
    gfx->setTextColor(WHITE);
    gfx->setTextSize(6, 6, 2);

    gfx->fillScreen(RED);
    gfx->setCursor(100, 100);
    gfx->printf("CLIPPITS");
    delay(2000);
        for (int i = 0; i < 10000; i++) { // Toggle dthe pin 10 times
    digitalWrite(Stepper_STEP, HIGH); // Turn the pin on
    delayMicroseconds(speed);                     // Wait for 500 milliseconds
    digitalWrite(Stepper_STEP, LOW);  // Turn the pin off
    delayMicroseconds(speed);  
    }

    gfx->fillScreen(GREEN);
    gfx->setCursor(100, 100);
    gfx->printf("CLIPPITS");
    delay(2000);
        for (int i = 0; i < 10000; i++) { // Toggle the pin 10 times
    digitalWrite(Stepper_STEP, HIGH); // Turn the pin on
    delayMicroseconds(speed);                     // Wait for 500 milliseconds
    digitalWrite(Stepper_STEP, LOW);  // Turn the pin off
    delayMicroseconds(speed);  
    }

    gfx->fillScreen(BLUE);
    gfx->setCursor(100, 100);
    gfx->printf("CLIPPITS");
    delay(2000);
}