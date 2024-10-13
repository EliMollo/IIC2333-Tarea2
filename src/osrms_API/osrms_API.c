#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"

#define PCB_TABLE_START 0
#define PAGE_TABLE_BITMAP 8192
#define SECONDARY_PAGE_TABLE_START 8320
#define FRAME_BITMAP_START 139392
#define FRAME_TABLE_START 147584
#define PCB_ENTRY_SIZE 256

FILE *memory_file = NULL;
char* mounted_memory_path = NULL;

// Funciones generales
void os_mount(char* memory_path) {
    mounted_memory_path = memory_path;
    memory_file = fopen(memory_path, "r+b");
};

void os_ls_processes() {
    fseek(memory_file, PCB_TABLE_START, SEEK_SET);
    PCB pcbs[32];
    fread(&pcbs, sizeof(struct PCB), 32, memory_file);

    for (int i = 0; i < 32; i ++) {
        PCB* pcb = &pcbs[i];
        if (pcb->state == 0x01) {
            printf("Proceso ID: %d, Nombre: %.11s, Estado: %d\n", pcb->pid, pcb->name, pcb->state);
        }
    }
};

int os_exists(int process_id, char* file_name){
    fseek(memory_file, 0, SEEK_SET); //necesito volver al inicio del archivo de mem

    PCB pcb;

    //Tabla de PCBs de 32 entradas
    for (int i = 0; i < 32; i++) {
        fread(&pcb, sizeof(struct PCB), 1, memory_file);

        if (pcb.pid == process_id){ //el PCB tiene el id del proceso?

            for (int j = 0; j < 5; j++) { //ahora busco el archivo dentro del proceso
                char valid = pcb.fileTable[j * 23];

                if (valid == 0x01){ // si la entrada es válida
                    char mem_file_name[15];
                    memcpy(mem_file_name, &pcb.fileTable[j * 23 + 1], 14);
                    mem_file_name[14] = '\0';

                    if (strcmp(mem_file_name, file_name) == 0) {
                        return 1;
                    } 
                }
            }
        }
    }  
    return 0;  
};

void os_ls_files(int process_id) {
    fseek(memory_file, 0, SEEK_SET);
    PCB pcb;

    for (int i = 0; i <  32; i++) {

        fread(&pcb, sizeof(struct PCB), 1, memory_file);

        if (pcb.pid == process_id) {
            printf("\nArchivos\n");
            for (int a = 0; a < 5; a++) {
                osrmsFile* file_entry = (osrmsFile*) &pcb.fileTable[a * 23];
                printf("Nombre: %.14s, Tamaño: %d Bytes\n",
                file_entry->fileName, file_entry->fileSize);
            }  
        }
    }
}

void os_frame_bitmap() {
    fseek(memory_file, FRAME_BITMAP_START, SEEK_SET);
    unsigned char bitmap[8192];
    fread(bitmap, 1, 8192, memory_file);

    int occupied_frames = 0;
    int free_frames = 0;

    for (int i = 0; i < 65536; i++) {
        int byte_index = i/8;
        int bit_index = i%8;
        
        if (bitmap[byte_index] & (1 << bit_index)) {
            occupied_frames++;
            // printf("Bit %d ocupado\n", i);
        } else {
            free_frames++;
            // printf("Bit %d deocupado\n", i);
        }
    }
    printf("Bitmap Frame: Ocupado: %d, Libres: %d\n", occupied_frames, free_frames);
};

void os_tp_bitmap() {
    fseek(memory_file, PAGE_TABLE_BITMAP, SEEK_SET);
    unsigned char byte;
    int occupied_tables = 0;
    int free_tables = 0;
    
    for (int i = 0; i < 128; i++) {
        fread(&byte, sizeof(char), 1, memory_file);

        for (int bit = 0; bit < 8; bit++){
            if (byte & (1 << bit)){
                occupied_tables++;
            } else {
                free_tables++;
            }
        }
    }
    printf("Bitmap de Tablas de Páginas: Ocupadas: %d, Libres: %d\n", occupied_tables, free_tables);
}
