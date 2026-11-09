// This exsists to lock the LVGL spi bus when the screen is being updated from a diffrent task. Advoids 2 tasks writing 
//bus at the same time causing a crash
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static SemaphoreHandle_t lvgl_mutex;

void lvgl_port_init_locking(void)
{
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();
}

void lvgl_port_lock(void)
{
    xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY);
}

void lvgl_port_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_mutex);
}
