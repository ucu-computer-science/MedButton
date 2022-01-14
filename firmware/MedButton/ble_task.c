/******************************************************************************
* File Name: ble_task.c
*
* Description: This file contains the task that initializes BLE and
*              handles different BLE events.
*
* Related Document: README.md
*
*******************************************************************************
* Copyright (2020), Cypress Semiconductor Corporation. All rights reserved.
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

/******************************************************************************
 * Include header files
 *****************************************************************************/
#include "ble_task.h"
#include "cybsp.h"
#include "cyhal.h"
#include "cycfg_ble.h"
#include "string.h"
#include "data_struct.h"
#ifdef EINK_DISPLAY_SHIELD_PRESENT
#include "display_task.h"
#endif

/******************************************************************************
 * Macros
 *****************************************************************************/
/* GATT parameters */
#define NOTIFY_CCCD_UUID            {0x02, 0x29}
#define NOTIFY_CCCD_SIZE            (02u)
#define WRITEME_CHAR_UUID           {0xC7u, 0x58u, 0xCFu, 0x70u, 0xB3u, 0xAFu,\
                                     0xE4u,0xADu, 0x65u, 0x44u, 0xA3u, 0x85u, 0x26u,\
                                     0x7Bu,0x70u, 0xD4u}
#define WRITEME_CHAR_UUID_SIZE      (16u)
#define BLE_INTERRUPT_PRIORITY      (1u)
#define ENABLE                      (1u)
#define DISABLE                     (0u)
#define CONN_INTERVAL_MULTIPLIER    (1.25f)
#define TARGET_NAME_LENGTH          (5u)
#define AD_TYPE_COMPLETE_LOCAL_NAME (9u)
/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Variables to hold GATT notification bytes count and GATT write bytes count */
static uint32_t  gatt_write_tx_bytes;
static uint32_t  notif_rx_bytes;

/* Variable to store latest connection interval for the BLE connection */
static float conn_interval;

/* CCCD value to enable/disable notification */
static uint8_t CCCD_VALUE[NOTIFY_CCCD_SIZE] = {ENABLE,DISABLE};

/* Flags */
/* Indication to start/stop GATT write */
bool gatt_write_flag = false;
/* To ignore button press before the device is connected with peer */
bool button_flag = false;
/* To scan only once, upon first button press by user */
bool scan_flag = false;
/* To indicate if BLE stack is busy or free */
static bool stack_free = true;
/* Flags used to indicate when to stop discovery procedure */
static bool notify_cccd_uuid_found;
static bool writeme_char_uuid_found;
/* To indicate display task that the device has disconnected */
bool device_disconnect_flag = false;

/* Variable to store discovered attribute handles of the custom service */
static cy_ble_gatt_db_attr_handle_t gatt_notify_cccd_attrHandle, gatt_write_val_attrHandle;

/* Constant to store server name and use during scanning */
static const char target_name[11] = {'T', 'h', 'e', 'r', 'm', 'i', 's', 't', 'o', 'r', '\0'};

/* Variable to store user LED status */
// static led_status_t led_status = {CYBSP_LED_STATE_OFF, CYBSP_LED_STATE_OFF};

/* Structure used for GATT writes */
static cy_stc_ble_gattc_write_cmd_req_t write_param_characteristic;

/* Variable to store TX and RX throughput values */
throughput_val_t client_throughput = {0u, 0u};

/* Connection handle to identify the connected peer device */
static cy_stc_ble_conn_handle_t conn_handle;

/* Variable to keep track of the BLE API result */
static cy_en_ble_api_result_t ble_api_result;

/* Variable to hold the address of the peer device */
static cy_stc_ble_gap_bd_addr_t peer_addr;

/* Variable to store MTU size for active BLE connection */
static uint16_t att_mtu_size = CY_BLE_GATT_MTU;

#ifdef EINK_DISPLAY_SHIELD_PRESENT
/* Variable used to refresh the E-ink display every 5 seconds */
static uint8_t count_five_sec;
#endif

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void ble_init(void);
static void ble_stack_event_handler(uint32_t event, void *eventParam, message_struct *mes);
static void ble_controller_interrupt_handler(void);
static void bless_interrupt_handler(void);
static void ble_initialize_gatt_write(cy_ble_gatt_db_attr_handle_t attribute_handle);
static cy_en_ble_api_result_t ble_enable_notification(cy_ble_gatt_db_attr_handle_t attribute_handle);
static cy_en_ble_api_result_t ble_disable_notification(cy_ble_gatt_db_attr_handle_t attribute_handle);
static uint8* adv_parser(uint16_t AD_type, cy_stc_ble_gapc_adv_report_param_t *scan_report, uint8 *adv_type_length);

