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
#include <stdlib.h>

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

/*******************************************************************************
* Function Name: NMEAtoDecimalDegrees
********************************************************************************
* Summary:
* Converts nmea raw latitude and longitute data to decimal degrees
*
* Parameters:
*  char
*
* Return:
*  float
*
*******************************************************************************/

float NMEAtoDecimalDegrees(const char *degree, char quadrant)
{
    // nmea format: "ddmm.mmmm" or "dddmm.mmmm" to decimal degrees
    // D+M/60
    float result = 0;
    if (strlen(degree) > 5)
    {
        char integerPart[3 + 1];
        int counter = (degree[4] == '.' ? 2 : 3);
        memcpy(integerPart, degree, counter);
        integerPart[counter] = 0;
        degree += counter;
        result = atoi(integerPart) + atof(degree) / 60.;
        if (quadrant == 'W' || quadrant == 'S')
            result = -result;
    }
    return result;
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
    /* through terminal */

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

    /* uart GPS */

    cyhal_uart_t gps_uart;

    const cyhal_uart_cfg_t uart_config =
        {
            .data_bits = 8,
            .stop_bits = 1,
            .parity = CYHAL_UART_PARITY_NONE,
            .rx_buffer = NULL,
            .rx_buffer_size = 0,
        };

    result = cyhal_uart_init(&gps_uart, P6_1, P6_0, NULL, &uart_config);

    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_uart_set_baud(&gps_uart, CY_RETARGET_IO_BAUDRATE, NULL);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("***********************************************************\r\n");
    printf("PSoC 6 MCU UART and GPS\r\n");
    printf("***********************************************************\r\n\n");

    __enable_irq();

    uint8_t c = 0;
    int k;
    char nmea[120];
    // char result[40];
    // float latitude;
    float longitude;
    char lon[30];
    char lat[30];

    cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);

    for (;;)
    {
        if (cyhal_uart_getc(&gps_uart, &c, 0) == CY_RSLT_SUCCESS)
        {
            //c = cyhal_uart_getc(&gps_uart, &c, 0);
            if (c)
            {
                if (c == '$')
                {
                    for (k = 0; k < 5; k++)
                    {
                        do
                        {
                            cyhal_uart_getc(&gps_uart, &c, 0);
                        } while (!(c));
                        nmea[k] = c; // G + P + R + M + C
                    }

                    if (strstr(nmea, "GPRMC"))
                    {
                        do
                        {
                            do
                            {
                                cyhal_uart_getc(&gps_uart, &c, 0);
                            } while (!(c));
                        } while (!(c == 'A' || c == 'V') && k < 120);
                        k = 0;
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        do
                        {
                            do
                            {
                                cyhal_uart_getc(&gps_uart, &c, 0);
                            } while (!(c));
                            printf("Lat: %c", (char) c);
                            lat[k] = c;
                            k++;
                        } while (!(c == 'N' || c == 'S') && k < 120);
                        printf("\r\n");
                        k = 0;
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        do
                        {
                            do
                            {
                                cyhal_uart_getc(&gps_uart, &c, 0);
                            } while (!(c));
                            printf("Lon: %c", (char) c);
                            lon[k] = c;
                            k++;
                        } while (!(c == 'E' || c == 'W') && k < 120);
                        printf("\r\n");
                        char lat1[30];
                        for (int i = 0; i < k - 3; ++i)
                        {
                            lat1[i] = lat[i];
                        }
                        NMEAtoDecimalDegrees(lat1, lat[-1]);

                        char lon1[30];
                        for (int i = 0; i < k - 3; ++i)
                        {
                            lon1[i] = lon[i];
                        }
                        longitude = NMEAtoDecimalDegrees(lon1, lon[-1]);
                        printf("%f", longitude);
                    }
                }
                // printf("%c", (char)c);
            }
        }
    }
}

/* [] END OF FILE */