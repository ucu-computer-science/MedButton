#define RX_BUF_SIZE 64
#define TX_BUF_SIZE 64
#define RESPONSE_LENGTH 4
#define GPRS_SETUP          (P9_2)
#define MAX_NUM_OF_TRIALS 2

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}

cyhal_uart_t gprs_uart;
uint8_t rx_buf[64];
uint8_t tx_buf[64];
size_t rx_length = RESPONSE_LENGTH;
int trials = 0;


const cyhal_uart_cfg_t uart_config_gprs =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf,
    .rx_buffer_size = 64,
};


void wait_uart_free(cyhal_uart_t *uart_obj)
{
    while (cyhal_uart_is_rx_active(uart_obj)) { cyhal_system_delay_ms(1); }
    while (cyhal_uart_is_tx_active(uart_obj)) { cyhal_system_delay_ms(1); }
}

void uart_send_cmd_and_wait(char *cmd, size_t cmd_len, cyhal_uart_t *uart_obj)
{
    cy_rslt_t result;
    wait_uart_free(uart_obj);

    if (cmd_len > 1)
    {
        // result = cyhal_uart_write_async(uart_obj, cmd, cmd_len);
        result = cyhal_uart_write(uart_obj, cmd, &cmd_len);
    }
    else
    {
        result = cyhal_uart_putc(uart_obj, cmd[0]);
    }

    CyDelay(1500);

    if (CY_RSLT_SUCCESS != result)
    {
        cyhal_system_delay_ms(100);
    }

    CyDelay(5000);
}

int main(void)
{
    cy_rslt_t result;
    
    uint8_t read_data;

    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    result = cy_retarget_io_init(P6_1, P6_0,
                                 115200);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    cyhal_uart_t gprs_uart;

    //result = cyhal_uart_init(&gprs_uart, P10_1, P10_0, NULL, &uart_config_gprs);
    result = cyhal_uart_init(&gprs_uart, P5_1, P5_0, NULL, &uart_config_gprs);

    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_uart_set_baud(&gprs_uart, 115200, NULL);
    }
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */

    printf("***********************************************************\r\n");
    printf("PSoC 6 MCU UART Transmit and Receive\r\n");
    printf("***********************************************************\r\n\n");
    printf(">> Start typing to see the echo on the screen \r\n\n");

    //printf("start1\r\n");

    __enable_irq();

    //printf("start2\r\n");

    cyhal_system_delay_ms(1000);

    //printf("start4\r\n");

    cy_rslt_t rslt;
    rslt = cyhal_gpio_init(GPRS_SETUP, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);

    if (rslt != CY_RSLT_SUCCESS) {
        return -1;
    }

    char message[6] = "hello!";
    char at_cmd[] = "AT\r";

    cyhal_uart_clear(&gprs_uart);      
    uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);
    uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);

    CyDelay(1000);

    while (rx_buf[7] != 'O') {
        trials++;
        cyhal_gpio_write(GPRS_SETUP, false);
        cyhal_system_delay_ms(1000);
        cyhal_gpio_write(GPRS_SETUP, true);
        cyhal_system_delay_ms(2000);
        cyhal_gpio_write(GPRS_SETUP, false);
        cyhal_system_delay_ms(3000);

        cyhal_uart_clear(&gprs_uart);
        uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);
        uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);

        if (trials >= MAX_NUM_OF_TRIALS) {
            return -1;
            //break;
        }
    }

    char sms_config_cmd[] = "AT+CMGF=1\r";
    uart_send_cmd_and_wait(sms_config_cmd, sizeof(sms_config_cmd), &gprs_uart);

    char sms_prepare_cmd[] = "AT+CMGS=\"+380634003787\"\r";
    uart_send_cmd_and_wait(sms_prepare_cmd, sizeof(sms_prepare_cmd), &gprs_uart);
    cyhal_uart_clear(&gprs_uart);
    uart_send_cmd_and_wait(message, sizeof(message), &gprs_uart);
    char sms_stop_cmd[] = { (char)26 };
    uart_send_cmd_and_wait(sms_stop_cmd, 1, &gprs_uart);    

    CY_ASSERT(0);
}

/* [] END OF FILE */
