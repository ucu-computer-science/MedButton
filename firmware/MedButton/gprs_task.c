#include "gprs_task.h"
#include "cryptography.h"

#define LED_RED             (P12_5)
#define GPRS_SETUP          (P9_2)
#define MAX_NUM_OF_TRIALS    2

cyhal_uart_t gprs_uart;
uint8_t rx_buf[450];       

const cyhal_uart_cfg_t uart_config_gprs =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf,
    .rx_buffer_size = 450,
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
        result = cyhal_uart_write_async(uart_obj, cmd, cmd_len);
    }
    else
    {
        result = cyhal_uart_putc(uart_obj, cmd[0]);
    }

    if (CY_RSLT_SUCCESS != result)
    {
        while(1)
        {
            // cyhal_gpio_toggle(LED_RED);
            cyhal_system_delay_ms(100);
        }
    }

    wait_uart_free(uart_obj);

    cyhal_system_delay_ms(3000);
}

void task_gprs(void* param) {
    /* uart GPRS */
    message_struct *message_data = (message_struct*) param;

    cy_rslt_t result;

    cyhal_uart_t gprs_uart;

    int trials = 0;

    result = cyhal_uart_init(&gprs_uart, P9_1, P9_0, NULL, &uart_config_gprs);

    if (result == CY_RSLT_SUCCESS)
    {
        cyhal_uart_set_baud(&gprs_uart, 115200, NULL);
    }

    cy_rslt_t rslt;
    rslt = cyhal_gpio_init(GPRS_SETUP, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);
    if (rslt != CY_RSLT_SUCCESS) {
        handle_error();
    }

    CY_ALIGN(4) char message[200];
    while (1) {
		if (xSemaphoreTake(message_data->semaphore_gprs, 5000)) {
            xSemaphoreTake(message_data->mutex, portMAX_DELAY);
		    //added unique id
            sprintf(message, "%s,%d:%d:%d:\n%s-%f,%f\n%s-%f,%f\n%s-%f,%f\n%s-%f,%f\n%s-%f,%f",
                            message_data->unique_id, message_data->current_time.tm_hour, message_data->current_time.tm_min, message_data->current_time.tm_sec,
                            message_data->resultTime[4], message_data->latitude[4], message_data->longitude[4],
                            message_data->resultTime[3], message_data->latitude[3], message_data->longitude[3],
                            message_data->resultTime[2], message_data->latitude[2], message_data->longitude[2],
                            message_data->resultTime[1], message_data->latitude[1], message_data->longitude[1],
                            message_data->resultTime[0], message_data->latitude[0], message_data->longitude[0]);
            xSemaphoreGive(message_data->mutex);
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
                cyhal_system_delay_ms(6000);
                cyhal_gpio_write(GPRS_SETUP, false);
                cyhal_system_delay_ms(15000);

                cyhal_uart_clear(&gprs_uart);
                uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);
                uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);

                if (trials >= MAX_NUM_OF_TRIALS) {
                    xSemaphoreGive(message_data->semaphore_lora);
                }
            }

            // CY_ALIGN(4) uint8_t encrypted_msg[MAX_MESSAGE_SIZE];
            // CY_ALIGN(4) message;
            //encrypt_message(message, sizeof(message), encrypted_msg);

            char sms_config_cmd[] = "AT+CMGF=1\r";
            uart_send_cmd_and_wait(sms_config_cmd, sizeof(sms_config_cmd), &gprs_uart);
            CyDelay(5000);
            char sms_prepare_cmd[] = "AT+CMGS=\"+380662349833\"\r";
            uart_send_cmd_and_wait(sms_prepare_cmd, sizeof(sms_prepare_cmd), &gprs_uart);
            cyhal_uart_clear(&gprs_uart);
            CyDelay(5000);
            uart_send_cmd_and_wait(message, sizeof(message), &gprs_uart);
            char sms_stop_cmd[] = { (char)26 };
            uart_send_cmd_and_wait(sms_stop_cmd, 1, &gprs_uart);
            uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);
            CyDelay(5000);
        }
    }
}
