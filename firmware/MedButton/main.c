#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "OnethinxCore01.h"
#include "LoRaWAN_keys.h"
#include "cy_pdl.h"
#include <stdlib.h>


coreStatus_t 	coreStatus;
coreInfo_t 		coreInfo;

uint8_t RXbuffer[64];
uint8_t TXbuffer[64];

#define LED_BLUE            (P12_4)
#define LED_RED             (P12_5)
#define GPIO_INTERRUPT_PRIORITY (7u)
#define TASK_GPS_STACK_SIZE         (256u)

coreConfiguration_t	coreConfig = {
    .Join.KeysPtr = 		&TTN_OTAAkeys,
    .Join.DataRate =		DR_AUTO,
    .Join.Power =			PWR_MAX,
    .Join.MAXTries = 		100,
    .Join.SubBand_1st =     EU_SUB_BANDS_DEFAULT,
    .Join.SubBand_2nd =     EU_SUB_BANDS_DEFAULT,
    .TX.Confirmed = 		false,
    .TX.DataRate = 			DR_0,
    .TX.Power = 			PWR_MAX,
    .TX.FPort = 			1,
};


char *UTCtoKyivTime(const char *utcTime);
float NMEAtoDecimalDegrees(const char *degree, char quadrant);
void task_gps(void* param);
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

volatile bool gpio_intr_flag = false;

static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);
void lora_send(void);

cyhal_uart_t gprs_uart;
uint8_t rx_buf[64];


cyhal_uart_t gprs_uart;
       

const cyhal_uart_cfg_t uart_config_gprs =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = NULL,
    .rx_buffer_size = 0,
};

bool call_gprs_task;

void task_gprs(void);

int main(void){
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

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(P10_1, P10_0, CY_RETARGET_IO_BAUDRATE);

    /* Initialize the user button */
    result = cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("Failed to init button!\n");
    }

    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(P0_4, gpio_interrupt_handler, NULL);
    cyhal_gpio_enable_event(P0_4, CYHAL_GPIO_IRQ_RISE, CYHAL_ISR_PRIORITY_DEFAULT, true);

    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));
    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));


    printf("Test printf\n");

    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cyhal_uart_init(&gprs_uart, P9_1, P9_0, NULL, &uart_config_gprs);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cyhal_uart_set_baud(&gprs_uart, 115200, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    coreStatus = LoRaWAN_Init(&coreConfig);
    /* Check Onethinx Core info */
    LoRaWAN_GetInfo(&coreInfo);
    /* send join using parameters in coreConfig, blocks until either success or MAXtries */
    coreStatus = LoRaWAN_Join(true);

    /* check for successful join */
    if (!coreStatus.mac.isJoined){
        while(1) {
            cyhal_gpio_toggle(LED_BLUE);
            CyDelay(100);
        }
    } else {
        printf("Joined network successfully!\n");
        cyhal_gpio_write(LED_BLUE, true);
        /*delay before first message will be sent */
        CyDelay(1000);
    }

    cyhal_uart_t gps_uart;

    // result = cyhal_uart_init(&gps_uart, P9_1, P9_0, NULL, &uart_config);

    // if (result == CY_RSLT_SUCCESS)
    // {
    //     result = cyhal_uart_set_baud(&gps_uart, 9600, NULL);
    // }

    /* GPS TASK */
    uint8_t c = 0;
    int k, index;
    //char that_time;
    char nmea[20], time[20];
    char lon[20], lat[20];
    

    //read and parse raw NMEA sentences

    for (;;)
    {
        printf("Testing\n");
        if (call_gprs_task)
        {
            task_gprs();
            call_gprs_task = false;
        }
        else
        {
            cyhal_system_delay_ms(1000);
        }
        // if (cyhal_uart_getc(&gps_uart, &c, 0) == CY_RSLT_SUCCESS)
        // {
        //     if (c)
        //     {
        //         if (c == '$')
        //         {
        //             for (k = 0; k < 5; k++)
        //             {
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 while (!(c))
        //                 {
        //                     cyhal_uart_getc(&gps_uart, &c, 0);
        //                 }
        //                 nmea[k] = c; // G + P + G + G + A
        //             }

        //             if (strstr(nmea, "GPGGA"))
        //             {
        //                 index = 0;
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 while (!(c == ','))
        //                 {
        //                     time[index] = c;
        //                     ++index;
        //                     cyhal_uart_getc(&gps_uart, &c, 0);
        //                 }
        //                 UTCtoKyivTime(time);
        //                 index = 0;
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 while (!(c == ','))
        //                 {
        //                     lat[index] = c;
        //                     ++index;
        //                     cyhal_uart_getc(&gps_uart, &c, 0);
        //                 }
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 latitude = NMEAtoDecimalDegrees(lon, c);
        //                 printf("\r\nLatitude: %f", latitude);

        //                 index = 0;
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 while (!(c == ','))
        //                 {
        //                     lon[index] = c;
        //                     ++index;
        //                     cyhal_uart_getc(&gps_uart, &c, 0);
        //                 }
        //                 cyhal_uart_getc(&gps_uart, &c, 0);
        //                 longitude = NMEAtoDecimalDegrees(lat, c);
        //                 printf("\r\nLongitude: %f\r\n\r\n", longitude);
        //             }
        //         }
        //     }
        // }
    }

}

