#ifndef PINS_H
#define PINS_H

// Display Pins (Example for ILI9488)
#define Stepper_DIR 12
#define Stepper_STEP 14
#define LCD_SCK_SCLK GPIO_NUM_18
#define LCD_SDI_MOSI GPIO_NUM_23
#define LCD_SDO_MISO -1
#define LCD_RS_DC GPIO_NUM_4
#define LCD_RST GPIO_NUM_2
#define LCD_CS GPIO_NUM_5

#define TOUCH_CS     33   // XPT2046 CS
#define TOUCH_MISO   26  // XPT2046 MISO
#define TOUCH_MOSI   23  // XPT2046 MOSI
#define TOUCH_CLK    18  // XPT2046 SCK



#endif // PINS_H
