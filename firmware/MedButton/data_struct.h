#ifndef C_DATA_STRUCT_H
#define C_DATA_STRUCT_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "info_queue.h"

struct message_struct {
    float latitude[QUEUE_SIZE];
    float longitude[QUEUE_SIZE];
    char resultTime[QUEUE_SIZE][TIME_STR_SIZE];
    uint8_t pulse;
    char unique_id[20];
    int16_t temp;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t semaphore_gprs;
    SemaphoreHandle_t semaphore_lora;
    struct tm current_time;
};

typedef struct message_struct message_struct;

#endif //C_DATA_STRUCT_H