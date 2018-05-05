#include <string.h>

#include "globals.h"
#include "directory.h"
#include "time.h"

 
//this will get the current time, and format it for use with files
//there is a very specific format for this, best to just have a function that handles it
//this will result in a long that has the binary format
// yyyyyyy mmmm ddddd hhhhh mmmmmm xxxxx
long format_current_time_for_file_entry() {
    long return_value = 0;
    time_t rawtime;
    struct tm * timeinfo;
 
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    
    //we always shift by the amount of places to allow for the next number to be added
    //and not the number of places of the number we just added, we are making space for the next one

    int year = timeinfo->tm_year - 80;//tm_year is years since 1900, we are shifting that to 1980 to match the original fat16 system
    return_value |= year;
    
    return_value <<= 4;//we are shifting it by four, to allow us to or(|) on the four bytes for the month
    
    int month = timeinfo->tm_mon;
    return_value |= month;

    return_value <<= 5;//the day amount

    int day = timeinfo->tm_mday;
    return_value |= day;

    return_value <<= 5;//shifting to allow for the hours
    
    int hour = timeinfo->tm_hour;
    return_value |= hour;

    return_value <<= 6;//shifting to allow for the minutes

    int minutes = timeinfo->tm_min;
    return_value |= minutes;

    return_value <<= 5;//shiting for seconds

    int seconds = timeinfo->tm_sec;
    return_value |= seconds;
    

    return return_value;
}

DirectoryEntry* create_root_directory() {
    DirectoryEntry* d = malloc(BLOCK_SIZE);
    char directory_name[] = ".a_user";//a period as the first value indicates that this is a directory
    for(int i = 0;i<16;i++)
    {
        strcpy(d[i].filename, directory_name);

        directory_name[1]++;//this will create a - j users

        strcpy(d[i].filename_extension, "sd");//sd stands for subdirectory

        d[i].file_attributes = 0x10;//0x10 is used for subdirectories, all entries in the root directory are subdirectories

        for(int k = 0;k<RESERVED_BLOCK_SIZE;k++){
            d[i].reserved[k] = 0;//zero it out
        }

        //put in the time
        long total_time = format_current_time_for_file_entry();

        short lower_time = total_time;//I AM HOPING THIS IS DOING WHAT I AM SAYING IT IS DOING
        d[i].time_created_or_last_updated = lower_time;
        d[i].date_created_or_last_updated = (total_time >> 16);
        d[i].starting_cluster_number_for_file = i + 1;//this is different then the traditional Fat16, which reservers starting blocks 0,1, we can do better
        if(i == 0) {
            d[i].file_size_in_bytes = 32;
        }
        else {
            d[i].file_size_in_bytes = 0;//each user directiory starts with one entry in it 
        }
    } 
    return d;
};

DirectoryEntry* find_directory_entry_for_file(Directory* d, char* file_path) {
    size_t path_count = 1;//the user directory is always included in a path
    
    //count and copy file_path so this is a nondestructive function
    char* string_length = strlen(file_path);
    char* path_walker = malloc(string_length);
    char* ptr_copy = path_walker;
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
            active_directory = NULL;
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
    
    free(ptr_copy);

    if(active_directory == NULL) {
        return NULL;
    }
    
    return &active_directory->directories[index_count];
}
