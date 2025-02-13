#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
//#include "lvgl.h"

#define Stepper_DIR 12
#define Stepper_STEP 14

void setup_stepper() {
    // Configure Direction Pin
    gpio_set_direction(Stepper_DIR, GPIO_MODE_OUTPUT);
    
    // Configure LEDC PWM for Step Pulse Generation
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_1_BIT,  // 1-bit resolution (on/off)
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,  // Initial frequency (1000 Hz)
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = Stepper_STEP,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 1,  // Set duty cycle to ON (1-bit resolution)
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
}

void set_stepper_speed(int frequency) {
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, frequency);
}


void app_main(void)
{
    gpio_config_t config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .pin_bit_mask = (1ULL << Stepper_STEP) | (1ULL << Stepper_DIR)};
    gpio_config(&config);
 
  gpio_set_direction(Stepper_STEP, GPIO_MODE_OUTPUT);
  gpio_set_direction(Stepper_DIR, GPIO_MODE_OUTPUT);
     //xTaskCreate(&task1, "temperature reading", 2048, "task 1", 19, NULL);
    setup_stepper();
    while (1){
        printf("Moving stepper forward...\n");
        gpio_set_level(Stepper_DIR, 1);  // Set direction to forward
        set_stepper_speed(4000);  // Set step pulse frequency to 6000 Hz
        vTaskDelay(pdMS_TO_TICKS(2000));  // Run for 2s
    }
}
