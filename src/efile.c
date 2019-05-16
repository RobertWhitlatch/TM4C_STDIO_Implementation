// eFile.c

#include "Master.h"

#define SUCCESS 0
#define FAIL 1

#define DIRECTORY 0
#define BLOCK 512
#define FREELIST 0
#define MAX_FILES 32
#define MAX_NUM_BLOCKS 65535
#define LINK_hw 255
#define MAX_FILE_SIZE BLOCK*254

#define CLOSED 0
#define OPEN 1
#define READ 2
#define WRITE 3
#define FILE_CLOSED 0xFF

#define TOTAL_SIZE_w 0
#define TOTAL_BLOCKS_hw 2
#define TABLE_LINK_hw 3
#define FIRST_BLOCK_hw 4

typedef struct Entry {
    char Name[8];
    uint32_t size;
    uint16_t end_block;
    uint16_t table_block;
} Entry;

typedef union Block {
    uint8_t byte[512];
    uint16_t halfword[256];
    uint32_t word[128];
} Block_t;

typedef union Directory {
    Block_t buff;
    // entry[0] reserved for freespace
    Entry entry[32];
} Directory;

Directory FS_SDC;
Block_t work;
Block_t file_table;
Block_t file_data;
Block_t write_file_table;
Block_t write_file_data;
Block_t read_file_table;
Block_t read_file_data;

int write_file_open = FILE_CLOSED;
int read_file_open = FILE_CLOSED;
int read_data_index = 0;
int activated = CLOSED;
char stdio_target[8];

void zeroizeBlockLocal(Block_t* buff){
    for(int i = 0; i < 128; i++){
        buff->word[i] = 0;
    }
}

void zeroizeTwoBlocksLocal(Block_t* buff_1, Block_t* buff_2){
    for(int i = 0; i < 128; i++){
        buff_1->word[i] = 0;
        buff_2->word[i] = 0;
    }
}

void zeroizeEntryLocal(Entry* entry){
    entry->table_block = 0;
    entry->end_block = 0;
    entry->size = 0;
    for(int i = 0; i < 8; i++){
        entry->Name[i] = 0;
    }
}

uint16_t popFreeBlock(Block_t* buff){
    if(FS_SDC.entry[FREELIST].size == 0){
        return(0);
    }
    int block = FS_SDC.entry[FREELIST].table_block;
    eDisk_ReadBlock(buff->byte, block);
    FS_SDC.entry[FREELIST].table_block = buff->halfword[LINK_hw];
    buff->halfword[LINK_hw] = 0;
    FS_SDC.entry[FREELIST].size--;
    return(block);
}

void pushFreeBlock(Block_t* buff, uint16_t blockNumber){
    buff->halfword[LINK_hw] = FS_SDC.entry[FREELIST].table_block;
    FS_SDC.entry[FREELIST].table_block = blockNumber;
    eDisk_WriteBlock(buff->byte, blockNumber);
    FS_SDC.entry[FREELIST].size++;
}

//---------- eFile_Init-----------------
// Activate the file system, without formating
// Input: none
// Output: 0 if successful and 1 on failure (already initialized)
int eFile_Init(void) {

    if(activated == OPEN){
        return FAIL;
    }

    int result;
    result = eDisk_Init(0);
    result |= eDisk_ReadBlock(FS_SDC.buff.byte,DIRECTORY);
    activated = OPEN;

    return result;
}

//---------- eFile_Close-----------------
// Deactivate the file system
// Input: none
// Output: 0 if successful and 1 on failure (not currently open)
int eFile_Close(void) {

    if(activated == CLOSED){
        return FAIL;
    }

    eFile_RClose();
    eFile_WClose();

    eDisk_WriteBlock(FS_SDC.buff.byte, DIRECTORY);

    zeroizeBlockLocal(&FS_SDC.buff);

    activated = CLOSED;

    return SUCCESS;
}

