#ifndef C_DATA_STRUCT_H
#define C_DATA_STRUCT_H

struct message_struct {
    float latitude;
    float longitude;
    char resultTime[11];
};

typedef struct message_struct message_struct;

#endif //C_DATA_STRUCT_H