#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "lvgl.h"
#include "esp_lcd_ili9488.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_touch_xpt2046.h"

static esp_lcd_touch_handle_t tp;

typedef struct touch_t
{   
    uint16_t x;
    uint16_t y;
    bool is_touched;
} touch_t;

void touch_display_init(void);
void touch_read_task(touch_t *touch);

#endif