#ifndef READ_H_
#define READ_H_
#include <stdlib.h>

typedef enum ReadStatusCode_e {
    NormalRead,
    BadRead_number_of_bytes_exceeds_block_size,
} ReadStatusCode;

void read(size_t start, size_t length, char* buffer, char* read_buffer);
int read_block_value(int block_index, char* drive);
ReadStatusCode read_block(int block_index, char* buffer, size_t place_in_buffer, size_t number_of_bytes_to_read, char* file);
void read_file(int start_cluster, size_t number_of_bytes_to_read, char* drive, char* buffer);
#endif