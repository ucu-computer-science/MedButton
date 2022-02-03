#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "cy_pdl.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ble_task.h"
#include "gps_task.h"
#include "gprs_task.h"
#include "lora_task.h"

message_struct message_str;

cyhal_timer_t timer;

#define LED_BLUE            (P12_4)
#define LED_RED             (P12_5)

#define TASK                (256u)
#define TASK_GPRS           (512u)
#define TASK_BLE            (configMINIMAL_STACK_SIZE * 4)


#define TIMER_CLOCK_HZ          (10000)
#define TIMER_PERIOD            (99999 * 60)

#define BLE_CMD_Q_LEN           (10u)

static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);

/* FOR BLE */
TaskHandle_t  ble_task_handle;
QueueHandle_t ble_cmdQ;
TimerHandle_t timer_handle;
tx_rx_mode mode_flag = GATT_NOTIF_STOC;
/***************/

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

    /* Enable global interrupts */
    __enable_irq();


    /* Initialize the user button */
    result = cyhal_gpio_init(P11_5, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(P11_5, gpio_interrupt_handler, NULL);
    cyhal_gpio_enable_event(P11_5, CYHAL_GPIO_IRQ_RISE, CYHAL_ISR_PRIORITY_DEFAULT, true);

    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));
    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));


    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* FOR BLE */
    ble_cmdQ = xQueueCreate(BLE_CMD_Q_LEN, sizeof(ble_command_type_t));

    timer_handle = xTimerCreate("Timer", pdMS_TO_TICKS(1000), pdTRUE,
                               NULL, rtos_timer_cb);


    /* Initialize current time */
    message_str.current_time.tm_hour = 10;
    message_str.current_time.tm_min = 38;
    message_str.current_time.tm_sec = 50;
    
	message_str.semaphore_lora = xSemaphoreCreateBinary();
    message_str.semaphore_gprs = xSemaphoreCreateBinary();
    message_str.mutex = xSemaphoreCreateMutex();

    xTaskCreate(task_gps, "GPS Task", TASK_GPRS, &message_str, 1, NULL);
    // xTaskCreate(task_BLE, "BLE Task", TASK_BLE, &message_str, 1, &ble_task_handle);
	xTaskCreate(lora_send, "LoRa", TASK , &message_str, 2, NULL);
    xTaskCreate(task_gprs, "GPRS Task", TASK_GPRS , &message_str, 2, NULL);
    

    vTaskStartScheduler();

    CY_ASSERT(0);
    
}

/*
    Interrupt by button. Wake up gprs task
*/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;;
    xSemaphoreGiveFromISR(message_str.semaphore_gprs, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
