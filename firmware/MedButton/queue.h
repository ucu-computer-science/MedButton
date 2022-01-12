#include <stdio.h>
#include <string.h>

#ifndef QUEUE_HEADER
#define QUEUE_HEADER

#define QUEUE_SIZE    5
#define MESSAGE_SIZE 40


void add_to_queue(char info_queue[QUEUE_SIZE][MESSAGE_SIZE], const char *element);
void print_queue(char info_queue[QUEUE_SIZE][MESSAGE_SIZE]);
#endif