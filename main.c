#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "globals.h"
#include "directory.h"
#include "write.h"
#include "read.h"
#define FAKE_DRIVE_SIZE_IN_BYTES 100000000
#define TRUE 1
#define FALSE 0
//ZERO out the drive
//write the out the boot sector
//reserve space for all FAT1, FAT2
//write root directory
//fill in the FAT1, and FAT2 entries for root directory
//done


//a helper and testing function to make sure that the system is working
void print_directory(Directory* d, size_t tab_amount) {
    for(int i = 0;i<16;i++){

        if(d->directories[i].filename[0] == 0) {
            continue;
        }

        for(int t = 0;t<tab_amount;t++){
            printf("\t");
        }

        printf("%s\n", d->directories[i].filename);

        if(d->directories[i].filename[0] == '.') {
            if(d->sub_directory_pointer[i] != NULL) {
                print_directory((Directory*)d->sub_directory_pointer[i], tab_amount + 1);
            }
        }
    }
}


void format_drive(char* file, size_t file_size) {
    
    //each filesystem needs to have the boot block
    BootBlock bb = create_boot_block();
    

    //the first part of formatting is writing out our BB to the head of the drive
    int status = write(0, BLOCK_SIZE, (char*)&bb, file, file_size);

    //now we need to zero out out first and second fat table
    status = write_zero(BLOCK_SIZE, FAT_TABLES_COMBINED_SIZE_IN_BYTES, file, file_size);

    write_zero(BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES, BLOCK_SIZE * 500, file, file_size);

    //need to also write into the start of both the flag values of the FAT table
    //so now we have our boot block, and two fat tables

    //format the root directory

    DirectoryEntry* d = create_root_directory();//this is a directory of ten size

    status = write(BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES, BLOCK_SIZE, (char*)d, file, file_size);
    //we set the fat table to have the user directory entries
    for(int i = 0;i<16;i++)
    {
        file[BLOCK_SIZE + i] = (char)0xffff;
    }

    DirectoryEntry* fake = malloc(sizeof(DirectoryEntry));
    char fake_name[] = "fake";
    strcpy(fake->filename, fake_name);
    strcpy(fake->filename_extension, "tx");
    fake->file_attributes = 0x0;
    for(int k = 0;k<RESERVED_BLOCK_SIZE;k++){
       fake->reserved[k] = 0;//zero it out
    }
    fake->file_size_in_bytes = 512;
    fake->starting_cluster_number_for_file = 16;

    //write into block 1, the .a_user directory, a fake directory entry
    size_t start_of_block_1 = BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES + BLOCK_SIZE;
    write(start_of_block_1, sizeof(DirectoryEntry), fake, file, file_size);

    size_t start_of_block_16 = BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES + (BLOCK_SIZE * 16);
    char* fake_data = malloc(BLOCK_SIZE);

    for(int i = 0;i<512;i++)
    {
        fake_data[i] = 64 + (i % 26);
    }

    write(start_of_block_16, BLOCK_SIZE, fake_data, file, file_size);

    free(fake);
    free(fake_data);
}

//given the start of a directory(cluster number) this function will return a tree sturcture of Directorys
//starting with the one at cluster_number and containg all subdirectories that it contains
// and subdirectories of those subdirectories
Directory* get_subdirectory(char* drive, size_t number_of_bytes, int cluster_number) {
    
    Directory* d = malloc(sizeof(Directory));
    
    char* cleaning = d;

    for(int i = 0;i<512;i++)
    {
        *cleaning = 0;
        cleaning++;
    }
    
    read_block(cluster_number, (char*)d->directories, 0, number_of_bytes, drive);

    for(int i = 0;i<number_of_bytes / 32;i++){
      //  printf("Index: %d", i);
        if(d->directories[i].filename[0] == '.') {//check to make sure that it is a directory, there names always start with a .
            if(d->directories[i].file_size_in_bytes > 0) {
                d->sub_directory_pointer[i] = (void*)get_subdirectory(drive, d->directories[i].file_size_in_bytes, d->directories[i].starting_cluster_number_for_file);
            }
            else { 
                d->sub_directory_pointer[i] = NULL;
            }
        }
        else {

            d->sub_directory_pointer[i] = NULL;
        }
    }

    return d;
}


//this assumes a file system is already on the drive
Directory* BootFilesystem(char* drive) {
    
    //we will start this by copying the root direectory into this structure
    Directory* directory = malloc(sizeof(Directory));
    
    //
    read(BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES, BLOCK_SIZE, drive, (char*)directory->directories);
    for(int i = 0;i<16;i++)
    {
        directory->sub_directory_pointer[i] = NULL;
    }


    for(int i = 0;i<16;i++){
        if(directory->directories[i].file_size_in_bytes > 0) {
            printf("%s\n", directory->directories[i].filename);
            printf("%d\n", directory->directories[i].file_size_in_bytes);
            printf("%hd\n",directory->directories[i].starting_cluster_number_for_file);
            directory->sub_directory_pointer[i] = (void*)get_subdirectory(drive, directory->directories[i].file_size_in_bytes, directory->directories[i].starting_cluster_number_for_file);
        }
    }
    return directory;
}


