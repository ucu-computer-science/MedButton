#include "cyhal.h"
#include "cybsp.h"
#include "cy_pdl.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cy_pdl.h"
#include <stdlib.h>
#include <stdint.h>

/*******************************************************************************
 * GPS HENDLER
 ******************************************************************************/
QueueHandle_t gps_command_data_q;
QueueHandle_t gprs_command_data_q;

/*******************************************************************************
 * Global constants
 ******************************************************************************/
/* Priorities of user tasks in this project. configMAX_PRIORITIES is defined in
 * the FreeRTOSConfig.h and higher priority numbers denote high priority tasks.
 */
#define TASK_GPS_PRIORITY      (configMAX_PRIORITIES - 2)
#define TASK_GPRS_PRIORITY           (configMAX_PRIORITIES - 1)

#define CY_RETARGET_IO_BAUDRATE             (9600)

#define GPIO_INTERRUPT_PRIORITY (7u)


/* Stack sizes of user tasks in this project */
#define TASK_GPS_STACK_SIZE         (256u)
#define TASK_GPRS_STACK_SIZE         (256u)

/* Queue lengths of message queues used in this project */
#define SINGLE_ELEMENT_QUEUE        (1u)


/*******************************************************************************
* Interrupt Function Prototypes
********************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);

TaskHandle_t gprsHandle = NULL;

/*******************************************************************************
* Global Variables
********************************************************************************/
volatile bool gpio_intr_flag = false;

char *UTCtoKyivTime(const char *utcTime);
float NMEAtoDecimalDegrees(const char *degree, char quadrant);
void task_gps(void* param);
void task_gprs(void* param);
float latitude, longitude;
char resultTime[11];
char resultMessage[100]; 
uint8_t * castedMessage;


const cyhal_uart_cfg_t uart_config =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = NULL,
    .rx_buffer_size = 0,
};


// GPRS
cyhal_uart_t gprs_uart;
uint8_t rx_buf[64];


cyhal_uart_t gprs_uart;
       

const cyhal_uart_cfg_t uart_config_gprs =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf,
    .rx_buffer_size = 64,
};

// TIMER
cyhal_timer_t timer_obj;
static void isr_timer(void *callback_arg, cyhal_timer_event_t event);

static void gprs_setup(void) {
    cy_rslt_t rslt;
    rslt = cyhal_gpio_init(P9_2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false);

    if (rslt != CY_RSLT_SUCCESS) {
        printf("Not powered\r\n");
    }

    cyhal_gpio_write(P9_2, false);
    CyDelay(1000);
    cyhal_gpio_write(P9_2, true);
    CyDelay(2000);
    cyhal_gpio_write(P9_2, false);
    CyDelay(3000);
}

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
//cyhal_uart_write_async();
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
    result = cyhal_gpio_init(P5_4, CYHAL_GPIO_DIR_INPUT,
                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_BTN_OFF);

    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(P5_4, 
                                 gpio_interrupt_handler, NULL);
    cyhal_gpio_enable_event(P5_4, CYHAL_GPIO_IRQ_FALL, 
                                 GPIO_INTERRUPT_PRIORITY, true);

     result = cyhal_uart_init(&gprs_uart, P9_1, P9_0, NULL, &uart_config_gprs);

    if (result == CY_RSLT_SUCCESS)
        {
            result = cyhal_uart_set_baud(&gprs_uart, CY_RETARGET_IO_BAUDRATE, NULL);
        }

    gprs_setup();

    // TIMER
    const cyhal_timer_cfg_t timer_cfg =
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = 600*9999,                      /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run the timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };
    /* Initialize the timer object. Does not use pin output ('pin' is NC) and
     * does not use a pre-configured clock source ('clk' is NULL). */
    result = cyhal_timer_init(&timer_obj, NC, NULL);
    CY_ASSERT(CY_RSLT_SUCCESS == result);
    /* Apply timer configuration such as period, count direction, run mode, etc. */
    result = cyhal_timer_configure(&timer_obj, &timer_cfg);
    /* Set the frequency of timer to 10000 Hz */
    result = cyhal_timer_set_frequency(&timer_obj, 10000);
    /* Assign the ISR to execute on timer interrupt */
    cyhal_timer_register_callback(&timer_obj, isr_timer, NULL);
    /* Set the event on which timer interrupt occurs and enable it */
    cyhal_timer_enable_event(&timer_obj, CYHAL_TIMER_IRQ_TERMINAL_COUNT, 3, true);
    /* Start the timer with the configured settings */
    result = cyhal_timer_start(&timer_obj);

    printf("***********************************************************\r\n");
    printf("PSoC 6 MCU UART and GPS\r\n");
    printf("***********************************************************\r\n\n");

    /* Enable global interrupts */
    __enable_irq();

    /* Create the queues. See the respective data-types for details of queue
     * contents
     */
    gps_command_data_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(uart_config));
    gprs_command_data_q = xQueueCreate(SINGLE_ELEMENT_QUEUE, sizeof(uart_config));

    /* Create the user tasks. See the respective task definition for more
     * details of these tasks.
     */

    xTaskCreate(task_gps, "GPS Task", TASK_GPS_STACK_SIZE,
                NULL, TASK_GPS_PRIORITY, NULL);

    xTaskCreate(task_gprs, "GPRS Task", TASK_GPRS_STACK_SIZE,
            NULL, TASK_GPRS_PRIORITY, &gprsHandle);

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
    int k, index;
    //char that_time;
    char nmea[20], time[20];
    char lon[20], lat[20];
    

    cyhal_gpio_write(CYBSP_USER_LED, CYBSP_LED_STATE_OFF);

    //read and parse raw NMEA sentences

    for (;;)
    {
        if (cyhal_uart_getc(&gps_uart, &c, 0) == CY_RSLT_SUCCESS)
        {
            if (c)
            {
                if (c == '$')
                {
                    for (k = 0; k < 5; k++)
                    {
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        while (!(c))
                        {
                            cyhal_uart_getc(&gps_uart, &c, 0);
                        }
                        nmea[k] = c; // G + P + G + G + A
                    }

                    if (strstr(nmea, "GPGGA"))
                    {
                        index = 0;
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        while (!(c == ','))
                        {
                            time[index] = c;
                            ++index;
                            cyhal_uart_getc(&gps_uart, &c, 0);
                        }
                        UTCtoKyivTime(time);
                        index = 0;
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        while (!(c == ','))
                        {
                            lat[index] = c;
                            ++index;
                            cyhal_uart_getc(&gps_uart, &c, 0);
                        }
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        latitude = NMEAtoDecimalDegrees(lon, c);
                        printf("\r\nLatitude: %f", latitude);

                        index = 0;
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        while (!(c == ','))
                        {
                            lon[index] = c;
                            ++index;
                            cyhal_uart_getc(&gps_uart, &c, 0);
                        }
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        longitude = NMEAtoDecimalDegrees(lat, c);
                        printf("\r\nLongitude: %f\r\n\r\n", longitude);
                    }
                }
            }
        }
    }
}

