/******************************************************************************
* File Name: main.c
*
* Description: This example demonstrates the UART transmit and receive
*              operation using HAL APIs
*
* Related Document: See Readme.md 
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
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

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

#define PGPRS P5_2

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
cyhal_uart_t gprs_uart;

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}

void powerUpOrDown(void) {
    cyhal_gpio_init(PGPRS, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    cyhal_gpio_write(PGPRS, 0);
    CyDelay(1000);
    cyhal_gpio_write(PGPRS, 1);
    CyDelay(2000);
    cyhal_gpio_write(PGPRS, 0);
    CyDelay(3000);
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
    // uint8_t read_data; /* Variable to store the received character
    //                     * through terminal */

    uint8_t tx_buf[3] = {'A', 'T', '\x0d'};
    size_t lenght1 = 3;
    uint8_t tx_buf1[10] = {'A', 'T', '+', 'C', 'M', 'G', 'F', '=', '1', '\x0d'};
    size_t lenght2 = 10;
    uint8_t tx_buf2[24] = {'A', 'T', '+', 'C', 'M', 'G', 'S', '=', '\x22', '+', '3', '8', '0', '9', '5', '8', '9', '5', '7', '8', '6', '5', '\x22', '\x0d'};
    size_t lenght3 = 24;
    uint8_t tx_buf3[5] = {'H', 'e', 'l', 'l', 'o'};
    size_t lenght4 = 6;
    uint8_t tx_buf4[5] = {'A', 'T', 'H', '0', '\x0d'};
    size_t lenght5 = 5;
    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

    /* uart GPRS */

    const cyhal_uart_cfg_t uart_config =
    {
        .data_bits = 8,
        .stop_bits = 1,
        .parity = CYHAL_UART_PARITY_NONE,
        .rx_buffer = NULL,
        .rx_buffer_size = 0,
    };



    result = cyhal_uart_init(&gprs_uart, P6_1, P6_0, NULL, &uart_config);

    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_uart_set_baud(&gprs_uart, CY_RETARGET_IO_BAUDRATE, NULL);
    }

    powerUpOrDown();

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("***********************************************************\r\n");
    printf("PSoC 6 MCU UART Transmit and Receive\r\n");
    printf("***********************************************************\r\n\n");
    CyDelay(1000);
    cyhal_uart_write(&gprs_uart, (void*)tx_buf, &lenght1);
    CyDelay(500);
    cyhal_uart_write(&gprs_uart, (void*)tx_buf1, &lenght2);
    CyDelay(500);
    cyhal_uart_write(&gprs_uart, (void*)tx_buf2, &lenght3);
    CyDelay(500);
    cyhal_uart_write(&gprs_uart, (void*)tx_buf3, &lenght4);
    CyDelay(500);
    cyhal_uart_putc(&gprs_uart, 26);

    __enable_irq();

 
    for (;;)
    {
    }
}

/* [] END OF FILE */
