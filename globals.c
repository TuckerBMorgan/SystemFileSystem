#include "globals.h"
char volume_labl_string[VOLUME_LABEL_SIZE] = "helloworld";

BootBlock create_boot_block() {
    BootBlock bb;

    //we dont need the boot strap program
    bb.boot_strap_program[0] = 0;
    bb.boot_strap_program[1] = 0;
    bb.boot_strap_program[2] = 0;
    bb.manufactuer = 1;

    //these two togather mean that one allocation unit(cluster) is only 512 bytes in size
    bb.bytes_per_block = 512;
    bb.blocks_per_allocation_unit = 1;

    bb.reserved_blocks = 1;
    //we will have no redudant table
    bb.number_of_file_allocation_tables = 2;
    bb.number_root_directory_entries = ROOT_DIRECTORY_SIZE;//not all ten might be used, but we might as well start with this many, easier to exapnd old drives
    bb.number_of_blocks = 0x20;//we will be having 100000 data blocks for files, 
    bb.media = 1;

    //one record in a FAT is 16 bits(2 bytes) long
    //so (file_allocation_table_copy_block_size * block_size) is size of FAT is bytes
    //that / by 2, gives us the number files we can store, minus 2 for the first FAT entries which are reserverd 
    bb.file_allocation_table_copy_block_size = 100;
    
    //I think that I can ignore most of these
    bb.number_blocks_per_track = 10;//? this is a total guess
    bb.number_of_heads = 5;//?
    bb.number_hidden_blocks = 0;//also not used so far as I know

    bb.number_of_blocks_extended = (100000/*data/file section*/) + (781/*FAT1 + FAT2*/) + (1 /*root directory*/) + (1/*boot block*/);//for now we are going with max total blocks, and seeing if that is too much , or to little

    //all the fields after this, I am not sure if they the most nessacary, but they might be, lets zero them out, so that way we have a consistant value for them
    bb.physical_drive_number = 0;
    bb.extended_boot_record_signature = 0;
    bb.volume_serial_number = 0;

    for(int i = 0;i<VOLUME_LABEL_SIZE;i++) {
        bb.volume_label[i] = volume_labl_string[i];
    }

    bb.file_system_indentifier = 0;//this might be some name for a fat16 eventually

    for(int i = 0;i<BOOTSTRAP_PROGRAM_LARGE;i++) {
        bb.boot_program[i] = 0;//we dont need this, so just zero it out
    }

    bb.boot_signature = 0;


    return bb;
}