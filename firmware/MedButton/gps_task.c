#include "gps_task.h"
#include <stdlib.h>


const cyhal_uart_cfg_t uart_config =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = NULL,
    .rx_buffer_size = 0,
};

const char disable_nmea[10][30] = {
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

    // for (size_t i = 0; i < 10; i++) {
    //     uart_send_cmd_and_wait(disable_nmea[i], 30, &gps_uart);
    // }

    /* GPS TASK */
    uint8_t c = 0;
    int k, index;
    char nmea[20], time[20];
    char lon[20], lat[20];
    int len;
    float ignore;
    
//     read data and find GPGGA part
    
    cyhal_system_delay_ms(5000);
    for (;;) {
        if (cyhal_uart_readable(&gps_uart) > 80){
            cyhal_uart_read(&gps_uart, (void*)rx_buf, &rx_length);

            char *gpgga = strstr(rx_buf, "GPGGA");
            if (strlen(gpgga) > 10) {
                char delim[] = ",";

                char *ptr = strtok(gpgga, delim);

                ptr = strtok(NULL, delim);
                // time
                UTCtoKyivTime(ptr, message_data)
                ptr = strtok(NULL, delim);
                // longtitude
                message_data->longitude = NMEAtoDecimalDegrees(lon, c);
                ptr = strtok(NULL, delim);
                // пропускаєм, бо нам не треба одна буква тут
                ptr = strtok(NULL, delim);
                // latitude
                message_data->latitude = NMEAtoDecimalDegrees(lat, c);
            }
            cyhal_uart_clear(&gps_uart);
        }
        cyhal_system_delay_ms(5000);
    }

    //read and parse raw NMEA sentences

//     for (;;)
//     {
//         if (cyhal_uart_getc(&gps_uart, &c, 0) == CY_RSLT_SUCCESS)
//         {
//             if (c)
//             {
//                 if (c == '$')
//                 {
//                     for (k = 0; k < 5; k++)
//                     {
//                         cyhal_uart_getc(&gps_uart, &c, 0);
//                         while (!(c))
//                         {
//                             cyhal_uart_getc(&gps_uart, &c, 0);
//                         }
//                         nmea[k] = c; // G + P + G + G + A
//                     }

//                     if (strstr(nmea, "GPGGA"))
//                     {
//                         memset(lon, 0, sizeof lon);
//                         memset(lat, 0, sizeof lat);
//                         memset(time, 0, sizeof time);
//                         index = 0;
//                         cyhal_uart_getc(&gps_uart, &c, 0);
//                         cyhal_uart_getc(&gps_uart, &c, 0);
//                         while (!(c == ','))
//                         {
//                             time[index] = c;
//                             ++index;
//                             cyhal_uart_getc(&gps_uart, &c, 0);
//                         }
//                         index = 0;
//                         cyhal_uart_getc(&gps_uart, &c, 0);
//                         while (!(c == ','))
//                         {
//                             lat[index] = c;
//                             ++index;
//                             cyhal_uart_getc(&gps_uart, &c, 0);
//                         }
//                         cyhal_uart_getc(&gps_uart, &c, 0);

//                         index = 0;
//                         cyhal_uart_getc(&gps_uart, &c, 0);
//                         cyhal_uart_getc(&gps_uart, &c, 0);
//                         while (!(c == ','))
//                         {
//                             lon[index] = c;
//                             ++index;
//                             cyhal_uart_getc(&gps_uart, &c, 0);
//                         }
//                         cyhal_uart_getc(&gps_uart, &c, 0);
//                         sscanf(lon, "%f %n", &ignore, &len);

//                         /// check if new longitude isn't empty
//                         if (lon[0] != '\0') {
//                             xSemaphoreTake(message_data->mutex, portMAX_DELAY);
//                             message_data->longitude = NMEAtoDecimalDegrees(lon, c);
//                             message_data->latitude = NMEAtoDecimalDegrees(lat, c);
//                             UTCtoKyivTime(time, message_data);
//                             xSemaphoreGive(message_data->mutex);
//                         }
//                         cyhal_system_delay_ms(5000);
//                     }
//                 }
//             }
//         }
//     }
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
