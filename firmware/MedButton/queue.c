#include "queue.h"


int idx = 0;

int main(){
    char info_queue[QUEUE_SIZE][MESSAGE_SIZE];
    char one[] = "gpgga";

    add_to_queue(info_queue, one);
    add_to_queue(info_queue, "hello");
    add_to_queue(info_queue, "third");
    add_to_queue(info_queue, "forth");
    add_to_queue(info_queue, "Fifth");
    print_queue(info_queue);
    add_to_queue(info_queue, "OVERRIDE1");
    add_to_queue(info_queue, "OVERRIDE2");
    print_queue(info_queue);
    return 0;
}

void add_to_queue(char info_queue[QUEUE_SIZE][MESSAGE_SIZE], const char *element){
    strcpy(info_queue[idx], element);
    idx++;
    if (idx == QUEUE_SIZE) {
        idx = 0;
    }
}

void print_queue(char info_queue[QUEUE_SIZE][MESSAGE_SIZE]){
    int i = idx;
    do {
        if(info_queue[i] != 0){
            printf("%s ", info_queue[i]);
        }
        ++i;
        if(i == QUEUE_SIZE){
            i = 0;
        }
    } while (i != idx);
    printf("\n");
}