message_struct *message_data;
/*******************************************************************************
* Function Name: void task_BLE(void *pvParameters)
********************************************************************************
* Summary: FreeRTOS task which handles the BLE activity of the application
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)
*
* Return:
*  None
*
*******************************************************************************/
void task_BLE(void *pvParameters)
{
    /* Variable to store return value from FreeRTOS APIs */
    BaseType_t rtos_api_result;

    /* Variable to store BLE command received from queue */
    ble_command_type_t ble_cmd = BLE_PROCESS_EVENTS;

    /* Remove compiler warning for unused variable */
    message_data = (message_struct*) pvParameters;

    /* Initialize BLE and process any stack events */
    ble_init();

    /* Repeatedly running part of the task */
    for(;;)
    {
        /* Block until a BLE command has been received over bleCmdQ */
        rtos_api_result = xQueueReceive(ble_cmdQ, &ble_cmd, portMAX_DELAY);

        /* Command has been received from bleCmdQ */
        if(rtos_api_result == pdTRUE)
        {
            /* Process the BLE command */
            switch(ble_cmd)
            {
                /* Process BLE stack events */
                case BLE_PROCESS_EVENTS:
                {
                    Cy_BLE_ProcessEvents();
                    break;
                }

                /* Enable BLE notifications on GATT server device */
                case BLE_ENABLE_NOTIFICATION:
                {
                    /* Stop the 1 second timer */
                    xTimerStop(timer_handle,(TickType_t)0);

                    /* Send user LED2 status to the LED queue */
                    // led_status.data_led = CYBSP_LED_STATE_OFF;
                    // xQueueSend(led_cmdQ, &led_status, (TickType_t)0);

                    /* Enable the notifications */
                    if(ble_enable_notification(gatt_notify_cccd_attrHandle) == CY_BLE_SUCCESS)
                    {
                        // tprintf("Notifications Enabled\r\n");
                        /* Start 1 second timer to calculate throughput */
                        xTimerStart(timer_handle, (TickType_t)0);
                    }
                    else
                    {
                        // tprintf("Failed to enable notifications\r\n");
                    }
                    break;
                }

                /* Disable BLE notifications on GATT server device */
                case BLE_DISABLE_NOTIFICATION:
                {
                    /* Stop the 1 second timer */
                    xTimerStop(timer_handle,(TickType_t)0);

                    /* Disable the notifications */
                    if(ble_disable_notification(gatt_notify_cccd_attrHandle) == CY_BLE_SUCCESS)
                    {
                        // tprintf("Notifications Disabled\r\n");
                        /* Start 1 second timer to calculate throughput */
                        xTimerStart(timer_handle, (TickType_t)0);
                    }
                    else
                    {
                        //tprintf("Failed to disable notifications\r\n");
                    }

                    /* Initialize the structure for GATT write */
                    ble_initialize_gatt_write(gatt_write_val_attrHandle);
                    break;
                }

                /* Invalid BLE command */
                default:
                {
                    // //iprintf("Invalid BLE command!");
                    break;
                }
            }
            /* If notifications are disabled and GATT write is enabled */
            if(gatt_write_flag)
            {
                /* Check the BLE stack status */
                if(stack_free)
                {
                    /* Write without response into server GATT Write characteristic */
                    ble_api_result = Cy_BLE_GATTC_WriteWithoutResponse(&write_param_characteristic);
                    if(ble_api_result == CY_BLE_SUCCESS)
                    {
                        /* Increment bytes count only on successful operation */
                        gatt_write_tx_bytes +=  write_param_characteristic.handleValPair.value.len;

                        /* Switch ON LED2 to show data TX */
                        // led_status.data_led = CYBSP_LED_STATE_ON;
                        // xQueueSend(led_cmdQ, &led_status, (TickType_t)0);
                    }
                    else
                    {
                        /* GATT write failed. Switch OFF LED2 to show there is no data TX */
                        // led_status.data_led = CYBSP_LED_STATE_OFF;
                        // xQueueSend(led_cmdQ, &led_status, (TickType_t)0);
                    }
                }
                else
                {
                    /* Stack is busy. Switch OFF LED2 to show data TX stopped */
                    // led_status.data_led = CYBSP_LED_STATE_OFF;
                    // xQueueSend(led_cmdQ, &led_status, (TickType_t)0);
                }
            }
        }
    }
}

