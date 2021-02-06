#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "OnethinxCore01.h"
#include "LoRaWAN_keys.h"

coreStatus_t 	coreStatus;
coreInfo_t 		coreInfo;

uint8_t RXbuffer[64];
uint8_t TXbuffer[64];

#define LED_BLUE            (P12_4)
#define LED_RED             (P12_5)

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

    /* Initialize retarget-io to use the debug UART port */
    result = cy_retarget_io_init(P10_1, P10_0, CY_RETARGET_IO_BAUDRATE);

    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));
    CY_ASSERT(CY_RSLT_SUCCESS == cyhal_gpio_init(LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false));

    printf("Test printf\n");

    /* retarget-io init failed. Stop program execution */
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
        cyhal_gpio_write(LED_BLUE, false);
		/*delay before first message will be sent */
		CyDelay(1000);
	}

    /* main loop */
	for(;;)
	{
		cyhal_gpio_write(LED_BLUE, true);

		/* compose a message to send */
        uint8_t j=0;
        TXbuffer[j++] = 0x48; /* H */
		TXbuffer[j++] = 0x45; /* E */
		TXbuffer[j++] = 0x4c; /* L */
		TXbuffer[j++] = 0x4c; /* L */
		TXbuffer[j++] = 0x4f; /* O */
		TXbuffer[j++] = 0x20; /*   */
		TXbuffer[j++] = 0x57; /* W */
		TXbuffer[j++] = 0x4f; /* O */
		TXbuffer[j++] = 0x52; /* R */
		TXbuffer[j++] = 0x4c; /* L */
		TXbuffer[j++] = 0x44; /* D */
        coreStatus = LoRaWAN_Send((uint8_t *) TXbuffer, j, true);
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

		/* wait before sending next message */
		CyDelay( 10000 );
	}

}