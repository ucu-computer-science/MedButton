#ifndef C_DATA_STRUCT_H
#define C_DATA_STRUCT_H

#include "FreeRTOS.h"
#include "semphr.h"

struct message_struct {
    float latitude;
    float longitude;
    char resultTime[11];
    int pulse;
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t semaphore_gprs;
    SemaphoreHandle_t semaphore_lora;
};

typedef struct message_struct message_struct;

#endif //C_DATA_STRUCT_H