/*******************************************************************************
* Function Name: static void ble_init(void)
********************************************************************************
* Summary:
*  This function initializes the BLE Host and Controller, configures BLE
*  interrupt, and registers Application Host callbacks.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
static void ble_init(void)
{
    cy_en_ble_api_result_t ble_api_result = CY_BLE_SUCCESS;

    /* BLESS interrupt configuration structure */
    const cy_stc_sysint_t bless_isr_config =
    {
       /* The BLESS interrupt */
      .intrSrc = bless_interrupt_IRQn,

       /* The interrupt priority number */
      .intrPriority = BLE_INTERRUPT_PRIORITY
    };

    /* Store the pointer to blessIsrCfg in the BLE configuration structure */
    cy_ble_config.hw->blessIsrConfig = &bless_isr_config;

    /* Hook interrupt service routines for BLESS */
    (void) Cy_SysInt_Init(&bless_isr_config, bless_interrupt_handler);

    /* Register the generic callback functions  */
    Cy_BLE_RegisterEventCallback(ble_stack_event_handler);

    /* Register the application Host callback */
    Cy_BLE_RegisterAppHostCallback(ble_controller_interrupt_handler);

    /* Initialize the BLE */
    ble_api_result = Cy_BLE_Init(&cy_ble_config);
    if(ble_api_result != CY_BLE_SUCCESS)
    {
        /* BLE stack initialization failed, check configuration, notify error
         * and halt CPU in debug mode
         */
        // eprintf("Cy_BLE_Init API, errorcode = 0x%X ", ble_api_result);
        vTaskSuspend(NULL);
    }

    /* Enable BLE */
    ble_api_result = Cy_BLE_Enable();
    if(ble_api_result != CY_BLE_SUCCESS)
    {
        /* BLE stack initialization failed, check configuration, notify error
         * and halt CPU in debug mode
         */
        //eprintf("Cy_BLE_Enable API, errorcode = 0x%X ", ble_api_result);
        vTaskSuspend(NULL);
    }
    /* Process BLE events after enabling BLE */
    Cy_BLE_ProcessEvents();
}

/*******************************************************************************
* Function Name: static void bless_interrupt_handler(void)
********************************************************************************
* Summary:
*  Wrapper function for BLESS interrupt
*
* Parameters:
*  None
*
* Return:
*  None
*******************************************************************************/
static void bless_interrupt_handler(void)
{
    /* Process interrupt events generated by the BLE sub-system */
    Cy_BLE_BlessIsrHandler();
}


