#ifndef GLOBALS_H_
#define GLOBALS_H_

#define BLOCK_SIZE 512
//general BootBlock magic numbers
#define BOOTSTRAP_PROGRAM_SMALL 3
#define BOOTSTRAP_PROGRAM_LARGE 448
#define NUNBER_OF_FAT_ENTRIES 100000
#define ROOT_DIRECTORY_SIZE 16
#define VOLUME_LABEL_SIZE 11
#define FAT_TABLES_COMBINED_SIZE_IN_BYTES 400000
extern char volume_labl_string[VOLUME_LABEL_SIZE];

//http://www.tavi.co.uk/phobos/fat.html
//using this website as a guide for sitting up the a FAT16 file system
typedef struct BootBlock_t {
    //0x00
    //Part of the bootstrap program
    //3 bytes
    char boot_strap_program[BOOTSTRAP_PROGRAM_SMALL];

    //0x03
    //Optional Manufactuer Description
    //8 bytes
    long long manufactuer;

    //0x0b
    //Number of bytes per block(almost always 512)
    //2 bytes
    short bytes_per_block;

    //0x0d 
    //number of blocks per allocation unit
    //1 byte
    char blocks_per_allocation_unit;

    //0x0e
    //number of reserved blocks(tends to be one)
    //2 bytes
    short reserved_blocks;

    //0x10
    //number of File Allocation tables
    //1 byte
    char number_of_file_allocation_tables;

    //0x11
    //Number of root directory entries
    //2 bytes
    short number_root_directory_entries;

    //0x13
    //Total number of blocks in the entire disk(will be zero, if this exceeds 65535 blocks, and instead will be listed at 0x20)
    //2 bytes
    short number_of_blocks;

    //0x15
    //Media Descriptor
    //1 byte
    char media;

    //0x16
    //The number of blocks occupied by one copy of the File Allocation Table
    //2 bytes
    short file_allocation_table_copy_block_size;

    //0x18 
    //The number of blocks per track
    //2 bytes
    short number_blocks_per_track;

    //0x1a 
    //the number of heads
    //2 bytes
    short number_of_heads;

    //0x1c
    //the number of hidden blocks(said to legacy, and mostly unsued)
    //4 bytes
    long number_hidden_blocks;

    //0x20
    //the number of blocks in the entire disk, will be used if this exceeds the 0x13 size
    //4 bytes 
    long number_of_blocks_extended;

    //0x24
    //Physical Drive Number
    //2 bytes
    short physical_drive_number;

    //0x26
    //Extended Boot Record Signature()
    //1 bytes
    char extended_boot_record_signature;

    //0x27
    //Volume Serial Number(used as a id for a particular disk)
    //4 bytes
    long volume_serial_number;

    //0x2b
    //Volume Label(Human readable id of the disk)
    //11 bytes 
    char volume_label[VOLUME_LABEL_SIZE];

    //0x36
    //File system identifier
    //8 bytes
    long long file_system_indentifier;

    //0x3e 
    //The remainder of the bootstrap program
    //0x1c0(448 decimal) bytes
    char boot_program[BOOTSTRAP_PROGRAM_LARGE];

    //0x1fe
    //Boot block signature(0x55, followed by 0xaa)
    short boot_signature;
} BootBlock;

BootBlock create_boot_block();
//genreal Directory magic numbers
#define FILENAME_SIZE 8
#define FILENAME_EXTENSION_SIZE 3
#define RESERVED_BLOCK_SIZE 10

//filename first character magic numbers
#define FILENAME_NEVER_USED 0x00
#define FILENAME_USED_BUT_DELETED 0xe5
#define FILENAME_FIRST_CHARACTER_ACTUALLY_0xe5 0x05
#define FILENAME_IS_DIRECTORY_FLAG_VALUE 0x2e

//file attribute values
#define FILE_READ_ONLY 0x01
#define FILE_HIDDEN 0x02
#define SYSTEM_FILE 0x04
#define DISK_VOLUME_LABEL 0x08
#define SUBDIRECTORY 0x10
#define ARCHIVE_FLAG 0x20
#define NOTUSED_40 0x40//must be set to zero
#define NOTUSED_80 0x80//must be set to zero
#endif