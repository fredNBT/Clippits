#include "pins.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "StepperMotor.h"
#include "Screens/Screens.h"
#include "Thermal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char *TAG = "stepper";

#define LEDC_MODE           LEDC_HIGH_SPEED_MODE

// Barrel stepper uses TIMER_0 / CHANNEL_0
// Close stepper uses TIMER_1 / CHANNEL_1 (separate timer so speeds are independent)
#define BARREL_LEDC_TIMER   LEDC_TIMER_0
#define BARREL_LEDC_CHANNEL LEDC_CHANNEL_0
#define CLOSE_LEDC_TIMER    LEDC_TIMER_1
#define CLOSE_LEDC_CHANNEL  LEDC_CHANNEL_1

// 10-bit resolution on the 80 MHz APB clock gives a usable step range of
// roughly 80 Hz .. 78 kHz. With 1-bit resolution the timer cannot go much
// below ~500 Hz, which makes a slow start (ramp) impossible.
#define STEP_DUTY_RES       LEDC_TIMER_10_BIT
#define STEP_DUTY_50        512             // 50% duty at 10-bit

// ---------- Ramp settings (tune these) ----------
#define RAMP_START_HZ       200   // start/stop speed: slow enough to never stall from standstill
#define RAMP_INTERVAL_MS    10    // keep >= one FreeRTOS tick (10 ms at the default 100 Hz tick)
#define RAMP_STEP_HZ        40    // added every interval -> 4000 Hz/s acceleration

#define BARREL_RUN_HZ       1000
#define CLOSE_RUN_HZ        1500

// ---------- Endstop settings ----------
#define CLOSE_ENDSTOP_DIR   0     // direction that moves the close motor towards CloseSensor
#define CLOSE_ENDSTOP_LEVEL 1     // CloseSensor reads HIGH at the end position

typedef struct {
    ledc_timer_t      timer;
    ledc_channel_t    channel;
    gpio_num_t        dir_pin;
    SemaphoreHandle_t lock;            // stops two tasks driving the same motor at once

    // Optional endstop (endstop_pin = GPIO_NUM_NC for none)
    gpio_num_t        endstop_pin;
    int               endstop_dir;     // only moves in this direction are stopped/blocked
    int               endstop_level;   // pin level meaning "at end position"
    SemaphoreHandle_t endstop_sem;     // given by the ISR when the switch trips
    volatile bool     endstop_armed;   // ISR only acts while a guarded move is running
} stepper_t;

static stepper_t barrel = { BARREL_LEDC_TIMER, BARREL_LEDC_CHANNEL, (gpio_num_t)Stepper_DIR, NULL,
                            GPIO_NUM_NC, 0, 0, NULL, false };
static stepper_t closer = { CLOSE_LEDC_TIMER,  CLOSE_LEDC_CHANNEL,  (gpio_num_t)CloseStepper_DIR, NULL,
                            (gpio_num_t)CloseSensor, CLOSE_ENDSTOP_DIR, CLOSE_ENDSTOP_LEVEL, NULL, false };

static void setup_channel(stepper_t *m, int step_pin) {
    ledc_timer_config_t t = {
        .speed_mode      = LEDC_MODE,
        .duty_resolution = STEP_DUTY_RES,
        .timer_num       = m->timer,
        .freq_hz         = RAMP_START_HZ,
        .clk_cfg         = LEDC_USE_APB_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&t));

    ledc_channel_config_t c = {
        .gpio_num   = step_pin,
        .speed_mode = LEDC_MODE,
        .channel    = m->channel,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = m->timer,
        .duty       = 0,        // no pulses until a move starts
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    ledc_stop(LEDC_MODE, m->channel, 0);

    m->lock = xSemaphoreCreateMutex();
}

// ---------- Endstop ----------

// ISR: runs in interrupt context, so it only signals the motor task.
// The LEDC driver calls are not ISR-safe, so the actual stop happens in the task,
// which wakes immediately (tens of microseconds, i.e. at most about one extra step).
static void IRAM_ATTR endstop_isr(void *arg) {
    stepper_t *m = (stepper_t *)arg;
    if (!m->endstop_armed) {
        return;                     // not closing: ignore (e.g. switch releasing as we move away)
    }
    m->endstop_armed = false;       // ignore any further contact bounce
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(m->endstop_sem, &woken);
    if (woken) {
        portYIELD_FROM_ISR();
    }
}

static void setup_endstop(stepper_t *m) {
    if (m->endstop_pin == GPIO_NUM_NC) {
        return;
    }

    gpio_config_t io = {
        .pin_bit_mask = 1ULL << m->endstop_pin,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,    // GPIO34 has no internal pulls:
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // the switch needs an external pull-down
        .intr_type    = m->endstop_level ? GPIO_INTR_POSEDGE : GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&io));

    m->endstop_sem = xSemaphoreCreateBinary();

    // Fine if another module already installed the ISR service.
    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }
    ESP_ERROR_CHECK(gpio_isr_handler_add(m->endstop_pin, endstop_isr, m));
}