/*******************************************************************************
* Function Name: static void ble_controller_interrupt_handler(void)
********************************************************************************
* Summary:
*  Call back event function to handle interrupts from BLE Controller
*
* Parameters:
*  None
*
* Return:
*  None
*******************************************************************************/
static void ble_controller_interrupt_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Send command to process BLE events  */
    ble_command_type_t bleCommand = BLE_PROCESS_EVENTS;
    xQueueSendFromISR(ble_cmdQ, &bleCommand, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/************************************************************************************
* Function Name: static void ble_stack_event_handler(uint32_t event, void *eventParam)
*************************************************************************************
*
* Summary: Call back event function to handle various events from the BLE stack.
*
* Parameters:
*  uint32_t event        :    event from BLE stack
*  void eventParam       :    Pointer to the value of event specific parameters
*
* Return:
*  None
************************************************************************************/
static void ble_stack_event_handler(uint32_t event, void *eventParam,  message_struct *mes)
{
    /* Take an action based on the current event */
    switch(event)
    {
        /***********************************************************************
        *                       General Events                                 *
        ***********************************************************************/
        /* This event is received when the BLE stack is Started */
        case CY_BLE_EVT_STACK_ON:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_STACK_ON");
            //tprintf("Press button SW2 on your kit to start Scanning ...\r\n");

            /* Wait till button press to start scanning */
            // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            /* Start scanning for other BLE devices */
            ble_api_result = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
            if( ble_api_result == CY_BLE_SUCCESS)
            {
                //tprintf("Scanning.....\r\n");
                //iprintf("BLE Start Scan API successfull");
            }
            else
            {
                // eprintf("BLE Start Scan API, errorcode = 0x%X", ble_api_result);
            }
            break;
        }

        /* This event indicates BLE stack status. This event is used to handle
         * data throttling which may occur due to continuous GATT write being
         * sent */
        case CY_BLE_EVT_STACK_BUSY_STATUS:
        {
            /* Variable to store status of the stack */
            cy_stc_ble_l2cap_state_info_t stack_status;
            stack_status = *( cy_stc_ble_l2cap_state_info_t*)eventParam;

            if(stack_status.flowState == CY_BLE_STACK_STATE_BUSY)
            {
                /* If stack is busy, stop GATT write */
                stack_free = false;
            }
            else
            {
                /* If stack is free, start GATT write */
                stack_free = true;
            }
            break;
        }

        /* This event is received when there is a timeout */
        case CY_BLE_EVT_TIMEOUT:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_TIMEOUT");
            break;
        }

        /***********************************************************************
        *                       Gap Events                                     *
        ***********************************************************************/
        /* This event indicates that the central device has started or stopped
         * scanning */
        case CY_BLE_EVT_GAPC_SCAN_START_STOP:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GAPC_SCAN_START_STOP");
            break;
        }

        /* This event is triggered every time a device is discovered */
        case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
        {
            cy_stc_ble_gapc_adv_report_param_t *scan_report;
            scan_report = (cy_stc_ble_gapc_adv_report_param_t*)eventParam;

            /* Pointer to store return value from advertisement parser */
            char* peer_name = NULL;
            uint8 name_length = 0u;
            bool  target_found = true;

            /* Process only for Advertisement packets, not on scan response
             * packets */
            // if(scan_report->eventType == CY_BLE_GAPC_SCAN_RSP)
            if(scan_report->eventType == CY_BLE_GAPC_CONN_UNDIRECTED_ADV)
            {
                /* Process the adv packets and get peer name if present in the
                 * packet */
                peer_name = (char*) adv_parser(AD_TYPE_COMPLETE_LOCAL_NAME, scan_report, &name_length);
                if(peer_name  == NULL)
                {
                    target_found =false;
                    break;
                }
                /* Compare peer name with "TPUT" */
                else
                {
                    peer_name[name_length]= '\0';
                    //tprintf("Current device is: %c \r\n", peer_name);
                    target_found = ((strcmp(peer_name, target_name)) ? false : true);
                }

                /* If target is found stop scanning and initiate connection */
                if (target_found)
                {
                    Cy_BLE_GAPC_StopScan();

                    /* Get address and address type of the peer device to
                     * initiate connection */
                    for(uint8 i = 0u; i < CY_BLE_BD_ADDR_SIZE; i++)
                    {
                        peer_addr.bdAddr[i] = scan_report->peerBdAddr[i];
                    }

                    //tprintf("Found Peer Device with address:");
                    /* Print the peer bd address on UART terminal */
                    for(uint8 i = (CY_BLE_BD_ADDR_SIZE); i > 0u; i--)
                    {
                        //tprintf(" %X", peer_addr.bdAddr[i - 1u]);
                    }

                    /* Get the peer address type */
                    peer_addr.type = scan_report->peerAddrType;
                    //tprintf("\r\nScan Completed\r\n");

                    /* Initiate connection with discovered peer device */
                    ble_api_result = Cy_BLE_GAPC_ConnectDevice(&peer_addr, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
                    if(ble_api_result == CY_BLE_SUCCESS)
                    {
                        //iprintf(" SUCCESS : Connection Initiation ");
                    }
                    else
                    {
                        //eprintf(" Connection Initiation API, errorcode = 0x%X", ble_api_result);
                    }
                }
            }
            break;
        }

        /* This event is generated at the GAP Peripheral end after connection
         * is completed with peer Central device */
        case CY_BLE_EVT_GAP_DEVICE_CONNECTED:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GAP_DEVICE_CONNECTED");
            /* Variable to store connection parameters after GAP connection */
            cy_stc_ble_gap_connected_param_t* conn_param;
            conn_param = (cy_stc_ble_gap_connected_param_t*)eventParam;

            /* Variable to store values to update PHY to 2M */
            cy_stc_ble_set_phy_info_t phy_pram;
            phy_pram.allPhyMask = CY_BLE_PHY_NO_PREF_MASK_NONE;
            phy_pram.bdHandle = conn_handle.bdHandle;
            phy_pram.rxPhyMask = CY_BLE_PHY_MASK_LE_1M;
            phy_pram.txPhyMask = CY_BLE_PHY_MASK_LE_1M;

            /* Reset the connection status flag upon reconnection */
            device_disconnect_flag = false;

            /* Reset the notify flag and stack_free flag after disconnection */
            gatt_write_flag = false;
            stack_free = true;

            /* Store the connection interval value */
            conn_interval = (conn_param->connIntv) * CONN_INTERVAL_MULTIPLIER;
            //tprintf("Connection Interval is: %f ms \r\n", conn_interval);

            /* Send user LED1 status to the LED queue */
            // led_status.conn_led = CYBSP_LED_STATE_ON;
            // xQueueSend(led_cmdQ, &led_status, (TickType_t)0);

            /* Function call to set PHY to 2M */
            ble_api_result = Cy_BLE_SetPhy(&phy_pram);
            if(ble_api_result == CY_BLE_SUCCESS)
            {
                //iprintf("Set PHY to 2M API successfull \r\n");
                //tprintf("Request sent to switch PHY to 2M \r\n");
            }
            else
            {
                //eprintf("Set PHY API, errorcode = 0x%X", ble_api_result);
            }
            break;
        }

        /* This event is generated when the device is disconnected from remote
         * device or fails to establish connection */
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GAP_DEVICE_DISCONNECTED");

            /* Stop the timer after device disconnection */
            xTimerStop(timer_handle,(TickType_t)0);

            /* Set the device disconnect flag */
            device_disconnect_flag = true;

            /* Reset the button flag to avoid any action due to button press
             * after disconnection */
            button_flag = false;

            /* Change the mode flag value to GATT notifications */
            mode_flag = GATT_NOTIF_STOC;

            /* Reset the flags after disconnection */
            notify_cccd_uuid_found = false;
            writeme_char_uuid_found =false;
            gatt_write_flag = false;

            /* Send user LED1 and LED2 status to the LED queue */
            // led_status.conn_led = CYBSP_LED_STATE_OFF;
            // led_status.data_led = CYBSP_LED_STATE_OFF;
            // xQueueSend(led_cmdQ, &led_status, (TickType_t)0);

            //tprintf("Device Disconnected!!!\r\n");
            //tprintf("Press button SW2 on your kit to start Scanning...\r\n");

            /* Start scanning again after button press */
            scan_flag = false;
            /* Wait till button press to start scanning */
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            ble_api_result = Cy_BLE_GAPC_StartScan(CY_BLE_SCANNING_FAST, CY_BLE_CENTRAL_CONFIGURATION_0_INDEX);
            if(ble_api_result == CY_BLE_SUCCESS)
            {
                //tprintf("Scanning.....\r\n");
                //iprintf("BLE Start Scan API successfull");
            }
            else
            {
                //eprintf("BLE Start Scan API, errorcode = 0x%X", ble_api_result);
            }
            break;
        }

        /* This event is generated when PHY is updated during an active
         * connection */
        case CY_BLE_EVT_PHY_UPDATE_COMPLETE:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_PHY_UPDATE_COMPLETE");

            cy_stc_ble_events_param_generic_t *generic_param;
            cy_stc_ble_phy_param_t *currentPHY;
            generic_param = (cy_stc_ble_events_param_generic_t*)eventParam;

            /* Local variable to set MTU for the active connection */
            cy_stc_ble_gatt_xchg_mtu_param_t mtuParam = {conn_handle, CY_BLE_GATT_MTU};

            /* GenericParam has to be cast to cy_stc_ble_phy_param_t to get
             * TX and RX PHY */
            currentPHY = (cy_stc_ble_phy_param_t*)(generic_param->eventParams);

            /* Print the RX PHY selected on UART terminal */
            switch(currentPHY->rxPhyMask)
            {
                case CY_BLE_PHY_MASK_LE_1M:
                //tprintf("Selected Rx PHY: 1M\r\n");
                break;

                case CY_BLE_PHY_MASK_LE_2M:
                //tprintf("Selected Rx PHY: 2M\r\n");
                break;

                case CY_BLE_PHY_MASK_LE_CODED:
                //tprintf("Selected Rx PHY: LE Coded\r\n");
                break;
            }

            /* Print the TX PHY selected on UART terminal */
            switch(currentPHY->txPhyMask)
            {
                case CY_BLE_PHY_MASK_LE_1M:
                //tprintf("Selected Tx PHY: 1M\r\n");
                break;

                case CY_BLE_PHY_MASK_LE_2M:
                //tprintf("Selected Tx PHY: 2M\r\n");
                break;

                case CY_BLE_PHY_MASK_LE_CODED:
                //tprintf("Selected Tx PHY: LE Coded\r\n");
                break;
            }

            /* Initiate MTU exchange request */
            ble_api_result = Cy_BLE_GATTC_ExchangeMtuReq(&mtuParam);
            if(ble_api_result == CY_BLE_SUCCESS)
            {
                //iprintf("GATT Exchange MTU Request successfull");
            }
            else
            {
                //eprintf("GATT Exchange MTU Request API, errorcode = 0x%X", ble_api_result);
            }
            break;
        }

        /* This event is generated when connection parameter update is
         * requested. If the request is accepted by the Central, this event is
         * generated on both the devices. If request is rejected, this event is
         * not generated */
        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE");
            cy_stc_ble_gap_conn_param_updated_in_controller_t new_conn_params;
            new_conn_params = *((cy_stc_ble_gap_conn_param_updated_in_controller_t*)eventParam);

            /* Store the new connection interval */
            conn_interval = (new_conn_params.connIntv) * CONN_INTERVAL_MULTIPLIER;

            //tprintf("Updated Connection interval : %f ms\r\n", conn_interval);
            break;
        }

        /***********************************************************************
        *                       GATT Events                                     *
        ***********************************************************************/
        /* This event is generated at the GAP Peripheral end after connection
         * is completed with peer Central device
         */
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            conn_handle = *((cy_stc_ble_conn_handle_t*)eventParam);
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATT_CONNECT_IND");
            //tprintf("GATT connected\r\n");
            /* Start 1 second timer to calculate throughput */
            xTimerStart(timer_handle, (TickType_t)0);
            break;
        }

        /* This event indicates that the GATT is disconnected.*/
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATT_DISCONNECT_IND");
            break;
        }

        /* This event is generated when response is received from GATT Server
         * for MTU exchange request */
        case CY_BLE_EVT_GATTC_XCHNG_MTU_RSP:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATTC_XCHNG_MTU_RSP");
            cy_stc_ble_gatt_xchg_mtu_param_t *mtu_xchg_resp =
                                  (cy_stc_ble_gatt_xchg_mtu_param_t*)eventParam;

            cy_stc_ble_gattc_find_info_req_t find_req_param;
            find_req_param.connHandle = conn_handle;
            /* 0x0001 - 0xFFFF is the range of attribute handles that can be present in a GATT database*/
            find_req_param.range.startHandle = 0x01;
            find_req_param.range.endHandle = 0xFFFF;

            //tprintf("Negotiated MTU size: %d\r\n", mtu_xchg_resp->mtu);
            if(mtu_xchg_resp->mtu > CY_BLE_GATT_MTU)
            {
                att_mtu_size = CY_BLE_GATT_MTU;
            }
            else
            {
                att_mtu_size = mtu_xchg_resp->mtu;
            }

            cy_stc_ble_gattc_read_by_type_req_t param;
            param.range.startHandle=0x0063;
            param.range.endHandle=0x0064;
            param.connHandle=conn_handle;
            param.uuidFormat=CY_BLE_GATT_16_BIT_UUID_FORMAT;
            // param.uuid.uuid16=0x2A37;
            param.uuid.uuid16=0x2A6E;
            ble_api_result= Cy_BLE_GATTC_DiscoverCharacteristicByUuid(&param);
            if(ble_api_result == CY_BLE_SUCCESS)
            {
                //tprintf("ReadCharacteristic value success  \r\n");
            }
            else
            {
                //tprintf("ReadCharacteristicValue, errorcode = 0x%X \r\n", ble_api_result);
            }
            break;
        }

        /* This event indicates that the 'Find Information Response' is received
         * from GATT Server device */
        case CY_BLE_EVT_GATTC_FIND_INFO_RSP:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATTC_FIND_INFO_RSP");

            cy_stc_ble_gattc_find_info_rsp_param_t find_info_param;

            find_info_param = *((cy_stc_ble_gattc_find_info_rsp_param_t*)eventParam);
            int index = 0u;
            uint8_t notify_cccd_uuid[NOTIFY_CCCD_SIZE] = NOTIFY_CCCD_UUID;
            uint8_t writeme_char_uuid[WRITEME_CHAR_UUID_SIZE] = WRITEME_CHAR_UUID;

            /* Search for 'WriteMe' characteristic UUID and get the attribute
             * handle */
            if(!writeme_char_uuid_found)
            {
                for (index = 0u; index < WRITEME_CHAR_UUID_SIZE; index++)
                {
                    writeme_char_uuid_found =(find_info_param.handleValueList.list[index + 2]
                                        != writeme_char_uuid[index]) ? false : true;
                }
                if(writeme_char_uuid_found)
                {
                    gatt_write_val_attrHandle= find_info_param.handleValueList.list[0]
                                | (find_info_param.handleValueList.list[1] << 8 );
                    //iprintf("Att Handle of Custom characteristic: 0x%X", gatt_write_val_attrHandle);
                }
            }

            /* Search for the CCCD UUID (0x2902) to identify the the CCCD
             * attribute and get the handle */
            if(!notify_cccd_uuid_found)
            {
                for(index = 0u; index < NOTIFY_CCCD_SIZE; index++)
                {
                    notify_cccd_uuid_found = (find_info_param.handleValueList.list[index + 2] !=
                                                notify_cccd_uuid[index]) ? false : true;
                }
                if(notify_cccd_uuid_found)
                {
                    gatt_notify_cccd_attrHandle= find_info_param.handleValueList.list[0]
                                    | (find_info_param.handleValueList.list[1] << 8 );
                    //iprintf("Att Handle of Custom characteristic: 0x%X",gatt_notify_cccd_attrHandle);
                }
            }


            /* If both the UUIDs are found, stop discovering characteristics */
            if((notify_cccd_uuid_found == true) && (writeme_char_uuid_found == true))
            {
                cy_stc_ble_gattc_stop_cmd_param_t stop_cmd;
                stop_cmd.connHandle = conn_handle;
                Cy_BLE_GATTC_StopCmd(&stop_cmd);
            }
            break;
        }

        case CY_BLE_EVT_GATTC_READ_RSP: 
        {
            cy_stc_ble_gattc_read_rsp_param_t read_info_param;
            read_info_param = *((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
            message_data->pulse = read_info_param.value.val;
            //tprintf("Characteristick value is = 0x%X \r\n", read_info_param.value.val);
            break;
        }

        case CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP:
        {
            cy_stc_ble_gattc_read_by_type_rsp_param_t read_info_param;
            read_info_param = *((cy_stc_ble_gattc_read_by_type_rsp_param_t*)eventParam);
            //tprintf("Characteristick value is = 0x%X \r\n", read_info_param.attrData.attrValue);
            break;
        }

        /* This event indicates that the GATT long procedure has ended and the
         * BLE Stack will not send any further requests to the peer */
        case CY_BLE_EVT_GATTC_LONG_PROCEDURE_END:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATTC_LONG_PROCEDURE_END");
            break;
        }

        /* This event is received by the GATT Client when the GATT Server cannot
         * perform the requested operation and sends out an error response */
        case CY_BLE_EVT_GATTC_ERROR_RSP:
        {
            cy_stc_ble_gatt_err_param_t error_rsp;
            error_rsp = *((cy_stc_ble_gatt_err_param_t*)eventParam);
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATTC_ERROR_RSP");
            /* To suppress compiler warning */
            (void)error_rsp;
            break;
        }

        /* This event indicates that a GATT group procedure has stopped or
         * completed. This event occurs only if the application has called the
         * Cy_BLE_GATTC_StopCmd() function. */
        case CY_BLE_EVT_GATTC_STOP_CMD_COMPLETE:
        {
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATTC_STOP_CMD_COMPLETE");
            //tprintf("Attributes discovery complete\r\n");

            /* Enable notification for the custom throughput service */
            ble_enable_notification(gatt_notify_cccd_attrHandle);
            break;
        }

        /* This event indicates that the 'Write Response' is received from GATT
         * Server device. */
        case CY_BLE_EVT_GATTC_WRITE_RSP:
        {
            button_flag = true;
            //iprintf("BLE Stack Event: CY_BLE_EVT_GATTC_WRITE_RSP");
            break;
        }

        /* This event indicates that Notification data is received from GATT
         * Server device. */
        case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
        {
            cy_stc_ble_gattc_handle_value_ntf_param_t *notif_parameter;
            notif_parameter = (cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam;

            /* Increment the notification byte count */
            notif_rx_bytes +=  notif_parameter->handleValPair.value.len;
            int16_t temperature = (notif_parameter->handleValPair.value.val[1] << 8u) | notif_parameter->handleValPair.value.val[0];
            if (temperature != 0) {
                //xSemaphoreTake(message_data->mutex, portMAX_DELAY);
                message_data->temp = temperature;
                //xSemaphoreGive(message_data->mutex);
            }
            break;
        }

        /***********************************************************************
        *                       L2CAP Events                                   *
        ***********************************************************************/
        /* This event indicates that the connection parameter update request
         * is receivedfrom the remote device. */
        case CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
        {
            cy_stc_ble_gap_conn_update_param_info_t* conn_param;
            conn_param = (cy_stc_ble_gap_conn_update_param_info_t*)eventParam;

            cy_stc_ble_l2cap_conn_update_param_rsp_info_t conn_param_update_rsp
                                                    = {true, conn_handle.bdHandle};

            /* Check minimum and maximum connection interval requested by
             * peripheral device */
            //tprintf("Connection Interval Update request received\r\n");
            if((conn_param->connIntvMin >= 54u) && (conn_param->connIntvMax <= 60u))
            {
                conn_param_update_rsp.result = false;
                //tprintf("Connection interval update request accepted\r\n");
                Cy_BLE_L2CAP_LeConnectionParamUpdateResponse(&conn_param_update_rsp);
            }
            else
            {
                conn_param_update_rsp.result = true;
                //tprintf("Connection interval update request rejected\r\n");
                Cy_BLE_L2CAP_LeConnectionParamUpdateResponse(&conn_param_update_rsp);
            }
            break;
        }
        /***********************************************************************
        *                           Other Events                               *
        ***********************************************************************/
        default:
        {
            //iprintf("Other BLE event: 0x%X", (uint32_t)event);
            break;
        }
    }
}