//sees if there is a valid path along file_path
//returning either the starting cluster for the path
//or -1 for a path that leads no were
int validate_path(Directory* d, char* file_path) {

    size_t path_count = 1;//the user directory is always included in a path
    
    //count and copy file_path so this is a nondestructive function
    char* string_length = strlen(file_path);
    char* path_walker = malloc(string_length);
    strcpy(path_walker, file_path);
    
    //snip out the / characters, replacing them with \0 so that we can treat each part of the path as its own cstring
    //so we can use the string as tap that we are moving along, and feeding into our strcmp function
    //no need to malloc new strings
    while(*path_walker != '\0') {
        if(*path_walker == '/') {
            path_count++;
           *path_walker = '\0';
        }
        if(*path_walker == '.') {
            *path_walker = '\0';//this is the file extention 
        }
        path_walker++;
    }

    Directory* active_directory = d;

    size_t index_count = 0;

    //each directory has at most 16 entries
    size_t matches = 0;
    int return_index = -1;
    while(index_count > 16) {
        //entries are packed "tightly" starting from the top
        //we can only have a empty file name if we are looked at all non entries in a directory
        //we can only get to a directory if we have it as part of our path
        //so finding a emptry entry means we are not at the end of our path, 
        //but out of places to look, so bad path, return -1;
        if(active_directory->directories[index_count].filename[0] == 0){
            break;
        }

        //the only thing we care about at first is, is this alone our path right now
        int result = strcmp(path_walker, active_directory->directories[index_count].filename);
        if(result) {
            matches++;
            if(matches == path_count) {//if these two equal each other then we are out of file, return where its contents are on the disk
                return_index = active_directory->directories[index_count].starting_cluster_number_for_file;
                break;
            }
            else if(active_directory->directories[index_count].filename[0] == "." ){
                //if we matched, and our it is a path, move into that directory and start our enumeration
                active_directory = active_directory->sub_directory_pointer;
                index_count = 0;
            }

            //move it until it is the '\0' character which will preced the next part of the path
            while(*path_walker != '\0') {
                path_walker++;
            }
            path_walker++;
        }

        //move to the next entry in the directory   
        index_count++;
    }

    free(path_walker);
    return -1;
}


//this will go through the drive, starting at starting_cluster and setting the FAT entries to empty
//this does not remove the information at the cluster level, nor does it cleanup the directory entry for that file
//this should only be used in conjunction with other functions
void unallocate_file_chain(char* drive, int starting_cluster) {
    char* current_cluster = &drive[BLOCK_SIZE + starting_cluster];//start this at the know allocated block
    char* base_pointer = &drive[BLOCK_SIZE];//we keep a base pointer, so we can caluate current_cluster as base_pointer + *current_cluster

    int done = 0;
    while(done == 0) {
        //0xff is flag value for end of file
        if(*current_cluster == 0xff) {
            done = 1;
        }

        //copy the value of the next locaiton we need to move to
        int next = *current_cluster;
        *current_cluster = 0;//set the cluster as open for allocation
        current_cluster = base_pointer + next;//move the pointer along to the next location in memory
    }
}

//this function will attempt to go through the FAT, and find number_of_clusters_needed open FAT entries
//it returns either the cluster number of the first block allocated
//or -1, to indicate that it was unable to allocate enough space for the drive
int allocate_storage_on_disk(char* drive, int number_of_clusters_needed) {
    int allocated_clusters = 0;
    int first_cluster = -1;
    char* previous_entry = NULL;
    char* fat_entry = &drive[BLOCK_SIZE];//this starts us at the first clusters, we prop forward from there
    size_t cluster_number = 0;

    //keep allocating until we have enough blocks allocated
    while(allocated_clusters != number_of_clusters_needed) {
        
        //if we run out of FILE ALLOCATION TABLE entries, we cannot fully allocate the file, so return -1
        //we will treat -1 as an error
        if(cluster_number >= (FAT_TABLES_COMBINED_SIZE_IN_BYTES / 2)) {

            //if we allocated at least one cluster, we need to undo that
            if(first_cluster != -1) {
                unallocate_file_chain(drive, first_cluster);
            }
            first_cluster = -1;
            break;
        }

        //*fat_entry == 0 means unallocated FAT entry, lets take it
        if(*fat_entry == 0) {

            //we need to know what was the last place we allocated a cluster
            //so when we find the next one we can set it to the current cluster number we are allocating
            //this creates the cluster chain that makes up a file
            if(previous_entry == NULL) {
                previous_entry = fat_entry;
                first_cluster = cluster_number;
            }
            else {
                //set the last cluster to this one, creating the file chain
                *previous_entry = cluster_number;
                //and move the pointer up for the next block
                previous_entry = fat_entry;
            }

            allocated_clusters++;
        }
        cluster_number++;
        fat_entry++;
    }

    //the last thing we need to do is set whatever our last cluster allocated, as the last cluster we need
    *previous_entry = 0xff;

    return first_cluster;
}

