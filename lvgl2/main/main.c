#include <stdio.h>
#include <inttypes.h>
#include "driver/spi_master.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "esp_lcd_ili9488.h"
#define LV_USE_XPT2046 1
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <string.h>
#include "StepperMotor.h"
#define TAG "LCD_LVGL"
#include "Screens/Screens.h"
#include "pins.h"
#include "Thermal.h"
#include "esp_lcd_touch_xpt2046.h"
#include "touch.h"
#include "WiFi.h"
#include "esp_heap_caps.h"


#define UART_NUM UART_NUM_0  // Use UART0 (USB Serial Monitor)
#define BUF_SIZE 1024
#include "driver/ledc.h"
#define LV_USE_XPT2046 1

#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_CMD_BITS 8
#define LCD_PARAM_BITS 8
#define LCD_H_RES 320
#define LCD_V_RES 480
#define LVGL_TICK_PERIOD_MS 2

 wigits_t wigits;
 
 void print_heap_info() {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);

    printf("Free heap: %lu bytes\n", (unsigned long)esp_get_free_heap_size());
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    ESP_LOGI("LVGL", "Used memory: %d bytes, Free: %d bytes", mon.total_size - mon.free_size, mon.free_size);

    ESP_LOGI("MEMORY", "Total Heap Size: %d bytes", info.total_free_bytes + info.total_allocated_bytes);
    ESP_LOGI("MEMORY", "Free Heap: %d bytes", info.total_free_bytes);
    ESP_LOGI("MEMORY", "Min Ever Free Heap: %d bytes", info.minimum_free_bytes);
    ESP_LOGI("MEMORY", "Allocated Heap: %d bytes", info.total_allocated_bytes);
    ESP_LOGI(TAG, "xPortGetFreeHeapSize %d = DRAM", xPortGetFreeHeapSize());

    int DRam = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    int IRam = heap_caps_get_free_size(MALLOC_CAP_32BIT) - heap_caps_get_free_size(MALLOC_CAP_8BIT);
  
    ESP_LOGI(TAG, "DRAM \t\t %d", DRam);
    ESP_LOGI(TAG, "IRam \t\t %d", IRam);
}
static void on_pointer(lv_indev_t *indev, lv_indev_data_t *data)
{
    touch_t touch;
    touch_read_task(&touch);
    data->point.x = touch.x;
    data->point.y = touch.y;
    data->state = touch.is_touched ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

esp_lcd_panel_handle_t *init_display(void)
{
    spi_bus_config_t bus_config = {
        .mosi_io_num = LCD_SDI_MOSI,
        .sclk_io_num = LCD_SCK_SCLK,
        .miso_io_num = TOUCH_MISO,
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
        .rgb_ele_order = COLOR_RGB_ELEMENT_ORDER_BGR, //1       
        .bits_per_pixel = 18,  // ILI9488 uses 18-bit color depth
        // Add other configurations as needed
    };
     esp_lcd_new_panel_ili9488(io_handel, &panel_config,  LCD_H_RES * LCD_V_RES * 2 / 10,  &panel_handle);
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
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    lv_disp_flush_ready(display);
}

static void lvgl_tick(void *arg)
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void app_main(void)
{
    esp_lcd_panel_handle_t *panel_handle = init_display();
    lv_init();
    lv_display_t *lv_display = lv_display_create(LCD_H_RES, LCD_V_RES);

    uint32_t buf_size = LCD_H_RES * LCD_V_RES * 2 / 20; // Decreased this from / 10 - seams to be working ok
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
    
    //screentext = xQueueCreate(10,sizeof(screentext_t));
    setup_stepper();
    CreateScreens();
    switch_screen(screens.splash); 
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(2000));
    switch_screen(screens.home);

    uart_init();
    touch_display_init();
    touch_t touch;
    touch_read_task(&touch);

    lv_indev_t *pointer = lv_indev_create();
    lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(pointer, on_pointer);

    lv_obj_t *icon = lv_label_create(lv_screen_active());
    lv_label_set_text(icon, LV_SYMBOL_UP);
    lv_obj_set_style_text_color(icon, lv_palette_main(LV_PALETTE_PINK), 0);

    lv_indev_set_cursor(pointer, icon);
    
    
    // printf("Free heap: %lu bytes\n", (unsigned long)esp_get_free_heap_size());
    InitWifi();
   

    //print_heap_info();
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(20));
        lv_timer_handler();
    }
}
