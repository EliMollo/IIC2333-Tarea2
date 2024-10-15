#pragma once
#include "../osrms_File/Osrms_File.h"

#define PCB_TABLE_START 0
#define PCB_ENTRY_SIZE 256
#define PCB_ENTRIES 32

#define PAGE_TABLE_BITMAP 8192

#define SECONDARY_PAGE_TABLE_START 8320

#define FRAME_BITMAP_START 139392
#define FRAME_TABLE_START 147584

#define VPNMASK 0xFFF0000
#define OFFSETMASK 0x7FF8000
#define FRAME_SIZE 32768

#pragma pack(push, 1)
typedef struct FileEntry {
    unsigned char validationByte;
    char fileName[14];
    unsigned int fileSize;
    unsigned int virtualAddress;
} FileEntry;

typedef struct PCB {
    unsigned char state;
    unsigned char pid;
    char name[11];
    char fileTable[115];
    char firstOrderTable[128];
} PCB;
#pragma pack(pop)

// Funciones generales
void os_mount(char* memory_path);
void os_ls_processes();
int os_exists(int process_id, char* file_name);
void os_ls_files(int process_id);
void os_frame_bitmap();
void os_tp_bitmap();

// Funciones para procesos
void os_start_process(int process_id, char* process_name);
void os_finish_process(int process_id);

// Funciones para archivos
osrmsFile* os_open(int process_id, char* file_name, char mode);
int os_read_file(osrmsFile* file_desc, char* dest);
int os_write_file(osrmsFile* file_desc, char* src);
void os_close(osrmsFile* file_desc);