int eFile_FreeSpace_Init(void) {

    FS_SDC.entry[FREELIST].Name[0] = 'F';
    FS_SDC.entry[FREELIST].Name[1] = 'r';
    FS_SDC.entry[FREELIST].Name[2] = 'e';
    FS_SDC.entry[FREELIST].Name[3] = 'e';
    FS_SDC.entry[FREELIST].size = 0;
    FS_SDC.entry[FREELIST].end_block = 0xFFFF;
    FS_SDC.entry[FREELIST].table_block = 0;

    zeroizeBlockLocal(&work);
    fprintf(uart, "Beginning free space formatting.\n");
    for(uint32_t i = 0; i < MAX_NUM_BLOCKS; i++){
        if((i & 0xFF) == 0 && i != 0){
            fprintf(uart,".");
            if((i & 0xFFF) == 0){
                uint16_t progress = (i/0x1000) * 625;
                fprintf(uart, " %2u.%02u%% Complete\n", progress/100, progress%100);
            }
        }

        pushFreeBlock(&work, MAX_NUM_BLOCKS-i);
    }
    fprintf(uart, ". Complete!\n");

    return SUCCESS;
}

//---------- eFile_Format-----------------
// Erase all files, create blank directory, initialize free space manager
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Format(void) {
    zeroizeBlockLocal(&FS_SDC.buff);
    eFile_FreeSpace_Init();
    eDisk_WriteBlock(FS_SDC.buff.byte,DIRECTORY);
    return SUCCESS;
}

//---------- eFile_Create-----------------
// Create a new, empty file with one allocated block
// Input: file name is an ASCII string up to seven characters 
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Create(char name[]) {

    if(activated == CLOSED){
        return FAIL;
    }

    if(eFile_getFilePtr(name) != null){
        return FAIL;
    }

    int FileNum;
    for(FileNum = 1; FileNum < MAX_FILES; FileNum++){
        if(FS_SDC.entry[FileNum].table_block == 0){
            break;
        }
    }

    if(FileNum == MAX_FILES){
        return FAIL; // TODO: Make this grow, then file system itself has no bounds
    }

    for(int j = 0; (name[j] != 0) || (j < 7); j++){
        FS_SDC.entry[FileNum].Name[j] = name[j];
    }
    FS_SDC.entry[FileNum].Name[7] = 0;

    FS_SDC.entry[FileNum].size = 0;
    FS_SDC.entry[FileNum].table_block = popFreeBlock(&file_table);
    FS_SDC.entry[FileNum].end_block = popFreeBlock(&file_data);

    file_table.word[TOTAL_SIZE_w] = 0;
    file_table.halfword[TOTAL_BLOCKS_hw] = 1;
    file_table.halfword[TABLE_LINK_hw] = 0;
    file_table.halfword[4] = FS_SDC.entry[FileNum].end_block;

    eDisk_WriteBlock(file_table.byte, FS_SDC.entry[FileNum].table_block);
    eDisk_WriteBlock(file_data.byte, FS_SDC.entry[FileNum].end_block);
    eDisk_WriteBlock(FS_SDC.buff.byte, DIRECTORY);

    zeroizeTwoBlocksLocal(&file_table, &file_data);

    return SUCCESS;
}

//---------- eFile_Delete-----------------
// delete this file
// Input: file name is a single ASCII letter
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Delete(char name[]) {

    if(activated == CLOSED){
        return FAIL;
    }

    int FileNum = (int)eFile_getFilePtr(name);

    if(FileNum == read_file_open || FileNum == write_file_open  || FileNum == FILE_CLOSED){
        return FAIL;
    }

    eDisk_ReadBlock(file_table.byte, FS_SDC.entry[FileNum].table_block);
    for(int i = 0; i < file_table.halfword[TOTAL_BLOCKS_hw]; i++){
        pushFreeBlock(&file_data, file_table.halfword[4+i]);
    }

    zeroizeBlockLocal(&file_table);
    pushFreeBlock(&file_table, FS_SDC.entry[FileNum].table_block);
    zeroizeEntryLocal(&FS_SDC.entry[FileNum]);
    eDisk_WriteBlock(FS_SDC.buff.byte, DIRECTORY);

    return SUCCESS;
}