void str_to_hex(void) {
    char message[29];
    sprintf(message, "%s,%f,%f", resultTime, longitude, latitude);
    uint8_t j=0;
    for (int i = 0; i < 29; ++i)
        TXbuffer[j++] = message[i] & 0xff;
}

void lora_send(void) {
    cyhal_gpio_write(LED_BLUE, true);
    str_to_hex();
    coreStatus = LoRaWAN_Send((uint8_t *) TXbuffer, 29, true);
    CyDelay(1000);
    if( coreStatus.system.errorStatus == system_BusyError ){
        for(int i=0; i<10; i++){
            cyhal_gpio_toggle(LED_BLUE);;
            CyDelay(100);
        }
    }
    else
    {
        printf("Sent a message!\n");
    }
    cyhal_gpio_write(LED_BLUE, false);
}

static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    call_gprs_task = true;
}

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
uint8_t rx_buf[64];
size_t rx_length = 64;

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
            cyhal_gpio_toggle(LED_RED);
            cyhal_system_delay_ms(100);
        }
    }

    wait_uart_free(uart_obj);
    bool data_received = false;
    while(cyhal_uart_readable(uart_obj) != 0)
    {
        data_received = true;
        char received_char;
        if (CY_RSLT_SUCCESS == cyhal_uart_getc(uart_obj, &received_char, 1))
        {
            printf("%c", received_char);
        }
    }
    if (data_received)
    {
        printf("\n");
    }
    cyhal_system_delay_ms(1000);
}

void task_gprs(void) {
    /* uart GPRS */

    // sprintf(resultMessage, "Time: %s\r\nLatitude and Longitude: %f, %f", resultTime, latitude, longitude);
    // printf("%s\r\n", resultMessage);

    castedMessage = (uint8_t *) resultMessage;

    CyDelay(1000);
    
    char at_cmd[] = "AT\r";
    uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);

    // CALLING WORKING

    //char call_cmd[] = "ATD+ +380679772051;\r";
    //uart_send_cmd_and_wait(call_cmd, sizeof(call_cmd), &gprs_uart);
    

    char sms_config_cmd[] = "AT+CMGF=1\r";
    uart_send_cmd_and_wait(sms_config_cmd, sizeof(sms_config_cmd), &gprs_uart);
    char sms_prepare_cmd[] = "AT+CMGS=\"+380679772051\"\r";
    uart_send_cmd_and_wait(sms_prepare_cmd, sizeof(sms_prepare_cmd), &gprs_uart);
    char sms_text_cmd[] = "Lat: 23.0005, Long: 24.0001";
    uart_send_cmd_and_wait(sms_text_cmd, sizeof(sms_text_cmd), &gprs_uart);
    char sms_stop_cmd[] = { (char)26 };
    uart_send_cmd_and_wait(sms_stop_cmd, 1, &gprs_uart);
}