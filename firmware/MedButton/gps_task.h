#ifndef C_GPS_TASK_H
#define C_GPS_TASK_H

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "data_struct.h"

void task_gps(void* param);
char *UTCtoKyivTime(const char *utcTime, message_struct *message_data);
float NMEAtoDecimalDegrees(const char *degree, char quadrant);

#endif //C_GPS_TASK_H