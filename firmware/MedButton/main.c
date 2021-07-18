#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "OnethinxCore01.h"
#include "LoRaWAN_keys.h"
#include "cy_pdl.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "data_struct.h"

message_struct message_str;

SemaphoreHandle_t	 semaphore_gprs;
SemaphoreHandle_t	 semaphore_lora;
 
coreStatus_t 	coreStatus;
coreInfo_t 		coreInfo;

uint8_t RXbuffer[64];
uint8_t TXbuffer[64];

#define LED_BLUE            (P12_4)
#define LED_RED             (P12_5)
#define GPRS_SETUP          (P9_2)

#define TASK                (256u)
#define TASK_GPRS           (512u)

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

void task_gps(void* param);
void task_gprs(void* param);
void lora_send(void* param);
static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_event_t event);

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

cyhal_uart_t gprs_uart;
uint8_t rx_buf[64];       

const cyhal_uart_cfg_t uart_config_gprs =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf,
    .rx_buffer_size = 64,
};

char disable_nmea[10][30] = {
    "$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n",
    "$PUBX,40,ZDA,0,0,0,0,0,0*44\r\n",
    "PUBX,40,VTG,0,0,0,0,0,0*5E\r\n",
    "PUBX,40,GSV,0,0,0,0,0,0*59\r\n",
    "$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n",
    "$PUBX,40,RMC,0,0,0,0,0,0*47\r\n",
    "$PUBX,40,GNS,0,0,0,0,0,0*41\r\n",
    "$PUBX,40,GRS,0,0,0,0,0,0*5D\r\n",
    "$PUBX,40,GST,0,0,0,0,0,0*5B\r\n",
    "$PUBX,40,TXT,0,0,0,0,0,0*43\r\n",
};


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
    result = cyhal_gpio_init(P0_4, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, CYBSP_BTN_OFF);
    if (result != CY_RSLT_SUCCESS)
    {
        // printf("Failed to init button!\n");
    }

    /* Configure GPIO interrupt */
    cyhal_gpio_register_callback(P0_4, gpio_interrupt_handler, NULL);
    cyhal_gpio_enable_event(P0_4, CYHAL_GPIO_IRQ_RISE, CYHAL_ISR_PRIORITY_DEFAULT, true);

    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));
    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));


    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }


	semaphore_lora = xSemaphoreCreateBinary();
    semaphore_gprs = xSemaphoreCreateBinary();

	xTaskCreate(lora_send, "LoRa", TASK , &message_str, 2, NULL);
    xTaskCreate(task_gprs, "GPRS Task", TASK_GPRS , &message_str, 2, NULL);
    xTaskCreate(task_gps, "GPS Task", TASK, &message_str, 1, NULL);

    vTaskStartScheduler();

    CY_ASSERT(0);
    
}


static void gpio_interrupt_handler(void *handler_arg, cyhal_gpio_irq_event_t event)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;;
    xSemaphoreGiveFromISR(semaphore_gprs, xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

char *UTCtoKyivTime(const char *utcTime, message_struct *message_data)
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
    sprintf(message_data->resultTime, "%d:%c%c:%c%c", number, utcTime[2], utcTime[3], utcTime[4], utcTime[5]);
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
        uint8_t received_char;
        if (CY_RSLT_SUCCESS == cyhal_uart_getc(uart_obj, &received_char, 1))
        {
            //printf("%c", received_char);
        }
    }
    if (data_received)
    {
        //printf("\n");
    }
    cyhal_system_delay_ms(3000);
}

static void gprs_setup(void) {
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
}

