
//https://randomnerdtutorials.com/esp32-tft-touchscreen-display-2-8-ili9341-arduino/#config-file-windows-pc
#include <lvgl.h>
#include <TFT_eSPI.h>  // For the display driver, assuming you're using a TFT display with SPI
#include <XPT2046_Touchscreen.h>


// Initialize TFT display and touch
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */


// Touchscreen pins
// #define XPT2046_IRQ 36   // T_IRQ
// #define XPT2046_MOSI 32  // T_DIN
// #define XPT2046_MISO 39  // T_OUT
// #define XPT2046_CLK 25   // T_CLK
// #define XPT2046_CS 33    // T_CS
#define XPT2046_IRQ 27   // T_IRQ
#define XPT2046_MOSI 23  // T_DIN
#define XPT2046_MISO 19  // T_OUT
#define XPT2046_CLK 18   // T_CLK
#define XPT2046_CS 17    // T_CS

#define XPT2046_IRQ 27   // T_IRQ
#define XPT2046_MOSI 23  // T_DIN
#define XPT2046_MISO 19  // T_OUT
#define XPT2046_CLK 18   // T_CLK
#define XPT2046_CS 17    // T_CS


SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ 480 * 10 ];

// Function to print "hello" when the button is clicked
void button_event_handler(lv_event_t * e) {
    Serial.println("hello");
    delay(1000);
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint16_t c;

    tft.startWrite(); /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            c = color_p->full;
            tft.pushColor(c);
            color_p++;
        }
    }
    tft.endWrite(); /* terminate TFT transaction */

    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

void setup() {
    // Initialize Serial
    Serial.begin(115200);

    // Initialize TFT
    tft.begin();        /* TFT init */
    tft.setRotation(1); /* Landscape orientation */

    // Initialize LVGL
    lv_init();

    // Setup display buffer
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 480 * 10);

    // Setup display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Create a simple label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello, LVGL!");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);  // Align the label near the top

    // Create a button
    lv_obj_t *btn = lv_btn_create(lv_scr_act());    // Create a button on the screen
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);       // Center the button

    // Create a label for the button
    lv_obj_t *btn_label = lv_label_create(btn);     // Add a label to the button
    lv_label_set_text(btn_label, "Press Me");       // Set the button text

    // Assign the event handler to the button
    lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_CLICKED, NULL);
}

void loop() {
  Serial.println("loop");
    lv_timer_handler(); // Let LVGL handle tasks
    delay(5);
}
