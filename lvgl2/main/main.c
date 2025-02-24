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
#include "lv_drivers/indev/xpt2046.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "BasicImage.h"
#include <string.h>
#include "StepperMotor.h"
#define TAG "LCD_LVGL"
#include "process_x.h"
#include "Screens/Screens.h"
#include "pins.h"
#include "Thermal.h"
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
static QueueHandle_t screentext;
wigits_t wigits;
lv_timer_t* timer;

void update_label(void *param) {
    screentext_t *data = (screentext_t *)param;
    const char *current_text = lv_label_get_text(wigits.label);
    char new_text[128];
    snprintf(new_text, sizeof(new_text), "%s%c", current_text, data->value);
    lv_label_set_text(wigits.label, new_text);  // Update label
    free(data);
}
void clear_label(void *param) {
    lv_label_set_text(wigits.label, "");  // Update label
}

void queue_listener_task(void *pvParameter) {
    screentext_t received_text;  // Buffer to store received message

    while (1) {
        // Wait indefinitely for data to arrive
        if (xQueueReceive(screentext, &received_text, portMAX_DELAY) == pdTRUE) {
            printf("Received from queue: %c\n", received_text.value);
            screentext_t *data = malloc(sizeof(screentext_t));
            memcpy(data, &received_text, sizeof(screentext_t));
            lv_async_call(update_label, data);  // Schedule label update
        }
    }
}

static int time = 0;
void on_timer(lv_timer_t* timer)
{
    time++;
    lv_bar_set_value(wigits.ProgressBar,time,false);
}

char rx_buffer[BUF_SIZE];
void uart_task(void *arg) {
    uint8_t data[BUF_SIZE];
    
    while (1) {
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, pdMS_TO_TICKS(200));

        data[len] = '\0';  // Null-terminate the string
            if (len > 0){
                screentext_t receive_payload = {};
                receive_payload.value = data[0];
                xQueueSend(screentext,&receive_payload,1000);
            if (data[0] == '1') 
            {
                const char *textToPrint = lv_label_get_text(wigits.label);
                print_text("\n\n");
                print_text(textToPrint);
                print_text("\n\n");
                lv_async_call(clear_label, NULL);
                lv_timer_resume(timer);
                  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 1); // Set duty cycle
                  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0); // Apply change
                  printf("Moter Baby Motor\n");
                  printf("Moving stepper forward...\n");
                  gpio_set_level(Stepper_DIR, 1);  // Set direction to forward
                  set_stepper_speed(4000);  // Set step pulse frequency to 6000 Hz
                  vTaskDelay(pdMS_TO_TICKS(10000));  // Run for 2s
                  ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0); 
                  lv_timer_pause(timer);
                  lv_bar_set_value(wigits.ProgressBar,0,false);
                  time = 0;
                  

            }else if(data[0] == 'n' || data[0] == 'n' )
            {
                  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 1); // Set duty cycle
                  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0); // Apply change
                  printf("Nurge\n");
                  printf("Moving stepper forward...\n");
                  gpio_set_level(Stepper_DIR, 1);  // Set direction to forward
                  set_stepper_speed(4000);  // Set step pulse frequency to 6000 Hz
                  vTaskDelay(pdMS_TO_TICKS(100));  // Run for 2s
                  ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0); 
                }
            }
        }
    }

esp_lcd_panel_handle_t *init_display(void)
{
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
    
    screentext = xQueueCreate(10,sizeof(screentext_t));
    setup_stepper();

    uart_driver_install(UART_NUM, BUF_SIZE, 0, 0, NULL, 0);  // Initialize UART
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 5, NULL);
    xTaskCreate(&queue_listener_task, "Queue Listener", 4096, NULL, 5, NULL);

    timer = lv_timer_create(on_timer, 100, NULL);
    lv_timer_pause(timer);

    CreateScreens();
    switch_screen(screens.splash); 
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(2000));
    switch_screen(screens.home);

    uart_init();
    

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