/***********************************************************************************************************
* Function Name: cy_en_ble_api_result_t ble_enable_notification(cy_ble_gatt_db_attr_handle_t attribute_handle)
************************************************************************************************************
*
* Summary: This function writes to the CCCD of custom characteristic
* 'GATT Notify' and enables notifications.
*
* Parameters:
*  cy_ble_gatt_db_attr_handle_t attribute_handle : peer CCCD attribute details
*
* Return:
*  cy_en_ble_api_result_t : error codes received as API result
*
***********************************************************************************************************/
static cy_en_ble_api_result_t ble_enable_notification(cy_ble_gatt_db_attr_handle_t attribute_handle)
{
    cy_stc_ble_gattc_write_req_t write_param_notification;

    write_param_notification.connHandle = conn_handle;
    write_param_notification.handleValPair.attrHandle = attribute_handle;
    write_param_notification.handleValPair.value.val = CCCD_VALUE;
    write_param_notification.handleValPair.value.len = NOTIFY_CCCD_SIZE;

    /* Clear TX/RX data byte count */
    gatt_write_tx_bytes = 0u;
    notif_rx_bytes = 0u;

    /* Enable notification */
    ble_api_result = Cy_BLE_GATTC_WriteCharacteristicValue(&write_param_notification);

    return(ble_api_result);
}

