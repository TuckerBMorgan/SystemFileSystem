#include "globals.h"
#include "read.h"


void read(size_t start, size_t length, char* drive, char* buffer) {
    
    for(size_t i = 0;i < length;i++){
            buffer[i] = drive[start + i];
    }

}
 
//these two functions should work in tandem to find all the contents of a file that the system ha asked for
int read_block_value(int block_index, char* drive) {
    return drive[BLOCK_SIZE + (block_index + block_index)];
}

ReadStatusCode read_block(int block_index, char* buffer, size_t place_in_buffer, size_t number_of_bytes_to_read, char* drive) {
    if(number_of_bytes_to_read > BLOCK_SIZE) {
        return BadRead_number_of_bytes_exceeds_block_size;
    }
    //boot block + fattables + root directory leaves us at block #1, block_index *BLOCK_SIZE gets us to the start of the block we want to read from
    size_t starting_place_in_drive = BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES + (block_index * BLOCK_SIZE);

    for(int i = 0;i<number_of_bytes_to_read;i++) {
        buffer[place_in_buffer + i] = drive[starting_place_in_drive + i];
    }

    return NormalRead;
}


//buffer.length == number_of_bytes_to_read
void read_file(int start_cluster, size_t number_of_bytes_to_read, char* drive, char* buffer) {
    
    //temp value
    size_t current_cluster = start_cluster;
    
    //we use this buffer as a fixed sized temporay storage for any single block that we are reading from
    char* block_buffer = malloc(sizeof(BLOCK_SIZE));
    size_t number_of_writes = 0;
    
    
    //we need to write into buffer number_of_bytes_to_read times
    while(number_of_writes < number_of_bytes_to_read) {
        
        //this will go off during the first itearation, and then after every BLOCK_SIZE number of writes that have occured
        //this will mount the next cluster that is holding part of the file we are reading from
        if(number_of_writes % BLOCK_SIZE == 0) {
            //read the block into our buffer
            read_block(current_cluster, block_buffer, 0, 512, drive);
            //look up where we will have to read next if we get to that
            current_cluster = read_block_value(current_cluster, drive);
        }

        //finally write the single byte for this iteration into the buffer
        buffer[number_of_writes] = block_buffer[number_of_writes % 512];
        number_of_writes++;
    }
    free(block_buffer);
}