#include <stdio.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define UART_NUM UART_NUM_0  // Use UART0 (USB Serial Monitor)
#define BUF_SIZE 1024

void uart_task(void *arg) {
    uint8_t data[BUF_SIZE];
    while (1) {
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, pdMS_TO_TICKS(200));
        data[len] = '\0';  // Null-terminate the string
            if (len > 0){
            if (data[0] == 'm' || data[0] == 'M') 
            {
                printf("Moter Baby Motor\n");
            }else
            {
                printf("Received: %s\n", data);
            }
            }
        }
    }


void app_main() {
    uart_driver_install(UART_NUM, BUF_SIZE, 0, 0, NULL, 0);  // Initialize UART
    xTaskCreate(uart_task, "uart_task", 4096, NULL, 5, NULL);
}