static bool at_endstop(stepper_t *m) {
    return m->endstop_pin != GPIO_NUM_NC &&
           gpio_get_level(m->endstop_pin) == m->endstop_level;
}

// Waits ms milliseconds, but returns early (true) if the endstop trips.
static bool wait_or_endstop(stepper_t *m, uint32_t ms) {
    if (m->endstop_sem == NULL) {
        vTaskDelay(pdMS_TO_TICKS(ms));
        return false;
    }
    return xSemaphoreTake(m->endstop_sem, pdMS_TO_TICKS(ms)) == pdTRUE;
}

static void set_freq(stepper_t *m, uint32_t hz) {
    esp_err_t err = ledc_set_freq(LEDC_MODE, m->timer, hz);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "ledc_set_freq(%lu Hz) failed: %s", (unsigned long)hz, esp_err_to_name(err));
    }
}

// Ramp up -> cruise -> ramp down. run_ms is the time until deceleration starts.
// Returns false if the move was blocked or cut short by the endstop.
static bool stepper_move(stepper_t *m, bool dir, uint32_t target_hz, uint32_t run_ms) {
    if (m->lock == NULL) {
        ESP_LOGE(TAG, "setup_stepper() not called");
        return false;
    }
    xSemaphoreTake(m->lock, portMAX_DELAY);

    // 0. Endstop guard: arm the ISR BEFORE checking the pin, so a switch that
    //    closes between the check and the first step is still caught.
    bool guarded = (m->endstop_sem != NULL) && ((int)dir == m->endstop_dir);
    if (guarded) {
        xSemaphoreTake(m->endstop_sem, 0);   // clear any stale signal
        m->endstop_armed = true;
        if (at_endstop(m)) {
            m->endstop_armed = false;
            ESP_LOGW(TAG, "already at end position, move blocked");
            xSemaphoreGive(m->lock);
            return false;
        }
    }

    // 1. Set direction FIRST and let it settle before any step pulse
    gpio_set_level(m->dir_pin, dir);
    esp_rom_delay_us(10);

    // 2. Start slow
    uint32_t hz = (target_hz < RAMP_START_HZ) ? target_hz : RAMP_START_HZ;
    set_freq(m, hz);
    ledc_set_duty(LEDC_MODE, m->channel, STEP_DUTY_50);
    ledc_update_duty(LEDC_MODE, m->channel);

    bool hit = false;

    // 3. Accelerate
    uint32_t elapsed = 0;
    while (!hit && hz < target_hz) {
        hit = wait_or_endstop(m, RAMP_INTERVAL_MS);
        if (hit) break;
        elapsed += RAMP_INTERVAL_MS;
        hz += RAMP_STEP_HZ;
        if (hz > target_hz) hz = target_hz;
        set_freq(m, hz);
    }

    // 4. Cruise
    if (!hit && run_ms > elapsed) {
        hit = wait_or_endstop(m, run_ms - elapsed);
    }

    // 5. Decelerate
    while (!hit && hz > RAMP_START_HZ) {
        hz = (hz > RAMP_START_HZ + RAMP_STEP_HZ) ? hz - RAMP_STEP_HZ : RAMP_START_HZ;
        set_freq(m, hz);
        hit = wait_or_endstop(m, RAMP_INTERVAL_MS);
    }

    // Endstop hit: hard stop, no deceleration.
    ledc_stop(LEDC_MODE, m->channel, 0);
    m->endstop_armed = false;
    xSemaphoreGive(m->lock);

    if (hit) {
        ESP_LOGI(TAG, "endstop reached, motor stopped");
    }
    return !hit;
}

// ================= Public API (same signatures as before) =================

void setup_stepper() {
    gpio_config_t config = {
        .intr_type    = GPIO_INTR_DISABLE,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en   = 0,
        .pin_bit_mask = (1ULL << Stepper_STEP) | (1ULL << Stepper_DIR) |
                        (1ULL << CloseStepper_STEP) | (1ULL << CloseStepper_DIR)};
    gpio_config(&config);

    setup_channel(&barrel, Stepper_STEP);
    setup_channel(&closer, CloseStepper_STEP);
    setup_endstop(&closer);
}

void set_stepper_speed(int frequency) {
    set_freq(&barrel, (uint32_t)frequency);
}

void set_close_stepper_speed(int frequency) {
    set_freq(&closer, (uint32_t)frequency);
}

void MakeLabel() {
    printf("making label\n");
    stepper_move(&barrel, 1, BARREL_RUN_HZ, 6000);
    printf("made label\n");
    vTaskDelete(NULL);  // Delete task when done
}

void NudgeBarrel(bool dir, int time) {
    if (dir) { printf("forwards\n"); }
    stepper_move(&barrel, dir, BARREL_RUN_HZ, (uint32_t)time);
}

void NudgeClose(bool dir, int time) {
    if (dir) { printf("close forwards\n"); }
    if (stepper_move(&closer, dir, CLOSE_RUN_HZ, time)) {
        printf("Finished\n");
    } else {
        printf("Close stopped at end position\n");
    }
}