/************************************************************************************************************
* Function Name: cy_en_ble_api_result_t ble_disable_notification(cy_ble_gatt_db_attr_handle_t attribute_handle)
*************************************************************************************************************
*
* Summary: This function writes to the CCCD of custom characteristic
*  'GATT Notify' and disables notifications.
*
* Parameters:
*  cy_ble_gatt_db_attr_handle_t attribute_handle : peer CCCD attribute details
*
* Return:
*  cy_en_ble_api_result_t : error codes received as API result
*
************************************************************************************************************/
static cy_en_ble_api_result_t ble_disable_notification(cy_ble_gatt_db_attr_handle_t attribute_handle)
{
    cy_stc_ble_gattc_write_req_t write_param_notification;

    write_param_notification.connHandle = conn_handle;
    write_param_notification.handleValPair.attrHandle = attribute_handle;
    write_param_notification.handleValPair.value.val = &CCCD_VALUE[1];
    write_param_notification.handleValPair.value.len = NOTIFY_CCCD_SIZE;

    /* Clear TX/RX data byte count */
    gatt_write_tx_bytes = 0u;
    notif_rx_bytes = 0u;

    /* Disable Notification */
    ble_api_result = Cy_BLE_GATTC_WriteCharacteristicValue(&write_param_notification);

    return(ble_api_result);
}

