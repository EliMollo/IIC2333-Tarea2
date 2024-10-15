#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"

FILE* memory_file = NULL;
char* mounted_memory_path = NULL;

// Funciones generales
void os_mount(char* memory_path) {
    mounted_memory_path = memory_path;
    memory_file = fopen(memory_path, "r+b");
};

void os_ls_processes() {
    fseek(memory_file, PCB_TABLE_START, SEEK_SET);
    PCB pcbs[32];
    fread(&pcbs, sizeof(PCB), PCB_ENTRIES, memory_file);

    for (int i = 0; i < PCB_ENTRIES; i ++) {
        PCB* pcb = &pcbs[i];
        if (pcb->state == 0x01) {
            printf("Proceso ID: %d, Nombre: %.11s, Estado: %d\n", pcb->pid, pcb->name, pcb->state);
        }
    }
};

int os_exists(int process_id, char* file_name){
    fseek(memory_file, PCB_TABLE_START, SEEK_SET); //necesito volver al inicio del archivo de mem
    PCB pcb;

    //Tabla de PCBs de 32 entradas
    for (int i = 0; i < PCB_ENTRIES; i++) {
        fread(&pcb, sizeof(PCB), 1, memory_file);

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
    fseek(memory_file, PCB_TABLE_START, SEEK_SET);
    PCB pcb;

    for (int i = 0; i < PCB_ENTRIES; i++) {

        fread(&pcb, sizeof(PCB), 1, memory_file);

        if (pcb.pid == process_id) {
            printf("\nArchivos\n");
            for (int a = 0; a < 5; a++) {
                FileEntry* file_entry = (FileEntry*) &pcb.fileTable[a * sizeof(FileEntry)];
                printf("Nombre: %.14s, Tamaño: %d Bytes\n",
                file_entry->fileName, file_entry->fileSize);
            }
            return;
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
    printf("\nBitmap Frame: Ocupado: %d, Libres: %d\n", occupied_frames, free_frames);
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
    printf("\nBitmap de Tablas de Paginas: Ocupadas: %d, Libres: %d\n", occupied_tables, free_tables);
}


// Funciones para procesos
void os_start_process(int process_id, char* process_name) {
    fseek(memory_file, PCB_TABLE_START, SEEK_SET);
    PCB pcb;

    for (int i = 0; i < PCB_ENTRIES; i++) {
        fread(&pcb, sizeof(PCB), 1, memory_file);

        if (pcb.state == 0x00) {
            pcb.state = 0x01;
            pcb.pid = process_id;
            strncpy(pcb.name, process_name, 11);

            for (int j = strlen(process_name); j < 11; j++) {
                pcb.name[j] = '\0';
            }

            memset(pcb.fileTable, 0, sizeof(pcb.fileTable));
            memset(pcb.firstOrderTable, 0, sizeof(pcb.firstOrderTable));

            fseek(memory_file, PCB_TABLE_START + i * PCB_ENTRY_SIZE, SEEK_SET);

            fwrite(&pcb, sizeof(PCB), 1, memory_file);

            printf("\nProceso %d (%s) iniciado exitosamente.\n", process_id, process_name);
            return;
        }
    }
}

void os_finish_process(int process_id) {
    fseek(memory_file, PCB_TABLE_START, SEEK_SET);
    PCB pcb;

    for (int i = 0; i < PCB_ENTRIES; i++) {
        fread(&pcb, sizeof(PCB), 1, memory_file);

        if (pcb.state == 0x01 && pcb.pid == process_id) {
            pcb.state = 0x00;
            pcb.pid = 0;

            memset(pcb.name, 0, sizeof(pcb.name));
            memset(pcb.fileTable, 0, sizeof(pcb.fileTable));
            memset(pcb.firstOrderTable, 0, sizeof(pcb.firstOrderTable));

            fseek(memory_file, PCB_TABLE_START + i * PCB_ENTRY_SIZE, SEEK_SET);

            fwrite(&pcb, sizeof(PCB), 1, memory_file);

            printf("\nProceso %d elimindao exitosamente.\n", process_id);
            return;
        }
    }
}

// Funciones para procesos
osrmsFile* os_open(int process_id, char* file_name, char mode) {
    osrmsFile* file_ptr = (osrmsFile*) calloc(1, sizeof(osrmsFile));
    
    if (mode == 'r') {
        PCB pcb;

        for (int i = 0; i < PCB_ENTRIES; i++) {
            fseek(memory_file, i * PCB_ENTRY_SIZE, SEEK_SET);
            fread(&pcb, sizeof(PCB), 1, memory_file);

            if (pcb.pid == process_id && pcb.state == 0x01) {
                for (int a = 0; a < 5; a++) {
                    FileEntry* file_entry = (FileEntry*) &pcb.fileTable[a * sizeof(FileEntry)];
                    if (strcmp(file_entry->fileName, file_name) == 0) {
                        file_ptr->validationByte = file_entry->validationByte;
                        strncpy(file_ptr->fileName, file_entry->fileName, 14);
                        file_ptr->fileSize = file_entry->fileSize;
                        file_ptr->virtualAddress = file_entry->virtualAddress;
                        file_ptr->mode = mode;
                        memcpy(file_ptr->firstOrderTable, pcb.firstOrderTable, sizeof(pcb.firstOrderTable));
                        return file_ptr;
                    }
                }  
            }
        }
    } else if (mode == 'w') {
        if (!os_exists(process_id, file_name)) {
            strncpy(file_ptr->fileName, file_name, 14);
            file_ptr->validationByte = 0x01;
            file_ptr->fileSize = 0;
            file_ptr->virtualAddress = 0;
            return file_ptr;
        }
    }

    free(file_ptr);
    return NULL;
}

int os_read_file(osrmsFile* file_desc, char* dest) {
    if (file_desc == NULL || file_desc->validationByte != 0x01){
        printf("Error, archivo invalido. \n");
        return -1;
    }

    unsigned int VPN =  (file_desc->virtualAddress & VPNMASK) >> 15;
    unsigned int OFFSET = file_desc->virtualAddress & OFFSETMASK;

    int remaining_size = file_desc->fileSize;
    unsigned int bytes_read = 0;

    while (remaining_size > 0) {
        unsigned int first_order_index = (VPN >> 6) & 0x3F;
        unsigned short SPTN;
        memcpy(&SPTN, &file_desc->firstOrderTable[first_order_index], sizeof(unsigned short));
        
        unsigned int second_order_index = VPN & 0x3F;
        unsigned int second_order_address = SECONDARY_PAGE_TABLE_START + 128 * SPTN + 2 * second_order_index;
        unsigned short PFN;
   
        fseek(memory_file, second_order_address, SEEK_SET);
        fread(&PFN, sizeof(unsigned short), 1, memory_file);

        unsigned int physical_address = (PFN << 15) | OFFSET;
        
        unsigned int bytes_to_read = 0;
        if (OFFSET != 0) {
            bytes_to_read = FRAME_SIZE - OFFSET;
            if (bytes_to_read > remaining_size) {
                bytes_to_read = remaining_size;
            }
        } else if (remaining_size >= FRAME_SIZE) {
            bytes_to_read = FRAME_SIZE;
        } else {
            bytes_to_read = remaining_size;
        }

        fseek(memory_file, physical_address, SEEK_SET);
        fread(dest + bytes_read, 1, bytes_to_read, memory_file);

        bytes_read += bytes_to_read;
        remaining_size -= bytes_to_read;
        OFFSET = 0;
        VPN++;
    }
    return bytes_read;
}


int os_write_file(osrmsFile* file_desc, char* src) {
    if (file_desc == NULL || file_desc->validationByte != 0x01){
        printf("Error, archivo invalido. \n");
        return -1;
    }
    
    unsigned int VPN = (file_desc->virtualAddress & VPNMASK) >> 15;
    unsigned int offset = file_desc->virtualAddress & OFFSETMASK;
    unsigned int remaining_size = file_desc->fileSize;
    unsigned int bytes_written = 0; //datos escritos en mem
    const unsigned int frame_size = 32*1024;
    
    while (remaining_size > 0) {
        fseek(memory_file, SECONDARY_PAGE_TABLE_START + VPN*2, SEEK_SET);
        unsigned short frame_number;
        fread(&frame_number, sizeof(unsigned short), 1, memory_file);

        unsigned int frame_address = FRAME_TABLE_START + (frame_number * frame_size);
        fseek(memory_file, frame_address + offset, SEEK_SET);

        unsigned int bytes_to_write = (remaining_size < (frame_size - offset)) ? remaining_size : (frame_size - offset);

        fwrite(src + bytes_written, 1, bytes_to_write, memory_file);

        bytes_written += bytes_to_write;
        remaining_size -= bytes_to_write;
        VPN++; //pasar a la siguiente pagina
        offset = 0; //resetear offset

        //no quedan frames o no hay espacio
        if (frame_number == 0xFFFF) {
            printf("Error: no se pudo traducir VPN %d\n", VPN);
            return bytes_written;
        }
    }
    return bytes_written;
}

void os_close(osrmsFile* file_desc) {
    if (file_desc != NULL) {
        free(file_desc);
        file_desc = NULL;
        printf("Archivo cerrado.\n");
    }
}