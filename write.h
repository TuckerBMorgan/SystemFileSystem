#ifndef WRITE_H_
#define WRITE_H_
#include <stdlib.h>

//all the possible ways in which this function could fail
typedef enum WriteStatusCode_e {
    NormalWrite,
    BadWrite_length_greater_then_size,
    BadWrite_start_not_within_bounds
} WriteStatusCode;


WriteStatusCode write(size_t start, size_t length, char* data, char* buffer, size_t buffer_size);
WriteStatusCode write_zero(size_t start, size_t length, char* buffer, size_t buffer_size);

#endif