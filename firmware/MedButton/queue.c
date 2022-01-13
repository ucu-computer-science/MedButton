#include "main.h"


int latitude_idx = 0;
int longitude_idx = 0;
int time_idx = 0;
int pulse_idx = 0;

int main(){
//    char info_queue[QUEUE_SIZE][MESSAGE_SIZE];
//    char one[] = "gpgga";
//
//    add_time(info_queue, one);
//    add_time(info_queue, "hello");
//    add_time(info_queue, "third");
//    add_time(info_queue, "forth");
//    add_time(info_queue, "Fifth");
//    print_queue(info_queue);
//    add_time(info_queue, "OVERRIDE1");
//    add_time(info_queue, "OVERRIDE2");
//    print_queue(info_queue);

//    int latitude[QUEUE_SIZE];
//    add_pulse(latitude, 1);
//    add_pulse(latitude, 2);
//    add_pulse(latitude, 3);
//    add_pulse(latitude, 4);
//    add_pulse(latitude, 5);
//    print_queue(latitude);
//
//    add_pulse(latitude, 6);
//    add_pulse(latitude, 7);
//    add_pulse(latitude, 8);
//    print_queue(latitude);
    return 0;
}

void add_time(char info_queue[QUEUE_SIZE][MESSAGE_SIZE], const char *element){
    strcpy(info_queue[time_idx], element);
    time_idx++;
    if (time_idx == QUEUE_SIZE) {
        time_idx = 0;
    }
}

void add_latitute(float info_queue[QUEUE_SIZE], float element){
    info_queue[latitude_idx] = element;
    latitude_idx++;
    if (latitude_idx == QUEUE_SIZE) {
        latitude_idx = 0;
    }
}

void add_longtitude(float info_queue[QUEUE_SIZE], float element){
    info_queue[longitude_idx] = element;
    longitude_idx++;
    if (longitude_idx == QUEUE_SIZE) {
        longitude_idx = 0;
    }
}

void add_pulse(int info_queue[QUEUE_SIZE], int element){
    info_queue[pulse_idx] = element;
    pulse_idx++;
    if (pulse_idx == QUEUE_SIZE) {
        pulse_idx = 0;
    }
}

//void print_queue(int info_queue[QUEUE_SIZE]){
//    int i = pulse_idx;
//    do {
//        if(info_queue[i] != 0){
//            printf("%d ", info_queue[i]);
//        }
//        ++i;
//        if(i == QUEUE_SIZE){
//            i = 0;
//        }
//    } while (i != pulse_idx);
//    printf("\n");
//}
