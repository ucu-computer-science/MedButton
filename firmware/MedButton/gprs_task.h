#ifndef C_GPRS_TASK_H
#define C_GPRS_TASK_H

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "data_struct.h"

void task_gprs(void* param);
void uart_send_cmd_and_wait(char *cmd, size_t cmd_len, cyhal_uart_t *uart_obj);
void wait_uart_free(cyhal_uart_t *uart_obj);

#endif //C_GPRS_TASK_H