/******************************************************************************************
* Function Name: void ble_initialize_gatt_write(cy_ble_gatt_db_attr_handle_t attribute_handle)
*******************************************************************************************
*
* Summary: This function stores the attribute handle and custom data for GATT
*  write.
*
* Parameters:
*  cy_ble_gatt_db_attr_handle_t attribute_handle : peer attribute details
*
* Return:
*  None
*
******************************************************************************************/
static void ble_initialize_gatt_write(cy_ble_gatt_db_attr_handle_t attribute_handle)
{
    /* Variable to hold custom data to write into server GATT database.
     * Size of the array is derived from MTU exchanged.
     * Packet Length = (ATT_MTU - ATT_OPCODE(1 byte) - ATT_HANDLE(2 bytes)) */
    uint8_t custom_data_length = (att_mtu_size -3u);
    uint8_t custom_write_data[CY_BLE_GATT_MTU] = {0};

    cy_stc_ble_gatt_value_t writeValue = {custom_write_data, custom_data_length, custom_data_length};

    write_param_characteristic.connHandle = conn_handle;
    write_param_characteristic.handleValPair.attrHandle = attribute_handle;
    write_param_characteristic.handleValPair.value = writeValue;
}

/***************************************************************************************************************************
* Function Name: uint8* adv_parser(uint16_t AD_type,cy_stc_ble_gapc_adv_report_param_t* scan_report, uint8* adv_type_length)
****************************************************************************************************************************
*
* Summary: This function searches adv packets for the given type.
*
* Parameters:
*  uint16_t AD_type : the type of value to be discovered
*  cy_stc_ble_gapc_adv_report_param_t* scan_report : advertisement report
*                                                    parameter
*  uint8* adv_type_length : length of discovered value
*
* Return:
*  uint8* : Pointer to the value discovered in ADV packet
*
***************************************************************************************************************************/
static uint8_t* adv_parser(uint16_t adv_type, cy_stc_ble_gapc_adv_report_param_t*
                         scan_report, uint8_t* adv_type_length)
{
    uint8_t length = 0u;
    uint8_t* pName = NULL;

    for(uint8_t i = 0u; i < scan_report->dataLen; i += (length+1))
    {
        length = scan_report->data[i];
        if(scan_report->data[i+1] == adv_type)
        {
            pName = &scan_report->data[i+2];
            *adv_type_length = length - 1;
            return(pName);
        }
    }
    return((uint8_t*)NULL);
}

