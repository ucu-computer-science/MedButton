#ifndef C_DATA_STRUCT_H
#define C_DATA_STRUCT_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

struct message_struct {
    float latitude[QUEUE_SIZE];
    float longitude[QUEUE_SIZE];
    char resultTime[QUEUE_SIZE][TIME_STR_SIZE];
    int pulse[QUEUE_SIZE];
    char unique_id[20];
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t semaphore_gprs;
    SemaphoreHandle_t semaphore_lora;
};

typedef struct message_struct message_struct;

#endif //C_DATA_STRUCT_H
