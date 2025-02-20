/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "lvgl.h"
#include "esp_lcd_ili9488.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "calc/calc.h"
#include "BasicImage.h"
//#include "ui/ui.h"
#define TAG "LCD_LVGL"


// #define TFT_MISO -1
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS 5 // Chip select control pin
// #define TFT_DC 4 // Data Command control pin
// #define TFT_RST 2 // Reset pin (could connect to RST pin)


#define LCD_SCK_SCLK GPIO_NUM_18
#define LCD_SDI_MOSI GPIO_NUM_23
#define LCD_SDO_MISO -1
#define LCD_RS_DC GPIO_NUM_4
#define LCD_RST GPIO_NUM_2
#define LCD_CS GPIO_NUM_5
//#define LCD_LED GPIO_NUM_10

#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8

#define LCD_H_RES 320
#define LCD_V_RES 480
#define LVGL_TICK_PERIOD_MS 2

esp_lcd_panel_handle_t *init_display(void)
{
    // gpio_set_direction(LCD_LED, GPIO_MODE_OUTPUT);
    // gpio_set_level(LCD_LED, 0);

    spi_bus_config_t bus_config = {
        .mosi_io_num = LCD_SDI_MOSI,
        .miso_io_num = LCD_SDO_MISO,
        .sclk_io_num = LCD_SCK_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io_handel = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_CS,
        .dc_gpio_num = LCD_RS_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handel));

    static esp_lcd_panel_handle_t panel_handle = NULL;

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,
        //.rgb_ele_order = COLOR_RGB_ELEMENT_ORDER_RGB, //0
        .rgb_ele_order = COLOR_RGB_ELEMENT_ORDER_BGR, //1

        //.color_space = ESP_LCD_COLOR_SPACE_RGB, //0
        //.color_space = ESP_LCD_COLOR_SPACE_BGR,   //1

        //.rgb_endian = LCD_RGB_ELEMENT_ORDER_BGR, //0
        //.rgb_endian = LCD_RGB_ELEMENT_ORDER_RGB, // 1
        
        .bits_per_pixel = 18,  // ILI9488 uses 18-bit color depth
        // Add other configurations as needed
    };
    //esp_lcd_color_conv_config_t
    
    // esp_lcd_panel_dev_config_t panel_config = {
    //     .reset_gpio_num = LCD_RST,
    //     .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
    //     .bits_per_pixel = 18,
    // };
    //ESP_ERROR_CHECK(esp_lcd_new_panel_ili9488(io_handel, &panel_config, &panel_handle));
    //esp_lcd_new_panel_ili9488
    //esp_lcd_new
    //esp_lcd_new_panel_ili9488(io_handel, &panel_config, &panel_handle);
     esp_lcd_new_panel_ili9488(io_handel, &panel_config,  LCD_H_RES * LCD_V_RES * 2 / 10,  &panel_handle);
    //ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handel, &panel_config, &panel_handle));
    //ESP_ERROR_CHECK(esp_lcd_new_panel_(io_handel, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));


    // correct for our screen
    esp_lcd_panel_invert_color(panel_handle, false);
    esp_lcd_panel_swap_xy(panel_handle, false);
    esp_lcd_panel_mirror(panel_handle, true, true);

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    return &panel_handle;
}

void my_flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t *panel_handle_ptr = (esp_lcd_panel_handle_t *)lv_display_get_user_data(display);
    esp_lcd_panel_handle_t panel_handle = *panel_handle_ptr;

    //lv_draw_sw_rgb565_swap(px_map, lv_area_get_size(area));
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    lv_disp_flush_ready(display);
}

static void lvgl_tick(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void app_main(void)
{

    printf("Hello world!\n");

    esp_lcd_panel_handle_t *panel_handle = init_display();

    printf("Hello init!\n");

    lv_init();
    lv_display_t *lv_display = lv_display_create(LCD_H_RES, LCD_V_RES);

    uint32_t buf_size = LCD_H_RES * LCD_V_RES * 2 / 10;
    lv_color_t *buf1 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
    assert(buf2);

    lv_display_set_buffers(lv_display, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_resolution(lv_display, LCD_H_RES, LCD_V_RES);
    lv_display_set_user_data(lv_display, panel_handle);
    lv_display_set_flush_cb(lv_display, my_flush_cb);

    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .name = "lvgl_tick",
        .callback = lvgl_tick
    };

    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000);

    // lv_obj_t *lbl = lv_label_create(lv_screen_active());
    // lv_label_set_text(lbl,"steven loves pooo");
    // lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    // lv_obj_center(lbl);
    // //Create a new style


    // lv_obj_t* kb = lv_keyboard_create(lv_screen_active());

    draw_image();

    lv_obj_t* bar = lv_bar_create(lv_screen_active());  
    lv_bar_set_value(bar, 50, LV_ANIM_OFF);
    lv_obj_set_pos(bar, 10, 420);
    lv_obj_t* kb = lv_keyboard_create(lv_screen_active());
     //lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);

   // lv_obj_center(bar);
    
   // void ui_init();


// 3. Center the label within its parent (the active screen)

//     lv_obj_t *rect = lv_obj_create(lv_screen_active());  // Create a fullscreen object
// lv_obj_set_size(rect, 480, 320);  // Set it to cover the entire screen
// lv_obj_set_style_bg_color(rect, lv_color_make(255, 0, 255), LV_PART_MAIN);
// lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0);  // Align to center

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
