#include "lora_task.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "OnethinxCore01.h"
#include "LoRaWAN_keys.h"
#include "data_struct.h"
#include <stdlib.h>

#define LED_BLUE            (P12_4)

coreStatus_t 	coreStatus;
coreInfo_t 		coreInfo;

uint8_t RXbuffer[64];
uint8_t TXbuffer[64];


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
        if(xSemaphoreTake(message_data->semaphore_lora, 5000)) {
            xSemaphoreTake(message_data->mutex, portMAX_DELAY);
            sprintf(message, "%s-%f,%f", message_data->resultTime, message_data->latitude, message_data->longitude);
            xSemaphoreGive(message_data->mutex);
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