//---------- eFile_WOpen-----------------
// Open the file, read into RAM last block
// Input: file name (1-7 characters)
// Output: File Handle, null if error
FILE* eFile_WOpen(char name[]) {

    if(activated == CLOSED){
        return null;
    }

    if(write_file_open != (int)null){
        return null;
    }

    int FileNum = (int)eFile_getFilePtr(name);

    if(FileNum == (int)null){
        return null;
    }

    eDisk_ReadBlock(write_file_table.byte,FS_SDC.entry[FileNum].table_block);
    eDisk_ReadBlock(write_file_data.byte,FS_SDC.entry[FileNum].end_block);
    write_file_open = FileNum;

    return (FILE*)FileNum;
}

void file_expand(void){
    eDisk_WriteBlock(write_file_data.byte, FS_SDC.entry[write_file_open].end_block);
    int table_index = 4 + write_file_table.halfword[TOTAL_BLOCKS_hw];
    if(table_index > 255){
        return;
    }
    write_file_table.halfword[TOTAL_BLOCKS_hw]++;
    FS_SDC.entry[write_file_open].end_block = popFreeBlock(&write_file_data);
    write_file_table.halfword[table_index] = FS_SDC.entry[write_file_open].end_block;
}

//---------- eFile_Write-----------------
// save at end of the open file
// Input: data to be saved
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_Write(char data) {

    if(activated == CLOSED){
        return FAIL;
    }

    if(write_file_open == FILE_CLOSED){
        return FAIL;
    }

    if(write_file_table.word[TOTAL_SIZE_w] == MAX_FILE_SIZE){
        return FAIL;
    }

    int data_index = write_file_table.word[TOTAL_SIZE_w] % BLOCK;
    write_file_data.byte[data_index] = data;
    write_file_table.word[TOTAL_SIZE_w]++;
    FS_SDC.entry[write_file_open].size++;

    if(write_file_table.word[TOTAL_SIZE_w] == MAX_FILE_SIZE){
        return FAIL;
    }

    if(write_file_table.word[TOTAL_SIZE_w] % BLOCK == 0){
        file_expand();
    }

    return SUCCESS;
}


//---------- eFile_WClose-----------------
// close the file, left disk in a state power can be removed
// Input: none
// Output: 0 if successful and 1 on failure (e.g., trouble writing to flash)
int eFile_WClose(void) {

    if(activated == CLOSED){
        return FAIL;
    }

    if(write_file_open == FILE_CLOSED){
        return FAIL;
    }

    eDisk_WriteBlock(write_file_data.byte, FS_SDC.entry[write_file_open].end_block);
    eDisk_WriteBlock(write_file_table.byte, FS_SDC.entry[write_file_open].table_block);
    eDisk_WriteBlock(FS_SDC.buff.byte, DIRECTORY);

    zeroizeTwoBlocksLocal(&write_file_table, &write_file_data);

    write_file_open = FILE_CLOSED;

    return SUCCESS;
}

//---------- eFile_ROpen-----------------
// Open the file, read first block into RAM 
// Input: File name (1-7 characters)
// Output: File Handle, null if error
FILE* eFile_ROpen(char name[]) {

    if(activated == CLOSED){
        return null;
    }

    if(read_file_open != (int)null){
        return null;
    }

    int FileNum = (int)eFile_getFilePtr(name);

    if(FileNum == (int)null){
        return null;
    }

    eDisk_ReadBlock(read_file_table.byte,FS_SDC.entry[FileNum].table_block);
    read_file_open = FileNum;
    read_data_index = 0;
    
    return (FILE*)FileNum;
}

//---------- eFile_ReadNext-----------------
// retreive data from open file
// Input: none
// Output: return by reference data
//         0 if successful and 1 on failure (e.g., end of file)
int eFile_ReadNext(char *pt) {

    if(activated == CLOSED){
        return FAIL;
    }

    if(read_file_open == FILE_CLOSED){
        return FAIL;
    }

    if(read_data_index == read_file_table.halfword[TOTAL_SIZE_w]){
        return FAIL;
    }

    if(read_data_index % BLOCK == 0){
        eDisk_ReadBlock(read_file_data.byte, read_file_table.halfword[ 4 + (read_data_index/BLOCK) ]);
    }

    *pt = read_file_data.byte[read_data_index % BLOCK];
    read_data_index++;

    return SUCCESS;
}