/*******************************************************************************
* Function Name: task_gprs(void* param)
********************************************************************************
* Summary:
*  Send data by GPRS
*
* Return:
*
*******************************************************************************/
void task_gprs(void* param) {
    /* uart GPRS */
    while (1) {
        vTaskSuspend(NULL); //suspend itself
        
        sprintf(resultMessage, "Time: %s\r\nLatitude and Longitude: %f, %f", resultTime, latitude, longitude);
        printf("%s\r\n", resultMessage);

        castedMessage = (uint8_t *) resultMessage;

        CyDelay(1000);
        cyhal_uart_write_async(&gprs_uart, "AT\r", 3);
        CyDelay(1000);
        cyhal_uart_write_async(&gprs_uart, "AT+CMGF=1\r", 10);
        CyDelay(1000);
        cyhal_uart_write_async(&gprs_uart, "AT+CMGS=\"+380958957865\"\r", 24);
        CyDelay(1000);
        cyhal_uart_write_async(&gprs_uart, (void*)castedMessage, 100);
        CyDelay(1000);
        cyhal_uart_putc(&gprs_uart, '\032');
        CyDelay(5000);

    }
}

/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   Button interrupt handler.
*
* Parameters:
*  void *handler_arg (unused)
*  cyhal_gpio_irq_event_t (unused)
*
*******************************************************************************/
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = xTaskResumeFromISR(gprsHandle);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*******************************************************************************
* Function Name: gpio_interrupt_handler
********************************************************************************
* Summary:
*   Timer interrupt handler.
*
* Parameters:
*  void *callback_arg (unused)
*  cyhal_timer_event_t (unused)
*
*******************************************************************************/
static void isr_timer(void *callback_arg, cyhal_timer_event_t event)
{
    (void) callback_arg;
    (void) event;

    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = xTaskResumeFromISR(gprsHandle);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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
* Function Name: UTCtoKyivTime
********************************************************************************
* Summary:
* Converts UTC time to Kyiv
*
* Parameters:
*  char
*
* Return: 
*   pass
*
*******************************************************************************/

char *UTCtoKyivTime(const char *utcTime)
{
    // 172814.0 - hhmmss.ss
    int i, digit, number = 0;
    char c;
    for (i = 0; i < 2; i++)
    {
        c = utcTime[i];
        if (c >= '0' && c <= '9') //to confirm it's a digit
        {
            digit = c - '0';
            number = number * 10 + digit;
        }
    }
    number = (number + 2) % 24;
    sprintf(resultTime, "%d:%c%c:%c%c", number, utcTime[2], utcTime[3], utcTime[4], utcTime[5]);
    printf("Time: %s", resultTime);
    return 0;
}
