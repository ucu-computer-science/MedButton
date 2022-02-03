#include "gps_task.h"
#include "info_queue.h"
#include <stdlib.h>


const cyhal_uart_cfg_t uart_config =
{
    .data_bits = 8,
    .stop_bits = 1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = NULL,
    .rx_buffer_size = 0,
};


void task_gps(void* param) {
    /* uart GPS */
    message_struct *message_data = (message_struct*) param;
//    message_data->longitude = 0.0;
//    message_data->latitude = 0.0;

    cy_rslt_t result;

    cyhal_uart_t gps_uart;

    result = cyhal_uart_init(&gps_uart, P10_1, P10_0, NULL, &uart_config);

    if (result == CY_RSLT_SUCCESS)
    {
        result = cyhal_uart_set_baud(&gps_uart, 9600, NULL);
    }


    /* GPS TASK */
    char c = 0;
    // int k, index;
    char nmea[20];
    // char nmea[20], time[20];
    char lon[20], lat[20];
    // int len;
    // float ignore;
    
    uint64_t uniqueId;

    uniqueId = Cy_SysLib_GetUniqueId();

    snprintf(message_data->unique_id, sizeof(message_data->unique_id), "%lu%lu", uniqueId);
    
    
//     read data from uart
    cyhal_system_delay_ms(5000);

    char rx_buf[64];
    size_t rx_length = 64;

    for (;;) {
        if (cyhal_uart_readable(&gps_uart) > 80){
            cyhal_uart_read(&gps_uart, (void*)rx_buf, &rx_length);

//             get only GPGGA part
            char *gpgga = strstr(rx_buf, "GPGGA");
            if (strlen(gpgga) > 10) {
                char delim[] = ",";

                char *ptr = strtok(gpgga, delim);

                ptr = strtok(NULL, delim);
                // time
                char time[11];
                UTCtoKyivTime(ptr, time);
                add_time(message_data->resultTime, time);
                ptr = strtok(NULL, delim);
                // longtitude
                add_longtitude(message_data->longitude, NMEAtoDecimalDegrees(lon, c));
                c = strtok(NULL, delim);
                // here would be letter "N" which we do not need
                ptr = strtok(NULL, delim);
                // latitude
                add_latitute(message_data->latitude, NMEAtoDecimalDegrees(lat, c));
            }
            cyhal_uart_clear(&gps_uart);
        }
        cyhal_system_delay_ms(5000);
    }

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

char *UTCtoKyivTime(const char *utcTime, char *time)
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
    sprintf(time, "%d:%c%c:%c%c", number, utcTime[2], utcTime[3], utcTime[4], utcTime[5]);
    return 0;
}