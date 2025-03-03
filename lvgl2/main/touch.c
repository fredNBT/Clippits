#include "touch.h"
#include "pins.h"
#include "esp_lcd_touch_xpt2046.h"

void touch_init() {
    printf("🟢 Initializing SPI bus...\n");

    printf("✅ SPI bus initialized!\n");

    // Configure SPI panel I/O
    esp_lcd_panel_io_handle_t touch_io;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = TOUCH_CS,
        .dc_gpio_num = -1,
        .spi_mode = 0,
        .pclk_hz = 500 * 1000,  // Lower speed for stability
        .trans_queue_depth = 10,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {
            .dc_low_on_data = 0,
            .cs_high_active = 0
        }
    };

    printf("🟢 Creating SPI Panel I/O...\n");
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &touch_io));
    printf("✅ SPI Panel I/O created!\n");
    // Touch configuration
    esp_lcd_touch_config_t touch_config = {
        .x_max = 320,
        .y_max = 480,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .flags = {
            .swap_xy = 1,  // Change if X/Y are swapped
            .mirror_x = 0, // Change if X-axis is flipped
            .mirror_y = 0  // Change if Y-axis is flipped
        },
        .user_data = NULL
    };
    printf("🟢 Initializing XPT2046 touch...\n");
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(touch_io, &touch_config, &tp));
    printf("✅ Touch initialized successfully!\n");
}

void touch_read_task(touch_t *touch) {
    uint16_t touch_x, touch_y;
    uint8_t point_num = 0;

        esp_lcd_touch_read_data(tp);

        if (esp_lcd_touch_get_coordinates(tp, &touch_x, &touch_y, NULL, &point_num, 1)) {
            touch->is_touched = 1;
            touch->y = touch_x;
            touch->x = abs(320 - touch_y);
        }
}