void write_file(char* path, char* filename, char* file, int file_size_in_bytes, char* drive, Directory* drive_directory) {
    DirectoryEntry* directory_entry = find_directory_entry_for_file(drive_directory, path);
    
    if(directory_entry == NULL) {
        printf("%s is not a valid path, file not saved\n", path);    
    }
    int entry_cluster = directory_entry->starting_cluster_number_for_file;
    
    //prob to the first open entry and write in this files info
    char* this_is_what_want_to_do = &drive[BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES + (BLOCK_SIZE * entry_cluster)];

    //now we must get the number of blocks we need to reserve, and write a function to allocate them.

    int total_number_of_clusters = (file_size_in_bytes / 512) + 1;// + 1 because we always need at least one, we are looking for how many more we need with the divide
    int starting_cluster = allocate_storage_on_disk(drive, total_number_of_clusters);

    if(starting_cluster == -1){
        printf("Unable to write %s to disk, not enough space\n", filename);
        return;
    }

    char* file_ptr = file;

    int current_cluster = starting_cluster;
    //while we still have bytes to read, write bytes
    while(file_size_in_bytes > 0) {
        char* start_of_block = drive[BLOCK_SIZE + FAT_TABLES_COMBINED_SIZE_IN_BYTES + (current_cluster * BLOCK_SIZE)];
        file_size_in_bytes = file_size_in_bytes - 512;
        int amount_of_bytes_to_write = 0;

        if(file_size_in_bytes < 0) {
        //    amount_of_bytes_to_writ
            amount_of_bytes_to_write = 512 - file_size_in_bytes;
        }
        else {
            //write the most we can into the block
            amount_of_bytes_to_write = 512;
        }
        int result = write(start_of_block, amount_of_bytes_to_write, file_ptr, drive, FAKE_DRIVE_SIZE_IN_BYTES);
        
        //we write in blocks of 512, so all we have to do is keep track of where in the file we are
        file_ptr = file_ptr + amount_of_bytes_to_write;

        if(result != 0) {
            printf("Error with writing %s to disk, please try again\n", filename);
            unallocate_file_chain(drive, starting_cluster);
            break;
        }
        //we store the next part of the file at the current cluster location, so we need to look up the next cluster we are going to write to
        current_cluster = drive[BLOCK_SIZE + current_cluster];
    }

}

typedef struct  File_t {
    int starting_cluster;
    int length_in_bytes;
    int current_location;
} FILE_HANDLE;


int open_file(FILE_HANDLE* file_handle, char* path, char* drive, Directory* d) {
    DirectoryEntry* directory_entry = find_directory_entry_for_file(d, path);
    if(directory_entry == NULL) {
        printf("Unable to find file %s\n", path);
        return -1;
    }

    //"opening" a file just tells the user if it exsist and then sets these up so from here on out 
    file_handle->starting_cluster = directory_entry->starting_cluster_number_for_file;
    file_handle->current_location = 0;//this is the byte in the file we are at
    file_handle->length_in_bytes = directory_entry->file_size_in_bytes;

    return 0;
}

int create_file(char* path, char* name, char* ex, Directory* d, char* drive) {
    DirectoryEntry* directory_to_write_to = find_directory_entry_for_file(d, path);

    if(directory_to_write_to == NULL) {
        printf("%s does not exsist, %s not created\n", path, name);
        return -1;
    }
    
    int length_of_name = strlen(name);
    int length_of_exten = strlen(ex);
    //simple limiation, files must be at least 7 characters long

    if(length_of_name > 7 || length_of_exten > 2) {
        return -1;
    }

    for(int i = 0;i<16;i++){
        if(directory_to_write_to->filename[0] == 0){//we are looking for an open directory entry to write the blank info into
            //copy the name over to the fileentry
            for(int j = 0;j<8;j++) {
                directory_to_write_to->filename[j] = name[j];
            }
            
                
        }
    }

    return 0;
}

int update_file() {

}

int delete_file() {

}

void flush_to_device() {

}

//the whole point is that this is just going to be larger then we are ever going to actually need
//this is around 50 MBS 

int main() {
    
    char* fake_drive = malloc(sizeof(char) * FAKE_DRIVE_SIZE_IN_BYTES);
    
    format_drive(fake_drive, FAKE_DRIVE_SIZE_IN_BYTES);

    Directory* directory = BootFilesystem(fake_drive);

  //  char* fake_block = malloc(BLOCK_SIZE);     
    print_directory(directory, 0);
    FILE_HANDLE file;
    int result = open_file(&file, ".a_user/fake.tx", fake_drive, directory);
    printf("WHAT %d\n", result);

    /*
    read_file(directory->sub_directory_pointer[0]->directories[0].starting_cluster_number_for_file, BLOCK_SIZE, fake_drive, fake_block);

    for(int i = 0;i<512;i++)
    {
        printf("%c", fake_block[i]);
    }

    printf("\n");
*/
    return 0; 
}