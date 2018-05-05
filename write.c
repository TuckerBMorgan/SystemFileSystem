#include "globals.h"
#include "write.h"

//a special version of write that does not require data, just will simply write out zero to that stated location
WriteStatusCode write_zero(size_t start, size_t length, char* buffer, size_t buffer_size) {
    //this means that the portion we have been asked to write is outside of the bounds of the buffer
    if((start + length) > buffer_size) {
        return BadWrite_length_greater_then_size;
    }

    for(size_t i = start; i < (start + length);i++){
        buffer[start + i] = 0;
    }

    return NormalWrite;
}
 
WriteStatusCode write(size_t start, size_t length, char* data, char* buffer, size_t buffer_size) {
    
    //this means that the portion we have been asked to write is outside of the bounds of the buffer
    if((start + length) > buffer_size) {
        return BadWrite_length_greater_then_size;
    }

    //write the data from data into buffer, from start, to (start + length)
    for(size_t i = 0;i < length;i++) {
        buffer[start + i] = data[i];
    }

    return NormalWrite;
}
