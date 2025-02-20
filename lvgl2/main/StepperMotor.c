
#include "pins.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "StepperMotor.h"
void setup_stepper() {
    // Configure Direction Pin
    gpio_config_t config = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .pin_bit_mask = (1ULL << Stepper_STEP) | (1ULL << Stepper_DIR)};
    gpio_config(&config);

    gpio_set_direction(Stepper_DIR, GPIO_MODE_OUTPUT);
    gpio_set_direction(Stepper_STEP, GPIO_MODE_OUTPUT);
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