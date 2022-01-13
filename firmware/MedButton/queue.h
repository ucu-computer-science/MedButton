#include <stdio.h>
#include <string.h>

#ifndef QUEUE_HEADER
#define QUEUE_HEADER

#define QUEUE_SIZE    5
#define MESSAGE_SIZE 40
#define TIME_STR_SIZE 40


void add_time(char info_queue[QUEUE_SIZE][MESSAGE_SIZE], const char *element);
void add_latitute(float info_queue[QUEUE_SIZE], float element);
void add_longtitude(float info_queue[QUEUE_SIZE], float element);
void add_pulse(int info_queue[QUEUE_SIZE], int element);
//void print_queue(int info_queue[QUEUE_SIZE]);
#endif
