
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_task_wdt.h"  // For Task Watchdog Timer
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#define TOUCH_HRES  480
#define TOUCH_VRES  320

#define TOUCH_CS     33   // XPT2046 CS
#define TOUCH_MISO   26  // XPT2046 MISO
#define TOUCH_MOSI   23  // XPT2046 MOSI
#define TOUCH_CLK    18  // XPT2046 SCK




// Touch handle
esp_lcd_touch_handle_t tp;

// Function prototypes
void touch_init();
void touch_read_task(void *pvParameter);

void app_main(void) {
    printf("🚀 Starting ESP32 Touchscreen...\n");

    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    //gpio_set_direction(11, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 1);  // Ensure display CS is HIGH (inactive)
    //gpio_set_level(11, 1); // Ensure touch CS is HIGH (inactive)
printf("🚀 Starting touch init...\n");

    // Initialize touch
    touch_init();
    // Create a task to read touch data
    xTaskCreate(touch_read_task, "touch_task", 4096, NULL, 5, NULL);
}

// 📌 Touch Initialization
void touch_init() {
    printf("🟢 Initializing SPI bus...\n");

    spi_bus_config_t buscfg = {
        .sclk_io_num = TOUCH_CLK,
        .mosi_io_num = TOUCH_MOSI,
        .miso_io_num = TOUCH_MISO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
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
        .x_max = 480,
        .y_max = 320,
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

// 📌 Task to Read Touch Data
void touch_read_task(void *pvParameter) {
    uint16_t touch_x, touch_y;
    uint8_t point_num = 0;

    while (1) {
        // Read touch data
        esp_lcd_touch_read_data(tp);

        if (esp_lcd_touch_get_coordinates(tp, &touch_x, &touch_y, NULL, &point_num, 1)) {
            if (point_num > 0) {
                printf("🖐 Touch detected at: X=%d, Y=%d\n", touch_x, touch_y);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // Read every 100ms
    }
}
