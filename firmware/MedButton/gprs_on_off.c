/******************************************************************************
* File Name: main.c
*
* Description: This example demonstrates the UART transmit and receive
*              operation using HAL APIs
*
* Related Document: See Readme.md 
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/
#define RX_BUF_SIZE 64
#define GPRS_SETUP          (P9_2)

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}

cyhal_uart_t gprs_uart;
uint8_t rx_buf[64];
size_t rx_len = 64;


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
        result = cyhal_uart_write_async(uart_obj, cmd, cmd_len);
    }
    else
    {
        result = cyhal_uart_putc(uart_obj, cmd[0]);
    }

    CyDelay(1500);

    printf("readable %lu\r\n", cyhal_uart_readable(&gprs_uart));

    for (size_t i = 0; i < 2; i++) {
        uint8_t received_char;
        cyhal_uart_getc(&gprs_uart, &received_char, 1);
        printf("%c", received_char);
    }

    CyDelay(500);

    if (CY_RSLT_SUCCESS != result)
    {
        cyhal_system_delay_ms(100);
        // while(1)
        // {
        //     cyhal_gpio_toggle(LED_RED);
        //     cyhal_system_delay_ms(100);
        // }
    }

    //wait_uart_free(uart_obj);
    // bool data_received = false;
    // while(cyhal_uart_readable(uart_obj) != 0)
    // {
    //     data_received = true;
    //     uint8_t received_char;
    //     if (CY_RSLT_SUCCESS == cyhal_uart_getc(uart_obj, &received_char, 1))
    //     {
    //         //printf("%c", received_char);
    //     }
    // }
    // if (data_received)
    // {
    //     //printf("\n");
    // }
    cyhal_system_delay_ms(3000);
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM4 CPU.
* Reads one byte from the serial terminal and echoes back the read byte.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    
    uint8_t read_data; /* Variable to store the received character
                        // through terminal */

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    /* Initialize retarget-io to use the debug UART port */
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
    printf("\x1b[2J\x1b[;H");

    printf("***********************************************************\r\n");
    printf("PSoC 6 MCU UART Transmit and Receive\r\n");
    printf("***********************************************************\r\n\n");
    printf(">> Start typing to see the echo on the screen \r\n\n");

    printf("start1\r\n");

    __enable_irq();

    printf("start2\r\n");

    cyhal_system_delay_ms(1000);

    printf("start4\r\n");

    // //TURN ON
    cy_rslt_t rslt;
    rslt = cyhal_gpio_init(GPRS_SETUP, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);

    if (rslt != CY_RSLT_SUCCESS) {
        //printf("Not powered\r\n");
    }

    cyhal_gpio_write(GPRS_SETUP, false);
    cyhal_system_delay_ms(1000);
    cyhal_gpio_write(GPRS_SETUP, true);
    cyhal_system_delay_ms(2000);
    cyhal_gpio_write(GPRS_SETUP, false);
    cyhal_system_delay_ms(3000);

    char message[6] = "hello!";
    char at_cmd[] = "AT\r";
    // char ok_cmd[3] = "OK\r";             
    cyhal_uart_clear(&gprs_uart);
    uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);
    printf("turn on\r\n");

    CyDelay(1000);

    // // cyhal_uart_read(&gprs_uart, (void*)rx_buf, &rx_len);
    // // if (rx_buf != ok_cmd) {
    // //     //TURN ON
    // //     uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);

    // //     if (rx_buf != ok_cmd) {
    // //         //TURN ON
    // //         uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);
    // //     }

    // //     else {
    // //         //do smth (exception, message, etc.)
    // //         //finish?
    // //     }
    // // }

    char sms_config_cmd[] = "AT+CMGF=1\r";
    uart_send_cmd_and_wait(sms_config_cmd, sizeof(sms_config_cmd), &gprs_uart);
    printf("config sms\r\n");
    char sms_prepare_cmd[] = "AT+CMGS=\"+380634003787\"\r";
    uart_send_cmd_and_wait(sms_prepare_cmd, sizeof(sms_prepare_cmd), &gprs_uart);
    printf("prepare sms\r\n");
    cyhal_uart_clear(&gprs_uart);
    uart_send_cmd_and_wait(message, sizeof(message), &gprs_uart);
    printf("send message\r\n");
    char sms_stop_cmd[] = { (char)26 };
    uart_send_cmd_and_wait(sms_stop_cmd, 1, &gprs_uart);    
    printf("stop\r\n");
}

/* [] END OF FILE */
