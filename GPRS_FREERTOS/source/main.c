/******************************************************************************
* File Name: main.c
*
* Description: This code example demonstrates displaying graphics on an EInk
* display using EmWin graphics library.
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
#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cy_pdl.h"

/*******************************************************************************
 * GPS HENDLER
 ******************************************************************************/
QueueHandle_t gps_command_data_q;

/*******************************************************************************
 * Global constants
 ******************************************************************************/
/* Priorities of user tasks in this project. configMAX_PRIORITIES is defined in
 * the FreeRTOSConfig.h and higher priority numbers denote high priority tasks.
 */
#define TASK_GPS_PRIORITY      (configMAX_PRIORITIES - 1)

#define CY_RETARGET_IO_BAUDRATE             (9600)

#define GPIO_INTERRUPT_PRIORITY (7u)


/* Stack sizes of user tasks in this project */
#define TASK_GPS_STACK_SIZE         (256u)

/* Queue lengths of message queues used in this project */
#define SINGLE_ELEMENT_QUEUE        (1u)


/*******************************************************************************
* Interrupt Function Prototypes
********************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);


/*******************************************************************************
* Global Variables
********************************************************************************/
volatile bool gpio_intr_flag = false;

void task_gps(void* param);

const cyhal_uart_cfg_t uart_config =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = NULL,
    .rx_buffer_size = 0,
};

/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*  System entrance point. This function sets up user tasks and then starts
*  the RTOS scheduler.
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the user button */
    result = cyhal_gpio_init(CYBSP_USER_BTN, CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);

    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(CYBSP_USER_BTN, 
                                 gpio_interrupt_handler, NULL);
    cyhal_gpio_enable_event(CYBSP_USER_BTN, CYHAL_GPIO_IRQ_FALL, 
                                 GPIO_INTERRUPT_PRIORITY, true);

    printf("***********************************************************\r\n");
    printf("PSoC 6 MCU UART and GPS\r\n");
    printf("***********************************************************\r\n\n");

    /* Enable global interrupts */
    __enable_irq();

    /* Create the queues. See the respective data-types for details of queue
     * contents
     */
    gps_command_data_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(uart_config));
    // task_command_data_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(uart_config));

    /* Create the user tasks. See the respective task definition for more
     * details of these tasks.
     */

    xTaskCreate(task_gps, "GPS Task", TASK_GPS_STACK_SIZE,
                NULL, TASK_GPS_PRIORITY, NULL);

    /* Start the RTOS scheduler. This function should never return */
    vTaskStartScheduler();

    /*~~~~~~~~~~~~~~~~~~~~~ Should never get here! ~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /* RTOS scheduler exited */
    /* Halt the CPU if scheduler exits */
    CY_ASSERT(0);

 
    for(;;)
    {
    }
}


/*******************************************************************************
* Function Name: task_gps(void* param)
********************************************************************************
* Summary:
*  Collect data from GPS
*
* Return:
*
*******************************************************************************/
void task_gps(void* param) {
    /* uart GPS */

    cy_rslt_t result;

    cyhal_uart_t gps_uart;

    result = cyhal_uart_init(&gps_uart, P6_1, P6_0, NULL, &uart_config);

    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_uart_set_baud(&gps_uart, CY_RETARGET_IO_BAUDRATE, NULL);
    }

    /* GPS TASK */
    uint8_t c = 0;

 
    for (;;)
    {
        if (cyhal_uart_getc(&gps_uart, &c, 0) == CY_RSLT_SUCCESS ) {
            if (c) {
                printf("%c", (char) c);
            }
        }
    }


}

/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   GPIO interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_irq_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    gpio_intr_flag = true;
}


/* [] END OF FILE  */