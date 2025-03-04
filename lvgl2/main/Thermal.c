#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#define UART_NUM UART_NUM_1  // Use UART1 or another available UART
#define TXD_PIN 25           
 #define RXD_PIN 27  
#define BUF_SIZE 1024

void uart_init()
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
void print_text(const char *text) {
    uart_write_bytes(UART_NUM, text, strlen(text));  // Send text
    uart_write_bytes(UART_NUM, "\n", 1);             // Send newline (LF)
    uart_write_bytes(UART_NUM, "\n", 1); 
    uart_write_bytes(UART_NUM, "\n", 1); 
    uart_write_bytes(UART_NUM, "\n", 1); 
    uart_write_bytes(UART_NUM, "\n", 1); 
    uart_write_bytes(UART_NUM, "\n", 1); 
}

void SendTestMessage()
{
     printf("sending to thermal\n");
    print_text("Hello World!");
    print_text("12345678901234567890123456789012");
}