/*******************************************************************************
* Function Name: void rtos_timer_cb(TimerHandle_t timer_handle)
********************************************************************************
* Summary: This is a freeRTOS Software timer callback function. It calculates
*          the throughput and displays the value over UART terminal.
*
* Parameters:
*  TimerHandle_t timer_handle: parameter defined during timer creation (unused)
*
* Return:
*  None
*******************************************************************************/
void rtos_timer_cb(TimerHandle_t timer_handle)
{
    /* Avoid warning for unused parameter */
    (void)timer_handle;

    client_throughput.rx = 0u;
    client_throughput.tx = 0u;

    /* Calculate TX and RX throughput
     *
     * throughput(kbps) = (number of bytes sent/received in 1 second) * 8(bits)
     *                    -----------------------------------------------------
     *                                   1 second * 2^10 (kilo)
     */

    if(notif_rx_bytes != 0u)
    {
        /* Number of bytes  */
        client_throughput.rx = (notif_rx_bytes) >> 7u;
        notif_rx_bytes = 0u;
        //tprintf("GATT NOTIFICATION: Client Throughput Rx = %lu kbps\r\n", client_throughput.rx);
    }

    if(gatt_write_tx_bytes != 0u)
    {
        client_throughput.tx = (gatt_write_tx_bytes) >> 7u;
        gatt_write_tx_bytes = 0u;
        //tprintf("GATT WRITE:        Client Throughput Tx = %lu kbps\r\n", client_throughput.tx);

    }
}

/*******************************************************************************
* Function Name: throughput_val_t* get_throughput(void)
********************************************************************************
* Summary: This function returns the pointer to the tx and rx throughput values.
*
* Parameters:
*  None
*
* Return:
*  throughput_val_t* gatt_throughput: BLE gatt throughput calculated in ble task.
*******************************************************************************/
throughput_val_t* get_throughput(void)
{
    return(&client_throughput);
}
/* [] END OF FILE */