void task_gprs(void* param) {
    /* uart GPRS */
    message_struct *message_data = (message_struct*) param;

    cy_rslt_t result;

    cyhal_uart_t gprs_uart;

    result = cyhal_uart_init(&gprs_uart, P9_1, P9_0, NULL, &uart_config_gprs);

    if (result == CY_RSLT_SUCCESS)
    {
        cyhal_uart_set_baud(&gprs_uart, 115200, NULL);
    }

    gprs_setup();

    char message[100];
    while (1) {
		if (xSemaphoreTake(semaphore_gprs, 1000)) {
            sprintf(message, "%s-%f,%f", message_data->resultTime, message_data->latitude, message_data->longitude);
            char at_cmd[] = "AT\r";
            uart_send_cmd_and_wait(at_cmd, sizeof(at_cmd), &gprs_uart);
            char sms_config_cmd[] = "AT+CMGF=1\r";
            uart_send_cmd_and_wait(sms_config_cmd, sizeof(sms_config_cmd), &gprs_uart);
            char sms_prepare_cmd[] = "AT+CMGS=\"+380958957865\"\r";
            uart_send_cmd_and_wait(sms_prepare_cmd, sizeof(sms_prepare_cmd), &gprs_uart);
            cyhal_uart_clear(&gprs_uart);
            uart_send_cmd_and_wait(message, sizeof(message), &gprs_uart);
            char sms_stop_cmd[] = { (char)26 };
            uart_send_cmd_and_wait(sms_stop_cmd, 1, &gprs_uart);
        }
    }
}


void lora_send(void* param) {
    message_struct * message_data = (message_struct*) param;

    coreStatus = LoRaWAN_Init(&coreConfig);
    // /* Check Onethinx Core info */
    LoRaWAN_GetInfo(&coreInfo);
    /* send join using parameters in coreConfig, blocks until either success or MAXtries */
    coreStatus = LoRaWAN_Join(true);

    // /* check for successful join */
    if (!coreStatus.mac.isJoined){
        // while(1) {
		cyhal_gpio_toggle(LED_BLUE);
        cyhal_system_delay_ms(100);
        // }
    } else {
        cyhal_gpio_write(LED_BLUE, true);
        /*delay before first message will be sent */
        cyhal_system_delay_ms(1000);
    }

    char message[50];
    while(1) {
        if(xSemaphoreTake(semaphore_lora, 1000)) {
            sprintf(message, "%s-%f,%f", message_data->resultTime, message_data->latitude, message_data->longitude);
            coreStatus = LoRaWAN_Send((uint8_t *) message, 64, true);
            cyhal_system_delay_ms(1000); // vTaskDelay 
            if( coreStatus.system.errorStatus == system_BusyError ){
                for(int i=0; i<10; i++){
                    cyhal_gpio_toggle(LED_BLUE);;
                    cyhal_system_delay_ms(100);
                }
            }
        }
    }
}

uint8_t is_float(const char *check) {
    int len;
    float ignore;
    int ret = sscanf(check, "%f %n", &ignore, &len);
    return (ret==1 && !check[len]);
}

void task_gps(void* param) {
    /* uart GPS */
    message_struct *message_data = (message_struct*) param;
    message_data->longitude = 0.0;
    message_data->latitude = 0.0;

    cy_rslt_t result;

    cyhal_uart_t gps_uart;

    result = cyhal_uart_init(&gps_uart, P10_1, P10_0, NULL, &uart_config);

    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_uart_set_baud(&gps_uart, 9600, NULL);
    }

    for (size_t i = 0; i < 10; i++) {
        uart_send_cmd_and_wait(disable_nmea[i], 30, &gps_uart);
    }

    /* GPS TASK */
    uint8_t c = 0;
    int k, index;
    char nmea[20], time[20];
    char lon[20], lat[20];    

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
                        index = 0;
                        cyhal_uart_getc(&gps_uart, &c, 0);
                        while (!(c == ','))
                        {
                            lat[index] = c;
                            ++index;
                            cyhal_uart_getc(&gps_uart, &c, 0);
                        }
                        cyhal_uart_getc(&gps_uart, &c, 0);

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

                        /// check if new longitude is float if not - don't change old data
                        if (is_float(lon)) {
                            message_data->longitude = NMEAtoDecimalDegrees(lon, c);
                            message_data->latitude = NMEAtoDecimalDegrees(lat, c);
                            UTCtoKyivTime(time, message_data);
                        }
                    }
                }
            }
        }
    }
}