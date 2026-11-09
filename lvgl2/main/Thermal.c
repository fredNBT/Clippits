#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#define UART_NUM UART_NUM_1  // Use UART1 or another available UART
#define TXD_PIN 25           
 #define RXD_PIN 27  
#define BUF_SIZE 1024
#define LINE_WIDTH   36   // total characters per line
#define LEFT_MARGIN  10   // always-empty padding at the start

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
#define LINE_WIDTH   36   // total characters per line
#define LEFT_MARGIN  10   // always-empty padding at the start

void print_text(const char *text)
{
    static const char spaces[LINE_WIDTH] =
        "                                    ";      // 36 spaces
    const int area = LINE_WIDTH - LEFT_MARGIN;       // 26 usable characters

    if (text == NULL) text = "";

    int len = (int)strlen(text);
    if (len > area) len = area;                      // cut off anything too long

    // Centre within the 26-char area; an odd leftover space goes on the left
    int pad = LEFT_MARGIN + (area - len + 1) / 2;

    uart_write_bytes(UART_NUM, spaces, pad);         // Leading spaces
    uart_write_bytes(UART_NUM, text, len);           // Text
    uart_write_bytes(UART_NUM, "\n", 1);             // Newline (LF)
}

void JogPrinter() {
    uart_write_bytes(UART_NUM, "\n", 1);      // Send empty line to move the printer forward
}
void SendTestMessage()
{
   print_text("     Kabel Sind Cool!");
   //uart_write_bytes(UART_NUM, "\n", 1); 
    // uart_write_bytes(UART_NUM, "Test Test Test\n", 15);  
  
}
