/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_lcd_touch_xpt2046.h"

// #define XPT2046_MISO  26
// #define XPT2046_MOSI  GPIO_NUM_23
// #define XPT2046_CLK   GPIO_NUM_18
// #define XPT2046_CS    GPIO_NUM_11


void app_main(void)
{
    printf("Hello world!\n");

}
