#pragma once
#include "../osrms_File/Osrms_File.h"

typedef struct osrmsFile {
    uint8_t validationByte;
    uint8_t fileName[14];
    uint32_t fileSize;
    uint32_t virtualAddress;
} osrmsFile;

typedef struct PCB {
    uint8_t state;
    uint8_t pid;
    uint8_t name[11];
    uint8_t fileTable[115];
    uint8_t firstOrderTable[128];
} PCB;

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
