// Counts each time a magnet passes the ES3144EUA Hall sensor on HALL_PIN.
//
// Wiring: VDD -> 5 V, GND -> GND, OUT -> HALL_PIN with a 10 kOhm pull-up to 3.3 V.
// GPIO36 has no internal pull-up, so the external resistor is required.

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "HallSensor.h"
#include "pins.h"

static const char *TAG = "hall";

// Ignores any further edges for this long after a counted pass.
// Lower it if a magnet passes faster than this.
#define LOCKOUT_US 2000

static volatile uint32_t s_pass_count = 0;

// OUT falls from HIGH to LOW as the magnet arrives.
static void IRAM_ATTR hall_isr(void *arg)
{
    static int64_t last_pass_us = -LOCKOUT_US;

    // A real detection is still LOW here. The brief low glitches GPIO36 can
    // produce while Wi-Fi modem sleep is active are normally over by now.
    if (gpio_get_level(HALL_PIN) != 0) {
        return;
    }

    int64_t now_us = esp_timer_get_time();
    if (now_us - last_pass_us >= LOCKOUT_US) {
        last_pass_us = now_us;
        s_pass_count = s_pass_count + 1;
    }
}

static void hall_print_task(void *arg)
{
    uint32_t last_printed_count = 0;

    while (true) {
        uint32_t current_count = s_pass_count;

        if (current_count != last_printed_count) {
            last_printed_count = current_count;
            ESP_LOGI(TAG, "Magnet passes: %" PRIu32, current_count);
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void hall_sensor_init(void)
{
    const gpio_config_t hall_config = {
        .pin_bit_mask = 1ULL << HALL_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&hall_config));

    // Fine if another module already installed the ISR service.
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }
    ESP_ERROR_CHECK(gpio_isr_handler_add(HALL_PIN, hall_isr, NULL));

    BaseType_t created = xTaskCreate(hall_print_task, "hall_print", 3072, NULL, 1, NULL);
    configASSERT(created == pdPASS);
}
