#ifndef DIR_H_
#define DIR_H_
#include <stdlib.h>
#include "globals.h"


typedef struct Directory_t {
    //0x00
    //Filename
    //8 bytes
    char filename[FILENAME_SIZE];//as this is supposed to be a filename, chars seem the best way to represent this

    //0x08
    //Filename extension
    //3 bytes
    char filename_extension[FILENAME_EXTENSION_SIZE];

    //0x0b
    //File attributes
    //1 byte
    char file_attributes;

    //0x0c
    //Reserved
    //10 bytes
    char reserved[RESERVED_BLOCK_SIZE];

    //0x16
    //Time created or last updated
    //2 bytes
    short time_created_or_last_updated;

    //0x18
    //Date created or last updated
    //2 bytes
    short date_created_or_last_updated;

    //0x1a
    //Starting cluster number for file
    //2 bytes
    short starting_cluster_number_for_file;

    //0x1c
    //File size in bytes
    //4 bytes
    int file_size_in_bytes;

} DirectoryEntry;

typedef struct Directory_t_t {
    DirectoryEntry directories[16];
    struct Directory_t_t* sub_directory_pointer[16];
} Directory;


DirectoryEntry* find_directory_entry_for_file(Directory* d, char* file_path);
DirectoryEntry* create_root_directory();
#endif