//---------- eFile_RClose-----------------
// close the reading file
// Input: none
// Output: 0 if successful and 1 on failure (e.g., wasn't open)
int eFile_RClose(void) {

    if(activated == CLOSED){
        return FAIL;
    }

    if(read_file_open == FILE_CLOSED){
        return FAIL;
    }

    zeroizeTwoBlocksLocal(&read_file_table, &read_file_data);

    read_file_open = FILE_CLOSED;
    read_data_index = 0;

    return SUCCESS;
}


//---------- eFile_Directory-----------------
// Display the directory with filenames and sizes
// Input: Output Stream to print to
// Output: none
//         0 if successful and 1 on failure (e.g., trouble reading from flash)
int eFile_Directory(FILE *target) {

    if(activated == CLOSED){
        return FAIL;
    }

    int content = 1;

    for(int i = 1; i < MAX_FILES; i++){
        if(FS_SDC.entry[i].table_block != 0){
            if(content == 1){
                fprintf(target, "Displaying Contents of microSD card...\n");
                content = 0;
            }
            fprintf(target, "File: %02u  Name: %7s  Size: %06u\n",i,FS_SDC.entry[i].Name,FS_SDC.entry[i].size);
        }
    }

    if(content){
        fprintf(target, "No files present.\n");
    }

    return SUCCESS;
}

//---------- eFile_DisplayFile-----------------
// Display the contents of given File
// Input: Output Stream to print to, File to display
// Output: none
//         0 if successful and 1 on failure (e.g., trouble reading from flash)
int eFile_DisplayFile(FILE *target, char file_name[]) {

    if(activated == CLOSED){
        return FAIL;
    }

    int FileNum = (int)eFile_getFilePtr(file_name);

    if(FileNum == FILE_CLOSED){
        return FAIL;
    }

    eDisk_ReadBlock(file_table.byte, FS_SDC.entry[FileNum].table_block);
    fprintf(target,"Displaying contents of File %s.\n",FS_SDC.entry[FileNum].Name);

    for(int i = 0; i < file_table.word[TOTAL_SIZE_w]; i++){
        if(i % BLOCK == 0){
            eDisk_ReadBlock(file_data.byte, file_table.halfword[4 + i/BLOCK]);
        }
        fputc(file_data.byte[i%BLOCK], target);
    }

    zeroizeTwoBlocksLocal(&file_table, &file_data);

    return SUCCESS;
}

FILE* eFile_getFilePtr(char* name){

    if(activated == CLOSED){
        return null;
    }

    if(name[0] == 0){
        return null;
    }

    int FileNum;
    for(FileNum = 1; FileNum < MAX_FILES; FileNum++){
        if(strcmp(name,FS_SDC.entry[FileNum].Name) == 0){
            break;
        }
    }

    if(FileNum == MAX_FILES){
        return null;
    }

    return (FILE*)FileNum;
}



int eFile_getName(FILE* file, char* buff){

    if(activated == CLOSED){
        return FAIL;
    }

    if(FS_SDC.entry[(int)file].table_block == 0){
        return FAIL;
    }

    strcpy(buff,FS_SDC.entry[(int)file].Name);

    return(SUCCESS);
}

FILE* eFile_getReadFile(void){
    return ((FILE*) read_file_open);
}

FILE* eFile_getWriteFile(void){
    return ((FILE*) write_file_open);
}

int eFile_isReadOpen(FILE* f){
    return ( ((int)f == read_file_open) ? SUCCESS : FAIL );
}

int eFile_isWriteOpen(FILE* f){
    return ( ((int)f == write_file_open) ? SUCCESS : FAIL );
}

unsigned long eFile_getSize_File(FILE* f){
    return(FS_SDC.entry[(int)f].size);
}

unsigned long eFile_getSize_Name(char* name){
    return(eFile_getSize_File(eFile_getFilePtr(name)));
}
